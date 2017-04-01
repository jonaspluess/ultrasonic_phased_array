################################################################################
#
#  Copyright (C) 2016, 2017, Mijnssen Raphael, Pluess Jonas
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301, USA.
#
################################################################################

from threading import Thread
import signal
import sys
import re
import itertools

import usb
from numpy import *

from dsp import DSP


################################################################################
# class FirmwareController
################################################################################
class FirmwareController:
	# usb definitions
	VID = 0x03eb
	PID = 0x2404
	IFACE = 1
	ep_in = None
	ep_out = None

	# control definitions
	thread = None
	running = None
	callback_function = None

	# Input settings
	angle_min = 0		# minimal angle [deg]
	angle_max = 0		# maximal angle [deg]
	aperture = 0		# aperture index
	num_bursts = 0		# number of ultrasonic bursts
	rec_buf_cnt = 0		# number of receiver buffers

	# main loop variables
	angle = 0		# actual angle
	y_env_downsamp = []	# downsampled values

	# signal processing variables
	sonic_speed = 0.0	# sonic speed [m/s]
	upsample_factor = 4	# upsampling factor
	downsample_factor = 4	# downsampling factor


	########################################################################
	# control functions
	########################################################################
	def __init__(self):
		print("FirmwareController: init")
		signal.signal(signal.SIGINT, self.sighandler)

		self.running = 0
		self.callback_function = self.empty_callback
		return

	def start(self):
		print("FirmwareController: start")
		self.usb_init(self.VID, self.PID, self.IFACE)
		self.running = 1
		self.thread = Thread(target=self.main_loop)
		self.thread.start()
		return

	def stop(self):
		print("FirmwareController: stop")
		self.running = 0;
		self.thread.join()
		return

	def sighandler(self, signum, frame):
		print("FirmwareController: sigint caught, exiting.")
		if signum == signal.SIGINT:
			self.stop()
			sys.exit(0)
		return


	########################################################################
	# server interface functions
	########################################################################
	def register_callback(self, callback_function):
		print("FirmwareController: register_callback")
		self.callback_function = callback_function
		return

	def empty_callback(self):
		print("FirmwareController: empty_callback")
		return

	def update_values(self, angle_min, angle_max, aperture, num_bursts, rec_buf_cnt, sonic_speed):
		print("FirmwareController: update_values")
		self.angle_min = angle_min
		self.angle_max = angle_max
		self.aperture = aperture
		self.num_bursts = num_bursts
		self.rec_buf_cnt = rec_buf_cnt
		self.sonic_speed = sonic_speed
		return

	def get_data(self):
		return self.angle, self.angle_min, self.angle_max, self.y_env_downsamp, self.rec_buf_cnt, self.upsample_factor, self.downsample_factor


	########################################################################
	# usb functions
	########################################################################
	# initialize usb transmission
	def usb_init(self, vid, pid, iface):
		dev = usb.core.find(idVendor=vid, idProduct=pid)

		if dev is None:
			raise ValueError('FirmwareController: usb_init - device not found.')

		try:
			if dev.is_kernel_driver_active(iface):
				try:
					dev.detach_kernel_driver(iface)
				except usb.core.USBError as error:
					sys.exit('FirmwareController: usb_init - coult not detach kernel driver: ' + str(error))
		except NotImplementedError:
			pass

		try:
			usb.util.claim_interface(dev, iface)
		except usb.core.USBError as error:
			sys.exit('FirmwareController: usb_init - could not get the device' + str(error))

		cfg = dev.get_active_configuration()
		intf = cfg[(iface, 0)]

		self.ep_in = usb.util.find_descriptor(
			intf,
			custom_match= \
			lambda e: \
				usb.util.endpoint_direction(e.bEndpointAddress) == \
				usb.util.ENDPOINT_IN)

		self.ep_out = usb.util.find_descriptor(
			intf,
			custom_match= \
			lambda e: \
				usb.util.endpoint_direction(e.bEndpointAddress) == \
				usb.util.ENDPOINT_OUT)

		assert self.ep_in is not None
		assert self.ep_out is not None

		print('FirmwareController: usb_init - EP_IN is ', str(self.ep_in.bEndpointAddress))
		print('FirmwareController: usb_init - EP_OUT is ', str(self.ep_out.bEndpointAddress))
		return

	# send configuration data to firmware via usb
	def usb_tx(self):
		# create transmit buffer
		tx_buffer = bytearray(b'\x00' * 1024)
		tx_buffer[0] = int(self.angle & 2**8-1)		# angle
		tx_buffer[1] = int(self.aperture & 2**8-1)	# aperture
		tx_buffer[2] = int(self.num_bursts & 2**8-1)	# number of bursts to stimulate the piezo
		tx_buffer[3] = int(self.rec_buf_cnt & 2**8-1)	# max number of received buffers

		self.ep_out.write(tx_buffer,1000)
		return

	# receive and extract data from firmware via usb
	def usb_rx(self):
		# static firmware definitions
		REC_NUM_CHANNELS = 8
		USB_BUF_LEN = 1024

		ch_cnt = 0	# channel counter
		c = []		# channel array
		for i in range(REC_NUM_CHANNELS):
			c.append([])

		while True:
			data_usb = self.ep_in.read(USB_BUF_LEN, 10000)
			data_int16 = frombuffer(data_usb, dtype=int16)
			if len(data_int16) > 0:
				if sum(data_int16[int(USB_BUF_LEN/4):]) == -256:
					ch_cnt = 0
					msg = "".join([chr(x) for x in frombuffer(data_usb, dtype=int8)[:int(USB_BUF_LEN/2)]])
					msg_gr = re.search(r'counter\:(\d+),flag\:(\d+),angle\:(-?\d+)', msg).groups()
					msg_counter = int(msg_gr[0])
					msg_flag = int(msg_gr[1])
					msg_angle = int(msg_gr[2])
					#print(msg)

					if msg_flag == 1:
						break
				else:
					c[ch_cnt].append(data_int16)
					ch_cnt = ch_cnt + 1

		for i in range(REC_NUM_CHANNELS):
			c[i] = list(itertools.chain.from_iterable(c[i]))

		return c, msg_angle


	########################################################################
	# main loop
	########################################################################
	def main_loop(self):
		while self.running == 1:
			if self.angle_min < self.angle_max:
				if self.angle >= self.angle_max or self.angle < self.angle_min:
					self.angle = self.angle_min
				else:
					self.angle = self.angle + 1
			else:
				if self.angle <= self.angle_max or self.angle > self.angle_min:
					self.angle = self.angle_min
				else:
					self.angle = self.angle - 1

			self.usb_tx()
			ch_data_raw, angle_sent = self.usb_rx()

			y_shift = []
			for i in range(len(ch_data_raw)):
				y_shift.append(array(ch_data_raw[i]) - 2048.0)

			y_shifted, Y_shifted, y_upsamp, Y_upsamp, y_beam_formed, Y_beam_formed = DSP.upsamp_beam_form(y_shift, -angle_sent/180.0*pi, self.aperture, self.sonic_speed, self.upsample_factor)

			y_env_downsamp = DSP.downsamp_env(y_beam_formed, self.downsample_factor)

			self.y_env_downsamp = y_env_downsamp.tolist()
			self.callback_function()
		return

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

import sys
import socket
import getopt
import os.path
import usb
import re
import itertools
import time
from numpy import *
from scipy.signal import hilbert
import matplotlib.pyplot as plt
from matplotlib import animation
import pyfftw

################################################################################
# definitions
################################################################################
COPYRIGHT	= "Copyright (C) 2016, Mijnssen Raphael, Pluess Jonas"
LICENCE		= "Licence GPLv2+: GNU GPL version 2 or later <http://gnu.org/licences/gpl.html>.\nThis is free software: you are free to change and redistribute it.\nThere is NO WARRANTY, to the extent permitted by law."

# usb device definitions
VID = 0x03eb
PID = 0x2404
IFACE = 1

# data reception definitions
USB_BUF_LEN		= 1024
REC_NUM_CHANNELS	= 8
REC_NUM_BANKS		= 4
SERVER_ADDRESS = ('localhost', 10000)

# dignal signal processing definitions
SAMP_FREQ	= 32000		# [Hz]
UPSAMP_FACTOR	= 32


# physical definitions
SONIC_SPEED = 343.0		# [m/s]
SENSOR_DISTANCE = 5.4e-3	# [m]
# delay between channels (adc multiplex delay)
CHANNEL_DELAY	= 2e-6		# [s]
# upsampled channel has to be shifted CHAN_SHIFT_NUM values right
CHAN_SHIFT_NUM	= int(CHANNEL_DELAY * SAMP_FREQ * UPSAMP_FACTOR)

# mode definitions
# MODE_USB_RAW: print raw data to stdout or to a file
# MODE_PLOT: plot raw data
# MODE_TCP: act as tcp-server
MODE_USB_RAW = 0
MODE_PLOT = 1
MODE_MAX = 1

################################################################################
# global variables
################################################################################
aperture = 0
angle = 0
num_bursts = 10
receiver_buffer_cnt = 4
ep_in = 0
ep_out = 0
lines = []
mode = MODE_PLOT

################################################################################
# functions
################################################################################
def usage():
	print("")
	print("This program ...")
	print("Usage:")
	print("\t" + sys.argv[0] + " \'filename\'")
	print("")

def version():
	print("")
	print(COPYRIGHT)
	print("")
	print(LICENCE)
	print("");

# init the usb endpoint
def usb_init(vid, pid, iface):
	dev = usb.core.find(idVendor=vid, idProduct=pid)

	if dev is None:
		raise ValueError('Device not found.')

	if dev.is_kernel_driver_active(iface):
		try:
			dev.detach_kernel_driver(iface)
		except usb.core.USBError as error:
			sys.exit('Coult not detach kernel driver: ' + str(error))

	try:
		usb.util.claim_interface(dev, iface)
	except usb.core.USBError as error:
		sys.exit('Could not get the device' + str(error))

	cfg = dev.get_active_configuration()
	intf = cfg[(iface, 0)]

	ep_in = usb.util.find_descriptor(
		intf,
		custom_match= \
		lambda e: \
			usb.util.endpoint_direction(e.bEndpointAddress) == \
			usb.util.ENDPOINT_IN)

	ep_out = usb.util.find_descriptor(
		intf,
		custom_match= \
		lambda e: \
			usb.util.endpoint_direction(e.bEndpointAddress) == \
			usb.util.ENDPOINT_OUT)

	assert ep_in is not None
	assert ep_out is not None

	print('EP_IN: ', str(ep_in.bEndpointAddress))
	print('EP_OUT: ', str(ep_out.bEndpointAddress))

	return ep_in, ep_out

# get values from usb device, fill them to a buffer
def usb_transmit_data():
	global ep_in, ep_out
	global aperture
	global angle
	global num_bursts
	global receiver_buffer_cnt

	ch_cnt = 0	# channel counter
	c = []		# channel array
	for i in range(REC_NUM_CHANNELS):
		c.append([])

	# create transmit buffer
	tx_buffer = bytearray(b'\x00' * 1024)
	tx_buffer[0] = int(angle & 2**8-1)		# angle
	tx_buffer[1] = int(aperture & 2**8-1)		# aperture
	tx_buffer[2] = int(num_bursts & 2**8-1)		# number of bursts to stimulate the piezo
	tx_buffer[3] = int(receiver_buffer_cnt & 2**8-1)# max number of received buffers

	angle = angle + 1
#	angle=0
	if angle > 30:
		angle = -30

	ep_out.write(tx_buffer,1000)

	while True:
		data_usb = ep_in.read(USB_BUF_LEN, 10000)
		data_int16 = frombuffer(data_usb, dtype=int16)
		if len(data_int16) > 0:
			if sum(data_int16[USB_BUF_LEN/4:]) == -256:
				ch_cnt = 0
				msg = "".join([chr(x) for x in frombuffer(data_usb, dtype=int8)[:USB_BUF_LEN/2]])
				msg_gr = re.search(r'counter\:(\d+),flag\:(\d+),angle\:(-?\d+)', msg).groups()
				#TODO
				msg_counter = int(msg_gr[0])
				msg_flag = int(msg_gr[1])
				msg_angle = int(msg_gr[2])
				print(msg)

				if msg_flag == 1:
					break
			else:
				c[ch_cnt].append(data_int16)
				ch_cnt = ch_cnt + 1

	for i in range(REC_NUM_CHANNELS):
		c[i] = list(itertools.chain.from_iterable(c[i]))

	return c, msg_angle

# beam forming and upsampling
def upsamp_beam_form(y, phi_rad):
	j = complex(0.0, 1.0)
	length = len(y[0])
	#TODO
	#f = linspace(-SAMP_FREQ/2, SAMP_FREQ/2*(1.0-2.0/length), length)
	#f = fft.fftshift(f)
	f1 = linspace(32000, 48000.0-32000.0/length, length/2)
	f2 = linspace(-48000,-32000-32000.0/length,length/2)
	f = concatenate((f1,f2), axis=0)

	y_shifted = []
	Y_shifted = []
	y_upsamp = []
	Y_upsamp = []
	y_beam_formed = zeros(length * UPSAMP_FACTOR)
	Y_beam_formed = zeros(length * UPSAMP_FACTOR)*j

	for i in range(len(y)):
		#delta_tau = i * SENSOR_DISTANCE * sin(phi_rad) / SONIC_SPEED + i * CHANNEL_DELAY
		delta_tau = i * SENSOR_DISTANCE * sin(phi_rad) / SONIC_SPEED + i * CHANNEL_DELAY
		shift = exp(-j * 2 * pi * f * delta_tau)

		Y_shifted.append(fft.fft(y[i]) * shift)
		# reset DC part
		Y_shifted[i][0] = 0.0;
		#y_shifted.append(real(fft.ifft(Y_shifted[i])))
		Y_upsamp.append(concatenate((zeros(len(Y_shifted[i])), Y_shifted[i][:1024], zeros(int(2*(UPSAMP_FACTOR*SAMP_FREQ/2.0-3.0/2*SAMP_FREQ)/(SAMP_FREQ/2048.0))), Y_shifted[i][1024:], zeros(len(Y_shifted[i]))), axis=0))
		y_upsamp.append(real(fft.ifft(Y_upsamp[i])))

		#Y_beam_formed += Y_upsamp[i]
		y_beam_formed += y_upsamp[i]

	return y_shifted, Y_shifted, y_upsamp, Y_upsamp, y_beam_formed, Y_beam_formed

# init plot
def init_plot():
	global lines

	fig = plt.figure()

	s_marker = [
		'None',
		'None',
		'None',
		'None',
		'None',
		'None',
		'None',
		'None']

	s_label = [
		'1',
		'2',
		'3',
		'4',
		'5',
		'6',
		'7',
		'8']

	s_color = [
		'#ff0000',
		'#00ff00',
		'#0000ff',
		'#009900',
		'#ffff00',
		'#bbbbbb',
		'#777777',
		'#000000']


	########################################################################
	# Subplot 1
	ax1 = fig.add_subplot(311)
	lines = []
	for i in range(REC_NUM_CHANNELS):
		lines.append(ax1.plot([], [], lw=1, marker=s_marker[i], color=s_color[i], label=s_label[i])[0])

	plt.title('')
	plt.xlim(0, 1.0/SAMP_FREQ*USB_BUF_LEN/2*REC_NUM_BANKS)
	plt.ylim(-100, 100)
	plt.xlabel("t [s]")
	plt.ylabel("")
	plt.legend(loc='best')

	########################################################################
	# Subplot 2
	ax2 = fig.add_subplot(312)
	for i in range(REC_NUM_CHANNELS):
		lines.append(ax2.plot([], [], lw=0, marker='.', color=s_color[i], label=s_label[i])[0])

	plt.xlim(32000, 48000)
	#plt.xlim(-512000, 512000)
	plt.ylim(0, 80000)
	plt.xlabel("f [Hz]")
	plt.ylabel("")
	#plt.legend(loc='best')

	########################################################################
	# Subplot 3
	ax3 = fig.add_subplot(313)
	lines.append(ax3.plot([], [], lw=1, marker='None', color='black', label='beam formed')[0])
	lines.append(ax3.plot([], [], lw=1, marker='None', color='yellow', label='beam formed')[0])
	lines.append(ax3.plot([], [], lw=1, marker='None', color='green', label='beam formed')[0])

	plt.xlim(0, 1.0/SAMP_FREQ*USB_BUF_LEN/2*REC_NUM_BANKS)
	plt.ylim(-700, 700)
	plt.xlabel("t [s]")
	plt.ylabel("")
	#plt.legend(loc='best')

	return fig

# init the lines of the plot (called from animation)
def init_lines():
	global lines

	for i in range(len(lines)):
		lines[i].set_data([], [])

	return lines

#	fill the lines with data (called from animation as callback function)
def get_lines(i):
	global lines

	y , angle2 = usb_transmit_data()
	y_shift = []
	for i in range(len(y)):
		y_shift.append(array(y[i]) - 2048)

	y_shifted, Y_shifted, y_upsamp, Y_upsamp, y_beam_formed, Y_beam_formed = upsamp_beam_form(y_shift, -angle2/180.0*pi)

	# calculate envelope and downsample
	analytic_signal = hilbert(y_beam_formed)
	y_beam_formed_env = abs(analytic_signal)
	y_beam_formed_env_downsampled = y_beam_formed_env[0:len(y_beam_formed_env):64]
	print(len(y_beam_formed_env))
	print(len(y_beam_formed_env_downsampled))
	print(len(linspace(0, 1.0/SAMP_FREQ*len(y[0]), len(y[0])*UPSAMP_FACTOR/32)))

	for i in range(len(lines)):
		if i < REC_NUM_CHANNELS:
			lines[i].set_data(linspace(0, 1.0/SAMP_FREQ*len(y[i]), len(y[i])*UPSAMP_FACTOR), y_upsamp[i])
		elif i < 2 * REC_NUM_CHANNELS:
			yfft = Y_upsamp[i-REC_NUM_CHANNELS]
			xfft = SAMP_FREQ * UPSAMP_FACTOR * fft.fftfreq(shape(Y_upsamp[i-REC_NUM_CHANNELS])[-1])
			lines[i].set_data(xfft,abs(yfft))
		elif i == len(lines)-3:
			lines[i].set_data(linspace(0, 1.0/SAMP_FREQ*len(y[i-2*REC_NUM_CHANNELS]), len(y[i-2*REC_NUM_CHANNELS])*UPSAMP_FACTOR), y_beam_formed)
		elif i == len(lines)-2:
			lines[i].set_data(linspace(0, 1.0/SAMP_FREQ*len(y[i-2*REC_NUM_CHANNELS]), len(y[i-2*REC_NUM_CHANNELS])*UPSAMP_FACTOR), y_beam_formed_env)
		elif i == len(lines)-1:
			lines[i].set_data(linspace(0, 1.0/SAMP_FREQ*len(y[0]), len(y[0])*UPSAMP_FACTOR/64), y_beam_formed_env_downsampled)
		else:
			pass

	return lines


################################################################################
# main
################################################################################
def main():
	global ep_in, ep_out
	global lines
	global b, a
	global mode

	try:
		opts, args = getopt.getopt(sys.argv[1:], "hvm:", ["help", "version", "mode="])
	except getopt.GetoptError as err:
		print str(err)
		usage()
		sys.exit(2)

	for opt, arg in opts:
		if opt in ("-h", "--help"):
			usage()
			sys.exit()
		elif opt in ("-v", "--version"):
			version()
			sys.exit()
		elif opt in ("-m", "--mode"):
			mode = int(arg)
			if mode > MODE_MAX or mode < 0:
				print("Unknown mode.")
				usage()
				sys.exit(2)
		else:
			assert False
			usage()
			sys.exit(2)

	#TODO: system error handling

	#init usb
	ep_in, ep_out = usb_init(VID, PID, IFACE)

	#TODO: check if necessary
	usb_transmit_data()

	if mode == MODE_USB_RAW:
		print("start")
		for i in range(10):
			c, msg_angle = usb_transmit_data()
			print('')
			print("sending angle:", msg_angle)
			for j in range(REC_NUM_CHANNELS):
				print('')
				print("channel:", j)
				print(c[j])
		print('')
		print("stop")
	elif mode == MODE_PLOT:
		fig = init_plot()
		anim = animation.FuncAnimation(fig, get_lines, init_func=init_lines, frames=100000, interval=0.0, blit=True)
		plt.show()
	else:
		print("error: selected mode not programmed yet")
		usage()
		sys.exit(2)


################################################################################
#
################################################################################

if __name__ == "__main__":
	main()

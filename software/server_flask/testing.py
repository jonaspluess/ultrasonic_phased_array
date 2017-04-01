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

import time
import sys
import getopt
import os.path

from numpy import *
import matplotlib.pyplot as plt

from firmware_controller import FirmwareController
from dsp import DSP


################################################################################
# class definitions
################################################################################
class Test():
	def __init__(self, angle, aperture, num_bursts):
		self.angle_min = angle		# test specification
		self.angle_max = angle		# test specification
		self.aperture = aperture	# test specification
		self.num_bursts = num_bursts	# test specification
		self.sensor_distance = 0.0054	# sensor distance [m]
		self.rec_buf_cnt = 1		# unimportant
		self.sonic_speed = 343.0	# unimportant
		self.upsample_factor = 32	# unimportant

		self.running = 1


	def pol2cart(self, rho, phi):
		return (rho * cos(phi), rho * sin(phi))


	def get_aperture(self, aperture, max_n):
		APERTURE_RECT = 0
		APERTURE_COS = 1
		APERTURE_COS2 = 2
		APERTURE_GAUSS = 3
		STDEV_GAUSS = 0.5

		res = []
		for i in range(max_n):
			if aperture == APERTURE_RECT:
				res.append(1.0)
			elif aperture == APERTURE_COS:
				res.append(sin(pi*i/((max_n+2)-1)))
			elif aperture == APERTURE_COS2:
				res.append(pow(sin(pi*i/((max_n+2)-1)),2))
			elif aperture == APERTURE_GAUSS:
				res.append(exp(-0.5*pow(((i-1)-(max_n-1)/2.0)/(STDEV_GAUSS*(max_n-1)/2.0),2)))
			else:
				res.append(0.0)
		return res


	def calc_characteristics(self, d, lamb, phi_0, phi, N, aperture):
		j = complex(0.0, 1.0)
		aperture = self.get_aperture(aperture, int(N))
		H = zeros(phi.shape) * complex(0.0, 0.0)
		for i in range(int(N)):
			H += exp(j*2*pi*d/lamb*(N-i+1)*(sin(phi_0)-sin(phi))) * complex(aperture[i], 0.0)
		return abs(H)/max(abs(H))


	########################################################################
	# main loop
	########################################################################
	def main_loop(self, firmware_controller):
		while self.running == 1:
			firmware_controller.angle = firmware_controller.angle_min

			firmware_controller.usb_tx()
			ch_data_raw, angle_sent = firmware_controller.usb_rx()

			y_shift = []
			for i in range(len(ch_data_raw)):
				y_shift.append(array(ch_data_raw[i]) - 2048.0)
				y_shift[i][:128] = 0





			########################################################
			# calculate data
			########################################################
			aperture_ = [0, 1, 2, 3]
			aperture_s = ["rect", "cos", "cos2", "gauss"]
			aperture_S = ["$Rechteck$", "$cos$", "$cos^2$", "$Gauss$"]

			phi_0 = firmware_controller.angle
			phi = linspace(-90.0, 90.0, 901)

			y_beam_formed_a = []
			max_beam_formed = []
			for aperture in aperture_:
				y_beam_formed_a.append([])
				max_beam_formed.append([])

				for angle in phi:
					print("Test: calculating aperture = " + str(aperture_s[aperture]) + ", angle = " + str(angle) + " degree")

					y_shifted, Y_shifted, y_upsamp, Y_upsamp, y_beam_formed, Y_beam_formed = DSP.upsamp_beam_form(y_shift, -angle/180.0*pi, aperture, self.sonic_speed, self.upsample_factor)

					y_beam_formed_a[aperture].append(y_beam_formed)
					max_beam_formed[aperture].append(max(y_beam_formed))

					if angle == phi_0 and aperture == 0:
						plt.plot(y_beam_formed)
						plt.show()


			for aperture in aperture_:
				################################################
				# fig:plot_test_characteristic_receiver_[angle]_deg_send_[aperture_sent]_receive_[aperture_received]_[num_bursts]_bursts
				################################################
				color_s = ["", "", "", ""]
				fig = plt.figure(figsize=(5,5))

				for k in range(-5, 6):
					x1, y1 = self.pol2cart(1.0, 10.0*k/180.0*pi)
					plt.plot([0, x1], [0, y1], marker='', color='lightgray')

				H = self.calc_characteristics(self.sensor_distance, self.sonic_speed/40000.0, phi_0/180.0*pi, phi/180.0*pi, 8, aperture)
				H = abs(H) / max(abs(H))
				x, y = self.pol2cart(H, phi/180.0*pi)

				plt.plot(x, y, marker='', color='green', label='Berechnete Werte')

				max_beam_formed[aperture] = max_beam_formed[aperture] / max(max_beam_formed[aperture])
				x, y = self.pol2cart(max_beam_formed[aperture], phi/180.0*pi)
				plt.plot(x, y, marker='', color='blue', label='Messwerte')

				#plt.xlabel('Abstrahlwinkel [Grad]')
				#plt.ylabel('Richtcharakteristik abs(H(phi))')
				plt.xlim(min(phi), max(phi))
				plt.ylim(0, 1)

				plt.axis('equal')
				plt.xlim(0.0,1.0)
				plt.ylim(-0.5, 0.5)


				plt.minorticks_on()
				plt.grid()
				plt.legend(loc='lower right')
				plt.show()
				#fig.savefig('test/plot_test_characteristic_receiver_' + str(phi_0)+ '_deg_send_' + str(aperture_s[firmware_controller.aperture]) + '_receive_' + str(aperture_s[aperture]) +'_' + str(firmware_controller.num_bursts) + '_bursts.png')



			self.running = 0
		return


################################################################################
# usage
################################################################################
def usage():
	print("usage: python testing.py --angle x --aperture y --num_bursts z")
	print("\t angle:\t\t integer between -30 and 30")
	print("\t aperture:\t integer between 0 and 3")
	print("\t\t 0:\t aperture rect")
	print("\t\t 1:\t aperture cos")
	print("\t\t 2:\t aperture cos**2")
	print("\t\t 3:\t aperture gauss")
	print("\t num_bursts:\t integer between 0 and 40")


################################################################################
# main
################################################################################
if __name__ == '__main__':
	angle = 0;
	aperture = 0
	num_bursts = 10

	try:
		opts, args = getopt.getopt(sys.argv[1:], "ha:A:n:", ["help", "angle=", "aperture=", "num_bursts="])
	except getopt.GetoptError as err:
		print str(err)
		usage()
		sys.exit(2)

	for opt, arg in opts:
		if opt in ("-a", "--angle"):
			angle = int(arg)
		elif opt in ("-A", "--aperture"):
			aperture = int(arg)
		elif opt in ("-n", "--num_bursts"):
			num_bursts = int(arg)
		elif opt in ("-h", "--help"):
			usage()
			sys.exit()
		else:
			usage()
			assert False
			sys.exit(2)

	time.sleep(2)
	firmware_controller = FirmwareController()
	test = Test(angle, aperture, num_bursts)

	firmware_controller.update_values(test.angle_min, test.angle_max, test.aperture, test.num_bursts, test.rec_buf_cnt, test.sonic_speed)
	firmware_controller.start()
	firmware_controller.stop()

	test.main_loop(firmware_controller)

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

from numpy import *
from scipy.signal import hilbert


################################################################################
# digital signal processing class
################################################################################
class DSP:
	# upsampling and beamforming
	@staticmethod
	def upsamp_beam_form(y, phi_rad, aperture, sonic_speed, upsample_factor):
		# static firmware definitions
		SAMP_FREQ	= 32000		# [Hz]
		SENSOR_DISTANCE = 5.4e-3	# [m]
		# delay between channels (adc multiplex delay)
		CHANNEL_DELAY	= 2e-6		# [s]


		j = complex(0.0, 1.0)
		length = len(y[0])
		f1 = linspace(SAMP_FREQ, (3*SAMP_FREQ/2.0)-(SAMP_FREQ/length), length/2)
		f2 = linspace(-(3*SAMP_FREQ/2.0),-SAMP_FREQ-(SAMP_FREQ/length),length/2)
		f = concatenate((f1,f2), axis=0)

		y_shifted = []
		Y_shifted = []
		y_upsamp = []
		Y_upsamp = []
		y_beam_formed = zeros(length * upsample_factor)
		Y_beam_formed = zeros(length * upsample_factor)*j

		aperture_a = DSP.get_aperture(aperture, len(y))

		for i in range(len(y)):
			delta_tau = i * SENSOR_DISTANCE * sin(phi_rad) / sonic_speed + i * CHANNEL_DELAY
			shift = exp(-j * 2 * pi * f * delta_tau)

			Y_shifted.append(fft.fft(y[i] * aperture_a[i]) * shift)
			# reset DC part
			Y_shifted[i][0] = 0.0;
			#y_shifted.append(real(fft.ifft(Y_shifted[i])))
			Y_upsamp.append(concatenate((zeros(len(Y_shifted[i])), Y_shifted[i][:int(length/2)], zeros(int(2*(upsample_factor*SAMP_FREQ/2.0-3.0/2*SAMP_FREQ)/(SAMP_FREQ/float(length)))), Y_shifted[i][int(length/2):], zeros(len(Y_shifted[i]))), axis=0))
			y_upsamp.append(real(fft.ifft(Y_upsamp[i])))
			#Y_beam_formed += Y_upsamp[i]
			y_beam_formed += y_upsamp[i]

		return y_shifted, Y_shifted, y_upsamp, Y_upsamp, y_beam_formed, Y_beam_formed

	# signal envelope and downsampling
	@staticmethod
	def downsamp_env(y, downsample_factor):
		analytic_signal = hilbert(y)
		y_env = abs(analytic_signal)
		y_env_dowsamp = y_env[0:len(y_env):downsample_factor]
		return y_env_dowsamp

	# return aperture
	@staticmethod
	def get_aperture(aperture, max_n):
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

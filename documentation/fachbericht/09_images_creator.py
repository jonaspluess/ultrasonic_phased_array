################################################################################
#
#  Copyright (C) 2016, 2017, Pluess Jonas
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
from scipy.optimize import curve_fit
import matplotlib.pyplot as plt
import scipy.signal as signal


################################################################################
# definitions
################################################################################
APERTURE_RECT = 0
APERTURE_COS = 1
APERTURE_COS2 = 2
APERTURE_GAUSS = 3
STDEV_GAUSS = 0.5


################################################################################
# function definitions
################################################################################

# read a .txt file to an array
def readarray(filename):
	array = loadtxt(filename, dtype="float", comments='#', delimiter="\t", converters=None, skiprows=0, usecols=None, unpack=False, ndmin=0)
	return array

# save an array to a .txt file
def savearray(array, filename):
	savetxt(filename, array, fmt='%.18e', delimiter='\t', newline='\n', header='', footer='', comments='# ')

def pol2cart(rho, phi):
	return (rho * cos(phi), rho * sin(phi))


# beam forming and upsampling
def upsamp_beam_form(y, phi_rad, aperture):
	# static firmware definitions
	SAMP_FREQ	= 32000		# [Hz]
	UPSAMP_FACTOR = 32
	SENSOR_DISTANCE = 5.4e-3	# [m]
	# delay between channels (adc multiplex delay)
	CHANNEL_DELAY	= 2e-6		# [s]

	j = complex(0.0, 1.0)
	length = len(y[0])
	#TODO
	#f = linspace(-SAMP_FREQ/2, SAMP_FREQ/2*(1.0-2.0/length), length)
	#f = fft.fftshift(f)
	f1 = linspace(SAMP_FREQ, (3*SAMP_FREQ/2.0)-(SAMP_FREQ/length), length/2)
	f2 = linspace(-(3*SAMP_FREQ/2.0),-SAMP_FREQ-(SAMP_FREQ/length),length/2)
	f = concatenate((f1,f2), axis=0)

	y_shifted = []
	Y_shifted = []
	y_upsamp = []
	Y_upsamp = []
	y_beam_formed = zeros(length * UPSAMP_FACTOR)
	Y_beam_formed = zeros(length * UPSAMP_FACTOR)*j

	aperture = self.get_aperture(aperture, len(y))

	for i in range(len(y)):
		delta_tau = i * SENSOR_DISTANCE * sin(phi_rad) / self.sonic_speed + i * CHANNEL_DELAY
		shift = exp(-j * 2 * pi * f * delta_tau)

		Y_shifted.append(fft.fft(y[i] * aperture[i]) * shift)
		# reset DC part
		Y_shifted[i][0] = 0.0;
		#y_shifted.append(real(fft.ifft(Y_shifted[i])))
		Y_upsamp.append(concatenate((zeros(len(Y_shifted[i])), Y_shifted[i][:length/2], zeros(int(2*(UPSAMP_FACTOR*SAMP_FREQ/2.0-3.0/2*SAMP_FREQ)/(SAMP_FREQ/float(length)))), Y_shifted[i][length/2:], zeros(len(Y_shifted[i]))), axis=0))
		y_upsamp.append(real(fft.ifft(Y_upsamp[i])))
		#Y_beam_formed += Y_upsamp[i]
		y_beam_formed += y_upsamp[i]

	return y_shifted, Y_shifted, y_upsamp, Y_upsamp, y_beam_formed, Y_beam_formed

# signal envelope and downsampling
def downsamp_env(y, r_downsamp):
	analytic_signal = hilbert(y)
	y_env = abs(analytic_signal)
	y_env_dowsamp = y_env[0:len(y_env):r_downsamp]
	return y_env_dowsamp

# return aperture
def get_aperture(aperture, max_n):
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

def calc_characteristics(d, lamb, phi_0, phi, N, aperture):
	j = complex(0.0, 1.0)
	aperture = get_aperture(aperture, int(N))
	H = zeros(phi.shape) * complex(0.0, 0.0)
	for i in range(int(N)):
		H += exp(j*2*pi*d/lamb*(N-i+1)*(sin(phi_0)-sin(phi))) * complex(aperture[i], 0.0)
	return abs(H)/max(abs(H))


################################################################################
# fig:plot_grundlagen_sharpness_factor_calc_0
################################################################################
phi_0 = linspace(0, 53.0/180*pi, 1000)
lamb = 343.0/40000
d = 5.4e-3
N = 8

fig = plt.figure(figsize=(10,5))

q = 1.0/pi*(arcsin(sin(phi_0)+lamb/(N*d))-arcsin(sin(phi_0)-lamb/(N*d)))
plt.plot(phi_0/pi*180, q, marker='')

plt.xlabel('Abstrahlwinkel [Grad]')
plt.ylabel('Schaerfefaktor q')
plt.minorticks_on()
plt.grid()
#plt.show()
fig.savefig('graphics/plot_grundlagen_sharpness_factor_calc_0.png')
plt.close()


################################################################################
# fig:plot_grundlagen_sharpness_factor_calc_1
################################################################################
phi_0 = 20.0/180*pi
lamb = 343.0/40000
d = 5.4e-3
N = arange(4, 64+1)

fig = plt.figure(figsize=(10,5))

q = 1.0/pi*(arcsin(sin(phi_0)+lamb/(N*d))-arcsin(sin(phi_0)-lamb/(N*d)))
plt.plot(N, q, marker='.')

plt.xlabel('Anzahl Schallquellen N')
plt.ylabel('Schaerfefaktor q')
plt.minorticks_on()
plt.grid()
#plt.show()
fig.savefig('graphics/plot_grundlagen_sharpness_factor_calc_1.png')
plt.close()


################################################################################
# fig:plot_grundlagen_characteristic_calc_0
################################################################################
phi_0 = 20.0
phi = linspace(-90.0, 90.0, 1000)
lamb = 343.0/40000
d = lamb / 2.0
N_ = [8.0, 16.0, 24.0, 32.0]
aperture = APERTURE_RECT

fig = plt.figure(figsize=(10,5))

for N in N_:
	H = calc_characteristics(d, lamb, phi_0/180.0*pi, phi/180.0*pi, N, aperture)
	plt.plot(phi, H, marker='', label='N='+str(int(N)))

plt.xlabel('Abstrahlwinkel [Grad]')
plt.ylabel('Richtcharakteristik abs(H(phi))')
plt.xlim(min(phi), max(phi))
plt.ylim(0, 1)
plt.minorticks_on()
plt.grid()
plt.legend(loc='best')
#plt.show()
fig.savefig('graphics/plot_grundlagen_characteristic_calc_0.png')
plt.close()


################################################################################
# fig:plot_grundlagen_characteristic_calc_1
################################################################################
phi_0 = 20.0
phi = linspace(-90.0, 90.0, 1000)
lamb = 343.0/40000
d_ = [lamb/4.0, lamb/2, lamb, lamb*2]
N = 8
aperture = APERTURE_RECT

fig = plt.figure(figsize=(10,5))

for d in d_:
	H = calc_characteristics(d, lamb, phi_0/180.0*pi, phi/180.0*pi, N, aperture)
	plt.plot(phi, H, marker='')

plt.xlabel('Abstrahlwinkel [Grad]')
plt.ylabel('Richtcharakteristik H(phi)')
plt.xlim(min(phi), max(phi))
plt.ylim(0, 1)
plt.minorticks_on()
plt.grid()
plt.legend(['$\lambda/4$','$\lambda/2$','$\lambda$','$\lambda*2$'])
#plt.legend(loc='best')
#plt.show()
fig.savefig('graphics/plot_grundlagen_characteristic_calc_1.png')
plt.close()


################################################################################
# fig:plot_grundlagen_characteristic_calc_aperture
################################################################################
phi_0 = 20.0
phi = linspace(-90.0, 90.0, 1000)
lamb = 343.0/40000
d = 0.0054
N = 8
aperture_ = [APERTURE_RECT, APERTURE_COS, APERTURE_COS2, APERTURE_GAUSS]

fig = plt.figure(figsize=(10,5))

for aperture in aperture_:
	H = calc_characteristics(d, lamb, phi_0/180.0*pi, phi/180.0*pi, N, aperture)
	plt.plot(phi, H, marker='')

plt.xlabel('Abstrahlwinkel [Grad]')
plt.ylabel('Richtcharakteristik H(phi)')
plt.xlim(min(phi), max(phi))
plt.ylim(0, 1)
plt.minorticks_on()
plt.grid()
plt.legend(['$Rechteck$','$cos$','$cos^2$','$Gauss$'])
#plt.legend(loc='best')
#plt.show()
fig.savefig('graphics/plot_grundlagen_characteristic_calc_aperture.png')
plt.close()


################################################################################
# fig:plot_grundlagen_characteristic_calc_aperture_log
################################################################################
phi_0 = 20.0
phi = linspace(-90.0, 90.0, 1000)
lamb = 343.0/40000
d = 0.0054
N = 8
aperture_ = [APERTURE_RECT, APERTURE_COS, APERTURE_COS2, APERTURE_GAUSS]

fig = plt.figure(figsize=(10,5))

for aperture in aperture_:
	H = calc_characteristics(d, lamb, phi_0/180.0*pi, phi/180.0*pi, N, aperture)
	plt.plot(phi, 20*log10(H), marker='')

plt.xlabel('Abstrahlwinkel [Grad]')
plt.ylabel('Richtcharakteristik H(phi) [dB]')
plt.xlim(min(phi), max(phi))
plt.ylim(-40, 0)
plt.minorticks_on()
plt.grid()
plt.legend(['$Rechteck$','$cos$','$cos^2$','$Gauss$'])
#plt.legend(loc='best')
#plt.show()
fig.savefig('graphics/plot_grundlagen_characteristic_calc_aperture_log.png')
plt.close()


################################################################################
# fig:plot_hardware_characteristic_calc
################################################################################
phi_0_ = [0.0, 10.0, 20.0, 30.0]
phi = linspace(-90.0, 90.0, 1000)
lamb = 343.0/40000
d = 0.0054
N = 8
aperture = APERTURE_RECT

fig = plt.figure(figsize=(10,5))

for phi_0 in phi_0_:
	H = calc_characteristics(d, lamb, phi_0/180.0*pi, phi/180.0*pi, N, aperture)
	plt.plot(phi, H, marker='', label='$\phi_0 = $'+str(int(phi_0)))

plt.xlabel('Abstrahlwinkel [Grad]')
plt.ylabel('Richtcharakteristik abs(H(phi))')
plt.xlim(min(phi), max(phi))
plt.ylim(0, 1)
plt.minorticks_on()
plt.grid()
plt.legend(loc='best')
#plt.show()
fig.savefig('graphics/plot_hardware_characteristic_calc.png')
plt.close()


################################################################################
# fig:plot_test_characteristic_rect_[0-30]
################################################################################
arr = readarray('measurements/09_characteristic_module.txt').transpose()

# calculated characteristic parameters
phi = linspace(-45.0, 45.0, 1000)

fig = plt.figure(figsize=(8,8))

for k in range(-5, 6):
	x1, y1 = pol2cart(1.0, 10.0*k/180*pi)
	plt.plot([0, x1], [0, y1], marker='', color='lightgray')
phi_meas = arr[0]
H_meas = arr[1] / max(arr[1])

x, y = pol2cart(H_meas, phi_meas/180.0*pi)
plt.plot(x, y, marker='.', color='blue', label='Messwerte')

plt.axis('equal')
plt.xlim(0.0, 1.0)
plt.ylim(-0.5, 0.5)

#plt.xlabel('Abstrahlwinkel [Grad]')
#plt.ylabel('Richtcharakteristik abs(H(phi))')
plt.minorticks_on()
#plt.grid()
plt.legend(loc='lower right')

#plt.show()
fig.savefig('graphics/plot_test_characteristic_module.png')
plt.close()

aperture_module = H_meas


################################################################################
# fig:plot_test_characteristic_rect_[0-30]
################################################################################
arr = readarray('measurements/09_characteristic_rect.txt').transpose()
NUM_MEASUREMENTS = 4

# calculated characteristic parameters
phi_0 = [0.0, 10.0, 20.0, 30.0]
phi = linspace(-45.0, 45.0, 910)
lamb = 343.0/40000
d = 0.0054
N = 8
aperture = APERTURE_RECT

for i in range(NUM_MEASUREMENTS):
	fig = plt.figure(figsize=(5, 5))

	for k in range(-5, 6):
		x1, y1 = pol2cart(1.0, 10.0*k/180*pi)
		plt.plot([0, x1], [0, y1], marker='', color='lightgray')

	phi_meas = arr[0]
	H_meas = arr[i+1] / max(arr[1])

	H = calc_characteristics(d, lamb, phi_0[i]/180.0*pi, phi/180.0*pi, N, aperture)
	x, y = pol2cart(H, phi/180.0*pi)
	plt.plot(x, y, marker='', color='green', label='Berechnete Werte')

	x, y = pol2cart(H_meas, phi_meas/180.0*pi)
	plt.plot(x, y, marker='.', color='blue', label='Messwerte')

	plt.axis('equal')
	plt.xlim(0.0, 1.0)
	plt.ylim(-0.5, 0.5)

	plt.minorticks_on()
	plt.legend(loc='lower right')

	#plt.show()
	fig.savefig('graphics/plot_test_characteristic_rect_' + str(abs(int(phi_0[i]))) + '_polar.png')
	plt.close()

	########################################################################
	fig = plt.figure(figsize=(10,5))

	phi_meas = arr[0]
	H_meas = arr[i+1] / max(arr[1])

	H = calc_characteristics(d, lamb, phi_0[i]/180.0*pi, phi/180.0*pi, N, aperture)
	plt.plot(phi, H, marker='', color='green', label='Berechnete Werte')

	plt.plot(phi_meas, H_meas, marker='.', color='blue', label='Messwerte')

	plt.xlim(min(phi), max(phi))
	plt.ylim(0.0, 1.0)

	plt.xlabel('Abstrahlwinkel [Grad]')
	plt.ylabel('Richtcharakteristik abs(H(phi))')
	plt.minorticks_on()
	plt.grid()
	plt.legend(loc='upper left')

	#plt.show()
	fig.savefig('graphics/plot_test_characteristic_rect_' + str(abs(int(phi_0[i]))) + '_cartesian.png')
	plt.close()

	########################################################################
	fig = plt.figure(figsize=(10,5))

	phi_meas = arr[0]
	H_meas = arr[i+1] / max(arr[1])

	H = calc_characteristics(d, lamb, phi_0[i]/180.0*pi, phi/180.0*pi, N, aperture)
	plt.plot(phi, 20*log10(H), marker='', color='green', label='Berechnete Werte')

	plt.plot(phi_meas, 20*log10(H_meas), marker='.', color='blue', label='Messwerte')

	plt.xlim(min(phi), max(phi))
	plt.ylim(-30, 0.0)

	plt.xlabel('Abstrahlwinkel [Grad]')
	plt.ylabel('Richtcharakteristik abs(H(phi)) [dB]')
	plt.minorticks_on()
	plt.grid()
	plt.legend(loc='upper left')

	#plt.show()
	fig.savefig('graphics/plot_test_characteristic_rect_' + str(abs(int(phi_0[i]))) + '_cartesian_log.png')
	plt.close()


################################################################################
# fig:plot_test_characteristic_aperture_[0-30]
################################################################################
arr = readarray('measurements/09_characteristic_aperture.txt').transpose()
NUM_MEASUREMENTS = 4

# calculated characteristic parameters
phi_0 = 10.0
phi = linspace(-45.0, 45.0, 1000)
lamb = 343.0/40000
d = 0.0054
N = 8
aperture = [APERTURE_RECT, APERTURE_COS, APERTURE_COS2, APERTURE_GAUSS]

for i in range(NUM_MEASUREMENTS):
	fig = plt.figure(figsize=(5,5))

	for k in range(-5, 6):
		x1, y1 = pol2cart(1.0, 10.0*k/180*pi)
		plt.plot([0, x1], [0, y1], marker='', color='lightgray')

	phi_meas = arr[0]
	H_meas = arr[i+1] / max(arr[i+1])

	H = calc_characteristics(d, lamb, phi_0/180.0*pi, phi/180.0*pi, N, aperture[i])
	x, y = pol2cart(H, phi/180.0*pi)
	plt.plot(x, y, marker='', color='green', label='Berechnete Werte')

	x, y = pol2cart(H_meas, phi_meas/180.0*pi)
	plt.plot(x, y, marker='.', color='blue', label='Messwerte')

	plt.axis('equal')
	plt.xlim(0.0, 1.0)
	plt.ylim(-0.5, 0.5)

	plt.minorticks_on()
	plt.legend(loc='lower right')

	#plt.show()
	fig.savefig('graphics/plot_test_characteristic_aperture_' + str(abs(int(aperture[i]))) + '_polar.png')
	plt.close()

	########################################################################
	fig = plt.figure(figsize=(10,5))

	phi_meas = arr[0]
	H_meas = arr[i+1] / max(arr[i+1])

	H = calc_characteristics(d, lamb, phi_0/180.0*pi, phi/180.0*pi, N, aperture[i])
	plt.plot(phi, H, marker='', color='green', label='Berechnete Werte')

	plt.plot(phi_meas, H_meas, marker='.', color='blue', label='Messwerte')

	plt.xlim(min(phi), max(phi))
	plt.ylim(0.0, 1.0)

	plt.xlabel('Abstrahlwinkel [Grad]')
	plt.ylabel('Richtcharakteristik abs(H(phi))')
	plt.minorticks_on()
	plt.grid()
	plt.legend(loc='upper left')

	#plt.show()
	fig.savefig('graphics/plot_test_characteristic_aperture_' + str(abs(int(aperture[i]))) + '_cartesian.png')
	plt.close()

	########################################################################
	fig = plt.figure(figsize=(10,5))

	phi_meas = arr[0]
	H_meas = arr[i+1] / max(arr[i+1])

	H = calc_characteristics(d, lamb, phi_0/180.0*pi, phi/180.0*pi, N, aperture[i])
	plt.plot(phi, 20*log10(H), marker='', color='green', label='Berechnete Werte')

	plt.plot(phi_meas, 20*log10(H_meas), marker='.', color='blue', label='Messwerte')

	plt.xlim(min(phi), max(phi))
	plt.ylim(-40, 0.0)

	plt.xlabel('Abstrahlwinkel [Grad]')
	plt.ylabel('Richtcharakteristik abs(H(phi)) [dB]')
	plt.minorticks_on()
	plt.grid()
	plt.legend(loc='upper left')

	#plt.show()
	fig.savefig('graphics/plot_test_characteristic_aperture_' + str(abs(int(aperture[i]))) + '_cartesian_log.png')
	plt.close()

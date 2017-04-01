from numpy import *
from scipy.optimize import curve_fit
import matplotlib.pyplot as plot

# read a .txt file to an array
def readarray(filename):
    array = loadtxt(filename, dtype="float", comments='#', delimiter="\t", converters=None, skiprows=0, usecols=None, unpack=False, ndmin=0)
    return array


table = readarray("kohlrausch_table.txt").transpose()
x = table[0]
y = []

y.append(table[1] / 1e3 / 2.303e-4 / 1e5)
y.append(table[2] / 1e3 / 2.303e-4 / 1e5)
y.append(table[3] / 1e3 / 2.303e-4 / 1e5)


def func(x, a, b):
    return a * x**b


plot.figure()
s_label = ['10% Luftfeuchtigkeit','60% Luftfeuchtigkeit','90% Luftfeuchtigkeit']
s_label_fit = ['','','']
s_color = ['blue','red','green']

for i in range(len(y)):
	popt, pcov = curve_fit(func, x, y[i])

	print(s_label[i] + ': ' + str(popt))
	#print(sqrt(diag(pcov)))

	x_fit = linspace(min(x), max(x), 5000)
	y_fit = func(x_fit, popt[0], popt[1])

	plot.semilogx(x, y[i], 'o', label=s_label[i], color=s_color[i])
	plot.semilogx(x_fit, y_fit, '-', label=s_label_fit[i], color=s_color[i])

#y_fit = func(x_fit, popt[0], popt[1], popt[2], popt[3])

#plot.xlim(min(x), max(x))
plot.xlabel('f [MHz]')
plot.ylabel('')
plot.legend(loc='upper left', numpoints=1)
plot.grid(True, which='both', ls='-')
plot.show()
#fig.savefig('allgemein.png')

#print(popt)
#print(sqrt(diag(pcov)))
#print(abs(sqrt(diag(pcov))/popt))

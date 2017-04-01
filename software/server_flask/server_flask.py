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

import gevent
from gevent.wsgi import WSGIServer
from flask import Flask, Response, render_template, request
from wtforms import Form, TextField, SelectField
import json

from firmware_controller import FirmwareController


################################################################################
# class definitions
################################################################################
class InputValues():
	# user interface standard input values
	angle_min = -30		# minimal angle [deg]
	angle_max = 30		# maximal angle [deg]
	aperture = 0		# aperture index
	num_bursts = 10		# number of ultrasonic bursts
	rec_buf_cnt = 4		# number of receiver buffers
	numb_of_maximums = 0	# upper limit maximums to be tracked
	sonic_speed = 343.0	# sonic speed [m/s]

	choices_angle = []
	for i in range(angle_min, angle_max+1):
		choices_angle.append((str(i),str(i)))

	choices_aperture = [('0','rect'),('1','cos'),('2','cos2'),('3','gauss')]

	choices_num_bursts = []
	for i in range(0, num_bursts*4+1):
		choices_num_bursts.append((str(i*2),str(i*2)))

	choices_rec_buf_cnt = [('1','1'),('2','2'),('4','4'),('8','8')]

	choices_numb_of_maximums = []
	for i in range(0, 16):
		choices_numb_of_maximums.append((str(i),str(i)))


class InputForm(Form, InputValues):
	# user interface input fields
	sonic_speed = TextField(default=InputValues.sonic_speed)
	angle_min = SelectField(choices=InputValues.choices_angle, default=InputValues.angle_min)
	angle_max = SelectField(choices=InputValues.choices_angle, default=InputValues.angle_max)
	aperture = SelectField(choices=InputValues.choices_aperture, default=InputValues.aperture)
	num_bursts = SelectField(choices=InputValues.choices_num_bursts, default=InputValues.num_bursts)
	rec_buf_cnt = SelectField(choices=InputValues.choices_rec_buf_cnt, default=InputValues.rec_buf_cnt)
	numb_of_maximums = SelectField(choices=InputValues.choices_numb_of_maximums, default=InputValues.numb_of_maximums)


# interface class between javascript client and flask server
class ServerSentEvent(object):
	def __init__(self, data):
		self.data = data
		self.event = None
		self.id = None

		self.desc_map = {
			self.data: "data",
			self.event: "event",
			self.id: "id",
		}

	def encode(self):
		if not self.data:
			return ""
		lines = ["%s: %s" % (v, k) for k, v in self.desc_map.items() if k]
		return "%s\n\n" % "\n".join(lines)


# interface class between firmware_controller and flask server
class Interface():
	def __init__(self):
		self.angle = 0
		self.angle_min = 0
		self.angle_max = 0
		self.rec_buf_cnt = 0
		self.y_env_downsamp = []
		self.upsample_factor = 0
		self.downsample_factor = 0
		self.lock = 1
		return

	def callback_func(self):
		print("ServerFlask: callback function")
		self.angle, self.angle_min, self.angle_max, self.y_env_downsamp, self.rec_buf_cnt, self.upsample_factor, self.downsample_factor = firmware_controller.get_data()
		self.lock = 0
		time.sleep(0.05)
		return

	def get_data(self):
		return json.dumps({"angle" : self.angle, "angle_min" : self.angle_min, "angle_max" : self.angle_max, "y_env_downsamp" : self.y_env_downsamp, "rec_buf_cnt" : self.rec_buf_cnt, "upsample_factor" : self.upsample_factor, "downsample_factor" : self.downsample_factor})


################################################################################
# global definitions
################################################################################
app = Flask(__name__)
firmware_controller = FirmwareController()
interface = Interface()


################################################################################
# render templates
################################################################################
@app.route('/', methods=['GET', 'POST'])
def index():
	input_values = InputValues()
	input_form = InputForm(request.form, input_values)

	if request.method == 'POST' and input_form.validate():
		try:
			input_values.sonic_speed = float(input_form.sonic_speed.data)
		except:
			pass
		input_values.angle_min = int(input_form.angle_min.data)
		input_values.angle_max = int(input_form.angle_max.data)
		input_values.aperture = int(input_form.aperture.data)
		input_values.num_bursts = int(input_form.num_bursts.data)
		input_values.rec_buf_cnt = int(input_form.rec_buf_cnt.data)

	firmware_controller.update_values(input_values.angle_min, input_values.angle_max, input_values.aperture, input_values.num_bursts, input_values.rec_buf_cnt, input_values.sonic_speed)

	return render_template("index.html", input_form=input_form)


@app.route('/about/', methods=['GET', 'POST'])
def about():
	return render_template("about.html")


@app.route('/subscribe/', methods=['GET', 'POST'])
def subscribe():
	print("ServerFlask: subscribe")
	def gen():
		try:
			while True:
				print("ServerFlask: event")
				while interface.lock == 1:
					time.sleep(0.1)
				interface.lock = 1
				event = ServerSentEvent(interface.get_data())
				yield event.encode()
		except GeneratorExit:
			print("ServerFlask: GeneratorExit")

	return Response(gen(), mimetype="text/event-stream")


################################################################################
# main
################################################################################
if __name__ == '__main__':
	firmware_controller.register_callback(interface.callback_func)
	firmware_controller.update_values(InputValues.angle_min, InputValues.angle_max, InputValues.aperture, InputValues.num_bursts, InputValues.rec_buf_cnt, InputValues.sonic_speed)
	firmware_controller.start()

	app.debug = False
	server = WSGIServer(("", 5000), app)
	server.serve_forever()

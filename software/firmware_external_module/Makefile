################################################################################
#
#  Copyright (C) 2016, 2017 Mijnssen Raphael, Pluess Jonas
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

NAME		= external_module
SRCS		= main.c
CFLAGS		= -D F_CPU=16000000UL -lm

PROCESSOR	= atmega328p
#PROCESSOR	= atmega2560
DEVICE		= /dev/ttyUSB0
#DEVICE		= /dev/ttyACM0
BAUDRATE	= 57600
#BAUDRATE	= 115200

build: ${SRCS}
	avr-gcc -g -O2 -mmcu=${PROCESSOR} ${CFLAGS} -c ${SRCS}
	avr-gcc -g -mmcu=${PROCESSOR} -o ${NAME}.elf ${SRCS:.c=.o -lm}
	avr-objcopy -j.text -j.data -O ihex ${NAME}.elf ${NAME}.hex

wall: ${SRCS}
	avr-gcc -g -O2 -mmcu=${PROCESSOR} ${CFLAGS} -Wall -c ${SRCS}
	avr-gcc -g -mmcu=${PROCESSOR} -o ${NAME}.elf ${SRCS:.c=.o -lm}
	avr-objcopy -j.text -j.data -O ihex ${NAME}.elf ${NAME}.hex

.PHONY: clean
clean:
	rm -rf *.hex *.o *.elf ${NAME}.tar.gz -rf ./doc

.PHONY: install_arduino
install_arduino: build
	avrdude -F -c arduino -p ${PROCESSOR} -P ${DEVICE} -b ${BAUDRATE} -U flash:w:${NAME}.hex:i

.PHONY: install_wiring
install_wiring: build
	avrdude -F -c wiring -p ${PROCESSOR} -P ${DEVICE} -b ${BAUDRATE} -U flash:w:${NAME}.hex:i

.PHONY: install_avrdragon
install_avrdragon: build
	avrdude -F -c dragon_isp -p ${PROCESSOR} -B 10 -U flash:w:${NAME}.hex:i

.PHONY: tar
tar:
	tar --exclude='*.o' --exclude='*~' --exclude=${NAME} --exclude=${NAME}.tar.gz -cvzf ${NAME}.tar.gz *

.PHONY: doc
doc:
	rm -rf ./doc/latex
	rm -rf ./doc/html
	doxygen ./Doxyfile

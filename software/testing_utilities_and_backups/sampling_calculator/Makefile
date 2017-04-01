################################################################################
#
#  Copyright (C) 2016, Pluess Jonas
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

PROG	= sampling_calculator
SRCS	= main.c
CC	= cc
CFLAGS	= -lm -std=c89

PREFIX	= /usr/local
BINDIR	= ${PREFIX}/bin

build: ${SRCS}
	${CC} ${CFLAGS} ${SRCS} -o ${PROG}

wall: ${SRCS}
	${CC} ${CFLAGS} -Wall -pedantic ${SRCS} -o ${PROG}

clean:
	rm -rf *.o *~ core ${PROG}.tar.gz ${PROG}

install: build
	cp ${PROG} ${BINDIR}

uninstall:
	rm -rf ${BINDIR}/${PROG}

remove: uninstall


tar:
	tar --exclude='*.o' --exclude='*~' --exclude='core' --exclude=${PROG} --exclude=${PROG}.tar.gz -cvzf ${PROG}.tar.gz *


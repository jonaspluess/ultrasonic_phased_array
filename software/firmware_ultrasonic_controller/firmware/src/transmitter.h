/*******************************************************************************
 *
 * Copyright (C) 2016, 2017, Mijnssen Raphael, Pluess Jonas
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 ******************************************************************************/


#ifndef TRANSMITTER_H
#define TRANSMITTER_H


#include <asf.h>
#include <stdint.h>


/******************************************************************************/
#define TRANSMITTER_INIT_PIO(pio, reg) \
	(pio)->PIO_PER = (reg); \
	(pio)->PIO_OER = (reg); \
	(pio)->PIO_PUDR = (reg)

#define TRANSMITTER_SET_PIO(pio, reg) \
	(pio)->PIO_SODR = (reg)

#define TRANSMITTER_CLR_PIO(pio, reg) \
	(pio)->PIO_CODR = (reg)
/******************************************************************************/


#define NOP_1		__asm("nop")

#define NOP_2		__asm("nop"); \
			__asm("nop")

#define NOP_3		__asm("nop"); \
			__asm("nop"); \
			__asm("nop")

#define NOP_5		__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop")

#define NOP_7		__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop")

#define NOP_16		__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop")

#define NOP_17		__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop")

#define NOP_18		__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop")

#define NOP_19		__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop")

#define NOP_20		__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop")

#define NOP_21		__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop")

#define NOP_22		__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop")

#define NOP_23		__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop"); \
			__asm("nop")


// transmitter control ports
#define TRM_PIO_PWM PIOB
#define TRM_PIO_GND PIOD
#define TRM_PIO_REC PIOD
#define TRM_PIO_IR PIOC

// transmitter control pins
#define TRM_PIN_PWM PIO_PB27
#define TRM_PIN_GND PIO_PD8
#define TRM_PIN_REC PIO_PD7
#define TRM_PIN_IR PIO_PC12


#define APERTURE_RECT 0
#define APERTURE_COS 1
#define APERTURE_COS_2 2
#define APERTURE_GAUSS 3

#define STDEV_GAUSS 0.5


extern void transmitter_init(unsigned long frequency, unsigned long num_bursts, int aperture);
extern int transmitter_send(int angle);
extern int transmitter_send_ir(void);


#endif /* TRANSMITTER_H */

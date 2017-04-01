/*******************************************************************************
 *
 * Copyright (C) 2016, 2017 Mijnssen Raphael, Pluess Jonas
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

#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>


/******************************************************************************/
#ifndef set_bit
	#define set_bit(reg, bit)	reg |= (1<<bit)
#endif
#ifndef clr_bit
	#define clr_bit(reg, bit)	reg &= ~(1<<bit)
#endif
#ifndef read_bit
	#define read_bit(reg, bit)	((reg & (1<<bit))>>bit)
#endif

#define OUTPUT_DDR DDRB
#define OUTPUT_PORT PORTB
#define OUTPUT_PIN PORTB1

#define NUM_BURSTS 10


/******************************************************************************/
static void ext_int_init(void);
static void timer_init(void);
static void timer_sleep(void);


/******************************************************************************/
int16_t main(void)
{
	set_bit(OUTPUT_DDR, OUTPUT_PIN);
	ext_int_init();
	timer_init();
	sei();

	while(1){
		/* do nothing */
	}

	return 0; // makes the compiler happy
}



/******************************************************************************/
static void ext_int_init(void)
{
	// enable external interrupts (rising edge trigger)
	EICRA = (1<<ISC01)|(1<<ISC00);
	EIMSK = (1<<INT0);
}

// initialize timer 1
static void timer_init(void)
{
	TCCR1B = (1<<WGM13)|(1<<WGM12)|(1<<CS10); // set timer prescaler / mode
	TIMSK1 = (1<<ICIE1); // allow timer overflow interrupt
	ICR1 = 296; // set timer MAX value
}

// set the timer 1 to sleep mode
static void timer_sleep(void)
{
	TCCR1B = 0;
	TIMSK1 = 0;
	TCNT1 = 0;
}


/******************************************************************************/
ISR(TIMER1_CAPT_vect)
{
	static uint16_t t = 0;

	if(t < NUM_BURSTS * 2){
		OUTPUT_PORT = (~OUTPUT_PORT) & OUTPUT_PIN;
	}else{
		clr_bit(OUTPUT_PORT, OUTPUT_PIN);
		timer_sleep();
		t = 0;
	}
}

ISR(INT0_vect)
{
	timer_init();
}

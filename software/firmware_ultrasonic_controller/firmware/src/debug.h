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


#ifndef DEBUG_H
#define DEBUG_H


#include <asf.h>


// comment or uncomment this definition to enable or disable debugging
//#define DEBUGGING


#ifdef DEBUGGING

	#define DEBUG_INIT_PIO(pio, reg) \
		(pio)->PIO_PER = (reg); \
		(pio)->PIO_OER = (reg); \
		(pio)->PIO_PUDR = (reg)

	#define DEBUG_SET_PIO(pio, reg) \
		(pio)->PIO_SODR = (reg)

	#define DEBUG_CLR_PIO(pio, reg) \
		(pio)->PIO_CODR = (reg)

#else /* DEBUGGING */

	// does nothing, because of debugging is disabled
	#define DEBUG_INIT_PIO(pio, reg)
	#define DEBUG_SET_PIO(pio, reg)
	#define DEBUG_CLR_PIO(pio, reg)

#endif /* DEBUGGING */


#endif /* DEBUG_H */

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


#ifndef RECEIVER_H
#define RECEIVER_H


#include <asf.h>
#include <stdint.h>


#define REC_NUM_BANKS 2
#define REC_NUM_CHANNELS 8
#define REC_BUF_LEN 512


extern void receiver_init(unsigned long sample_freq);
extern void receiver_start(int bank_cnt);
extern volatile int16_t (*receiver_get_buffer(int *flag))[REC_BUF_LEN];


#endif /* RECEIVER_H */

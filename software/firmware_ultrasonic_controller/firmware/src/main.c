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


#include <asf.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

#include "debug.h"
#include "receiver.h"
#include "transmitter.h"
#include "uart.h"


/******************************************************************************/
#define SET_IRQ_PRIORITY(irq, prio) \
	NVIC_DisableIRQ((IRQn_Type) (irq)); \
	NVIC_ClearPendingIRQ((IRQn_Type) (irq)); \
	NVIC_SetPriority((IRQn_Type) (irq), (prio)); \
	NVIC_EnableIRQ((IRQn_Type) (irq))


/******************************************************************************/
#define TRM_FREQUENCY 40000UL	/* ultrasonic transmitter frequency */
#define REC_SAMP_FREQ 32000UL	/* receiver sample frequency */

#define COM_BUF_LEN 1024	/* length of the communication buffers */

// communication buffers
static char com_tx_buffer[COM_BUF_LEN];
static char com_rx_buffer[COM_BUF_LEN];

// possible states of the statemachine
typedef enum {
	RX_ULTRASONIC,
	TX_ULTRASONIC,
	RX_USB,
	TX_USB
} state_t;

// settings struct
typedef struct {
	int8_t angle;		/* transmitter sending angle */
	int8_t trm_aperture;	/* transmitter sending aperture */
	int8_t trm_num_bursts;	/* number of bursts to stimulate the piezo */
	int8_t rec_buf_cnt_max;	/* max number of received buffers */
} s_settings;


/******************************************************************************/
extern int _read(int file, char *ptr, int len);
extern int _write(int file, char *ptr, int len);

static void init_com_buffers(void);
static void state_machine(void);


/******************************************************************************/
int main(void)
{
	// init irq vectors (asf/common/utils/interrupt/interrupt_sam_nvic.h)
	irq_initialize_vectors();

	// set the irq priority (0 is the highest, 9 is the lowest priority)
	// example code: asf/sam/utils/cmsis/cm4_nvic_example/main.c
	SET_IRQ_PRIORITY(ADC_IRQn, 0);		/* used in receiver.c */
	SET_IRQ_PRIORITY(PWM_IRQn, 1);		/* used in transmitter.c */
	SET_IRQ_PRIORITY(TC3_IRQn, 1);		/* used in transmitter.c */
	SET_IRQ_PRIORITY(UOTGHS_IRQn, 2);
	SET_IRQ_PRIORITY(UART_IRQn, 3);

	// enable irq (asf/common/utils/interrupt/interrupt_sam/nvic.h)
	cpu_irq_enable();

	// init the system clock (asf/common/services/clock/sam3x/sysclk.c)
	sysclk_init();

	// init the arduino due board (asf/sam/boards/arduino_due_x/init.c)
	board_init();

	// start usb device controller (asf/common/services/usb/udc/udc.c)
	udc_start();

	// init transmitter, apply trm_aperture (pwm duty cycle)
	transmitter_init(TRM_FREQUENCY, 0, APERTURE_RECT);

	// init receiver
	receiver_init(REC_SAMP_FREQ);

	// init the communication buffers
	init_com_buffers();

	// init debug pins
	DEBUG_INIT_PIO(PIOC, PIO_PC18);
	DEBUG_INIT_PIO(PIOC, PIO_PC19);
	DEBUG_INIT_PIO(PIOC, PIO_PC14);

	while(1){
		state_machine();
	}

	// never reached
	return EXIT_SUCCESS;
}

static void state_machine(void)
{
	static state_t current_state = RX_USB;
	static state_t next_state = RX_USB;

	static int rec_buf_cnt = 0;		/* receiver buffer count */
	static int rec_buf_cnt_max = 0;		/* max num of received buffers */
	static int rec_finish_flag = 0;		/* receiver finished flag */
	static int angle = 0;			/* transmit/receive angle */
	static int trm_aperture = APERTURE_RECT;/* transmit aperture */
	static int trm_num_bursts = 0;		/* transmit number of bursts */
	static volatile int16_t (*rec_buf_ptr)[REC_BUF_LEN] = NULL;

	int i;
	int err = 0;
	s_settings *settings;
	int temp_trm_aperture;
	int temp_trm_num_bursts;

	switch(current_state){
		// receive data via usb
		case RX_USB:
			// read configuration buffer
			do{
				int avail;

				// discard buffers with length < 512
				do{
					avail = udi_cdc_get_nb_received_data();

					if((avail % 512) != 0){
						udi_cdc_read_buf(com_rx_buffer, avail % 512);
					}
				}while(avail >= REC_BUF_LEN);

				err = udi_cdc_read_buf(com_rx_buffer, COM_BUF_LEN * sizeof(com_rx_buffer[0]));
			}while(err != 0);

			// read settings from configuration buffer
			settings = (s_settings *)(com_rx_buffer);

			angle = settings->angle;
			temp_trm_aperture = settings->trm_aperture;
			temp_trm_num_bursts = settings->trm_num_bursts;
			rec_buf_cnt_max = settings->rec_buf_cnt_max;

			if(temp_trm_aperture != trm_aperture || temp_trm_num_bursts != trm_num_bursts){
				trm_aperture = temp_trm_aperture;
				trm_num_bursts = temp_trm_num_bursts;

				// init transmitter, apply trm_aperture (pwm duty cycle)
				transmitter_init(TRM_FREQUENCY, trm_num_bursts, trm_aperture);
			}

			next_state = TX_ULTRASONIC;
			break;

		// transmit ultrasonic signal
		case TX_ULTRASONIC:
			if(trm_num_bursts != 0){
				// send ultrasonic signal
				// no risk for race condition
				if(transmitter_send(angle) != 0){
					next_state = TX_ULTRASONIC;
				}else{
					next_state = RX_ULTRASONIC;
					receiver_start(rec_buf_cnt_max);
				}
			}else{
				// send infrared signal
				// no risk for race condition
				if(transmitter_send_ir() != 0){
					next_state = TX_ULTRASONIC;
				}else{
					next_state = RX_ULTRASONIC;
					receiver_start(rec_buf_cnt_max);
				}
			}

			break;

		// receive ultrasonic signal
		case RX_ULTRASONIC:
			// no risk for race condition
			rec_buf_ptr = receiver_get_buffer(&rec_finish_flag);
			if(rec_buf_ptr != NULL){
				next_state = TX_USB;
			}else{
				next_state = RX_ULTRASONIC;
			}
			break;

		// transmit data via usb
		case TX_USB:
			for(i = 0; i < REC_NUM_CHANNELS; i++){
				do{
					err = udi_cdc_write_buf((const void *)rec_buf_ptr[i], REC_BUF_LEN * sizeof(int16_t));
				}while(err != 0);
			}

			printf("counter:%i,flag:%i,angle:%i,trm_aperture:%i,trm_num_bursts:%i,rec_buf_cnt_max:%i\n", rec_buf_cnt, rec_finish_flag, angle, trm_aperture, trm_num_bursts, rec_buf_cnt_max);

			do{
				err = udi_cdc_write_buf(com_tx_buffer, COM_BUF_LEN * sizeof(com_tx_buffer[0]));
			}while(err != 0);

			rec_buf_cnt++;

			if(rec_finish_flag != 0){
				rec_finish_flag = 0;
				rec_buf_cnt = 0;
				next_state = RX_USB;
			}else{
				next_state = RX_ULTRASONIC;
			}
			break;

		// should never reach this
		default:
			next_state = RX_USB;
			break;
	}

	current_state = next_state;
}


/*******************************************************************************
 * Communication functions (used for printf)
 ******************************************************************************/
// used for printf
extern int _read(int file, char *ptr, int len)
{
    return 0;
}

// used for printf
extern int _write(int file, char *ptr, int len)
{
	int i, k;

	for(i = 0; i < len && i < COM_BUF_LEN/2; i++)
	{
		com_tx_buffer[i] = ptr[i];
	}

	for(k = i; k < COM_BUF_LEN/2; k++){
		com_tx_buffer[k] = '\0';
	}

	return i;
}

// init the communication transmission buffer
static void init_com_buffers(void)
{
	int i;

	for(i = COM_BUF_LEN / 2; i < COM_BUF_LEN; i++){
		com_tx_buffer[i] = -1;
	}

	for(i = 0; i < COM_BUF_LEN; i++){
		com_rx_buffer[i] = '\0';
	}
}

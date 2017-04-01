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


#include <string.h>
#include <math.h>

#include "transmitter.h"
#include "debug.h"


/******************************************************************************/
#define PWM_NUM_CHANNELS 8
#define PWM_PERIOD 525
#define PWM_DUTY_MAX_P 262	// maximal power duty cycle value
#define PWM_DUTY_ZERO_P 525	// zero power duty cycle value

#define PROGRESS 1
#define FINISHED 0

// Timer counter definitions (don't ask...)
#define TC TC1			// Timer 1
#define CH 0			// Channel 0
#define TC_Handler TC3_Handler	// TC1, CH0 uses TC3_Handler
#define TC_IRQn TC3_IRQn	// TC1, CH0 uses TC3_IRQn
#define ID_TC ID_TC3		// TC1, CH0 uses ID_TC3

// arduino due board: "pwm 9"
#define PWM0_CHAN PWM_CHANNEL_4
#define PWM0_FLAG (PIO_PERIPH_B | PIO_DEFAULT)
#define PWM0_GPIO PIO_PC21_IDX

// arduino due board: "pwm 8"
#define PWM1_CHAN PWM_CHANNEL_5
#define PWM1_FLAG (PIO_PERIPH_B | PIO_DEFAULT)
#define PWM1_GPIO PIO_PC22_IDX

// arduino due board: "pwm 7"
#define PWM2_CHAN PWM_CHANNEL_6
#define PWM2_FLAG (PIO_PERIPH_B | PIO_DEFAULT)
#define PWM2_GPIO PIO_PC23_IDX

// arduino due board: "pwm 6"
#define PWM3_CHAN PWM_CHANNEL_7
#define PWM3_FLAG (PIO_PERIPH_B | PIO_DEFAULT)
#define PWM3_GPIO PIO_PC24_IDX

// arduino due board: "digital 40"
#define PWM4_CHAN PWM_CHANNEL_3
#define PWM4_FLAG (PIO_PERIPH_B | PIO_DEFAULT)
#define PWM4_GPIO PIO_PC8_IDX

// arduino due board: "digital 38"
#define PWM5_CHAN PWM_CHANNEL_2
#define PWM5_FLAG (PIO_PERIPH_B | PIO_DEFAULT)
#define PWM5_GPIO PIO_PC6_IDX

// arduino due board: "digital 36"
#define PWM6_CHAN PWM_CHANNEL_1
#define PWM6_FLAG (PIO_PERIPH_B | PIO_DEFAULT)
#define PWM6_GPIO PIO_PC4_IDX

// arduino due board: "digital 34"
#define PWM7_CHAN PWM_CHANNEL_0
#define PWM7_FLAG (PIO_PERIPH_B | PIO_DEFAULT)
#define PWM7_GPIO PIO_PC2_IDX

static pwm_channel_t pwm_ch[PWM_NUM_CHANNELS];
static uint32_t pwm_chan[PWM_NUM_CHANNELS] = {
	PWM0_CHAN,
	PWM1_CHAN,
	PWM2_CHAN,
	PWM3_CHAN,
	PWM4_CHAN,
	PWM5_CHAN,
	PWM6_CHAN,
	PWM7_CHAN
};
static int max_num_bursts = 0;

typedef enum {
	START,
	SEND,
	ATTENTUATE,
} state_t;

static state_t state_pwm = START;

// timer value
volatile static int t = 0;
volatile static int last_channel = 0;


/******************************************************************************/
static float get_aperture(int aperture, int n, int max_n);
static int get_pwm_value(float aperture_relative);
static void delay_a(int angle);
static void init_timer_counter(unsigned long freq);


/******************************************************************************/
extern void transmitter_init(unsigned long frequency, unsigned long num_bursts, int aperture)
{
	int i;

	// init pwm enable pin, gnd switch enable pin and receiver enable pin
	TRANSMITTER_INIT_PIO(TRM_PIO_PWM, TRM_PIN_PWM);	// pwm enable pin
	TRANSMITTER_INIT_PIO(TRM_PIO_GND, TRM_PIN_GND);	// ground switch enable pin
	TRANSMITTER_INIT_PIO(TRM_PIO_REC, TRM_PIN_REC);	// receiver enable pin
	TRANSMITTER_INIT_PIO(TRM_PIO_IR, TRM_PIN_IR);	// ir send pin

	// disable pwm transmitter (active low)
	TRANSMITTER_SET_PIO(TRM_PIO_PWM, TRM_PIN_PWM);
	// disable gnd switch (active low)
	TRANSMITTER_SET_PIO(TRM_PIO_GND, TRM_PIN_GND);
	// disable receiver (active high)
	TRANSMITTER_CLR_PIO(TRM_PIO_REC, TRM_PIN_REC);

	max_num_bursts = num_bursts;

	int32_t pwm_gpio[PWM_NUM_CHANNELS] = {
		PWM0_GPIO,
		PWM1_GPIO,
		PWM2_GPIO,
		PWM3_GPIO,
		PWM4_GPIO,
		PWM5_GPIO,
		PWM6_GPIO,
		PWM7_GPIO
	};

	int32_t pwm_flag[PWM_NUM_CHANNELS] = {
		PWM0_FLAG,
		PWM1_FLAG,
		PWM2_FLAG,
		PWM3_FLAG,
		PWM4_FLAG,
		PWM5_FLAG,
		PWM6_FLAG,
		PWM7_FLAG
	};

	pwm_clock_t pwm_clk = {
		.ul_clka = 2 * frequency * PWM_PERIOD,
		.ul_clkb = 0,
		.ul_mck = sysclk_get_cpu_hz()
	};

	// set pwm_channel_t structs to zero
	memset(&pwm_ch, 0, sizeof(pwm_ch));

	// config power management for pwm (asf/sam/drivers/pmc/pmc.c)
	pmc_enable_periph_clk(ID_PWM);

	for(i = 0; i < PWM_NUM_CHANNELS; i++){
		gpio_configure_pin(pwm_gpio[i], pwm_flag[i]);

		pwm_ch[i].alignment = PWM_ALIGN_CENTER;
		pwm_ch[i].polarity = PWM_HIGH;
		pwm_ch[i].ul_prescaler = PWM_CMR_CPRE_CLKA;
		pwm_ch[i].ul_period = PWM_PERIOD;
		pwm_ch[i].ul_duty = get_pwm_value(get_aperture(aperture, i+1, PWM_NUM_CHANNELS));
		pwm_ch[i].channel = pwm_chan[i];

		pwm_channel_disable(PWM, pwm_chan[i]);
	}

	pwm_init(PWM, &pwm_clk);

	for(i = 0; i < PWM_NUM_CHANNELS; i++){
		pwm_channel_init(PWM, &pwm_ch[i]);
		pwm_channel_enable_interrupt(PWM, pwm_chan[i], 0);
	}

	// configure timer counter for attentuation delay
	init_timer_counter(frequency);
}

// send ultrasonic signal
extern int transmitter_send(int angle)
{
	int i;

	switch(state_pwm){
		case START:
			// disable receiver (active high)
			TRANSMITTER_CLR_PIO(TRM_PIO_REC, TRM_PIN_REC);
			// enable pwm transmitter (active low)
			TRANSMITTER_CLR_PIO(TRM_PIO_PWM, TRM_PIN_PWM);

			cpu_irq_disable();
			if(angle < 0){
				angle *= -1;
				last_channel = 0;
				for(i = (PWM_NUM_CHANNELS - 1); i >= 0; i--){
					pwm_channel_enable(PWM, pwm_chan[i]);
					delay_a(angle);
				}
			}else{
				last_channel = PWM_NUM_CHANNELS-1;
				for(i = 0; i < PWM_NUM_CHANNELS; i++){
					pwm_channel_enable(PWM, pwm_chan[i]);
					delay_a(angle);
				}
			}

			state_pwm = SEND;
			cpu_irq_enable();
			return PROGRESS;

		case ATTENTUATE:
			if(t > 60 - max_num_bursts/2){
				// enable receiver (active high)
				TRANSMITTER_SET_PIO(TRM_PIO_REC, TRM_PIN_REC);
			}

			if(t > 70 - max_num_bursts/2){
				// disable gnd switch (active low)
				TRANSMITTER_SET_PIO(TRM_PIO_GND, TRM_PIN_GND);
			}

			// 80 clock ticks => 2ms
			if(t > 80 - max_num_bursts/2){
				tc_disable_interrupt(TC, CH, TC_IDR_CPCS);
				tc_stop(TC, CH);
				t = 0;
				state_pwm = START;
				return FINISHED;
			}

			return PROGRESS;

		default:
			return PROGRESS;
	}
}

// send infrared signal
extern int transmitter_send_ir(void)
{
	switch(state_pwm){
		case START:
			TRANSMITTER_SET_PIO(TRM_PIO_IR, TRM_PIN_IR);
			tc_enable_interrupt(TC, CH, TC_IER_CPAS);
			tc_start(TC, CH);
			state_pwm = SEND;
			return PROGRESS;

		case SEND:
			if(t > 40){
				TRANSMITTER_CLR_PIO(TRM_PIO_IR, TRM_PIN_IR);
				tc_disable_interrupt(TC, CH, TC_IDR_CPCS);
				tc_stop(TC, CH);
				t = 0;
				state_pwm = START;
				return FINISHED;
			}else{
				return PROGRESS;
			}
		default:
			return PROGRESS;
	}
}


/******************************************************************************/
// return the weighting-factor (0<w<1) of channel n, for the desired aperture
static float get_aperture(int aperture, int n, int max_n)
{
	float result_n;
	float result_max;

	int index_at_max = (max_n+1)/2;

	switch(aperture){
		case APERTURE_RECT:
			return 1.0;
			break;
		case APERTURE_COS:
			result_n = sin(M_PI*n/((max_n+2)-1));
			result_max = sin(M_PI*index_at_max/((max_n+2)-1));
			return result_n*1.0/result_max;
			break;
		case APERTURE_COS_2:
			result_n = powf(sin(M_PI*n/((max_n+2)-1)), 2.0);
			result_max = powf(sin(M_PI*index_at_max/((max_n+2)-1)), 2.0);
			return result_n*1.0/result_max;
			break;
		case APERTURE_GAUSS:
			result_n = exp(-0.5*powf(((n-1)-(max_n-1)/2.0)/(STDEV_GAUSS*(max_n-1)/2.0), 2.0));
			result_max = exp(-0.5*powf(((index_at_max-1)-(max_n-1)/2.0)/(STDEV_GAUSS*(max_n-1)/2.0), 2.0));
			return result_n*1.0/result_max;
			break;
		default:
			break;
	}

	return 0.0;
}

// return the pwm duty value (max for input 1 and minimum for input 0)
static int get_pwm_value(float aperture_relative)
{
	return (int)((float)(PWM_DUTY_ZERO_P) - roundf(powf(aperture_relative, 2.0) * (PWM_DUTY_ZERO_P - PWM_DUTY_MAX_P)));
}

static void delay_a(int angle)
{
	switch(angle){
		case 30:
			NOP_23;
		case 29:
			NOP_21;
		case 28:
			NOP_18;
		case 27:
			NOP_18;
		case 26:
			NOP_19;
		case 25:
			NOP_19;
		case 24:
			NOP_17;
		case 23:
			NOP_18;
		case 22:
			NOP_23;
		case 21:
			NOP_18;
		case 20:
			NOP_20;
		case 19:
			NOP_19;
		case 18:
			NOP_18;
		case 17:
			NOP_19;
		case 16:
			NOP_19;
		case 15:
			NOP_19;
		case 14:
			NOP_19;
		case 13:
			NOP_22;
		case 12:
			NOP_19;
		case 11:
			NOP_19;
		case 10:
			NOP_19;
		case 9:
			NOP_19;
		case 8:
			NOP_19;
		case 7:
			NOP_18;
		case 6:
			NOP_19;
		case 5:
			NOP_19;
		case 4:
			NOP_19;
		case 3:
			NOP_19;
		default:
		break;
	}
}

// init the timer counter for attentuation
static void init_timer_counter(unsigned long freq)
{
	uint32_t ul_div = 0;
	uint32_t ul_tc_clks = 0;

	// config power management for timer/counter (asf/sam/drivers/pmc/pmc.c)
	pmc_enable_periph_clk(ID_TC);

	// init timer/counter (asf/sam/drivers/tc/tc.c)
	tc_find_mck_divisor(freq, sysclk_get_cpu_hz(), &ul_div, &ul_tc_clks, sysclk_get_cpu_hz());
	tc_init(TC, CH, ul_tc_clks | TC_CMR_CPCTRG | TC_CMR_WAVE | TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_SET);

	TC->TC_CHANNEL[CH].TC_RA = (sysclk_get_cpu_hz() / ul_div / freq) / 2;
	TC->TC_CHANNEL[CH].TC_RC = (sysclk_get_cpu_hz() / ul_div / freq) / 1;
}


/******************************************************************************/
extern void PWM_Handler(void)
{
	int ch;
	int32_t event = pwm_channel_get_interrupt_status(PWM);
	static int tick_cnt[PWM_NUM_CHANNELS] = {0};

	for(ch = 0; ch < PWM_NUM_CHANNELS; ch++){
		if((event & (1<<pwm_chan[ch])) == (1<<pwm_chan[ch])){
			tick_cnt[ch]++;

			if(tick_cnt[ch] >= max_num_bursts){
				tick_cnt[ch] = 0;
				pwm_channel_disable(PWM, pwm_chan[ch]);

				if(ch == last_channel){
					// disable pwm transmitter (active low)
					TRANSMITTER_SET_PIO(TRM_PIO_PWM, TRM_PIN_PWM);
					// enable ground switch (active low)
					TRANSMITTER_CLR_PIO(TRM_PIO_GND, TRM_PIN_GND);

					tc_enable_interrupt(TC, CH, TC_IER_CPAS);
					tc_start(TC, CH);

					state_pwm = ATTENTUATE;
				}
			}
		}
	}
}

extern void TC_Handler(void){
	uint32_t dummy __attribute__((unused));

	dummy = tc_get_status(TC, CH);
	t++;
}

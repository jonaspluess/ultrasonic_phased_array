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

/*******************************************************************************
 * Timing measurement of the adc:
 * 3.0 us between trigger and first sample
 * 2.0 us between the samples
 ******************************************************************************/


#include "receiver.h"
#include "debug.h"


/******************************************************************************/
// Timer counter definitions
#define TC TC0			// Timer 1
#define CH 0			// Channel 0
#define ID_TC ID_TC0		// TC1, CH0 uses ID_TC3


/******************************************************************************/
static int max_num_banks = 0;

enum adc_channel_num_t chlist[REC_NUM_CHANNELS] = {
	ADC_CHANNEL_0,
	ADC_CHANNEL_1,
	ADC_CHANNEL_2,
	ADC_CHANNEL_3,
	ADC_CHANNEL_4,
	ADC_CHANNEL_5,
	ADC_CHANNEL_6,
	ADC_CHANNEL_7,
};

static volatile int16_t receiver_buffer[REC_NUM_BANKS][REC_NUM_CHANNELS][REC_BUF_LEN];
static volatile int16_t temp_buffer[REC_NUM_BANKS][REC_NUM_CHANNELS * REC_BUF_LEN];
static volatile int16_t (*active_buffer)[REC_BUF_LEN];
static volatile int buffer_full_flag = 0;
static volatile int finished_flag = 0;


/******************************************************************************/
static void init_timer_trigger(unsigned long sample_freq);
static int pdc_read_buffer(Adc *adc_ptr, volatile int16_t *buffer_ptr, int buf_len);


/******************************************************************************/
extern void receiver_init(unsigned long sample_freq)
{
	int i;

	// config power management for adc (asf/sam/drivers/pmc/pmc.c)
	pmc_enable_periph_clk(ID_ADC);

	// init adc (asf/sam/drivers/adc/adc.c)
	// ADC_CLK = MCK / (prescaler + 1) * 2
	// tracking_time = (TRACKING_TIME + 1) / ADCClock
	#define ADC_CLK 8600000UL
	#define TRACKING_TIME 0
	#define TRANSFER_PERIOD 0
	adc_init(ADC, sysclk_get_cpu_hz(), ADC_CLK, ADC_STARTUP_TIME_15);
	adc_configure_timing(ADC, TRACKING_TIME, ADC_SETTLING_TIME_0, TRANSFER_PERIOD);

	// enable channel number tag (asf/sam/drivers/adc/adc.c)
	adc_enable_tag(ADC);

	// configure and start the adc sequencer (asf/sam/drivers/adc/adc.c)
	adc_configure_sequence(ADC, chlist, REC_NUM_CHANNELS);
	adc_start_sequencer(ADC);

	// disable adc power save mode
	ADC->ADC_MR |= ADC_MR_FWUP_ON;

	// disable analog change (asf/sam/drivers/adc/adc.c)
	adc_disable_anch(ADC);

	for(i = 0; i < REC_NUM_CHANNELS; i++){
		adc_enable_channel(ADC, chlist[i]);
	}

	// submit buffer to the peripheral DMA controller
	pdc_read_buffer(ADC, temp_buffer[0], REC_NUM_CHANNELS * REC_BUF_LEN);
	pdc_read_buffer(ADC, temp_buffer[1], REC_NUM_CHANNELS * REC_BUF_LEN);

	// enable adc buffer full interrupt (asf/sam/drivers/adc/adc.c)
	adc_enable_interrupt(ADC, ADC_ISR_RXBUFF);

	// configure adc timer trigger
	init_timer_trigger(sample_freq);
	adc_configure_trigger(ADC, ADC_TRIG_TIO_CH_0, 0);
}

extern void receiver_start(int bank_cnt)
{
	max_num_banks = bank_cnt;

	buffer_full_flag = 0;
	finished_flag = 0;

	tc_start(TC, CH);
}

extern volatile int16_t (*receiver_get_buffer(int *stopped_flag))[REC_BUF_LEN]
{
	if(buffer_full_flag != 0){
		buffer_full_flag = 0;

		if(finished_flag != 0){
			*stopped_flag = 1;
		}else{
			*stopped_flag = 0;
		}
		return  active_buffer;
	}else{
		return NULL;
	}
}


/******************************************************************************/
// init timer trigger
static void init_timer_trigger(unsigned long sample_freq)
{
	uint32_t ul_div = 0;
	uint32_t ul_tc_clks = 0;

	// config power management for timer/counter (asf/sam/drivers/pmc/pmc.c)
	pmc_enable_periph_clk(ID_TC);

	// init timer/counter (asf/sam/drivers/tc/tc.c)
	tc_find_mck_divisor(sample_freq, sysclk_get_cpu_hz(), &ul_div, &ul_tc_clks, sysclk_get_cpu_hz());
	tc_init(TC, CH, ul_tc_clks | TC_CMR_CPCTRG | TC_CMR_WAVE | TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_SET);

	TC->TC_CHANNEL[CH].TC_RA = (sysclk_get_cpu_hz() / ul_div / sample_freq) / 2;
	TC->TC_CHANNEL[CH].TC_RC = (sysclk_get_cpu_hz() / ul_div / sample_freq) / 1;
}

// submit a buffer to the peripheral DMA controller
static int pdc_read_buffer(Adc *adc_ptr, volatile int16_t *buffer_ptr, int buf_len)
{
	if((adc_ptr->ADC_RCR == 0) && (adc_ptr->ADC_RNCR == 0)){
		adc_ptr->ADC_RPR = (uint32_t) buffer_ptr;
		adc_ptr->ADC_RCR = buf_len;
		adc_ptr->ADC_PTCR = ADC_PTCR_RXTEN;

		return 1;
	}else{
		if(adc_ptr->ADC_RNCR == 0){
			adc_ptr->ADC_RNPR = (uint32_t) buffer_ptr;
			adc_ptr->ADC_RNCR = buf_len;

			return 1;
		}else{
			return 0;
		}
	}
}


/******************************************************************************/
extern void ADC_Handler(void)
{
	static int bnk_cnt = 0;			/* bank counter */
	static int bnk_switch_cnt = 0;		/* bank switch counter */
	int i;					/* buffer counter */
	int k;					/* channel counter */

	if((adc_get_status(ADC) & ADC_ISR_RXBUFF) == ADC_ISR_RXBUFF){
		bnk_switch_cnt++;
		if(bnk_switch_cnt >= max_num_banks){
			tc_stop(TC, CH);
		}

		pdc_read_buffer(ADC, temp_buffer[(bnk_cnt+1)%REC_NUM_BANKS], REC_NUM_CHANNELS * REC_BUF_LEN);

		for(i = 0; i < REC_BUF_LEN; i++){
			for(k = 0; k < REC_NUM_CHANNELS; k++){
				receiver_buffer[bnk_cnt][k][i] = temp_buffer[bnk_cnt][i * REC_NUM_CHANNELS + k] & ADC_LCDR_LDATA_Msk;
			}
		}

		buffer_full_flag = 1;
		active_buffer = receiver_buffer[bnk_cnt];

		bnk_cnt++;
		if(bnk_cnt >= REC_NUM_BANKS){
			bnk_cnt = 0;
		}
		if(bnk_switch_cnt >= max_num_banks){
			bnk_cnt = 0;
			bnk_switch_cnt = 0;
			finished_flag = 1;
		}
	}
}

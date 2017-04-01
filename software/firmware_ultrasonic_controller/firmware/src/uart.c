/**
 * \file
 *
 * \brief UART functions
 *
 * Copyright (c) 2012-2015 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#include <asf.h>
#include <stdint.h>
#include <stdbool.h>

#include "uart.h"


#define USART_PERIPH_CLK_ENABLE() sysclk_enable_peripheral_clock(USART_ID)


#define USART_BASE       ((Usart*)UART)
#define USART_ID         ID_UART

#define USART_HANDLER    UART_Handler
#define USART_INT_IRQn   UART_IRQn
#define USART_INT_LEVEL  3


static sam_usart_opt_t usart_options;

ISR(USART_HANDLER)
{
	uint32_t sr = usart_get_status(USART_BASE);

	if (sr & US_CSR_RXRDY) {
		// Data received
		uint32_t value;
		bool b_error = usart_read(USART_BASE, &value) ||
			(sr & (US_CSR_FRAME | US_CSR_TIMEOUT | US_CSR_PARE));
		if (b_error) {
			usart_reset_rx(USART_BASE);
			usart_enable_rx(USART_BASE);
			udi_cdc_signal_framing_error();
		}
		// Transfer UART RX fifo to CDC TX
		if (!udi_cdc_is_tx_ready()) {
			// Fifo full
			udi_cdc_signal_overrun();
		} else {
			udi_cdc_putc(value);
		}
		return;
	}

	if (sr & US_CSR_TXRDY) {
		// Data send
		if (udi_cdc_is_rx_ready()) {
			// Transmit next data
			int c = udi_cdc_getc();
			usart_write(USART_BASE, c);

		} else {
			// Fifo empty then Stop UART transmission
			usart_disable_tx(USART_BASE);
			usart_disable_interrupt(USART_BASE, US_IDR_TXRDY);
		}
	}
}


void uart_rx_notify(uint8_t port)
{
	UNUSED(port);
	// If UART is open
	if (usart_get_interrupt_mask(USART_BASE) & US_IMR_RXRDY) {
		// Enable UART TX interrupt to send a new value
		usart_enable_tx(USART_BASE);
		usart_enable_interrupt(USART_BASE, US_IER_TXRDY);
	}
}


void uart_config(uint8_t port, usb_cdc_line_coding_t * cfg)
{
	uint32_t stopbits, parity, databits;
	uint32_t imr;
	UNUSED(port);

	switch (cfg->bCharFormat) {
	case CDC_STOP_BITS_2:
		stopbits = US_MR_NBSTOP_2_BIT;
		break;
	case CDC_STOP_BITS_1_5:
		stopbits = US_MR_NBSTOP_1_5_BIT;
		break;
	case CDC_STOP_BITS_1:
	default:
		// Default stop bit = 1 stop bit
		stopbits = US_MR_NBSTOP_1_BIT;
		break;
	}

	switch (cfg->bParityType) {
	case CDC_PAR_EVEN:
		parity = US_MR_PAR_EVEN;
		break;
	case CDC_PAR_ODD:
		parity = US_MR_PAR_ODD;
		break;
	case CDC_PAR_MARK:
		parity = US_MR_PAR_MARK;
		break;
	case CDC_PAR_SPACE:
		parity = US_MR_PAR_SPACE;
		break;
	default:
	case CDC_PAR_NONE:
		parity = US_MR_PAR_NO;
		break;
	}

	switch(cfg->bDataBits) {
	case 5: case 6: case 7:
		databits = cfg->bDataBits - 5;
		break;
	default:
	case 8:
		databits = US_MR_CHRL_8_BIT;
		break;
	}

	// Options for USART.
	usart_options.baudrate = LE32_TO_CPU(cfg->dwDTERate);
	usart_options.char_length = databits;
	usart_options.parity_type = parity;
	usart_options.stop_bits = stopbits;
	usart_options.channel_mode = US_MR_CHMODE_NORMAL;
	imr = usart_get_interrupt_mask(USART_BASE);
	usart_disable_interrupt(USART_BASE, 0xFFFFFFFF);
	usart_init_rs232(USART_BASE, &usart_options,
			sysclk_get_peripheral_bus_hz(USART_BASE));
	// Restore both RX and TX
	usart_enable_tx(USART_BASE);
	usart_enable_rx(USART_BASE);
	usart_enable_interrupt(USART_BASE, imr);
}

bool uart_open(uint8_t port)
{
	UNUSED(port);
	return true;
}

void uart_close(uint8_t port)
{
	UNUSED(port);
}

/* Copyright (c) 2007 Fabian Greif
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
// ----------------------------------------------------------------------------


#include <avr/io.h>
#include <util/delay.h>

#if ARDUINO>=100
#include <Arduino.h> // Arduino 1.0
#else
#include <Wprogram.h> // Arduino 0022
#endif
#include <stdint.h>
#include <avr/pgmspace.h>

#include "global.h"
#include "mcp2515.h"
#include "mcp2515_defs.h"


#include "defaults.h"
#include "TinySoftwareSPI.h"

// -------------------------------------------------------------------------
void mcp2515_write_register( uint8_t adress, uint8_t data )
{
	RESET(MCP2515_CS);

	SPI.transfer(SPI_WRITE);
	SPI.transfer(adress);
	SPI.transfer(data);

	SET(MCP2515_CS);
}

// -------------------------------------------------------------------------
uint8_t mcp2515_read_register(uint8_t adress)
{
	uint8_t data;

	RESET(MCP2515_CS);

	SPI.transfer(SPI_READ);
	SPI.transfer(adress);

	data = SPI.transfer(0xff);

	SET(MCP2515_CS);

	return data;
}

// -------------------------------------------------------------------------
void mcp2515_bit_modify(uint8_t adress, uint8_t mask, uint8_t data)
{
	RESET(MCP2515_CS);

	SPI.transfer(SPI_BIT_MODIFY);
	SPI.transfer(adress);
	SPI.transfer(mask);
	SPI.transfer(data);

	SET(MCP2515_CS);
}

// ----------------------------------------------------------------------------
uint8_t mcp2515_read_status(uint8_t type)
{
	uint8_t data;

	RESET(MCP2515_CS);

	SPI.transfer(type);
	data = SPI.transfer(0xff);

	SET(MCP2515_CS);

	return data;
}

// -------------------------------------------------------------------------
uint8_t mcp2515_init(uint8_t speed)
{


	SET(MCP2515_CS);
	SET_OUTPUT(MCP2515_CS);

	RESET(P_SCK);
	RESET(P_MOSI);
	RESET(P_MISO);

	SET_OUTPUT(P_SCK);
	SET_OUTPUT(P_MOSI);
	SET_INPUT(P_MISO);

	SET_INPUT(MCP2515_INT);
	SET(MCP2515_INT);

	// active SPI master interface
	// SPCR = (1<<SPE)|(1<<MSTR) | (0<<SPR1)|(1<<SPR0); // enable, master,
	// SPSR = 0; // f.osc / 16 (SPR0 set above)
        SPI.begin();
        SPI.setDataMode(SPI_MODE0); // just a guess as to which mode

	// reset MCP2515 by software reset.
	// After this he is in configuration mode.
	RESET(MCP2515_CS);
	SPI.transfer(SPI_RESET);
	SET(MCP2515_CS);

	// wait a little bit until the MCP2515 has restarted
	// _delay_us(10);
        delay(10);

	// load CNF3..1 Registers (they are in that order)
	RESET(MCP2515_CS);
	SPI.transfer(SPI_WRITE);
	SPI.transfer(CNF3); // start with 3, next is 2, then 1, then CANINTE

/*	SPI.transfer((1<<PHSEG21));		// Bitrate 125 kbps at 16 MHz
	SPI.transfer((1<<BTLMODE)|(1<<PHSEG11));
	SPI.transfer((1<<BRP2)|(1<<BRP1)|(1<<BRP0));
*/
/*
	SPI.transfer((1<<PHSEG21));		// Bitrate 250 kbps at 16 MHz
	SPI.transfer((1<<BTLMODE)|(1<<PHSEG11));
	SPI.transfer((1<<BRP1)|(1<<BRP0));
*/
	SPI.transfer((1<<PHSEG21)); // CNF3     // Bitrate 250 kbps at 16 MHz
	SPI.transfer((1<<BTLMODE)|(1<<PHSEG11)); // CNF2
	//SPI.transfer(1<<BRP0);
    SPI.transfer(speed); // CNF1

	// activate interrupts
	SPI.transfer((1<<RX1IE)|(1<<RX0IE)); // CANINTE
	SET(MCP2515_CS);

	// test if we could read back the value => is the chip accessible?
	if (mcp2515_read_register(CNF1) != speed) {
		SET(LED2_HIGH);
		return false;
	}

	// deaktivate the RXnBF Pins (High Impedance State)
	mcp2515_write_register(BFPCTRL, 0);

	// set TXnRTS as inputs
	mcp2515_write_register(TXRTSCTRL, 0);

	// turn off filters => receive any message
	mcp2515_write_register(RXB0CTRL, (1<<RXM1)|(1<<RXM0));
	mcp2515_write_register(RXB1CTRL, (1<<RXM1)|(1<<RXM0));

	// reset device to normal mode
	mcp2515_write_register(CANCTRL, 0);
//	SET(LED2_HIGH);
	return true;
}

// ----------------------------------------------------------------------------
// check if there are any new messages waiting

uint8_t mcp2515_check_message(void) {
	return (!IS_SET(MCP2515_INT));
}

// ----------------------------------------------------------------------------
// check if there is a free buffer to send messages

uint8_t mcp2515_check_free_buffer(void)
{
	uint8_t status = mcp2515_read_status(SPI_READ_STATUS);

	if ((status & 0x54) == 0x54) {
		// all buffers used
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------------
uint8_t mcp2515_get_message(tCAN *message)
{
	// read status
	uint8_t status = mcp2515_read_status(SPI_RX_STATUS);
	uint8_t addr;
	uint8_t t;
	if (bit_is_set(status,6)) {
		// message in buffer 0
		addr = SPI_READ_RX;
	}
	else if (bit_is_set(status,7)) {
		// message in buffer 1
		addr = SPI_READ_RX | 0x04;
	}
	else {
		// Error: no message available
		return 0;
	}

	RESET(MCP2515_CS);
	SPI.transfer(addr);

	// read id
	message->id  = (uint16_t) SPI.transfer(0xff) << 3;
	message->id |=            SPI.transfer(0xff) >> 5;

	SPI.transfer(0xff);
	SPI.transfer(0xff);

	// read DLC
	uint8_t length = SPI.transfer(0xff) & 0x0f;

	message->header.length = length;
	message->header.rtr = (bit_is_set(status, 3)) ? 1 : 0;

	// read data
	for (t=0;t<length;t++) {
		message->data[t] = SPI.transfer(0xff);
	}
	SET(MCP2515_CS);

	// clear interrupt flag
	if (bit_is_set(status, 6)) {
		mcp2515_bit_modify(CANINTF, (1<<RX0IF), 0);
	}
	else {
		mcp2515_bit_modify(CANINTF, (1<<RX1IF), 0);
	}

	return (status & 0x07) + 1;
}

// ----------------------------------------------------------------------------
uint8_t mcp2515_send_message(tCAN *message)
{
	uint8_t status = mcp2515_read_status(SPI_READ_STATUS);

	/* Statusbyte:
	 *
	 * Bit	Function
	 *  2	TXB0CNTRL.TXREQ
	 *  4	TXB1CNTRL.TXREQ
	 *  6	TXB2CNTRL.TXREQ
	 */
	uint8_t address;
	uint8_t t;
//	SET(LED2_HIGH);
	if (bit_is_clear(status, 2)) {
		address = 0x00;
	}
	else if (bit_is_clear(status, 4)) {
		address = 0x02;
	}
	else if (bit_is_clear(status, 6)) {
		address = 0x04;
	}
	else {
		// all buffer used => could not send message
		return 0;
	}

	RESET(MCP2515_CS);
	SPI.transfer(SPI_WRITE_TX | address);

	SPI.transfer(message->id >> 3);
    SPI.transfer(message->id << 5);

	SPI.transfer(0);
	SPI.transfer(0);

	uint8_t length = message->header.length & 0x0f;

	if (message->header.rtr) {
		// a rtr-frame has a length, but contains no data
		SPI.transfer((1<<RTR) | length);
	}
	else {
		// set message length
		SPI.transfer(length);

		// data
		for (t=0;t<length;t++) {
			SPI.transfer(message->data[t]);
		}
	}
	SET(MCP2515_CS);

	_delay_us(1);

	// send message
	RESET(MCP2515_CS);
	address = (address == 0) ? 1 : address;
	SPI.transfer(SPI_RTS | address);
	SET(MCP2515_CS);

	return address;
}

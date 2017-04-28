/*
 * Copyright (c) 2012 by Thomas Carpenter
 * Software based SPI Master Library for Tiny core.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 *
 * Currently, this runs at 125kHz on an 8MHz clock.
 */

#include "TinySoftwareSPI.h"
#include "Arduino.h"
#include "defaults.h" // pinouts of kitchen sink to MCP2515
#include "global.h" // SET() RESET() IS_SET() SET_OUTPUT()

SoftSPIClass::SoftSPIClass(){
	_bitOrder = MSBFIRST;
	_mode = SPI_MODE0;
	_running = false;
        transferType = &SoftSPIClass::noTransfer;
}

void SoftSPIClass::writeSS(boolean state){
	if (state) {
		SET(MCP2515_CS);// *_SS_PORT |= _SS_HIGH;
	} else {
		RESET(MCP2515_CS);// *_SS_PORT &= _SS_LOW;
	}
}

void SoftSPIClass::begin(){
	//_SS = SS_;
	//_SCK = SCK_;
	//_MISO = MISO_;
	//_MOSI = MOSI_;
	//
	//byte MOSIport = digitalPinToPort(_MOSI);
	//byte SSport = digitalPinToPort(_SS);
	//byte SCKport = digitalPinToPort(_SCK);
	//byte MISOport = digitalPinToPort(_MISO);
	//	
	//if ((MOSIport == NOT_A_PIN) ||
	//	(  SSport == NOT_A_PIN) ||
	//	( SCKport == NOT_A_PIN) ||
	//	(MISOport == NOT_A_PIN) ){
	//	end();
	//} else {
		_running = true;
		SET_OUTPUT(P_MOSI); // pinMode(_MOSI, OUTPUT);
		SET_OUTPUT(P_MISO); // pinMode(_MISO, INPUT);
		SET_OUTPUT(P_SCK ); // pinMode(_SCK, OUTPUT);
		SET_OUTPUT(MCP2515_CS); // pinMode(_SS, OUTPUT);
		// _MOSI_PORT = portOutputRegister(MOSIport);
		// _MOSI_HIGH = digitalPinToBitMask(_MOSI);
		// _MOSI_LOW = ~_MOSI_HIGH;
		// _SCK_PORT = portOutputRegister(SCKport);
		// _SCK_HIGH = digitalPinToBitMask(_SCK);
		// _SCK_LOW = ~_SCK_HIGH;
		// _SS_PORT = portOutputRegister(SSport);
		// _SS_HIGH = digitalPinToBitMask(_SS);
		// _SS_LOW = ~_SS_HIGH;
		// _MISO_PIN = portInputRegister(MISOport);
		// _MISO_MASK = digitalPinToBitMask(_MISO);
		SET(MCP2515_CS);// *_SS_PORT |= _SS_HIGH;
		RESET(P_SCK);// RESET(P_SCK);
		RESET(P_MOSI);// RESET(P_MOSI);
		
		_mode = SPI_MODE0;
		transferType = &SoftSPIClass::transferMode0;
	//}
}

byte SoftSPIClass::noTransfer(byte _data){
	//This does nothing. If you call SPI.transfer() before calling begin() or after calling end(), the call will be redirected here to avoid crash.
	return 0xFF;
}

byte SoftSPIClass::transferMode0(byte _data){
	byte _newData = 0;
	for (byte i = 0;i < 8; i++){
		if(_data & 0x80){
			SET(P_MOSI);
		} else {
			RESET(P_MOSI);
		}
		_data <<= 1;
		SET(P_SCK);
		_newData <<= 1;
		_newData |= IS_SET(P_MISO);
		RESET(P_SCK);
	}
	return _newData;
}
byte SoftSPIClass::transferMode1(byte _data){
	byte _newData = 0;
	for (byte i = 0;i < 8; i++){
		SET(P_SCK);
		if(_data & 0x80){
			SET(P_MOSI);
		} else {
			RESET(P_MOSI);
		}
		_data <<= 1;
		RESET(P_SCK);
		_newData <<= 1;
		_newData |= IS_SET(P_MISO);
	}
	return _newData;
}
byte SoftSPIClass::transferMode2(byte _data){
	byte _newData = 0;
	for (byte i = 0;i < 8; i++){
		if(_data & 0x80){
			SET(P_MOSI);
		} else {
			RESET(P_MOSI);
		}
		_data <<= 1;
		RESET(P_SCK);
		_newData <<= 1;
		_newData |= IS_SET(P_MISO);
		SET(P_SCK);
	}
	return _newData;
}
byte SoftSPIClass::transferMode3(byte _data){
	byte _newData = 0;
	for (byte i = 0;i < 8; i++){
		RESET(P_SCK);
		if(_data & 0x80){
			SET(P_MOSI);
		} else {
			RESET(P_MOSI);
		}
		_data <<= 1;
		SET(P_SCK);
		_newData <<= 1;
		_newData |= IS_SET(P_MISO);
	}
	return _newData;
}

byte SoftSPIClass::transfer(byte _data){
	byte _newData = 0;
    byte oldSREG = SREG;
	cli();
	if (_bitOrder == MSBFIRST){
	//Send data
		_newData = (*this.*transferType)(_data);
		SREG = oldSREG; 
		return _newData;
	} else {
		//flip the data
		for(byte i = 0; i < 8; i++){
		  _newData <<= 1;
		  _newData |= _data & 1;
		  _data >>= 1;
		}
		//SPI transfer
		_newData = (*this.*transferType)(_newData);
		SREG = oldSREG;
		//flip data back.
		_data = 0;
		for(byte i = 0; i < 8; i++){
		  _data <<= 1;
		  _data |= _newData & 1;
		  _newData >>= 1;
		}
		return _data;
	}
}

void SoftSPIClass::end(){
	_running = false;
	transferType = &SoftSPIClass::noTransfer;
}

void SoftSPIClass::setBitOrder(uint8_t bitOrder) {
	_bitOrder = bitOrder;
}

void SoftSPIClass::setDataMode(uint8_t mode)
{
	_mode = mode;
	if(_mode == SPI_MODE0){
		transferType = &SoftSPIClass::transferMode0;
	} else if (_mode == SPI_MODE1){
		transferType = &SoftSPIClass::transferMode1;
	} else if (_mode == SPI_MODE2){
		transferType = &SoftSPIClass::transferMode2;
	} else if (_mode == SPI_MODE3){
		transferType = &SoftSPIClass::transferMode3;
	} else {
		_mode = SPI_MODE0;
		transferType = &SoftSPIClass::transferMode0;
	}
	if(_mode & 0x02){
		SET(P_SCK);
	} else {
		RESET(P_SCK);
	}
}

void SoftSPIClass::setClockDivider(uint8_t rate)
{
	
	//does nothing as the speed cannot be changed - fixed at Fcpu/16
	
}

SoftSPIClass SPI;

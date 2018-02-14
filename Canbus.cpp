/**
 *
 *
 * Copyright (c) 2008-2009  All rights reserved.
 */

#if ARDUINO>=100
#include <Arduino.h> // Arduino 1.0
#else
#include <Wprogram.h> // Arduino 0022
#endif
#include <stdint.h>
#include <avr/pgmspace.h>

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "pins_arduino.h"
#include <inttypes.h>
#include "mcp2515.h"
#include "Canbus.h"




/* C++ wrapper */
CanbusClass::CanbusClass() {


}
char CanbusClass::message_rx(unsigned char *buffer, uint16_t *id, uint8_t *length) {
    tCAN message;
    if (mcp2515_check_message()) { // read the message from the buffer of the MCP2515
      if (mcp2515_get_message(&message)) {
      //  print_can_message(&message); PRINT("\n");
        *id = message.id;
        *length = message.header.length;
        buffer[0] = message.data[0];
        buffer[1] = message.data[1];
        buffer[2] = message.data[2];
        buffer[3] = message.data[3];
        buffer[4] = message.data[4];
        buffer[5] = message.data[5];
        buffer[6] = message.data[6];
        buffer[7] = message.data[7];
//        buffer[] = message[];
//        buffer[] = message[];
//        buffer[] = message[];
//        buffer[] = message[];
      }
      else {
      Serial.println("Cannot read message");
      }
    }
}

char CanbusClass::zero_control(byte dest, uint16_t control, byte num_modules) {
  tCAN message;


  // einige Testwerte
  message.id = 0x506;
  message.header.rtr = 0;
  message.header.length = 8;
  message.data[0] = dest;
  message.data[1] = control & 255;
  message.data[2] = control >> 8;
  message.data[3] = num_modules;
  message.data[4] = 0x10;
  message.data[5] = 0x00;
  message.data[6] = 0x00;
  message.data[7] = 0x00;

  if (mcp2515_send_message(&message)) {
    return 1;
  } else {
    //Serial.println("Error: could not send the message");
    return 0;
  }
return 1;
}


char CanbusClass::init(unsigned char speed) {

  //mcp2515_bit_modify(CANCTRL, (1<<REQOP2)|(1<<REQOP1)|(1<<REQOP0), 0); // i guess this tells the chip to receive everything?
  return mcp2515_init(speed);

}

CanbusClass Canbus;

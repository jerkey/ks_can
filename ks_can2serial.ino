// fuses must have lfuse bit 6 low, aka cleared, aka programmed, to enable CKOUT so that PE7 sends system clock to MCP2515 OSC1 input
// E:F5, H:DA, L:BF  avrdude -p m1280 -U lfuse:w:0xBF:m
#include "Canbus.h"
#include "mcp2515.h"
byte buffer[16];
uint16_t id;
uint8_t length;

void setup(){
Serial.begin(230400);
Serial.println("CAN-Bus Demo");

if(Canbus.init(CANSPEED_500)) {
    Serial.println("CAN Init ok");
  } else {
    Serial.println("Can't init CAN");
  }
}

void loop(){
  id=0;
  Canbus.message_rx(buffer,&id,&length);
  if (id > 2047) {
    Serial.print("WTF ");
    Serial.println(id);
  }
  if (id < 256) Serial.print("0"); // pad zero
  if (id < 16) Serial.print("0"); // pad zero
  Serial.print(id,HEX);
  Serial.print(':');
  printBuf();
}

void printBuf() {
  for (int i=0; i < length; i++) {
    if (buffer[i] < 16) Serial.print("0");
    Serial.print(buffer[i],HEX);
  }
  Serial.println();
}

// fuses must have lfuse bit 6 low, aka cleared, aka programmed, to enable CKOUT so that PE7 sends system clock to MCP2515 OSC1 input
// E:F5, H:DA, L:BF  avrdude -p m1280 -U lfuse:w:0xBF:m
#include "Canbus.h"
#include "mcp2515.h"
byte buffer[16];
uint16_t id;
uint8_t length;
unsigned long lastCanSent = 0; // last time we sent a canbus packet
#define SENDBRUSA_INTERVAL      90 // how many milliseconds between messages sent
int16_t deciVoltsRequested = -1; // we won't send CAN if negative
int16_t deciAmpsRequested = 10; // 5 = 5 Amps

void setup(){
Serial.begin(230400);
Serial.println("CAN-Bus control Brusa NLG5");

if(Canbus.init(CANSPEED_500)) {
    Serial.println("CAN Init ok");
  } else {
    Serial.println("Can't init CAN");
  }
}

void loop(){
  id=0;
  while (id==0) {
    Canbus.message_rx(buffer,&id,&length); // wait until a can packet comes in
    if (Serial.available() > 0) {
      int16_t inInt = Serial.parseInt(); // get next valid integer in the incoming serial stream (or 0)
      Serial.print("deciVoltsRequested = ");
      Serial.println(inInt);
      deciVoltsRequested = inInt; // why not
    }
  }
  if (deciVoltsRequested >= 0) sendBrusa(deciVoltsRequested, deciAmpsRequested);
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

void sendBrusa(int16_t deciVolts, int16_t deciAmps) {
  if (millis() - lastCanSent > SENDBRUSA_INTERVAL) {
    lastCanSent = millis(); // reset the timer
//Bytes: 1=NLG5_C_C_EN, 0, AC_I*10, BATT_V*10/256, BATT_V*10&255, BATT_I*10/256, BATT_I*10&255  (NOTE: add 2 to first byte to clear latched errors)
    id = 0x618; // brusa control CAN ID
    length = 7;
    buffer[0] = flipByte(1); // 1=NLG5_C_C_EN=enable, +2 to clear latched faults
    if (deciVolts==1) { // clear errors
      buffer[0] = flipByte(3); // clear errors
    }
    buffer[1] = (0); // MSB of mains current*10 = 0 always :)
    buffer[2] = (100); // 100=10.0 AMPS mains current
    buffer[3] = (0x08);
    buffer[4] = (0xfc);
    buffer[5] = (0x00);
    buffer[6] = (0x64);
    //buffer[3] = (deciVolts * 10) / 256;
    //buffer[4] = (deciVolts * 10) % 256;
    //buffer[5] = (deciAmps * 10) / 256;
    //buffer[6] = (deciAmps * 10) % 256;
    Canbus.message_tx(buffer,&id,&length);
    //id = 0x718;
    //length = 8;
    //Canbus.message_tx(buffer,&id,&length);
  }
}

void printBuf() {
  for (int i=0; i < length; i++) {
    if (buffer[i] < 16) Serial.print("0");
    Serial.print(buffer[i],HEX);
  }
  Serial.print(" ");
  Serial.println(millis());
}

byte flipByte(byte c) { // https://forum.arduino.cc/index.php?topic=117966.0
  c = ((c>>1)&0x55)|((c<<1)&0xAA);
  c = ((c>>2)&0x33)|((c<<2)&0xCC);
  c = (c>>4) | (c<<4) ;
  return c;
}

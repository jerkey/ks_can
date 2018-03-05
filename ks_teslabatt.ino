// fuses must have lfuse bit 6 low, aka cleared, aka programmed, to enable CKOUT so that PE7 sends system clock to MCP2515 OSC1 input
// E:F5, H:DA, L:BF  avrdude -p m1280 -U lfuse:w:0xBF:m
#include "Canbus.h"
#include "mcp2515.h"
byte buffer[16];
uint16_t id;
uint8_t length;
uint16_t x126count=0;
uint16_t DI_vBat; // from BO_ 294
uint16_t DI_motorCurrent = 0; // from BO_ 294

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
  fakeBMS();
  id=0;
  Canbus.message_rx(buffer,&id,&length);
  if (id == 0x0126) { if (x126count++ > 5) { x126count=0; // add ++ after first x???count to enable display
    // BO_ 294 DI_motorControl: 8 DI  SG_ DI_vBat : 0|10@1+ (0.5,0) [0|500] "V" GTW  SG_ DI_motorCurrent : 16|11@1+ (1,0) [0|2047] "A" GTW
    DI_vBat = (((uint16_t)buffer[0]) + (((uint16_t)buffer[1] & 3) << 8)) / 2;
    DI_motorCurrent = (((uint16_t)buffer[2]) + ((uint16_t)(buffer[3] & 7) << 8));
    Serial.print("DI_vBat:");
    Serial.print(DI_vBat);
    Serial.print("   DI_motorCurrent:");
    Serial.println(DI_motorCurrent);
  }
  } else if (id == 0x288) {// Serial.print("288 ");printBuf();
  } else if (id) {// Serial.print(" 0x"); Serial.print(id,HEX);
  }
}

void fakeBMS() {
  unsigned long time = millis();

  static unsigned long every10 = 0;
  if (time - every10 > 10) {
    every10 = time; // reset interval timer
    id=0x102; // every 10ms   // 0x102 - Dec 258 - BMS - HV Bus Status
    length=8;                 // 0x16 0x92 0x03 0xA7 0x06 0x4E 0x43 0x01
    buffer[0]=DI_vBat*100%256;
    buffer[1]=DI_vBat*100/256;
    buffer[2]=0x03;
    buffer[3]=0xA7;
    buffer[4]=0x06;
    buffer[5]=0x4E;
    buffer[6]=0x43;
    buffer[7]=0x01;
    Canbus.message_tx(buffer,id,length);
  }

  static unsigned long every100 = 0;
  if (time - every100 > 100) {
    every100 = time; // reset interval timer
    id=0x202; // every 100ms      // 0x202 - Dec 514 - BMS - Drive Limits
    length=8;
    buffer[0]=0xF7;
    buffer[1]=0x5E;
    buffer[2]=0x4E;
    buffer[3]=0x9D;
    buffer[4]=0xC4;
    buffer[5]=0x09;
    buffer[6]=0x3F;
    buffer[7]=0x23;
    Canbus.message_tx(buffer,id,length);

    id=0x212; // every 100ms      // 0x212 - Dec 530 - BMS - Status
    length=5;
    buffer[0]=0xD8;
    buffer[1]=0x08;
    buffer[2]=0x12;
    buffer[3]=0x72;
    buffer[4]=0x00;
    Canbus.message_tx(buffer,id,length);

    id=0x232; // every 100ms      // 0x232 - Dec 562 - BMS - Power Available (Max Regen and Discharge)
    length=4;
    buffer[0]=0x1B;           // 16 bit Max Regen power in kW (divide 16 bit by 100, so 65535 = 655.35kW)
    buffer[1]=0x19;           // 191B = 64.27kW  (26EA = 99.62kW)
    buffer[2]=0xF5;           // 16 bit Max Drive power in kW (divide 16 bit by 100, so 65535 = 655.35kW)
    buffer[3]=0x84;           // 84F5 = 340.37kW  (7EA3 = 324.19)
    Canbus.message_tx(buffer,id,length);
  }
}

void printBuf() {
  for (int i=0; i < length; i++) {
    if (buffer[i] < 16) Serial.print("0");
    Serial.print(buffer[i],HEX);
    Serial.print(" ");
  }
  Serial.println();
}

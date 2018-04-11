// fuses must have lfuse bit 6 low, aka cleared, aka programmed, to enable CKOUT so that PE7 sends system clock to MCP2515 OSC1 input
// E:F5, H:DA, L:BF  avrdude -p m1280 -U lfuse:w:0xBF:m
#include "Canbus.h"
#include "mcp2515.h"
byte buffer[16];
uint16_t id;
uint8_t length;
uint16_t x126count=0;
uint16_t x227count=0;
uint16_t DI_vBat; // from BO_ 294
uint16_t DI_motorCurrent = 0; // from BO_ 294
float CHGPH1_vBat = 0; // from BO_ 551 CHGPH1_HVStatus: 8 CHGPH1
uint16_t BMS_chargeCommand = 0; // charging requested, in kW 0 to 10
uint8_t  BMS_chargeEnable = 0; // 0 for don't charge, 0x40 for do charge
uint8_t  BMS_chgVLimitMode = 0; // 0 for disable, 0x10 for enable

#define BMS_CTRSET_CLOSED       2
#define BMS_DRIVE               (1*16)
#define BMS_SUPPORT             (2*16)
#define BMS_CHARGER             (3*16)
uint8_t BMS_stateByte = BMS_CTRSET_CLOSED + BMS_DRIVE; // lower 4 bits watched by CP,CHG,DI, upper 4 bits watched by CP

void setup(){
Serial.begin(230400);
Serial.println("CAN-Bus Demo");

if(Canbus.init(CANSPEED_500)) {
    Serial.println("CAN Init ok");
  } else {
    Serial.println("Can't init CAN");
  }
  Serial.println("press ? for info"); // for charging
  Serial.println("press d for drive mode (default)"); // for charging
  Serial.println("press s for support mode"); // for charging
  Serial.println("press c for charge mode"); // for charging
}

void loop(){
  if (Serial.available()) handleSerial();
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
  } else if (id == 0x227) {
    CHGPH1_vBat = (((uint16_t)buffer[2]) + ((uint16_t)buffer[3] << 8)) * 0.010528564;// BO_ 551 CHGPH1_HVStatus: 8 CHGPH1
    if (x227count++ > 5) { x227count=0;
      Serial.print("CHGPH1_vBat:");
      Serial.println(CHGPH1_vBat);
    }
  } else if (id) {// Serial.print(" 0x"); Serial.print(id,HEX);
  }
}

void handleSerial() {
  byte inByte = Serial.read();
  Serial.print(char(inByte));
  if (inByte == 'd') {
    BMS_stateByte = BMS_CTRSET_CLOSED + BMS_DRIVE; // for driving
    BMS_chargeEnable = 0;
    BMS_chgVLimitMode = 0;
    BMS_chargeCommand = 0;
  }
  if (inByte == 's') {
    BMS_stateByte = BMS_CTRSET_CLOSED + BMS_SUPPORT; // for before charging
    BMS_chargeEnable = 0;
    BMS_chgVLimitMode = 0;
    BMS_chargeCommand = 0;
  }
  if (inByte == 'c') {
    BMS_stateByte = BMS_CTRSET_CLOSED + BMS_CHARGER; // for charging
    BMS_chargeEnable = 0x40;
    BMS_chgVLimitMode = 0x10; // may need to wait 16 seconds before doing this one
    BMS_chargeCommand = 10;
  }
  if (inByte == '?') {
    Serial.println(BMS_stateByte,HEX); // for charging
  }
}

void fakeBMS() {
  unsigned long time = millis();

  static unsigned long every10 = 0;
  if (time - every10 > 10) {
    every10 = time; // reset interval timer
    id=0x102; // every 10ms   // 0x102 - Dec 258 - BMS - HV Bus Status
    length=8;                 // 0x16 0x92 0x03 0xA7 0x06 0x4E 0x43 0x01
    buffer[0]=(uint32_t)(CHGPH1_vBat*100)%256;
    buffer[1]=(uint32_t)(CHGPH1_vBat*100)/256;
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
    buffer[2]=BMS_stateByte; // lowest 4 bits are BMS_contactorState : 6 "BMS_CTRSET_CLEANING" 5 "BMS_CTRSET_WELD" 4 "BMS_CTRSET_SNA" 3 "BMS_CTRSET_OPENING" 2 "BMS_CTRSET_CLOSED" 1 "BMS_CTRSET_PRECHARGE" 0 "BMS_CTRSET_OPEN"
    // upper 4 bits are BMS_state : 15 "BMS_SNA" 8 "BMS_WELD" 7 "BMS_FAULT" 6 "BMS_CLEARFAULT" 5 "BMS_CHARGERVOLTAGE" 4 "BMS_FASTCHARGE" 3 "BMS_CHARGER" 2 "BMS_SUPPORT" 1 "BMS_DRIVE" 0 "BMS_STANDBY"
    buffer[3]=0x72;
    buffer[4]=0x00;
    Canbus.message_tx(buffer,id,length);

    id=0x232; // every 100ms      // 0x232 - Dec 562 - BMS - Power Available (Max Regen and Discharge)
    length=4;
    buffer[0]=0x1B;           // 16 bit Max Regen power in kW (divide 16 bit by 100, so 65535 = 655.35kW)
    buffer[1]=0x19;           // 191B = 64.27kW  (26EA = 99.62kW)
    buffer[2]=0xE4;           // 16 bit Max Drive power in kW (divide 16 bit by 100, so 65535 = 655.35kW)
    buffer[3]=0x25;           // 84F5 = 340.37kW  (7EA3 = 324.19) (25E4 = 97.00KW)
    Canbus.message_tx(buffer,id,length);

    id=546; // every 100ms      // 546 - BMS_chargerRequest
    length=6; // DBC says 7 bytes but the the 2018-4-9 log showed 6 bytes
    buffer[0]=(BMS_chargeCommand * 100) & 255;           // 16 bit chargeCommand BMS Commanded AC power
    buffer[1]=(BMS_chargeCommand * 100) >> 8;           // units 0.0001 of kW, 0BB8 = 3.000 kW
    buffer[2]=38880 & 255;           // 16 bit Pack Voltage Limit
    buffer[3]=38880 >> 8;           // units 0.01 of V, 97E0 = 388.80 volts
    buffer[4]=0xF0;           // units 0.16666, F0 = 40 amps
    buffer[5]=BMS_chargeEnable + BMS_chgVLimitMode;        // 4="BMS Clear Faults"  16="Tells the charger to either follow the BMS_chargeLimit or to track the pack voltage to prevent current spikes"  32="BMS Charge Enable"
    //buffer[6]=0x00;           // 0 "PT_FC_STATUS_NOTREADY_SNA" + 16*0 "PT_FC_TYPE_SUPERCHARGER"
    Canbus.message_tx(buffer,id,length);

    id=930; // every 100ms    // 930  BMS_chargeStatus: 8 BMS
    length=8;
    buffer[0]=0xFF;
    buffer[1]=0x0F;
    buffer[2]=0x06;
    buffer[3]=0x00;
    buffer[4]=0x00;
    buffer[5]=0x00;
    buffer[6]=0x00;
    buffer[7]=0x19;
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

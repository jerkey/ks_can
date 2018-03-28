// fuses must have lfuse bit 6 low, aka cleared, aka programmed, to enable CKOUT so that PE7 sends system clock to MCP2515 OSC1 input
// E:F5, H:DA, L:BF
#include "Canbus.h"
#include "mcp2515.h"
#include "zero.h"
byte buffer[16];
uint16_t command;
uint8_t num_modules;
uint16_t zero_control_tail = 0; // last two bytes of Canbus.zero_control() message
uint16_t cellVoltages[32];
uint32_t packVoltage = 0;
uint16_t id;
uint8_t length;
uint16_t x188count=0;
uint16_t x408count=0;
uint16_t x308count=0;
unsigned long lastCellPrint = 0; // last time we printed cell voltages

void setup(){
Serial.begin(230400);
Serial.println("CAN-Bus Demo");
for (int i=0; i<32; i++) cellVoltages[i] = 0; // zero battery voltages

if(Canbus.init(CANSPEED_500)) {
    Serial.println("CAN Init ok");
  } else {
    Serial.println("Can't init CAN");
  }
}

void loop(){
  send506(); // tell the battery what we want
  id=0;
  Canbus.message_rx(buffer,&id,&length);
  if (id == 0x0188) { if (x188count++ > 10) { x188count=0; // add ++ after first x???count to enable display
      Serial.print("SOC:");
      Serial.print(buffer[0]);
      Serial.print("  status:");
      printBMSStatusMessages(buffer[1]+buffer[2]*256);
      Serial.print("  charge cycles:");
      Serial.print(buffer[4]*256+buffer[3]);
      Serial.print("  balance mV:");
      Serial.print(buffer[6]*256+buffer[5]);
      Serial.print("  number of bricks:");
      Serial.print(buffer[7]);
      Serial.print("  command: 0x");
      Serial.print(command, HEX);
      Serial.print("  num_modules: ");
      Serial.print(num_modules);
      Serial.print("  time: ");
      Serial.println((float)millis() / 1000.0,2);
    }
  } else if (id == 0x0408) { if (x408count > 8) { x408count=0; // add ++ after first x???count to enable display
      Serial.print("Highest FET temp C:");
      Serial.print(buffer[0]);
      Serial.print("  Highest Pack Temp C:");
      Serial.print(buffer[1]);
      Serial.print("  Lowest Pack Temp C:");
      Serial.print(buffer[2]);
      Serial.print("  Pack Discharge Current Amps:");
      Serial.print(buffer[4]*256+buffer[3]);
      Serial.print("  Pack Capacity Remaining AH:");
      Serial.println(buffer[6]*256+buffer[5]);
    }
  } else if (id == 0x388 && buffer[0]<32) { // cell voltages won't fully populate unless 0x506 traffic is happening on the canbus
    cellVoltages[buffer[0]] = (buffer[2] << 8) + buffer[1];
    packVoltage = ((uint32_t)buffer[5] << 16) + ((uint16_t)buffer[4] << 8) + buffer[3];
    if (millis() - lastCellPrint > 60000) { // how much time between printing cell voltages
      lastCellPrint = millis();
      Serial.print("cell voltages: ");
      for (int i=0; i<28; i++) {
        if (cellVoltages[i]>0) {
        Serial.print((float)cellVoltages[i]/1000,2);
        Serial.print(" ");
        } else {
          Serial.print("-.-- ");
        }
      }
      Serial.print("  Pack voltage: ");
      Serial.println((float)packVoltage/1000,2);
    }
  } else if (id == 0x488) {// Serial.print("488 ");printBuf();
  } else if (id == 0x508) {// Serial.print("508 ");printBuf();
  } else if (id == 0x308) { if (x308count > 4) { x308count=0;
      Serial.print("BMS firmware Revision: ");
      Serial.print(buffer[0]);
      Serial.print("  BMS Board Revision: ");
      Serial.print(buffer[1]);
      Serial.print("  Run Time Seconds:");
      Serial.print(buffer[3]*256+buffer[2]);
      Serial.print("  Total Energy Used WH:");
      Serial.println(buffer[4]+buffer[5]*256+buffer[6]*65536+buffer[7]*16777216);
    }
  } else if (id == 0x288) {// Serial.print("288 ");printBuf();
  } else if (id) {
    Serial.print(" 0x");
    Serial.print(id,HEX);
  }
}

void send506() {
  if (millis() < 10300) {
    open_contactor();
  } else if (millis() < 10500) {
    close_contactor();
  } else {
    close_contactor2();
  }
}

void open_contactor() {
  tCAN m;
  m.id = 0x506;
  m.header.rtr = 0;
  m.header.length = 8;
  m.data[0] = 8;
  m.data[1] = 0x58;
  m.data[2] = 0x40;
  m.data[3] = 0;
  m.data[4] = 0x10;
  m.data[5] = 0;
  m.data[6] = 0xff;
  m.data[7] = 0xff;
  mcp2515_send_message(&m);
}

void close_contactor() {
  tCAN m;
  m.id = 0x506;
  m.header.rtr = 0;
  m.header.length = 8;
  m.data[0] = 8;
  m.data[1] = 0x68;
  m.data[2] = 0x40;
  m.data[3] = 0;
  m.data[4] = 0x10;
  m.data[5] = 0;
  m.data[6] = 0xff;
  m.data[7] = 0xff;
  mcp2515_send_message(&m);
}

void close_contactor2() {
  tCAN m;
  m.id = 0x506;
  m.header.rtr = 0;
  m.header.length = 8;
  m.data[0] = 8;
  m.data[1] = 0x68;
  m.data[2] = 0x40;
  m.data[3] = 1;
  m.data[4] = 0x10;
  m.data[5] = 0;
  m.data[6] = 0xff;
  m.data[7] = 0xff;
  mcp2515_send_message(&m);
}


void printBuf() {
  for (int i=0; i < length; i++) {
    if (buffer[i] < 16) Serial.print("0");
    Serial.print(buffer[i],HEX);
    Serial.print(" ");
  }
  Serial.println();
}

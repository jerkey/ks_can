// fuses must have lfuse bit 6 low, aka cleared, aka programmed, to enable CKOUT so that PE7 sends system clock to MCP2515 OSC1 input
// E:F5, H:DA, L:BF
#include "Canbus.h"
#include "mcp2515.h"
char UserInput;
int data;
byte buffer[16];
uint16_t cellVoltages[32];
uint32_t packVoltage = 0;
uint16_t id;
uint8_t length;
uint16_t x188count=0;
uint16_t x408count=0;
unsigned long lastCellPrint = 0; // last time we printed cell voltages

//********************************Setup Loop*********************************//

void setup(){
Serial.begin(230400);
Serial.println("CAN-Bus Demo");
for (int i=0; i<32; i++) cellVoltages[i] = 0; // zero battery voltages

if(Canbus.init(CANSPEED_500))  /* Initialise MCP2515 CAN controller at the specified speed */
  {
    Serial.println("CAN Init ok");
  } else {
    Serial.println("Can't init CAN");
  }
}

void loop(){
  id=0;
  Canbus.message_rx(buffer,&id,&length);
  if (id == 0x0188) { if (x188count > 6) { x188count=0;
      Serial.print("SOC:");
      Serial.print(buffer[0]);
      Serial.print("  status:");
      Serial.print(buffer[1],BIN);
      Serial.print(".");
      Serial.print(buffer[2],BIN);
      Serial.print("  charge cycles:");
      Serial.print(buffer[3]<<8+buffer[4]);
      Serial.print("  balance mV:");
      Serial.print(buffer[5]<<8+buffer[6]);
      Serial.print("  number of bricks:");
      Serial.println(buffer[7]);
    }
  } else if (id == 0x0408) { if (x408count > 4) { x408count=0;
      Serial.print("Highest FET temp C:");
      Serial.print(buffer[0]);
      Serial.print("  Highest Pack Temp C:");
      Serial.print(buffer[1]);
      Serial.print("  Lowest Pack Temp C:");
      Serial.print(buffer[2]);
      Serial.print("  Pack Discharge Current Amps:");
      Serial.print(buffer[3]<<8+buffer[4]);
      Serial.print("  Pack Capacity Remaining AH:");
      Serial.println(buffer[5]<<8+buffer[6]);
    }
  } else if (id == 0x388 && buffer[0]<32) {
    printBuf();
    cellVoltages[buffer[0]] = (buffer[2] << 8) + buffer[1];
    packVoltage = (buffer[5] << 16) + (buffer[4] << 8) + buffer[3];
    if (millis() - lastCellPrint > 1000) { // time to print cell voltages
      lastCellPrint = millis();
      Serial.print("cell voltages: ");
      for (int i=0; i<28; i++) {
        if (cellVoltages[i]>0) {
        Serial.print((float)cellVoltages[i]/1000,1);
        Serial.print(" ");
        } else {
          Serial.print("-.- ");
        }
      }
      Serial.print("  Pack voltage: ");
      //Serial.println((float)packVoltage/12800,2);
      Serial.println(packVoltage);
      Serial.print("cell indexes:   ");
      for (int i=0; i<28; i++) {
        if (i<10) Serial.print(" ");
        Serial.print(i);
        Serial.print("  ");
      }
      Serial.println();
    }
  } else if (id == 0x488) {// Serial.print("488 ");printBuf();
  } else if (id == 0x508) {// Serial.print("508 ");printBuf();
  } else if (id == 0x308) {// Serial.print("308 ");printBuf();
  } else if (id == 0x288) {// Serial.print("288 ");printBuf();
  } else if (id) {
    Serial.print(" 0x");
    Serial.print(id,HEX);
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

// sequence of commands when charger is connected
// 506     8       18      40      0       10      0       0       0
// 506     8       98      40      0       10      0       0       0
// 506     8       A8      41      1       10      0       0       0
//
#define ZERO_BMS_CONTROL_CODE_CLOSE_FET                 0x0001
#define ZERO_BMS_CONTROL_CODE_OPEN_FET                  0x0002
#define ZERO_BMS_CONTROL_CODE_CLOSE_CONTACTOR           0x0004
#define ZERO_BMS_CONTROL_CODE_OPEN_CONTACTOR            0x0008
#define ZERO_BMS_CONTROL_CODE_DISCONNECT_MODULE         0x0010
#define ZERO_BMS_CONTROL_CODE_CONNECT_MODULE            0x0020
#define ZERO_BMS_CONTROL_CODE_KEY_ON                    0x0040
#define ZERO_BMS_CONTROL_CODE_CHARGER_CONNECTED         0x0080
#define ZERO_BMS_CONTROL_CODE_CHARGING_ENABLED          0x0100
#define ZERO_BMS_CONTROL_CODE_INHIBIT_ISOLATION_TEST    0x0200
#define ZERO_BMS_CONTROL_CODE_NOT_SAFETY_OVERRIDE       0x4000
#define ZERO_BMS_CONTROL_CODE_SAFETY_OVERRIDE           0x8000

void printBMSStatusMessages(uint16_t status) {
  if (status & 0x0001) Serial.print("  CHARGER");
  if (status & 0x0002) Serial.print("  BATT_TEMP_TOO_HIGH");
  if (status & 0x0004) Serial.print("  BATT_TEMP_HIGH");
  if (status & 0x0008) Serial.print("  BATT_TEMP_TOO_LOW");
  if (status & 0x0010) Serial.print("  LOW_BATT");
  if (status & 0x0020) Serial.print("  CRITICAL_BATT");
  if (status & 0x0040) Serial.print("  IMBALANCE");
  if (status & 0x0080) Serial.print("  INTERNAL_FAULT");
  if (status & 0x0100) Serial.print("  FETS_CLOSED");
  if (status & 0x0200) Serial.print("  CONTACTOR_CLOSED");
  if (status & 0x0400) Serial.print("  ISOLATION_FAULT");
  if (status & 0x0800) Serial.print("  CELL_TOO_HIGH");
  if (status & 0x1000) Serial.print("  CELL_TOO_LOW");
  if (status & 0x2000) Serial.print("  CHARGE_HALT");
  if (status & 0x4000) Serial.print("  FULL");
  if (status & 0x8000) Serial.print("  INTERNAL_DISABLE");
}

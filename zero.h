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
  if (status & 0x0001) Serial.println("ZERO_BMS_STATUS_CODE_CHARGER");
  if (status & 0x0002) Serial.println("ZERO_BMS_STATUS_CODE_BATT_TEMP_TOO_HIGH");
  if (status & 0x0004) Serial.println("ZERO_BMS_STATUS_CODE_BATT_TEMP_HIGH");
  if (status & 0x0008) Serial.println("ZERO_BMS_STATUS_CODE_BATT_TEMP_TOO_LOW");
  if (status & 0x0010) Serial.println("ZERO_BMS_STATUS_CODE_LOW_BATT");
  if (status & 0x0020) Serial.println("ZERO_BMS_STATUS_CODE_CRITICAL_BATT");
  if (status & 0x0040) Serial.println("ZERO_BMS_STATUS_CODE_IMBALANCE");
  if (status & 0x0080) Serial.println("ZERO_BMS_STATUS_CODE_INTERNAL_FAULT");
  if (status & 0x0100) Serial.println("ZERO_BMS_STATUS_CODE_FETS_CLOSED");
  if (status & 0x0200) Serial.println("ZERO_BMS_STATUS_CODE_CONTACTOR_CLOSED");
  if (status & 0x0400) Serial.println("ZERO_CAN_BMS_STATUS_CODE_ISOLATION_FAULT");
  if (status & 0x0800) Serial.println("ZERO_CAN_BMS_STATUS_CODE_CELL_TOO_HIGH");
  if (status & 0x1000) Serial.println("ZERO_CAN_BMS_STATUS_CODE_CELL_TOO_LOW");
  if (status & 0x2000) Serial.println("ZERO_CAN_BMS_STATUS_CODE_CHARGE_HALT");
  if (status & 0x4000) Serial.println("ZERO_CAN_BMS_STATUS_CODE_FULL");
  if (status & 0x8000) Serial.println("ZERO_CAN_BMS_STATUS_CODE_INTERNAL_DISABLE");
}

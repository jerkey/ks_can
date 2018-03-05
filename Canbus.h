/**
 * CAN BUS
 *
 * Copyright (c) 2010 Sukkin Pang All rights reserved.
 */

#ifndef canbus__h
#define canbus__h

#define CANSPEED_125 	7		// CAN speed at 125 kbps
#define CANSPEED_250  	3		// CAN speed at 250 kbps
#define CANSPEED_500	1		// CAN speed at 500 kbps

class CanbusClass
{
  public:

	CanbusClass();
        char init(unsigned char);
	char message_tx(unsigned char *buffer, uint16_t *id, uint8_t *length);
	char message_rx(unsigned char *buffer, uint16_t *id, uint8_t *length);
private:

};
extern CanbusClass Canbus;
//extern tCAN message;

#endif

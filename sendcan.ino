/*
  OBDMPG - Display current speed, and gas mileage stats
  (instantaneous, past minute, trip) on serial LCD display.
  Uses MCP2515 library.
  
  Written by Frank Kienast in November, 2010.
  
  Modified by Frank Kienast in October, 2012 for Arduino 1.
*/

#include <SPI.h>
#include <MCP2515.h>

int secondCnt = 0;
int minuteCnt = 0;
double secondMafSum = 0.0;
double minuteMafSum = 0.0;
double secondVelSum = 0.0;
double minuteVelSum = 0.0;
double minuteMpg = 0.0, allMpg = 0.0;


void setup()
{
  Serial.begin(9600);
  
  Serial.print("Starting....");
  
  //Reset
  if(!MCP2515::initCAN(CAN_BAUD_500K))
    abort("Failed initCAN");

  //Set to normal mode non single shot
  if(!MCP2515::setCANNormalMode(LOW))
    abort("Failed CANNormalMode"); 
}

void loop()
{
  long val;
  double kmPerHr = 0.0, miPerHr = 0.0, maf = 0.0, mpg = 0.0;
  
  secondCnt = (secondCnt + 1) % 60;

  kmPerHr = (double) MCP2515::queryOBD(0x0d);
  miPerHr = 0.6214 * kmPerHr;
  maf = (double) (MCP2515::queryOBD(0x10));
  if(maf > 0)
    mpg = 710.7 * kmPerHr / maf;
  else
    mpg = 0;
  secondMafSum += maf;
  secondVelSum += kmPerHr;
    
  if(secondCnt == 0)
  {
    if(secondMafSum > 0)
      minuteMpg = 710.7 * secondVelSum / secondMafSum;
    else
      minuteMpg = 0;

    minuteCnt++;
    minuteMafSum += secondMafSum; secondMafSum = 0;
    minuteVelSum += secondVelSum; secondVelSum = 0;
    if(minuteMafSum > 0)
      allMpg = 710.7 * minuteVelSum / minuteMafSum;
    else
      allMpg = 0;
  }
    
  Serial.print(miPerHr,0); 
  Serial.print(mpg,1); Serial.print(" "); 
  Serial.print(minuteMpg,1); Serial.print(" "); 
  Serial.print(allMpg,1);
  
  delay(975);

}

void abort(char *msg)
{
  Serial.print(msg);

  while(true);
}

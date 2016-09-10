/* David Naylor    Ultrasonic Ranging for Model Helicopters
 *
 * SRF10
 */

//#define DEBUG

#include "srf10.h"

extern "C" {
#include "../lib/i2c.h"
#include "../lib/myprint.h"
#include "../lib/timer.h"
}

SRF10::SRF10(tU16 addr)
{
  address = addr;
  rangeData = 0;
  newData = false;
  rangeSetting = 255;
  gainSetting = 16;
}

void SRF10::doRangeMs()
{
  char rangeCommand[] = {0x00,0x52};
  i2c0Write(address, rangeCommand, 2);

#ifdef DEBUG
  printS("ultrasonic - ms range command sent to: 0x");
  printX(address);
  printC('\n');
#endif
}

void SRF10::doRangeCm() 
{
  char rangeCommand[] = {0x00,0x51};
  i2c0Write(address, rangeCommand, 2);

#ifdef DEBUG
  printS("ultrasonic - cm range command sent to: 0x");
  printX(address);
  printC('\n');
#endif
}

void SRF10::doRangeInch() 
{
  char rangeCommand[] = {0x00,0x50};
  i2c0Write(address, rangeCommand, 2);

#ifdef DEBUG
  printS("ultrasonic - inch range command sent to: 0x");
  printX(address);
  printC('\n');
#endif
}

void SRF10::doRequestRangeData() 
{
  char regAddr = 0x00; 
  i2c0Write(address,&regAddr, 1);

#ifdef DEBUG
  printS("ultrasonic - read request sent to: 0x");
  printX(address);
  printC('\n');
#endif
}

void SRF10::doReadRangeData()
{
  i2c0Read(address, 4);

#ifdef DEBUG
  printS("ultrasonic - reading range data for: 0x");
  printX(address);
  printC('\n');
#endif
}

void SRF10::doPoll()
{
  i2c0Read(address, 1);
}

bool SRF10::hasReading()
{
  return i2c0GetData(0) != 0xFF;
}

void SRF10::doUpdateRangeData() {
  rangeData = (i2c0GetData(2) << 8) + i2c0GetData(3);
  newData = true;

#ifdef DEBUG
  printS("ultrasonic - updating range data for: 0x");
  printX(address);
  printC('\n');
#endif
}

void SRF10::changeAddressS1()
{
  char addrCommand[] = {0x00, 0xA0};
  i2c0Write(address,addrCommand, 2);

#ifdef DEBUG
  printS("ultrasonic - change address sequence 1 for: 0x");
  printX(address);
  printC('\n');
#endif
}

void SRF10::changeAddressS2()
{
  char addrCommand[] = {0x00, 0xAA};
  i2c0Write(address,addrCommand, 2);

#ifdef DEBUG
  printS("ultrasonic - change address sequence 2 for: 0x");
  printX(address);
  printC('\n');
#endif
}

void SRF10::changeAddressS3()
{
  char addrCommand[] = {0x00, 0xA5};
  i2c0Write(address,addrCommand, 2);

#ifdef DEBUG
  printS("ultrasonic - change address sequence 3 for: 0x");
  printX(address);
  printC('\n');
#endif
}

void SRF10::setNewAddress(tU16 newAddress)
{
  char addrCommand[2];
  addrCommand[0] = 0x00;
  addrCommand[1] = newAddress;
  i2c0Write(address,addrCommand, 2);

#ifdef DEBUG
  printS("ultrasonic - changed i2c address from: 0x");
  printX(address);
  printS(" to: 0x");
  printX(newAddress);
  printC('\n');
#endif

  address = newAddress;
}

void SRF10::setMaxRange(tU8 rangeValue)
{
#ifdef DEBUG
  printS("ultrasonic - changing max range for: 0x");
  printX(address);
  printS(" to: ");
  printN(rangeValue);
  printC('\n');
#endif
  
  char rangeCommand[2];
  rangeCommand[0] = 0x02;
  rangeCommand[1] = rangeValue;
  i2c0Write(address, rangeCommand, 2);
  rangeSetting = rangeValue;
}

void SRF10::setGain(tU8 gainValue)
{
#ifdef DEBUG
  printS("ultrasonic - changing gain for: 0x");
  printX(address);
  printS(" to: ");
  printN(gainValue);
  printC('\n');
#endif

  char gainCommand[2];
  gainCommand[0] = 0x01;
  gainCommand[1] = gainValue;
  i2c0Write(address, gainCommand, 2);
  gainSetting = gainValue;
}

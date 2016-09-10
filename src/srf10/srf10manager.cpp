#include "srf10manager.h"
#include "../startup/config.h"

extern "C" {
#include "../lib/myprint.h"
#include "../lib/timer.h"
}

SRF10Manager* SRF10Manager::manager[SRF10_DEVICES];
tU8 SRF10Manager::count = 0;
//tU32 SRF10Manager::minFireDelay = MS_DELAY * 100;
tU32 SRF10Manager::minFireDelay = 1000000;
tU32 SRF10Manager::minFireDiff = 1000;
bool SRF10Manager::useDynamicTiming = false;
tU32 SRF10Manager::globalTime = 0;

SRF10Manager::SRF10Manager(SRF10 *s, tU8 n)
{
  srf10 = s;
  name = n;
  state = RANGE;
  index = count++;
  manager[index] = this;
}

void SRF10Manager::resetAll()
{
  globalTime = 0;
  for (int i = 0; i < count; i++) {
    manager[i]->reset();
  }
}

void SRF10Manager::reset() {
  for (int i = 0; i < count; i++) {
    lastFireTime[i] = 0;
    lastTimeDiff[i] = 0;
  }
  offsetTime = 0;
  lastFirerIndex = index;
  state = RANGE;
}

void SRF10Manager::notifyAllFiring(const tU8 &firerIndex, const tU32 &fireTime)
{
  for (int i = 0; i < count; i++) {
    manager[i]->notifyFiring(firerIndex, fireTime);
  }
}

void SRF10Manager::notifyFiring(const tU8 &firerIndex, const tU32 &fireTime)
{
  lastFireTime[firerIndex] = offsetTime + fireTime;
  lastFirerIndex = firerIndex;
}

void SRF10Manager::notifyAllTimerReset()
{
  tU32 currentTime = timer0Time();
  globalTime += currentTime;
  for (int i = 0; i < count; i++) {
    manager[i]->addTimeOffset(currentTime);
  }
}

void SRF10Manager::onBusSuccess()
{
#ifdef DEBUG
  printS("manager - i2c received for: 0x");
  printX(address);
  printC('\n');
#endif
  switch (state) {
  case RANGE:
    state = POLL;
    break;
  case POLL:
    if (srf10->hasReading()) {
      state = REQUEST;
    }
    break;
  case REQUEST:
    state = READ;
    break;
  case READ:
    state = RANGE;
    srf10->doUpdateRangeData();

    printC('~');
    printC(name);
    printNumber(10,7,FALSE,'0',srf10->getRangeData());
    printC('\n');
   
    break;
  }
}

void SRF10Manager::doNextAction()
{
#ifdef DEBUG
  printS("manager - run ranging state: ");
  printX(state);
  printC('\n');
#endif
  switch (state) {
  case RANGE:
    printC('/');
    printC(name);
    //printS("-x\n");
    printNumber(10,10,FALSE,'0',globalTime + timer0Time());
    printC('\n');

    srf10->doRangeMs();
    break;
  case POLL:
    //printC(name);
    //printS("-p\n");
    srf10->doPoll();
    break;
  case REQUEST:
    //printC(name);
    //printS("-y\n");
    srf10->doRequestRangeData();
    break;
  case READ:
    //printC(name);
    //printS("-z\n");
    srf10->doReadRangeData();
    break;
  }
}

tU32 SRF10Manager::calculateFireTime()
{
  tU32 nextFireTime;

  tU32 timePassed = timer0Time() + offsetTime - lastFireTime[lastFirerIndex];
  if (timePassed >= minFireDelay) {
    nextFireTime = 1;
  } else {
    nextFireTime = minFireDelay - timePassed;
  }

  /*
  printC(name);
  printC('\n');

  printS("offset ");
  printNumber(10,10,FALSE,'0',offsetTime);
  printS("\n");

  printS("last fire time ");
  printNumber(10,10,FALSE,'0',lastFireTime[lastFirerIndex]);
  printS("\n");
  
  for (int i = 0; i < count; i++) {
    printC(manager[i]->getName());
    printNumber(10,10,FALSE,'0',lastFireTime[i]);
    printC(',');
  }
  printC('\n');
  
  for (int i = 0; i < count; i++) {
    printC(manager[i]->getName());
    printNumber(10,10,FALSE,'0',lastTimeDiff[i]);
    printC(',');
  }
  printS("\n\n");
  */

  if (useDynamicTiming) {
    bool timeFound = false;
    while (!timeFound) {
      timeFound = true;
      for (int i = 0; i < count; i++) {
	tU32 currentTime = offsetTime + timer0Time();
	tU32 newDiff = currentTime + nextFireTime - lastFireTime[i];
	if (newDiff + minFireDiff > lastTimeDiff[i]
	    && lastTimeDiff[i] + minFireDiff > newDiff) {
	  timeFound = false;
	  nextFireTime = lastTimeDiff[i] + minFireDiff + lastFireTime[i] - currentTime;
	  break;
	}
      }
    }
  }

  tU32 currentTime = offsetTime + timer0Time();
  tU32 minFireTime = 0xFFFFFFFF;
  for (int i = 0; i < count; i++) {
    lastTimeDiff[i] = currentTime + nextFireTime - lastFireTime[i];
    if (lastFireTime[i] < minFireTime) {
      minFireTime = lastFireTime[i];
    }
  }

  for (int i = 0; i < count; i++) {
    lastFireTime[i] -= minFireTime;
  }
  offsetTime -= minFireTime;

  notifyAllFiring(index, nextFireTime + timer0Time());
  return nextFireTime;
}



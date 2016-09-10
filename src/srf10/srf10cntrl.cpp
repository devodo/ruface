/* David Naylor    Ultrasonic Ranging for Model Helicopters
 *
 * SRF10 Controller
 */

#include "srf10cntrl.h"
#include "../startup/config.h"

extern "C" {
#include "../lib/myprint.h"
#include "../lib/timer.h"
}

#define ERROR

SRF10Controller* SRF10Controller::controller[SRF10_DEVICES];
tU32 SRF10Controller::lastFireTime[SRF10_DEVICES];
tU8 SRF10Controller::count = 0;
tU32 SRF10Controller::minFireDelay = 1000000;
tU32 SRF10Controller::minDelayDiff = 0;
bool SRF10Controller::useDynamicTiming = false;
tU32 SRF10Controller::globalTime = 0;
tU32 SRF10Controller::offsetTime = 0;
tU8 SRF10Controller::lastFirerIndex = 0;

SRF10Controller::SRF10Controller(SRF10 *s, tU8 n)
{
  srf10 = s;
  name = n;
  state = RANGE;
  index = count++;
  controller[index] = this;
}

void SRF10Controller::resetAll()
{
  globalTime = 0;
  offsetTime = 0;
  lastFirerIndex = 0;
  for (int i = 0; i < count; i++) {
    lastFireTime[i] = 0;
    controller[i]->reset();
  }
}

void SRF10Controller::reset() {
  for (int i = 0; i < count; i++) {
    lastDelay[i] = 0;
  }
  state = RANGE;
}

void SRF10Controller::notifyFiring(const tU8 &firerIndex, const tU32 &fireTime)
{
  lastFireTime[firerIndex] = fireTime;
  lastFirerIndex = firerIndex;
}

void SRF10Controller::trimOffsetTime()
{
  tU32 minTime = offsetTime;
  for (int i = 0; i < count; i++) {
    if (lastFireTime[i] < minTime) {
      minTime = lastFireTime[i];
    }
  }
  
  for (int i = 0; i < count; i++) {
    lastFireTime[i] -= minTime;
  }
  offsetTime -= minTime;
}

void SRF10Controller::notifyTimerReset()
{
  tU32 currentTime = timer0Time();
  globalTime += currentTime;
  offsetTime += currentTime;
}

void SRF10Controller::onBusSuccess()
{
#ifdef DEBUG
  printS("controller - i2c received for: 0x");
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

void SRF10Controller::doNextAction()
{
#ifdef DEBUG
  printS("controller - run ranging state: ");
  printX(state);
  printC('\n');
#endif
  switch (state) {
  case RANGE:
    printC('/');
    printC(name);
    printNumber(10,10,FALSE,'0',globalTime + timer0Time());
    printC('\n');

    srf10->doRangeMs();
    break;
  case POLL:
    srf10->doPoll();
    break;
  case REQUEST:
    srf10->doRequestRangeData();
    break;
  case READ:
    srf10->doReadRangeData();
    break;
  }
}

tU32 SRF10Controller::calculateFireTime()
{

  trimOffsetTime();

#ifdef DEBUG
  printC(name);
  printC('\n');

  printS("global time ");
  printNumber(10,10,FALSE,'0',globalTime);
  printS("\n");

  printS("offset ");
  printNumber(10,10,FALSE,'0',offsetTime);
  printS("\n");

  for (int i = 0; i < count; i++) {
    printC(controller[i]->getName());
    printNumber(10,10,FALSE,'0',lastFireTime[i]);
    printC(',');
  }
  printC('\n');
  
  for (int i = 0; i < count; i++) {
    printC(controller[i]->getName());
    printNumber(10,10,FALSE,'0',lastDelay[i]);
    printC(',');
  }
  printS("\n");
#endif

  // Calculate the nextFireTime according to the minFireDelay constraint
  tU32 nextFireTime;
  tU32 currentTime = offsetTime + timer0Time();
  // Subtract any time from the min delay that has already passed
  // since the last sensor fired
  tU32 timePassed = currentTime - lastFireTime[lastFirerIndex];
  if (timePassed >= minFireDelay) {
    nextFireTime = currentTime; // already passed min delay so fire asap
  } else {
    nextFireTime = currentTime + minFireDelay - timePassed;
  }

#ifdef DEBUG
  printS("current time ");
  printNumber(10,10,FALSE,'0',currentTime);
  printS("\n");

  printS("next time ");
  printNumber(10,10,FALSE,'0',nextFireTime);
  printS("\n");
#endif

  // If the dynamic timing algorithm is enabled,
  // determine the minimum nextFireTime that satisfy all
  // time differential constraints
  if (useDynamicTiming) {

    // Calculate and store all the maximum, acceptable times
    static tU32 possibleTime[SRF10_DEVICES];
    for (int i = 0; i < count; i++) {
      possibleTime[i] = lastDelay[i] +
	lastFireTime[i] - minDelayDiff;
    }

    tU8 index;
    bool nextTimeUpdated = true;
    // Loop if nextFireTime gets updated
    while (nextTimeUpdated) {
      nextTimeUpdated = false;
      for (index = 0; index < count; index++) {
	// If this minimum time is still being considered
	if (possibleTime[index] != 0) {
	  // Check if the min time is not acceptable
	  if (nextFireTime > possibleTime[index]) {
	    // We have passed the allowable maximum time so now we must ensure
	    // nextFireTime occurs after the minimum acceptable time
	    possibleTime[index] = lastDelay[index] +
	      lastFireTime[index] + minDelayDiff;
	    // Check if nextFireTime needs to be increased
	    if (nextFireTime < possibleTime[index]) {
	      nextFireTime = possibleTime[index];
	      // The nextFireTime has been updated so we must recheck all 
	      // remaining maximum times by setting this flag to true
	      nextTimeUpdated = true;
	    }
	    // This maximum time no longer need to be considered
	    possibleTime[index] = 0;
	  }
	}
      }
    }
#ifdef DEBUG
    printS("dynamic next time ");
    printNumber(10,10,FALSE,'0',currentTime);
    printS("\n");
#endif
  }

  // Save the new time differences for next time
  for (int i = 0; i < count; i++) {
    lastDelay[i] = nextFireTime - lastFireTime[i];
  }

  // Let everyone know when we are going to fire
  notifyFiring(index, nextFireTime);
  // Get the time to delay before firing by subtracting
  // the current time from the calculated nextFireTime
  return (nextFireTime - currentTime);
}



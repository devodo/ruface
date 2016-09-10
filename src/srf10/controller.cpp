//#define DEBUG

#include "controller.h"
#include "../startup/config.h"

extern "C" {
#include "../lib/myprint.h"
#include "../lib/timer.h"
}

const tU32 Controller::FIRE_PAUSE = 
 (10 * CRYSTAL_FREQUENCY * PLL_FACTOR)/ (10000 * VPBDIV_FACTOR);

Controller::Controller()
{
  printNumber(10,10,FALSE,'0',FIRE_PAUSE);
  printC('\n');
  reset();
}

void Controller::reset()
{
  timer0DisableMRInt();
  timerQ.clear();
  busQ.clear();
  busUser = 0;
  timerUser = 0;
  busReady = true;
  busPaused = false;
  busSuccess = false;
  timerReady = false;
}

void Controller::i2cCallBack(tU8 status)
{
#ifdef DEBUG
  printS("controller - i2c callback with status: ");
  printNumber(10,3,FALSE,'0',status);
  printC('\n');
#endif
  /*
    printC(busUser->getName());
    printNumber(10,3,FALSE,'0',status);
    printC('\n');
  */
  switch (status) {
  case 32:
  case 48:
  case 72:
    busSuccess = false;
    break;
  case 40:
  case 88: 
    busSuccess = true;
    break;
  default:
    printS("controller i2c callback error: unrecognised state - ");
    printNumber(10,3,FALSE,'0',status);
    printC('\n');
    busSuccess = false;
    break;
  }
  busReady = true;
}

void Controller::timerCallBack()
{
  //printS("Time\n");
  timerReady = true;
}

void Controller::executeNext()
{
  checkTimerQueue();
  checkBusQueue();
}

void Controller::checkTimeout()
{
  // Check for timeout and reset
  if (!busReady) {
    resetCounter++;
    if (resetCounter == 0) {
      printS("controller - reset counter overflow, resetting bus ready\n");
      busReady = true;
      busSuccess = false;
    }
  } else {
    resetCounter = 0; //reset timeout counter
  }
}

void Controller::checkTimerQueue()
{
  if (timerReady) {
    timerReady = false;
    if (timerUser) {
      // add srf10 to front of busQ
      busQ.queueFront(timerUser);
      //set timer interupt for next waiting srf10
    } else {
      busPaused = false;
      if (!timerQ.isEmpty()) {
	timerQ.popFront(timerUser);
	tU32 delayTime = timerUser->calculateFireTime();
	SRF10Manager::notifyAllTimerReset();
	timer0setMR0Int(delayTime);
      } else {
	timer0DisableMRInt();
      }
    }
  }
}

void Controller::checkBusQueue()
{
  if (busReady && !busPaused) {
    
    if(busUser) {
      if (busSuccess) {
	// Notify previous manager that bus command was successfull
	busSuccess = false;
	busUser->onBusSuccess();
      } 
      
      if (busUser->getState() == RANGE) {
	queueForTimer(busUser);
      } else {
	busQ.enqueue(busUser);
      }
      busUser = 0;
    }
    
    if (!busQ.isEmpty()) {
      busReady = false;
      SRF10Manager *bM;
      busQ.popFront(bM);
      busUser = bM;
      bM->doNextAction();
      if (bM->getState() == RANGE) {
	busPaused = true;
	timerUser = 0;
	SRF10Manager::notifyAllTimerReset();
	timer0setMR0Int(FIRE_PAUSE);
      }
    }
  }
}

void Controller::queueForTimer(SRF10Manager *manager)
{
  bool wasEmpty = timerQ.isEmpty();
  timerQ.enqueue(manager);
  if (wasEmpty && !timerUser) {
    timerReady = true;
  }
}

/* David Naylor    Ultrasonic Ranging for Model Helicopters
 *
 * Q Manager
 */

//#define DEBUG
#define ERROR

#include "qmanager.h"
#include "../startup/config.h"

extern "C" {
#include "../lib/myprint.h"
#include "../lib/timer.h"
}

const tU32 QManager::FIRE_PAUSE = 
 (CRYSTAL_FREQUENCY * PLL_FACTOR)/ (800 * VPBDIV_FACTOR);

QManager::QManager()
{
  printNumber(10,10,FALSE,'0',FIRE_PAUSE);
  printC('\n');
  reset();
}

void QManager::reset()
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

void QManager::i2cCallBack(tU8 status)
{

#ifdef DEBUG
  printS("qmanager - i2c callback with status: ");
  printNumber(10,3,FALSE,'0',status);
  printC('\n');
#endif

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
#ifdef ERROR
    printS("qmanager i2c callback error: unrecognised state - ");
    printNumber(10,3,FALSE,'0',status);
    printC('\n');
#endif
    busSuccess = false;
    break;
  }
  busReady = true;
}

void QManager::timerCallBack()
{

#ifdef DEBUG
  printS("qmanager - received timer interrupt\n");
#endif

  timerReady = true;
}

void QManager::executeNext()
{

#ifdef DEBUG
  printS("qmanager - execute next step\n");
#endif

  checkTimerQueue();
  checkBusQueue();
}

void QManager::checkTimeout()
{
  // Check for timeout and reset
  if (!busReady) {
    resetCounter++;
    if (resetCounter == 0) {
      printS("qmanager - reset counter overflow, resetting bus ready\n");
      busReady = true;
      busSuccess = false;
    }
  } else {
    resetCounter = 0; //reset timeout counter
  }
}

void QManager::checkTimerQueue()
{
  if (timerReady) {
    timerReady = false;
    if (timerUser) {
      // srf10 controller wants to fire so add to front of busQ
      busQ.queueFront(timerUser);
    } else {
      busPaused = false;
      if (!timerQ.isEmpty()) {
	//set timer interupt for next waiting srf10 controller
	timerQ.popFront(timerUser);
	// ask the srf10 controller how long it wants to wait
	tU32 delayTime = timerUser->calculateFireTime();
	// If the required delay is 0 then bypass timer interrupt by calling
	if (delayTime == 0) {
	  timerCallBack(); // the function that gets called on timer interrupt
	} else {
	  // notify the srf10 controllers that the timer is getting reset
	  // so they can adjust their timer offset accordingly
	  SRF10Controller::notifyTimerReset();
	  // schedule interrupt and reset timer #0
	  // timer #0 reset to 0 to prevent counter overflow
	  timer0setMR0Int(delayTime);
	}
      } else {
	timer0DisableMRInt(); //disable timer #0 interrupts
      }
    }
  }
}

void QManager::checkBusQueue()
{
  // busReady set to true on i2c #0 stop condition interrupts
  // busPaused set to false on next time interrupt
  if (busReady && !busPaused) {
    
    if(busUser) { // the srf10 controller that issued the i2c command 
      if (busSuccess) {
	busSuccess = false;
	// Notify previous controller that the i2c command was successfull
	busUser->onBusSuccess();
      } 
      // Put the srf10 controller back on the right queue
      // if the controller's next command is to fire then
      // queue for timer, else add back to busQ
      if (busUser->getState() == RANGE) {
	queueForTimer(busUser);
      } else {
	busQ.enqueue(busUser);
      }
      busUser = 0; // srf10 controller has finished using the bus
    }
    
    if (!busQ.isEmpty()) { // there is an srf10 controller waiting
      busReady = false;
      busQ.popFront(busUser); // set bus user as next srf10 controller
      busUser->doNextAction(); // send i2c command

      // After an srf10 has fired the i2c bus must not be used
      // for a small amount of time else the bus will crash.
      // It is therefore necessary to introduce a pause after
      // any srf10 fires.
      if (busUser->getState() == RANGE) {
	busPaused = true;
	// The last srf10 manager has finished using the timer
	timerUser = 0;
	// Notify the srf10 controllers that the timer is going to
	// be reset so they can adjust their timer offset
	SRF10Controller::notifyTimerReset();
	// schedule interrupt and reset timer #0
	timer0setMR0Int(FIRE_PAUSE);
      }
    }
  }
}

void QManager::queueForTimer(SRF10Controller *srf10Cntrl)
{
  bool wasEmpty = timerQ.isEmpty();
  timerQ.enqueue(srf10Cntrl);
  // If the timer queue is empty and the timer is not being used
  // then restart processing.
  if (wasEmpty && !timerUser) {
    timerReady = true;
  }
}

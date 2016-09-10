#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "../lib/datatypes.h"
#include "srf10manager.h"
#include "../staticqueue.h"

class Controller {
 public:
  static const tU32 FIRE_PAUSE;

  Controller();
  void i2cCallBack(tU8 status);
  void timerCallBack();
  bool isBusReady() { return busReady; };
  void executeNext();
  void queueForTimer(SRF10Manager *);
  void reset();

 private:
  void checkTimeout();
  void checkTimerQueue();
  void checkBusQueue();

  volatile bool busReady;
  volatile bool busSuccess;
  volatile bool busPaused;
  volatile bool timerReady;

  tU16 resetCounter;
  SRF10Manager *busUser;
  SRF10Manager *timerUser;
  StaticQueue<SRF10Manager *> busQ;
  StaticQueue<SRF10Manager *> timerQ;
};
#endif

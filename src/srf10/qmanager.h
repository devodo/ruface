/* David Naylor    Ultrasonic Ranging for Model Helicopters
 *
 * Q Manager
 */

#ifndef QMANAGER_H
#define QMANAGER_H

#include "../lib/datatypes.h"
#include "staticqueue.h"
#include "srf10cntrl.h"

class QManager {
 public:
  static const tU32 FIRE_PAUSE;
  QManager();
  void i2cCallBack(tU8 status);
  void timerCallBack();
  bool isBusReady() { return busReady; };
  void executeNext();
  void queueForTimer(SRF10Controller *);
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
  SRF10Controller *busUser;
  SRF10Controller *timerUser;
  StaticQueue<SRF10Controller *> busQ;
  StaticQueue<SRF10Controller *> timerQ;
};

#endif

/* David Naylor    Ultrasonic Ranging for Model Helicopters
 *
 * Command Handler
 */

#ifndef CMMDHANDLER_H
#define CMMDHANDLER_H

#include "srf10cntrl.h"
#include "qmanager.h"

class CommandHandler {
 public:
  CommandHandler(QManager *qMngr, SRF10Controller *cntrl, tU8 count);
  void checkForRequest();
  void sendInitInfo();

 private:
  void start();
  void stop();
  void resetSRF10s();
  void startIndividual();
  void setDelay();
  void setMinDiff();
  void setPWM();
  void setDynamicTiming();
  void setGain();
  void setMaxRange();

  tU8 deviceCount;
  QManager *qManager;
  SRF10Controller *controller;
};

#endif

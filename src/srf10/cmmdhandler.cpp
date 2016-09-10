/* David Naylor    Ultrasonic Ranging for Model Helicopters
 *
 * Command Handler
 */

#include "cmmdhandler.h"

#include "../lib/datatypes.h"
#include "../startup/config.h"

extern "C" {
#include "../lib/uart.h"
#include "../lib/myprint.h"
#include "../lib/timer.h"
#include "../lib/gpio.h"
#include "../lib/pwm.h"
}

CommandHandler::CommandHandler(QManager *qMngr, SRF10Controller *cntrl, tU8 count)
{
  qManager = qMngr;
  controller = cntrl;
  deviceCount = count;
}

void CommandHandler::checkForRequest()
{
  tU8 uartIn;

  if (uart0GetChar(&uartIn)) {
    delayMs(10);
    printS("\nack\n");

    switch(uartIn) {
    case 'd':
      setDelay();
      break;
    case 'v':
      setMinDiff();
      break;
    case 'r':
      resetSRF10s();
      break;
    case 's':
      start();
      break;
    case 'x':
      stop();
      break;
    case 'o':
      startIndividual();
      break;
    case 'p':
      setPWM();
      break;
    case 'i':
      sendInitInfo();
      break;
    case 'f':
      setDynamicTiming();
      break;
    case 'g':
      setGain();
      break;
    case 'w':
      setMaxRange();
    default:
      break;
    }
    printS("\nCommand Success\n");
  }
}

void CommandHandler::start()
{
  qManager->reset();
  SRF10Controller::resetAll();
  for (int i = 0; i < deviceCount; i++) {
    qManager->queueForTimer(&controller[i]);
  }

  pin30High();
  delayMs(500);
  timer0Reset();
}

void CommandHandler::stop()
{
  qManager->reset();
  SRF10Controller::resetAll();
  pin30Low();
}

void CommandHandler::resetSRF10s()
{
  qManager->reset();
  SRF10Controller::resetAll();
  for (int i = 0; i < deviceCount; i++) {
    qManager->queueForTimer(&controller[i]);
  }

  timer0Reset();
}

void CommandHandler::startIndividual()
{
  tU8 count = uart0GetCh();

  qManager->reset();
  SRF10Controller::resetAll();

  tU8 name;
  for (int j = 0; j < count; j++) {
    name = uart0GetCh();
    for (int i = 0; i < deviceCount; i++) {
      if (controller[i].getName() == name) {
	qManager->queueForTimer(&controller[i]);
      }
    }
  }

  pin30High();
  delayMs(500);
  timer0Reset();
}

void CommandHandler::setDelay()
{
  tU8 uartIn;
  tU32 delay = 0;
  tU8 count = uart0GetCh();

  for (int i = 0; i < count; i++) {
    uartIn = uart0GetCh();
    delay += uartIn << (i*8);
  }

  delay = ((CRYSTAL_FREQUENCY * PLL_FACTOR)/
	   (1000 * VPBDIV_FACTOR / delay));
  SRF10Controller::setMinDelay(delay);
}

void CommandHandler::setMinDiff()
{
  tU8 uartIn;
  tU32 delayDiff = 0;
  tU8 count = uart0GetCh();
  
  for (int i = 0; i < count; i++) {
    uartIn = uart0GetCh();
    delayDiff += uartIn << (i*8);
  }

  delayDiff = ((CRYSTAL_FREQUENCY * PLL_FACTOR) /
	  (10000 * VPBDIV_FACTOR / delayDiff));
  SRF10Controller::setMinDelayDiff(delayDiff);
}

void CommandHandler::setPWM()
{
  tU8 uartIn;
  tU32 dutyValue = 0;
  tU8 count = 0;

  count = uart0GetCh();

  for (int i = 0; i < count; i++) {
    uartIn = uart0GetCh();
    dutyValue += uartIn << (i*8);
  }

  dutyValue = ((CRYSTAL_FREQUENCY * PLL_FACTOR)/ (VPBDIV_FACTOR * dutyValue));
  setPwmDuty(dutyValue);
}

void CommandHandler::sendInitInfo()
{
  SRF10 *srf10;

  printS("startInit\n");

  for (int i = 0; i < deviceCount; i++) {
    srf10 = controller[i].getSRF10();
    printS("srf10 ");
    printC(controller[i].getName());
    printS(" ");
    printNumber(10,5,FALSE,'0',srf10->getAddress());
    printS(" ");
    printNumber(10,3,FALSE,'0',srf10->getMaxRange());
    printS(" ");
    printNumber(10,3,FALSE,'0',srf10->getGain());
    printC('\n');
  }

  printS("min_delay ");
  tU32 delay = (((SRF10Controller::getMinDelay() * 1000) / CRYSTAL_FREQUENCY) *
		(VPBDIV_FACTOR / PLL_FACTOR));
  printNumber(10,10,FALSE,'0',delay);
  printS("\nmin_diff ");
  tU32 diff = (((SRF10Controller::getMinDelayDiff() * 10000) / CRYSTAL_FREQUENCY) *
	       (VPBDIV_FACTOR / PLL_FACTOR));
  printNumber(10,10,FALSE,'0',diff);
  printS("\nuse_dynamic ");
  if (SRF10Controller::usingDynamicTiming()) {
    printC('1');
  } else {
    printC('0');
  }
  printS("\nendInit\n");
}

void CommandHandler::setDynamicTiming()
{
  tU8 onOff = uart0GetCh();
  if (onOff == 1) {
    printS("Dynamic On\n");
    SRF10Controller::setDynamicTiming(true);
  } else {
    printS("Dynamic Off\n");
    SRF10Controller::setDynamicTiming(false);
  }
}

void CommandHandler::setGain()
{
  tU8 name = uart0GetCh();
  tU8 gain = uart0GetCh();

  for (int i = 0; i < deviceCount; i++) {
    delayMs(100);
    if (name == 'a') {
      controller[i].getSRF10()->setGain(gain);
    } else if (controller[i].getName() == 'a') {
      controller[i].getSRF10()->setGain(gain);
    }
  }
}

void CommandHandler::setMaxRange()
{
  tU8 name = uart0GetCh();
  tU8 range = uart0GetCh();

  for (int i = 0; i < deviceCount; i++) {
    delayMs(100);
    if (name == 'a') {
      controller[i].getSRF10()->setMaxRange(range);
    } else if (controller[i].getName() == 'a') {
      controller[i].getSRF10()->setMaxRange(range);
    }
  }
}

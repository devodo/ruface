/* David Naylor    Ultrasonic Ranging for Model Helicopters
 *
 * Main Loop
 */

#include "lib/datatypes.h"
#include "startup/config.h"
#include "srf10/srf10.h"
#include "srf10/srf10cntrl.h"
#include "srf10/qmanager.h"
#include "srf10/cmmdhandler.h"

extern "C" {
#include "lib/uart.h"
#include "lib/i2c.h"
#include "lib/irq/irq.h"
#include "lib/myprint.h"
#include "lib/timer.h"
#include "lib/gpio.h"
#include "lib/pwm.h"
}

/******************************************************************************
 * C -> C++ interrupt callback bridging functions
 *****************************************************************************/
QManager *qManagerPtr;

// prototypes
void i2c0CallBack(tU8 status);
void timer0CallBack();

// C -> C++ i2c #0 interrupt callback bridge
// All i2c #0 stop condition interrupts are forwarded to the queuemanager
void i2c0CallBack(tU8 status)
{
  qManagerPtr->i2cCallBack(status);
}

//C -> C++ timer #0 interrupt callback bridge
// All timer #0 interrupts are forwarded to the queuemanager
void timer0CallBack()
{
  qManagerPtr->timerCallBack();
}

/******************************************************************************
 * The main function
 *****************************************************************************/
int main()
{
  // Initialise pin 30 for GPIO
  initPin30();
  // Set pin 30 high to turn on SRF10s
  pin30High();
  // Initialise PWM #2 and set frequency to 1000 Hz (1 kHz)
  initPwm((CRYSTAL_FREQUENCY * PLL_FACTOR)/ (VPBDIV_FACTOR * 50));
  // Initialise UART #0: 115.2 kbps, 8N1, no FIFO
  initUart0(B115200((CRYSTAL_FREQUENCY * PLL_FACTOR) / VPBDIV_FACTOR),
	    UART_8N1, UART_FIFO_OFF);
   // Initialise the i2c #0 bus to 400000 Hz (400 kHz)
  i2c0Init(400000);

  // Create the SRF10 object instances
  SRF10 srf10Front((tU16)0xE6);
  SRF10 srf10Back((tU16)0xF2);
  SRF10 srf10Left((tU16)0xEE);
  SRF10 srf10Right((tU16)0xEC);
  SRF10 srf10Bottom((tU16)0xE0);

  // Create and initialise the queue manager
  QManager qManager;
  qManagerPtr = &qManager;
  i2c0SetCallback(i2c0CallBack); // set qmanager callback

  // Initialise Timer #0
  initTimer0(timer0CallBack); // set qmanager callback

  // Create the SRF10 controller instances
  SRF10Controller controller[] = {
    SRF10Controller(&srf10Back,'B'), 
    SRF10Controller(&srf10Front,'F'),
    SRF10Controller(&srf10Left,'L'),
    SRF10Controller(&srf10Right,'R'),
    SRF10Controller(&srf10Bottom,'U')
  };

  // Initialise SRF10 controllers
  SRF10Controller::resetAll();

  // Send all SRF10 controllers to the firing queue
  for (int i = 0; i < SRF10_DEVICES; i++) {
    qManager.queueForTimer(&controller[i]);
  }

  // Create the command handler so requests from the client can be processed
  CommandHandler cmmdHandler(&qManager, controller, SRF10_DEVICES);

  // Print welcome message
  printS("***************Ultrasonic Ranging System Starting*****************\n");

  // Send initialisation status to the client
  cmmdHandler.sendInitInfo();

  // The main execution loop
  while (true) {
    // Poll for client requests
    cmmdHandler.checkForRequest();
    // Process the SRF10 controllers queues
    qManager.executeNext();
  }

  return 0;
}
  

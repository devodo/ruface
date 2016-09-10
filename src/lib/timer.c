/******************************************************************************
 *
 * TIMER LIBRARY
 * 
 * File:
 *    timer.c
 *
 * Description:
 *    Simple library to control the timer modules:
 * 
 * Helistix software package
 *   
 * Copyright:
 *    (C) 2006 
 * 					
 * 
 *****************************************************************************/
#include "timer.h"
#include "../startup/config.h"
#include <lpc2xxx.h>
#include "irq/irq.h"
#include "myprint.h"

void (*pTimerCallback)(void);

/*****************************************************************************
 *
 * Description:
 *    Initialize Timer #0 (and VIC) for generating system tick interrupts.
 *
 * Params:
 *    [in] pISR      - function pointer to timer ISR.
 *    [in] pCallback - function pointer to timer callback function.
 *                     NULL if no callback function shall be registered.
 *
 ****************************************************************************/
void
initTimer0(void (*pCallback)())
{
  //store timer callback function pointer
  pTimerCallback = pCallback;
  
  //initialize VIC for Timer0 interrupts
  VICIntSelect &= ~0x10;       //Timer0 interrupt is assigned to IRQ (not FIQ)
  VICVectAddr4  = (tU32)timer0ISR;  //register ISR address
  VICVectCntl4  = 0x24;        //enable vector interrupt for timer0
  VICIntEnable  = 0x10;        //enable timer0 interrupt

  //initialize and start Timer0
  T0TCR = 0x00000002;          //disable and reset Timer0
  T0PC  = 0x00000000;          //no prescale of clock
  //T0MR0 = (CRYSTAL_FREQUENCY * PLL_FACTOR) /    //calculate no of timer ticks
  //        (hZ * VPBDIV_FACTOR);   //for a given system tick rate
  
  T0MCR = 0x00000000;          //disable all MR matches
  T0TCR = 0x00000001;          //start Timer0
}

void
initTimer1()
{
  //initialize and start Timer1
  T1TCR = 0x00000002;          //disable and reset Timer0
  T1PC  = 0x00000000;          //no prescale of clock
  T1MCR = 0x00000000;          //disable all MR matches
  T1TCR = 0x00000001;          //start Timer0
}

tU32
timer0Time()
{
  return T0TC;
}

tU32
timer1Time()
{
  return T1TC;
}

void
timer0setMR0Int(tU32 mr0)
{
  T0TCR = 0x00000002;                           //disable and reset Timer0
  T0PC  = 0x00000000;                           //no prescale of clock
  T0MR0 = mr0;                                  //set no of timer ticks
  T0IR  = 0x000000ff;                           //reset all flags before enable IRQs
  T0MCR = 0x00000001;                           //reset counter and generate IRQ on MR0 match
  T0TCR = 0x00000001;                           //start Timer0
}

void 
timer0DisableMRInt()
{
  T0MCR = 0x00000000; //disable all MR matches
}

void
timer0Reset()
{
  T0TCR = 0x00000002; //disable and reset Timer0
  T0TCR = 0x00000001; //start Timer0
}

void
timer0Stop()
{
  T0TCR = 0x00000002; //disable and reset Timer0
}

/*****************************************************************************
 *
 * Description:
 *    Delay execution by a specified number of milliseconds by using
 *    timer #1. A polled implementation.
 *
 * Params:
 *    [in] delayInMs - the number of milliseconds to delay.
 *
 ****************************************************************************/
void
delayMs(tU16 delayInMs)
{
  /*
   * setup timer #1 for delay
   */
  TIMER1_TCR = 0x02;          //stop and reset timer
  TIMER1_PR  = 0x00;          //set prescaler to zero
  TIMER1_MR0 = delayInMs * ((CRYSTAL_FREQUENCY * PLL_FACTOR)/ (1000 * VPBDIV_FACTOR));
  TIMER1_IR  = 0xff;          //reset all interrrupt flags
  TIMER1_MCR = 0x04;          //stop timer on match
  TIMER1_TCR = 0x01;          //start timer
  
  //wait until delay time has elapsed
  while (TIMER1_TCR & 0x01);
}

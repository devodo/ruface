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
#ifndef _TIMER_H
#define _TIMER_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "datatypes.h"
 

/******************************************************************************
 * Public functions
 *****************************************************************************/
void initTimer0(void (*pCallback)());
void initTimer1();
tU32 timer0Time();
tU32 timer1Time();
void timer0setMR0Int(tU32 mr0);
void timer0DisableMRInt();
void timer0Reset();
void delayMs(tU16 delayInMs);
#endif

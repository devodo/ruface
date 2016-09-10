/******************************************************************************
 *
 * Copyright:
 *    (C) 2005 Embedded Artists AB
 *
 * File:
 *    irq.h
 *
 * Description:
 *    Contains interface definitions for interrupt routines
 *
 *****************************************************************************/
#ifndef _IRQ_H_
#define _IRQ_H_

#include "../datatypes.h"

/*****************************************************************************
 * External variables
 ****************************************************************************/


extern tU8 uart0TxBuf[];
extern volatile tU32 uart0TxHead;
extern volatile tU32 uart0TxTail;
extern volatile tU8  uart0TxRunning;

extern tU8 uart0RxBuf[];
extern volatile tU32 uart0RxHead;
extern volatile tU32 uart0RxTail;
extern volatile tU8 uart0RxNew;

extern volatile tU32 rpsUp;
extern volatile tU32 rpsDw;

extern tU8 i2c0Buf[];
extern volatile tU32 i2c0Len;
extern volatile tU32 i2c0Seeker;
extern tU8 i2c0Valid;
extern tU16 i2c0Addr;

extern tU8 i2c1Buf[];
extern volatile tU32 i2c1Len;
extern volatile tU32 i2c1Seeker;
extern tU8 i2c1Valid;
extern tU16 i2c1Addr;

/*****************************************************************************
 * Public function prototypes
 ****************************************************************************/

//void uart0ISR(void);
//void eint2ISR(void);
//void eint3ISR(void);
void timer0ISR(void);
void i2c0ISR(void);
void i2c1ISR(void);
void i2c0CheckSum();
void i2c1CheckSum();
void i2c0SetCallback(void (*callback)(tU8));


tU32 disIrq(void);
void restoreIrq(tU32 restoreValue);


#endif

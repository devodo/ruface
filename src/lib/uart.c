/******************************************************************************
 *
 * Copyright:
 *    (C) 2005 Embedded Artists AB
 *
 * File:
 *    uart.c
 *
 * Description:
 *    Implementation of polled mode for UART #0.
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "datatypes.h"
#include "lpc2xxx.h"
#include "uart.h"

/*****************************************************************************
 * Implementation of public functions
 ****************************************************************************/

/*****************************************************************************
 *
 * Description:
 *    Initialize UART #0 in polled mode, i.e., interrupts are not used.
 *
 * Parameters:
 *    [in] div_factor - UART clock division factor to get desired bit rate.
 *                      Use definitions in uart.h to calculate correct value.
 *    [in] mode       - transmission format settings. Use constants in uart.h
 *    [in] fifo_mode  - FIFO control settings. Use constants in uart.h
 *
 ****************************************************************************/
void
initUart0(tU16 div_factor, tU8 mode, tU8 fifo_mode)
{
  volatile tU32 dummy;
  
  //enable uart #0 pins in GPIO (P0.0 = TxD0, P0.1 = RxD0)
  PINSEL0 = (PINSEL0 & 0xfffffff0) | 0x00000005;

  U0IER = 0x00;                        //disable all uart interrupts
  dummy = U0IIR;                       //clear all pending interrupts
  dummy = U0RBR;                       //clear receive register
  dummy = U0LSR;                       //clear line status register

  //set the bit rate = set uart clock (pclk) divisionfactor
  U0LCR = 0x80;                        //enable divisor latches (DLAB bit set, bit 7)
  U0DLL = (tU8)div_factor;             //write division factor LSB
  U0DLM = (tU8)(div_factor >> 8);      //write division factor MSB

  //set transmissiion and fifo mode
  U0LCR = (mode & ~0x80);              //DLAB bit (bit 7) must be reset
  U0FCR = fifo_mode;
}

/*****************************************************************************
 *
 * Description:
 *    Blocking output routine, i.e., the routine waits until the uart 
 *    buffer is free and then sends the character. 
 *
 * Params:
 *    [in] charToSend - The character to print (to uart #0) 
 *
 ****************************************************************************/
void
uart0SendChar(tU8 charToSend)
{
  //Wait until THR (Transmit Holding Register) is empty = THRE bit gets set
  while(!(U0LSR & 0x20))
    ;
  U0THR = charToSend;
}


/*****************************************************************************
 *
 * Description:
 *    Output routine that adds extra line feeds at line breaks. 
 *
 * Params:
 *    [in] charToSend - The character to print (to uart #0) 
 *
 ****************************************************************************/
void
uart0SendCh(tU8 charToSend)
{
  if(charToSend == '\n')
    uart0SendChar('\r');

  uart0SendChar(charToSend);
}


/*****************************************************************************
 *
 * Description:
 *    Print NULL-terminated string to uart #0. 
 *
 * Params:
 *    [in] pString - Pointer to NULL-terminated string to be printed 
 *
 ****************************************************************************/
void
uart0SendString(tU8 *pString)
{
  while(*pString)
    uart0SendChar(*pString++);
}

/*****************************************************************************
 *
 * Description:
 *    Print a fixed number of bytes (as opposed to NULL-terminated string).
 *
 * Params:
 *    [in] pBuff - The character to print (to uart #0) 
 *    [in] count - Number of characters to print
 *
 ****************************************************************************/
void
uart0SendChars(char *pBuff, tU16 count)
{
  while (count--)
    uart0SendChar(*pBuff++);
}

/*****************************************************************************
 *
 * Description:
 *    Returns the status of the UART transmit function.
 *
 * Returns:
 *    TRUE if both tx holding and tx shift register are empty, else FALSE.
 *
 ****************************************************************************/
tU8
uart0TxEmpty(void)
{
  //return TRUE if both THRE and TEMT bits are set
  //THRE = Transmitter Holding Register Empty (0x20)
  //TEMT = Transmitter Empty (0x40)
  return ((U0LSR & 0x60) == 0x60);
}

/*****************************************************************************
 *
 * Description:
 *    Removes all characters in the UART transmit queue. 
 *
 ****************************************************************************/
void
uart0TxFlush(void)
{
  //clear/reset the tx fifo
  U0FCR |= 0x04;
}


/*****************************************************************************
 *
 * Description:
 *    Blocking function that waits for a received character. 
 *
 * Return:
 *    The received character. 
 *
 ****************************************************************************/
tU8
uart0GetCh(void)
{
  //wait for Received Data Ready bit (0x01) to be set 
  while(!(U0LSR & 0x01))
    ;
  return U0RBR;
}

/*****************************************************************************
 *
 * Description:
 *    Non-blocking receive function.
 *
 * Params:
 *    [in] pRxChar - Pointer to buffer where the received character shall
 *                   be placed.
 *
 * Return:
 *    TRUE if character was received, else FALSE.
 *
 ****************************************************************************/
tU8
uart0GetChar(tU8 *pRxChar)
{
  //check if Received Data Ready bit (0x01) to be set 
  if((U0LSR & 0x01) != 0x00)
  {
    *pRxChar = U0RBR;
    return TRUE;
  }
  return FALSE;
}

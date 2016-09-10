/******************************************************************************
 *
 * Copyright:
 *    (C) 2005 Embedded Artists AB
 *
 * File:
 *    uart.h
 *
 * Description:
 *    Contains interface definitions for the polled UART implementation
 *
 *****************************************************************************/
#ifndef _UART_H_
#define _UART_H_

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "datatypes.h"
#include "lpc2xxx.h"

/******************************************************************************
 * Defines and typedefs
 *****************************************************************************/
//bit definitions in LCR and FCR registers in the UART
#define ULCR_CHAR_7   0x02
#define ULCR_CHAR_8   0x03
#define ULCR_PAR_NO   0x00
#define ULCR_PAR_EVEN 0x30
#define ULCR_PAR_ODD  0x10
#define ULCR_STOP_1   0x00
#define ULCR_STOP_2   0x04

#define UFCR_FIFO_ENABLE 0x01
#define UFCR_FIFO_TRIG1  0x00
#define UFCR_FIFO_TRIG4  0x40
#define UFCR_FIFO_TRIG8  0x80
#define UFCR_FIFO_TRIG16 0xc0

//define for determning the correct uart clock division factor
//Note that the expressions should always be constants and fully evaluated at
//compile time, else a lot of code will be generated.
#define UART_BPS(pclk,bps) (tU16)((pclk / ((bps) * 16.0)) + 0.5)

//definitions for some common bitrates
#define B1200(pclk)         UART_BPS(pclk,1200)
#define B2400(pclk)         UART_BPS(pclk,2400)
#define B9600(pclk)         UART_BPS(pclk,9600)
#define B19200(pclk)        UART_BPS(pclk,19200)
#define B38400(pclk)        UART_BPS(pclk,38400)
#define B57600(pclk)        UART_BPS(pclk,57600)
#define B115200(pclk)       UART_BPS(pclk,115200)

//definitions for mode settings
#define UART_7N1      (tU8)(ULCR_CHAR_7 + ULCR_PAR_NO   + ULCR_STOP_1)
#define UART_7N2      (tU8)(ULCR_CHAR_7 + ULCR_PAR_NO   + ULCR_STOP_2)
#define UART_7E1      (tU8)(ULCR_CHAR_7 + ULCR_PAR_EVEN + ULCR_STOP_1)
#define UART_7E2      (tU8)(ULCR_CHAR_7 + ULCR_PAR_EVEN + ULCR_STOP_2)
#define UART_7O1      (tU8)(ULCR_CHAR_7 + ULCR_PAR_ODD  + ULCR_STOP_1)
#define UART_7O2      (tU8)(ULCR_CHAR_7 + ULCR_PAR_ODD  + ULCR_STOP_2)
#define UART_8N1      (tU8)(ULCR_CHAR_8 + ULCR_PAR_NO   + ULCR_STOP_1)
#define UART_8N2      (tU8)(ULCR_CHAR_8 + ULCR_PAR_NO   + ULCR_STOP_2)
#define UART_8E1      (tU8)(ULCR_CHAR_8 + ULCR_PAR_EVEN + ULCR_STOP_1)
#define UART_8E2      (tU8)(ULCR_CHAR_8 + ULCR_PAR_EVEN + ULCR_STOP_2)
#define UART_8O1      (tU8)(ULCR_CHAR_8 + ULCR_PAR_ODD  + ULCR_STOP_1)
#define UART_8O2      (tU8)(ULCR_CHAR_8 + ULCR_PAR_ODD  + ULCR_STOP_2)

//definitions for FIFO control settings
#define UART_FIFO_OFF (0x00)
#define UART_FIFO_1   (tU8)(UFCR_FIFO_ENABLE + UFCR_FIFO_TRIG1)
#define UART_FIFO_4   (tU8)(UFCR_FIFO_ENABLE + UFCR_FIFO_TRIG4)
#define UART_FIFO_8   (tU8)(UFCR_FIFO_ENABLE + UFCR_FIFO_TRIG8)
#define UART_FIFO_14  (tU8)(UFCR_FIFO_ENABLE + UFCR_FIFO_TRIG14)

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
void initUart0(tU16 div_factor, tU8 mode, tU8 fifo_mode);


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
void uart0SendChar(tU8 charToSend);


/*****************************************************************************
 *
 * Description:
 *    Output routine that adds extra line feeds at line breaks. 
 *
 * Params:
 *    [in] charToSend - The character to print (to uart #0) 
 *
 ****************************************************************************/
void uart0SendCh(tU8 charToSend);


/*****************************************************************************
 *
 * Description:
 *    Print NULL-terminated string to uart #0. 
 *
 * Params:
 *    [in] pString - Pointer to NULL-terminated string to be printed 
 *
 ****************************************************************************/
void uart0SendString(tU8 *pString);


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
void uart0SendChars(char *pBuff, tU16 count);


/*****************************************************************************
 *
 * Description:
 *    Returns the status of the UART transmit function.
 *
 * Returns:
 *    TRUE if both tx holding and tx shift register are empty, else FALSE.
 *
 ****************************************************************************/
tU8 uart0TxEmpty(void);


/*****************************************************************************
 *
 * Description:
 *    Removes all characters in the UART transmit queue. 
 *
 ****************************************************************************/
void uart0TxFlush(void);


/*****************************************************************************
 *
 * Description:
 *    Blocking function that waits for a received character. 
 *
 * Return:
 *    The received character. 
 *
 ****************************************************************************/
tU8 uart0GetCh(void);


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
tU8 uart0GetChar(tU8 *pRxChar);


#endif

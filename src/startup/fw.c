/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2005 Embedded Artists AB
 *
 * Description:
 *    Framework for ARM7 processor
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "lpc2xxx.h"                            /* LPC2xxx definitions */
#include "fw.h"

/******************************************************************************
 * Defines, macros, and typedefs
 *****************************************************************************/


/******************************************************************************
 * External variables
 *****************************************************************************/
extern char end asm ("end");  //the symbol "end" is defined by the linker script


/******************************************************************************
 * Public variables
 *****************************************************************************/
unsigned char *pHeapStart;
unsigned char *pHeapEnd;


/******************************************************************************
 * External functions
 *****************************************************************************/


/******************************************************************************
 * Public functions
 *****************************************************************************/
void lowLevelInit(void);

/*****************************************************************************
 *
 * Description:
 *    Initialize system functions and GPIO
 *
 ****************************************************************************/
void
lowLevelInit(void)
{
  PINSEL0 = 0x00000000;  
  PINSEL1 = 0x00000000;  

  IOSET = 0x00000000;       //Initialize pins to high level
  IOCLR = 0xffffffff;       //Initialize pins to low level
  IODIR = 0x00000000;       //Set pin direction

  //initialize the MAM (Memory Accelerator Module)
  MAMTIM = MAM_TIMING;       //number of CCLK to read from the FLASH
  MAMCR  = MAM_SETTING;      //0=disabled, 1=partly enabled (enabled for code prefetch, but not for data), 2=fully enabled

  //initialize the exception vector mapping
  MAMMAP = MAM_MAP;

  //set the peripheral bus speed, PCLK = CCLK / PBSD
	VPBDIV = PBSD;
	
	//initialize VIC
  VICIntEnClr    = 0xFFFFFFFF;           /* Disable ALL interrupts                             */
  VICProtection  = 0;                    /* Setup interrupt controller                         */
  VICDefVectAddr = (unsigned int)0;      /* Direct unvectored IRQs to reset, i.e., address 0x0 */

  VICVectAddr0   = (unsigned int)0;      /* Set the vector address                             */
  VICVectAddr1   = (unsigned int)0;      /* Set the vector address                             */
  VICVectAddr2   = (unsigned int)0;      /* Set the vector address                             */
  VICVectAddr3   = (unsigned int)0;      /* Set the vector address                             */
  VICVectAddr4   = (unsigned int)0;      /* Set the vector address                             */
  VICVectAddr5   = (unsigned int)0;      /* Set the vector address                             */
  VICVectAddr6   = (unsigned int)0;      /* Set the vector address                             */
  VICVectAddr7   = (unsigned int)0;      /* Set the vector address                             */
  VICVectAddr8   = (unsigned int)0;      /* Set the vector address                             */
  VICVectAddr9   = (unsigned int)0;      /* Set the vector address                             */
  VICVectAddr10  = (unsigned int)0;      /* Set the vector address                             */
  VICVectAddr11  = (unsigned int)0;      /* Set the vector address                             */
  VICVectAddr12  = (unsigned int)0;      /* Set the vector address                             */
  VICVectAddr13  = (unsigned int)0;      /* Set the vector address                             */
  VICVectAddr14  = (unsigned int)0;      /* Set the vector address                             */
  VICVectAddr15  = (unsigned int)0;      /* Set the vector address                             */
	
  //enable interrupts (both IRQ and FIQ) 
  asm volatile ("mrs r3, cpsr       \n\t"                          
                "bic r3, r3, #0xC0  \n\t"                      
                "msr cpsr, r3       \n\t"                          
                :                                       
                :                                       
                : "r3" );
}

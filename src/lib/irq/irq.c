/******************************************************************************
 *
 * Copyright:
 *    (C) 2005 Embedded Artists AB
 *
 * File:
 *    irqUart.c
 *
 * Description:
 *    Sample irq code, that must be compiled in ARM code.
 *
 *****************************************************************************/
//#define DEBUG
/******************************************************************************
 * Includes
 *****************************************************************************/
#include "../lpc2xxx.h"
#include "irq.h"
//#include "../odo.h"
#include "../uart.h"
#include "../i2c.h"

#ifdef DEBUG
#include "../uart.h"
#endif

/*****************************************************************************
 * Local variables
 ****************************************************************************/
tU32 upCnt;
tU32 dwCnt;
void (*i2c0CallBack)(tU8) = 0;
extern void (*pTimerCallback)(void);

/*****************************************************************************
 * Public function prototypes
 ****************************************************************************/
//void uart0ISR(void) __attribute__ ((interrupt));
void i2c0ISR(void) __attribute__ ((interrupt));
void i2c1ISR(void) __attribute__ ((interrupt));
void timer0ISR(void) __attribute__ ((interrupt));
//void tim0ISR(void) __attribute__ ((interrupt));
//void eint2ISR(void) __attribute__ ((interrupt));
//void eint3ISR(void) __attribute__ ((interrupt));
void i2c0CheckSum();
void i2c1CheckSum();

/*****************************************************************************
 * Implementation of public functions
 ****************************************************************************/
void i2c0SetCallback(void (*callback)(tU8))
{
  i2c0CallBack = callback;
}

/*****************************************************************************
 *
 * Description:
 *    Actual uart #0 ISR that is called whenever the uart generated an interrupt.
 *
 ****************************************************************************/
/*
  void uart0ISR(void)
  {
  volatile tU8  statusReg;
  volatile tU8  dummy;
  volatile tU32 tmpHead;
  volatile tU32 tmpTail;
  
  //loop until not more interrupt sources
  while (((statusReg = U0IIR) & 0x01) == 0)
  {
  //identify and process the highest priority interrupt
  switch (statusReg & 0x0E){
  
  case 0x06:  //Receive Line Status
  dummy = U0LSR;  //read LSR to clear bits
  break;
  
  case 0x0c:  //Character Timeout Indicator
  
  case 0x04:  //Receive Data Available
  do{
  tmpHead     = (uart0RxHead + 1) & RX_BUFFER_MASK;
  uart0RxHead = tmpHead;
  
  if(tmpHead == uart0RxTail)
  tmpHead = U0RBR;    //dummy read to reset IRQ flag
  else
  uart0RxBuf[tmpHead] = U0RBR;  //will reset IRQ flag
  
  } while (U0LSR & 0x01);
  break;
  
  case 0x02:  //Transmit Holding Register Empty
  //check if all data is transmitted
  if (uart0TxHead != uart0TxTail){
  
  tU32 bytesToSend;
  
  if (statusReg & 0xc0)
  bytesToSend = 16;    //FIFO enabled
  else
  bytesToSend = 1;     //no FIFO enabled
  
  do{ //calculate buffer index
  tmpTail = (uart0TxTail + 1) & TX_BUFFER_MASK;
  uart0TxTail = tmpTail;
  U0THR = uart0TxBuf[tmpTail]; 
  } while((uart0TxHead != uart0TxTail) && --bytesToSend);
  } 
  //all data has been transmitted
  else {
  uart0TxRunning = FALSE;
  U0IER &= ~0x02;        //disable TX IRQ
  }
  break;
  
  default:  //unknown
  dummy = U0LSR;
  dummy = U0RBR;
  break;
  }
  }
  
  VICVectAddr = 0x00000000;    //dummy write to VIC to signal end of interrupt
  }
*/
/*****************************************************************************
 *
 * Description:
 *    Actual I2C0 ISR that is called whenever the IC2CON SI bit generates an interrupt.
 *
 ****************************************************************************/
void i2c0ISR(void)
{
  tU8 status;
  status = I2C0_STAT;
#ifdef DEBUG
  uart0SendString((unsigned char*)"  STATUS(");
  uart0SendCh((status/10)%10+'0');uart0SendCh(status%10+'0');
  uart0SendString((unsigned char*)")\n");
#endif
  
  switch(status){
    
    // Non mode specific states
  case 0: /* Bus Error */ 
#ifdef DEBUG
    uart0SendString((unsigned char*) "Bus error!!!\n");
#endif
    /* Set I2EN and STO bits */
    I2C0_CONSET=0x50;
    /* Clear SI and STA bits */
    I2C0_CONCLR=0x28;
    break;
    
    // Master states
  case 8: /* Start condition transmitted */
    /* Slave address + read/write */
    I2C0_DAT = i2c0Addr;
#ifdef DEBUG
    uart0SendString((unsigned char*) "Start sent Sending now the slave address...\n");
#endif
    /* Set I2EN bit */
    I2C0_CONSET=0x40;
    /* Clear SI and STO bits*/
    I2C0_CONCLR=0x18;
    break;
    
  case 16: /* Repeted Start condition transmitted */
    /* Slave address + read/write */
    I2DAT = i2c0Addr;
#ifdef DEBUG
    uart0SendString((unsigned char*) "Repeated start sent Sending now the slave address...\n");
#endif
    /* Set I2EN bit */
    I2C0_CONSET=0x40;
    /* Clear SI and STO bits*/
    I2C0_CONCLR=0x18;
    break;
    
    // Master Transmit states            
  case 24: /* Acknowledgement received from slave for slave address */
    /* Data to be transmitted */
    I2DAT = i2c0Buf[i2c0Seeker];
    i2c0Seeker++; 
#ifdef DEBUG
    uart0SendString((unsigned char*) "Acknowledgement to write received, writing first byte...["); 
    uart0SendCh(i2c0Buf[i2c0Seeker-1]);
    uart0SendString((unsigned char*) "]\n");    
#endif
    /* Set I2EN bit */
    I2C0_CONSET=0x40;
    /* clear SI STO and STA bits*/
    I2C0_CONCLR=0x38;        
    break;
    
  case 32: /* Slave address + write has been transmitted, NOT ACK has been received
	      a Stop */ 
#ifdef DEBUG					
    uart0SendString((unsigned char*) "NOT ACK recv transmitting Stop!\n");  
#endif
    if (i2c0CallBack != 0)
      (*i2c0CallBack)(status);
    /* Set STO I2EN and STA bits*/
    I2C0_CONSET=0x50;
    /* clear SI */
    I2C0_CONCLR=0x28;              
    break;           
    
  case 40: /* Data has been transmitted, ACK has been received
	      transmit the new byte or transmit a Stop if Seeker = Len */ 					
    if ( i2c0Seeker == i2c0Len ){
#ifdef DEBUG
      uart0SendString((unsigned char*) "data finished transmitting Stop!\n");				
#endif
      if (i2c0CallBack != 0)
	(*i2c0CallBack)(status);
      /* Set STO and E2IN bits*/
      I2C0_CONSET=0x50;
      /* clear SI and STA bits*/
      I2C0_CONCLR = 0x28;					
    } else {
      I2C0_DAT = i2c0Buf[i2c0Seeker];
      i2c0Seeker++;
#ifdef DEBUG
      uart0SendString((unsigned char*) "trasmitting a data byte [");
      uart0SendCh(i2c0Buf[i2c0Seeker-1]);
      uart0SendString((unsigned char*) "]...\n");				
#endif
      /* Set I2EN bit */
      I2C0_CONSET=0x40;
      /* clear SI STO and STA bits*/
      I2C0_CONCLR = 0x38;
    }			                
    break;
    
  case 48: /* Data has been transmitted, NOT ACK has been received a
	      Stop will be transmitted */
#ifdef DEBUG		
    uart0SendString((unsigned char*) "data transmitted NOT ACK recv transmitting Stop!\n");        			
#endif
    if (i2c0CallBack != 0)
      (*i2c0CallBack)(status);
    /* Set Stop and I2EN bits*/
    I2C0_CONSET=0x50;
    //I2C0_CONSET=0x14;
    /* clear SI and STA*/
    I2C0_CONCLR=0x28;
    //I2C0_CONCLR=0x08;
    break;
    
  case 56: /* Arbitration has been lost during Slave Address + Write or data.
	      The bus has been released and not addressed Slave mode is entered.
	      A new Start condition will be transmitted when the bus is free again.*/
#ifdef DEBUG
    uart0SendString((unsigned char*) "Arbitration lost!\n"); 
#endif
    /* Set STA and I2EN bits*/
    I2C0_CONSET=0x60;
    /* clear SI and STO*/
    I2C0_CONCLR=0x18;
    break;     
    
    // Master Receive states    
  case 64: /* Previous state was State 08 or state 10. Slave address + Read has been transmitted,
	      ACK has been received. Data will be received and ACK returned. */
#ifdef DEBUG
    uart0SendString((unsigned char*) "master recv state ACK...\n"); 
#endif
    /* Set AA and I2EN bits*/
    I2C0_CONSET=0x44;
    /* clear SI and STO*/
    I2C0_CONCLR=0x38;
    break;
    
  case 72: /* Slave address + Read has been transmitted, NOT ACK has been received.
	      A Stop condition followed by a Start will be transmitted. */
#ifdef DEBUG
    uart0SendString((unsigned char*) "read transmitted NOT ACK recv, STOP and then Start will be transmitted...\n"); 
#endif
    if (i2c0CallBack != 0)
      (*i2c0CallBack)(status);
    /* Set Stop Start and I2EN bits*/
    I2C0_CONSET=0x50; // I2C0_CONSET=0x70;
    /* clear SI */
    I2C0_CONCLR=0x28; // I2C0_CONCLR=0x08;
    
    break;
    
  case 80: /* Data has been received, ACK has been returned. Data will be read from I2DAT.
	      Additional data will be received. If this is the last data byte then NOT ACK will be returned,
	      otherwise ACK will be returned. */
    /* save the received data */
    i2c0Buf[i2c0Seeker] = I2C0_DAT;
    i2c0Seeker++;
    if (i2c0Seeker == i2c0Len ){
      i2c0CheckSum();
#ifdef DEBUG
      uart0SendString((unsigned char*) "all data read transmitting NOT ACK [");
      uart0SendCh(i2c0Buf[i2c0Seeker-1]);
      uart0SendString((unsigned char*) "]...\n");
#endif
      /* Set I2EN */     
      I2C0_CONSET=0x40;    		        		
      /* clear SI STO and AA*/
      I2C0_CONCLR=0x1C;       	    	
    }else {
#ifdef DEBUG
      uart0SendString((unsigned char*) "not all data read transmitting ACK ["); 
      uart0SendCh(i2c0Buf[i2c0Seeker-1]);
      uart0SendString((unsigned char*) "]...\n"); 
#endif				
      /* Set I2EN and AA bit*/
      I2C0_CONSET=0x44;
      /* clear SI and STO*/      			
      I2C0_CONCLR=0x18;     	
    }
    break;                 
    
  case 88: /* Data has been received, NOT ACK has been returned. Data will be read from I2DAT.
	      A Stop condition will be transmitted. */
    /* save the received data */
    i2c0Buf[i2c0Seeker] = I2C0_DAT;
    i2c0Seeker++;
#ifdef DEBUG
    uart0SendString((unsigned char*) "all data read transmitting Stop!! \n");
#endif
    if (i2c0CallBack != 0)
      (*i2c0CallBack)(status);
    /* flag as validat received data */
    if ( i2c0Seeker == i2c0Len ) {
      i2c0CheckSum();
    }
    /* Set Stop and I2EN bits*/
    I2C0_CONSET = 0x50;
    /* clear SI and STA */
    I2C0_CONCLR=0x28;
    break;
    
  default :
    break;
  }
  
  VICVectAddr = 0x00000000;    //dummy write to VIC to signal end of interrupt
}

/*****************************************************************************
 *
 * Description:
 *    Actual I2C1 ISR that is called whenever the IC2CON SI bit generates an interrupt.
 *
 ****************************************************************************/

void i2c1ISR(void)
{
  tU8 status;
  status = I2C1_STAT;
#ifdef DEBUG
  uart0SendString((unsigned char*)"  STATUS(");
  uart0SendCh((status/10)%10+'0');uart0SendCh(status%10+'0');
  uart0SendString((unsigned char*)")\n");
#endif
  
  switch(status){
    
    // Non mode specific states
  case 0: /* Bus Error */ 
#ifdef DEBUG
    uart0SendString((unsigned char*) "Bus error!!!\n");
#endif
    /* Set I2EN and STO bits */
    I2C1_CONSET=0x50;
    /* Clear SI and STA bits */
    I2C1_CONCLR=0x28;
    break;
    
    // Master states
  case 8: /* Start condition transmitted */
    /* Slave address + read/write */
    I2C1_DAT = i2c1Addr;
#ifdef DEBUG
    uart0SendString((unsigned char*) "Start sent Sending now the slave address...\n");
#endif
    /* Set I2EN bit */
    I2C1_CONSET=0x40;
    /* Clear SI and STO bits*/
    I2C1_CONCLR=0x18;
    break;
    
  case 16: /* Repeted Start condition transmitted */
    /* Slave address + read/write */
    I2DAT = i2c1Addr;
#ifdef DEBUG
    uart0SendString((unsigned char*) "Repeated start sent Sending now the slave address...\n");
#endif
    /* Set I2EN bit */
    I2C1_CONSET=0x40;
    /* Clear SI and STO bits*/
    I2C1_CONCLR=0x18;
    break;
    
    // Master Transmit states            
  case 24: /* Acknowledgement received from slave for slave address */
    /* Data to be transmitted */
    I2DAT = i2c1Buf[i2c1Seeker];
    i2c1Seeker++; 
#ifdef DEBUG
    uart0SendString((unsigned char*) "Acknowledgement to write received, writing first byte...["); 
    uart0SendCh(i2c1Buf[i2c1Seeker-1]);
    uart0SendString((unsigned char*) "]\n");    
#endif
    /* Set I2EN bit */
    I2C1_CONSET=0x40;
    /* clear SI STO and STA bits*/
    I2C1_CONCLR=0x38;        
    break;
    
  case 32: /* Slave address + write has been transmitted, NOT ACK has been received
	      a Stop */ 
#ifdef DEBUG					
    uart0SendString((unsigned char*) "NOT ACK recv transmitting Stop!\n");  
#endif
    /* Set STO I2EN and STA bits*/
    I2C1_CONSET=0x50;
    /* clear SI */
    I2C1_CONCLR=0x28;              
    break;           
    
  case 40: /* Data has been transmitted, ACK has been received
	      transmit the new byte or transmit a Stop if Seeker = Len */ 					
    if ( i2c1Seeker == i2c1Len ){
#ifdef DEBUG
      uart0SendString((unsigned char*) "data finished transmitting Stop!\n");				
#endif
      /* Set STO and E2IN bits*/
      I2C1_CONSET=0x50;
      /* clear SI and STA bits*/
      I2C1_CONCLR = 0x28;					
    } else {
      I2C1_DAT = i2c1Buf[i2c1Seeker];
      i2c1Seeker++;
#ifdef DEBUG
      uart0SendString((unsigned char*) "trasmitting a data byte [");
      uart0SendCh(i2c1Buf[i2c1Seeker-1]);
      uart0SendString((unsigned char*) "]...\n");				
#endif
      /* Set I2EN bit */
      I2C1_CONSET=0x40;
      /* clear SI STO and STA bits*/
      I2C1_CONCLR = 0x38;
    }			                
    break;
    
  case 48: /* Data has been transmitted, NOT ACK has been received a
	      Stop will be transmitted */
#ifdef DEBUG		
    uart0SendString((unsigned char*) "data transmitted NOT ACK recv transmitting Stop!\n");        			
#endif
    /* Set Stop and I2EN bits*/
    I2C1_CONSET=0x50;
    /* clear SI and STA*/
    I2C1_CONCLR=0x28; 
    break;
    
  case 56: /* Arbitration has been lost during Slave Address + Write or data.
	      The bus has been released and not addressed Slave mode is entered.
	      A new Start condition will be transmitted when the bus is free again.*/
#ifdef DEBUG
    uart0SendString((unsigned char*) "Arbitration lost!\n"); 
#endif
    /* Set STA and I2EN bits*/
    I2C1_CONSET=0x60;
    /* clear SI and STO*/
    I2C1_CONCLR=0x18;
    break;     
    
    // Master Receive states    
  case 64: /* Previous state was State 08 or state 10. Slave address + Read has been transmitted,
	      ACK has been received. Data will be received and ACK returned. */
#ifdef DEBUG
    uart0SendString((unsigned char*) "master recv state ACK...\n"); 
#endif
    /* Set AA and I2EN bits*/
    I2C1_CONSET=0x44;
    /* clear SI and STO*/
    I2C1_CONCLR=0x38;
    break;
    
  case 72: /* Slave address + Read has been transmitted, NOT ACK has been received.
	      A Stop condition followed by a Start will be transmitted. */
#ifdef DEBUG
    uart0SendString((unsigned char*) "read transmitted NOT ACK recv, STOP and then Start will be transmitted...\n"); 
#endif
    /* Set Stop Start and I2EN bits*/
    I2C1_CONSET=0x50; // I2C1_CONSET=0x70;
    /* clear SI */
    I2C1_CONCLR=0x28; // I2C1_CONCLR=0x08;
    
    break;
    
  case 80: /* Data has been received, ACK has been returned. Data will be read from I2DAT.
	      Additional data will be received. If this is the last data byte then NOT ACK will be returned,
	      otherwise ACK will be returned. */
    /* save the received data */
    i2c1Buf[i2c1Seeker] = I2C1_DAT;
    i2c1Seeker++;
    if (i2c1Seeker == i2c1Len){
      i2c1CheckSum();
#ifdef DEBUG
      uart0SendString((unsigned char*) "all data read transmitting NOT ACK [");
      uart0SendCh(i2c1Buf[i2c1Seeker-1]);
      uart0SendString((unsigned char*) "]...\n");
#endif
      /* Set I2EN */     
      I2C1_CONSET=0x40;    		        		
      /* clear SI STO and AA*/
      I2C1_CONCLR=0x1C;       	    	
    }else {
#ifdef DEBUG
      uart0SendString((unsigned char*) "not all data read transmitting ACK ["); 
      uart0SendCh(i2c1Buf[i2c1Seeker-1]);
      uart0SendString((unsigned char*) "]...\n"); 
#endif				
      /* Set I2EN and AA bit*/
      I2C1_CONSET=0x44;
      /* clear SI and STO*/      			
      I2C1_CONCLR=0x18;     	
    }
    break;                 
    
  case 88: /* Data has been received, NOT ACK has been returned. Data will be read from I2DAT.
	      A Stop condition will be transmitted. */
    /* save the received data */
    i2c1Buf[i2c1Seeker] = I2C1_DAT;
    i2c1Seeker++;
#ifdef DEBUG
    uart0SendString((unsigned char*) "all data read transmitting Stop!! \n");
#endif        	
    /* flag as validat received data */
    if ( i2c1Seeker == i2c1Len ) i2c1CheckSum();		
    /* Set Stop and I2EN bits*/
    I2C1_CONSET = 0x50;
    /* clear SI and STA */
    I2C1_CONCLR=0x28;
    break;
    
  default :
    break;
  }
  
  VICVectAddr = 0x00000000;    //dummy write to VIC to signal end of interrupt
}

/*****************************************************************************
 *
 * Description:
 *    Calculate the sum off all the bytes received, compare it with the
 *    checksum and set the i2cxValid flag accordingly.
 *
 ****************************************************************************/
void i2c0CheckSum()
{
  tU32 i = 0;
  tU8 cksum = 0;
  
  for (i=0 ; i < (i2c0Len - 1); i++) {
    cksum += i2c0Buf[i];	
  }
  
  if (cksum == i2c0Buf[i2c0Len-1]){ 
    i2c0Valid = TRUE;
#ifdef DEBUG
    uart0SendString((unsigned char*) "*");
#endif
  }
  i2c0Valid = TRUE;
}

void i2c1CheckSum()
{
  tU32 i = 0;
  tU8 cksum = 0;
  
  for (i=0 ; i < (i2c1Len - 1); i++) {
    cksum += i2c1Buf[i];	
  }
  
  if (cksum == i2c1Buf[i2c1Len-1]) {
    i2c1Valid = TRUE;
#ifdef DEBUG
    uart0SendString((unsigned char*) "*");
#endif
  }
}
/*****************************************************************************
 *
 * Description:
 *    Actual EINT2 ISR that is called whenever P0.15 generates an interrupt.
 *
 ****************************************************************************/
/*
  void eint2ISR(void)
  {  	
  upCnt++;
  
  EXTINT = 0x00000004; 		   // clear interrupt
  VICVectAddr = 0x00000000;    // dummy write to VIC to signal end of interrupt
  }
*/
/*****************************************************************************
 *
 * Description:
 *    Actual EINT3 ISR that is called whenever P0.20 generates an interrupt.
 *
 ****************************************************************************/
/*void eint3ISR(void)
  { 
  dwCnt++;
  
  EXTINT = 0x00000008;		   // clear interrupt
  VICVectAddr = 0x00000000;    // dummy write to VIC to signal end of interrupt
  }
*/
/*****************************************************************************
 *
 * Description:
 *    Actual timer ISR that is called whenever timer 0 generated an interrupt.
 *
 ****************************************************************************/
/*void tim0ISR(void)
  {
  //update the rps
  rpsUp = ( 1000 * upCnt ) / ( COG_HOLES * MES_INTERVAL );
  upCnt = 0; 
  
  rpsDw = ( 1000 * dwCnt ) / ( COG_HOLES * MES_INTERVAL );
  dwCnt = 0;
  
  T0IR        = 0xff;        //reset all IRQ flags
  VICVectAddr = 0x00;        //dummy write to VIC to signal end of interrupt
  }
*/

/*****************************************************************************
 *
 * Description:
 *    Actual timer ISR that is called whenever timer 0 generated an interrupt.
 *
 ****************************************************************************/
void
timer0ISR(void)
{
  //call timer callback, if registered
  if (pTimerCallback != 0)
    (*pTimerCallback)();
  
  T0IR        = 0xff;        //reset all IRQ flags
  VICVectAddr = 0x00;        //dummy write to VIC to signal end of interrupt
}

/*****************************************************************************
 *
 * Description:
 *    Disable interrupts 
 *
 * Returns:
 *    The current status register, before disabling interrupts. 
 *
 ****************************************************************************/
tU32 disIrq(void)
{
  tU32 returnReg;
  
  asm volatile ("disIrq1: mrs %0, cpsr  \n\t"
                "orr r1, %0, #0xC0      \n\t"
                "msr cpsr_c, r1         \n\t"
                "mrs r1, cpsr           \n\t"
                "and r1, r1, #0xC0      \n\t"
                "cmp r1, #0xC0          \n\t"
                "bne disIrq1            \n\t"
                : "=r"(returnReg)
                :
                : "r1"
		);
  
  return returnReg;
}

/*****************************************************************************
 *
 * Description:
 *    Restore interrupt state. 
 *
 * Params:
 *    [in] restoreValue - The value of the new status register. 
 *
 ****************************************************************************/
void restoreIrq(tU32 restoreValue)
{
  asm volatile ("msr cpsr_c, %0  \n\t"
                :
                : "r" (restoreValue)
                : "r1"
		);
}

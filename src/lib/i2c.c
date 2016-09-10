/******************************************************************************
 *
 * I2C LIBRARY
 * 
 * File:
 *    i2c.c
 *
 * Description:
 *    Simple library to control the I2C ports:
 * 
 * Helistix software package
 *   
 * Copyright:
 *    (C) 2006 Renzo De Nardi rdenar@essex.ac.uk 
 * 					
 * 
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "i2c.h"
#include "./irq/irq.h"
#include <lpc2xxx.h>

/******************************************************************************
 * Defines and typedefs
 *****************************************************************************/

//different modes
#define I2C_MODE_ACK0 0
#define I2C_MODE_ACK1 1
#define I2C_MODE_READ 2

// Data queues
#define I2C0_BUFFER_SIZE 256
#define I2C1_BUFFER_SIZE 256


// CONVERSION RATIOS:
//
//                                       ⎛ 600       ⎞
//                                       |---- × 1.5 ⎟
//                                       ⎜  2	     |
// gyros_value_x = raw_payload_value_x × ⎜-----------⎟
//                                       ⎜   32768   ⎟
//                                       ⎝           ⎠

#define GYRO_CONV_RATIO 450/32768

//
//                                     ⎛ 3.8       ⎞
//                                     |---- × 1.5 ⎟
//                                     ⎜  2	       |
// mag_value_x = raw_payload_value_x × ⎜-----------⎟
//                                     ⎜   32768   ⎟
//                                     ⎝           ⎠

#define MAG_CONV_RATIO 2.85/32768

//
//                                     ⎛  4        ⎞
//                                     |---- × 1.5 ⎟
//                                     ⎜  2	       |
// acc_value_x = raw_payload_value_x × ⎜-----------⎟
//                                     ⎜   32768   ⎟
//                                     ⎝           ⎠

#define ACC_CONV_RATIO 3/32768

//
//                ⎡⎛ raw_payload_value × 5 ⎞⎤
//                ⎢⎜-----------------------⎟⎥
//                |⎝        32768          ⎠⎥ 
// result_deg_C = ⎢-------------------------| + 25
//                ⎢                         ⎥
//                |          0.0084         |
//                ⎣                         ⎦


#define TEMP_CONV_RATIO 5/275.2512
#define TEMP_COMP 25

//Sample Timer: bytes 7 (MSB) and 8 (LSB) when combined represent a 16-bit timer value of the
//time at which the ADC started the conversion for the X Gyro with a scale of 2.1701 x 10-6
//seconds/count.

#define TIME_CONV_RATIO 2.1701   //useconds

/******************************************************************************
 * Variables
 *****************************************************************************/

// I2C0
tU8 i2c0Buf[I2C0_BUFFER_SIZE];
volatile tU32 i2c0Len = 0;
volatile tU32 i2c0Seeker = 0;
tU16 i2c0Addr;
tU8 i2c0Valid = FALSE;

tU8 i2c1Buf[I2C1_BUFFER_SIZE];
volatile tU32 i2c1Len = 0;
volatile tU32 i2c1Seeker = 0;
tU16 i2c1Addr;
tU8 i2c1Valid = FALSE;

/*
tU8 i2c0RxBuf[I2C0_RX_BUFFER_SIZE];
volatile tU32 i2c0RxHead = 0;
volatile tU32 i2c0RxSeeker = 0;
tU16 sl0RdAddr;

// I2C1
tU8 i2c1TxBuf[I2C1_TX_BUFFER_SIZE];
volatile tU32 i2c1TxHead = 0;
volatile tU32 i2c1TxSeeker = 0;

tU8 i2c1RxBuf[I2C1_RX_BUFFER_SIZE];
volatile tU32 i2c1RxHead = 0;
volatile tU32 i2c1RxSeeker = 0;

tU16 sl1WrAddr;
tU16 sl1RdAddr;
*/
/******************************************************************************
 *
 * Description:
 *    Reset and initialize the I2C peripheral.
 *
 * Params:
 *    [in]  i2cFrequency  - frequency on clock signal in Hz (max 400 kHz)
 *
 *****************************************************************************/
void
i2c0Init(tU32 frequency){
	
  //----------- I2C0 -------------
  //connect I2C0 signals (SDA0 & SCL0) to P0.2 and P0.3
  PINSEL0 &= ~0x000000F0;
  PINSEL0 |=  0x00000050;

  //clear flags
  I2C0_CONCLR = 0x6c;

  //set bit timing ans start with checking for maximum speed (400kHz)
  if (frequency > 400000)
     frequency = 400000;
  I2C0_SCLH   = (((CRYSTAL_FREQUENCY * PLL_FACTOR) / VPBDIV_FACTOR) / frequency + 1) / 2;
  I2C0_SCLL   = (((CRYSTAL_FREQUENCY * PLL_FACTOR) / VPBDIV_FACTOR) / frequency) / 2;

  //initialize the interrupt vector
  VICIntSelect &= ~0x00000200;      // I2C0 selected as IRQ
  VICVectCntl5  =  0x00000029;      // Enable vec int for I2C0
  VICVectAddr5  =  (tU32)i2c0ISR;   // address of the ISR
  VICIntEnable |=  0x00000200;      // I2C0 interrupt enabled

  //reset registers
  I2C0_ADR    = 0x45;
  I2C0_CONSET = 0x40;
}

void
i2c1Init(tU32 frequency){
	
  //----------- I2C1 -------------
  //connect I2C1 signals (SDA1 & SCL1) to P0.14 and P0.11
  PINSEL0 &= ~0x30000030;
  PINSEL0 |=  0x30000030;

  //clear flags
  I2C1_CONCLR = 0x6c;

  //set bit timing ans start with checking for maximum speed (400kHz)
  if (frequency > 400000)
     frequency = 400000;
  I2C1_SCLH   = (((CRYSTAL_FREQUENCY * PLL_FACTOR) / VPBDIV_FACTOR) / frequency + 1) / 2;
  I2C1_SCLL   = (((CRYSTAL_FREQUENCY * PLL_FACTOR) / VPBDIV_FACTOR) / frequency) / 2;
    
  VICIntSelect &= ~0x00080000;      // I2C1 selected as IRQ
  VICVectCntl7  =  0x00000033;		// Enable vec int for I2C1
  VICVectAddr7  =  (tU32)i2c1ISR;  	// address of the ISR
  VICIntEnable |=  0x00080000;      // I2C1 interrupt enabled
    
  //reset registers
  I2C1_ADR    = 0x37;
  I2C1_CONSET = 0x44;

}


/******************************************************************************
 *
 * Description:
 *    Send the data buffer to the specified address, 
 * 	  a start condition is generated, and the then the ISR routine takes over
 *    and carries on the communication
 * 
 * Params:
 *    [address to write to, pointer to the data, data length]
 *
 *****************************************************************************/
void 
i2c0Write(tU16  addr,char* buf, tU16 len)
{
	tU16 i;
	
	// flag the databuffer as unvalid 
	i2c0Valid = FALSE;
	
	// set the address to write to +1
	// since we are writing
	i2c0Addr = addr;
	
	// copy the data to write
	for (i = 0; i < len; i++){
		i2c0Buf[i] = buf[i];		
	} 	
	
	// set the data length
	i2c0Len = len;
	
	// reset the dta counter
	i2c0Seeker = 0;
	#ifdef DEBUG
	uart0SendString((unsigned char*) "Sending a Write Start...\n");
	#endif
	// set the STA bit
	I2C0_CONSET = 0x20;
}

/******************************************************************************
 *
 * Description:
 *    Start a reading from the specified address, 
 * 	  a start condition is generated, and the then the ISR routine takes over
 *    and carries on the communication
 * 
 * Params:
 *    [address to read from]
 *
 *****************************************************************************/

void
i2c0Read(tU16 addr, tU16 len)
{
	// flag the databuffer as unvalid
	i2c0Valid = FALSE;
	
	// reset the data counter
	i2c0Seeker = 0;
	
	// set the slave address to read from +0
	// since we are reading 
	i2c0Addr = addr + 1;

	// set the data lenght
	i2c0Len = len;
	#ifdef DEBUG
	uart0SendString((unsigned char*) "Sending a Read Start...\n");
	#endif	
	// set the STA bit
	I2C0_CONSET = 0x20;
		
	return;	
}

/******************************************************************************
 *
 * Description:
 *    Retrieve the specified data from the las received IMU,
 *    a read call is usually issued some times before this call 
 * 
 * Returns:
 *    [sensor value]
 *
 *****************************************************************************/
tU8
i2c0GetData(tU8 index)
{
	//wait status
	if (i2c0Valid == TRUE) {
		return i2c0Buf[index];
	} else {
		return NOTVALID;
	}
}

double i2c0GetTime()
{
	//wait status
	if (i2c0Valid == TRUE) {
		tU16 val;
		val = (i2c0Buf[7] << 8) + i2c0Buf[8];
		return (double)(val)*TIME_CONV_RATIO;
	} else {
		return NOTVALID;
	}
}

double i2c0GetGyroX()
{
	//wait status
	if (i2c0Valid == TRUE) {
		tS16 val;
		val = (i2c0Buf[13] << 8) + i2c0Buf[14];
		return (double)(val)* GYRO_CONV_RATIO;
	} else {
		return NOTVALID;
	}
}

double i2c0GetGyroY()
{
	if (i2c0Valid == TRUE) {
		tS16 val;
		val = (i2c0Buf[15] << 8) + i2c0Buf[16];
		return (double)(val)* GYRO_CONV_RATIO;
	} else {
		return NOTVALID;
	}
}

double i2c0GetGyroZ()
{
	if (i2c0Valid == TRUE) {
		tS16 val;
		val = (i2c0Buf[17] << 8) + i2c0Buf[18];
		return (double)(val)* GYRO_CONV_RATIO;
	} else {
		return NOTVALID;
	}
}

double i2c0GetAccX()
{
	if (i2c0Valid == TRUE) {
		tS16 val;
		val = (i2c0Buf[19] << 8) + i2c0Buf[20];
		return (double)(val)* ACC_CONV_RATIO;
	} else {
		return NOTVALID;
	}
}

double i2c0GetAccY()
{	
	if (i2c0Valid == TRUE) {
		tS16 val;
		val = (i2c0Buf[21] << 8) + i2c0Buf[22];
		return (double)(val)* ACC_CONV_RATIO;
	} else {
		return NOTVALID;
	}
}

double i2c0GetAccZ()
{
	if (i2c0Valid == TRUE) {
		tS16 val;
		val = (i2c0Buf[23] << 8) + i2c0Buf[24];
		return (double)(val)* ACC_CONV_RATIO;
	} else {
		return NOTVALID;
	}
}

double i2c0GetMagX()
{
	if (i2c0Valid == TRUE) {
		tS16 val;
		val = (i2c0Buf[25] << 8) + i2c0Buf[26];
		return (double)(val)* MAG_CONV_RATIO;
	} else {
		return NOTVALID;
	}
}

double i2c0GetMagY()
{
	if (i2c0Valid == TRUE) {
		tS16 val;
		val = (i2c0Buf[27] << 8) + i2c0Buf[28];
		return (double)(val)* MAG_CONV_RATIO;
	} else {
		return NOTVALID;
	}
}

double i2c0GetMagZ()
{
	if (i2c0Valid == TRUE) {
		tS16 val;
		val = (i2c0Buf[29] << 8) + i2c0Buf[30];
		return (double)(val)* MAG_CONV_RATIO;
	} else {
		return NOTVALID;
	}
}

double i2c0GetTempGyroX()
{
	if (i2c0Valid == TRUE) {
		tS16 val;
		val = (i2c0Buf[31] << 8) + i2c0Buf[32];
		return (double)(val)* TEMP_CONV_RATIO + TEMP_COMP;
	} else {
		return NOTVALID;
	}
}

double i2c0GetTempGyroY()
{
	if (i2c0Valid == TRUE) {
		tS16 val;
		val = (i2c0Buf[33] << 8) + i2c0Buf[34];
		return (double)(val)* TEMP_CONV_RATIO + TEMP_COMP;
	} else {
		return NOTVALID;
	}
}

double i2c0GetTempGyroZ()
{
	if (i2c0Valid == TRUE) {
		tS16 val;
		val = (i2c0Buf[35] << 8) + i2c0Buf[36];
		return (double)(val)* TEMP_CONV_RATIO + TEMP_COMP;
	} else {
		return NOTVALID;
	}
}
/******************************************************************************
 *
 * Description:
 *    Checks the I2C status.
 *
 *  Returns:
 *      00h Bus error
 *      08h START condition transmitted
 *      10h Repeated START condition transmitted
 *      18h SLA + W transmitted, ACK received
 *      20h SLA + W transmitted, ACK not received
 *      28h Data byte transmitted, ACK received
 *      30h Data byte transmitted, ACK not received
 *      38h Arbitration lost
 *      40h SLA + R transmitted, ACK received
 *      48h SLA + R transmitted, ACK not received
 *      50h Data byte received in master mode, ACK transmitted
 *      58h Data byte received in master mode, ACK not transmitted
 *      60h SLA + W received, ACK transmitted
 *      68h Arbitration lost, SLA + W received, ACK transmitted
 *      70h General call address received, ACK transmitted
 *      78h Arbitration lost, general call addr received, ACK transmitted
 *      80h Data byte received with own SLA, ACK transmitted
 *      88h Data byte received with own SLA, ACK not transmitted
 *      90h Data byte received after general call, ACK transmitted
 *      98h Data byte received after general call, ACK not transmitted
 *      A0h STOP or repeated START condition received in slave mode
 *      A8h SLA + R received, ACK transmitted
 *      B0h Arbitration lost, SLA + R received, ACK transmitted
 *      B8h Data byte transmitted in slave mode, ACK received
 *      C0h Data byte transmitted in slave mode, ACK not received
 *      C8h Last byte transmitted in slave mode, ACK received
 *      F8h No relevant status information, SI=0
 *      FFh Channel error
 *
 *****************************************************************************/
/*tU8
i2c0CheckStatus(void)
{
  tU8 status;

  //wait for I2C Status changed
  while( (I2C_I2CONSET & 0x08) == 0)   //while SI == 0
    ;

  //read I2C State
  status = I2C_I2STAT;

  //NOTE! SI flag is not cleared here

  return status;
}
*/
/******************************************************************************
 *
 * Description:
 *    Generates a start condition on I2C when bus is free.
 *    Master mode will also automatically be entered.
 *
 * Returns:
 *    I2C_CODE_OK or I2C status code
 *
 *****************************************************************************/
/*tS8 
i2cStart(void)
{
  tU8   status  = 0;
  tS8    retCode = 0;

  //issue a start condition
  I2C_I2CONSET |= 0x20;   //STA = 1, set start flag

  //wait until START transmitted
  while(1)
  {
    status = i2cCheckStatus();

    //start transmitted
    if((status == 0x08) || (status == 0x10))
    {
      retCode = I2C_CODE_OK;
      break;
    }

    //error
    else if(status != 0xf8)
    {
      retCode = (tS8 ) status;
      break;
    }

    else
    {
      //clear SI flag
      I2C_I2CONCLR = 0x08;
    }    
  }

  //clear start flag
  I2C_I2CONCLR = 0x20;

  return retCode;
}
*/
/******************************************************************************
 *
 * Description:
 *    Generates a start condition on I2C when bus is free.
 *    Master mode will also automatically be entered.
 *
 * Returns:
 *    I2C_CODE_OK or I2C status code
 *
 *****************************************************************************/
/*tS8 
i2cRepeatStart(void)
{
  tU8   status  = 0;
  tS8    retCode = 0;

  //issue a start condition
  I2C_I2CONSET |= 0x20;   //STA = 1, set start flag
  I2C_I2CONCLR = 0x08;    //clear SI flag

  //wait until START transmitted
  while(1)
  {
    status = i2cCheckStatus();

    //start transmitted
    if((status == 0x08) || (status == 0x10))
    {
      retCode = I2C_CODE_OK;
      break;
    }

    //error
    else if(status != 0xf8)
    {
      retCode = (tS8 ) status;
      break;
    }

    else
    {
      //clear SI flag
      I2C_I2CONCLR = 0x08;
    }    
  }

  //clear start flag
  I2C_I2CONCLR = 0x20;

  return retCode;
}
*/
/******************************************************************************
 *
 * Description:
 *    Generates a stop condition in master mode or recovers from an error
 *    condition in slave mode.
 *
 * Returns:
 *    I2C_CODE_OK
 *
 *****************************************************************************/
/*tS8 
i2cStop(void)
{
  I2C_I2CONSET |= 0x10;  //STO = 1, set stop flag
  I2C_I2CONCLR = 0x08;   //clear SI flag

  //wait for STOP detected (while STO = 1)
  while((I2C_I2CONSET & 0x10) == 0x10)
    ;

  return I2C_CODE_OK;
}
*/
/******************************************************************************
 *
 * Description:
 *    Sends a tS8acter on the I2C network
 *
 * Params:
 *    [in] data - the tS8acter to send
 *
 * Returns:
 *    I2C_CODE_OK   - successful
 *    I2C_CODE_BUSY - data register is not ready -> byte was not sent
 *
 *****************************************************************************/
/*tS8 
i2cPutChar(tU8 data)
{
  tS8  retCode = 0;

  //check if I2C Data register can be accessed
  if((I2C_I2CONSET & 0x08) != 0)    //if SI = 1
  {
    // send data 
    I2C_I2DAT    = data;
    I2C_I2CONCLR = 0x08;       //clear SI flag
    retCode    = I2C_CODE_OK;
  }
  else
  {
    //data register not ready
    retCode = I2C_CODE_BUSY;
  }

  return retCode;
}
*/
/******************************************************************************
 *
 * Description:
 *    Read a tS8acter. I2C master mode is used.
 *    This function is also used to prepare if the master shall generate
 *    acknowledge or not acknowledge.
 *
 * Params:
 *    [in]  mode  - I2C_MODE_ACK0 Set ACK=0. Slave sends next byte
 *                  I2C_MODE_ACK1 Set ACK=1. Slave sends last byte
 *                  I2C_MODE_READ Read data from data register
 *    [out] pData - a pointer to where the data shall be saved.
 *
 * Returns:
 *    I2C_CODE_OK    - successful
 *    I2C_CODE_EMPTY - no data is available
 *
 *****************************************************************************/
/*tS8 
i2cGetChar(tU8  mode,
           tU8* pData)
{
  tS8  retCode = I2C_CODE_OK;

  if(mode == I2C_MODE_ACK0)
  {
    //the operation mode is changed from master transmit to master receive
    //set ACK=0 (informs slave to send next byte)
    I2C_I2CONSET |= 0x04;   //AA=1
    I2C_I2CONCLR = 0x08;    //clear SI flag
  }
  else if(mode == I2C_MODE_ACK1)
  {
    //set ACK=1 (informs slave to send last byte)
    I2C_I2CONCLR = 0x04;     
    I2C_I2CONCLR = 0x08;   //clear SI flag
  }
  else if(mode == I2C_MODE_READ)
  {
    //check if I2C Data register can be accessed
    if((I2C_I2CONSET & 0x08) != 0)    //SI = 1
    {
      //read data
      *pData = (tU8)I2C_I2DAT;
    }
    else
    {
      //No data available
      retCode = I2C_CODE_EMPTY;
    }
  }

  return retCode;
}
*/

/******************************************************************************
 *
 * Description:
 *    Wait after transmission of a byte until done
 *
 * Returns:
 *    I2C_CODE_OK    - successful / done
 *    I2C_CODE_ERROR - an error occured
 *
 *****************************************************************************/
/*tS8 
i2cWaitAfterTransmit(void)
{
  tU8 status = 0;

  //wait until data transmitted
  while(1)
  {
    //get new status
    status = i2cCheckStatus();

    // 
    // SLA+W transmitted, ACK received or
    // data byte transmitted, ACK received
    //
    if( (status == 0x18) || (status == 0x28) )
    {
      //data transmitted and ACK received
      return I2C_CODE_OK;
    }

    //no relevant status information
    else if(status != 0xf8 )
    {
      //error
      return I2C_CODE_ERROR;
    }
  }

}
*/

/******************************************************************************
 *
 * Description:
 *    Sends a tS8acter on the I2C network and wait until done
 *
 * Params:
 *    [in] data - the tS8acter to send
 *
 * Returns:
 *    I2C_CODE_OK    - successful
 *    I2C_CODE_BUSY  - data register is not ready -> byte was not sent
 *    I2C_CODE_ERROR - an error occured
 *
 *****************************************************************************/
/*
tS8 
i2cPutCharAndWait(tU8 data)
{
  tS8  retCode;
  
  retCode = i2cPutChar(data);
  while(retCode == (tS8 )I2C_CODE_BUSY)
    retCode = i2cPutChar(data);

  if(retCode == I2C_CODE_OK)
    retCode = i2cWaitAfterTransmit();

  return retCode;
}

*/
/******************************************************************************
 *
 * Description:
 *    Sends data on the I2C network
 *
 *    Note: No stop condition is generated after the transmission - this must
 *          be done after calling this function.
 *
 * Params:
 *    [in] addr     - address
 *    [in] extraCmd - controls if 0, 1 or 2 extra bytes shall be transmitted
 *                    before data buffer
 *    [in] extra    - byte of word to be transmitted before data buffer
 *    [in] pData    - data to transmit
 *    [in] len      - number of bytes to transmit
 *
 * Returns:
 *    I2C_CODE_OK    - successful
 *    I2C_CODE_ERROR - an error occured
 *
 *****************************************************************************/
/* 
tS8 
i2cWrite(tU8  addr,
         tU8  extraCmd,
         tU16 extra,
         tU8* pData,
         tU16 len)
{
  tS8  retCode = 0;
  tU8 i       = 0;

  do
  {
    //generate Start condition
    retCode = i2cStart();
    if(retCode != I2C_CODE_OK)
      break;
    
    //Transmit slave address
    retCode = i2cPutCharAndWait(addr);
    if(retCode != I2C_CODE_OK)
      break;

    //Transmit MSB of extra word (if wanted)
   	if (extraCmd == I2C_EXTRA_WORD)
  	{
      retCode = i2cPutCharAndWait((tU8)(extra >> 8));
      if(retCode != I2C_CODE_OK)
        break;
    }

    //Transmit LSB of extra work (if wanted)
   	if ((extraCmd == I2C_EXTRA_BYTE) || (extraCmd == I2C_EXTRA_WORD))
   	{
      retCode = i2cPutCharAndWait((tU8)(extra & 0xff));
      if(retCode != I2C_CODE_OK)
        break;
	  }

    //wait until address transmitted and transmit data
    for(i = 0; i < len; i++)
    {
      retCode = i2cPutCharAndWait(*pData);
      if(retCode != I2C_CODE_OK)
        break;
      pData++;
    }
  } while(0);

  return retCode;
}
*/

/******************************************************************************
 *
 * Description:
 *    Waits till slave device returns ACK (after busy period)
 *
 * Params:
 *    [in] addr  - address
 *
 * Returns:
 *    I2C_CODE_OK or I2C_CODE_ERROR
 *
 *****************************************************************************/
/* 
tS8  
i2cPoll(tU8 addr)
{
  tS8  retCode  = I2C_CODE_OK;
  tU8 status   = 0;
  volatile tU8 deviceReady = FALSE;

  while(deviceReady == FALSE)
  {
    //Generate Start condition
    retCode = i2cStart();

    //Transmit SLA+W
    if(retCode == I2C_CODE_OK)
    {
      //write SLA+W
      retCode = i2cPutChar(addr);
      while(retCode == (tS8 )I2C_CODE_BUSY)
        retCode = i2cPutChar(addr);
    }

    if(retCode == I2C_CODE_OK)
    {
      //Wait until SLA+W transmitted
      //Get new status
      status = i2cCheckStatus();

      if(status == 0x18)
      {
        //data transmitted and ACK received
        deviceReady = TRUE;
      }
      else if(status == 0x20)
      {
        //data transmitted and ACK not received
        //send start bit, start again
        deviceReady = FALSE;
      }
      else if( status != 0xf8 )
      {
        //error
        retCode = I2C_CODE_ERROR;
        deviceReady = TRUE;
      }
    }

    //Generate Stop condition
    i2cStop();
  }

  return retCode;
}
*/

/******************************************************************************
 *
 * Description:
 *    Read a specified number of bytes from the I2C network.
 *
 *    Note: No start condition is generated in the beginningis must be
 *          done before calling this function.
 *
 * Params:
 *    [in] addr - address
 *    [in] pBuf - receive buffer
 *    [in] len  - number of bytes to receive
 *
 * Returns:
 *    I2C_CODE_OK or I2C status code
 *
 *****************************************************************************/
/* 
tS8 
i2cRead(tU8  addr,
        tU8* pBuf,
        tU16 len)
{
  tS8  retCode = 0;
  tU8 status  = 0;
  tU8 i       = 0;

  //write SLA+R
  retCode = i2cPutChar(addr);
  while(retCode == (tS8 )I2C_CODE_BUSY){
   
    retCode = i2cPutChar(addr);
  }
  if(retCode == I2C_CODE_OK )
  {
    //wait until address transmitted and receive data
    for(i = 1; i <= len; i++ )
    {
      //wait until data transmitted
      while(1)
      {
        //get new status
        status = i2cCheckStatus();

        // SLA+R transmitted, ACK received or
        // SLA+R transmitted, ACK not received
        // data byte received in master mode, ACK transmitted

         //printf("  status %x\n",status);
        if((status == 0x40 ) || (status == 0x48 ) || (status == 0x50 ))
        {
          //data received
          if(i == len)
          {
            //Set generate NACK
            retCode = i2cGetChar(I2C_MODE_ACK1, pBuf);
          }
          else
          {
            retCode = i2cGetChar(I2C_MODE_ACK0, pBuf);
          }

          //Read data
          retCode = i2cGetChar(I2C_MODE_READ, pBuf);
          while(retCode == (tS8 )I2C_CODE_EMPTY){
            retCode = i2cGetChar(I2C_MODE_READ, pBuf);
          	
          }
          //printf("\nbyte[%d] = 0x%x",i,*pBuf);
          pBuf++;
          break;
        }

        //no relevant status information
        else if(status != 0xf8 )
        {
          //error
          i = len;
          retCode = I2C_CODE_ERROR;
          break;
        }
      }
    }
  }

  //Generate Stop condition
  i2cStop();

  return retCode;
}
*/



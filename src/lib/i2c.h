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
#ifndef _I2C_H
#define _I2C_H

/******************************************************************************
 * Includes
 *****************************************************************************/
 #include "datatypes.h"
 #include "../startup/config.h"
 
/******************************************************************************
 * Defines, macros, and typedefs
 *****************************************************************************/
//Return codes
#define I2C_CODE_OK     1
#define I2C_CODE_ERROR  0
#define I2C_CODE_EMPTY -1
#define I2C_CODE_BUSY  -2

//Command codes for transmitting extra bytes
#define I2C_EXTRA_NONE 0
#define I2C_EXTRA_BYTE 1
#define I2C_EXTRA_WORD 2

#define NOTVALID 99

/******************************************************************************
 * Public functions
 *****************************************************************************/
void i2c0Init(tU32 frequency);
void i2c1Init(tU32 frequency);

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
void i2c0Write(tU16  addr,char* buf, tU16 len);

/******************************************************************************
 *
 * Description:
 *    Start a reading from the specified address, 
 * 	  a start condition is generated, and the then the ISR routine takes over
 *    and carries on the communication
 * 
 * Params:
 *    [address to read from, lenght of the data to read]
 *
 *****************************************************************************/

void i2c0Read(tU16  addr, tU16 len); 

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
tU8 i2c0GetData(tU8 index);
double i2c0GetTime();
double i2c0GetGyroX();
double i2c0GetGyroY();
double i2c0GetGyroZ();
double i2c0GetAccX();
double i2c0GetAccY();
double i2c0GetAccZ();
double i2c0GetMagX();
double i2c0GetMagY();
double i2c0GetMagZ();
double i2c0GetTempGyroX();
double i2c0GetTempGyroY();
double i2c0GetTempGyroZ();

#endif

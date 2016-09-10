/******************************************************************************
 *
 * CMD LIBRARY header
 * 
 * File:
 *    cmd.h
 *
 * Description:
 *    Simple library to define and parse the commands
 *    accepted and sent trough the serial port
 * 
 * Helistix software package
 *   
 * Copyright:
 *    (C) 2006 Renzo De Nardi rdenar@essex.ac.uk 
 * 					
 * 
 *****************************************************************************/


#ifndef CMD_H_
#define CMD_H_

/******************************************************************************
 * Includes
 *****************************************************************************/

/******************************************************************************
 * Defines and typedefs
 *****************************************************************************
 
 
 	+-----+-----+-----+-----+-----+-----+-----+-...-+-----+-----+-----+-----+
 	|START|LGTH0|LGTH1| CMD |val1 |val1 |val1 | ... |valn |valn |valn |  <  | 	
 	+-----+-----+-----+-----+-----+-----+-----+-...-+-----+-----+-----+-----+
 
 	@ is the '@' character used as separator
 	LGTHn is the lenght of the command`s payload which is the number of bytes from CMD to the end
 	
*/ 
#define CMD_MINL	8		// minimum length of a command
 
#define CMD_START '>'		// one of these bytes in a row identify the start of a command 

#define CMD_STOP '<'			// stop command

#define CMD_SM	 'M'			// command which sets the servos and the motors
#define CMD_SM_L	13	    // command M lenght

struct SMstct {
	
	tS16 m1;		// M1 value
	tS16 m2;		// M2 value
	tS16 sl;		// servo left value
	tS16 sr;		// servo right value
};

#endif /*CMD_H_*/

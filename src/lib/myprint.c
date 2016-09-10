#include "uart.h"
#include "myprint.h"

/*****************************************************************************
 *
 * Description:
 *    Routine for printing integer numbers in various formats. The number is 
 *    printed in the specified 'base' using exactly 'noDigits', using +/- if 
 *    signed flag 'sign' is TRUE, and using the character specified in 'pad' 
 *    to pad extra characters. 
 *
 * Params:
 *    [in] base     - Base to print number in (2-16) 
 *    [in] noDigits - Number of digits to print (max 32) 
 *    [in] sign     - Flag if sign is to be used (TRUE), or not (FALSE) 
 *    [in] pad      - Character to pad any unused positions 
 *    [in] number   - Signed number to print 
 *

 ****************************************************************************/
 
void printNumber(tU8  base,
            tU8  noDigits,
            tU8  sign,
            tU8  pad,
            tS32 number)
{
  static tU8  hexChars[] = "0123456789ABCDEF";
  tU8        *pBuf;
  tU8         buf[32];
  tU32        numberAbs;
  tU32        count;

  // prepare negative number
  if(sign && (number < 0))
    numberAbs = -number;
  else
    numberAbs = number;

  // setup little string buffer
  count = (noDigits - 1) - (sign ? 1 : 0);
  pBuf = buf + sizeof(buf);
  *--pBuf = '\0';

  // force calculation of first digit
  // (to prevent zero from not printing at all!!!)
  *--pBuf = hexChars[(numberAbs % base)];
  numberAbs /= base;

  // calculate remaining digits
  while(count--)
  {
    if(numberAbs != 0)
    {
      //calculate next digit
      *--pBuf = hexChars[(numberAbs % base)];
      numberAbs /= base;
    }
    else
      // no more digits left, pad out to desired length
      *--pBuf = pad;
  }

  // apply signed notation if requested
  if(sign)
  {
    if(number < 0)
      *--pBuf = '-';
    else if(number > 0)
       *--pBuf = '+';
    else
       *--pBuf = ' ';
  }

  // print the string right-justified
  uart0SendString(pBuf);
}

void printN(tS32 number){
	
	printNumber(3, 6,TRUE,' ',number);
	
}

void printC(tU8 chr){

	unsigned char buf[2];
	buf[0]=chr;
	buf[1]='\0';
	
	uart0SendString(buf);	
}
	
void printX(tU8 chr){

	//uart0SendString("0x");
	printNumber(16,2,FALSE,'0',chr);
		
}	

void printNf(float number){
	
	static tU8 noDigits = 2;
	static tU8 noDecDigits = 4;
  	tU8        *pBuf;
  	tU8         buf[32];
  	float     numberAbs;
  	tU32        i,j,div,intNumberAbs;
  
  	// prepare negative number
  	if(number < 0)
    	numberAbs = -number;
  	else
    	numberAbs = number;

  	// setup little string buffer  	
  	pBuf = buf + sizeof(buf);
  	*--pBuf = '\0';
	
  	// calculate remaining digits
  	for(i = noDecDigits; i > 0; i--  ){
  		
  		div = 1;
  		
  		for( j = 1 ; j<i+1; j++)
  			div *= 10;	

  		intNumberAbs = (int) (numberAbs * div);
    	
    	*--pBuf = (int)(intNumberAbs % 10) + '0';
  	}
  	
  	*--pBuf ='.';
  	
  	intNumberAbs = (int) numberAbs;
  	
  	for(i = noDigits; i > 0; i--  ){ 
  		
    	if(intNumberAbs != 0){
      		//calculate next digit
      		*--pBuf = (int)(intNumberAbs % 10) + '0';
      		intNumberAbs /= 10;
    	}
    	else
      		// no more digits left, pad out to desired length
      		*--pBuf = '0';
  	}

	// apply signed notation
   	if(number < 0)
   		*--pBuf = '-';
 	else 
   		*--pBuf = '0';

	// print the string right-justified
	uart0SendString(pBuf);
}

void printS(char *buf){
	
	uart0SendString((unsigned char*)buf);	
}

/*
*******************************************************
Vector and matrix display utilities
*******************************************************
*/
//Display a matrix on screen (for debugging only)
/*
void matPrint(gsl_matrix *m) {
	unsigned int i, j;
	double tmp;

	for (i=0; i<m->size1; i++) {
		for (j=0; j<m->size2; j++) {
			tmp = gsl_matrix_get(m, i,j);
			if(tmp < 0){
				printNf(tmp);
				printC('\t');
			} else {
				printNf(tmp);
				printC('\t');
			}	
		}
		printC('\n');
	}   
	printC('\n');
}

//Display a vector on screen (for debugging only)
void vecPrint(gsl_vector *v) {
	unsigned int i;
	double tmp;

	for (i=0; i<v->size; i++) {	
		tmp = gsl_vector_get(v, i);
		if(tmp < 0){
			printNf(tmp);
			printC('\t');
		} else {
			printNf(tmp);
			printC('\t');
		}	
	}
	printC('\n');
}

*/

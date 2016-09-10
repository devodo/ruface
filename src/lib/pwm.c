#include "pwm.h"
#include "../startup/config.h"
#include "../startup/lpc2xxx.h"

/*****************************************************************************
 *
 * Description:
 *    Initialize the PWM unit to generate a variable duty cycle signal on
 *    PWM2. Connect signal PWM2 to pin P0.7.
 *    The function sets initial frequency. Initial duty cucle is set to 0%.
 *
 * Params:
 *    [in] initialFreqValue - the initial frequency value. Value calculated as:
 *
 *                     (crystal frequency * PLL multiplication factor)
 *                     -----------------------------------------------
 *                           (VPBDIV factor * desired frequency)
 *
 ****************************************************************************/
void
initPwm(tU32 initialFreqValue)
{
  /*
   * initialize PWM
   */
  PWM_PR  = 0x00000000;             //set prescale to 0
  PWM_MCR = 0x0002;                 //counter resets on MR0 match (period time)
  PWM_MR0 = initialFreqValue;       //MR0 = period cycle time
  PWM_MR2 = 0;                      //MR2 = duty cycle control, initial = 0%
  PWM_LER = 0x05;                   //latch new values for MR0 and MR2
  PWM_PCR = 0x0400;                 //enable PWM2 in single edge control mode
  PWM_TCR = 0x09;                   //enable PWM and Counter

  /*
   * connect signal PWM2 to pin P0.7
   */
  PINSEL0 &= ~0x0000c000;  //clear bits related to P0.7
  PINSEL0 |=  0x00008000;  //connect signal PWM2 to P0.7 (second alternative function)
}

/*****************************************************************************
 *
 * Description:
 *    Update the duty cycle value of the PWM signal.
 *
 * Params:
 *    [in] dutyValue - the new duty cycle value. Value calculated as:
 *    (crystal frequency * PLL multiplication factor)
 *    ----------------------------------------------- * duty cycle in percent
 *           (VPBDIV factor * PWM frequency)
 *
 *                    or
 *
 *    value in MR0 register * duty cycle in percent
 *
 ****************************************************************************/
void
setPwmDuty(tU32 dutyValue)
{
  PWM_MR2 = dutyValue;    //update duty cycle
  PWM_LER = 0x04;         //latch new values for MR2
}

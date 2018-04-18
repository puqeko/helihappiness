/**********************************************************
 *
 * pwmGen.c - Example code which generates a single PWM
 *    output on J4-05 (M0PWM7) with variable duty cycle and
 *    fixed frequency in
 *    the range 50 Hz to 400 Hz.
 * 2018: Modified by Ryan Hall for helicopter application.
 *
 * P.J. Bones   UCECE
 * Last modified:  18.4.2018
 **********************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/pin_map.h" //Needed for pin configure
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/systick.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "buttons4.h"

#include "inc/hw_ints.h"
#include "driverlib/adc.h"
#include "driverlib/debug.h"
#include "utils/ustdlib.h"
#include "stdlib.h"
#include "OrbitOLED_2/OrbitOLEDInterface.h"
#include "pwmModule.h"

/**********************************************************
 * Generates a single PWM signal on Tiva board pin J4-05 =
 * PC5 (M0PWM7).  This is the same PWM output as the
 * helicopter main rotor.
 **********************************************************/

/*******************************************
 *      Globals
 *******************************************/





/***********************************************************
 * Initialisation functions: clock, SysTick, PWM
 ***********************************************************
 * Clock
 ***********************************************************/
void
initClocks (void)
{
    // Set the PWM clock rate (using the prescaler)
    SysCtlPWMClockSet(PWM_DIVIDER_CODE);
}

/*************************************************************
 * SysTick interrupt
 ************************************************************/


/*********************************************************
 * initialisePWM
 * M0PWM7 (J4-05, PC5) is used for the main rotor motor
 *********************************************************/
void
initialisePWM (void)
{
    SysCtlPeripheralEnable(PWM_MAIN_PERIPH_PWM);
    SysCtlPeripheralEnable(PWM_TAIL_PERIPH_PWM);
    SysCtlPeripheralEnable(PWM_MAIN_PERIPH_GPIO);
    SysCtlPeripheralEnable(PWM_TAIL_PERIPH_GPIO);

    GPIOPinConfigure(PWM_MAIN_GPIO_CONFIG);
    GPIOPinConfigure(PWM_TAIL_GPIO_CONFIG);
    GPIOPinTypePWM(PWM_MAIN_GPIO_BASE, PWM_MAIN_GPIO_PIN);
    GPIOPinTypePWM(PWM_TAIL_GPIO_BASE, PWM_TAIL_GPIO_PIN);

    PWMGenConfigure(PWM_MAIN_BASE, PWM_MAIN_GEN,
                    PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenConfigure(PWM_TAIL_BASE, PWM_TAIL_GEN,
                        PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
    // Set the initial PWM parameters ***
    setPWM (PWM_START_RATE_HZ, PWM_FIXED_DUTY, MAIN_ROTOR);
    setPWM (PWM_START_RATE_HZ, PWM_FIXED_DUTY, TAIL_ROTOR);

    PWMGenEnable(PWM_MAIN_BASE, PWM_MAIN_GEN);
    PWMGenEnable(PWM_TAIL_BASE, PWM_TAIL_GEN);

    // Disable the output.  Repeat this call with 'true' to turn O/P on.
    PWMOutputState(PWM_MAIN_BASE, PWM_MAIN_OUTBIT, false);
    PWMOutputState(PWM_TAIL_BASE, PWM_TAIL_OUTBIT, false);
}

/********************************************************
 * Function to set the freq, duty cycle of M0PWM7
 ********************************************************/
void
setPWM (uint32_t ui32Freq, uint32_t ui32Duty, enum rotor_type rotorType)
{
    uint32_t ui32Period;
    switch(rotorType)
    {
    case MAIN_ROTOR:
        // Calculate the PWM period corresponding to the freq.
        ui32Period = SysCtlClockGet() / PWM_DIVIDER / ui32Freq;
        PWMGenPeriodSet(PWM_MAIN_BASE, PWM_MAIN_GEN, ui32Period);
        PWMPulseWidthSet(PWM_MAIN_BASE, PWM_MAIN_OUTNUM, ui32Period * ui32Duty / 100);
        break;
    case TAIL_ROTOR:
        // Calculate the PWM period corresponding to the freq.
        ui32Period = SysCtlClockGet() / PWM_DIVIDER / ui32Freq;
        PWMGenPeriodSet(PWM_TAIL_BASE, PWM_TAIL_GEN, ui32Period);
        PWMPulseWidthSet(PWM_TAIL_BASE, PWM_TAIL_OUTNUM, ui32Period * ui32Duty / 100);
        break;
    }
}

void
pwmSetDuty (uint32_t ui32Duty, enum rotor_type rotorType)
{
    uint32_t ui32Period;
    switch(rotorType)
    {
    case MAIN_ROTOR:
        // Calculate the PWM period corresponding to the freq.
        ui32Period = SysCtlClockGet() / PWM_DIVIDER / PWM_START_RATE_HZ;
        PWMGenPeriodSet(PWM_MAIN_BASE, PWM_MAIN_GEN, ui32Period);
        PWMPulseWidthSet(PWM_MAIN_BASE, PWM_MAIN_OUTNUM, ui32Period * ui32Duty / 100);
        break;
    case TAIL_ROTOR:
        // Calculate the PWM period corresponding to the freq.
        ui32Period = SysCtlClockGet() / PWM_DIVIDER / PWM_START_RATE_HZ;
        PWMGenPeriodSet(PWM_TAIL_BASE, PWM_TAIL_GEN, ui32Period);
        PWMPulseWidthSet(PWM_TAIL_BASE, PWM_TAIL_OUTNUM, ui32Period * ui32Duty / 100);
        break;
    }
}

//*****************************************************************************
// Initialisation functions: clock, display
//*****************************************************************************
void
initClock (void)
{
    // Set the clock rate to 20 MHz
    SysCtlClockSet (SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);
}

void
pwmSetOutput (bool state, enum rotor_type rotorType)
{
    switch(rotorType)
    {
    case MAIN_ROTOR:
        // Calculate the PWM period corresponding to the freq.
        PWMOutputState(PWM_MAIN_BASE, PWM_MAIN_OUTBIT, state);
        break;
    case TAIL_ROTOR:
        // Calculate the PWM period corresponding to the freq.
        PWMOutputState(PWM_TAIL_BASE, PWM_TAIL_OUTBIT, state);
        break;
    }

}


// *******************************************************


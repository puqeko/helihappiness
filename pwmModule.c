/**********************************************************
 *
 * pwmModule.c - Generates multiple PWM outputs, with variable
 * duty cycle, to control the main and tail rotor.
 *
 * P.J. Bones   UCECE
 * Modified by Ryan H and Thomas M 23.4.2018.
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
#include "pwmModule.h"

// Systick configuration
#define SYSTICK_RATE_HZ    100

// PWM configuration
#define PWM_START_RATE_HZ  250
#define PWM_START_DUTY     10  // %
#define PWM_DIVIDER_CODE   SYSCTL_PWMDIV_4
#define PWM_DIVIDER        4

//  PWM Hardware Details M0PWM7 (gen 3)
//  ---Main Rotor PWM: PC5, J4-05
#define PWM_MAIN_BASE        PWM0_BASE
#define PWM_MAIN_GEN         PWM_GEN_3
#define PWM_MAIN_OUTNUM      PWM_OUT_7
#define PWM_MAIN_OUTBIT      PWM_OUT_7_BIT
#define PWM_MAIN_PERIPH_PWM  SYSCTL_PERIPH_PWM0
#define PWM_MAIN_PERIPH_GPIO SYSCTL_PERIPH_GPIOC
#define PWM_MAIN_GPIO_BASE   GPIO_PORTC_BASE
#define PWM_MAIN_GPIO_CONFIG GPIO_PC5_M0PWM7
#define PWM_MAIN_GPIO_PIN    GPIO_PIN_5

#define PWM_TAIL_BASE        PWM1_BASE
#define PWM_TAIL_GEN         PWM_GEN_2
#define PWM_TAIL_OUTNUM      PWM_OUT_5
#define PWM_TAIL_OUTBIT      PWM_OUT_5_BIT
#define PWM_TAIL_PERIPH_PWM  SYSCTL_PERIPH_PWM1
#define PWM_TAIL_PERIPH_GPIO SYSCTL_PERIPH_GPIOF
#define PWM_TAIL_GPIO_BASE   GPIO_PORTF_BASE
#define PWM_TAIL_GPIO_CONFIG GPIO_PF1_M1PWM5
#define PWM_TAIL_GPIO_PIN    GPIO_PIN_1


// M0PWM7 (J4-05, PC5) is used for the main rotor motor
void pwmInit(void)
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
    setPWM (PWM_START_RATE_HZ, PWM_START_DUTY, MAIN_ROTOR);
    setPWM (PWM_START_RATE_HZ, PWM_START_DUTY, TAIL_ROTOR);

    PWMGenEnable(PWM_MAIN_BASE, PWM_MAIN_GEN);
    PWMGenEnable(PWM_TAIL_BASE, PWM_TAIL_GEN);

    // Disable the output.  Repeat this call with 'true' to turn O/P on.
    PWMOutputState(PWM_MAIN_BASE, PWM_MAIN_OUTBIT, false);
    PWMOutputState(PWM_TAIL_BASE, PWM_TAIL_OUTBIT, false);

    // Set the PWM clock rate (using the prescaler)
    SysCtlPWMClockSet(PWM_DIVIDER_CODE);
}


// take a duty cycle, as a percentage from 0 to 100, a precision multiplier (i.e. a multiplier of 100 means
// the supplied duty cycle is a factor of 100 larger than needs and should be divided down), and an enum
// with the name of the channel to change the pwm duty cycle of.
void pwmSetDuty(uint32_t dutyPercent, uint32_t precision, pwm_channel channel)
{
    // Calculate the PWM period corresponding to the freq.
    uint32_t period = SysCtlClockGet() / PWM_DIVIDER / PWM_START_RATE_HZ;
    switch(channel)
    {
    case MAIN_ROTOR:
        PWMGenPeriodSet(PWM_MAIN_BASE, PWM_MAIN_GEN, period);
        PWMPulseWidthSet(PWM_MAIN_BASE, PWM_MAIN_OUTNUM, period * dutyPercent / 100  / precision);
        break;
    case TAIL_ROTOR:
        PWMGenPeriodSet(PWM_TAIL_BASE, PWM_TAIL_GEN, period);
        PWMPulseWidthSet(PWM_TAIL_BASE, PWM_TAIL_OUTNUM, period * dutyPercent / 100 / precision);
        break;
    }
}


// enable or disable output from a given channel.
void pwmSetOutputState(bool state, pwm_channel channel)
{
    switch(channel)
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


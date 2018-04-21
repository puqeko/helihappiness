#ifndef PWM_MODULE_H_
#define PWM_MODULE_H_

//************************************************************************
// pwmModule.c
//
// Helicopter project
//
// Group:       A03 Group 10
// Created:     18/4/18
// Last Edited: 18/4/18
//
// Purpose: control PWM output to rotors
//
//************************************************************************

/**********************************************************
 * Constants
 **********************************************************/
// Systick configuration
#define SYSTICK_RATE_HZ    100

// PWM configuration
#define PWM_START_RATE_HZ  250
#define PWM_RATE_STEP_HZ   5
#define PWM_RATE_MIN_HZ    50
#define PWM_RATE_MAX_HZ    400
#define PWM_FIXED_DUTY     10
#define PWM_DIVIDER_CODE   SYSCTL_PWMDIV_4
#define PWM_DIVIDER        4

#define PWM_DUTY_MAX_HZ 95
#define PWM_DUTY_MIN_HZ 5
#define PWM_DUTY_STEP_HZ 5
#define PWM_START_DUTY_HZ 10

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

enum rotor_type {MAIN_ROTOR, TAIL_ROTOR};

void
initClocks (void);

void
initialisePWM (void);

void
setPWM (uint32_t ui32Freq, uint32_t ui32Duty, enum rotor_type rotorType);

void
pwmSetDuty (uint32_t ui32Duty,  enum rotor_type rotorType);

void
initClock (void);

void
pwmSetOutput (bool state, enum rotor_type rotorType);

#endif /*PWM_MODULE_H_*/

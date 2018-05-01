#ifndef PWM_MODULE_H_
#define PWM_MODULE_H_

//************************************************************************
// pwmModule.h
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

typedef enum pwm_channel_t {
    MAIN_ROTOR, TAIL_ROTOR
} pwm_channel;

void pwmInit(void);

// take a duty cycle, as a percentage from 0 to 100, a precision multiplier (i.e. a multiplier of 100 means
// the supplied duty cycle is a factor of 100 larger than needs and should be divided down), and an enum
// with the name of the channel to change the pwm duty cycle of.
void pwmSetDuty(uint32_t dutyPercent, uint32_t precision, pwm_channel channel);

// enable or disable output from a given channel.
void pwmSetOutputState(bool state, pwm_channel channel);

#endif /*PWM_MODULE_H_*/

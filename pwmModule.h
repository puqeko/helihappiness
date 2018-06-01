// ************************************************************
// pwmModule.h
// Helicopter project
// Group: A03 Group 10
// Created by P.J. Bones UCECE
// Last edited: 23-04-2018 by Thomas M
//
// Purpose: Generates multiple PWM outputs, with variable
// duty cycle, to control the main and tail rotor.
// ************************************************************

#ifndef PWM_MODULE_H_
#define PWM_MODULE_H_


// two PWM channels are defined
typedef enum pwm_channel {
    MAIN_ROTOR, TAIL_ROTOR
} pwm_channel_t;


void pwmInit(void);


// take a duty cycle, as a percentage from 0 to 100, a precision multiplier (i.e. a multiplier of 100 means
// the supplied duty cycle is a factor of 100 larger than needs and should be divided down), and an enum
// with the name of the channel to change the pwm duty cycle of.
void pwmSetDuty(uint32_t dutyPercent, uint32_t precision, pwm_channel_t channel);


// enable or disable output from a given channel.
void pwmSetOutputState(bool state, pwm_channel_t channel);

#endif /*PWM_MODULE_H_*/

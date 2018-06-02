// ************************************************************
// control.h
// Helicopter project
// Group: A03 Group 10
// Last edited: 2-06-2018
//
// Purpose: Control the helicopter using the tail and main rotor.
// Explaination: Control the output to the two helicoptor motors. This module
// preforms PID control by enabling and disabling channels. The motor duty cycle
// is limited to between 5 % and 95 %, otherwise the motors are off and PWM output
// is disabled. Different channels will be active depending on the helicopter's
// current mode. Channels use pwmModule.h with yaw.h and height.h in a feedback loop.
// ************************************************************

#ifndef CONTROL_H_
#define CONTROL_H_

#include <stdint.h>
#include <stdbool.h>

#include "stateInfo.h"  // needs to know about state_t

#define PRECISION 1000  // zeros represent how many dp of precision to get with integer math
#define CONTROL_MIN_DUTY 5  // % duty cycle for motors
#define CONTROL_MAX_DUTY 95  // % duty cycle for motors
#define CONTROL_DESCEND_SPEED 7  // % per second, for controlling decent speed when landing


// Define channels for applying control on motors. Controls can be enabled and disabled.
// If all channels are disabled, the motors are also disabled.
typedef enum {
    CONTROL_HEIGHT=0,
    CONTROL_YAW,
    CONTROL_DESCENDING,
    CONTROL_POWER_DOWN,

    // Always equal to the number of channels above
    CONTROL_NUM_CHANNELS
} control_channel_t;


// Give access to the pwm duty cycles for displaying.
typedef enum {
    CONTROL_DUTY_MAIN,
    CONTROL_DUTY_TAIL
} control_duty_t;


// Initalise PWM outputs and motors
void controlInit(void);


// Resets the integral components of the controllers between runs
void controlReset(void);


// Sets the specified control channel to be enabled. If all channels were previously
// disabled, then the pwm signal output will be enabled in the next call to controlUpdate(..).
void controlEnable(state_t* state, control_channel_t channel);


// Sets the specified control channel to be disabled. If all channels become disabled
// then the pwm output will be disabled in the next call to controlUpdate(..)
void controlDisable(state_t* state, control_channel_t channel);


// Return true if the specified control channel is currently enabled.
// All channels are disabled by default.
bool controlIsEnabled(control_channel_t channel);


// Checks which channels are active and applies the associated controls to the
// main and tail pwm rotor output for the helicopter. If all channels are disabled,
// then the pwm output is also disabled. Takes a state object contiaining the target
// yaw and height and the time between updates in milliseconds as arguments.
void controlUpdate(state_t* state, uint32_t deltaTime);


#endif /* CONTROL_H_ */

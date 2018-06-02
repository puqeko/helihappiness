#ifndef CONTROL_H_
#define CONTROL_H_

// *******************************************************
// pidControl.h
//
// Generates serial data to print for debugging
// P.J. Bones UCECE, modified by Ryan hall
// Last modified:  18.04.2017
// Helicopter project
// Group: A03 Group 10
// *******************************************************

#include <stdint.h>
#include <stdbool.h>
#include "kernalMustardWithThePipeInTheDiningRoom.h"

#define PRECISION 1000  // zeros represent how many dp of precision to get with integer math
#define MS_TO_SEC 1000  // number of ms in one s
#define MIN_DUTY 5  // %
#define MAX_DUTY 95  // %
#define DUTY_DECREMENT_PER_SECOND 7  // % per second, for controlling decent speed when landing


typedef enum control_channel {
    CONTROL_HEIGHT=0,
    CONTROL_YAW,
    CONTROL_DESCENDING,
    CONTROL_POWER_DOWN,

    // Always equal to the number of channels above
    CONTROL_NUM_CHANNELS
} control_channel_t;


// Give access to the pwm duty cycles for displaying.
typedef enum control_duty {
    CONTROL_DUTY_MAIN,
    CONTROL_DUTY_TAIL
} control_duty_t;


void controlInit(void);


void controlEnable(control_channel_t channel);


void controlDisable(control_channel_t channel);


bool controlIsEnabled(control_channel_t channel);


void controlUpdate(state_t* state, uint32_t deltaTime);


void controlReset(void);


int32_t controlGetPWMDuty(control_duty_t channel);


#endif /* CONTROL_H_ */

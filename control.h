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
#include "pwmModule.h"

#define PRECISION 1000  // zeros represent how many dp of precision to get with integer math
#define MIN_DUTY 5  // %
#define MAX_DUTY 95  // %
#define MS_TO_SEC 1000
#define LANDING_DUTY (25 * PRECISION)
#define LANDING_RATE 25
#define DUTY_DECREMENT_PER_SECOND 7
#define DUTY_DECREMENT_PER_CYCLE (DUTY_DECREMENT_PER_SECOND * PRECISION / MS_TO_SEC)

typedef enum control_channel {
    CONTROL_HEIGHT=0, CONTROL_YAW, CONTROL_CALIBRATE_MAIN, CONTROL_CALIBRATE_TAIL, CONTROL_NUM_CHANNELS
} control_channel_t;

void controlInit(void);

void controlMotorSet(bool state, pwm_channel_t channel);

void controlEnable(control_channel_t channel);

void controlDisable(control_channel_t channel);

bool controlIsEnabled(control_channel_t channel);

void controlSetTarget(int32_t target, control_channel_t channel);

int32_t controlGetPWMDuty(control_channel_t channel);

void controlUpdate(uint32_t deltaTime);

void enableMotorRampSequence(bool state);

void setRampActive(bool state);

bool getRampActive(void);

void resetController(void);


#endif /* CONTROL_H_ */

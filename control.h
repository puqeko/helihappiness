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

#define PRECISION 1000  // zeros represent how many dp of precision to get with integer math
#define MIN_DUTY 5  // %
#define MAX_DUTY 95  // %

typedef enum control_channel {
    CONTROL_HEIGHT=0, CONTROL_YAW, CONTROL_CALIBRATE, CONTROL_NUM_CHANNELS
} control_channel_t;

void controlInit(void);

void controlMotorSet(bool state);

void controlEnable(control_channel_t channel);

void controlDisable(control_channel_t channel);

bool controlIsEnabled(control_channel_t channel);

void controlSetTarget(int32_t target, control_channel_t channel);

int32_t controlGetPWMDuty(control_channel_t channel);

void controlUpdate(uint32_t deltaTime);


#endif /* CONTROL_H_ */

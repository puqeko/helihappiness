// ************************************************************
// landingController.h
// Helicopter project
// Group: A03 Group 10
// Last edited: 21-04-2018 by Ryan Hall
//
// Purpose: This modules controls the ramp function for both yaw
//          and height that are activated upon landing. It contains
//          functions to check for stability to communicate if the
//          landing task stages have been executed successfully
// ************************************************************

#ifndef LANDINGCONTROLLER_H_
#define LANDINGCONTROLLER_H_

#include <stdint.h>
#include <stdbool.h>
#include "stateInfo.h"

#define LANDING_RATE 25 // height descent rate in % per second
#define LANDING_TIME_OUT 7500 // heli will time out (force land) if stability not reached in 7.5 seconds
#define STABILITY_TIME_MAIN 500 // Heli must remain within specified bounds for 500 ms to register as stable
#define MS_TO_SEC 1000 // used for converting milliseconds into seconds
#define YAW_STABILITY_ERROR 2 // accept yaw stability if within +- 2 degrees of target


// Description: This function shedules the landing sequence. First it calls the function
//              to ramp the yaw to the reference position It then checks the yaw has met
//              the target before ramping down the height at a set rate
// Parameters:  state_t* state points to the struct containing the targets for yaw and height
//              deltaTime is a 32-bit unsigned integer. It is the task period in ms
//              int32_t yawDegrees is the measured yaw scaled by PRECISION
void land(state_t *state, uint32_t deltaTime, int32_t yawDegrees);


// Description: This function checks whether the yaw is within the specified error range of the reference yaw position.
// Parameters:  The function takes the int32_t yawDegrees parameter; the measured quantity for the yaw scaled by PRECISION
// Return:      The function returns a boolean type; true when the yaw is within the specified range
bool isLandingYawStable(int32_t yawDegrees);


// Description: This function checks that the helicopter remains stable while in its landing position
//              (reference yaw and 0 height) for a set period of time. The function includes a time
//              out in case the helicopter does not stabilize.
// Parameters:  state_t* state points to the struct containing the target height and target yaw variables
//              deltaTime is the task period in ms and yawDegrees and heightPercentage pass in the measured
//              quantities for yaw and height respectively scaled by PRECISION.
// Return:      Returns a boolean indicating true when landing stability has been reached.
bool checkLandingStability (state_t *state, uint32_t deltaTime, int32_t yawDegrees, int32_t heightPercentage);


#endif /* LANDINGCONTROLLER_H_ */

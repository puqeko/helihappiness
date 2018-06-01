// ************************************************************
// landingController.c
// Helicopter project
// Group: A03 Group 10
// Last edited: 21-04-2018
//
// Purpose: This modules controls the ramp function for both yaw
//          and height that are activated upon landing
// ************************************************************


#include "landingController.h"
#include "control.h"

// Description: Ramp function for yaw: Finds nearest 360 degree target and increments/decrements the target to this position
// Parameters:
void rampYaw(state_t *state, int32_t yawDegrees)
{
    if (yawDegrees > 0 && abs(state->targetYaw) % 360 != 0) { // Case for +ve yaw
        if (abs(yawDegrees) % (360 * PRECISION) <= (180 * PRECISION)) { // Evaluates position relative to target to find nearest
            state->targetYaw -= 1;
        } else {
            state->targetYaw += 1;
        }
    } else if (yawDegrees < 0 && abs(state->targetYaw) % 360 != 0) { // Case for -ve yaw
        if (abs(yawDegrees) % (360 * PRECISION) <= (180 * PRECISION)) {
            state->targetYaw += 1;
        } else {
            state->targetYaw -= 1;
        }
    }
}


// Description: This function checks whether the yaw is within the specified error range of the reference yaw position.
// Parameters: The function takes the int32_t yawDegrees parameter; the measured quantity for the yaw scaled by PRECISION
// Return: The function returns a boolean type; true when the yaw is within the specified range
bool isLandingYawStable(int32_t yawDegrees) {
    // 360 represents the number of degrees in a full rotation.
    // % operator returns remainder after division.
    return ((abs(yawDegrees) % (360 * PRECISION)) <= (YAW_STABILITY_ERROR * PRECISION) ||
            (abs(yawDegrees) % (360 * PRECISION)) >= ((360 - YAW_STABILITY_ERROR) * PRECISION));
}


// Description: This function checks that the helicopter remains stable while in its landing position (reference yaw and 0 height)
// for a set period of time. The function includes a time out in case the helicopter does not stabilise.
// Parameters: state_t* state points to the struct containing the target height and target yaw variables
//             deltaTime is the task period in ms and yawDegrees and heightPercentage pass in the measured
//             quantities for yaw and height respectively scaled by PRECISION.
// Return: returns a boolean indicating true when landing stability has been reached.
bool checkLandingStability (state_t* state, uint32_t deltaTime, int32_t yawDegrees, int32_t heightPercentage)
{
    static uint32_t stabilityCounter;
    static uint32_t landingTime = 0; // time out counter
    // check that the height is less than 1% and that the target has been set to 0
    // PRECISION accounts for precision scaling of input parameter heightPercentage
    if (heightPercentage <= PRECISION && state->targetHeight == 0) {
        if (isLandingYawStable(yawDegrees)) { // is the yaw at the target (within specified error)
            stabilityCounter++;
        } else {
            stabilityCounter = 0; // if the heli moves out of stable range, reset the counter
        }
        landingTime++;
        // time out in case heli gets stuck in non-stable state whilst landing.
        if (landingTime >= LANDING_TIME_OUT / deltaTime) {
            return true;
        }
    } else {
        stabilityCounter = 0;
    }
    // check that the heli has remained in a stable state for a set period of time
    return stabilityCounter >= STABILITY_TIME_MAIN / deltaTime;
}


//
void land(state_t *state, uint32_t deltaTime, int32_t yawDegrees)
{
    static uint32_t landingCounter = 0;
    rampYaw(state, yawDegrees);
    if (isLandingYawStable(yawDegrees)) {
        if (state->targetHeight != 0 && landingCounter >= MS_TO_SEC / (LANDING_RATE * deltaTime) ) {
            state->targetHeight -= 1;
            landingCounter = 0;
        }
    }
    landingCounter++;
}




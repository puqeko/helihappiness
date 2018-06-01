// ************************************************************
// landingController.c
// Helicopter project
// Group: A03 Group 10
// Last edited: 21-04-2018
//
// Purpose: This modules controls the ramp function for both yaw and height that are activated upon landing
// ************************************************************


#include "landingController.h"
#include "control.h"

static bool shouldRampMain = false;
void setRampActive(bool state)
{
    shouldRampMain = state;
}

bool getRampActive(void)
{
    return shouldRampMain;
}

// Ramp function for yaw: Finds nearest 360 degree target and moves to this position
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

bool isLandingYawStable(int32_t yawDegrees) {
    return ((abs(yawDegrees) % (360 * PRECISION)) <= (2 * PRECISION) ||
            (abs(yawDegrees) % (360 * PRECISION)) >= (358 * PRECISION));
}

bool checkLandingStability (state_t *state, uint32_t deltaTime, int32_t yawDegrees, int32_t heightPercentage)
{
    static uint32_t stabilityCounter;
    static uint32_t landingTime = 0;

    if (heightPercentage <= PRECISION && state->targetHeight == 0) {
        if (isLandingYawStable(yawDegrees)) {
            stabilityCounter++;
        } else {
            stabilityCounter = 0;
        }
        landingTime++;
        if (landingTime >= LANDING_TIME_OUT / deltaTime) {
            return true;
        }
    } else {
        stabilityCounter = 0;
    }
    return stabilityCounter >= STABILITY_TIME_MAIN / deltaTime;
}


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




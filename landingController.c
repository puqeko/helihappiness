// ************************************************************
// main.c
// Helicopter project
// Group: A03 Group 10
// Last edited: 21-04-2018
//
// Purpose: This program may destroy helicopters.
// ************************************************************


#include "landingController.h"

void land(state_t *state, uint32_t deltaTime, int32_t yawDegrees)
{
    rampYaw(state, yawDegrees);
    if (isLandingYawStable(yawDegrees)) {
        rampHeight(state, deltaTime);
    }
}

// Ramp function for yaw: Finds nearest 360 degree target and moves to this position
void rampYaw(state_t *state, yawDegrees)
{
    if (yawDegrees > 0 && abs(state->targetYaw) % 360 != 0) { // Case for +ve yaw
        if (abs(yawDegrees) % 360 <= 180) { // Evaluates position relative to target to find nearest
            state->targetYaw -= 1;
        } else {
            state->targetYaw += 1;
        }
    } else if (yawDegrees < 0 && abs(state->targetYaw) % 360 != 0) { // Case for -ve yaw
        if (abs(yawDegrees) % 360 <= 180) {
            state->targetYaw += 1;
        } else {
            state->targetYaw -= 1;
        }
    }
}

void rampHeight(state_t *state, uint32_t deltaTime)
{
    static uint32_t landingDecrementCounter = 0;

    if (landingDecrementCounter == MS_TO_SEC / (deltaTime * LANDING_SPEED) && state->targetHeight != 0) {
        state->targetHeight -= 1;
        landingDecrementCounter = 0;
    }
    landingDecrementCounter++;
}

bool isLandingYawStable(int32_t yawDegrees) {
    return ((abs(yawDegrees) % 360) <= 5 || (abs(yawDegrees) % 360) >= 355);
}

bool hasFinishedLanding (state_t *state, int32_t yawDegrees, int32_t heightPercentage)
{
    static uint32_t stabilityCounter;
    static uint32_t landingTime = 0;
    static bool shouldEnterLandedState = false;
    static bool plzLandplz = false;

    if (heightPercentage <= 1 && state->targetHeight == 0) {
        if (isLandingYawStable(yawDegrees)) {
            stabilityCounter++;
        } else {
            stabilityCounter = 0;
        }
        landingTime++;
        if (landingTime >= LANDING_TIME_OUT / deltaTime) {
            plzLandplz = true;
        }
    } else {
        stabilityCounter = 0;
    }
    if (stabilityCounter >= STABILITY_TIME_MAIN / deltaTime || plzLandplz) {
        shouldEnterLandedState = true;
    }
}




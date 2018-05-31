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

// Ramp function for yaw: Finds nearest 360 degree target and moves to this position
void rampYaw(state_t *state, int32_t yawDegrees)
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

bool isLandingYawStable(int32_t yawDegrees) {
    return ((abs(yawDegrees) % 360) <= 5 || (abs(yawDegrees) % 360) >= 355);
}

void checkLandingStability (state_t *state, uint32_t deltaTime, int32_t yawDegrees, int32_t heightPercentage)
{
    static uint32_t stabilityCounter;
    static uint32_t landingTime = 0;

    if (heightPercentage <= 1 && state->targetHeight == 0) {
        if (isLandingYawStable(yawDegrees)) {
            stabilityCounter++;
        } else {
            stabilityCounter = 0;
        }
        landingTime++;
        if (landingTime >= LANDING_TIME_OUT / deltaTime) {
            setRampActive(true);
        }
    } else {
        stabilityCounter = 0;
    }
    if (stabilityCounter >= STABILITY_TIME_MAIN / deltaTime) {
        setRampActive(true);
    }
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




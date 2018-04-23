
// *******************************************************
// pidControl.h
//
// Generates serial data to print for debugging
// P.J. Bones UCECE, modified by Ryan hall
// Last modified:  18.04.2017
// Helicopter project
// Group: A03 Group 10
// *******************************************************

#include <stdbool.h>
#include "pidControl.h"
#include "timerer.h"

static uint32_t Kpu = 2000;
static uint32_t Kpd = 300;
static uint32_t offsetDuty = 30000;

bool PIDsetMainOffset(uint32_t measured) {
    if (measured < CALIBRATION_TARGET) {
        offsetDuty = offsetDuty + CALIBRATION_INCREMENT;
        return false;
    }
    offsetDuty = offsetDuty - CALIBRATION_INCREMENT;
    return true;
}
// Take the target height values, actual measured height values, and the time since
// the last update, and return the next value (as a duty cycle between 5% and 95%)
// to set the main rotor to.
uint32_t PIDUpdateHeight(uint32_t target, uint32_t measured, uint32_t deltaTime)
{
    if (measured > target) {
        return offsetDuty - (measured - target) * Kpd * deltaTime / 1000;
    }

    return offsetDuty + (target - measured) * Kpu * deltaTime / 1000;
}


// Take the target yaw values, actual measured yaw values, and the time since
// the last update, and return the next value (as a duty cycle between 5% and 95%)
// to set the tail rotor to.
uint32_t PIDUpdateYaw(uint32_t cur, uint32_t target, uint32_t measured, uint32_t deltaTime)
{
    return target;
}

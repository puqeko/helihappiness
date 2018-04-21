
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

static const uint32_t Kpu = 2000;
static const uint32_t Kpd = 300;


// Take the target height values, actual measured height values, and the time since
// the last update, and return the next value (as a duty cycle between 5% and 95%)
// to set the main rotor to.
uint32_t PIDUpdateHeight(uint32_t cur, uint32_t target, uint32_t measured, uint32_t deltaTime)
{
    if (measured > target) {
        return cur - (measured - target) * Kpd * deltaTime / 1000;
    }

    return cur + (target - measured) * Kpu * deltaTime / 1000;
}


// Take the target yaw values, actual measured yaw values, and the time since
// the last update, and return the next value (as a duty cycle between 5% and 95%)
// to set the tail rotor to.
uint32_t PIDUpdateYaw(uint32_t cur, uint32_t target, uint32_t measured, uint32_t deltaTime)
{
    return target;
}

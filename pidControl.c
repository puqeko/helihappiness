
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

// Take the target height values, actual measured height values, and the time since
// the last update, and return the next value (as a duty cycle between 5% and 95%)
// to set the main rotor to.
uint32_t PIDUpdateHeight(uint32_t target, uint32_t measured, uint32_t deltaTime)
{
    return target;
}


// Take the target yaw values, actual measured yaw values, and the time since
// the last update, and return the next value (as a duty cycle between 5% and 95%)
// to set the tail rotor to.
uint32_t PIDUpdateYaw(uint32_t target, uint32_t measured, uint32_t deltaTime)
{
    return target;
}

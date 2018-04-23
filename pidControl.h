#ifndef PIDCONTROL_H_
#define PIDCONTROL_H_

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
#define MULT 1000  // for integer calcs
#define CALIBRATION_INCREMENT 1000
#define CALIBRATION_TARGET 1

// Take the target height values, actual measured height values, and the time since
// the last update, and return the next value (as a duty cycle between 5% and 95%)
// to set the main rotor to. cur is the current output.
uint32_t PIDUpdateHeight(uint32_t target, uint32_t measured, uint32_t deltaTime);

// Take the target yaw values, actual measured yaw values, and the time since
// the last update, and return the next value (as a duty cycle between 5% and 95%)
// to set the tail rotor to. cur is the current output.
uint32_t PIDUpdateYaw(uint32_t cur, uint32_t target, uint32_t measured, uint32_t deltaTime);

bool PIDsetMainOffset(uint32_t measured);

#endif /* PIDCONTROL_H_ */

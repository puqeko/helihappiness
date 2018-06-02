//************************************************************************
// yaw.c
// Helicopter project
// Group: A03 Group 10
// Last Edited: 31-5-18
//
// Purpose: Handles the yaw of the helicopter
//************************************************************************

#ifndef YAW_H_
#define YAW_H_

#include <stdint.h>
#include <stdbool.h>


// Initiate Calibration
// Enables yaw calibration interrupt
void yawCalibrate(void);


// Return true if yaw has been calibrated against the reference position
bool yawIsCalibrated(void);


// Initialize quadrature encoder and configure yaw reference GPIO pin and interrupt
void yawInit(void);


// Return the current yaw in degrees
//
// Parameters:
//   int32_t precision    scale factor for retaining accuracy with integers
int32_t yawGetDegrees(int32_t precision);


// Remove excess factors of 360 degrees from yaw
// Returns true if the adjusted yaw value is less than 180 degree
void yawClipTo360Degrees(void);

#endif /*YAW_H_*/

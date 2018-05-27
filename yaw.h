#ifndef YAW_H_
#define YAW_H_

//************************************************************************
// yaw.h
//
// Helicopter project
//
// Group:       A03 Group 10
// Created:     16/4/18
// Last Edited: 16/4/18
//
// Purpose: Handles the yaw of the helicopter
//************************************************************************

#include <stdint.h>
#include <stdbool.h>

void yawRefIntHandler(void);
void yawCalibrate(void);
bool yawIsCalibrated(void);

//************************************************************************
//
//************************************************************************
void yawInit(void);


//************************************************************************
//
//************************************************************************
int32_t yawGetDegrees(int32_t precision);


//************************************************************************
//
//************************************************************************
void yawCalibrate(void); //perhaps not part of yaw module???


bool yawClipTo360Degrees(void);


#endif /*YAW_H_*/

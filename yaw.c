//************************************************************************
// yaw.c
//
// Helicopter project
//
// Group:       A03 Group 10
// Created:     16/4/18
// Last Edited: 16/4/18
//
// Purpose: Handles the yaw of the helicopter
//************************************************************************

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// TivaWare library includes
#include "inc/hw_memmap.h"        // defines GPIO_PORTF_BASE
#include "inc/tm4c123gh6pm.h"     // defines interrupt vectors and register addresses
#include "driverlib/gpio.h"       // defines for GPIO peripheral
#include "driverlib/sysctl.h"     // system control functions
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"

#include "quadratureEncoder.h"

#include "yaw.h"


int32_t yawGetDegrees(void)
{
    int32_t rawCounts = quadEncoderGetCount();
    int32_t angle = rawCounts; /*add appropriate calculation*/

    return angle;
}

void yawCalibrate(void) //perhaps not part of yaw module???
{
    //do calibration stuff
}

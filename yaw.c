//************************************************************************
// yaw.c
//
// Helicopter project
//
// Group:       A03 Group 10
// Created:     16/4/18
// Last Edited: 10/5/18
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


#define COUNTS_PER_ROTATION (112*4) //112 slots and x4 because quadrature encoding used


void yawInit(void)
{
    quadEncoderInit();
}


int32_t yawGetDegrees(int32_t precision)
{
    // no update required as quadEncoderGetCount does not do any heavy calculations.
    return quadEncoderGetCount() * (precision*360)/COUNTS_PER_ROTATION;
}

void yawClipTo360Degrees(void)
{
    quadEncoderSetCount(quadEncoderGetCount() % COUNTS_PER_ROTATION);
}

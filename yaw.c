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


#define COUNTS_PER_ROTATION (112*4) //112 slots and x4 because quadrature encoding used
#define BASE_YAW 100000
#define DEGREES_PER_COUNT ((BASE_YAW*360)/COUNTS_PER_ROTATION)



void yawInit(void)
{
    quadEncoderInit(SYSCTL_PERIPH_GPIOB, GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_PIN_1);
}

int32_t yawGetDegrees(void)
{
    int32_t rawCounts = quadEncoderGetCount();
    return (rawCounts * DEGREES_PER_COUNT)/(BASE_YAW);
}

void yawCalibrate(void) //perhaps not part of yaw module???
{
    //do calibration stuff
}

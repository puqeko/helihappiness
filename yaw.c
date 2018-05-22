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

#define GPIO_REF_PERIPH         SYSCTL_PERIPH_GPIOC
#define REF_PORT_BASE           GPIO_PORTC_BASE
#define REF_PIN                 GPIO_PIN_4

#define COUNTS_PER_ROTATION (112*4) //112 slots and x4 because quadrature encoding used

static volatile bool isCalibrated;

void yawRefIntHandler(void)
{
    isCalibrated = true;
    quadEncoderResetCount();
    GPIOIntDisable(REF_PORT_BASE, REF_PIN);
    GPIOIntClear(REF_PORT_BASE, REF_PIN);
}



void yawCalibrate(void)
{
    isCalibrated = false;
    GPIOIntEnable(REF_PORT_BASE, REF_PIN);
}



bool yawIsCalibrated(void)
{
    return isCalibrated;
}


//************************************************************************
//
//************************************************************************
void yawInit(void)
{
    //initialise quadrature encoder module
    quadEncoderInit();

    //Enable GPIO peripheral on used ports
    SysCtlPeripheralEnable(GPIO_REF_PERIPH);

    //configure reference input pin
    GPIOPadConfigSet(REF_PORT_BASE, REF_PIN, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU);
    GPIODirModeSet(REF_PORT_BASE, REF_PIN, GPIO_DIR_MODE_IN);

    //initialise reference interrupt but do not enable
    GPIOIntRegister(REF_PORT_BASE, yawRefIntHandler);
    GPIOIntTypeSet(REF_PORT_BASE, REF_PIN, GPIO_FALLING_EDGE);
}

//************************************************************************
//
//************************************************************************
int32_t yawGetDegrees(int32_t precision)
{
    // no update required as quadEncoderGetCount does not do any heavy calculations.
    return quadEncoderGetCount() * (precision*360)/COUNTS_PER_ROTATION;
}

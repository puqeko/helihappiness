//************************************************************************
// yaw.c
// Helicopter project
// Group: A03 Group 10
// Last Edited: 31-5-18
//
// Purpose: Handles the yaw of the helicopter
//************************************************************************

#include "yaw.h"

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "quadratureEncoder.h"
#include "driverlib/interrupt.h"

// Define constants for yaw reference GPIO pin PC4
#define GPIO_REF_PERIPH         SYSCTL_PERIPH_GPIOC
#define REF_PORT_BASE           GPIO_PORTC_BASE
#define REF_PIN                 GPIO_PIN_4

#define COUNTS_PER_ROTATION (112*4) // 112 slots and x4 because quadrature encoding used


static volatile bool isCalibrated;


// Interrupt handler for the yaw reference
// Resets the yaw to zero when reference point is reached
void yawRefIntHandler(void)
{
    isCalibrated = true;
    quadEncoderResetCount();

    // Reset and disable the interrupt
    GPIOIntDisable(REF_PORT_BASE, REF_PIN);
    GPIOIntClear(REF_PORT_BASE, REF_PIN);
}


// Initiate Calibration
// Enables yaw calibration interrupt
void yawCalibrate(void)
{
    isCalibrated = false;
    GPIOIntEnable(REF_PORT_BASE, REF_PIN);
}


// Return true if yaw has been calibrated against the reference position
bool yawIsCalibrated(void)
{
    return isCalibrated;
}


// Initialize quadrature encoder and configure yaw reference GPIO pin and interrupt
void yawInit(void)
{
    // Initialize quadrature encoder module
    quadEncoderInit();

    // Enable GPIO peripheral on used port
    SysCtlPeripheralEnable(GPIO_REF_PERIPH);

    // Configure yaw reference input pin (active LOW)
    GPIOPadConfigSet(REF_PORT_BASE, REF_PIN, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU);
    GPIODirModeSet(REF_PORT_BASE, REF_PIN, GPIO_DIR_MODE_IN);

    // Initialize yaw reference interrupt but do not enable
    GPIOIntRegister(REF_PORT_BASE, yawRefIntHandler);
    GPIOIntTypeSet(REF_PORT_BASE, REF_PIN, GPIO_FALLING_EDGE);
}


// Return the current yaw in degrees
//
// Parameters:
//   int32_t precision    scale factor for retaining accuracy with integers
int32_t yawGetDegrees(int32_t precision)
{
    // No update required as quadEncoderGetCount does not do any heavy calculations.
    return quadEncoderGetCount() * (precision*360)/COUNTS_PER_ROTATION;
}


// Remove excess factors of 360 degrees from yaw and normalises difference about 0
void yawClipTo360Degrees(void)
{
    static bool prevIntState;
    prevIntState = IntMasterDisable();
    int32_t quadEncoderCount = quadEncoderGetCount();
    int32_t sign = (quadEncoderCount < 0) ? -1 : 1;
    quadEncoderCount = quadEncoderCount % COUNTS_PER_ROTATION;
    if (abs(quadEncoderCount) > (COUNTS_PER_ROTATION / 2)) {
        quadEncoderSetCount(quadEncoderCount - sign * COUNTS_PER_ROTATION);
    }
    if (prevIntState) {
        IntMasterEnable();
    }
}

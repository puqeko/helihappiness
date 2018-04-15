//************************************************************************
// quadratureEncoder.c
//
// Helicopter project
//
// Group:       A03 Group 10
// Created:     16/4/18
// Last Edited: 16/4/18
//
// Purpose: Handles input from the quadrature encoder
//************************************************************************

#include <stdio.h>

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

static volatile int32_t encoderCount = 0;
static uint32_t portBase, channelAPin, channelBPin;

void quadEncoderIntHandler(void)
{
    //do some encoding
}

void quadEncoderCountReset(void)
{
    encoderCount = 0;
}

int32_t quadEncoderCountReset(void)
{
    return encoderCount;
}

void quadEncoderInit(uint32_t GPIOPortPerif, uint32_t GPIOPortBase, uint32_t GPIOChannelAPin, uint32_t GPIOChannelBPin)
{
    portBase = GPIOPortBase;
    channelAPin = GPIOChannelAPin;
    channelBPin = GPIOChannelBPin;

    SysCtlPeripheralEnable(GPIOPortPerif);
    GPIOPadConfigSet(portBase, channelAPin | channelBPin, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPD);
    GPIODirModeSet(portBase, channelAPin | channelBPin, GPIO_DIR_MODE_IN);


}

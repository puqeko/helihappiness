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

#define CLOCKWISE_COUNT 1
#define ANTICLOCKWISE_COUNT -1
#define INIT_LASTCHANGED = 0xFFFFFFFF

static volatile int32_t encoderCount = 0;
static uint32_t portBase, channelAPin, channelBPin;
static uint32_t lastChanged = INIT_LASTCHANGED;
static int8_t direction = CLOCKWISE_COUNT;

void quadEncoderIntHandler(void)
{
    uint32_t intStatus = GPIOIntStatus(portBase, true);
    GPIOIntClear(portBase, channelAPin | channelBPin);

    //if(lastChanged == INIT_LASTCHANGED)
    //{
    //    //check what direction its going in
    //}
    /*else*/ if (lastChanged == intStatus)
    {
        direction *= -1; //toggle direction
    }

    encoderCount += direction;
    lastChanged = intStatus;
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

    //configure input pins
    SysCtlPeripheralEnable(GPIOPortPerif);
    GPIOPadConfigSet(portBase, channelAPin | channelBPin, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPD);
    GPIODirModeSet(portBase, channelAPin | channelBPin, GPIO_DIR_MODE_IN);

    //Initialize interrupts
    GPIOIntRegister(portBase, quadEncoderIntHandler);
    GPIOIntTypeSet(portBase, channelAPin | channelBPin, GPIO_BOTH_EDGES);
    GPIOIntEnable(portBase, channelAPin | channelBPin);
}

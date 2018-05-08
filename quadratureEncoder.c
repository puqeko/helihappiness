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

#define CW_DIRECTION    1
#define CCW_DIRECTION  -1
#define INITIAL_INT_STATUS  0xFFFFFFFF

static volatile int32_t direction, encoderCount = 0;
static uint32_t portBase, channelAPin, channelBPin, initialPinState;
static volatile uint32_t lastIntStatus = INITIAL_INT_STATUS;

void quadEncoderIntHandler(void)
{
    uint32_t intStatus = GPIOIntStatus(portBase, true);
    GPIOIntClear(portBase, channelAPin | channelBPin);

    if(lastIntStatus == INITIAL_INT_STATUS)
    {
        //as only one of intChannelA or intChannelB will be true and they will
        //not both be true at the same time, only one of the two variables
        //are needed. Both are included here for clarity & readability
        bool intChannelA = intStatus & channelAPin;
        bool intChannelB = intStatus & channelBPin;

        bool channelA = initialPinState & channelAPin;
        bool channelB = initialPinState & channelBPin;

        if(intChannelA && channelA) //A rising edge
        {
            !channelB ? (direction = CW_DIRECTION) : (direction = CCW_DIRECTION);
        }
        else if(intChannelA && !channelA) //A falling edge
        {
            channelB ? (direction = CW_DIRECTION) : (direction = CCW_DIRECTION);
        }
        else if(intChannelB && channelB) //B rising edge
        {
            channelA ? (direction = CW_DIRECTION) : (direction = CCW_DIRECTION);
        }
        else if(intChannelB && !channelB) //B falling edge
        {
            !channelA ? (direction = CW_DIRECTION) : (direction = CCW_DIRECTION);
        }
    }
    else if (lastIntStatus == intStatus)
    {
        direction *= -1;
    }

    encoderCount += direction;
    lastIntStatus = intStatus;
}

void quadEncoderResetCount(void)
{
    encoderCount = 0;
}

int32_t quadEncoderGetCount(void)
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
    //Get initial state of channel A and channel B for initially determining direction
    initialPinState = GPIOPinRead(portBase, channelAPin | channelBPin);
    //Enable encoder interrupts
    GPIOIntEnable(portBase, channelAPin | channelBPin);
}

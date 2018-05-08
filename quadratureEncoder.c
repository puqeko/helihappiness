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
static uint32_t channelPortBase, channelAPin, channelBPin, refPortBase, refPin, initialPinState;
static volatile uint32_t lastIntStatus = INITIAL_INT_STATUS;
static volatile bool isCalibrated;



void quadEncoderIntHandler(void)
{
    uint32_t intStatus = GPIOIntStatus(channelPortBase, true);
    GPIOIntClear(channelPortBase, channelAPin | channelBPin);

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



void quadEncoderRefIntHandler(void)
{
    isCalibrated = true;
    GPIOIntDisable(refPortBase, refPin);
    GPIOIntClear(refPortBase, refPin);
}



void quadEncoderCalibrate(void)
{
    isCalibrated = false;
    GPIOIntEnable(refPortBase, refPin);
}



bool quadEncoderIsCalibrated(void)
{
    return isCalibrated;
}



//TODO is this still needed? i assume not
void quadEncoderResetCount(void)
{
    encoderCount = 0;
}




int32_t quadEncoderGetCount(void)
{
    return encoderCount;
}



void quadEncoderInit(uint32_t GPIOPortPerif, uint32_t GPIOChannelPortBase, uint32_t GPIOChannelAPin, uint32_t GPIOChannelBPin, uint32_t GPIORefPortBase, uint32_t GPIORefPin)
{
    channelPortBase = GPIOChannelPortBase;
    channelAPin     = GPIOChannelAPin;
    channelBPin     = GPIOChannelBPin;

    refPortBase     = GPIORefPortBase;
    refPin          = GPIORefPin;

    SysCtlPeripheralEnable(GPIOPortPerif);

    //configure channel input pins
    GPIOPadConfigSet(channelPortBase, channelAPin | channelBPin, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPD);
    GPIODirModeSet(channelPortBase, channelAPin | channelBPin, GPIO_DIR_MODE_IN);

    //configure reference input pin
    GPIOPadConfigSet(refPortBase, refPin, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU);
    GPIODirModeSet(refPortBase, refPin, GPIO_DIR_MODE_IN);

    //Initialize channel interrupts
    GPIOIntRegister(channelPortBase, quadEncoderIntHandler);
    GPIOIntTypeSet(channelPortBase, channelAPin | channelBPin, GPIO_BOTH_EDGES);
    //Get initial state of channel A and channel B for initially determining direction
    initialPinState = GPIOPinRead(channelPortBase, channelAPin | channelBPin);
    //Enable encoder interrupts
    GPIOIntEnable(channelPortBase, channelAPin | channelBPin);

    //initialize reference interrupt but do not enable
    GPIOIntRegister(refPortBase, quadEncoderRefIntHandler);
    GPIOIntTypeSet(refPortBase, refPin, GPIO_FALLING_EDGE);
}

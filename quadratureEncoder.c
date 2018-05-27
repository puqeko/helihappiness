 //************************************************************************
// quadratureEncoder.c
//
// Helicopter project
//
// Group:       A03 Group 10
// Created:     16/4/18
// Last Edited: 10/5/18
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

#define GPIO_CHANNEL_PERIPH     SYSCTL_PERIPH_GPIOB
#define CHANNEL_PORT_BASE       GPIO_PORTB_BASE
#define CHANNEL_A_PIN           GPIO_PIN_0
#define CHANNEL_B_PIN           GPIO_PIN_1

//#define GPIO_REF_PERIPH         SYSCTL_PERIPH_GPIOC
//#define REF_PORT_BASE           GPIO_PORTC_BASE
//#define REF_PIN                 GPIO_PIN_4

#define CW_DIRECTION    1
#define CCW_DIRECTION  -1
#define INITIAL_INT_STATUS  0xFFFFFFFF


static uint32_t initialPinState;
static volatile int32_t direction, encoderCount = 0;
static volatile uint32_t lastIntStatus = INITIAL_INT_STATUS;
//static volatile bool isCalibrated;



void quadEncoderIntHandler(void)
{
    uint32_t intStatus = GPIOIntStatus(CHANNEL_PORT_BASE, true);
    GPIOIntClear(CHANNEL_PORT_BASE, CHANNEL_A_PIN | CHANNEL_B_PIN);

    if(lastIntStatus == INITIAL_INT_STATUS)
    {
        //as only one of intChannelA or intChannelB will be true and they will
        //not both be true at the same time, only one of the two variables
        //are needed. Both are included here for clarity & readability
        bool intChannelA = intStatus & CHANNEL_A_PIN;
        bool intChannelB = intStatus & CHANNEL_B_PIN;

        bool channelA = initialPinState & CHANNEL_A_PIN;
        bool channelB = initialPinState & CHANNEL_B_PIN;

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



/*void quadEncoderRefIntHandler(void)
{
    isCalibrated = true;
    encoderCount = 0;
    GPIOIntDisable(REF_PORT_BASE, REF_PIN);
    GPIOIntClear(REF_PORT_BASE, REF_PIN);
}



void quadEncoderCalibrate(void)
{
    isCalibrated = false;
    GPIOIntEnable(REF_PORT_BASE, REF_PIN);
}



bool quadEncoderIsCalibrated(void)
{
    return isCalibrated;
}*/



void quadEncoderResetCount(void)
{
    encoderCount = 0;
}


void quadEncoderSetCount(uint32_t newCount)
{
    encoderCount = newCount;
}



int32_t quadEncoderGetCount(void)
{
    return encoderCount;
}



void quadEncoderInit(void)
{
    //Enable GPIO peripheral on used ports
    SysCtlPeripheralEnable(GPIO_CHANNEL_PERIPH);
    //SysCtlPeripheralEnable(GPIO_REF_PERIPH);

    //configure channel input pins
    GPIOPadConfigSet(CHANNEL_PORT_BASE, CHANNEL_A_PIN | CHANNEL_B_PIN, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPD);
    GPIODirModeSet(CHANNEL_PORT_BASE, CHANNEL_A_PIN | CHANNEL_B_PIN, GPIO_DIR_MODE_IN);

    /*//configure reference input pin
    GPIOPadConfigSet(REF_PORT_BASE, REF_PIN, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU);
    GPIODirModeSet(REF_PORT_BASE, REF_PIN, GPIO_DIR_MODE_IN);*/

    //Initialize channel interrupts
    GPIOIntRegister(CHANNEL_PORT_BASE, quadEncoderIntHandler);
    GPIOIntTypeSet(CHANNEL_PORT_BASE, CHANNEL_A_PIN | CHANNEL_B_PIN, GPIO_BOTH_EDGES);
    //Get initial state of channel A and channel B for initially determining direction
    initialPinState = GPIOPinRead(CHANNEL_PORT_BASE, CHANNEL_A_PIN | CHANNEL_B_PIN);
    //Enable encoder interrupts
    GPIOIntEnable(CHANNEL_PORT_BASE, CHANNEL_A_PIN | CHANNEL_B_PIN);

    /*//initialize reference interrupt but do not enable
    GPIOIntRegister(REF_PORT_BASE, quadEncoderRefIntHandler);
    GPIOIntTypeSet(REF_PORT_BASE, REF_PIN, GPIO_FALLING_EDGE);*/
}

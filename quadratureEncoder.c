//************************************************************************
// quadratureEncoder.h
// Helicopter project
// Group: A03 Group 10
// Last Edited: 31-5-18
//
// Purpose: Handles input from the quadrature encoder
//************************************************************************

#include "quadratureEncoder.h"

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

// Define constants for A and B encoder channel GPIO pins (PB0 and PB1 respectively)
#define GPIO_CHANNEL_PERIPH     SYSCTL_PERIPH_GPIOB
#define CHANNEL_PORT_BASE       GPIO_PORTB_BASE
#define CHANNEL_A_PIN           GPIO_PIN_0
#define CHANNEL_B_PIN           GPIO_PIN_1

#define CW_DIRECTION    1
#define CCW_DIRECTION  -1
#define INITIAL_INT_STATUS  0xFFFFFFFF // A normally invalid interrupt status value to
                                       //  indicate if it is the first time the interrupt
                                       //  has been triggered
static uint32_t initialPinState;
static volatile int32_t direction, encoderCount = 0;
static volatile uint32_t lastIntStatus = INITIAL_INT_STATUS;


// Handles interrupts from A and B channels of the quadrature encoder
// Adds or subtracts from encoderCount depending on current rotation direction
void quadEncoderIntHandler(void)
{
    uint32_t intStatus = GPIOIntStatus(CHANNEL_PORT_BASE, true); // Gets the pin that triggered
                                                                 // the interrupt
    // Clear the interrupt
    GPIOIntClear(CHANNEL_PORT_BASE, CHANNEL_A_PIN | CHANNEL_B_PIN);

    if(lastIntStatus == INITIAL_INT_STATUS) // If this is the first interrupt, determine
    {                                       //  the initial direction of rotation

        // As only one of intChannelA or intChannelB can be true at the same time,
        //  only one of the two variables are needed.
        //  Both are included here for clarity & readability
        bool intChannelA = intStatus & CHANNEL_A_PIN;
        bool intChannelB = intStatus & CHANNEL_B_PIN;

        bool channelA = initialPinState & CHANNEL_A_PIN;
        bool channelB = initialPinState & CHANNEL_B_PIN;

        if(intChannelA && channelA) { // A rising edge
            !channelB ? (direction = CW_DIRECTION) : (direction = CCW_DIRECTION);
        }
        else if(intChannelA && !channelA) { // A falling edge
            channelB ? (direction = CW_DIRECTION) : (direction = CCW_DIRECTION);
        }
        else if(intChannelB && channelB) { // B rising edge
            channelA ? (direction = CW_DIRECTION) : (direction = CCW_DIRECTION);
        }
        else if(intChannelB && !channelB) { // B falling edge
            !channelA ? (direction = CW_DIRECTION) : (direction = CCW_DIRECTION);
        }
    }
    else if (lastIntStatus == intStatus) { // If the same pin triggers an interrupt twice
                                           //  in succession then direction must have changed
        // Reverse the direction
        direction *= -1;
    }

    encoderCount += direction; // Either add or subtract one depending on direction
    lastIntStatus = intStatus;
}


// Resets the running encoder count to zero
void quadEncoderResetCount(void)
{
    encoderCount = 0;
}


// Set the running encoder count to the parameter newCount
//
// WARNING: When setting count value based on the existing count value,
//          do so in an atomic manner as the existing count value could
//          change before it is set to the new count value. Failing to
//          do so could result in the zero position "moving" over time.
void quadEncoderSetCount(uint32_t newCount)
{
    encoderCount = newCount;
}


// Returns the current encoder count
//
// WARNING: Function must be used in an atomic manner as the encoder count
//          can change at any time because it is interrupt driven
int32_t quadEncoderGetCount(void)
{
    return encoderCount;
}


// Configure GPIO pins and initialize interrupts
// Get initial state of channel pins for determining direction on first interrupt
void quadEncoderInit(void)
{
    // Enable GPIO peripheral on used ports
    SysCtlPeripheralEnable(GPIO_CHANNEL_PERIPH);

    // Configure channel input pins (active HIGH)
    GPIOPadConfigSet(CHANNEL_PORT_BASE, CHANNEL_A_PIN | CHANNEL_B_PIN, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPD);
    GPIODirModeSet(CHANNEL_PORT_BASE, CHANNEL_A_PIN | CHANNEL_B_PIN, GPIO_DIR_MODE_IN);

    // Initialize channel interrupts
    GPIOIntRegister(CHANNEL_PORT_BASE, quadEncoderIntHandler);
    GPIOIntTypeSet(CHANNEL_PORT_BASE, CHANNEL_A_PIN | CHANNEL_B_PIN, GPIO_BOTH_EDGES);

    // Get initial state of channel A and channel B for initially determining direction
    initialPinState = GPIOPinRead(CHANNEL_PORT_BASE, CHANNEL_A_PIN | CHANNEL_B_PIN);

    // Enable encoder interrupts
    GPIOIntEnable(CHANNEL_PORT_BASE, CHANNEL_A_PIN | CHANNEL_B_PIN);
}

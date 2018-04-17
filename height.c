// *******************************************************
// height.c
//
// Generates and uses averaging function to smooth data stream from ADCs
// P.J. Bones UCECE, modified by Ryan hall
// Last modified:  17.04.2017
// Helicopter project
// Group: A03 Group 10
// *******************************************************

#include "height.h"
#include "circBufT.h"
#include "adcModule.h"

static circBuf_t buf;
int convolutionArray[CONV_SIZE];

void SysTickIntHandler(void)
{
    adcTrigger();
}

void initConv(char convType)
{
    SysTickPeriodSet(SysCtlClockGet() / ADC_SAMPLE_RATE);  // frequency of 120 Hz
    SysTickIntRegister(SysTickIntHandler);
    // Enable interrupt and device
    SysTickIntEnable();
    SysTickEnable();
    initCircBuf(&buf, CONV_SIZE);
    adcInit(handleNewADCValue);
    getConvArray (convType);
}


void handleNewADCValue(uint32_t val)
{
    writeCircBuf(&buf, val);
}

uint32_t getAverage ()
{
    // Background task: calculate the (approximate) mean of the values in the
    // circular buffer and return it.

    uint32_t sum = 0;
    int i;
    for (i = 0; i < CONV_SIZE; i++) {
        sum = sum + readCircBuf (&buf);
    }
    // Calculate and display the rounded mean of the buffer contents
    return (2 * sum + CONV_SIZE) / 2 / CONV_SIZE;
}

void getConvArray (char convType)
{
    int i;
    switch (convType)
        {
        case UNIFORM:
            for (i = 0; i < CONV_SIZE; i++) {
                convolutionArray[i] = CONV_UNIFORM_MULTIPLIER;
            }
            break;
        }
}

uint32_t getHeight (void)
{
    uint32_t sum = 0;
    int i;
    for (i = 0; i < CONV_SIZE; i++) {
        sum = sum + (readCircBuf (&buf) * convolutionArray[i]);
    }
    return sum / CONV_BASE;
}

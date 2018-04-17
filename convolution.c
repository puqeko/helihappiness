// *******************************************************
// convolution.c
//
// Generates and uses averaging function to smooth data stream from ADCs
// P.J. Bones UCECE, modified by Ryan hall
// Last modified:  14.3.2017
// Helicopter project
// Group: A03 Group 10
// *******************************************************

#include "convolution.h"
#include "circBufT.h"

static circBuf_t buf;


void initConv(uint32_t convolutionSize)
{
    initCircBuf(&buf, convolutionSize);
}

void handleNewADCValue(uint32_t val)
{
    writeCircBuf(&buf, val);
}

uint32_t getAverage (uint32_t convolutionSize)
{
    // Background task: calculate the (approximate) mean of the values in the
    // circular buffer and return it.

    uint32_t sum = 0;
    int i;
    for (i = 0; i < convolutionSize; i++) {
        sum = sum + readCircBuf (&buf);
    }
    // Calculate and display the rounded mean of the buffer contents
    return (2 * sum + convolutionSize) / 2 / convolutionSize;
}

void convGetConvArray (uint32_t convolutionSize, char type, float *convolutionArray)
{
    int i;
    switch (type)
        {
        case UNIFORM:
            for (i = 0; i < convolutionSize; i++) {
                convolutionArray[i] = 1.0/convolutionSize;
            }
            break;
        }
}

float convDoConvolution (float *convolutionArray, uint32_t convolutionSize)
{
    float sum = 0;
    int i;
    for (i = 0; i < convolutionSize; i++) {
        sum = sum + (readCircBuf (&buf) * convolutionArray[i]);
    }
    return sum;
}

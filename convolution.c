// *******************************************************
//
// convolution.c
//
// Generates and uses averaging function to smooth data stream from ADCs
// P.J. Bones UCECE, modified by Ryan hall
// Last modified:  14.3.2017
//
// *******************************************************

#include "convolution.h"
#include "circBufT.h"

static circBuf_t buf;
#define CONV_SIZE 10

void initConv(void)
{
    initCircBuf(&buf, CONV_SIZE);
}

void handleNewADCValue(uint32_t val)
{
    writeCircBuf(&buf, val);
}

uint32_t getAverage (void)
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

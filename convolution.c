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

static circBuf_t buffer;
#define CONV_SIZE 20

void initConv()
{
    initCircBuf (&buffer, CONV_SIZE);
}

void handleNewADCValue(uint32_t val)
{
    writeCircBuf(&buffer, val);
}

uint32_t getAverage (uint32_t bufferSize)
{
//    // Background task: calculate the (approximate) mean of the values in the
//    // circular buffer and return it.
//    initCircBuf (&buffer, bufferSize);
//
//    uint32_t sum = 0;
//    int i;
//    for (i = 0; i < bufferSize; i++) {
//        sum = sum + readCircBuf (&buffer);
//    }
//    // Calculate and display the rounded mean of the buffer contents
//    return (2 * sum + bufferSize) / 2 / bufferSize;
    return readCircBuf (&buffer);
}


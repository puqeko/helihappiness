#ifndef CONVOLUTION_H_
#define CONVOLUTION_H_

// *******************************************************
// convolution.h
//
// Generates and uses averaging function to smooth data stream from ADCs
// P.J. Bones UCECE, modified by Ryan hall
// Last modified:  14.3.2017
// Helicopter project
// Group: A03 Group 10
// *******************************************************

#include <stdint.h>
#include "utils/ustdlib.h"

// *******************************************************
// getAverage: calculate the (approximate) mean of the values in the
// circular buffer and return it.
uint32_t getAverage (void);

// *******************************************************
void initConv(void);

// deal with new value from ADC interupt by adding it to the buffer
void handleNewADCValue(uint32_t);


#endif /* CONVOLUTION_H_ */

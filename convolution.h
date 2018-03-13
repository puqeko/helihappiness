#ifndef CONVOLUTION_H_
#define CONVOLUTION_H_

// *******************************************************
//
// convolution.h
//
// Generates and uses averaging function to smooth data stream from ADCs
// P.J. Bones UCECE, modified by Ryan hall
// Last modified:  14.3.2017
//
// *******************************************************
#include <stdint.h>
#include "utils/ustdlib.h"

// *******************************************************
// getConvolutionArray: Generate values for convolution array for averaging.
uint32_t * getConvolutionArray (char type, uint32_t size)

// *******************************************************
// getAverage: calculate the (approximate) mean of the values in the
// circular buffer and return it.
uint32_t getAverage (uint32_t bufferSize)

// *******************************************************



#endif /* CONVOLUTION_H_ */

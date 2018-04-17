#ifndef HEIGHT_H_
#define HEIGHT_H_

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
#define UNIFORM 'u'
#define CONV_SIZE 20
#define CONV_UNIFORM_MULTIPLIER 100
#define CONV_BASE (CONV_SIZE * CONV_UNIFORM_MULTIPLIER)


// *******************************************************
// convGetConvArray: Generate values for convolution array for averaging.
void getConvArray(char convType);

// *******************************************************
// getAverage: calculate the (approximate) mean of the values in the
// circular buffer and return it.
uint32_t getAverage();

// *******************************************************
void initConv(char convType);
void handleNewADCValue(uint32_t val);
uint32_t getHeight();


#endif /* HEIGHT_H_ */

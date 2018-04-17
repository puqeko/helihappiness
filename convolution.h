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
#define UNIFORM 'u'


// *******************************************************
// convGetConvArray: Generate values for convolution array for averaging.
void convGetConvArray (uint32_t convolutionSize, char type, float *convolutionArray);

// *******************************************************
// getAverage: calculate the (approximate) mean of the values in the
// circular buffer and return it.
uint32_t getAverage (uint32_t convolutionSize);

// *******************************************************
void initConv(uint32_t convolutionSize);
void handleNewADCValue(uint32_t);
float convDoConvolution (float *convolutionArray, uint32_t convolutionSize);

#endif /* CONVOLUTION_H_ */

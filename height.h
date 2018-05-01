#ifndef HEIGHT_H_
#define HEIGHT_H_

// *******************************************************
// height.h
//
// Generates and uses averaging function to smooth data stream from ADCs
// P.J. Bones UCECE, modified by Ryan H and Thomas M
// Last modified:  17.04.2017
// Helicopter project
// Group: A03 Group 10
// *******************************************************

#include <stdint.h>
#include <stdbool.h>

#include "utils/ustdlib.h"
#include "driverlib/gpio.h"       // defines for GPIO peripheral
#include "driverlib/sysctl.h"     // system control functions
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"

// *******************************************************
#define ADC_SAMPLE_RATE 160  // Hz
#define CONV_SIZE 20

// convolution type
typedef enum height_conv_t {
    CONV_UNIFORM
} height_conv;


void heightInit(height_conv convType);

void heightCalibrate(void);

int32_t heightGetRaw(void);

// Returns a percentage form 0 to 100 scaled by precision
int32_t heightAsPercentage(int32_t precision);

void heightUpdate(void);


#endif /* HEIGHT_H_ */

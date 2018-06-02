// ************************************************************
// height.h
// Helicopter project
// Group: A03 Group 10
// Last edited: 18.04.2017
//
// Generates and uses averaging function to smooth data stream from ADCs
// Warning: Systick timer is used, so don't use it for other stuff
// ************************************************************

#ifndef HEIGHT_H_
#define HEIGHT_H_

#include <stdint.h>
#include <stdbool.h>

#include "utils/ustdlib.h"
#include "driverlib/gpio.h"       // defines for GPIO peripheral
#include "driverlib/sysctl.h"     // system control functions
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"

// *******************************************************
#define ADC_SAMPLE_RATE 160  // Hz
#define CONV_SIZE 20  // length of convolution array


// convolution type used for avera
typedef enum height_conv_t {
    CONV_UNIFORM
} height_conv;


// setup the system tick timer to run adc
void heightInit(height_conv convType);


// reset the height with which the percentage is calculated in reference to
void heightCalibrate(void);


// return the raw adc height value after averaging
int32_t heightGetRaw(void);


// returns a percentage height from 0 to 100 scaled by precision
int32_t heightAsPercentage(int32_t precision);


// recalculate the averaged height
void heightUpdate(void);


#endif /* HEIGHT_H_ */

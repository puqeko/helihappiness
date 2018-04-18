#ifndef HEIGHT_H_
#define HEIGHT_H_

// *******************************************************
// height.h
//
// Generates and uses averaging function to smooth data stream from ADCs
// P.J. Bones UCECE, modified by Ryan hall
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
enum conv_type {CONV_UNIFORM};

void heightInit(enum conv_type);

uint32_t heightGetRaw(void);

uint32_t heightAsPercentage(void);

void heightCalibrate(void);

#endif /* HEIGHT_H_ */

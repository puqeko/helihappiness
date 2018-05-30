// ************************************************************
// adcModule.h
// Helicopter project
// Group: A03 Group 10
// Last edited: 16-04-2018
//
// Purpose: Initialize and handle analog to digital
//          conversion (ADC) peripheral
// ************************************************************

#ifndef ADC_MODULE_H_
#define ADC_MODULE_H_

#include <stdint.h>

// Function pointer definition for the specifiable ADC value handler
typedef void (*valueHandler_t)(uint32_t);


// Initiate an ADC conversion
void adcTrigger(void);


// Initialize ADC and register interrupt handler
// Parameters:
// valueHandler_t handler -> handler function called to store ADC value
void adcInit(valueHandler_t);

#endif /*ADC_MODULE_H_*/

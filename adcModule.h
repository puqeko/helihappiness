#ifndef ADC_MODULE_H_
#define ADC_MODULE_H_

// ***********************************************************************
// adcModule.h
//
// Helicopter project
//
// Group:       A03 Group 10
// Created:     14/3/18
// Last Edited: 15/3/18
//
// Purpose: Initialise and handle analogue to digital conversion (ADC)
// ************************************************************************

#include <stdint.h>

//Function pointer definition for the specifiable ADC value handler
typedef void (*valueHandler_t)(uint32_t);


//************************************************************************
// Interrupt handler for completion of ADC conversion
// Resets interrupt and registers ADC value with ADCValueHandler function
//************************************************************************
void ADCIntHandler(void);


//************************************************************************
// Initiate an ADC conversion
//************************************************************************
void triggerADC(void);


//************************************************************************
// Initialize ADC and register interrupt handler
//
// Parameters:
// -valueHandler_t handler -> handler function called to store ADC value
//************************************************************************
void initADC(valueHandler_t);

#endif

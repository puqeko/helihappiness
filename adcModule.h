#ifndef ADC_MODULE
#define ADC_MODULE

// ***********************************************************************
// adcModule.c
//
// Helicopter project
//
// Group:       A03 Group 10
// Created:     14/3/18
// Last Edited: 14/3/18
//
// Purpose: Initialise and handle analogue to digital conversion (ADC)
// ************************************************************************

#include <stdint.h>

typedef void (*valueHandler_t)(uint32_t);

void ADCIntHandler(void);

void triggerADC(void);

void initADC(valueHandler_t);

#endif

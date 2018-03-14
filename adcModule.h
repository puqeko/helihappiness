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

void ADCIntHandler(void);

void triggerADC(void);

void initADC(void (*handler)(uint32_t));

#endif

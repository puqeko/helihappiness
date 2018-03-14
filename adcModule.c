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


#include "adcModule.h"
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/adc.h"
#include "driverlib/pwm.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/debug.h"
#include "utils/ustdlib.h"
#include "circBufT.h"
#include "OrbitOLED/OrbitOLEDInterface.h"

static valueHandler_t ADCValueHandler;

void ADCIntHandler(void)
{
    uint32_t ADCValue;
    ADCSequenceDataGet(ADC0_BASE, 3, &ADCValue);
    (*ADCValueHandler)(ADCValue);
    ADCIntClear(ADC0_BASE, 3);
}

void triggerADC(void)
{
    ADCProcessorTrigger(ADC0_BASE, 3);
}

void initADC(valueHandler_t handler)
{
    ADCValueHandler = handler;
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, 3);
    ADCIntRegister (ADC0_BASE, 3, ADCIntHandler);
    ADCIntEnable(ADC0_BASE, 3);
}

//************************************************************************
// adcModule.c
//
// Helicopter project
//
// Group:       A03 Group 10
// Created:     14/3/18
// Last Edited: 15/3/18
//
// Purpose: Initialize and handle analog to digital
//          conversion (ADC) peripheral
//************************************************************************


#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/adc.h"
#include "driverlib/sysctl.h"

#include "adcModule.h"

//function called to store ADC value
static valueHandler_t ADCValueHandler;


//************************************************************************
// Interrupt handler for completion of ADC conversion
// Resets interrupt and registers ADC value with ADCValueHandler function
//************************************************************************
void adcIntHandler(void)
{
    uint32_t ADCValue;

    //get the ADC sample value from ADC0
    ADCSequenceDataGet(ADC0_BASE, 3, &ADCValue);

    //register the ADC value with the specified ADCValueHandler function
    ADCValueHandler(ADCValue);

    //clear the interrupt
    ADCIntClear(ADC0_BASE, 3);
}



//************************************************************************
// Initiate an ADC conversion
//************************************************************************
void adcTrigger(void)
{
    ADCProcessorTrigger(ADC0_BASE, 3);
}



//************************************************************************
// Initialize ADC and register interrupt handler
//
// Parameters:
// -valueHandler_t handler -> handler function called to store ADC value
//************************************************************************
void adcInit(valueHandler_t handler)
{
    ADCValueHandler = handler;

    //enable ADC peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    // Enable sample sequence 3 with a processor signal trigger.  Sequence 3
    // will do a single sample when the processor sends a signal to start the
    // conversion.
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

    // Configure step 0 on sequence 3.  Sample channel 0 (ADC_CTL_CH0) in
    // single-ended mode (default) and configure the interrupt flag
    // (ADC_CTL_IE) to be set when the sample is done.  Tell the ADC logic
    // that this is the last conversion on sequence 3 (ADC_CTL_END).  Sequence
    // 3 has only one programmable step.  Sequence 1 and 2 have 4 steps, and
    // sequence 0 has 8 programmable steps.  Since we are only doing a single
    // conversion using sequence 3 we will only configure step 0.  For more
    // on the ADC sequences and steps, refer to the LM3S1968 datasheet.
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END); // ADC_CTL_CH9, ADC_CTL_CH0 = rig, pot

    // Since sample sequence 3 is now configured, it must be enabled.
    ADCSequenceEnable(ADC0_BASE, 3);

    // Register the interrupt handler
    ADCIntRegister (ADC0_BASE, 3, adcIntHandler);

    // Enable interrupts for ADC0 sequence 3 (clears any outstanding interrupts)
    ADCIntEnable(ADC0_BASE, 3);
}

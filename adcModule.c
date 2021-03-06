// ************************************************************
// adcModule.c
// Helicopter project
// Group: A03 Group 10
// Last edited: 16-04-2018
//
// Purpose: Initialize and handle analog to digital
//          conversion (ADC) peripheral
// ************************************************************

#include "adcModule.h"

#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/adc.h"
#include "driverlib/sysctl.h"

//function called to store ADC value
static valueHandler_t adcValueHandler;


// Interrupt handler for completion of ADC conversion
// Resets interrupt and registers ADC value with ADCValueHandler function
void adcIntHandler(void)
{
    uint32_t adcValue;

    //get the ADC sample value from ADC0
    ADCSequenceDataGet(ADC0_BASE, 3, &adcValue);

    //register the ADC value with the specified ADCValueHandler function
    adcValueHandler(adcValue);

    //clear the interrupt
    ADCIntClear(ADC0_BASE, 3);
}


// Initiate an ADC conversion
void adcTrigger(void)
{
    ADCProcessorTrigger(ADC0_BASE, 3);
}


// Initialize ADC and register interrupt handler
//
// Parameters:
// valueHandler_t handler -> handler function called to store ADC value
//
// WARNING: Ensure passed handler function has a short execution time
//          as it is executed on every ADC conversion interrupt
void adcInit(valueHandler_t handler)
{
    adcValueHandler = handler;

    SysCtlPeripheralReset(SYSCTL_PERIPH_ADC0);  // reset for good measure

    // Enable ADC peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    // Enable sample sequence 3 with a processor signal trigger.  Sequence 3
    // will do a single sample when the processor sends a signal to start the
    // conversion.
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

    // Configure step 0 on sequence 3.  Sample channel 9 (ADC_CTL_CH9) in
    // single-ended mode (default) and configure the interrupt flag
    // (ADC_CTL_IE) to be set when the sample is done.  Tell the ADC logic
    // that this is the last conversion on sequence 3 (ADC_CTL_END).  Sequence
    // 3 has only one programmable step.  Sequence 1 and 2 have 4 steps, and
    // sequence 0 has 8 programmable steps.  Since we are only doing a single
    // conversion using sequence 3 we will only configure step 0.
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH9 | ADC_CTL_IE | ADC_CTL_END); // ADC_CTL_CH9, ADC_CTL_CH0 = rig, pot

    // Since sample sequence 3 is now configured, it must be enabled.
    ADCSequenceEnable(ADC0_BASE, 3);

    // Register the interrupt handler
    ADCIntRegister (ADC0_BASE, 3, adcIntHandler);

    // Enable interrupts for ADC0 sequence 3 (clears any outstanding interrupts)
    ADCIntEnable(ADC0_BASE, 3);
}

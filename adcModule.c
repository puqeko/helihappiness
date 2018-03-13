#include "adcModule.h"

void triggerADC()
{
    ADCProcessorTrigger(ADC0_BASE, 3);
}

void initADC(void (*handler)(void))
{

}

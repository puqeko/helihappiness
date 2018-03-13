#ifndef ADC_MODULE
#define ADC_MODULE

void ADCIntHandler(/*?*/);

void triggerADC(void);

void initADC(void (*handler)(void));

#endif

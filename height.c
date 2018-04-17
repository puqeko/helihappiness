// *******************************************************
// height.c
//
// Generates and uses averaging function to smooth data stream from ADCs
// P.J. Bones UCECE, modified by Ryan Hall
// Last modified:  18.04.2017
// Helicopter project
// Group: A03 Group 10
// *******************************************************

#include "height.h"
#include "circBufT.h"
#include "adcModule.h"

#define CONV_UNIFORM_MULTIPLIER 100
#define CONV_BASE (CONV_SIZE * CONV_UNIFORM_MULTIPLIER)
#define ADC_MAX_RANGE 4095
// How many divisions in a 0.8 voltage range if the rail is 3.3 volts
// 0.8 volts is assumed as the range of motion
#define MEAN_RANGE (ADC_MAX_RANGE * 8 / 33)

static circBuf_t buf;
static int convolutionArray[CONV_SIZE];
static uint32_t baseMean = 0;


void SysTickIntHandler(void)
{
    adcTrigger();
}


void handleNewADCValue(uint32_t val)
{
    writeCircBuf(&buf, val);
}


void getConvArray(enum conv_type convType)
{
    int i;
    switch (convType)
    {
    case CONV_UNIFORM:
        for (i = 0; i < CONV_SIZE; i++) {
            convolutionArray[i] = CONV_UNIFORM_MULTIPLIER;
        }
        break;
    }
}


uint32_t heightGetRaw(void)
{
    uint32_t sum = 0;
    int i;
    for (i = 0; i < CONV_SIZE; i++) {
        sum = sum + (readCircBuf (&buf) * convolutionArray[i]);
    }
    return sum / CONV_BASE;
}


uint32_t heightAsPercentage()
{
    uint32_t mean = heightGetRaw();
    return 100 * ((int32_t)baseMean - (int32_t)mean) / MEAN_RANGE;
}


void heightCalibrate(void)
{
    baseMean = heightGetRaw();
}


void heightInit(enum conv_type convType)
{
    SysTickPeriodSet(SysCtlClockGet() / ADC_SAMPLE_RATE);  // frequency of 120 Hz
    SysTickIntRegister(SysTickIntHandler);
    // Enable interrupt and device
    SysTickIntEnable();
    SysTickEnable();
    initCircBuf(&buf, CONV_SIZE);
    adcInit(handleNewADCValue);
    getConvArray (convType);
}

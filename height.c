// ************************************************************
// height.c
// Helicopter project
// Group: A03 Group 10
// Created by P.J. Bones UCECE
// Last edited: 18.04.2017
//
// Generates and uses averaging function to smooth data stream from ADCs
// Warning: Systick timer is used, so don't use it for other stuff
// P.J. Bones UCECE, modified by Ryan Hall
// ************************************************************

#include "height.h"
#include "circBufT.h"
#include "adcModule.h"

#define   100
#define CONV_BASE (CONV_SIZE * CONV_UNIFORM_MULTIPLIER)
#define ADC_MAX_RANGE 4095

// How many divisions in a 0.8 voltage range if the rail is 3.3 volts
// 0.8 volts is assumed as the range of motion
#define MEAN_RANGE (ADC_MAX_RANGE * 8 / 33)

static circBuf_t buf;
static int32_t convolutionArray[CONV_SIZE];
static int32_t baseMean = 0;
static int32_t meanHeight = 0;


// start an adc read for each system tick internal
void SysTickIntHandler(void)
{
    adcTrigger();
}


// handle the result of the adc read by storing in the buffer
void handleNewADCValue(uint32_t val)
{
    writeCircBuf(&buf, val);
}


// setup the system tick timer to run adc
void heightInit(height_conv convType)
{
    SysTickPeriodSet(SysCtlClockGet() / ADC_SAMPLE_RATE);  // frequency of 120 Hz
    SysTickIntRegister(SysTickIntHandler);
    // Enable interrupt and device
    SysTickIntEnable();
    SysTickEnable();
    initCircBuf(&buf, CONV_SIZE);
    adcInit(handleNewADCValue);

    // Initalise convolution array for averaging.
    // It is possible to add more types of convolution (e.g. gaussian, triangle, lpf, etc)
    int i;
    switch (convType) {
    case CONV_UNIFORM:
        for (i = 0; i < CONV_SIZE; i++) {
            convolutionArray[i] = CONV_UNIFORM_MULTIPLIER;
        }
        break;
    }
}


// reset the height with which the percentage is calculated in reference to
void heightCalibrate(void)
{
    baseMean = heightGetRaw();
}


// return the raw adc height value after averaging
int32_t heightGetRaw(void)
{
    return meanHeight;
}


// returns a percentage height from 0 to 100 scaled by precision
int32_t heightAsPercentage(int32_t precision)
{
    return 100 * precision * (baseMean - meanHeight) / MEAN_RANGE;
}


// recalculate the averaged height
void heightUpdate(void)
{
    int32_t sum = 0;
    int i;
    for (i = 0; i < CONV_SIZE; i++) {
        sum = sum + (readCircBuf (&buf) * convolutionArray[i]);
    }
    meanHeight = sum / CONV_BASE;
}

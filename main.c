// ************************************************************
// main.c
// Helicopter project
// Group: A03 Group 10
// Last edited: 12-03-2018
//
// Purpose: This program may destroy helicopters.
// ************************************************************

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// TivaWare library includes
#include "inc/hw_memmap.h"        // defines GPIO_PORTF_BASE
#include "inc/tm4c123gh6pm.h"     // defines interrupt vectors and register addresses
#include "driverlib/gpio.h"       // defines for GPIO peripheral
#include "driverlib/sysctl.h"     // system control functions
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"

// 3rd party libraries
#include "buttons4.h"             // left, right, up, down buttons (debouncing)
#include "adcModule.h"
#include "utils/ustdlib.h"
#include "circBufT.h"
#include "OrbitOLED/OrbitOLEDInterface.h"
#include "yaw.h"
#include "height.h"

#define GREEN_LED GPIO_PIN_3
#define UNIFORM 'u'

void initalise(uint32_t clock_rate)
{
    // .. do any pin configs, timer setups, interrupt setups, etc
    initButtons();
    OLEDInitialise();
    yawInit();

    // Enable GPIO Port F
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    IntMasterEnable();

    // Set up the specific port pin as medium strength current & pull-down config.
    // Refer to TivaWare peripheral lib user manual for set up for configuration options
    GPIOPadConfigSet(GPIO_PORTF_BASE, GREEN_LED, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPD);

    // Set data direction register as output
    GPIODirModeSet(GPIO_PORTF_BASE, GREEN_LED, GPIO_DIR_MODE_OUT);

    // Write a zero to the output pin 3 on port F
    GPIOPinWrite(GPIO_PORTF_BASE, GREEN_LED, 0x00);
}


enum heli_state {LANDED = 0, FLYING, NUM_HELI_STATES};
enum display_state {PERCENTAGE = 0, MEAN_ADC, DISPLAY_OFF, NUM_DISPLAY_STATES};
uint8_t current_heli_state = LANDED;
uint8_t current_display_state = PERCENTAGE;

char meanFormatString[] = "Mean ADC = %4d";
char percentFormatString[] = "Height = %3d%%";
#define DISPLAY_CHAR_WIDTH 16

void displayValueWithFormat(char* format, uint32_t value)
{
    char str[17] = "                 ";  // 16 characters across the display
    usnprintf (str, sizeof(str), format, value);
    str[strlen(str)] = ' ';  // overwrite null terminator added by usnprintf
    str[DISPLAY_CHAR_WIDTH] = '\0';  // ensure there is one at the end of the string
    OLEDStringDraw (str, 0, 1);
}

void displayClear()
{
    OLEDStringDraw ("                 ", 0, 1);  // 16 characters across the display
}

#define ADC_MAX_RANGE 4095
// How many divisions in a 0.8 voltage range if the rail is 3.3 volts
// 0.8 volts is assumed as the range of motion
#define MEAN_RANGE (ADC_MAX_RANGE * 8 / 33)
uint32_t baseMean = 0;

void displayMode(uint32_t clock_rate)
{
    uint32_t mean = getHeight();
    int32_t percentage;

    switch (current_display_state)
    {
    case MEAN_ADC:
        displayValueWithFormat(meanFormatString, mean);
        break;

    case PERCENTAGE:
        // this is okay because the mean is capped to 4095
        percentage = 100 * ((int32_t)baseMean - (int32_t)mean) / MEAN_RANGE;
        displayValueWithFormat(percentFormatString, yawGetDegrees());
        break;

    case DISPLAY_OFF:
        displayClear();
        break;

    }
}

void heliMode(uint32_t clock_rate)
{
    switch (current_heli_state) {

    // M1.3 Measure the mean sample value for a bit and display on the screen
    case LANDED:
        displayValueWithFormat(percentFormatString, 0);  // clear to zero

        GPIOPinWrite(GPIO_PORTF_BASE,  GREEN_LED, GREEN_LED);
        SysCtlDelay(clock_rate / 3 * CONV_SIZE / ADC_SAMPLE_RATE);
        //baseMean = getAverage(CONV_SIZE);  // take new average to be the lowest value
        baseMean = getHeight();

        current_heli_state = FLYING;
        break;  // measure 0% height value

    // M1.4 Display altitude
    case FLYING:

        // Run M1.3 again (M1.5)
        if (checkButton(LEFT) == PUSHED) {
            current_heli_state = LANDED;
            current_display_state = PERCENTAGE;
        }

        // Transition between display modes (M1.6)
        if (checkButton(UP) == PUSHED) {
            current_display_state += 1;
            current_display_state %= NUM_DISPLAY_STATES;
        }

        GPIOPinWrite(GPIO_PORTF_BASE,  GREEN_LED, 0x00);
        break;
    }
}

int main(void) {
    uint32_t clock_rate;
	// Set system clock rate to 20 MHz.
	SysCtlClockSet(SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ | SYSCTL_SYSDIV_10);

	SysCtlDelay(100);  // Allow time for the oscillator to settle down. Uses 3 instructions per loop.
	
	clock_rate = SysCtlClockGet();  // Get the clock rate in pulses/s.
	
	initalise(clock_rate);
	
	initConv(UNIFORM);

	// main loop
	while (true) {

	    // TODO: this function is not very accurate so choose a different delay method
	    SysCtlDelay(clock_rate / 3 / 100);  // 100 hz
	    updateButtons();  // recommended 100 hz update
	    heliMode(clock_rate);
	    displayMode(clock_rate);
	}
}

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

// TivaWare library includes
#include "inc/hw_memmap.h"        // defines GPIO_PORTF_BASE
#include "inc/tm4c123gh6pm.h"     // defines interrupt vectors and register addresses
#include "driverlib/gpio.h"       // defines for GPIO peripheral
#include "driverlib/sysctl.h"     // system control functions

// 3rd party libraries
#include "buttons4.h"             // left, right, up, down buttons (debouncing)
#include "utils/ustdlib.h"
#include "OrbitOLED/OrbitOLEDInterface.h"

#define GREEN_LED GPIO_PIN_3

void initalise(uint32_t clock_rate)
{
    // .. do any pin configs, timer setups, interrupt setups, etc

    initButtons();
    OLEDInitialise();

    // Enable GPIO Port F
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

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


#define MEAN_RANGE (4095 * 33 / 8)  // How many divisions in a 0.8 voltage range if the rail is 3.3 volts


void displayMode()
{
    uint32_t mean = getAverage();

    switch (display_state)
    {
    case MEAN_ADC:
        char string[17];  // 16 characters across the display

        usnprintf (string, sizeof(string), "Mean ADC = %4d", mean);
        OLEDStringDraw (string, 0, 1);

    case PERCENTAGE:
        char string[17];  // 16 characters across the display
        // scale from mean range to 100 percent where baseVal is 0%
        uint32_t percentage = 100 * (baseVal - mean) / MEAN_RANGE;
        percentage = (percentage > 100) ? 100 : percentage;  // clamp to range 0 - 100

        usnprintf (string, sizeof(string), "Height = %3d%", percetage);
        OLEDStringDraw (string, 0, 1);

    case DISPLAY_OFF:
        char string[17] = "                ";
        OLEDStringDraw (string, 0, 1);
    }
}


void heliMode()
{
    switch (current_heli_state) {

    // M1.3 Measure the mean sample value for a bit and display on the screen
    case LANDED:
        GPIOPinWrite(GPIO_PORTF_BASE,  GREEN_LED, GREEN_LED);
        SysCtlDelay(clock_rate / 3);
        current_heli_state = FLYING;
        break;  // measure 0% height value

    // M1.4 Display altitude
    case FLYING:

        // Run M1.3 again (M1.5)
        if (checkButton(LEFT) == PUSHED) {
            current_heli_state = LANDED;
            current_display_state = PERCENTAGE;  // reset for init sequence
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
	
	// main loop
	while (true) {

	    // TODO: this function is not very accurate so choose a different delay method
	    SysCtlDelay(clock_rate / 3 / 100);  // 100 hz

	    updateButtons();  // recommended 100 hz update

	    heliMode();
	    displayMode();
	}
}

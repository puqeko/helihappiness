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

#include "buttons4.h"             // left, right, up, down buttons (debouncing)

#include "utils/ustdlib.h"
#include "OrbitOLED/OrbitOLEDInterface.h"

void initalise(uint32_t clock_rate)
{
    // .. do any pin configs, timer setups, interrupt setups, etc

    initButtons();
    OLEDInitialise();
}

enum heli_state {LANDED, FLYING, NUM_HELI_STATES};
enum display_state {PERCENTAGE, MEAN_ADC, DISPLAY_OFF, NUM_DISPLAY_STATES};

uint8_t current_heli_state = LANDED;
uint8_t current_display_state = PERCENTAGE;

int main(void) {
    uint32_t clock_rate;
	// Set system clock rate to 20 MHz.
	SysCtlClockSet(SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ | SYSCTL_SYSDIV_10);

	SysCtlDelay(100);  // Allow time for the oscillator to settle down.
	// SysCtlDelay() is an API function which executes a 3-instruction loop the number of
	//   times specified by the argument).
	
	clock_rate = SysCtlClockGet();  // Get the clock rate in pulses/s.
	
	initalise(clock_rate);
	
	while (true) {
	    // .. things that need continuous updates

	    updateButtons();

	    switch (current_heli_state) {

	    // M1.3 Measure the mean sample value for a bit and display on the screen
        case LANDED:
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
            break;
	    }
	}
}

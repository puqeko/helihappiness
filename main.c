// ************************************************************
// main.c
// Helicopter project
// Group: A03 Group 10
// Last edited: 12-03-2018
//
// Purpose: This program may destory helicopters.
// ************************************************************

//Comment by Ryan Hall to test with git

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// TivaWare library includes
#include "inc/hw_memmap.h"        // defines GPIO_PORTF_BASE
#include "inc/tm4c123gh6pm.h"     // defines interrupt vectors and register addresses
#include "driverlib/gpio.h"       // defines for GPIO peripheral
#include "driverlib/sysctl.h"     // system control functions


void initalise(uint32_t clock_rate)
{
    // .. do any pin configs, timer setups, interupt setups, etc
}


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
	}
}

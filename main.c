// ************************************************************
// main.c
// Helicopter project
// Group: A03 Group 10
// Last edited: 18-04-2018
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
#include "circBufT.h"
#include "yaw.h"
#include "height.h"
#include "timerer.h"
#include "pwmModule.h"
#include "display.h"
#include "uartDisplay.h"

enum heli_state {LANDED = 0, LANDING, ALIGNING, FLYING, NUM_HELI_STATES};
static uint8_t current_heli_state = LANDED;

char meanFormatString[] = "Mean ADC = %4d";
char yawFormatString[] = "     Yaw = %4d~";
char percentFormatString[] = "  Height = %4d%%";
char dutyCycleFormatString[] = " M = %2d, T = %2d";

uint32_t ui32DutyMain = 0;
uint32_t ui32DutyTail = 0;

void initalise()
{
    // .. do any pin configs, timer setups, interrupt setups, etc
    initButtons();
    displayInit();
    heightInit(CONV_UNIFORM);
    yawInit();
    initClocks();
    initialisePWM();
    initialiseUSB_UART();

    // Enable GPIO Port F
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
}


void heliMode(void)
{
    switch (current_heli_state) {

    case LANDED:
        displayValueWithFormat(percentFormatString, 0, 1);  // clear to zero

        timererWait(1000 * CONV_SIZE / ADC_SAMPLE_RATE);  // in ms, hence the 1000
        heightCalibrate();

        current_heli_state = FLYING;
        break;  // measure 0% height value

    case LANDING:
        // done landing...
        ignoreButton(SW1);
        current_heli_state = LANDED;
        break;

    case ALIGNING:
        // done aligning...
        ignoreButton(SW1);
        current_heli_state = FLYING;

    // M1.4 Display altitude
    case FLYING:
        if (checkButton(UP) == PUSHED && ui32DutyMain < PWM_DUTY_MAX_HZ) {
            ui32DutyMain += PWM_DUTY_STEP_HZ;
            pwmSetDuty(ui32DutyMain, MAIN_ROTOR);
        }
        if (checkButton(DOWN) == PUSHED && ui32DutyMain > PWM_DUTY_MIN_HZ) {
            ui32DutyMain -= PWM_DUTY_STEP_HZ;
            pwmSetDuty(ui32DutyMain, MAIN_ROTOR);
        }
        if (checkButton(LEFT) == PUSHED && ui32DutyTail > PWM_DUTY_MIN_HZ) {
            ui32DutyTail -= PWM_DUTY_STEP_HZ;
            pwmSetDuty(ui32DutyTail, TAIL_ROTOR);
        }
        if (checkButton(RIGHT) == PUSHED && ui32DutyTail < PWM_DUTY_MAX_HZ) {
            ui32DutyTail += PWM_DUTY_STEP_HZ;
            pwmSetDuty(ui32DutyTail, TAIL_ROTOR);
        }
        displayTwoValuesWithFormat(dutyCycleFormatString, ui32DutyMain, ui32DutyTail, 3);

        // Run M1.3 again (M1.5)
        if (checkButton(SW1) == RELEASED) {  // switch down
            current_heli_state = LANDING;
        }
        break;
    }
}


int main(void) {
	// Set system clock rate to 20 MHz.
	SysCtlClockSet(SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ | SYSCTL_SYSDIV_10);
	timererInit();
	timererWait(1);  // Allow time for the oscillator to settle down.
	
	initalise();

    // Initialisation is complete, so turn on the output.
	pwmSetOutput(true, MAIN_ROTOR);
	pwmSetOutput(true, TAIL_ROTOR);

    // Enable interrupts to the processor.
    IntMasterEnable ();


    //Some fake variables to test UART
    char mode[] = "landed";

    int uartCount = 0;

	// main loop
	while (true) {
	    uint32_t referenceTime = timererGetTicks();

	    updateButtons();  // recommended 100 hz update
	    heliMode();

	    // this is okay because the mean is capped to 4095
	    uint32_t percentageHeight = heightAsPercentage();
	    displayValueWithFormat(percentFormatString, percentageHeight, 1);
	    // TODO: include duty cycle on OLED

//	    Test Uart here
	    if (uartCount == 25) {
	        UARTPrintfln("%s", "\n\n----------------\n");
	        UARTPrintfln("ALT: %d [%d] %%\n", 0, percentageHeight);
	        UARTPrintfln("YAW: %d [%d] deg\n", 0, yawGetDegrees());
	        UARTPrintfln("MAIN: %d %%, TAIL: %d %%\n", ui32DutyMain, ui32DutyTail);
	        UARTPrintfln("MODE: %s\n", mode);
	        uartCount = 0;
	    }
	    uartCount++;


	    timererWaitFrom(10, referenceTime);  // 100 hz, 10 ms
	}
}


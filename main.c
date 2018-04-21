// ************************************************************
// main.c
// Helicopter project
// Group: A03 Group 10
// Last edited: 21-04-2018
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
#include "pidControl.h"

enum heli_state {LANDED = 0, LANDING, ALIGNING, FLYING, NUM_HELI_STATES};
// list the mode that should be displayed for each state.
static const char* heli_state_map [] = {"Landed", "Landing", "Aligning", "Flying"};
static enum heli_state current_heli_state = LANDED;

uint32_t targetHeight = 0;
uint32_t targetYaw = 0;
uint32_t mainDuty = 0;
uint32_t tailDuty = 0;
uint32_t percentageHeight = 0;
uint32_t degreesYaw = 0;

#define DELTA_TIME 10  // 100 hz, 10 ms
#define UART_DISPLAY_FREQUENCY 4  // hz
#define LOOP_FREQUENCY (1000 / DELTA_TIME)

#define MIN_DUTY 10  // %
#define MAX_DUTY 95  // %
#define MAIN_STEP 10  // %
#define TAIL_STEP 15  // deg

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))


void initalise()
{
    // TODO: reset peripherals

    // Set system clock rate to 20 MHz.
    SysCtlClockSet(SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ | SYSCTL_SYSDIV_10);
    timererInit();
    timererWait(1);  // Allow time for the oscillator to settle down.

    initButtons();
    displayInit();
    yawInit();
    heightInit(CONV_UNIFORM);
    initClocks();
    initialisePWM();
    initialiseUSB_UART();
    IntMasterEnable ();  // Enable interrupts to the processor.
    timererWait(1);

    heightCalibrate();
    ignoreButton(SW1);
}


void heliMode(void)
{
    switch (current_heli_state) {

    case LANDED:

        timererWait(1000 * CONV_SIZE / ADC_SAMPLE_RATE);  // in ms, hence the 1000
        heightCalibrate();

        if (checkButton(SW1) == PUSHED) {
            current_heli_state = ALIGNING;
            pwmSetOutput(true, MAIN_ROTOR);
            pwmSetOutput(true, TAIL_ROTOR);
            mainDuty = MIN_DUTY;
            tailDuty = MIN_DUTY;
            targetHeight = 0;
        }
        break;

    case LANDING:
        // TODO: Ramp input for landing
        // done landing...
        ignoreButton(SW1);
        current_heli_state = LANDED;

        // Set motor speed
        pwmSetOutput(false, MAIN_ROTOR);
        pwmSetOutput(false, TAIL_ROTOR);
        mainDuty = 0;
        tailDuty = 0;
        targetHeight = 0;
        targetYaw = 0;
        break;

    case ALIGNING:
        // done aligning...
        ignoreButton(SW1);
        current_heli_state = FLYING;
        targetYaw = 0;
        break;

    case FLYING:
        if (checkButton(UP) == PUSHED && targetHeight < MAX_DUTY) {
            targetHeight += MAIN_STEP;
        }
        if (checkButton(DOWN) == PUSHED && targetHeight > MIN_DUTY) {
            targetHeight -= MAIN_STEP;
        }
        if (checkButton(LEFT) == PUSHED && targetYaw > MIN_DUTY) {
            targetYaw -= TAIL_STEP;
        }
        if (checkButton(RIGHT) == PUSHED && targetYaw < MAX_DUTY) {
            targetYaw += TAIL_STEP;
        }
        if (checkButton(SW1) == RELEASED) {  // switch down
            current_heli_state = LANDING;

            targetHeight = 0;
            targetYaw = 0;
        }

        // Apply proportional, integral, derivative control
        mainDuty = PIDUpdateHeight(targetHeight, percentageHeight, DELTA_TIME);
        tailDuty = PIDUpdateYaw(targetYaw, degreesYaw, DELTA_TIME);

        mainDuty = MIN(mainDuty, MAX_DUTY);
        mainDuty = MAX(mainDuty, MIN_DUTY);
        tailDuty = MIN(tailDuty, MAX_DUTY);
        tailDuty = MAX(tailDuty, MIN_DUTY);

        // Set motor speed
        pwmSetDuty(mainDuty, MAIN_ROTOR);
        pwmSetDuty(tailDuty, TAIL_ROTOR);

        break;
    }
}

int main(void)
{
    initalise();

    //Some fake variables to test UART
    int uartCount = 0;

	// main loop
	while (true) {
	    // don't include the time to execute the main loop in our time measurement
	    // so measure DELTA_TIME from this point
	    uint32_t referenceTime = timererGetTicks();

	    // Take measurements
	    percentageHeight = heightAsPercentage();
	    degreesYaw = yawGetDegrees();

	    // Update OLED display
	    displayValueWithFormat("  Height = %4d%%", percentageHeight, 1);  // line 1
	    displayTwoValuesWithFormat(" M = %2d, T = %2d", mainDuty, tailDuty, 2);  // line 2

	    // Update UART display
	    if (uartCount == LOOP_FREQUENCY / UART_DISPLAY_FREQUENCY) {
	        UARTPrintLineWithFormat("%s", "\n\n----------------\n");
	        UARTPrintLineWithFormat("ALT: %d [%d] %%\n", targetHeight, percentageHeight);
	        UARTPrintLineWithFormat("YAW: %d [%d] deg\n", targetYaw, degreesYaw);
	        UARTPrintLineWithFormat("MAIN: %d %%, TAIL: %d %%\n", mainDuty, tailDuty);
	        UARTPrintLineWithFormat("MODE: %s\n", heli_state_map[current_heli_state]);
	        uartCount = 0;
	    }
	    uartCount++;

	    // Update user inputs and run state machine
        updateButtons();  // recommended 100 hz update
        heliMode();

	    // Wait any time that remains for this cycle to take DELTA_TIME
	    timererWaitFrom(DELTA_TIME, referenceTime);
	}
}


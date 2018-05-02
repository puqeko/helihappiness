// ************************************************************
// main.c
// Helicopter project
// Group: A03 Group 10
// Last edited: 21-04-2018
//
// Purpose: This program may destroy helicopters.
// ************************************************************

#include <control.h>
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
#include "display.h"
#include "uartDisplay.h"
#include "control.h"

enum heli_state {LANDED = 0, LANDING, ALIGNING, FLYING, NUM_HELI_STATES};
// list the mode that should be displayed for each state.
static const char* heli_state_map [] = {"Landed", "Landing", "Aligning", "Flying"};
static enum heli_state current_heli_state = LANDED;

uint32_t targetHeight = 0;
uint32_t targetYaw = 0;

#define DELTA_TIME 10  // 100 hz, 10 ms
#define UART_DISPLAY_FREQUENCY 4  // hz
#define LOOP_FREQUENCY (1000 / DELTA_TIME)

#define MAIN_STEP 10  // %
#define TAIL_STEP 15  // deg

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
    initialiseUSB_UART();
    IntMasterEnable ();  // Enable interrupts to the processor.
    timererWait(1);

    heightCalibrate();
    controlInit();
}


void heliMode(void)
{
    switch (current_heli_state) {

    case LANDED:

        timererWait(1000 * CONV_SIZE / ADC_SAMPLE_RATE);  // in ms, hence the 1000
        heightCalibrate();

        if (checkButton(SW1) == PUSHED) {
            current_heli_state = ALIGNING;
            controlMotorSet(true);  // turn  on motors
            controlEnable(CONTROL_CALIBRATE);  // start calibration
            targetHeight = 0;
        }
        break;

    case ALIGNING:
        // calibration is auto disabled when complete
        if (!controlIsEnabled(CONTROL_CALIBRATE)) {
            // done aligning...
            ignoreButton(SW1);
            controlEnable(CONTROL_HEIGHT);
            controlEnable(CONTROL_YAW);
            current_heli_state = FLYING;
            targetYaw = 0;
        }
        break;

    case LANDING:
        // TODO: Ramp input for landing
        // done landing...
        ignoreButton(SW1);
        controlMotorSet(false);
        current_heli_state = LANDED;

        controlDisable(CONTROL_HEIGHT);
        controlDisable(CONTROL_YAW);
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
            // TODO: add landing control
            controlSetTarget(0, CONTROL_HEIGHT);
            controlSetTarget(0, CONTROL_YAW);
            current_heli_state = LANDING;
        }
        controlSetTarget(targetHeight, CONTROL_HEIGHT);
        controlSetTarget(targetYaw, CONTROL_YAW);
        break;
    }
}


void displayInfo()
{
    static int uartCount = 0;

    // Take measurements
    uint32_t percentageHeight = heightAsPercentage(1);  // precision = 1
    uint32_t degreesYaw = yawGetDegrees(1);  // precision = 1
    uint32_t mainDuty = controlGetPWMDuty(CONTROL_HEIGHT);
    uint32_t tailDuty = controlGetPWMDuty(CONTROL_YAW);

    // Update OLED display
    displayPrintLineWithFormat("  Height = %4d%%", 1, percentageHeight);  // line 1
    displayPrintLineWithFormat(" M = %2d, T = %2d", 2, mainDuty, tailDuty);  // line 2

    // Update UART display
    // Use a collaborative technique to update the display across updates

    switch (uartCount) {
    case UPDATE_COUNT:
        uartCount = 0;
        UARTPrintLineWithFormat("%s", "\n\n----------------\n");
        break;
    case UPDATE_COUNT + 1:
        UARTPrintLineWithFormat("ALT: %d [%d] %%\n", targetHeight, percentageHeight);
        break;
    case UPDATE_COUNT + 2:
        UARTPrintLineWithFormat("YAW: %d [%d] deg\n", targetYaw, degreesYaw);
        break;
    case UPDATE_COUNT + 3:
        UARTPrintLineWithFormat("MAIN: %d %%, TAIL: %d %%\n", mainDuty, tailDuty);
        break;
    case UPDATE_COUNT + 4:
        UARTPrintLineWithFormat("MODE: %s\n", heli_state_map[current_heli_state]);
        break;
    }
    uartCount++;
}


int main(void)
{
    initalise();

	// main loop
	while (true) {
	    // don't include the time to execute the main loop in our time measurement
	    // so measure DELTA_TIME from this point
	    uint32_t referenceTime = timererGetTicks();

	    heightUpdate();  // do convolution step
	    controlUpdate(DELTA_TIME);  // update control

	    displayInfo();

	    // Update user inputs and run state machine
        updateButtons();  // recommended 100 hz update
        heliMode();

	    // Wait any time that remains for this cycle to take DELTA_TIME
	    timererWaitFrom(DELTA_TIME, referenceTime);
	}
}


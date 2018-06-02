// ************************************************************
// main.c
// Helicopter project
// Group: A03 Group 10
// Last edited: 21-04-2018
//
// Purpose: Apply control to a helicopter rig which allows the user to set the height
// and yaw, and switch between a landed and flying mode.
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
#include "circBufT.h"

// Other
#include "adcModule.h"
#include "yaw.h"
#include "height.h"
#include "timerer.h"
#include "display.h"
#include "uartDisplay.h"
#include "control.h"
#include "kernel.h"
#include "quadratureEncoder.h"
#include "landingController.h"

#define TASK_BASE_FREQ 100  // Hz, the maximum frequency of a task
#define UART_DISPLAY_FREQUENCY 4  // Hz
#define UPDATE_DISPLAY_COUNT (TASK_BASE_FREQ / UART_DISPLAY_FREQUENCY)

#define MAIN_STEP 10  // %
#define TAIL_STEP 15  // deg


void softResetIntHandler(void)
{
    GPIOIntClear(GPIO_PORTA_BASE, GPIO_PIN_6);
    SysCtlReset();
}


void initSoftReset(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    GPIOPadConfigSet(GPIO_PORTA_BASE, GPIO_PIN_6, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU);
    GPIODirModeSet(GPIO_PORTA_BASE, GPIO_PIN_6, GPIO_DIR_MODE_IN);

    GPIOIntRegister(GPIO_PORTA_BASE, softResetIntHandler);
    GPIOIntTypeSet(GPIO_PORTA_BASE, GPIO_PIN_6, GPIO_FALLING_EDGE);
    GPIOIntEnable(GPIO_PORTA_BASE, GPIO_PIN_6);
}


void initalise()
{
    // Reset ports for good measure
    SysCtlPeripheralReset(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralReset(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralReset(SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralReset(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralReset(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralReset(SYSCTL_PERIPH_GPIOF);

    // Other peripherals are reset inside each module

    // Set system clock rate to 20 MHz.
    SysCtlClockSet(SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ | SYSCTL_SYSDIV_10);
    timererInit();
    timererWait(1);  // Allow time for the oscillator to settle down (for 1 millisecond).

    buttonsInit();
    initSoftReset();
    displayInit();
    yawInit();
    heightInit(CONV_UNIFORM);
    uartInit();

    // Enable interrupts to the processor.
    IntMasterEnable ();
    timererWait(1);
    heightCalibrate();
    controlInit();
}


void stateTransitionUpdate(state_t* state, uint32_t deltaTime)
{
    static bool shouldCalibrate = true;  // only calibrate the yaw once

    switch (state->heliMode) {
    case STATE_LANDED:
        heightCalibrate();  // always re-calibrate the height

        // wait for switch to trigger take off
        if (buttonsCheck(SW1) == PUSHED) {
            // start calibration and enable PID control on height and yaw
            controlEnable(state, CONTROL_HEIGHT);
            controlEnable(state, CONTROL_YAW);
            state->targetHeight = 0;

            // set integral controllers back to zero to prevent windup
            controlReset();

            if (shouldCalibrate) {
                controlEnable(state, CONTROL_CALIBRATE_YAW);
                state->heliMode = STATE_CALIBRATE_YAW;
            } else {
                state->heliMode = STATE_FLYING;
            }
        }
        break;

    case STATE_CALIBRATE_YAW:
        // seek the zero point for the yaw

        if (!controlIsEnabled(CONTROL_CALIBRATE_YAW)) {
            // when the zero point is found, move to the flying mode
            shouldCalibrate = false;
            state->targetYaw = 0;
            state->heliMode = STATE_FLYING;
        }
        break;

    case STATE_FLYING:
        // change height with buttons
        if (buttonsCheck(UP) == PUSHED && state->targetHeight < CONTROL_MAX_DUTY)
            state->targetHeight += MAIN_STEP;
        if (buttonsCheck(DOWN) == PUSHED && state->targetHeight > CONTROL_MIN_DUTY)
            state->targetHeight -= MAIN_STEP;

        // change yaw with buttons
        if (buttonsCheck(LEFT) == PUSHED)
            state->targetYaw -= TAIL_STEP;
        if (buttonsCheck(RIGHT) == PUSHED)
            state->targetYaw += TAIL_STEP;

        // switch one to go into the landing sequence
        if (buttonsCheck(SW1) == RELEASED) {  // switch down
            controlEnable(state, CONTROL_DESCENDING);
            state->heliMode = STATE_DESCENDING;
        }
        break;

    case STATE_DESCENDING:
        if (!controlIsEnabled(CONTROL_DESCENDING)) {
            // when the descending controller auto-disables, move to the power down sequence
            // and disable PID control on the height
            controlDisable(state, CONTROL_HEIGHT);
            controlEnable(state, CONTROL_POWER_DOWN);

            // prevent a toggle of the switch causing the heli to take off again once landed
            buttonsIgnore(SW1);
            state->heliMode = STATE_POWER_DOWN;
        }
        break;

    case STATE_POWER_DOWN:
        if (!controlIsEnabled(CONTROL_POWER_DOWN)) {
            // when the power down controller auto-disables, move to the landed sequence
            controlDisable(state, CONTROL_YAW);
            state->targetHeight = 0;
            state->targetYaw = 0;

            // round the yaw measurement so that we always return to 0 degrees by the shortest
            // path. i.e. don't unwind if we have spun multiple times.
            yawClipTo360Degrees();

            // prevent a toggle of the switch causing the heli to take off again once landed
            buttonsIgnore(SW1);
            state->heliMode = STATE_LANDED;
        }
        break;
    }
}


//
// Tasks to be run by the scheduler
//


void displayUpdate(state_t* state, uint32_t deltaTime)
{
    static int uartCount = 0;
    // Remember to update these strings when changing the states above
    // These are the string values to be displayed when in each state
    static const char* heliModeDisplayStringMap[] = {
       "Landed",
       "Calibrate Yaw",
       "Flying",
       "Descending",
       "Power Down"
    };

    // Take measurements
    uint32_t percentageHeight = heightAsPercentage(1);  // precision = 1
    uint32_t degreesYaw = yawGetDegrees(1);  // precision = 1

    // Update OLED display
    displayPrintLineWithFormat("Height = %4d%%", 1, percentageHeight);  // line 1
    displayPrintLineWithFormat("M = %2d, T = %2d", 2, state->outputMainDuty, state->outputTailDuty);  // line 2

    // Update UART display
    // Use a collaborative technique to update the display across update cycles.
    // Thus, the time spent on each call to displayUpdate is reduced allowing higher frequency tasks
    // to be serviced at more consistent rates.

    switch (uartCount) {
    case UPDATE_DISPLAY_COUNT - 5:
        uartPrintLineWithFormat("\nALT %d [%d] %%\n", state->targetHeight, percentageHeight);
        break;
    case UPDATE_DISPLAY_COUNT - 4:
        uartPrintLineWithFormat("YAW %d [%d] deg\n", state->targetYaw, degreesYaw);
        break;
    case UPDATE_DISPLAY_COUNT - 3:
        uartPrintLineWithFormat("MAIN %d %%, TAIL %d %%\n", state->outputMainDuty, state->outputTailDuty);
        break;
    case UPDATE_DISPLAY_COUNT - 2:
        uartPrintLineWithFormat("MODE %s\n", heliModeDisplayStringMap[state->heliMode]);
        break;
    case UPDATE_DISPLAY_COUNT - 1:
        uartPrintLineWithFormat("%s", "----------------\n");
        break;
    case UPDATE_DISPLAY_COUNT:
        uartCount = 0;
    }
    uartCount++;
}


void mainUpdate(state_t* state, uint32_t deltaTime)
{
    heightUpdate();  // recalculate height average
    controlUpdate(state, deltaTime);
    buttonsUpdate();
}


int main(void)
{
    initalise();

    // make sure ADC buffer has a chance to fill up for the height measurement
    timererWait(1000 * CONV_SIZE / ADC_SAMPLE_RATE);

    // the tasks which need to run at what frequency
    // the frequency cannot be larger than the TASK_BASE_FREQ
    task_t tasks[] = {
        {mainUpdate, 100},
        {displayUpdate, 100},  // actually about 4 Hz due to co-operative behaviour
        {stateTransitionUpdate, 5},
        {0}  // terminator (read until this value when processing the array)
    };

    // any data which many tasks might need to know about
    state_t sharedState = {
        .heliMode = STATE_LANDED,
        .targetHeight = 0,
        .targetYaw = 0,
        .outputMainDuty = 0,
        .outputTailDuty = 0
    };

    runTasks(tasks, &sharedState, TASK_BASE_FREQ);
}

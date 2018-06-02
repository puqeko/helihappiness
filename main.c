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
#include "circBufT.h"

// Other
#include "adcModule.h"
#include "yaw.h"
#include "height.h"
#include "timerer.h"
#include "display.h"
#include "uartDisplay.h"
#include "control.h"
#include "quadratureEncoder.h"
#include "landingController.h"
#include "kernalMustardWithThePipeInTheDiningRoom.h"


#define UART_DISPLAY_FREQUENCY 4  // hz
#define LANDING_UPDATE_FREQUENCY 10 // hz
#define UPDATE_DISPLAY_COUNT (TASK_BASE_FREQ / UART_DISPLAY_FREQUENCY)

#define HEIGHT_LANDING_COUNT (LOOP_FREQUENCY / LANDING_UPDATE_FREQUENCY)

#define MAIN_STEP 10  // %
#define TAIL_STEP 15  // deg

#define NUM_TASKS 5
#define TASK_BASE_FREQ 100


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
    // TODO: reset peripherals
    // SysCtlPeripheralReset(SYSCTL_PERIPH_PWM);

    // Set system clock rate to 20 MHz.
    SysCtlClockSet(SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ | SYSCTL_SYSDIV_10);
    timererInit();
    timererWait(1);  // Allow time for the oscillator to settle down (for 1.

    buttonsInit();
    initSoftReset();
    displayInit();
    yawInit();
    heightInit(CONV_UNIFORM);
    uartInit();
    IntMasterEnable ();  // Enable interrupts to the processor.
    timererWait(1);

    heightCalibrate();
    controlInit();

}

void heliMode(state_t* state, uint32_t deltaTime)
{
    static bool shouldCalibrate = true;

    switch (state->heliMode) {

    case LANDED:
        heightCalibrate();
        if (buttonsCheck(SW1) == PUSHED) {
            if (shouldCalibrate) {
                state->heliMode = CALIBRATE_YAW;
                yawCalibrate();
            } else {
                state->heliMode = FLYING;
            }

            // start calibration
            controlEnable(CONTROL_HEIGHT);
            controlEnable(CONTROL_YAW);
            state->targetHeight = 0;
            controlReset();
        }
        break;

    case CALIBRATE_YAW:
        //Find the zero point for the yaw
        state->targetYaw += 1;
        if (yawIsCalibrated()) {
            shouldCalibrate = false;
            state->heliMode = FLYING;
            state->targetYaw = 0;
        }
        break;

    case DESCENDING:
        if (!controlIsEnabled(CONTROL_DESCENDING)) {
            buttonsIgnore(SW1);
            controlDisable(CONTROL_HEIGHT);
            controlEnable(CONTROL_POWER_DOWN);
            state->heliMode = POWER_DOWN;
        }
        break;

    case POWER_DOWN:
        if (!controlIsEnabled(CONTROL_POWER_DOWN)) {
            buttonsIgnore(SW1);
            controlDisable(CONTROL_YAW);
            state->heliMode = LANDED;
            state->targetHeight = 0;
            if (yawClipTo360Degrees()) {
                state->targetYaw = 0;
            }
        }
        break;

    case FLYING:
        if (buttonsCheck(UP) == PUSHED && state->targetHeight < MAX_DUTY)
            state->targetHeight += MAIN_STEP;
        if (buttonsCheck(DOWN) == PUSHED && state->targetHeight > MIN_DUTY)
            state->targetHeight -= MAIN_STEP;

        if (buttonsCheck(LEFT) == PUSHED)
            state->targetYaw -= TAIL_STEP;
        if (buttonsCheck(RIGHT) == PUSHED)
            state->targetYaw += TAIL_STEP;

        if (buttonsCheck(SW1) == RELEASED) {  // switch down
            state->heliMode = DESCENDING;
            controlEnable(CONTROL_DESCENDING);
        }
        break;
    }
}


void displayUpdate(state_t* state, uint32_t deltaTime)
{
    static int uartCount = 0;
    // Remember to update these strings when changing the states above
    // These are the string values to be displayed when in each state
    static const char* heliModeDisplayStringMap[] = {
       "Landed",
       "Descending",
       "Power Down",
       "Flying",
       "Calibrate Yaw"
    };

    // Take measurements
    uint32_t percentageHeight = heightAsPercentage(1);  // precision = 1
    uint32_t degreesYaw = yawGetDegrees(1);  // precision = 1

    // Update OLED display
    displayPrintLineWithFormat("Height = %4d%%", 1, percentageHeight);  // line 1
    displayPrintLineWithFormat("M = %2d, T = %2d", 2, state->outputMainDuty, state->outputTailDuty);  // line 2

    // Update UART display
    // Use a collaborative technique to update the display across updates

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


void controllerUpdate(state_t* state, uint32_t deltaTime)
{
    heightUpdate();
    controlUpdate(state, deltaTime);
}


void stateUpdate(state_t* state , uint32_t deltaTime)
{
    buttonsUpdate();
    heliMode(state, deltaTime);
}


int main(void)
{
    initalise();

    timererWait(1000 * CONV_SIZE / ADC_SAMPLE_RATE);  // make sure ADC buffer has a chance to fill up

    // the tasks which need to run at what frequency
    // the frequency cannot be larger than the TASK_BASE_FREQ
    task_t tasks[] = {
        {controllerUpdate, 100},
        {displayUpdate, 100},
        {stateUpdate, 100},
        {0}  // terminator (read until this value when processing the array)
    };

    // any data which many tasks might need to know about
    state_t sharedState = {
        .heliMode = LANDED,
        .targetHeight = 0,
        .targetYaw = 0,
        .outputMainDuty = 0,
        .outputTailDuty = 0
    };

    runTasks(tasks, &sharedState, TASK_BASE_FREQ);
}

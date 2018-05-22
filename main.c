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
#include "quadratureEncoder.h"

typedef enum {
    LANDED = 0, LANDING, ALIGNING, FLYING, CALIBRATE_YAW, NUM_HELI_STATES
} heli_state_e;

#define UART_DISPLAY_FREQUENCY 4  // hz
#define LANDING_UPDATE_FREQUENCY 10 // hz

#define HEIGHT_LANDING_COUNT (LOOP_FREQUENCY / LANDING_UPDATE_FREQUENCY)
#define STABILITY_TIME_MAIN 500 // 500 ms
#define STABILITY_TIME_TAIL 2000 // 2000 ms

#define MAIN_STEP 10  // %
#define TAIL_STEP 15  // deg

typedef struct {
    heli_state_e heliMode;
    uint32_t targetHeight;
    uint32_t targetYaw;
} state_t;

typedef struct {
    void (*handler) (state_t* state, uint32_t deltaTime);  // pointer to task handler function
    uint32_t updateFreq;  // number of ms between runs
    uint32_t count;
    uint32_t triggerAt;
} task_t;


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

    // Set system clock rate to 20 MHz.
    SysCtlClockSet(SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ | SYSCTL_SYSDIV_10);
    timererInit();
    timererWait(1);  // Allow time for the oscillator to settle down (for 1.

    initButtons();
//    initSoftReset();
    displayInit();
    yawInit();
    heightInit(CONV_UNIFORM);
    initialiseUSB_UART();
    IntMasterEnable ();  // Enable interrupts to the processor.
    timererWait(1);

    heightCalibrate();
    controlInit();

}


void heliMode(state_t* state, uint32_t deltaTime)
{
    static int stabilityCounter;
    static bool shouldCalibrate = true;

    switch (state->heliMode) {

    case LANDED:
        heightCalibrate();
        if (checkButton(SW1) == PUSHED) {
            if (shouldCalibrate) {
                state->heliMode = CALIBRATE_YAW;
                quadEncoderCalibrate();
            } else {
                state->heliMode = ALIGNING;
            }
            controlMotorSet(true, MAIN_ROTOR);  // turn  on motors
            controlMotorSet(true, TAIL_ROTOR);
              // start calibration
            controlEnable(CONTROL_CALIBRATE_MAIN);
            controlEnable(CONTROL_CALIBRATE_TAIL);
            controlEnable(CONTROL_HEIGHT);
            controlEnable(CONTROL_YAW);
            state->targetHeight = 0;
            resetController();
        }
        break;

    case ALIGNING:
        // calibration is auto disabled when complete
        if (!controlIsEnabled(CONTROL_CALIBRATE_MAIN) && !controlIsEnabled(CONTROL_CALIBRATE_TAIL)) {
            // done aligning...
            ignoreButton(SW1);
            controlEnable(CONTROL_HEIGHT);
            controlEnable(CONTROL_YAW);
            state->heliMode = FLYING;
            state->targetYaw = 0;
        }
        break;

    case CALIBRATE_YAW:
        //Find the zero point for the yaw
        state->targetYaw += 1;
        controlSetTarget(state->targetYaw, CONTROL_YAW);
        controlSetTarget(state->targetHeight, CONTROL_HEIGHT);
        if (quadEncoderIsCalibrated()) {
            shouldCalibrate = false;
            state->heliMode = ALIGNING;
            state->targetYaw = 0;
        }
        break;

    case LANDING:
        // done landing...
//        if (quadEncoderIsCalibrated()) {
//            state->targetYaw = 0;
//        }
        if (yawGetDegrees(1) > 0 && abs(state->targetYaw) % 360 != 0) {
            state->targetYaw -= 1;
        } else if (yawGetDegrees(1) < 0 && abs(state->targetYaw) % 360 != 0) {
            state->targetYaw += 1;
        }
        controlSetTarget(state->targetHeight, CONTROL_HEIGHT);
        controlSetTarget(state->targetYaw, CONTROL_YAW);

        if (abs(yawGetDegrees(1) - state->targetYaw) <= 1 && heightAsPercentage(1) <= 1) {
            stabilityCounter++;
        } else {
            stabilityCounter = 0;
        }
        // STABILITY_TIME_MAIN is the number of milliseconds to wait for stability.
        if (stabilityCounter >= STABILITY_TIME_MAIN / deltaTime) {
            if (controlLandingStability()) {
                controlMotorSet(false, MAIN_ROTOR);
                controlMotorSet(false, TAIL_ROTOR);
                state->heliMode = LANDED;
                ignoreButton(SW1);
                controlDisable(CONTROL_YAW);
                controlSetLandingSequence(false);
                state->targetHeight = 0;
                yawClipTo360Degrees();
            }
        }
        break;

    case FLYING:
        if (checkButton(UP) == PUSHED && state->targetHeight < MAX_DUTY) {
            state->targetHeight += MAIN_STEP;
        }
        if (checkButton(DOWN) == PUSHED && state->targetHeight > MIN_DUTY) {
            state->targetHeight -= MAIN_STEP;
        }
        if (checkButton(LEFT) == PUSHED) {
            state->targetYaw -= TAIL_STEP;
        }
        if (checkButton(RIGHT) == PUSHED) {
            state->targetYaw += TAIL_STEP;
        }
        if (checkButton(SW1) == RELEASED) {  // switch down
            controlSetLandingSequence(true);
            state->targetHeight = 0;
            state->heliMode = LANDING;
        }
        controlSetTarget(state->targetHeight, CONTROL_HEIGHT);
        controlSetTarget(state->targetYaw, CONTROL_YAW);
        break;
    }
}

// TODO: remove
#define UPDATE_COUNT (TASK_BASE_FREQ / UART_DISPLAY_FREQUENCY)
void displayUpdate(state_t* state, uint32_t deltaTime)
{
    static int uartCount = 0;
    static const char* heliStateWordMap[] = {
       "Landed", "Landing", "Aligning", "Flying", "Calibrate Yaw"
    };

    // Take measurements
    uint32_t percentageHeight = heightAsPercentage(1);  // precision = 1
    uint32_t degreesYaw = yawGetDegrees(1);  // precision = 1
    uint32_t mainDuty = controlGetPWMDuty(CONTROL_HEIGHT);
    uint32_t tailDuty = controlGetPWMDuty(CONTROL_YAW);

    // Update OLED display
    displayPrintLineWithFormat("Height = %4d%%", 1, percentageHeight);  // line 1
    displayPrintLineWithFormat("M = %2d, T = %2d", 2, mainDuty, tailDuty);  // line 2

    // Update UART display
    // Use a collaborative technique to update the display across updates

    switch (uartCount) {
    case UPDATE_COUNT - 5:
        UARTPrintLineWithFormat("\nALT %d [%d] %%\n", state->targetHeight, percentageHeight);
        break;
    case UPDATE_COUNT - 4:
        UARTPrintLineWithFormat("YAW %d [%d] deg\n", state->targetYaw, degreesYaw);
        break;
    case UPDATE_COUNT - 3:
        UARTPrintLineWithFormat("MAIN %d %%, TAIL %d %%\n", mainDuty, tailDuty);
        break;
    case UPDATE_COUNT - 2:
        UARTPrintLineWithFormat("MODE %s\n", heliStateWordMap[state->heliMode]);
        break;
    case UPDATE_COUNT - 1:
        UARTPrintLineWithFormat("%s", "----------------\n");
        break;
    case UPDATE_COUNT:
        uartCount = 0;
    }
    uartCount++;
}

void runTasks(task_t* tasks, state_t* sharedState, int32_t baseFreq)
{
    // initalise the value to count up to for each task so that
    // tasks can run at different frequencies
    int32_t deltaTime = 1000 / baseFreq;  // in milliseconds, hence the 1000 factor
    int i = 0;
    while (tasks[i].handler) {
        uint32_t triggerCount = baseFreq / tasks[i].updateFreq;
        if (triggerCount == 0) {
            triggerCount = 1;
        }

        tasks[i].count = 0;
        tasks[i].triggerAt = triggerCount;  // make sure not zero
        i++;
    }

    // begin the main loop
    while (true) {
        int32_t referenceTime = timererGetTicks();

        int i = 0;
        while (tasks[i].handler) {
            tasks[i].count++;

            // check if task should run in this update
            if (tasks[i].count == tasks[i].triggerAt) {
                tasks[i].count = 0;

                // run the task
                tasks[i].handler(sharedState, deltaTime);
            }
              i++;
        }

        // make sure loop runs as a consistent speed
        timererWaitFrom(deltaTime, referenceTime);
    }
}

void controllerUpdate(state_t* state, uint32_t deltaTime)
{
    heightUpdate();
    controlUpdate(deltaTime);
}

void stateUpdate(state_t* state, uint32_t deltaTime)
{
    updateButtons();
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
        {0}  // terminator
    };

    // any data which many tasks might need to know about
    state_t sharedState = {
        .heliMode = LANDED,
        .targetHeight = 0,
        .targetYaw = 0
    };

    runTasks(tasks, &sharedState, TASK_BASE_FREQ);
}

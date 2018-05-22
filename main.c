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

enum heli_state {LANDED = 0, LANDING, ALIGNING, FLYING, CALIBRATE_YAW, NUM_HELI_STATES};
// list the mode that should be displayed for each state.
static const char* heli_state_map [] = {"Landed", "Landing", "Aligning", "Flying", "Calibrate Yaw"};
static enum heli_state current_heli_state = LANDED;

int32_t targetHeight = 0;
int32_t targetYaw = 0; // should this be an int?

#define DELTA_TIME 10  // 100 hz, 10 ms
#define UART_DISPLAY_FREQUENCY 4  // hz
#define LANDING_UPDATE_FREQUENCY 10 // hz
#define LOOP_FREQUENCY (1000 / DELTA_TIME)
#define UPDATE_COUNT (LOOP_FREQUENCY / UART_DISPLAY_FREQUENCY)
#define HEIGHT_LANDING_COUNT (LOOP_FREQUENCY / LANDING_UPDATE_FREQUENCY)
#define STABILITY_TIME_MAIN 500 // 500 ms
#define STABILITY_TIME_TAIL 2000 // 2000 ms

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
    // TODO: reset peripherals

    // Set system clock rate to 20 MHz.
    SysCtlClockSet(SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ | SYSCTL_SYSDIV_10);
    timererInit();
    timererWait(1);  // Allow time for the oscillator to settle down.

    initButtons();
    initSoftReset();
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
    static int stabilityCounter;
    static bool shouldCalibrate = true;
    switch (current_heli_state) {

    case LANDED:
        heightCalibrate();
        if (checkButton(SW1) == PUSHED) {
            if (shouldCalibrate) {
                current_heli_state = CALIBRATE_YAW;
                quadEncoderCalibrate();
            } else {
                current_heli_state = ALIGNING;
            }
            controlMotorSet(true, MAIN_ROTOR);  // turn  on motors
            controlMotorSet(true, TAIL_ROTOR);
              // start calibration
            controlEnable(CONTROL_CALIBRATE_MAIN);
            controlEnable(CONTROL_CALIBRATE_TAIL);
            controlEnable(CONTROL_HEIGHT);
            controlEnable(CONTROL_YAW);
            targetHeight = 0;
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
            current_heli_state = FLYING;
            targetYaw = 0;

        }
        break;

    case CALIBRATE_YAW:
        //Find the zero point for the yaw
        targetYaw += 1;
        controlSetTarget(targetYaw, CONTROL_YAW);
        controlSetTarget(targetHeight, CONTROL_HEIGHT);
        if (quadEncoderIsCalibrated()) {
            shouldCalibrate = false;
            current_heli_state = ALIGNING;
            targetYaw = 0;
        }
        break;

    case LANDING:
        // TODO: Ramp input for landing
        // done landing...

        if (yawGetDegrees(1) <= 1 && heightAsPercentage(1) <= 1) {
            stabilityCounter++;
        } else {
            stabilityCounter = 0;
        }
        // STABILITY_TIME_MAIN is the number of milliseconds to wait for stability.
        if (stabilityCounter >= STABILITY_TIME_MAIN / DELTA_TIME) {
            if (controlLandingStability()) {;
                controlMotorSet(false, MAIN_ROTOR);
                controlMotorSet(false, TAIL_ROTOR);
                current_heli_state = LANDED;
                ignoreButton(SW1);
                controlDisable(CONTROL_YAW);
                controlSetLandingSequence(false);
                targetHeight = 0;
            }
        }
        break;

    case FLYING:
        if (checkButton(UP) == PUSHED && targetHeight < MAX_DUTY) {
            targetHeight += MAIN_STEP;
        }
        if (checkButton(DOWN) == PUSHED && targetHeight > MIN_DUTY) {
            targetHeight -= MAIN_STEP;
        }
        if (checkButton(LEFT) == PUSHED) {
            targetYaw -= TAIL_STEP;
        }
        if (checkButton(RIGHT) == PUSHED) {
            targetYaw += TAIL_STEP;
        }
        if (checkButton(SW1) == RELEASED) {  // switch down
            // TODO: add landing control
//            controlSetLandingSequence(true);
            targetYaw = 0;
            targetHeight = 0;
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
    displayPrintLineWithFormat("Height = %4d%%", 1, percentageHeight);  // line 1
    displayPrintLineWithFormat("M = %2d, T = %2d", 2, mainDuty, tailDuty);  // line 2

    // Update UART display
    // Use a collaborative technique to update the display across updates

    switch (uartCount) {
    case UPDATE_COUNT - 5:
        UARTPrintLineWithFormat("\nALT %d [%d] %%\n", targetHeight, percentageHeight);
        break;
    case UPDATE_COUNT - 4:
        UARTPrintLineWithFormat("YAW %d [%d] deg\n", targetYaw, degreesYaw);
        break;
    case UPDATE_COUNT - 3:
        UARTPrintLineWithFormat("MAIN %d %%, TAIL %d %%\n", mainDuty, tailDuty);
        break;
    case UPDATE_COUNT - 2:
        UARTPrintLineWithFormat("MODE %s\n", heli_state_map[current_heli_state]);
        break;
    case UPDATE_COUNT - 1:
        UARTPrintLineWithFormat("%s", "----------------\n");
        break;
    case UPDATE_COUNT:
        uartCount = 0;
    }
    uartCount++;
}


//typedef struct {
//    void (*handler) (state_t state, uint32_t deltaTime);  // pointer to task handler function
//    uint32_t period;  // number of ms between runs
//    uint32_t referenceTime = 0;
//} task_t;
//
//
//#define NUM_TASKS 5
//task_t tasks[NUM_TASKS] = {
//    {heightUpdate, 100},  // for height smoothing
//    {controlUpdate, 100},
//    {displayInfo, 25},
//    {updateButtons, 100},
//    {heliMode, 100}
//};
//
//
//void runTasks(task_t* tasks) {
////static const ticks_per_ms = ;
//
//    uint32_t initReference = timererGetTicks();
//    int i = 0;
//    for (; i < NUM_TASKS; i++) {
//        tasks[i].referenceTime = initReference;
//    }
//
//    while (true) {
//        task_t* nextTask = 0;
//        uint32_t largestOvershoot = 0;
//
//        uint32_t cur = timererGetTicks(); //get time, which is also the reference
//
//        int i = 0;
//        for (; i < NUM_TASKS; i++) {
//            // minus since counts down
//            uint32_t target = tasks[i].referenceTime - tasks[i].period * ticks_per_ms;
//            uint32_t diff = target - cur;  // +ve small number when past target (timer counts down)
//
//            // find the next task which has overshot and needs to be most urgently addressed.
//            if (diff < overshoot_ticks && diff > largestOvershoot) {
//                largestOvershoot = diff;
//                nextTask = &tasks[i];
//            }
//        }
//
//
//    }
//}

// TODO: round robin?


int main(void)
{
    initalise();

    timererWait(1000 * CONV_SIZE / ADC_SAMPLE_RATE);  // make sure ADC buffer has a chance to fill up

//	// main loop
//	while (true) {
//	    // don't include the time to execute the main loop in our time measurement
//	    // so measure DELTA_TIME from this point
//	    uint32_t referenceTime = timererGetTicks();
//
//	    heightUpdate();  // do convolution step
//	    controlUpdate(DELTA_TIME);  // update control
//
//	    displayInfo();
//
//	    // Update user inputs and run state machine
//        updateButtons();  // recommended 100 hz update
//        heliMode();
//
//	    // Wait any time that remains for this cycle to take DELTA_TIME
//	    timererWaitFrom(DELTA_TIME, referenceTime);
//	}
}


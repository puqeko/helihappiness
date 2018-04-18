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
#include "utils/ustdlib.h"
#include "circBufT.h"
#include "yaw.h"
#include "height.h"
#include "timerer.h"
#include "OrbitOLED_2/OrbitOLEDInterface.h"
#include "pwmModule.h"

#define GREEN_LED GPIO_PIN_3
#define DISPLAY_CHAR_WIDTH 16

enum heli_state {LANDED = 0, FLYING, NUM_HELI_STATES};
enum display_state {PERCENTAGE = 0, MEAN_ADC, DISPLAY_OFF, NUM_DISPLAY_STATES};
static uint8_t current_heli_state = LANDED;
static uint8_t current_display_state = PERCENTAGE;

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
    OLEDInitialise();
    heightInit(CONV_UNIFORM);
    yawInit();
    initClocks ();
    initialisePWM ();

    // Enable GPIO Port F
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    IntMasterEnable();

    // Set up the specific port pin as medium strength current & pull-down config.
    // Refer to TivaWare peripheral lib user manual for set up for configuration options
    GPIOPadConfigSet(GPIO_PORTF_BASE, GREEN_LED, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPD);

    // Set data direction register as output
    GPIODirModeSet(GPIO_PORTF_BASE, GREEN_LED, GPIO_DIR_MODE_OUT);

    // Write a zero to the output pin 3 on port F
    GPIOPinWrite(GPIO_PORTF_BASE, GREEN_LED, 0x00);
}

void displayValueWithFormat(char* format, uint32_t value, uint32_t line)
{
    char str[17] = "                 ";  // 16 characters across the display
    usnprintf (str, sizeof(str), format, value);
    str[strlen(str)] = ' ';  // overwrite null terminator added by usnprintf
    str[DISPLAY_CHAR_WIDTH] = '\0';  // ensure there is one at the end of the string
    OLEDStringDraw (str, 0, line);
}
void displayTwoValuesWithFormat(char* format, uint32_t value1, uint32_t value2, uint32_t line)
{
    char str[17] = "                 ";  // 16 characters across the display
    usnprintf (str, sizeof(str), format, value1, value2);
    str[strlen(str)] = ' ';  // overwrite null terminator added by usnprintf
    str[DISPLAY_CHAR_WIDTH] = '\0';  // ensure there is one at the end of the string
    OLEDStringDraw (str, 0, line);
}


void displayClear(uint32_t line)
{
    OLEDStringDraw ("                 ", 0, line);  // 16 characters across the display
}


void displayMode(void)
{
    switch (current_display_state)
    {
    case MEAN_ADC:
        displayValueWithFormat(meanFormatString, heightGetRaw(), 1);
        break;

    case PERCENTAGE:
        // this is okay because the mean is capped to 4095
        displayValueWithFormat(percentFormatString, heightAsPercentage(), 1);
        break;

    case DISPLAY_OFF:
        displayClear(1);
        break;
    }
}


void heliMode(void)
{
    switch (current_heli_state) {

    // M1.3 Measure the mean sample value for a bit and display on the screen
    case LANDED:
        displayValueWithFormat(percentFormatString, 0, 1);  // clear to zero

        GPIOPinWrite(GPIO_PORTF_BASE,  GREEN_LED, GREEN_LED);
        timererWait(1000 * CONV_SIZE / ADC_SAMPLE_RATE);
        heightCalibrate();

        current_heli_state = FLYING;
        break;  // measure 0% height value

    // M1.4 Display altitude
    case FLYING:
        if (checkButton(UP) == PUSHED && ui32DutyMain < PWM_DUTY_MAX_HZ)
        {
            ui32DutyMain += PWM_DUTY_STEP_HZ;
            pwmSetDuty(ui32DutyMain, MAIN_ROTOR);
        }
        if (checkButton(DOWN) == PUSHED && ui32DutyMain > PWM_DUTY_MIN_HZ)
        {
            ui32DutyMain -= PWM_DUTY_STEP_HZ;
            pwmSetDuty(ui32DutyMain, MAIN_ROTOR);
        }
        if (checkButton(LEFT) == PUSHED && ui32DutyTail > PWM_DUTY_MIN_HZ)
        {
            ui32DutyTail -= PWM_DUTY_STEP_HZ;
            pwmSetDuty(ui32DutyTail, TAIL_ROTOR);
        }
        if (checkButton(RIGHT) == PUSHED && ui32DutyTail < PWM_DUTY_MAX_HZ)
        {
            ui32DutyTail += PWM_DUTY_STEP_HZ;
            pwmSetDuty(ui32DutyTail, TAIL_ROTOR);
        }
        displayTwoValuesWithFormat(dutyCycleFormatString, ui32DutyMain, ui32DutyTail, 3);

        // Run M1.3 again (M1.5)
//        if (checkButton(LEFT) == PUSHED) {
//            current_heli_state = LANDED;
//            current_display_state = PERCENTAGE;
//        }

        // Transition between display modes (M1.6)
//        if (checkButton(UP) == PUSHED) {
//            current_display_state += 1;
//            current_display_state %= NUM_DISPLAY_STATES;
//        }
//
        GPIOPinWrite(GPIO_PORTF_BASE,  GREEN_LED, 0x00);
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

	// main loop
	while (true) {
	    uint32_t referenceTime = timererGetTicks();

	    updateButtons();  // recommended 100 hz update
	    heliMode();
	    displayMode();
	    displayValueWithFormat(yawFormatString, yawGetDegrees(), 2);  // line 2

	    timererWaitFrom(10, referenceTime);  // 100 hz, 10 ms
	}
}


// ************************************************************
// display.c
// Helicopter project
// Group: A03 Group 10
// Last edited: 20-04-2018
//
// Purpose: Interface with OLED display
// ************************************************************

#include <stdlib.h>

#include "display.h"
#include "utils/ustdlib.h"
#include "OrbitOLED_2/OrbitOLEDInterface.h"


void displayInit(void)
{
    OLEDInitialise();
}


void displayPrintLineWithFormat(const char* format, int lineNum, ...)
{
    va_list args;
    int storedChars;
    char str[DISPLAY_CHAR_WIDTH + 1];  // for \0

    va_start(args, lineNum);
    storedChars = uvsnprintf(str, DISPLAY_CHAR_WIDTH, format, args);
    va_end(arg);

    // add ellipsis if string too long
    if (storedChars >= DISPLAY_CHAR_WIDTH) {
        int i = DISPLAY_CHAR_WIDTH;
        while (i-- > DISPLAY_CHAR_WIDTH - 3) {
            str[i] = '.';
        }
        str[DISPLAY_CHAR_WIDTH] = '\0';
    } else {
        str[storedChars + 1] = '\0';
    }
    OLEDStringDraw (str, 0, lineNum);
}


// ************************************************************
// Place blank spaces on the display at the given line.
void displayClear(uint32_t line)
{
    OLEDStringDraw ("                 ", 0, line);  // 16 characters across the display
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

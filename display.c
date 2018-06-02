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

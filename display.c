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


//// ************************************************************
//// Display a single value on a line of the OLED display according to a format
//// string provided.
//void displayValueWithFormat(char* format, uint32_t value, uint32_t line)
//{
//    char str[17] = "                 ";  // 16 characters across the display
//    usnprintf (str, sizeof(str), format, value);
//    str[strlen(str)] = ' ';  // overwrite null terminator added by usnprintf
//    str[DISPLAY_CHAR_WIDTH] = '\0';  // ensure there is one at the end of the string
//    OLEDStringDraw (str, 0, line);
//}


//// ************************************************************
//// Display a two values on a line of the OLED display according to a format
//// string provided.
//void displayTwoValuesWithFormat(char* format, uint32_t value1, uint32_t value2, uint32_t line)
//{
//    char str[17] = "                 ";  // 16 characters across the display
//    usnprintf (str, sizeof(str), format, value1, value2);
//    str[strlen(str)] = ' ';  // overwrite null terminator added by usnprintf
//    str[DISPLAY_CHAR_WIDTH] = '\0';  // ensure there is one at the end of the string
//    OLEDStringDraw (str, 0, line);
//}


void displayPrintLineWithFormat(const char* format, int lineNum, ...)
{
    va_list args;
    int storedChars;
    char str[DISPLAY_CHAR_WIDTH + 1];  // for \0

    va_start(args, format);
    storedChars = uvsnprintf(str, DISPLAY_CHAR_WIDTH, format, args);
    va_end(arg);

    // add ellipsis if string too long
    if (storedChars >= DISPLAY_CHAR_WIDTH) {
        int i = DISPLAY_CHAR_WIDTH;
        while (i-- > DISPLAY_CHAR_WIDTH - 3) {
            str[i] = '.';
        }
        str[DISPLAY_CHAR_WIDTH + 1] = '\0';
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

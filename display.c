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


// ************************************************************
// Display a single value on a line of the OLED display according to a format
// string provided.
void displayValueWithFormat(char* format, uint32_t value, uint32_t line)
{
    char str[17] = "                 ";  // 16 characters across the display
    usnprintf (str, sizeof(str), format, value);
    str[strlen(str)] = ' ';  // overwrite null terminator added by usnprintf
    str[DISPLAY_CHAR_WIDTH] = '\0';  // ensure there is one at the end of the string
    OLEDStringDraw (str, 0, line);
}


// ************************************************************
// Display a two values on a line of the OLED display according to a format
// string provided.
void displayTwoValuesWithFormat(char* format, uint32_t value1, uint32_t value2, uint32_t line)
{
    char str[17] = "                 ";  // 16 characters across the display
    usnprintf (str, sizeof(str), format, value1, value2);
    str[strlen(str)] = ' ';  // overwrite null terminator added by usnprintf
    str[DISPLAY_CHAR_WIDTH] = '\0';  // ensure there is one at the end of the string
    OLEDStringDraw (str, 0, line);
}


// ************************************************************
// Place blank spaces on the display at the given line.
void displayClear(uint32_t line)
{
    OLEDStringDraw ("                 ", 0, line);  // 16 characters across the display
}

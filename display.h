#ifndef DISPLAY_H_
#define DISPLAY_H_

// ************************************************************
// display.h
// Helicopter project
// Group: A03 Group 10
// Last edited: 20-04-2018
//
// Purpose: Interface with OLED display
// ************************************************************

#include <stdint.h>


// ************************************************************
// Display a single value on a line of the OLED display according to a format
// string provided.
void displayValueWithFormat(char* format, uint32_t value, uint32_t line)


// ************************************************************
// Display a two values on a line of the OLED display according to a format
// string provided.
void displayTwoValuesWithFormat(char* format, uint32_t value1, uint32_t value2, uint32_t line)


// ************************************************************
// Place blank spaces on the display at the given line.
void displayClear(uint32_t line)

#endif /* DISPLAY_H_ */

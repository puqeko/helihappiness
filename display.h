// ************************************************************
// display.h
// Helicopter project
// Group: A03 Group 10
// Last edited: 20-04-2018
//
// Purpose: Format strings for printing to the tiva OLED display
// using the OLED module.
// ************************************************************

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <stdint.h>
#include <string.h>

// The number of chars which fit on a line of the display
#define DISPLAY_CHAR_WIDTH 16


// Configure OLED display
void displayInit(void);


// Format a line for printing to the lineNum'th line of the display.
// If the line is too long, the string is truncated and an elipsis is
// added to the end. All other behaviour is as per printf.
void displayPrintLineWithFormat(const char* format, uint32_t lineNum, ...);


// Place blank spaces on the display at the given line.
void displayClear(uint32_t line);

#endif /* DISPLAY_H_ */

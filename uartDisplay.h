// ************************************************************
// uartDisplay.h
// Helicopter project
// Group: A03 Group 10
// Based on code from P.J. Bones 21.04.2018
// Last edited: 30-05-2018
//
// Purpose: Write string output to USB using UART.
// ************************************************************

#ifndef UARTDISPLAY_H_
#define UARTDISPLAY_H_

#include <stdint.h>
#include <stdbool.h>

#include "utils/ustdlib.h"
#include "driverlib/gpio.h"       // defines for GPIO peripheral
#include "driverlib/sysctl.h"     // system control functions
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"

#define UART_LINE_LENGTH 25  // where should a line be truncated

// Configure the UART with 8 bits, 1 stop bit, no parity
void uartInit (void);


// Transmit a string via UART0
// The FIFO is 16 bytes so only send a 16 byte string for optimal performance
void uartSend (char *pucBuffer);


// Print a single formated line to the terminal and truncate if too long.
// Takes format with arguments (like fprintf) but adds a newline to the end
// and truncates to UART_LINE_LENGTH. An ellipsis is added to the end if the
// line is too long. A format string is always required.
void uartPrintLineWithFormat(const char* format, ...);

#endif /* UARTDISPLAY_H_ */

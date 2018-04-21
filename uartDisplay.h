#ifndef UARTDISPLAY_H_
#define UARTDISPLAY_H_

// *******************************************************
// uartDisplay.h
//
// Generates serial data to print for debugging
// P.J. Bones UCECE, modified by Ryan H and Thomas M
// Last modified:  21.04.2017
// Helicopter project
// Group: A03 Group 10
// *******************************************************

#include <stdint.h>
#include <stdbool.h>

#include "utils/ustdlib.h"
#include "driverlib/gpio.h"       // defines for GPIO peripheral
#include "driverlib/sysctl.h"     // system control functions
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"

#define UART_LINE_LENGTH 25

// *******************************************************
void initialiseUSB_UART (void);

void UARTSend (char *pucBuffer);

void UARTPrintLineWithFormat(const char* format, ...);

#endif /* UARTDISPLAY_H_ */

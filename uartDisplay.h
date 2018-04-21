#ifndef UARTDISPLAY_H_
#define UARTDISPLAY_H_

// *******************************************************
// uartDisplay.h
//
// Generates serial data to print for debugging
// P.J. Bones UCECE, modified by Ryan hall
// Last modified:  18.04.2017
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
void
initialiseUSB_UART (void);

void
UARTSend (char *pucBuffer);

//void
//UARTPrint (int32_t yawTarget, int32_t yawActual, int32_t heightTarget, int32_t heightActual, uint32_t dutyMain, uint32_t dutyTail,  char * mode);

void UARTPrintfln(const char* format, ...);
//void UARTPrintValuesWithFormat(char* format, int32_t n, int32_t m);

#endif /* UARTDISPLAY_H_ */

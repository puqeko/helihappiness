// ************************************************************
// uartDisplay.c
// Helicopter project
// Group: A03 Group 10
// Based on code from P.J. Bones 21.04.2018
// Last edited: 30-05-2018
//
// Purpose: Write string output to USB using UART.
// ************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/debug.h"
#include "stdio.h"
#include "stdlib.h"
#include "utils/ustdlib.h"
#include "uartDisplay.h"


// USB Serial comms: UART0, Rx:PA0 , Tx:PA1
#define BAUD_RATE 9600
#define UART_USB_BASE           UART0_BASE
#define UART_USB_PERIPH_UART    SYSCTL_PERIPH_UART0
#define UART_USB_PERIPH_GPIO    SYSCTL_PERIPH_GPIOA
#define UART_USB_GPIO_BASE      GPIO_PORTA_BASE
#define UART_USB_GPIO_PIN_RX    GPIO_PIN_0
#define UART_USB_GPIO_PIN_TX    GPIO_PIN_1
#define UART_USB_GPIO_PINS      UART_USB_GPIO_PIN_RX | UART_USB_GPIO_PIN_TX


// Configure the UART with 8 bits, 1 stop bit, no parity
void uartInit (void)
{
    // reset for good measure
    SysCtlPeripheralReset(UART_USB_PERIPH_UART);

    // Enable GPIO port A which is used for UART0 pins.
    SysCtlPeripheralEnable(UART_USB_PERIPH_UART);
    SysCtlPeripheralEnable(UART_USB_PERIPH_GPIO);

    // Select the alternate (UART) function for these pins.
    GPIOPinTypeUART(UART_USB_GPIO_BASE, UART_USB_GPIO_PINS);
    UARTConfigSetExpClk(UART_USB_BASE, SysCtlClockGet(), BAUD_RATE,
                        UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                        UART_CONFIG_PAR_NONE);
    UARTFIFOEnable(UART_USB_BASE);  // use a queue to buffer the sending data
    UARTEnable(UART_USB_BASE);

    uartSend("\n");  // required to start printing to remote interface
}


// Transmit a string via UART0
// The FIFO is 16 bytes so only send a 16 byte string for optimal performance
void uartSend (char *pucBuffer)
{
    // Loop while there are more characters to send.
    while(*pucBuffer)
    {
        // Write the next character to the UART Tx FIFO.
        UARTCharPut(UART0_BASE, *pucBuffer);
        pucBuffer++;
    }
}


// Print a single formated line to the terminal and truncate if too long.
// Takes format with arguments (like fprintf) but adds a newline to the end
// and truncates to UART_LINE_LENGTH. An ellipsis is added to the end if the
// line is too long. A format string is always required.
void uartPrintLineWithFormat(const char* format, ...)
{
    va_list args;
    int storedChars;
    char uartString[UART_LINE_LENGTH + 2];  // +2 for \r and \0

    // replace using arguments
    va_start(args, format);
    storedChars = uvsnprintf(uartString, UART_LINE_LENGTH, format, args);
    va_end(arg);

    // add ellipsis if string too long
    if (storedChars >= UART_LINE_LENGTH) {
        // truncate
        int i = UART_LINE_LENGTH;
        while (i-- > UART_LINE_LENGTH - 3) {
            uartString[i] = '.';
        }
        uartString[UART_LINE_LENGTH] = '\r';
        uartString[UART_LINE_LENGTH + 1] = '\0';
    } else {
        // add new line only
        uartString[storedChars] = '\r';
        uartString[storedChars + 1] = '\0';
    }

    uartSend(uartString);
}

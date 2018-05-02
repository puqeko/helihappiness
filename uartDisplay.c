//********************************************************
//
// uartDemo.c - Example code for ENCE361
//
// Link with modules:  buttons2, OrbitOLEDInterface
//
// Author:  P.J. Bones  UCECE edited by Ryan H and Thomas M
// Last modified:   21.04.2018
//

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

//********************************************************
// Constants
//********************************************************

//---USB Serial comms: UART0, Rx:PA0 , Tx:PA1
#define BAUD_RATE 9600
#define UART_USB_BASE           UART0_BASE
#define UART_USB_PERIPH_UART    SYSCTL_PERIPH_UART0
#define UART_USB_PERIPH_GPIO    SYSCTL_PERIPH_GPIOA
#define UART_USB_GPIO_BASE      GPIO_PORTA_BASE
#define UART_USB_GPIO_PIN_RX    GPIO_PIN_0
#define UART_USB_GPIO_PIN_TX    GPIO_PIN_1
#define UART_USB_GPIO_PINS      UART_USB_GPIO_PIN_RX | UART_USB_GPIO_PIN_TX


//********************************************************
// initialiseUSB_UART - 8 bits, 1 stop bit, no parity
//********************************************************
void initialiseUSB_UART (void)
{
    // Enable GPIO port A which is used for UART0 pins.
    //
    SysCtlPeripheralEnable(UART_USB_PERIPH_UART);
    SysCtlPeripheralEnable(UART_USB_PERIPH_GPIO);
    //
    // Select the alternate (UART) function for these pins.
    //
    GPIOPinTypeUART(UART_USB_GPIO_BASE, UART_USB_GPIO_PINS);
    UARTConfigSetExpClk(UART_USB_BASE, SysCtlClockGet(), BAUD_RATE,
                        UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                        UART_CONFIG_PAR_NONE);
    UARTFIFOEnable(UART_USB_BASE);
    UARTEnable(UART_USB_BASE);

    UARTSend("\n");  // required to start printing to remote interface
}


//**********************************************************************
// Transmit a string via UART0
//**********************************************************************
void UARTSend (char *pucBuffer)
{
    // Loop while there are more characters to send.
    while(*pucBuffer)
    {
        // Write the next character to the UART Tx FIFO.
        UARTCharPut(UART0_BASE, *pucBuffer);
        pucBuffer++;
    }
}


//**********************************************************************
// Print a single formated line to the terminal and truncate if too long.
// Takes format with arguments (like fprintf) but adds a newline to the end
// and truncates to UART_LINE_LENGTH. An ellipsis is added to the end if the
// line is too long. A format string is always required.
//**********************************************************************
void UARTPrintLineWithFormat(const char* format, ...)
{
    va_list args;
    int storedChars;
    char uartString[UART_LINE_LENGTH + 2];  // for \n and \0

    va_start(args, format);
    storedChars = uvsnprintf(uartString, UART_LINE_LENGTH, format, args);
    va_end(arg);

    // add ellipsis if string too long
    if (storedChars >= UART_LINE_LENGTH) {
        int i = UART_LINE_LENGTH;
        while (i-- > UART_LINE_LENGTH - 3) {
            uartString[i] = '.';
        }
        uartString[UART_LINE_LENGTH] = '\r';
        uartString[UART_LINE_LENGTH + 1] = '\0';
    } else {
        uartString[storedChars] = '\r';
        uartString[storedChars + 1] = '\0';
    }
    UARTSend(uartString);
}

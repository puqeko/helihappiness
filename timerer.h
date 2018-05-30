// ************************************************************
// timerer.h
// Helicopter project
// Group: A03 Group 10
// Last edited: 14-04-2018 by Thomas M
//
// Purpose: More accurate timer for delays and loop timing.
// ************************************************************

#ifndef TIMERER_H_
#define TIMERER_H_

#include <stdint.h>
#include <stdbool.h>
#include "stdlib.h"

#include "inc/hw_memmap.h"  // for TIMER0_BASE etc
#include "driverlib/sysctl.h"


// enable the hardware timer and calculate clock parameters
void timererInit(void);


// return the current timer value in tick s.
uint32_t timererGetTicks(void);


// waits for some given milliseconds.
void timererWait(uint32_t milliseconds);


// returns true after milliseconds have passed from reference
bool timererBeen(uint32_t milliseconds, uint32_t reference);


// waits until a given milliseconds passed some reference timer value.
// useful for keeping time in a loop with many operations.
void timererWaitFrom(uint32_t milliseconds, uint32_t reference);

#endif /* TIMERER_H_ */

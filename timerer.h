#ifndef TIMERER_H_
#define TIMERER_H_

// ************************************************************
// timerer.h
// Helicopter project
// Group: A03 Group 10
// Last edited: 14-04-2018 by Thomas M
//
// Purpose: More accurate timer for delays and loop timing.
// ************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "stdlib.h"

#include "inc/hw_memmap.h"  // for TIMER0_BASE etc
#include "driverlib/sysctl.h"

void TimererInit(void);

// returns the current value of the timer.
uint32_t TimererGetTicks(void);

// Waits for some given milliseconds.
void TimererWait(uint32_t milliseconds);

// Waits until a given milliseconds passed some reference timer value.
// Useful for keeping time in a loop with many operations.
void TimererWaitFrom(uint32_t milliseconds, uint32_t reference);

#endif /* TIMERER_H_ */

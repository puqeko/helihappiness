// *******************************************************
//
// timer.h
//
// More accurate timer for delays
// T Morrison
// Last modified:  14.04.18
//
// *******************************************************

#ifndef TIMERER_H_
#define TIMERER_H_

#include <stdint.h>
#include <stdbool.h>
#include "stdlib.h"

#include "inc/hw_memmap.h"  // for TIMER0_BASE etc
#include "driverlib/sysctl.h"

#define TIMERER_PERIPH SYSCTL_PERIPH_TIMER0
#define TIMERER_BASE TIMER0_BASE
#define TIMERER_INTERAL TIMER_A
static const uint32_t TIMERER_MAX_TICKS = (0xffffffff - 1);  // 32 bits for Timer0 A

void TimererInit(void);

// returns the current value of the timer.
uint32_t TimererGetTicks(void);

// Waits for some given milliseconds.
void TimererWait(uint32_t milliseconds);

// Waits until a given milliseconds passed some reference timer value.
// Useful for keeping time in a loop with many operations.
void TimererWaitFrom(uint32_t milliseconds, uint32_t reference);

#endif /* TIMERER_H_ */

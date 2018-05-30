// ************************************************************
// timerer.c
// Helicopter project
// Group: A03 Group 10
// Last edited: 30-05-2018 by Thomas M
//
// Purpose: More accurate timer for delays and loop timing using a
// 32-bit down counter and timer 5.
// ************************************************************


#include "timerer.h"
#include "driverlib/timer.h"

#define TIMERER_PERIPH SYSCTL_PERIPH_WTIMER5
#define TIMERER_BASE WTIMER5_BASE
#define TIMERER_INTERAL TIMER_A
#define TIMERER_MODE TIMER_CFG_A_PERIODIC
#define TIMERER_MAX_TICKS INT32_MAX  // 32 bits for Timer0 A
#define TIMERER_OVERSHOOT_TICKS (TIMERER_MAX_TICKS / 2)

static uint32_t clockRate;
static uint32_t ticksPerMs;


// enable the hardware timer and calculate clock parameters
void timererInit(void)
{
    clockRate = SysCtlClockGet();
    ticksPerMs = clockRate / 1000;  // 1000 ms = 1 s

    // timer counts down by default
    // config to reset to max value
    SysCtlPeripheralEnable(TIMERER_PERIPH);
    TimerDisable(TIMERER_BASE, TIMERER_INTERAL);
    TimerConfigure(TIMERER_BASE, TIMERER_MODE);
    TimerLoadSet(TIMERER_BASE, TIMERER_INTERAL, TIMERER_MAX_TICKS);
    TimerEnable(TIMERER_BASE, TIMERER_INTERAL);
}


// return the current timer value in ticks.
uint32_t timererGetTicks(void)
{
    return TimerValueGet(TIMERER_BASE, TIMERER_INTERAL);
}


// waits for some given milliseconds.
void timererWait(uint32_t milliseconds)
{
    timererWaitFrom(milliseconds, timererGetTicks());
}


// returns true after milliseconds have passed from reference.
bool timererBeen(uint32_t milliseconds, uint32_t reference)
{
    // minus since counts down
    uint32_t target = reference - milliseconds * ticksPerMs;
    uint32_t cur = timererGetTicks(); //get time
    uint32_t diff = target - cur;  // +ve small number when past target (timer counts down)

    // false until this condition is met
    return diff < TIMERER_OVERSHOOT_TICKS;
}


// waits until a given milliseconds passed some reference timer value.
// useful for keeping time in a loop with many operations.
void timererWaitFrom(uint32_t milliseconds, uint32_t reference)
{
    while (true) {

        // block until we pass the target time
        if (timererBeen(milliseconds, reference)) return;
    }
}

// *******************************************************
//
// timer.c
//
// More accurate timer for delays
// T Morrison
// Last modified:  14.04.18
//
// *******************************************************

#include "timerer.h"
#include "driverlib/timer.h"

static uint32_t clock_rate;
static uint32_t overshoot_ticks;
static uint32_t ticks_per_ms;

#ifdef DEBUG_MODE
static uint32_t max_ms;
#endif

// *******************************************************
void TimererInit()
{
    clock_rate = SysCtlClockGet();
    ticks_per_ms = clock_rate / 1000;
    overshoot_ticks = ticks_per_ms;

    // timer counts down by default
    // config to reset to max value
    SysCtlPeripheralEnable(TIMERER_PERIPH);
    TimerDisable(TIMERER_BASE, TIMERER_INTERAL);
    TimerConfigure(TIMERER_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMERER_BASE, TIMERER_INTERAL, TIMERER_MAX_TICKS);
    TimerEnable(TIMERER_BASE, TIMERER_INTERAL);

#ifdef DEBUG_MODE
    max_ms = (TIMERER_MAX_TICKS - overshoot_ticks - 1) / ticks_per_ms;
#endif
}

uint32_t TimererGetTicks(void)
{
    return TimerValueGet(TIMERER_BASE, TIMERER_INTERAL);
}

void TimererWait(uint32_t milliseconds)
{
    TimererWaitFrom(milliseconds, TimererGetTicks());
}

void TimererWaitFrom(uint32_t milliseconds, uint32_t reference)
{
#ifdef DEBUG_MODE
    if (milliseconds > max_ms) {
        exit(1);  // wait time too large for this timer
    }
#endif

    uint32_t target = reference + milliseconds * ticks_per_ms;

    while (true) {
        uint32_t cur = TimererGetTicks(); //get time;
        uint32_t diff = cur - target;  // +ve small number when past target
        if (diff < overshoot_ticks)
            return;  // we have passed the target time
    }
}

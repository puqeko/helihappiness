// ************************************************************
// timerer.c
// Helicopter project
// Group: A03 Group 10
// Last edited: 14-04-2018 by Thomas M
//
// Purpose: More accurate timer for delays and loop timing.
// ************************************************************


#include "timerer.h"
#include "driverlib/timer.h"

#define TIMERER_PERIPH SYSCTL_PERIPH_WTIMER0
#define TIMERER_BASE WTIMER0_BASE
#define TIMERER_INTERAL TIMER_A
#define TIMERER_MODE TIMER_CFG_A_PERIODIC
static const uint32_t TIMERER_MAX_TICKS = (0xffffffff - 1);  // 32 bits for Timer0 A

static uint32_t clock_rate;
static uint32_t overshoot_ticks;
static uint32_t ticks_per_ms;

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
    TimerConfigure(TIMERER_BASE, TIMERER_MODE);
    TimerLoadSet(TIMERER_BASE, TIMERER_INTERAL, TIMERER_MAX_TICKS);
    TimerEnable(TIMERER_BASE, TIMERER_INTERAL);
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
    uint32_t target = reference - milliseconds * ticks_per_ms;  // minus since counts down

    while (true) {
        uint32_t cur = TimererGetTicks(); //get time;
        uint32_t diff = target - cur;  // +ve small number when past target (timer counts down)
        if (diff < overshoot_ticks) {
            return;  // we have passed the target time
        }
    }
}

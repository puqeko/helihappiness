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

#define TIMERER_PERIPH SYSCTL_PERIPH_WTIMER5
#define TIMERER_BASE WTIMER5_BASE
#define TIMERER_INTERAL TIMER_A
#define TIMERER_MODE TIMER_CFG_A_PERIODIC
static const uint32_t TIMERER_MAX_TICKS = (0xffffffff - 1);  // 32 bits for Timer0 A

static uint32_t clock_rate;
static uint32_t overshoot_ticks;
static uint32_t ticks_per_ms;


// *******************************************************
void timererInit(void)
{
    clock_rate = SysCtlClockGet();
    ticks_per_ms = clock_rate / 1000;
    overshoot_ticks = TIMERER_MAX_TICKS / 2;

    // timer counts down by default
    // config to reset to max value
    SysCtlPeripheralEnable(TIMERER_PERIPH);
    TimerDisable(TIMERER_BASE, TIMERER_INTERAL);
    TimerConfigure(TIMERER_BASE, TIMERER_MODE);
    TimerLoadSet(TIMERER_BASE, TIMERER_INTERAL, TIMERER_MAX_TICKS);
    TimerEnable(TIMERER_BASE, TIMERER_INTERAL);
}


uint32_t timererGetTicks(void)
{
    return TimerValueGet(TIMERER_BASE, TIMERER_INTERAL);
}


void timererWait(uint32_t milliseconds)
{
    timererWaitFrom(milliseconds, timererGetTicks());
}


void timererWaitFrom(uint32_t milliseconds, uint32_t reference)
{
    uint32_t target = reference - milliseconds * ticks_per_ms;  // minus since counts down

    while (true) {
        uint32_t cur = timererGetTicks(); //get time
        uint32_t diff = target - cur;  // +ve small number when past target (timer counts down)

        // block until this condition is met
        if (diff < overshoot_ticks) {
            return;  // we have passed the target time
        }
    }
}

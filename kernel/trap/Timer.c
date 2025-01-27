#include <Driver.h>
#include <Timer.h>
#include <Process.h>
#include <Riscv.h>

static u32 ticks;

IntervalTimer timer;

void setNextTimeout() {
    SBI_CALL_1(SBI_SET_TIMER, r_realTime() + INTERVAL);
}

void timerTick() {
    ticks++;
    setNextTimeout();
}

void setTimer(IntervalTimer new) {
    timer = new;
}

IntervalTimer getTimer() {
    return timer;
}
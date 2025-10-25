#include "platform/timer.h"

uint64_t ticks = 0;

void platform_timer_init(unsigned hz)
{
    // Initialize the timer with the given frequency
    uint16_t divisor = 1193180 / hz;
    outb(PIT_COMMAND, 0x34);
    outb(PIT_CHANNEL_0, (uint8_t)(divisor));
    outb(PIT_CHANNEL_0, (uint8_t)(divisor >> 8));
}

uint64_t platform_time_ns(void)
{
    return ticks * (1000000000 / 1000);
}
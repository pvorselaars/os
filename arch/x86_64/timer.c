#include "arch/arch.h"
#include "arch/x86_64/pit.h"

static uint32_t timer_frequency_hz = 0;
static uint64_t timer_ticks = 0;

void timer_handler(void) {
    timer_ticks++;
    if (timer_ticks % timer_frequency_hz == 0) {
        arch_debug_printf("Timer: %lu seconds\n", timer_ticks / timer_frequency_hz);
    }
}

arch_result arch_timer_init(uint32_t frequency_hz)
{
    timer_frequency_hz = frequency_hz;
    x86_64_pit_init(frequency_hz);
    return ARCH_OK;
}

uint64_t arch_time_ns(void) 
{
    // TODO: handle case where timer_frequency_hz is variable
    return (uint64_t)timer_ticks * 1000000ULL / timer_frequency_hz;
}


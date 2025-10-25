#ifndef PLATFORM_TIMER_H
#define PLATFORM_TIMER_H

#include "kernel/definitions.h"
#include "platform/pc/pit.h"
#include "arch/x86_64/io.h"

/* Platform timer interface used by the scheduler and timekeeping code. */

/* Initialize timer subsystem (tick frequency in Hz) */
void platform_timer_init(unsigned hz);

/* Read a monotonic nanosecond-resolution counter (if available) */
uint64_t platform_time_ns(void);

#endif

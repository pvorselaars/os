#ifndef PLATFORM_TIMER_H
#define PLATFORM_TIMER_H

#include "definitions.h"

/* Platform timer interface used by the scheduler and timekeeping code. */

/* Initialize timer subsystem (tick frequency in Hz) */
void platform_timer_init(unsigned hz);

/* Read a monotonic nanosecond-resolution counter (if available) */
uint64_t platform_time_ns(void);

/* Set a one-shot timer to fire after ns nanoseconds (used for sleep/wakeup) */
void platform_timer_set_oneshot(uint64_t ns);

#endif

#ifndef PLATFORM_INIT_H
#define PLATFORM_INIT_H

#include "definitions.h"

/* Platform-level initialization hooks. Implemented by each board/platform. */

/* Called early from kernel() to initialize platform devices (UART, timers).
   This should be safe to call once.
*/
void platform_init(void);

#endif

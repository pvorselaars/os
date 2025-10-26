#ifndef PLATFORM_INIT_H
#define PLATFORM_INIT_H

#include "definitions.h"

/* Platform capability flags */
#define PLATFORM_CAP_TIMER     (1 << 0)
#define PLATFORM_CAP_SERIAL    (1 << 1)
#define PLATFORM_CAP_KEYBOARD  (1 << 2)
#define PLATFORM_CAP_DISPLAY   (1 << 3)

/* Platform information structure */
typedef struct {
    const char *name;           /* Platform name (e.g., "PC-Compatible") */
    const char *arch;           /* Architecture (e.g., "x86_64") */
    uint32_t capabilities;      /* Capability flags */
    uint32_t max_frequency_hz;  /* Maximum timer frequency */
} platform_info_t;

/* Platform initialization API */
void platform_init(void);
void platform_shutdown(void);
void platform_halt(void);
void platform_reboot(void);

/* Platform information */
const platform_info_t *platform_get_info(void);

#endif

#ifndef DRIVERS_PARALLEL_H
#define DRIVERS_PARALLEL_H

#include "arch/arch.h"

/* Initialize parallel port driver and register devices */
arch_result parallel_driver_init(void);

#endif
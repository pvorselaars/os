#ifndef ARCH_X86_64_PROCESS_H
#define ARCH_X86_64_PROCESS_H

/* x86_64-specific process definitions
 * 
 * This file is deprecated - use arch/process.h for the
 * architecture-agnostic interface instead.
 */

#include "arch/process.h"

/* Legacy compatibility - redirect to new interface */
#define process_init() process_subsystem_init()

#endif
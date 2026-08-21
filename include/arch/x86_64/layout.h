#ifndef X86_64_LAYOUT_H
#define X86_64_LAYOUT_H

#define X86_64_KERNEL_BASE 0xFFFFFF8000000000
#define X86_64_KERNEL_STACK (X86_64_KERNEL_BASE + 0x200000 - 1)

#ifndef __ASSEMBLER__
#include "kernel/base.h"

#define x86_64_physical_address(virtual_address) \
    ((uint64_t)(virtual_address) - X86_64_KERNEL_BASE)
#define x86_64_virtual_address(physical_address) \
    ((void *)((uint64_t)(physical_address) + X86_64_KERNEL_BASE))
#endif

#endif

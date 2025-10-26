#ifndef ARCH_TRAP_H
#define ARCH_TRAP_H

#include "definitions.h"

struct fault_info
{
    addr_t fault_va;
    uint64_t error_code;
    addr_t rip;
};

void arch_get_fault_info(struct fault_info *out);

#endif

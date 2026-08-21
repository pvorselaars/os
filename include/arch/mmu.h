#ifndef ARCH_MMU_H
#define ARCH_MMU_H

#include "kernel/base.h"

enum {
    ARCH_MMU_MAP_READ = 1 << 0,
    ARCH_MMU_MAP_WRITE = 1 << 1,
    ARCH_MMU_MAP_USER = 1 << 2,
};

result_t arch_mmu_map_page(uint64_t virtual_address, uint64_t physical_address,
                           uint32_t flags);
result_t arch_mmu_unmap_page(uint64_t virtual_address);

#endif

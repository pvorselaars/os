#ifndef LIB_VMM_H
#define LIB_VMM_H

#include "definitions.h"

typedef enum {
    VMM_MAP_READ = 1 << 0,
    VMM_MAP_WRITE = 1 << 1,
    VMM_MAP_USER = 1 << 2,
} vmm_map_flags_t;

result_t vmm_map_page(uint64_t virtual_address, uint64_t physical_address,
                      vmm_map_flags_t flags);
result_t vmm_unmap_page(uint64_t virtual_address);

#endif

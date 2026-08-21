#include "lib/vmm.h"

#include "arch/mmu.h"

result_t vmm_map_page(uint64_t virtual_address, uint64_t physical_address,
                      vmm_map_flags_t flags)
{
    uint32_t arch_flags = ARCH_MMU_MAP_READ;
    if (flags & VMM_MAP_WRITE) {
        arch_flags |= ARCH_MMU_MAP_WRITE;
    }
    if (flags & VMM_MAP_USER) {
        arch_flags |= ARCH_MMU_MAP_USER;
    }
	return arch_mmu_map_page(virtual_address, physical_address, arch_flags);
}

result_t vmm_unmap_page(uint64_t virtual_address)
{
	return arch_mmu_unmap_page(virtual_address);
}

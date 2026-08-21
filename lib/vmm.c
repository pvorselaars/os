#include "lib/vmm.h"

#include "arch/arch.h"

result_t vmm_map_page(uint64_t virtual_address, uint64_t physical_address, int flags)
{
	return arch_vmm_map_page(virtual_address, physical_address, flags);
}

result_t vmm_unmap_page(uint64_t virtual_address)
{
	return arch_vmm_unmap_page(virtual_address);
}

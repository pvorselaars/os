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

void vmm_map_user_pages(uint64_t page_directory_pointer_table)
{
	arch_vmm_map_user_pages(page_directory_pointer_table);
}

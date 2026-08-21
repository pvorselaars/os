#ifndef LIB_VMM_H
#define LIB_VMM_H

#include "definitions.h"

result_t vmm_map_page(uint64_t virtual_address, uint64_t physical_address, int flags);
result_t vmm_unmap_page(uint64_t virtual_address);
void vmm_map_user_pages(uint64_t page_directory_pointer_table);

#endif

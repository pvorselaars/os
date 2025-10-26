#include "arch/x86_64/memory.h"
#include "arch/x86_64/gdt.h"

extern void flush_tlb(void);

#pragma pack(1)
typedef struct region
{
	uint64_t start;
	uint64_t size;
	uint32_t flags;
} region_t;
#pragma pack()

region_t *memory_map;

static unsigned long total_regions;
static unsigned long total_memory;
static unsigned long total_memory_free;
static unsigned long total_memory_reserved;

pml4e *pml4 = (pml4e *)virtual_address(PML4_ADDRESS);

arch_result arch_memory_map_page(uint64_t va, uint64_t pa, int flags)
{
	assert(IS_ALIGNED(pa, PAGE_SIZE));
	assert(IS_ALIGNED(va, PAGE_SIZE));

	pdpte *pdpt;
	pde *pd;
	pte *pt;

	unsigned short pml4_offset = (va >> 39) & 0x1FF;
	unsigned short pdpt_offset = (va >> 30) & 0x1FF;
	unsigned short pd_offset = (va >> 21) & 0x1FF;
	unsigned short pt_offset = (va >> 12) & 0x1FF;

	if (!(pml4[pml4_offset] & PAGE_PRESENT))
	{
		void *p = arch_memory_allocate_page();

		if (p == NULL)
		{
			fatal("Unable to get page for PDPT\n");
		}

		pml4[pml4_offset] = (pml4e)p | flags;
	}

	pdpt = (pdpte *)virtual_address((pml4[pml4_offset] >> 12) << 12);

	if (!(pdpt[pdpt_offset] & PAGE_PRESENT))
	{
		void *p = arch_memory_allocate_page();

		if (p == NULL)
		{
			fatal("Unable to get page for PD\n");
		}

		pdpt[pdpt_offset] = (pdpte)p | flags;
	}

	pd = (pde *)virtual_address((pdpt[pdpt_offset] >> 12) << 12);

	if (pd[pd_offset] & (PAGE_PS | PAGE_PRESENT))
	{
		pd[pd_offset] = flags | PAGE_PS;
		return pml4[pml4_offset];
	}

	if (!(pd[pd_offset] & PAGE_PRESENT))
	{
		void *p = arch_memory_allocate_page();

		if (p == NULL)
		{
			fatal("Unable to get page for PT\n");
		}

		pd[pd_offset] = (pde)p | flags;
	}

	pt = (pte *)virtual_address((pd[pd_offset] >> 12) << 12);

	if (pt[pt_offset] & PAGE_PRESENT)
	{
		return ARCH_ERROR; // Page already mapped
	}
	else
	{
		pt[pt_offset] = (pte)pa | flags;
	}

	return ARCH_OK;
}

/*
static bool empty(unsigned long *table)
{
	for (int i = 0; i < 512; i++)
	{
		if (table[i] & PAGE_PRESENT)
			return false;
	}

	return true;
}
*/

arch_result arch_memory_unmap_page(uint64_t va)
{
	pdpte *pdpt;
	pde *pd;
	pte *pt;

	unsigned short pml4_offset = (va >> 39) & 0x1FF;
	unsigned short pdpt_offset = (va >> 30) & 0x1FF;
	unsigned short pd_offset = (va >> 21) & 0x1FF;
	unsigned short pt_offset = (va >> 12) & 0x1FF;

	if (!(pml4[pml4_offset] & PAGE_PRESENT))
		return ARCH_ERROR;

	pdpt = (pdpte *)virtual_address((pml4[pml4_offset] >> 12) << 12);

	if (!(pdpt[pdpt_offset] & PAGE_PRESENT))
		return ARCH_ERROR;

	if (pdpt[pdpt_offset] & PAGE_PS)
	{
		pdpt[pdpt_offset] = 0;
		return 0;
	}

	pd = (pde *)virtual_address((pdpt[pdpt_offset] >> 12) << 12);

	if (!(pd[pd_offset] & PAGE_PRESENT))
		return ARCH_ERROR;

	if (pd[pd_offset] & PAGE_PS)
	{
		pd[pd_offset] = 0;
		return ARCH_OK;
	}

	pt = (pte *)virtual_address((pd[pd_offset] >> 12) << 12);

	if (!(pt[pt_offset] & PAGE_PRESENT))
		return ARCH_ERROR;

	pt[pt_offset] = 0;

	return ARCH_OK;
}

void *arch_memory_allocate_page(void)
{
	void *page = NULL;

	
	return page;
}

void arch_memory_deallocate_page(void *page)
{
	
}

arch_result arch_memory_init(void)
{
	memory_map = (region_t *)virtual_address(MEMORY_MAP);
	uint32_t total_entries = *(uint32_t *)virtual_address(MEMORY_MAP_ENTRIES);
	uint32_t entry_size = *(uint32_t *)virtual_address(MEMORY_MAP_ENTRIES + 4);

	assert(entry_size == sizeof(region_t));

	for (unsigned long i = 0; i < total_entries - 1; i++)
	{
		total_regions++;

		total_memory += memory_map[i].size;

		if (memory_map[i].flags == 1)
		{
			total_memory_free += memory_map[i].size;
		}
		else
		{
			total_memory_reserved += memory_map[i].size;
		}

		arch_debug_printf("Memory: %lu: Start: 0x%016lx, Size: 0x%016lx, Flags: 0x%08x\n",
		                  i,
		                  memory_map[i].start,
		                  memory_map[i].size,
		                  memory_map[i].flags);
	}

	// TODO: account for kernel memory usage

	arch_debug_printf("Memory: %lu regions, %lu KB total, %lu KB free, %lu KB reserved\n",
	                  total_regions, total_memory / 1024, total_memory_free / 1024, total_memory_reserved / 1024);

	arch_debug_printf("Memory: %lu free pages available\n", total_memory_free / PAGE_SIZE);


	/* Remove bootstrap identity mapping */
	pml4[0] = 0;
	flush_tlb();
	
	return ARCH_OK;
}

void arch_memory_set(void *ptr, const uint8_t value, const uint64_t count)
{
    if (value == 0 || count < 8) {
        arch_memory_set_byte(ptr, value, count);
        return;
    }
    
    uint64_t addr = (uint64_t)ptr;
    
    if (IS_ALIGNED(addr, 8) && count >= 64 && IS_ALIGNED(count, 8)) {
        uint64_t qword_val = ((uint64_t)value << 56) | ((uint64_t)value << 48) |
                            ((uint64_t)value << 40) | ((uint64_t)value << 32) |
                            ((uint64_t)value << 24) | ((uint64_t)value << 16) |
                            ((uint64_t)value << 8) | value;
        arch_memory_set_qword(ptr, qword_val, count / 8);

        uint64_t remaining = count % 8;
        if (remaining > 0) {
            arch_memory_set_byte((uint8_t *)ptr + (count - remaining), value, remaining);
        }
        return;
    }
    
    if (IS_ALIGNED(addr, 4) && count >= 32 && IS_ALIGNED(count, 4)) {
        uint32_t dword_val = ((uint32_t)value << 24) | ((uint32_t)value << 16) |
                            ((uint32_t)value << 8) | value;
        arch_memory_set_dword(ptr, dword_val, count / 4);
        
        uint64_t remaining = count % 4;
        if (remaining > 0) {
            arch_memory_set_byte((uint8_t *)ptr + (count - remaining), value, remaining);
        }
        return;
    }
    
    if (IS_ALIGNED(addr, 2) && count >= 16 && IS_ALIGNED(count, 2)) {
        uint16_t word_val = ((uint16_t)value << 8) | value;
        arch_memory_set_word(ptr, word_val, count / 2);

        uint64_t remaining = count % 2;
        if (remaining > 0) {
            arch_memory_set_byte((uint8_t *)ptr + (count - remaining), value, remaining);
        }
        return;
    }
    
    arch_memory_set_byte(ptr, value, count);
}
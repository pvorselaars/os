#include "arch/x86_64/memory.h"
#include "arch/x86_64/gdt.h"

typedef struct region
{
	unsigned long start;
	unsigned long size;
} region;

static region free_regions[32]; /* Static array in kernel .bss section */

static unsigned long total_regions;
static unsigned long total_memory;
static unsigned long total_memory_free;
static unsigned long total_memory_reserved;

pml4e *pml4 = (pml4e *)virtual_address(PML4_ADDRESS);

// TODO: more efficient sorting algorithm
static void sort(region r[], int size)
{
	region temp;
	bool swapped = false;
	for (int i = 0; i < size - 1; i++)
	{
		for (int j = 0; j < size - i - 1; j++)
		{
			if (r[j].start > r[j + 1].start)
			{
				temp = r[j];
				r[j] = r[j + 1];
				r[j + 1] = temp;
				swapped = true;
			}
		}
		if (!swapped)
			break;
	}
}

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

	if (free_regions[0].size > 0)
	{
		page = (void *)free_regions[0].start;
		free_regions[0].start += PAGE_SIZE;
		free_regions[0].size--;

		sort(free_regions, total_regions);
	}

	return page;
}

void arch_memory_deallocate_page(void *page)
{
	region *current;
	uint64_t address = (uint64_t)page;

	for (int r = 0; r < total_regions; r++)
	{
		current = &free_regions[r];

		// Page is located before start of free regions
		if (current->start > (address + PAGE_SIZE))
		{
			assert(total_regions < 256);
			free_regions[total_regions].start = address;
			free_regions[total_regions].size = 1;
			total_regions++;
			break;
		}
		// Page fits at start of free region
		if (current->start == (address + PAGE_SIZE))
		{

			current->start -= PAGE_SIZE;
			current->size++;
			break;
		}
		// Page fits at end of free region
		if (current->start + current->size * PAGE_SIZE == address)
		{

			current->size++;

			// Page joins two free regions
			if (free_regions[r + 1].start == (address + PAGE_SIZE))
			{
				current->size += free_regions[r + 1].size;
				free_regions[r + 1].start = -1;
				free_regions[r + 1].size = 0;
				sort(free_regions, total_regions--);
				return;
			}

			break;
		}
		// Page is the new last region
		if (r == total_regions - 1)
		{
			assert(total_regions < 512);
			free_regions[total_regions].start = address;
			free_regions[total_regions].size = 1;
			total_regions++;
		}
	}

	sort(free_regions, total_regions);
}

void arch_memory_map_userpages(uint64_t pdpt)
{
	pml4[0] = pdpt;
}

arch_result arch_memory_init(void)
{
	total_regions = 4;
	total_memory = 0x200000;

	free_regions[0].start = PAGE_SIZE * 6;
	free_regions[0].size = 0xA0000 - free_regions[0].start;

	free_regions[1].start = 0xC0000;
	free_regions[1].size = 0xF0000 - 0xC0000;

	free_regions[2].start = 0xF0000 + ALIGN_UP((uint64_t)KERNEL_END - (uint64_t)KERNEL_VMA, PAGE_SIZE);
	free_regions[2].size = 0x100000 - free_regions[2].start;

	free_regions[3].start = 0x100000;
	free_regions[3].size = total_memory - 0x100000;

	total_memory_free = free_regions[0].size + free_regions[1].size + free_regions[2].size + free_regions[3].size;
	total_memory_reserved = total_memory - total_memory_free;

	/* GDT construction moved to arch/x86_64/gdt.c; call arch_gdt_init from
	 * arch initialization instead. */

	/* Remove bootstrap identity mapping */
	pml4[0] = 0;
	arch_memory_flush_tlb();
	
	return ARCH_OK;
}

void arch_memory_set(void *ptr, const uint8_t value, const uint64_t count)
{
    /* For simple byte patterns, use byte set */
    if (value == 0 || count < 8) {
        arch_memory_set_byte(ptr, value, count);
        return;
    }
    
    /* For aligned addresses and sizes, use larger operations */
    uint64_t addr = (uint64_t)ptr;
    
    /* Use qword operations for 8-byte aligned addresses and large counts */
    if (IS_ALIGNED(addr, 8) && count >= 64 && IS_ALIGNED(count, 8)) {
        uint64_t qword_val = ((uint64_t)value << 56) | ((uint64_t)value << 48) |
                            ((uint64_t)value << 40) | ((uint64_t)value << 32) |
                            ((uint64_t)value << 24) | ((uint64_t)value << 16) |
                            ((uint64_t)value << 8) | value;
        arch_memory_set_qword(ptr, qword_val, count / 8);

        /* Handle remaining bytes */
        uint64_t remaining = count % 8;
        if (remaining > 0) {
            arch_memory_set_byte((uint8_t *)ptr + (count - remaining), value, remaining);
        }
        return;
    }
    
    /* Use dword operations for 4-byte aligned addresses */
    if (IS_ALIGNED(addr, 4) && count >= 32 && IS_ALIGNED(count, 4)) {
        uint32_t dword_val = ((uint32_t)value << 24) | ((uint32_t)value << 16) |
                            ((uint32_t)value << 8) | value;
        arch_memory_set_dword(ptr, dword_val, count / 4);
        
        /* Handle remaining bytes */
        uint64_t remaining = count % 4;
        if (remaining > 0) {
            arch_memory_set_byte((uint8_t *)ptr + (count - remaining), value, remaining);
        }
        return;
    }
    
    /* Use word operations for 2-byte aligned addresses */
    if (IS_ALIGNED(addr, 2) && count >= 16 && IS_ALIGNED(count, 2)) {
        uint16_t word_val = ((uint16_t)value << 8) | value;
        arch_memory_set_word(ptr, word_val, count / 2);

        /* Handle remaining bytes */
        uint64_t remaining = count % 2;
        if (remaining > 0) {
            arch_memory_set_byte((uint8_t *)ptr + (count - remaining), value, remaining);
        }
        return;
    }
    
    /* Fall back to byte operations */
    arch_memory_set_byte(ptr, value, count);
}
#include "memory.h"

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

uint64_t memory_map(uint64_t va, uint64_t pa, int flags)
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
		void *p = memory_allocate();

		if (p == NULL)
		{
			fatal("Unable to get page for PDPT\n");
		}

		pml4[pml4_offset] = (pml4e)p | flags;
	}

	pdpt = (pdpte *)virtual_address((pml4[pml4_offset] >> 12) << 12);

	if (!(pdpt[pdpt_offset] & PAGE_PRESENT))
	{
		void *p = memory_allocate();

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
		void *p = memory_allocate();

		if (p == NULL)
		{
			fatal("Unable to get page for PT\n");
		}

		pd[pd_offset] = (pde)p | flags;
	}

	pt = (pte *)virtual_address((pd[pd_offset] >> 12) << 12);

	if (pt[pt_offset] & PAGE_PRESENT)
	{
		return 0;
	}
	else
	{
		pt[pt_offset] = (pte)pa | flags;
	}

	return pml4[pml4_offset];
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

int memory_unmap(address va)
{
	pdpte *pdpt;
	pde *pd;
	pte *pt;

	unsigned short pml4_offset = (va >> 39) & 0x1FF;
	unsigned short pdpt_offset = (va >> 30) & 0x1FF;
	unsigned short pd_offset = (va >> 21) & 0x1FF;
	unsigned short pt_offset = (va >> 12) & 0x1FF;

	if (!(pml4[pml4_offset] & PAGE_PRESENT))
		return -1;

	pdpt = (pdpte *)virtual_address((pml4[pml4_offset] >> 12) << 12);

	if (!(pdpt[pdpt_offset] & PAGE_PRESENT))
		return -1;

	if (pdpt[pdpt_offset] & PAGE_PS)
	{
		pdpt[pdpt_offset] = 0;
		return 0;
	}

	pd = (pde *)virtual_address((pdpt[pdpt_offset] >> 12) << 12);

	if (!(pd[pd_offset] & PAGE_PRESENT))
		return -1;

	if (pd[pd_offset] & PAGE_PS)
	{
		pd[pd_offset] = 0;
		return 0;
	}

	pt = (pte *)virtual_address((pd[pd_offset] >> 12) << 12);

	if (!(pt[pt_offset] & PAGE_PRESENT))
		return -1;

	pt[pt_offset] = 0;

	return 0;
}

void *memory_allocate()
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

void memory_deallocate(void *page)
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

void memory_map_userpages(uint64_t pdpt)
{
	pml4[0] = pdpt;
}

void print_pagetable_entries(address a)
{
	pdpte *pdpt;
	pde *pd;
	pte *pt;

	unsigned short pml4_offset = (a >> 39) & 0x1FF;
	unsigned short pdpt_offset = (a >> 30) & 0x1FF;
	unsigned short pd_offset = (a >> 21) & 0x1FF;
	unsigned short pt_offset = (a >> 12) & 0x1FF;

	printf("Addr: %016lx\n", a);
	if (!(pml4[pml4_offset] & PAGE_PRESENT))
	{
		printf("Page not present %x\n", pml4_offset * 8);
		return;
	}

	printf("PML4: %016lx\n", pml4[pml4_offset]);
	pdpt = (pdpte *)virtual_address((pml4[pml4_offset] >> 12) << 12);

	if (!(pdpt[pdpt_offset] & PAGE_PRESENT))
	{
		printf("Page not present\n");
		return;
	}

	printf("PDPT: %016lx\n", pdpt[pdpt_offset]);
	pd = (pde *)virtual_address((pdpt[pdpt_offset] >> 12) << 12);

	if (!(pd[pd_offset] & PAGE_PRESENT))
	{
		printf("Page not present\n");
		return;
	}

	printf("PD:   %016lx\n", pd[pd_offset]);

	if (pd[pd_offset] & PAGE_PS)
	{
		return;
	}

	pt = (pte *)virtual_address((pd[pd_offset] >> 12) << 12);

	if (!(pt[pt_offset] & PAGE_PRESENT))
	{
		printf("Page not present\n");
		return;
	}

	printf("PT:   %016lx\n", pt[pt_offset]);
}

void print_regions()
{

	printf("Free memory regions:\n");
	printf("%-16s %-16s\n", "address", "size");

	for (int r = 0; r < total_regions; r++)
	{
		printf("%-16lx %-16lx\n", free_regions[r].start, free_regions[r].size);
	}

	printf("%ld/%ld KiB free\n", total_memory_free / 0x400, total_memory / 0x400);
	printf("%ld/%ld KiB used\n", total_memory_reserved / 0x400, total_memory / 0x400);
}

void examine(void *ptr, unsigned long bytes)
{

	unsigned char *mem = (unsigned char *)ptr;

	for (int i = 0; i < bytes; i++)
	{
		printf("%02x ", *mem++);
	}
	printf("\n");
}

void memory_init()
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

	// Remove bootstrap identity mapping
	pml4[0] = 0;
	flush_tlb();
}

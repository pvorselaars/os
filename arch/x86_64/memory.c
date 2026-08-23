#include "arch/x86_64/memory.h"

#include "arch/memory.h"
#include "lib/utils.h"


typedef struct region
{
	unsigned long start;
	unsigned long size;
} region;

static region free_regions[32];

static unsigned long total_regions;
static unsigned long total_memory;
static unsigned long total_memory_free;
static unsigned long total_memory_reserved;

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

void x86_64_memory_init(void)
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

}

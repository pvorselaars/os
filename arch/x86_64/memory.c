#include "arch/x86_64/memory.h"
#include "arch/x86_64/gdt.h"

extern void flush_tlb(void);

typedef uint64_t physical_address_t;
#define PHYSICAL_ADDRESS_T_MAX ((physical_address_t)(-1))

#pragma pack(1)
typedef struct memory_map_entry
{
	uint64_t start;
	uint64_t size;
	uint32_t type;
} memory_map_entry_t;

typedef struct region region_t;

typedef struct region
{
	uint64_t frames;
	physical_address_t next;
	physical_address_t prev;
} region_t;

#pragma pack()


memory_map_entry_t *memory_map;

static region_t *free_regions = NULL;

static unsigned long total_memory;
static unsigned long total_memory_free;
static unsigned long total_memory_reserved;
static unsigned long kernel_size;

pml4e *pml4 = (pml4e *)virtual_address(PML4_ADDRESS);

/*
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

static bool empty(unsigned long *table)
{
for (int i = 0; i < 512; i++)
{
	if (table[i] & PAGE_PRESENT)
		return false;
}

return true;
}

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
*/

static physical_address_t region_head = PHYSICAL_ADDRESS_T_MAX;

static inline region_t* region_at(physical_address_t address)
{
	return (region_t *)virtual_address(address);
}

uint64_t arch_memory_allocate_frame(void)
{
	if (region_head == PHYSICAL_ADDRESS_T_MAX) return PHYSICAL_ADDRESS_T_MAX;

	region_t *region = region_at(region_head);
	physical_address_t base = region_head;
	uint64_t frames = region->frames;

	assert(frames > 0);

	physical_address_t allocated_frame = base + (frames - 1) * PAGE_SIZE;

	region->frames--;

	if (region->frames == 0)
	{
		physical_address_t next = region->next;
		physical_address_t prev = region->prev;

		if (prev != PHYSICAL_ADDRESS_T_MAX)
		{
			region_at(prev)->next = next;
		}
		else
		{
			region_head = next;
		}

		if (next != PHYSICAL_ADDRESS_T_MAX)
		{
			region_at(next)->prev = prev;
		}
	}

	arch_memory_set((void*)allocated_frame, 0, PAGE_SIZE);

	return allocated_frame;
}

void arch_memory_deallocate_frame(uint64_t frame)
{
	physical_address_t frames = 1;

	physical_address_t current = region_head;
	physical_address_t prev = PHYSICAL_ADDRESS_T_MAX;

	while (current != PHYSICAL_ADDRESS_T_MAX && current < frame)
	{
		prev = current;
		current = region_at(current)->next;
	}

	// coalesce with previous region
	if (prev != PHYSICAL_ADDRESS_T_MAX)
	{
		region_t *prev_region = region_at(prev);
		physical_address_t prev_end = prev + prev_region->frames * PAGE_SIZE;
		if (prev_end == frame)
		{
			prev_region->frames += frames;

			if (current != PHYSICAL_ADDRESS_T_MAX)
			{
				region_t *current_region = region_at(current);
				physical_address_t frame_end = frame + frames * PAGE_SIZE;
				if (frame_end == current)
				{
					prev_region->frames += current_region->frames;
					prev_region->next = current_region->next;
					if (current_region->next != PHYSICAL_ADDRESS_T_MAX)
					{
						region_at(current_region->next)->prev = prev;
					}
				}
			}

			return;
		}

	}

	// coalesce with next region
	if (current != PHYSICAL_ADDRESS_T_MAX)
	{
		region_t *current_region = region_at(current);
		physical_address_t frame_end = frame + frames * PAGE_SIZE;
		if (frame_end == current)
		{
			current_region->frames += frames;
			current_region->prev = prev;
			if (prev != PHYSICAL_ADDRESS_T_MAX)
			{
				region_at(prev)->next = frame;
			}
		}
	}

	// create new region
	region_t *new_region = region_at(frame);
	new_region->frames = frames;
	new_region->next = current;
	new_region->prev = prev;

	if (prev != PHYSICAL_ADDRESS_T_MAX)
	{
		region_at(prev)->next = frame;
	}
	else
	{
		region_head = frame;
	}

	if (current != PHYSICAL_ADDRESS_T_MAX)
	{
		region_at(current)->prev = frame;
	}
}

static void parse_memory_map(void)
{
	memory_map = (memory_map_entry_t *)virtual_address(MEMORY_MAP);
	uint32_t total_entries = *(uint32_t *)virtual_address(MEMORY_MAP_ENTRIES);
	uint32_t entry_size = *(uint32_t *)virtual_address(MEMORY_MAP_ENTRIES + 4);
	uint32_t total_regions = 0;

	assert(entry_size == sizeof(memory_map_entry_t));

	for (unsigned long i = 0; i < total_entries - 1; i++)
	{
		total_regions++;

		total_memory += memory_map[i].size;

		if (memory_map[i].type == 1)
		{
			if (region_head == PHYSICAL_ADDRESS_T_MAX)
			{
				region_head = memory_map[i].start;
				region_t *region = region_at(region_head);
				region->frames = memory_map[i].size / PAGE_SIZE;
				region->next = PHYSICAL_ADDRESS_T_MAX;
				region->prev = PHYSICAL_ADDRESS_T_MAX;
			}
			else
			{
				physical_address_t current = region_head;
				physical_address_t last = PHYSICAL_ADDRESS_T_MAX;

				while (current != PHYSICAL_ADDRESS_T_MAX && current < memory_map[i].start)
				{
					last = current;
					current = region_at(current)->next;
				}

				physical_address_t new_region_addr = memory_map[i].start;
				region_t *new_region = region_at(new_region_addr);
				new_region->frames = memory_map[i].size / PAGE_SIZE;
				new_region->next = current;
				new_region->prev = last;

				if (last != PHYSICAL_ADDRESS_T_MAX)
				{
					region_at(last)->next = new_region_addr;
				}
				else
				{
					region_head = new_region_addr;
				}

				if (current != PHYSICAL_ADDRESS_T_MAX)
				{
					region_at(current)->prev = new_region_addr;
				}
			}

			total_memory_free += memory_map[i].size;
		}
		else
		{
			total_memory_reserved += memory_map[i].size;
		}

	}

	kernel_size = (uint64_t)KERNEL_END - (uint64_t)KERNEL_VMA;
	total_memory -= kernel_size;
	total_memory_reserved += kernel_size;

}

static void print_regions(void)
{
	uint64_t current = (uint64_t)free_regions;

	arch_debug_printf("Free memory regions:\n");

	while (current != -1)
	{
		region_t *region = (region_t *)current;
		arch_debug_printf("  Address: 0x%016lx, frames: %lu\n",
						  (uint64_t)region,
						  region->frames);
		current = (uint64_t)region->prev;
	}
}

static void print_memory_map(void)
{
	arch_debug_printf("Memory map:\n");

	for (unsigned long i = 0; i < *(uint32_t *)virtual_address(MEMORY_MAP_ENTRIES) - 1; i++)
	{
		arch_debug_printf("  Entry %lu: Start: 0x%016lx, Size: 0x%016lx, Type: %d\n",
						  i,
						  memory_map[i].start,
						  memory_map[i].size,
						  memory_map[i].type);
	}

	print_regions();

	arch_debug_printf("%lu KB total, %lu KB free, %lu KB reserved\n",
					  total_memory / 1024, total_memory_free / 1024, total_memory_reserved / 1024);

	arch_debug_printf("%lu KB used by kernel\n", kernel_size / 1024);
}

static void test_frame_allocation(void)
{
	uint64_t first_frame = arch_memory_allocate_frame();
	assert(first_frame != (uint64_t)-1);

	arch_memory_deallocate_frame(first_frame);

	uint64_t second_frame = arch_memory_allocate_frame();
	assert(second_frame != (uint64_t)-1);
	assert(second_frame == first_frame);
}

arch_result arch_memory_init(void)
{
	parse_memory_map();
	print_memory_map();

	test_frame_allocation();

	/* Remove bootstrap identity mapping */
	pml4[0] = 0;
	flush_tlb();

	return ARCH_OK;
}

void arch_memory_set(void *ptr, const uint8_t value, const uint64_t count)
{
	if (value == 0 || count < 8)
	{
		arch_memory_set_byte(ptr, value, count);
		return;
	}

	uint64_t addr = (uint64_t)ptr;

	if (IS_ALIGNED(addr, 8) && count >= 64 && IS_ALIGNED(count, 8))
	{
		uint64_t qword_val = ((uint64_t)value << 56) | ((uint64_t)value << 48) |
							 ((uint64_t)value << 40) | ((uint64_t)value << 32) |
							 ((uint64_t)value << 24) | ((uint64_t)value << 16) |
							 ((uint64_t)value << 8) | value;
		arch_memory_set_qword(ptr, qword_val, count / 8);

		uint64_t remaining = count % 8;
		if (remaining > 0)
		{
			arch_memory_set_byte((uint8_t *)ptr + (count - remaining), value, remaining);
		}
		return;
	}

	if (IS_ALIGNED(addr, 4) && count >= 32 && IS_ALIGNED(count, 4))
	{
		uint32_t dword_val = ((uint32_t)value << 24) | ((uint32_t)value << 16) |
							 ((uint32_t)value << 8) | value;
		arch_memory_set_dword(ptr, dword_val, count / 4);

		uint64_t remaining = count % 4;
		if (remaining > 0)
		{
			arch_memory_set_byte((uint8_t *)ptr + (count - remaining), value, remaining);
		}
		return;
	}

	if (IS_ALIGNED(addr, 2) && count >= 16 && IS_ALIGNED(count, 2))
	{
		uint16_t word_val = ((uint16_t)value << 8) | value;
		arch_memory_set_word(ptr, word_val, count / 2);

		uint64_t remaining = count % 2;
		if (remaining > 0)
		{
			arch_memory_set_byte((uint8_t *)ptr + (count - remaining), value, remaining);
		}
		return;
	}

	arch_memory_set_byte(ptr, value, count);
}
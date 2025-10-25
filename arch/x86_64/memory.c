#include "../../include/arch/x86_64/memory.h"
#include "../../include/arch/x86_64/gdt.h"
#include "../../include/platform/pc/serial.h"

/* Small serial helpers for early boot debug (avoid pulling printf). */
static void mem_serial_print(const char *s)
{
	while (*s)
		serial_write(SERIAL_PORT_0, (uint8_t)*s++);
}

static void mem_serial_print_hex64(uint64_t v)
{
	const char hex[] = "0123456789abcdef";
	mem_serial_print("0x");
	for (int i = 15; i >= 0; --i)
	{
		uint8_t nib = (v >> (i * 4)) & 0xF;
		serial_write(SERIAL_PORT_0, hex[nib]);
	}
}

/* Define the runtime gdt pointer expected by gdt_init (assembly). We own
	this symbol here so the kernel can update/load a kernel-owned GDT. */
uint64_t *gdt;

/* Kernel-owned GDT (page aligned). We keep 16 entries like before. */
static uint64_t kernel_gdt[16] __attribute__((aligned(4096)));

/* Helper to build a GDT entry (first 8 bytes). For system descriptors like
	TSS the high base (bits 32-63) must be placed in the following entry. */
static void build_gdt_entry_at(uint64_t *table, int index, uint64_t base, uint64_t limit, uint8_t access, uint8_t flags)
{
	uint64_t v = 0;
	v |= (limit & 0xffffULL);					// limit 0:15
	v |= (base & 0xffffffULL) << 16;			// base 0:23
	v |= (uint64_t)(access & 0xff) << 40;		// access
	v |= (uint64_t)((limit >> 16) & 0xf) << 48; // limit 16:19
	v |= (uint64_t)(flags & 0xf) << 52;			// flags
	v |= (uint64_t)((base >> 24) & 0xff) << 56; // base 24:31

	table[index] = v;
}

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

	/* Build and load a kernel-owned GDT here (moved from kernel.c). This
	   avoids referencing boot-time GDT symbols and keeps one writable
	   `gdt` symbol in the kernel data segment for gdt_init to update. */
	{
		/* Null entry */
		kernel_gdt[0] = 0;

		/* 32-bit code segment (index 1, selector 0x08) */
		build_gdt_entry_at(kernel_gdt, 1, 0x0, 0xFFFFF, SDA_P | SDA_S | SDA_E | SDA_R, SDF_DB | SDF_G);

		/* 32-bit data segment (index 2, selector 0x10) */
		build_gdt_entry_at(kernel_gdt, 2, 0x0, 0xFFFFF, SDA_P | SDA_S | SDA_W, SDF_DB | SDF_G);

		/* 64-bit kernel code segment (index 3, selector 0x18) */
		build_gdt_entry_at(kernel_gdt, 3, 0x0, 0x0, SDA_P | SDA_S | SDA_E | SDA_R, SDF_L);

		/* 64-bit kernel data segment (index 4, selector 0x20) */
		build_gdt_entry_at(kernel_gdt, 4, 0x0, 0x0, SDA_P | SDA_S | SDA_W, 0x0);

		/* TSS slots (index 5 and 6) left zeroed; interrupt_init() will set them */
		kernel_gdt[5] = 0;
		kernel_gdt[6] = 0;

		/* 64-bit user code segment (index 7, selector 0x38) */
		build_gdt_entry_at(kernel_gdt, 7, 0x0, 0x0, SDA_P | SDA_S | SDA_E | SDA_R | SDA_U, SDF_L);

		/* 64-bit user data segment (index 8, selector 0x40) */
		build_gdt_entry_at(kernel_gdt, 8, 0x0, 0xFFFFF, SDA_P | SDA_S | SDA_W | SDA_U, SDF_DB | SDF_G);

		gdt_descriptor desc = {
			.limit = sizeof(kernel_gdt) - 1,
			.base = (uint64_t)kernel_gdt,
		};
		gdt_init(&desc);

		/* Debug: report that GDT was loaded and show base */
		mem_serial_print("[mem] gdt_loaded base=");
		mem_serial_print_hex64((uint64_t)kernel_gdt);
		mem_serial_print("\n");
	}

	/* Remove bootstrap identity mapping */
	pml4[0] = 0;
	flush_tlb();
}

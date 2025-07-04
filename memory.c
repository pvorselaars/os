#include "memory.h"

#include "arg.h"
#include "console.h"
#include "defs.h"
#include "utils.h"

#pragma pack(1)
typedef struct {
	unsigned long base;
	unsigned long size;
	unsigned int type;
} E820;

typedef struct {
	unsigned long base;
	unsigned long size;
	unsigned int type;
	unsigned int attributes;
} E820_3;

typedef struct region {
	unsigned long start;
	unsigned long size;
} region;

#pragma pack()

#define KERNELVA(pa) (pa + UPPER_MEMORY)
#define ALIGN(x, size) ((((x) + (size)) & ~(size - 1)))

extern char __KERNEL_START;
extern char __KERNEL_END;

unsigned long kernel_start_pa;
unsigned long kernel_end_pa;

unsigned long total_regions;
unsigned long total_memory;
unsigned long total_memory_free;
unsigned long total_memory_reserved;

region *free_regions = (region *) KERNELVA(0x0);

pml4e *pml4 = (pml4e *) PML4_ADDRESS;

// TODO: more efficient sorting algorithm
void sort(region r[], int size)
{
	region temp;
	bool swapped = false;
	for (int i = 0; i < size - 1; i++) {
		for (int j = 0; j < size - i - 1; j++) {
			if (r[j].start > r[j + 1].start) {
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

void evaluate_region(unsigned long start, unsigned long size, int type)
{

	if (type != 1) {
		total_memory += size;
		total_memory_reserved += size;
		return;
	}

	assert(total_regions < 256);

	// Does region overlap with bootstrap or kernel code?
	if (start < kernel_end_pa && (start + size > 0x7000)) {

		// Split the region up
		if (start < 0x7000) {

			if (start == 0) {
				start += PAGE_SIZE;
				size -= PAGE_SIZE;
				total_memory_reserved += PAGE_SIZE;
			}

			free_regions[total_regions].start = start;
			free_regions[total_regions].size = (0x7000 - start) / PAGE_SIZE;
			total_regions++;
		}

		if (start + size > kernel_end_pa) {
			assert(total_regions < 256);

			free_regions[total_regions].start = kernel_end_pa;
			free_regions[total_regions].size = (start + size - kernel_end_pa) / PAGE_SIZE;
			total_regions++;
		}

	} else {
		free_regions[total_regions].start = start;
		free_regions[total_regions].size = size / PAGE_SIZE;
		total_regions++;
	}

	total_memory += size;
	total_memory_free += size;

}

int memory_init()
{
	short *entries = (short *)E820_ADDRESS;
	short *size = (short *)(E820_ADDRESS + 2);
	kernel_start_pa = (unsigned long)&__KERNEL_START & 0x7fffffffffff;
	kernel_end_pa = (unsigned long)&__KERNEL_END & 0x7fffffffffff;

	kernel_end_pa = ALIGN(kernel_end_pa, PAGE_SIZE);

	total_regions = 0;
	total_memory = 0;
	total_memory_free = 0;
	total_memory_reserved = 0;

	// Convert the E820 memory map to a doubly linked list 
	// of free memory pages of size PAGE_SIZE. However
	// not all pages are actually free as 0x7000 - 0xffff
	// is in use by the bootstrap code, E820 and page tables.
	// 0x10000 - KERNEL_END is in use by the kernel code.
	// These regions need to be removed from the free regions
	// list.

	if (*size == 20) {
		E820 *regions = (E820 *) (E820_ADDRESS + 4);

		for (int r = 0; r < *entries; r++) {
			evaluate_region(regions[r].base, regions[r].size, regions[r].type);
		}

	} else if (*size == 24) {

		E820_3 *regions = (E820_3 *) (E820_ADDRESS + 4);

		for (int r = 0; r < *entries; r++) {
			if (regions[r].attributes & 1) {
				evaluate_region(regions[r].base, regions[r].size, regions[r].type);
			}
		}

	} else {
		fatal("invalid E820 entry size\n");
	}

	sort(free_regions, total_regions);

#ifdef DEBUG
	printf("Kernel start: %016lx\n", kernel_start_pa);
	printf("Kernel end:   %016lx\n", kernel_end_pa);
	printf("Kernel size:  %12ld KiB\n", (kernel_end_pa - kernel_start_pa) / 0x400);
	print_regions();
#endif

	return 0;
}

int map(address va, address pa, int flags)
{
	assert(pa = ALIGN(pa, PAGE_SIZE));

	pdpte *pdpt;
	pde *pd;
	pte *pt;

	unsigned short pml4_offset = (va >> 39) & 0x1FF;
	unsigned short pdpt_offset = (va >> 30) & 0x1FF;
	unsigned short pd_offset = (va >> 21) & 0x1FF;
	unsigned short pt_offset = (va >> 12) & 0x1FF;

	flags |= PAGE_PRESENT;

	if (!(pml4[pml4_offset] & PAGE_PRESENT)) {
		page *p = alloc();

		if (p == NULL) {
			fatal("Unable to get page for PDPT\n");
		}

		pml4[pml4_offset] = (pml4e) p | flags;
	}

	pdpt = (pdpte *) ((pml4[pml4_offset] >> 12) << 12);

	if (!(pdpt[pdpt_offset] & PAGE_PRESENT)) {
		page *p = alloc();

		if (p == NULL) {
			fatal("Unable to get page for PD\n");
		}

		pdpt[pdpt_offset] = (pdpte) p | flags;
	}

	pd = (pde *) ((pdpt[pdpt_offset] >> 12) << 12);

	if (!(pd[pd_offset] & PAGE_PRESENT)) {
		page *p = alloc();

		if (p == NULL) {
			fatal("Unable to get page for PT\n");
		}

		pd[pd_offset] = (pde) p | flags;
	}

	pt = (pte *) ((pd[pd_offset] >> 12) << 12);

	if (pt[pt_offset] & PAGE_PRESENT) {
		return -1;
	} else {
		pt[pt_offset] = (pte) pa | flags;
	}

	return 0;
}

bool empty(unsigned long *table)
{
	for (int i = 0; i < 512; i++) {
		if (table[i] & PAGE_PRESENT)
			return false;
	}

	return true;
}

int unmap(address va)
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

	pdpt = (pdpte *) ((pml4[pml4_offset] >> 12) << 12);

	if (!(pdpt[pdpt_offset] & PAGE_PRESENT))
		return -1;

	pd = (pde *) ((pdpt[pdpt_offset] >> 12) << 12);

	if (!(pd[pd_offset] & PAGE_PRESENT))
		return -1;

	pt = (pte *) ((pd[pd_offset] >> 12) << 12);

	pt[pt_offset] ^= PAGE_PRESENT;

	// Cascade free pages in page table structure
	if (empty(pt)) {
		pd[pd_offset] ^= PAGE_PRESENT;
		dealloc(pt);

		if (empty(pd)) {
			pdpt[pdpt_offset] ^= PAGE_PRESENT;
			dealloc(pd);

			if (empty(pdpt)) {
				pml4[pml4_offset] ^= PAGE_PRESENT;
				dealloc(pdpt);
			}

		}

	}

	return 0;
}

page *alloc()
{
	page *p = NULL;

	if (free_regions[0].size > 0) {
		p = (page *) free_regions[0].start;
		free_regions[0].start += PAGE_SIZE;
		free_regions[0].size--;

		sort(free_regions, total_regions);

		memsetq((void *)p, 0, PAGE_SIZE / 8);
	}

	return p;
}

void dealloc(page * p)
{
	address a = (address) p;
	region *current;

	for (int r = 0; r < total_regions; r++) {
		current = &free_regions[r];

		// Page is located before start of free regions
		if (current->start > (a + PAGE_SIZE)) {
			assert(total_regions < 256);
			free_regions[total_regions].start = a;
			free_regions[total_regions].size = 1;
			total_regions++;
			break;
		}
		// Page fits at start of free region
		if (current->start == (a + PAGE_SIZE)) {

			current->start -= PAGE_SIZE;
			current->size++;
			break;
		}
		// Page fits at end of free region
		if (current->start + current->size * PAGE_SIZE == a) {

			current->size++;

			// Page joins two free regions
			if (free_regions[r + 1].start == (a + PAGE_SIZE)) {
				current->size += free_regions[r + 1].size;
				free_regions[r + 1].start = -1;
				free_regions[r + 1].size = 0;
				sort(free_regions, total_regions--);
				return;
			}

			break;
		}
		// Page is the new last region
		if (r == total_regions - 1) {
			assert(total_regions < 256);
			free_regions[total_regions].start = a;
			free_regions[total_regions].size = 1;
			total_regions++;

		}

	}

	sort(free_regions, total_regions);
}

#ifdef DEBUG
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
	if (!(pml4[pml4_offset] & PAGE_PRESENT)) {
		printf("Page not present %x\n", pml4_offset * 8);
		return;
	}

	printf("PML4: %016lx\n", pml4[pml4_offset]);
	pdpt = (pdpte *) ((pml4[pml4_offset] >> 12) << 12);

	if (!(pdpt[pdpt_offset] & PAGE_PRESENT)) {
		printf("Page not present\n");
		return;
	}

	printf("PDPT: %016lx\n", pdpt[pdpt_offset]);
	pd = (pde *) ((pdpt[pdpt_offset] >> 12) << 12);

	if (!(pd[pd_offset] & PAGE_PRESENT)) {
		printf("Page not present\n");
		return;
	}

	printf("PD:   %016lx\n", pd[pd_offset]);

	if (pd[pd_offset] & PAGE_PS) {
		return;
	}

	pt = (pte *) ((pd[pd_offset] >> 12) << 12);

	if (!(pt[pt_offset] & PAGE_PRESENT)) {
		printf("Page not present\n");
		return;
	}

	printf("PT:   %016lx\n", pt[pt_offset]);

}

void print_regions()
{

	printf("Free memory regions:\n");
	printf("%-16s %-16s\n", "address", "size");

	for (int r = 0; r < total_regions; r++) {
		printf("%-16lx %-16lx\n", free_regions[r].start, free_regions[r].size);
	}

	printf("%ld/%ld KiB\n", total_memory_free / 0x400, total_memory / 0x400);

}
#endif

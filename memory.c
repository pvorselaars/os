#include "memory.h"
#include "defs.h"
#include "console.h"

#pragma pack(1)
typedef struct {
	unsigned long base;
	unsigned long length;
	unsigned int type;
} E820;

typedef struct region {
	unsigned long size;
	struct region *prev;
	struct region *next;
} region;

#pragma pack()

region *first_region = NULL;
region *last_region = NULL;

pml4e *pml4 = (pml4e *) PML4_ADDRESS;

int memory_init()
{
	short *entries = (short *)E820_ADDRESS;
	E820 *regions = (E820 *) (E820_ADDRESS + 4);

	// Convert the E820 memory map to a doubly linked list 
	// of free memory pages of size PAGE_SIZE. However
	// not all pages are actually free as the first
	// 2MiB of memory is already mapped by the bootstrap
	// paging tables and are in use.

	region *current;
	for (int r = 0; r < *entries; r++) {
		if (regions[r].type == 1) {

			// Ignore first 2MiB
			if (regions[r].base + regions[r].length < 0x200000) {
				continue;
			} else if (regions[r].base < 0x200000) {
				map(0x200000, 0x200000, PAGE_WRITE);
				current = (region *) 0x200000;
				current->size = (regions[r].length - (0x200000 - regions[r].base)) / PAGE_SIZE;
			} else {
				map(regions[r].base, regions[r].base, PAGE_WRITE);
				current = (region *) regions[r].base;
				current->size = regions[r].length / PAGE_SIZE;
			}

			current->next = NULL;
			current->prev = last_region;

			if (last_region != NULL) {
				last_region->next = current;
			} else {
				first_region = current;
			}

			last_region = current;
		}
	}

	return 0;
}

int map(address va, address pa, int flags)
{
	pdpte *pdpt;
	pde *pd;
	pte *pt;

	unsigned short pml4_offset = (va >> 39) & 0x1FF;
	unsigned short pdpt_offset = (va >> 30) & 0x1FF;
	unsigned short pd_offset = (va >> 21) & 0x1FF;
	unsigned short pt_offset = (va >> 12) & 0x1FF;

	if (va % PAGE_SIZE != 0) {
		va += PAGE_SIZE - (va % PAGE_SIZE);
	}

	if (pa % PAGE_SIZE != 0) {
		pa += PAGE_SIZE - (pa % PAGE_SIZE);
	}

	flags |= PAGE_PRESENT;

	if (!pml4[pml4_offset] & PAGE_PRESENT) {
		page p = calloc();

		if (p == NULL) {
			return -1;
		}

		pml4[pml4_offset] = (pml4e) p | flags;
	}

	pdpt = (pdpte *) ((pml4[pml4_offset] >> 12) << 12);

	if (!pdpt[pdpt_offset] & PAGE_PRESENT) {
		page p = calloc();

		if (p == NULL) {
			return -1;
		}

		pdpt[pdpt_offset] = (pdpte) p | flags;
	}

	pd = (pde *) ((pdpt[pdpt_offset] >> 12) << 12);

	if (!pd[pd_offset] & PAGE_PRESENT) {
		page p = calloc();

		if (p == NULL) {
			return -1;
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
		free(pt);

		if (empty(pd)) {
			pdpt[pdpt_offset] ^= PAGE_PRESENT;
			free(pd);

			if (empty(pdpt)) {
				pml4[pml4_offset] ^= PAGE_PRESENT;
				free(pdpt);
			}

		}

	}

	return 0;
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

	printf("Addr: %016x\n", a);
	if (!(pml4[pml4_offset] & PAGE_PRESENT)) {
		printf("Page not present\n");
		return;
	}

	printf("PML4: %016x\n", pml4[pml4_offset]);
	pdpt = (pdpte *) ((pml4[pml4_offset] >> 12) << 12);

	if (!(pdpt[pdpt_offset] & PAGE_PRESENT)) {
		printf("Page not present\n");
		return;
	}

	printf("PDPT: %016x\n", pdpt[pdpt_offset]);
	pd = (pde *) ((pdpt[pdpt_offset] >> 12) << 12);

	if (!(pd[pd_offset] & PAGE_PRESENT)) {
		printf("Page not present\n");
		return;
	}

	printf("PD:   %016x\n", pd[pd_offset]);
	pt = (pte *) ((pd[pd_offset] >> 12) << 12);

	if (!(pt[pt_offset] & PAGE_PRESENT)) {
		printf("Page not present\n");
		return;
	}

	printf("PT:   %016x\n", pt[pt_offset]);

}

void print_regions()
{

	region *current = first_region;
	printf("%-16s %-16s %-16s %-16s\n", "address", "size", "prev", "next");
	while (current) {
		printf("%16p %16x %16x %16x\n", current, current->size, current->prev, current->next);
		current = current->next;
	}

}

page alloc()
{
	page p = NULL;

	if (first_region != NULL) {
		p = (page) first_region;

		region *new = (region *) ((unsigned long)first_region + PAGE_SIZE);

		if (first_region->size > 1) {
			new->size = first_region->size - 1;
			new->next = first_region->next;
			first_region = new;
		} else {
			first_region = first_region->next;
		}
	}

	return p;
}

page calloc()
{
	page p = alloc();

	if (p != NULL) {
		memsetq((void *)p, 0, PAGE_SIZE / 8);
	}

	return p;
}

void free(page * p)
{
	address a = (address) p;
	region *current = first_region;

	// Page is located before start of free regions
	if ((unsigned long)current > (a + PAGE_SIZE)) {
		region *new = (region *) a;
		new->next = first_region;
		new->prev = NULL;
		new->size = 1;

		first_region = new;
		return;
	}

	for (; current != NULL; current = current->next) {

		// Page fits at start of free region
		if ((unsigned long)current == (a + PAGE_SIZE)) {

			region *new = (region *) a;
			new->next = current->next;
			new->prev = current->prev;
			new->size = current->size + 1;

			if (new->next) {
				new->next->prev = new;
			} else {
				last_region = new;
			}

			if (new->prev) {
				new->prev->next = new;
			} else {
				first_region = new;
			}

			return;
		}
		// Page fits at end of free region
		if ((unsigned long)current + current->size * PAGE_SIZE == a) {

			current->size++;

			// Page joins two free regions
			if ((unsigned long)current->next == (a + PAGE_SIZE)) {
				current->size += current->next->size;
				current->next = current->next->next;
			}

			return;
		}

	}

	// Page is the new last region

	region *new = (region *) a;
	new->prev = last_region;
	new->next = NULL;
	new->size = 1;

	last_region = new;
}

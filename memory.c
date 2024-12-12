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
	// of free memory pages of size PAGE_SIZE.

	region *current;
	for (int r = 0; r < *entries; r++) {
		if (regions[r].type == 1) {

			current = (region *) regions[r].base;

			// Ignore first page to avoid a (valid) null pointer
			if (current == NULL) {
				current = (region *) PAGE_SIZE;
				current->size = regions[r].length / PAGE_SIZE - 1;
			} else {
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

void print_pagetable_entries(address a)
{
	pdpte *pdpt;
	pde *pd;
	pte *pt;

	unsigned short pml4_offset = (a >> 39) & 0x1FF;
	unsigned short pdpt_offset = (a >> 30) & 0x1FF;
	unsigned short pd_offset = (a >> 21) & 0x1FF;
	unsigned short pt_offset = (a >> 12) & 0x1FF;

	if (!pml4[pml4_offset] & PAGE_PRESENT) {
		printf("Page not present\n");
		return;
	}

	printf("PML4: %016x\n", pml4[pml4_offset]);
	pdpt = (pdpte *) ((pml4[pml4_offset] >> 12) << 12);

	if (!pdpt[pdpt_offset] & PAGE_PRESENT) {
		printf("Page not present\n");
		return;
	}

	printf("PDPT: %016x\n", pdpt[pdpt_offset]);
	pd = (pde *) ((pdpt[pdpt_offset] >> 12) << 12);

	if (!pd[pd_offset] & PAGE_PRESENT) {
		printf("Page not present\n");
		return;
	}

	printf("PD:   %016x\n", pd[pd_offset]);
	pt = (pte *) ((pd[pd_offset] >> 12) << 12);

	if (!pt[pt_offset] & PAGE_PRESENT) {
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

void free(page p)
{

	for (region * current = first_region; current != NULL; current = current->next) {

		// Page is located before start of free regions
		if ((unsigned long)current > (p + PAGE_SIZE)) {
			region *new = (region *) p;
			new->next = first_region;
			new->prev = NULL;
			new->size = 1;

			first_region = new;
			return;
		}
		// Page fits at start of free region
		if ((unsigned long)current == (p + PAGE_SIZE)) {

			region *new = (region *) p;
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
		if ((unsigned long)current + current->size * PAGE_SIZE == p) {

			current->size++;

			// Page joins two free regions
			if ((unsigned long)current->next == (p + PAGE_SIZE)) {
				current->size += current->next->size;
				current->next = current->next->next;
			}

			return;
		}

	}

	// Page is the new last region

	region *new = (region *) p;
	new->prev = last_region;
	new->next = NULL;
	new->size = 1;

	last_region = new;
}

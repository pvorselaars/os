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
			current->size = regions[r].length / PAGE_SIZE;

			// Ignore first page to avoid a (valid) null pointer
			if (current == NULL) {
				unsigned int size = current->size - 1;
				current = (region *) PAGE_SIZE;
				current->size = size;
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

void print_regions()
{

	region *current = first_region;
	while (current) {
		printf("%16p, %16x %16x %16x\n", current, current->size, current->prev, current->next);

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

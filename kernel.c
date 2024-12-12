#include "console.h"
#include "memory.h"

void kernel()
{
	console_init();
	memory_init();

	page p = alloc();
	print_pagetable_entries(p);
	print_regions();

	free(p);
	print_pagetable_entries(p);
	print_regions();
	while (1) ;
}

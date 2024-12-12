#include "console.h"
#include "memory.h"

void kernel()
{
	console_init();
	memory_init();

	print_pagetable_entries(0x0);
	print_pagetable_entries(0x1000);
	print_pagetable_entries(0x200000);

	while (1) ;
}

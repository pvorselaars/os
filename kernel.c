#include "console.h"
#include "memory.h"

void kernel()
{
	console_init();

	unsigned long memory = 0;
	unsigned long free = 0;
	short *entries = (short *)0x7000;
	E820 *regions = (E820 *) 0x7004;

	printf("Memory map:\n");
	printf("%-16s %-16s %s\n", "base", "length", "type");
	for (int r = 0; r < *entries; r++) {
		printf("%016x:%016x %d\n", regions[r].base, regions[r].length, regions[r].type);
		memory += regions[r].length;
		if (regions[r].type == 1) {
			free += regions[r].length;
		}
	}
	printf("%d/%d KB\n", free / 1024, memory / 1024);

	pml4e *pml4 = (pml4e *) 0x8000;
	pdpte *pdpt = (pdpte *) (((*pml4) >> 12) << 12);
	pde *pd = (pde *) (((*pdpt) >> 12) << 12);
	pte *pt = (pte *) (((*pd) >> 12) << 12);

	printf("Page tables:\n");
	printf("PML4  %08x\n", *pml4);
	printf("PDPT0 %08x\n", *pdpt);
	printf("PD0   %08x\n", *pd);

	for (int p = 0; p < 512; p += 64) {
		printf("PT0   %08x\n", pt[p]);
	}

	while (1) ;
}

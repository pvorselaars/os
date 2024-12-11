#include "console.h"
#include "memory.h"

void kernel()
{
	console_init();

	unsigned long memory = 0;
	unsigned long free = 0;
	short *entries = (short *) 0x7000;
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

	while (1) ;
}

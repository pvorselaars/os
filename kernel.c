#include "console.h"
#include "memory.h"

void kernel()
{
  console_init();
	memory_init();

	print_regions();

	while (1);
}

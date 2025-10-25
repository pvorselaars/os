#include "platform/init.h"

void kernel(void)
{
	platform_init();

	while (1)
		platform_halt();
}

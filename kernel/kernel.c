#include "arch/arch.h"

void kernel(void)
{
	arch_result result = arch_init();
	if (result != ARCH_OK) {
		arch_halt();
	}

	while (1) {
		arch_halt();
	}
}

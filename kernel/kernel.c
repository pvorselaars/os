#include "arch/arch.h"

void kernel(void)
{
	arch_result result = arch_init();
	if (result != ARCH_OK) {
		// If arch initialization fails, halt immediately
		arch_halt();
	}

	// Main kernel execution would go here
	
	arch_halt();
}

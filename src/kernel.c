#include "console.h"
#include "memory.h"
#include "interrupt.h"
#include "disk.h"
#include "utils.h"

void kernel()
{

	memory_init();
	interrupt_init();

	enable_interrupts();

	while (1);
}

#include "console.h"
#include "memory.h"
#include "interrupt.h"

void kernel()
{
  console_init();
	memory_init();
	//interrupt_init();

	//enable_interrupts();

	while (1);
}

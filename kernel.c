#include "console.h"
#include "memory.h"
#include "interrupt.h"
#include "disk.h"
#include "utils.h"

void kernel()
{
  console_init();
	memory_init();
	interrupt_init();

	enable_interrupts();

	examine((void*)0x7dfe, 2);

	while (1);
}

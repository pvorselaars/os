#include "console.h"
#include "memory.h"
#include "interrupt.h"
#include "disk.h"
#include "utils.h"
#include "audio.h"

void kernel()
{

	memory_init();
	interrupt_init();

	beep();

	while (1);
}

#include "gdt.h"
#include "memory.h"
#include "interrupt.h"
#include "process.h"
#include "console.h"
#include "disk.h"
#include "utils.h"
#include "audio.h"
#include "serial.h"
#include "parallel.h"
#include "video.h"
#include "ps2.h"

void kernel(void)
{
	serial_init();
	memory_init();

	interrupt_init();

	vga_init();
	console_init();

	process_init();
	ps2_init();

	while (1)
		halt();
}

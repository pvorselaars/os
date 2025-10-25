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

	{
		const char *banner = "[kernel] serial initialized\n";
		const char *p = banner;
		while (*p)
			serial_write(SERIAL_PORT_0, (uint8_t)*p++);
	}
	memory_init();

	interrupt_init();

	vga_init();
	console_init();

	process_init(); // IDEA: 1 page for kernel stack and proc info, 1 page for PDPT
	ps2_init();

	while (1)
		halt();
}

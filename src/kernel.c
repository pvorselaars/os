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

void kernel(gdt_descriptor *gdt_descriptor)
{

	memory_init();
	gdt_init(virtual_address(gdt_descriptor));
	interrupt_init(); // TODO: setup kernel GDT
	vga_init();
	console_init();

	process_init(); // IDEA: 1 page for kernel stack and proc info, 1 page for PDPT

	serial_init();
	ps2_init();

	while (1)
		halt();
}

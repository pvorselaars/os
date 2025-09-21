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

void kernel(uint64_t *gdt_address)
{

	memory_init();
	interrupt_init(gdt_address); // TODO: setup kernel GDT
	vga_init();
	console_init();

	print_regions();

	process_init(); // IDEA: 1 page for kernel stack and proc info, 1 page for PDPT

	serial_init();
	ps2_init();

	while (1)
		halt();
}

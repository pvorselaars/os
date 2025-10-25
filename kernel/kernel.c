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
#include "gdt.h"
#include "memory.h"

uint64_t *gdt; // Will be initialized in kernel()

uint64_t *gdt; // Will be initialized in kernel()
uint64_t kernel_gdt[16]; // Clean GDT in kernel memory

void kernel(void)
{
	// Create a clean GDT in kernel memory
	gdt = kernel_gdt;
	
	// Null segment
	gdt[0] = 0;
	
	// 64-bit kernel code segment (0x08)
	gdt[1] = 0x00209a0000000000;
	
	// 64-bit kernel data segment (0x10)  
	gdt[2] = 0x0000920000000000;
	
	// 64-bit user code segment (0x18)
	gdt[3] = 0x0020fa0000000000;
	
	// 64-bit user data segment (0x20)
	gdt[4] = 0x0000f20000000000;
	
	// TSS segments (0x28 - will be set up by interrupt_init)
	gdt[5] = 0;
	gdt[6] = 0;
	
	// Load the clean GDT
	gdt_descriptor gdt_desc = {
		.limit = sizeof(kernel_gdt) - 1,
		.base = (uint64_t)kernel_gdt
	};
	gdt_init(&gdt_desc);

	memory_init();
	interrupt_init(); // TSS will be set up in the clean GDT
	vga_init();
	console_init();

	process_init(); // IDEA: 1 page for kernel stack and proc info, 1 page for PDPT

	serial_init();
	ps2_init();

	while (1)
		halt();
}

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

void kernel()
{

	memory_init();
	interrupt_init();
	process_init();

	// TOOD: move these to loadable driver processes
	serial_init();
	vga_init();
	console_init();
	ps2_init();

	while (1)
		halt();
}

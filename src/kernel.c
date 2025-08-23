#include "console.h"
#include "memory.h"
#include "interrupt.h"
#include "disk.h"
#include "utils.h"
#include "audio.h"
#include "serial.h"
#include "parallel.h"
#include "video.h"

void kernel()
{

	memory_init();
	serial_init();
	interrupt_init();
	vga_init_minimal();

	while (1) {
		uint8_t data = serial_read();
		parallel_write(PARALLEL_PORT_0, data);
	};
}

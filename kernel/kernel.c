#include "board/board.h"
#include "kernel/device.h"
#include "lib/string.h"

void kernel(void)
{
	arch_result result = arch_init();
	if (result != ARCH_OK) {
		arch_halt();
	}

	result = board_init();
	if (result != ARCH_OK) {
		arch_halt();
	}

	arch_interrupt_enable();
	
	// Initialize device subsystem
	result = device_init();
	if (result != ARCH_OK) {
		arch_debug_printf("Device subsystem initialization failed\n");
		arch_halt();
	}
	
	// Initialize all device drivers
	result = device_init_drivers();
	if (result != ARCH_OK) {
		arch_debug_printf("Some device drivers failed to initialize\n");
		arch_halt();
	}
	
	arch_debug_printf("🧪 Running device tests...\n");
	
	// Test 1: Console device
	device_t *console = device_find_by_name("console0");
	if (console && console->open(console) == ARCH_OK) {
		console->char_ops.write(console, "Console: VGA text mode working\n", 30);
		console->close(console);
	} else {
		arch_debug_printf("❌ Console test failed\n");
		arch_halt();
	}
	
	// Test 2: Block device
	device_t *disk = device_find_by_name("ata0");
	if (disk && disk->open(disk) == ARCH_OK) {
		disk->close(disk);
	} else {
		arch_debug_printf("❌ Disk test failed\n");
		arch_halt();
	}
	
	device_list_all();
	
	arch_debug_printf("🎉 Tests complete!\n");

	while (1) {
		arch_halt();
	}
}

#include "arch/arch.h"
#include "kernel/device.h"
#include "lib/string.h"

void kernel(void)
{
	arch_result result = arch_init();
	if (result != ARCH_OK) {
		arch_halt();
	}
	
	// Initialize device subsystem
	result = device_init();
	if (result != ARCH_OK) {
		arch_debug_printf("Device subsystem initialization failed\n");
		arch_halt();
	}
	
	// List all registered devices
	device_list_all();
	
	// Test the device API with serial device
	device_t *serial = device_find_by_name("serial0");
	if (serial) {
		arch_debug_printf("Found serial device: %s\n", serial->name);
		
		// Test writing to the device
		const char *test_msg = "Hello from device API!\n";
		int bytes_written = serial->char_ops.write(serial, test_msg, strlen(test_msg));
		arch_debug_printf("Wrote %d bytes to serial device\n", bytes_written);
	} else {
		arch_debug_printf("Serial device not found!\n");
	}

	// Test the device API with parallel device
	device_t *parallel = device_find_by_name("parallel0");
	if (parallel) {
		arch_debug_printf("Found parallel device: %s\n", parallel->name);
		
		// Test writing to the device
		const char *test_msg = "Hello from device API!\n";
		int bytes_written = parallel->char_ops.write(parallel, test_msg, strlen(test_msg));
		arch_debug_printf("Wrote %d bytes to parallel device\n", bytes_written);
	} else {
		arch_debug_printf("Parallel device not found!\n");
	}

	while (1) {
		arch_halt();
	}
}

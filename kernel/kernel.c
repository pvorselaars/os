#include "arch/arch.h"
#include "kernel/device.h"
#include "lib/string.h"

// Driver initialization functions
extern arch_result serial_driver_init(void);
extern arch_result parallel_driver_init(void);
extern arch_result keyboard_driver_init(void);
extern arch_result audio_driver_init(void);
extern arch_result disk_driver_init(void);

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
	
	result = serial_driver_init();
	if (result != ARCH_OK) {
		arch_debug_printf("Serial driver initialization failed\n");
	}
	
	result = parallel_driver_init();
	if (result != ARCH_OK) {
		arch_debug_printf("Parallel driver initialization failed\n");
	}
	
	result = keyboard_driver_init();
	if (result != ARCH_OK) {
		arch_debug_printf("Keyboard driver initialization failed\n");
	}
	
	result = audio_driver_init();
	if (result != ARCH_OK) {
		arch_debug_printf("Audio driver initialization failed\n");
	}

	
	result = disk_driver_init();
	if (result != ARCH_OK) {
		arch_debug_printf("Disk driver initialization failed\n");
	} else {
		arch_debug_printf("Disk driver initialized successfully\n");
	}
	
	device_list_all();

	// Test the disk device - simple block I/O test
	arch_debug_printf("Looking for disk device...\n");
	device_t *disk = device_find_by_name("ata0");
	if (disk) {
		arch_debug_printf("Found disk device: %s\n", disk->name);
		
		result = disk->open(disk);
		if (result == ARCH_OK) {
			arch_debug_printf("💾 Testing block device...\n");
			
			uint32_t block_size = disk->block_ops.get_block_size(disk);
			uint64_t block_count = disk->block_ops.get_block_count(disk);
			
			arch_debug_printf("📊 Disk info: %d bytes/block, %lu total blocks\n", 
				block_size, block_count);
			arch_debug_printf("📦 Total capacity: %lu MB\n", 
				(block_count * block_size) / (1024 * 1024));
			
			// Simple read/write test using first block
			static uint8_t test_buffer[512];
			static uint8_t read_buffer[512];
			
			// Fill test buffer with pattern
			for (int i = 0; i < 512; i++) {
				test_buffer[i] = (uint8_t)(i & 0xFF);
			}
			
			arch_debug_printf("📝 Writing test pattern to block 0...\n");
			int blocks_written = disk->block_ops.write_blocks(disk, test_buffer, 0, 1);
			if (blocks_written == 1) {
				arch_debug_printf("✅ Write successful\n");
				
				// Sync the write
				disk->block_ops.sync(disk);
				
				arch_debug_printf("📖 Reading back from block 0...\n");
				int blocks_read = disk->block_ops.read_blocks(disk, read_buffer, 0, 1);
				if (blocks_read == 1) {
					arch_debug_printf("✅ Read successful\n");
					
					// Verify data
					bool match = true;
					for (int i = 0; i < 512; i++) {
						if (test_buffer[i] != read_buffer[i]) {
							match = false;
							break;
						}
					}
					
					if (match) {
						arch_debug_printf("🎉 Block device test PASSED - data matches!\n");
					} else {
						arch_debug_printf("❌ Block device test FAILED - data mismatch\n");
					}
				} else {
					arch_debug_printf("❌ Read failed\n");
				}
			} else {
				arch_debug_printf("❌ Write failed\n");
			}
			
			disk->close(disk);
		} else {
			arch_debug_printf("Failed to open disk device\n");
		}
	} else {
		arch_debug_printf("Disk device not found!\n");
	}

	while (1) {
		arch_halt();
	}
}

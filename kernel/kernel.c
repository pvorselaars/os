#include "board/board.h"
#include "kernel/device.h"
#include "lib/string.h"
#include "arch/arch.h"
#include "test.h"

void kernel(void)
{
	arch_result result;
	TEST_CASE("Architecture initialization") {
		result = arch_init();
		TEST_ASSERT_EQUAL(ARCH_OK, result);
	}


	TEST_CASE("Board initialization") {
		result = board_init();
		TEST_ASSERT_EQUAL(ARCH_OK, result);
	}

	arch_interrupt_enable();
	
	TEST_CASE("Device initialization") {
		result = device_init();
		TEST_ASSERT_EQUAL(ARCH_OK, result);
	}
	
	TEST_CASE("Memory allocation") {
		TEST_ASSERT_EQUAL(0, 0);
	}

	TEST_REPORT();

	while (1)
		arch_halt();
}

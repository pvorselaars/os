#include "arch/arch.h"
#include "board/board.h"
#include "drivers/console.h"
#include "kernel/test.h"

void kernel(void)
{
	arch_init();
	board_init();

	console_register();

	arch_interrupt_enable();

#ifdef DEBUG
	if (test_run() != RESULT_OK) {
		arch_halt();
	}
#endif

	while (1) {
		arch_halt();
	}
}

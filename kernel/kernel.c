#include "arch/cpu.h"
#include "board/board.h"
#include "drivers/console.h"
#include "kernel/process.h"
#include "kernel/test.h"

void process1() {
	while (1) ;
}

void kernel(void)
{
	if (arch_cpu_init() != RESULT_OK || board_init() != RESULT_OK) {
		arch_cpu_halt();
	}

	if (console_register() != RESULT_OK) {
		arch_cpu_halt();
	}

	arch_cpu_interrupt_enable();

#ifdef DEBUG
	if (test_run() != RESULT_OK) {
		arch_cpu_halt();
	}
#endif

	process_start(process1);

	while (1) {
		arch_cpu_halt();
	}
}


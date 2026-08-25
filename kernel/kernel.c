#include "arch/cpu.h"
#include "board/board.h"
#include "drivers/console.h"
#include "kernel/process.h"
#include "kernel/syscall.h"
#include "kernel/test.h"

void process_1() {
	while (1) {
		sys_write("A\n");
	};
}

void process_2() {
	while (1) {
		sys_write("B\n");
	};
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

	process_create(process_1);
	process_create(process_2);
	process_scheduler_start();

	while (1) {
		arch_cpu_halt();
	}
}


#include "arch/x86_64/memory.h"

.globl load_idt
.globl load_tss
.globl arch_irq_enable
.globl arch_irq_disable
.globl interrupt

.globl timer_interrupt_handler
.globl ps2_keyboard_interrupt_handler
.globl serial_receive_interrupt_handler

.globl halt

.code64
.section .text

halt:
	hlt
	ret

load_idt:
	lidt (%rdi)
	ret

load_tss:
	ltr %di
	ret

arch_irq_enable:
	sti
	ret

arch_irq_disable:
	cli
	ret

interrupt:
	int $0x80
	ret

timer_interrupt_handler:
	call timer_interrupt
	mov $0x20, %al
	out %al, $0x20
	iretq

ps2_keyboard_interrupt_handler:
	call ps2_keyboard_interrupt
	mov $0x20, %al
	out %al, $0x20
	iretq

serial_receive_interrupt_handler:
	call serial_receive_interrupt
	mov $0x20, %al
	out %al, $0x20
	iretq

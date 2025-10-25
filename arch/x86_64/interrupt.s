#include "memory.h"

.globl load_idt
.globl load_tss
.globl enable_interrupts
.globl disable_interrupts
.globl interrupt

.globl interrupt0
.globl interrupt2
.globl interrupt4
.globl interrupt8
.globl interrupt13
.globl interrupt14
.globl interrupt32
.globl interrupt33
.globl interrupt36

.globl halt

.extern zero
.extern nmi
.extern overflow
.extern general_protection_fault
.extern page_fault

.extern schedule

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

enable_interrupts:
	sti
	ret

disable_interrupts:
	cli
	ret

interrupt:
	int $0x80
	ret

interrupt0:
	call zero
	iretq

interrupt2:
	call nmi
	iretq

interrupt4:
	call overflow
	iretq

interrupt8:
	call double_fault
	iretq

interrupt13:
	push %rsi
	push %rdi

	mov 8(%rsp), %rsi  # Get error code
	mov 16(%rsp), %rdi # Get RIP
	call general_protection_fault

	pop %rsi
	pop %rdi
	add $8, %rsp
	iretq

interrupt14:
	pop %rdx
	mov %cr2, %rsi
	pop %rdi
	call page_fault
	iretq

interrupt32:
	incq ticks(%rip)
	mov $0x20, %al
	out %al, $0x20

switch:
	push %rax
	mov 16(%rsp), %rax
	and $3, %rax
	cmp $0, %rax
	pop %rax

	je .skip # TODO: mark current process for reschedule 

	push %rax
	push %rbx
	push %rcx
	push %rdx
	push %rsi
	push %rdi
	push %r8
	push %r9
	push %r10
	push %r11
	push %r12
	push %r13
	push %r14
	push %r15
	push %rbp

	mov $DATA_SEG, %rax
	mov %rax, %ds
	mov %rax, %es
	mov %rax, %fs
	mov %rax, %gs
	mov %rax, %ss

	mov %rsp, %rdi
	call schedule

	mov %rax, %rsp
	pop %rbp

	pop %r15
	pop %r14
	pop %r13
	pop %r12
	pop %r11
	pop %r10
	pop %r9
	pop %r8
	pop %rdi
	pop %rsi
	pop %rdx
	pop %rcx
	pop %rbx
	pop %rax

	.skip:

	iretq

interrupt33:
	call ps2_keyboard_interrupt
	mov $0x20, %al
	out %al, $0x20
	iretq

interrupt36:
	call serial_receive
	mov $0x20, %al
	out %al, $0x20
	iretq

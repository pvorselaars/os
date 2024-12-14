.globl load_idt
.globl enable_interrupts
.globl disable_interrupts
.globl interrupt

.globl interrupt0
.globl interrupt2
.globl interrupt4
.globl interrupt13
.globl interrupt14

.extern zero
.extern nmi
.extern overflow
.extern general_protection_fault
.extern page_fault

.code64
.section .text

load_idt:
	lidt (%rdi)
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

interrupt13:
	pop %rsi
	pop %rdi
	call general_protection_fault
	iretq

interrupt14:
	pop %rdx
	mov %cr2, %rsi
	pop %rdi
	call page_fault
	iretq



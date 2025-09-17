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
.globl interrupt20
.globl interrupt21
.globl interrupt24

.globl halt

.extern zero
.extern nmi
.extern overflow
.extern general_protection_fault
.extern page_fault

.code64
.section .text

halt:
	hlt
	ret

load_idt:
	lidt (%rdi)
	ret

load_tss:
	ltr (%rdi)
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

interrupt20:
	incq ticks(%rip)
	mov $0x20, %al
	out %al, $0x20
	iretq

interrupt21:
	call ps2_keyboard_interrupt
	mov $0x20, %al
	out %al, $0x20
	iretq

interrupt24:
	call serial_receive
	mov $0x20, %al
	out %al, $0x20
	iretq



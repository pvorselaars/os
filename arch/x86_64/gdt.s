.globl gdt_init

.extern gdt

.code64
.section .text
gdt_init:
    lea gdt(%rip), %rax   # Load the address of the GDT
    mov 2(%rdi), %rdx     # Load the GDT base address from the descriptor
    mov %rdx, (%rax)      # Update the GDT base address
    lgdtq (%rdi)
    ret
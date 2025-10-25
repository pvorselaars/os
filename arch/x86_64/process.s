.globl arch_context_switch_asm
.globl arch_process_start_user_asm

.code64
.section .text

# arch_context_switch_asm(uint64_t *old_sp, uint64_t new_sp)
# Save current context and switch to new stack pointer
arch_context_switch_asm:
    # Save current registers on stack
    push %rbp
    push %rbx
    push %r12
    push %r13
    push %r14
    push %r15
    pushfq
    
    # Save current stack pointer to old_sp
    mov %rsp, (%rdi)
    
    # Switch to new stack pointer
    mov %rsi, %rsp
    
    # Restore registers from new stack
    popfq
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %rbx
    pop %rbp
    
    ret

# arch_process_start_user_asm(arch_context_t *context)
# Start a process in user mode (first time execution)
arch_process_start_user_asm:
    mov $0x43, %rcx    # user data segment selector (index 8, RPL=3)
    mov %rcx, %ds
    mov %rcx, %es
    mov %rcx, %fs
    mov %rcx, %gs

    # Load kernel stack pointer from context
    mov 8(%rdi), %rsp  # kstack_pointer
    
    # The stack is already set up with interrupt frame, just return
    # This will pop the interrupt frame and jump to user mode
    iretq
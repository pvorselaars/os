#include "lib/memory.h"
#include "arch/x86_64/memory.h"
#include "arch/process.h"

/* x86_64-specific process context */
struct arch_context {
    uint64_t kstack_base;       /* Kernel stack base address */
    uint64_t kstack_pointer;    /* Saved kernel stack pointer */
    uint64_t ustack_base;       /* User stack base address */
    uint64_t pagetable;         /* Page table address */
    uint64_t stack_size;          /* Stack size */
};

/* x86_64-specific constants */
#define USER_STACK_VADDR    0x0000000000700000ULL
#define USER_CODE_SELECTOR  0x3B  /* User code segment (RPL=3) */
#define USER_DATA_SELECTOR  0x43  /* User data segment (RPL=3) */

/* Assembly functions */
extern void arch_context_switch_asm(uint64_t *old_sp, uint64_t new_sp);
extern void arch_process_start_user_asm(arch_context_t *context);

void arch_process_init(void)
{
    /* x86_64-specific initialization if needed */
}

arch_context_t *arch_context_create(void *entry_point, uint64_t stack_size)
{
    arch_context_t *ctx = memory_allocate();
    if (!ctx) {
        return NULL;
    }
    
    memory_zero_struct(ctx);
    
    /* Allocate kernel stack */
    void *kstack_phys = memory_allocate();
    if (!kstack_phys) {
        memory_deallocate(ctx);
        return NULL;
    }
    
    ctx->kstack_base = (uint64_t)virtual_address(kstack_phys);
    ctx->kstack_pointer = ctx->kstack_base + PAGE_SIZE;
    ctx->stack_size = stack_size;
    
    /* Setup initial stack frame for x86_64 */
    uint64_t *sp = (uint64_t *)(ctx->kstack_base + PAGE_SIZE);
    
    /* Push initial interrupt frame for iretq */
    *(--sp) = USER_DATA_SELECTOR;                    /* SS */
    *(--sp) = USER_STACK_VADDR + stack_size;         /* RSP */
    *(--sp) = 0x202;                                 /* RFLAGS (IF=1) */
    *(--sp) = USER_CODE_SELECTOR;                    /* CS */
    *(--sp) = (uint64_t)entry_point;                 /* RIP */
    
    /* Push general-purpose registers (saved by interrupt handler) */
    for (int i = 0; i < 15; ++i) {
        *(--sp) = 0;  /* RAX, RBX, RCX, RDX, RSI, RDI, R8-R14 */
    }
    *(--sp) = ctx->kstack_base;  /* RBP */
    
    ctx->kstack_pointer = (uint64_t)sp;
    
    return ctx;
}

void arch_context_destroy(arch_context_t *context)
{
    if (!context) return;
    
    /* Free kernel stack */
    if (context->kstack_base) {
        void *kstack_phys = (void *)((void *)context->kstack_base);
        memory_deallocate(kstack_phys);
    }
    
    /* Free context structure */
    memory_deallocate(context);
}

uint64_t arch_context_switch(arch_context_t *old_ctx, arch_context_t *new_ctx)
{
    if (!old_ctx || !new_ctx) {
        return 0;
    }
    
    /* Switch page tables */
    if (new_ctx->pagetable != old_ctx->pagetable) {
        memory_map_userpages(new_ctx->pagetable);
        flush_tlb();
    }
    
    /* Update interrupt stack pointer */
    arch_set_interrupt_stack_pointer(new_ctx->kstack_base + PAGE_SIZE);
    
    /* Save old stack pointer and load new one */
    uint64_t old_sp;
    arch_context_switch_asm(&old_sp, new_ctx->kstack_pointer);
    
    return old_sp;
}

void arch_process_start_user(arch_context_t *context)
{
    if (!context) return;
    
    /* Switch to process page table */
    memory_map_userpages(context->pagetable);
    flush_tlb();
    
    /* Set interrupt stack pointer */
    arch_set_interrupt_stack_pointer(context->kstack_base + PAGE_SIZE);
    
    /* Start in user mode */
    arch_process_start_user_asm(context);
}

uint64_t arch_context_get_sp(arch_context_t *context)
{
    return context ? context->kstack_pointer : 0;
}

void arch_context_set_sp(arch_context_t *context, uint64_t sp)
{
    if (context) {
        context->kstack_pointer = sp;
    }
}

int arch_process_setup_memory(arch_context_t *context, void *entry_point)
{
    if (!context) return -1;
    
    /* Setup process page table */
    uint64_t entry_addr = (uint64_t)physical_address(entry_point);
    uint64_t entry_page = ALIGN_DOWN(entry_addr, PAGE_SIZE);
    
    /* Map process code page */
    uint64_t pagetable = memory_map(0, entry_page, 
                                   PAGE_PRESENT | PAGE_WRITE | PAGE_USER);
    if (!pagetable) {
        return -1;
    }
    
    /* Allocate and map user stack */
    void *ustack_phys = memory_allocate();
    if (!ustack_phys) {
        return -1;
    }
    
    if (memory_map(USER_STACK_VADDR, (uint64_t)ustack_phys, 
                   PAGE_PRESENT | PAGE_WRITE | PAGE_USER) == 0) {
        memory_deallocate(ustack_phys);
        return -1;
    }
    
    context->pagetable = pagetable;
    context->ustack_base = USER_STACK_VADDR;
    
    return 0;
}

void arch_process_cleanup_memory(arch_context_t *context)
{
    if (!context) return;
    
    /* TODO: Cleanup page tables and user memory mappings */
    /* This would involve unmapping pages and freeing physical memory */
}
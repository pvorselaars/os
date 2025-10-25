#ifndef ARCH_MM_H
#define ARCH_MM_H

#include "definitions.h"

typedef uint64_t vaddr_t;
typedef uint64_t paddr_t;

/* Abstract page mapping flags used by kernel core; architecture layer
   translates these to hardware-specific PTE bits. */
#define MMAP_PRESENT (1 << 0)
#define MMAP_WRITE (1 << 1)
#define MMAP_USER (1 << 2)
#define MMAP_PS (1 << 3) /* large page */

/* Opaque page-table handle for the arch implementation */
typedef struct arch_pagetable arch_pagetable_t;

/* Page table lifecycle */
arch_pagetable_t *arch_pagetable_create(void);
void arch_pagetable_destroy(arch_pagetable_t *pt);

/* Map/unmap a single page (or large page when MMAP_PS set). Return 0 on success */
int arch_pagetable_map(arch_pagetable_t *pt, vaddr_t va, paddr_t pa, unsigned flags);
int arch_pagetable_unmap(arch_pagetable_t *pt, vaddr_t va);

/* Activate the given top-level page table (CR3/TTBR write) */
void arch_pagetable_activate(arch_pagetable_t *pt);

/* Flush TLB (local/core or global) */
void arch_flush_tlb_local(void);
void arch_flush_tlb_global(void);

#endif

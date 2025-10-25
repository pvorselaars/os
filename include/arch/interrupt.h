#ifndef ARCH_INTERRUPT_H
#define ARCH_INTERRUPT_H

#include "kernel/definitions.h"

/* Architecture-level interrupt API

   Implementations must provide:
   - irq enable/disable
   - registration of vectors/IRQs
   - EOI (end of interrupt) if required by controller

   The kernel core uses these functions; implementations should hide
   APIC/GIC/PLATFORM differences.
*/

void arch_irq_enable(void);
void arch_irq_disable(void);

/* Register an interrupt handler for a vector/irq. Flags are arch-specific.
   Handler will be called in interrupt context with interrupts disabled. */
int arch_register_interrupt(unsigned vector, addr_t handler);
void arch_interrupt_init(void);

void arch_set_interrupt_stack_pointer(uint64_t sp);

void arch_irq_eoi(unsigned vector);

#endif

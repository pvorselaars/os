#include "../../include/arch/x86_64/gdt.h"
#include "../../include/arch/x86_64/memory.h"

/* Kernel-owned GDT (page aligned). Keep 16 entries. */
static uint64_t kernel_gdt[16] __attribute__((aligned(4096)));

void arch_gdt_set_entry(int index, uint64_t base, uint64_t limit, uint8_t access, uint8_t flags)
{
    uint64_t v = 0;
    v |= (limit & 0xffffULL);
    v |= (base & 0xffffffULL) << 16;
    v |= (uint64_t)(access & 0xff) << 40;
    v |= (uint64_t)((limit >> 16) & 0xf) << 48;
    v |= (uint64_t)(flags & 0xf) << 52;
    v |= (uint64_t)((base >> 24) & 0xff) << 56;

    kernel_gdt[index] = v;
}

void arch_gdt_init(void)
{
    /* Null entry */
    kernel_gdt[0] = 0;

    /* 32-bit code segment (index 1, selector 0x08) */
    arch_gdt_set_entry(1, 0x0, 0xFFFFF, SDA_P | SDA_S | SDA_E | SDA_R, SDF_DB | SDF_G);

    /* 32-bit data segment (index 2, selector 0x10) */
    arch_gdt_set_entry(2, 0x0, 0xFFFFF, SDA_P | SDA_S | SDA_W, SDF_DB | SDF_G);

    /* 64-bit kernel code segment (index 3, selector 0x18) */
    arch_gdt_set_entry(3, 0x0, 0x0, SDA_P | SDA_S | SDA_E | SDA_R, SDF_L);

    /* 64-bit kernel data segment (index 4, selector 0x20) */
    arch_gdt_set_entry(4, 0x0, 0x0, SDA_P | SDA_S | SDA_W, 0x0);

    /* TSS slots (index 5 and 6) left zeroed; arch interrupt_init will set them */
    kernel_gdt[5] = 0;
    kernel_gdt[6] = 0;

    /* 64-bit user code segment (index 7, selector 0x38) */
    arch_gdt_set_entry(7, 0x0, 0x0, SDA_P | SDA_S | SDA_E | SDA_R | SDA_U, SDF_L);

    /* 64-bit user data segment (index 8, selector 0x40) */
    arch_gdt_set_entry(8, 0x0, 0xFFFFF, SDA_P | SDA_S | SDA_W | SDA_U, SDF_DB | SDF_G);

    gdt_descriptor desc = {
        .limit = sizeof(kernel_gdt) - 1,
        .base = (uint64_t)kernel_gdt,
    };

    __asm__ volatile("lgdtq %0" : : "m"(desc));
}

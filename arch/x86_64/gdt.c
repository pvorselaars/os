#include "arch/x86_64/gdt.h"

#pragma pack(1)
typedef struct
{
    uint16_t limit;
    uint64_t base;
} gdt_descriptor;
#pragma pack()

static uint64_t gdt[16] __attribute__((aligned(4096)));

void x86_64_gdt_set_entry(int index, uint64_t base, uint64_t limit, uint8_t access, uint8_t flags)
{
    uint64_t v = 0;
    v |= (limit & 0xffffULL);
    v |= (base & 0xffffffULL) << 16;
    v |= (uint64_t)(access & 0xff) << 40;
    v |= (uint64_t)((limit >> 16) & 0xf) << 48;
    v |= (uint64_t)(flags & 0xf) << 52;
    v |= (uint64_t)((base >> 24) & 0xff) << 56;

    gdt[index] = v;
}

void x86_64_gdt_set_tss_entry(uint64_t base, uint64_t limit)
{
    x86_64_gdt_set_entry(7, base, limit, SDA_P | SDA_A | SDA_TSS, 0x0);
    gdt[8] = base >> 32;
}

void x86_64_gdt_init(void)
{
    // Null entry
    gdt[0] = 0;

    // 32-bit code segment (index 1, selector 0x08)
    x86_64_gdt_set_entry(1, 0x0, 0xFFFFF, SDA_P | SDA_S | SDA_E | SDA_R, SDF_DB | SDF_G);

    // 32-bit data segment (index 2, selector 0x10)
    x86_64_gdt_set_entry(2, 0x0, 0xFFFFF, SDA_P | SDA_S | SDA_W, SDF_DB | SDF_G);

    // 64-bit kernel code segment (index 3, selector 0x18)
    x86_64_gdt_set_entry(3, 0x0, 0x0, SDA_P | SDA_S | SDA_E | SDA_R, SDF_L);

    // 64-bit kernel data segment (index 4, selector 0x20)
    x86_64_gdt_set_entry(4, 0x0, 0x0, SDA_P | SDA_S | SDA_W, 0x0);

    // 64-bit user data segment (index 5, selector 0x28)
    x86_64_gdt_set_entry(5, 0x0, 0xFFFFF, SDA_P | SDA_S | SDA_W | SDA_U, SDF_DB | SDF_G);

    // 64-bit user code segment (index 6, selector 0x30)
    x86_64_gdt_set_entry(6, 0x0, 0x0, SDA_P | SDA_S | SDA_E | SDA_R | SDA_U, SDF_L);

    // TSS slots (index 7 and 8)
    gdt[7] = 0;
    gdt[8] = 0;

    gdt_descriptor desc = {
        .limit = sizeof(gdt) - 1,
        .base = (uint64_t)gdt,
    };

    __asm__ volatile("lgdtq %0" : : "m"(desc));
}

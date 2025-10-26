#ifndef GDT_H
#define GDT_H


#define SDA_TSS (0x09)  // Available 64-bit TSS
#define SDA_P (1 << 7) // Present segment
#define SDA_U (3 << 5) // User segment
#define SDA_S (1 << 4) // Descriptor type; system or code/data segment
#define SDA_E (1 << 3) // Executable segment
#define SDA_D (1 << 2) // Expand down (non-executable segments)
#define SDA_C (1 << 2) // Conforming code segment (executable only)
#define SDA_W (1 << 1) // Writeable (non-executable segments)
#define SDA_R (1 << 1) // Readable (executable segments)
#define SDA_A (1 << 0) // Accessed

#define SDF_G (1 << 3)  // Granularity flag
#define SDF_DB (1 << 2) // Size flag
#define SDF_L (1 << 1)  // Long mode flag

#ifndef __ASSEMBLER__
#include "definitions.h"

void x86_64_gdt_set_entry(int index, uint64_t base, uint64_t limit, uint8_t access, uint8_t flags);

void x86_64_gdt_init(void);
#endif

#endif
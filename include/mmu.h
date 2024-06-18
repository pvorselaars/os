#ifndef MMU_H
#define MMU_H

#ifdef __ASSEMBLER__

// GDT entry macros
#define SEGMENT_NULL \
        .word 0, 0;  \
        .byte 0, 0, 0, 0
#define SEGMENT(type, base, limit) \
        .word ((limit >> 12) & 0xffff), (base & 0xffff); \
        .byte ((base >> 16) & 0xff), (0x90 | type), (0xC0 | ((limit >> 28) & 0xf)), ((base >> 24) & 0xff)

#endif

#define STA_X   0x8     // Executable segment
#define STA_E   0x4     // Expand down (non-executable segments)
#define STA_C   0x4     // Conforming code segment (executable only)
#define STA_W   0x2     // Writeable (non-executable segments)
#define STA_R   0x2     // Readable (executable segments)
#define STA_A   0x1     // Accessed

#define CR0_PE  0x1     // Protected Mode enable flag

#define CODE_SEG 0x8    // Kernel code segment index
#define DATA_SEG 0x10   // Kernel data segment index

#endif

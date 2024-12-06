#ifndef MEMORY_H
#define MEMORY_H

#ifdef __ASSEMBLER__

#define SEGMENT(access, flags, base, limit) \
        .word (limit & 0xffff), (base & 0xffff); \
        .byte ((base >> 16) & 0xff), access, ((flags << 4) | ((limit >> 16) & 0xf)), ((base >> 24) & 0xff)

#else

void memsetb(void *ptr, char value, unsigned int num);
void memsetw(void *ptr, short value, unsigned int num);
void memsetl(void *ptr, int value, unsigned int num);
void memsetq(void *ptr, long value, unsigned int num);

int memcmp(void *ptr1, void *ptr2, unsigned int num);

void memmove(void *dst, void *src, unsigned int num);
void memcpy(void *dst, void *src, unsigned int num);

#endif

#define PAGE_PRESENT (1 << 0)
#define PAGE_WRITE   (1 << 1)

#define SDA_P        (1 << 7)     // Present segment
#define SDA_U        (3 << 5)     // User segment
#define SDA_S        (1 << 4)     // Descriptor type; system or code/data segment
#define SDA_E        (1 << 3)     // Executable segment
#define SDA_D        (1 << 2)     // Expand down (non-executable segments)
#define SDA_C        (1 << 2)     // Conforming code segment (executable only)
#define SDA_W        (1 << 1)     // Writeable (non-executable segments)
#define SDA_R        (1 << 1)     // Readable (executable segments)
#define SDA_A        (1 << 0)     // Accessed

#define SDF_G        (1 << 3)     // Granularity flag
#define SDF_DB       (1 << 2)     // Size flag
#define SDF_L        (1 << 1)     // Long mode flag

#define CR0_PE       (1 << 0)     // Protected Mode Enable bit
#define CR0_PG       (1 << 31)    // Paging bit

#define CR4_PGE      (1 << 7)     // Page Global Enable bit
#define CR4_PAE      (1 << 5)     // Physical Address Extension bit

#define MSR_EFER     0xC0000080   // EFER model specific register
#define EFER_LME     (1 << 8)     // Long mode bit

#define CODE_SEG     0x8          // Kernel code segment index
#define DATA_SEG     0x10         // Kernel data segment index




#endif

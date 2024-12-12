#ifndef MEMORY_H
#define MEMORY_H

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

#define E820_ADDRESS 0x7000
#define PML4_ADDRESS 0x8000

#define PAGE_SIZE    0x1000

#ifdef __ASSEMBLER__

#define SEGMENT(access, flags, base, limit) \
        .word (limit & 0xffff), (base & 0xffff); \
        .byte ((base >> 16) & 0xff), access, ((flags << 4) | ((limit >> 16) & 0xf)), ((base >> 24) & 0xff)

#else

typedef unsigned long int pte;
typedef unsigned long int pde;
typedef unsigned long int pdpte;
typedef unsigned long int pml4e;

typedef unsigned long int page;
typedef unsigned long int address;

void memsetb(const void *ptr, const char value, const unsigned int num);
void memsetw(const void *ptr, const short value, const unsigned int num);
void memsetl(const void *ptr, const int value, const unsigned int num);
void memsetq(const void *ptr, const long value, const unsigned int num);

int memcmp(const void *ptr1, const void *ptr2, const unsigned int num);

void memmove(const void *dst, const void *src, const unsigned int num);
void memcpy(const void *dst, const void *src, const unsigned int num);

int memory_init(void);

int map(address va, address pa, int flags);
int unmap(address va);
page* alloc(void);
page* calloc(void);
void free(page *p);

void print_regions();
void print_pagetable_entries(address a);



#endif


#endif

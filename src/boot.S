#include "memory.h"

.set gdt_offset, gdt_descriptor - _start
.set start_offset, 0x0

.globl reset_vector
.code16
_start:
  # Disable interrupts until kernel can setup interrupt routines
  cli
    
  lgdt %cs:gdt_offset
  
  # Enable protected mode
  mov %cr0, %eax
  or $0x1, %eax
  mov %eax, %cr0
  ljmp $0x08, $(protected_mode-_start)

.code32
protected_mode:
  mov $0x10, %ax
  mov %ax, %ds
  # In order to enable long mode some simple paging data structures need to be
  # present. So we identity map the first 2MiB of memory and map the higher half of memory to the lower half.

  # Clearing a buffer for page tables
  mov $PML4_ADDRESS, %edi
  mov $(PAGE_SIZE*5), %ecx
  xor %eax, %eax
  cld
  rep stosl
  
  # Setup PML4
  mov $PML4_ADDRESS, %edi
  mov $(PML4_ADDRESS + PAGE_SIZE + (PAGE_PRESENT | PAGE_WRITE)), %eax       # Point to PDPT, present + writable
  mov %eax, (%edi)
  mov %eax, 511*8(%edi)
  
  # Setup Page Directory Pointer Table
  mov $(PML4_ADDRESS + PAGE_SIZE), %edi
  mov $(PML4_ADDRESS + PAGE_SIZE * 2 + (PAGE_PRESENT | PAGE_WRITE)), %eax   # Point to PD, present + writable
  mov %eax, (%edi)
  
  # Setup Page Directory
  mov $(PML4_ADDRESS + PAGE_SIZE * 2), %edi
  mov $(0x0 | PAGE_PRESENT | PAGE_PS | PAGE_WRITE), %eax                    # 2MB page, present + writable + large
  mov %eax, (%edi)

  # Load PML4
  mov $PML4_ADDRESS, %eax
  mov %eax, %cr3
  
  # Enable PAE
  mov %cr4, %eax
  or $(CR4_PAE | CR4_PGE), %eax
  mov %eax, %cr4
  
  mov $(MSR_EFER), %ecx
  rdmsr
  or $(EFER_LME), %eax
  wrmsr
  
  mov %cr0, %eax
  or $(CR0_PE | CR0_PG), %eax
  mov %eax, %cr0
  
  ljmp $CODE_SEG, $long_mode

.code64
long_mode:
  mov $DATA_SEG, %rax
  mov %rax, %ds
  mov %rax, %es
  mov %rax, %fs
  mov %rax, %gs
  mov %rax, %ss
  mov $0xF0000, %rsp

  movabs $gdt, %rdi
  movabs $kernel, %rax
  jmp *%rax

.align 8
gdt:
  # null segment (0x00)
  SEGMENT(0, 0, 0, 0)

  # 32-bit code segment (0x08)
  SEGMENT(SDA_P | SDA_S | SDA_E | SDA_R, SDF_DB | SDF_G, BOOT_SEGMENT*16, 0xFFFFF)

  # 32-bit data segment (0x10)
  SEGMENT(SDA_P | SDA_S | SDA_W, SDF_DB | SDF_G, 0x0, 0xFFFFF)

  # 64-bit kernel code segment (0x18)
  SEGMENT(SDA_P | SDA_S | SDA_E | SDA_R, SDF_L, 0x0, 0x0)

  # 64-bit kernel data segment (0x20)
  SEGMENT(SDA_P | SDA_S | SDA_W, 0x0, 0x0, 0x0)

  # 64-bit TSS segment (0x28)
  SEGMENT(0, 0, 0, 0)
  SEGMENT(0, 0, 0, 0)

  # 64-bit user code segment (0x38)
  SEGMENT(SDA_P | SDA_S | SDA_E | SDA_R | SDA_U, SDF_L, 0x0, 0x0)

  # 64-bit user data segment (0x40)
  SEGMENT(SDA_P | SDA_S | SDA_W | SDA_U, SDF_DB | SDF_G, 0x0, 0xFFFFF)

gdt_descriptor:
  .word gdt_descriptor - gdt - 1    # size of GDT - 1
  .long gdt                         # GDT address

.section .reset,"ax"
.code16
reset_vector:
  ljmp $BOOT_SEGMENT, $start_offset

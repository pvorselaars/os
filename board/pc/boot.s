#include "arch/x86_64/memory.h"
#include "arch/x86_64/gdt.h"

.code16
_start:
  # Disable interrupts until kernel can setup interrupt routines
  cli

  # Load kernel sectors from disk using LBA addressing (BIOS int 13h, function 42h)
  # First check if LBA extensions are available
  mov $0x41, %ah       # Check extensions present
  mov $0x55aa, %bx     # Magic number
  mov $0x80, %dl       # Drive 0x80 (first hard disk)
  int $0x13
  jc disk_error        # No LBA support
  cmp $0xaa55, %bx
  jne disk_error       # No LBA support
  
  mov $dap, %si        # DS:SI points to DAP
  mov $0x42, %ah
  mov $0x80, %dl       # Drive 0x80 (first hard disk)
  int $0x13
  jc disk_error
    
  lgdt gdt_descriptor_16
  
  # Enable protected mode
  mov %cr0, %eax
  or $0x1, %eax
  mov %eax, %cr0
  ljmp $0x08, $protected_mode

.code32
protected_mode:
  mov $0x10, %ax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  mov %ax, %gs
  mov %ax, %ss
  mov $0x7c00, %esp
  
  # In order to enable long mode some simple paging data structures need to be
  # present. So we identity map the first 2MiB of memory and map the higher half of memory to the lower half.

  # Clearing a buffer for page tables
  mov $PML4_ADDRESS, %edi
  mov $(PAGE_SIZE*5), %ecx
  xor %eax, %eax
  cld
  rep stosb
  
  # Setup PML4 for both identity mapping and higher half
  mov $PML4_ADDRESS, %edi
  mov $(PML4_ADDRESS + PAGE_SIZE + (PAGE_PRESENT | PAGE_WRITE)), %eax       # Point to PDPT, present + writable
  mov %eax, (%edi)
  mov %eax, 511*8(%edi)
  
  # Setup Page Directory Pointer Table (maps both identity and higher half to same physical memory)
  mov $(PML4_ADDRESS + PAGE_SIZE), %edi
  mov $(PML4_ADDRESS + PAGE_SIZE * 2 + (PAGE_PRESENT | PAGE_WRITE)), %eax   # Point to PD, present + writable
  mov %eax, (%edi)         # PDPT[0] for identity mapping
  mov %eax, 510*8(%edi)    # PDPT[510] for higher half (0xFFFFFF8000000000 maps here)
  
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
  lgdt gdt_descriptor
  
  mov $DATA_SEG, %rax
  mov %rax, %ds
  mov %rax, %es
  mov %rax, %fs
  mov %rax, %gs
  mov %rax, %ss
  mov $KERNEL_STACK, %rsp
  mov %rsp, %rbp

  movabs $kernel, %rax
  jmp *%rax

disk_error:
  hlt
  jmp disk_error

# Disk Address Packet (DAP)
.align 4
dap:
  .byte 0x10           # Size of DAP (16 bytes)
  .byte 0x00           # Reserved (must be 0)
  .word KERNEL_SECTORS # Number of sectors to read (calculated by linker)
  .word 0x0000         # Offset: 0 (low 16 bits of 0x10000)
  .word 0x1000         # Segment: 0x1000 (0x10000 >> 4 = 0x1000)
  .quad 1              # Starting LBA sector (sector 1, after boot sector)

.align 8
gdt:
  # null segment (0x00)
  SEGMENT(0, 0, 0, 0)

  # 32-bit code segment (0x08)
  SEGMENT(SDA_P | SDA_S | SDA_E | SDA_R, SDF_DB | SDF_G, 0x0, 0xFFFFF)

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

# 16-bit GDT descriptor for initial load in real mode
gdt_descriptor_16:
  .word gdt_descriptor - gdt - 1    # size of GDT - 1
  .long gdt                         # GDT physical address (32-bit)

# 64-bit GDT descriptor for later use in long mode
gdt_descriptor:
  .word gdt_descriptor - gdt - 1    # size of GDT - 1
  .quad KERNEL_BASE + gdt           # GDT virtual address

# Pad to 510 bytes (512 - 2 for signature)
.fill 510 - (. - _start), 1, 0x00
.word 0xAA55  # Boot sector signature
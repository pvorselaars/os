# boot.S
#
# A very simple x64 bootloader which has 2 tasks:
#
# 1. Initialize the processor from 16-bit real mode to 64-bit long mode
# 2. Hand control to the kernel
#

#include "boot.h"
#include "memory.h"

.globl _start
.code16
_start:
  # Disable interrupts until kernel can setup interrupt routines
  cli
    
  lgdt %cs:gdt_descriptor
  
  # Enable protected mode
  mov %cr0, %eax
  or $0x1, %eax
  mov %eax, %cr0
  ljmp $0x08, $protected_mode

.code32
protected_mode:
  mov $0x10, %ax
  mov %ax, %ds
  # In order to enable long mode some simple paging data structures need to be
  # present. So we identity map the first 2MiB of memory

    # Clearing a buffer for PML4, PDPT, PD and PT
    mov $0x1000, %edi
    mov $0x5000, %ecx
    xor %eax, %eax
    cld
    rep stosl
    
    # Setup PML4
    mov $0x1000, %edi
    mov $0x2003, %eax       # Point to PDPT, present + writable
    mov %eax, (%edi)
    
    # Setup Page Directory Pointer Table
    mov $0x2000, %edi
    mov $0x3003, %eax       # Point to PD, present + writable
    mov %eax, (%edi)
    
    # Setup Page Directory
    mov $0x3000, %edi
    mov $0x83, %eax         # 2MB page, present + writable + large
    mov %eax, (%edi)
    
    # Load PML4
    mov $0x1000, %eax
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
    
    ljmp $0x18, $(0xF0000 + long_mode)

.code64
long_mode:
    mov $0x20, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    # For now, just halt
    cli
    hlt

.align 8
gdt:
    .quad 0x0000000000000000
    # 32-bit code segment (0x08)
    .quad 0x00CF9A0F0000FFFF
    # 32-bit data segment (0x10)
    .quad 0x00CF92000000FFFF
    # 64-bit code segment (0x18)
    .quad 0x00A09A0000000000
    # 64-bit data segment (0x20)
    .quad 0x0000920000000000

gdt_descriptor:
    .word gdt_descriptor - gdt - 1    # size of GDT - 1
    .long 0xF0000 + gdt               # GDT address

.org 0xFFF0
.code16
reset_vector:
    ljmp $0xf000, $0
.org 0xFFFF
    .byte 0x0

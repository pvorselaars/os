# boot.S
#
# A very simple x64 bootloader which has 3 tasks:
#
# 1. Initialize the processor from 16-bit real mode to 64-bit long mode
# 2. Load the kernel from disk to memory
# 3. Hand control to the kernel
#

#include "mmu.h"

.globl _start
.code16

_start:
  # 1. Initializing the processor

  # Disable interrupts until kernel can setup interrupt routines
  cli

  # Clear segment registers and set stack pointer to the start of the bootsector
  # in memory. The stack grows downward so it will not overwrite boot code.
  xorw %ax, %ax
  movw %ax, %ds
  movw %ax, %es
  movw %ax, %ss
  movw $0x7c00, %sp

  # In order to enable long mode some simple paging data structures need to be
  # present.

  # Clearing a buffer
  movw $0x8000, %di
  push %di
  movw $0x1000, %cx
  cld
  rep stosl

  # Create PML4
  pop %di
  lea 0x1000(%di), %ax
  or $3, %ax
  movw %ax, (%di)



spin:
  jmp spin

# Initial GDT to identity map virtual addresses
# to physical addresses
.p2align 2
gdt:
SEGMENT_NULL                            # null segment
SEGMENT(STA_X|STA_R, 0x0, 0xffffffff)   # code segment
SEGMENT(STA_W,       0x0, 0xffffffff)   # data segment

gdt_descriptor:
  .word gdt_descriptor - gdt - 1        # size of GDT - 1
  .long gdt                             # address of GDT


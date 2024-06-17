#include "mmu.h"
.globl _start
.code16

_start:
  cli

  xorw %ax, %ax
  movw %ax, %ds
  movw %ax, %es
  movw %ax, %ss

  # Switch to protected mode by
  # loading GDT and setting PE flag
  # in cr0 register
  lgdt gdt_descriptor
  movl %cr0, %eax
  orl  $CR0_PE, %eax
  mov  %eax, %cr0

  ljmp $CODE_SEG, $protected_mode

.code32
protected_mode:
  movw $DATA_SEG, %ax
  movw %ax, %ds
  movw %ax, %es
  movw %ax, %fs
  movw %ax, %gs
  movw %ax, %ss

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


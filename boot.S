# boot.S
#
# A very simple x64 bootloader which has 3 tasks:
#
# 1. Initialize the processor from 16-bit real mode to 64-bit long mode
# 2. Load the kernel from disk to memory
# 3. Hand control to the kernel
#

#include "memory.h"

.globl _start
.code16

  # Bogus boot parameter block
_start:
  jmp _entry
  nop

.fill 53, 1, 0

_entry:
  # 1. Initializing the processor

  # Disable interrupts until kernel can setup interrupt routines
  cli

  # Clear segment registers and set stack pointer to the start of the bootsector
  # in memory. The stack grows downward so it will not overwrite boot code.
  xor %ax, %ax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %ss
  mov $0x7c00, %bp
  mov %bp, %sp

  # 2. Read kernel from disk using BIOS routines
  call read

  # In order to enable long mode some simple paging data structures need to be
  # present.

  # Clearing a buffer for PML4, PDPT, PD and PT
  mov $0x8000, %di
  push %di
  mov $0x4000, %cx
  cld
  rep stosb

  # Setup PML4
  pop %di
  lea 0x1000(%di), %ax
  or $(PAGE_PRESENT | PAGE_WRITE), %ax
  mov %ax, (%di)

  # Setup PDPT
  lea 0x2000(%di), %ax
  or $(PAGE_PRESENT | PAGE_WRITE), %ax
  mov %ax, 0x1000(%di)

  # Setup PD
  lea 0x3000(%di), %ax
  or $(PAGE_PRESENT | PAGE_WRITE), %ax
  mov %ax, 0x2000(%di)
  
  # Build Page Table
  push %di
  lea 0x3000(%di), %di
  mov $(PAGE_PRESENT | PAGE_WRITE), %eax

  # Identity map the first 2MB
  .loop:
  mov %eax, (%di)
  add $0x1000, %eax
  add $0x8, %di
  cmp $0x200000, %eax
  jb .loop

  pop %di

  mov $0xff, %al
  out %al, $0xa1 
  out %al, $0x21

  lidt idt

  mov $(CR4_PAE | CR4_PGE), %eax
  mov %eax, %cr4

  mov %edi, %edx
  mov %edx, %cr3

  mov $(MSR_EFER), %ecx
  rdmsr

  or $(EFER_LME), %eax
  wrmsr

  mov %cr0, %ebx
  or $(CR0_PE | CR0_PG), %ebx
  mov %ebx, %cr0

  lgdt gdt_descriptor
  
  ljmp $CODE_SEG, $longmode

read:
  pusha
  mov $dap, %si
  mov $0x42, %ah
  int $0x13
  popa
  ret

halt:
  hlt
  jmp halt

.code64
longmode:
  cli
  mov $DATA_SEG, %ax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  mov %ax, %gs
  mov %ax, %ss

  jmp 0x80000

error:
  mov $2000, %rcx
  mov $0xb8000, %rdi
  mov $0x4f20, %ax
  rep stosw

spin:
  jmp spin

print_hex:
  add $3, %rsi
  mov $4, %rcx
  std
  1:
  pml4:
  lodsb
  push %ax
  rol $4, %al
  and $0xf, %al
  cmp $9, %al
  jbe 2f
  add $7, %al
  2:
  add $48, %al
  mov %al, (%rdi)
  inc %di
  inc %di
  pop %ax
  and $0xf, %al
  cmp $9, %al
  jbe 3f
  add $7, %al
  3:
  add $0x30, %al
  mov %al, (%rdi)
  inc %di
  inc %di
  loop 1b
  ret


.align 4
dap:
  .byte 16
  .byte 0
  .word 64
  .word 0x0000
  .word 0x8000
  .long 0x1
  .long 0x0
  
# Initial GDT to identity map virtual addresses to physical addresses
idt:
  .word 0
  .long 0

gdt:
SEGMENT(0, 0, 0, 0)                                             # null segment
SEGMENT(SDA_P | SDA_S | SDA_E | SDA_R, SDF_L, 0x0, 0xffffffff)  # code segment
SEGMENT(SDA_P | SDA_S | SDA_W | SDA_D, SDF_L, 0x0, 0xffffffff)  # data segment

gdt_descriptor:
  .word gdt_descriptor - gdt - 1        # size of GDT - 1
  .long gdt                             # address of GDT

# boot signature
  .space 510-(.-_start)
  .word 0xaa55

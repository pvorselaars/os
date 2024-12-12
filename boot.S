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

  # Detect memory regions
  mov $0x7000, %di
  call detect_memory

  # 2. Read kernel from disk using BIOS routines
  call read

  # In order to enable long mode some simple paging data structures need to be
  # present. So we identity map the first 4MiB of memory and
  # map 0xFFFF800000000000 - 0xFFFF800000400000 to 0x0 - 0x400000 for the kernel.

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

  lea 0x1000(%di), %ax
  or $(PAGE_PRESENT | PAGE_WRITE), %ax
  mov %ax, 0x800(%di)

  # Setup Page Directory Pointer Tables
  lea 0x2000(%di), %ax
  or $(PAGE_PRESENT | PAGE_WRITE), %ax
  mov %ax, 0x1000(%di)

  lea 0x4000(%di), %ax
  or $(PAGE_PRESENT | PAGE_WRITE), %ax
  mov %ax, 0x3000(%di)

  # Setup Page Directories
  xor %ax, %ax
  or $(PAGE_PRESENT | PAGE_WRITE | PAGE_PS), %ax
  mov %ax, 0x2000(%di)
  mov %ax, 0x2008(%di)
  mov %ax, 0x4000(%di)
  mov %ax, 0x4008(%di)

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

detect_memory:
  pusha
  mov %di, %si
  add $4, %di
  xor %ebx, %ebx
  mov $0x534d4150, %edx
  mov $0xe820, %eax
  mov $24, %ecx
  int $0x15
  jc 1f
  2:
  cmp $0, %ebx
  je 1f
  mov %bx, (%si)
  mov %cx, 2(%si)
  add %cx, %di
  mov $0xe820, %eax
  mov $24, %ecx
  int $0x15
  jmp 2b
  1:
  popa
  ret

read:
  pusha
  mov $dap, %si
  mov $0x42, %ah
  int $0x13
  popa
  ret

.code64
longmode:
  mov $DATA_SEG, %ax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  mov %ax, %gs
  mov %ax, %ss
	mov $0xffff800000200000, %rsp
	mov %rsp, %rbp

  mov $0xffff800000010000, %rax
  jmpq *%rax

.align 4
dap:
  .byte 16			# size of DAP
  .byte 0				# should be zero
  .word 99			# sectors to read
  .word 0x0000  # offset
  .word 0x1000	# segment
  .quad 0x1			# starting lba
  
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

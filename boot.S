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
  movw $0x7c00, %bp
  movw %bp, %sp

  # Check for long mode support
  call check_processor
  jc cpu_error

  # In order to enable long mode some simple paging data structures need to be
  # present.

  # Clearing a buffer for PML4, PDPT, PD and PT
  movw $0x8000, %di
  push %di
  movw $0x1000, %cx
  cld
  rep stosl

  # Setup PML4
  pop %di
  lea 0x1000(%di), %eax
  or $(PAGE_PRESENT | PAGE_WRITE), %eax
  movw %ax, (%di)

  # Setup PDPT
  lea 0x2000(%di), %eax
  or $(PAGE_PRESENT | PAGE_WRITE), %eax
  movw %ax, 0x1000(%di)

  # Setup PD
  lea 0x3000(%di), %eax
  or $(PAGE_PRESENT | PAGE_WRITE), %eax
  movw %ax, 0x2000(%di)
  
  # Build Page Table
  push %di
  lea 0x3000(%di), %di
  mov $(PAGE_PRESENT | PAGE_WRITE), %eax

  # Identity map the first 2MB
  .loop:
  mov %eax, (%di)
  add $0x1000, %eax
  add $0x8, %edi
  cmp $0x200000, %eax
  jb .loop

  pop %di

  mov $0xff, %al
  out %al, $0xa1 
  out %al, $0x21

  lidt idt

  mov $0b10100000, %eax
  mov %eax, %cr4

  mov %edi, %edx
  mov %edx, %cr3

  mov $0xC0000080, %ecx
  rdmsr

  or $0x100, %eax
  wrmsr

  mov %cr0, %ebx
  or $0x80000001, %ebx
  mov %ebx, %cr0

  lgdt gdt_descriptor
  
  ljmp $CODE_SEG, $longmode

check_processor:
  # check whether extended CPUID functions are available
  push %eax
  mov $0x80000000, %eax
  cpuid
  cmp $0x80000001, %eax
  jb .error
  mov $0x80000001, %eax
  cpuid
  # test LM bit
  test $(1<<29), %edx
  jnz .noerror
  .error:
  stc
  .noerror:
  pop %eax
  ret

print:
  push %ax
  mov $0x0e, %ah
  .char:
  lodsb
  test %al, %al
  jz .ret
  int $0x10
  jmp .char
  .ret: 
  pop %ax
  ret

cpu_error:
  mov $cpu_error_msg, %si
  call print

halt:
  hlt
  jmp halt

.code64
longmode:
  mov $DATA_SEG, %ax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  mov %ax, %gs
  mov %ax, %ss

  # load remainder of bootstrap code from disk

  mov $1, %rax
  mov $1, %rcx
  mov $0x7e00, %rdi
  call ata_read
  jmp boot

error:
  mov $2000, %rcx
  mov $0xb8000, %rdi
  mov $0x4f20, %ax
  rep stosw

spin:
  jmp spin

ata_read:
  push %rdi
  push %rcx
  push %rax
  push %rdx
  mov %eax, %ebx

  shr $24, %eax
  or $0xe0, %eax
  mov $0x1f6, %dx
  out %al, %dx

  mov %ecx, %eax
  mov $0x1f2, %dx
  out %al, %dx

  mov %ebx, %eax
  mov $0x1f3, %dx
  out %al, %dx

  mov $0x1f4, %dx
  shr $8, %eax
  out %al, %dx

  mov $0x1f5, %dx
  shr $8, %eax
  out %al, %dx

  mov $0x1f7, %dx
  mov $0x20, %al
  out %al, %dx

  .next_sector:
  push %rcx

  # TODO: check for errors
  mov $0x1f7, %dx
  .try:
  in %dx, %al
  test $0x80, %al
  jnz .try
  test $8, %al
  jz .try

  mov $128, %ecx
  mov $0x1f0, %dx
  rep insl

  pop %rcx
  loop .next_sector
  pop %rdx
  pop %rcx
  pop %rax
  pop %rdi
  ret

cpu_error_msg: .ascii "CPU not supported"
  
# Initial GDT to identity map virtual addresses to physical addresses
idt:
  .word 0
  .long 0

gdt:
SEGMENT_NULL
SEGMENT(STA_X|STA_R, 0x0, 0xffffffff)   # code segment
SEGMENT(STA_W,       0x0, 0xffffffff)   # data segment

gdt_descriptor:
  .word gdt_descriptor - gdt - 1        # size of GDT - 1
  .long gdt                             # address of GDT


# boot signature
  .space 510-(.-_start)
  .word 0xaa55

# 2. Load kernel from disk

# check for bootable partition

boot:
  mov $4, %rcx
  mov $partitions, %edi
next_partition:
  movb (%edi), %al
  cmp $0x80, %al
  jz boot_partition
  add $20, %edi
  loop next_partition
  jmp error

boot_partition:
  # load super block
  movw 2(%edi), %ax
  mov $0x200, %rbx
  div %rbx
  mov %ax, %cx

  movq 4(%edi), %rax
  mul %rcx

  mov $0xc000, %rdi
  call ata_read
  jmp spin


# partition table
  .space 924-(.-_start)
partitions:
  .byte 0x80       # bootable
  .byte 0x63       # partition type
  .word 1024       # block size
  .quad 1          # start LBA
  .quad 19         # size

  .space 1024-(.-_start)

superblock:
  .quad 18        # number of blocks
  .quad 0         # number of free blocks
  .word 1024      # block size
  .word 0         # state
  .long 0         # reserved
  .quad 1024/64   # number of inodes
  .quad 0         # number of free inodes

  .space 2048-(.-_start)

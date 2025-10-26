.code64

.globl arch_memory_set_byte
.globl arch_memory_set_word
.globl arch_memory_set_dword
.globl arch_memory_set_qword
.globl arch_memory_compare
.globl arch_memory_move
.globl arch_memory_copy
.globl arch_memory_flush_tlb

.section .text

arch_memory_set_byte:
  cld
  mov %rsi, %rax
  mov %rdx, %rcx
  rep stosb
  ret

arch_memory_set_word:
  cld
  mov %rsi, %rax
  mov %rdx, %rcx
  rep stosw
  ret

arch_memory_set_dword:
  cld
  mov %rsi, %rax
  mov %rdx, %rcx
  rep stosl
  ret

arch_memory_set_qword:
  cld
  mov %rsi, %rax
  mov %rdx, %rcx
  rep stosq
  ret

arch_memory_compare:
  cld
  xor %rax, %rax
  mov %rdx, %rcx
  repe cmpsb
  setnz %al
  ret

arch_memory_move:
arch_memory_copy:
  cld
  cmp %rdi, %rsi
  jae 1f
  mov %rsi, %r8
  add %rdx, %r8
  cmp %rdi, %r8
  jbe 1f

  std
  add %rdx, %rdi
  add %rdx, %rsi
  sub $1, %rdi
  sub $1, %rsi

  1:
  mov %rdx, %rcx
  rep movsb
  ret

arch_memory_flush_tlb:
  mov %cr3, %rax
  mov %rax, %cr3
  ret
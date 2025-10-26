.code64

.globl memory_set_byte
.globl memory_set_word
.globl memory_set_dword
.globl memory_set_qword
.globl memory_compare
.globl memory_move
.globl memory_copy
.globl flush_tlb

.section .text

memory_set_byte:
  cld
  mov %rsi, %rax
  mov %rdx, %rcx
  rep stosb
  ret

memory_set_word:
  cld
  mov %rsi, %rax
  mov %rdx, %rcx
  rep stosw
  ret

memory_set_dword:
  cld
  mov %rsi, %rax
  mov %rdx, %rcx
  rep stosl
  ret

memory_set_qword:
  cld
  mov %rsi, %rax
  mov %rdx, %rcx
  rep stosq
  ret

memory_compare:
  cld
  xor %rax, %rax
  mov %rdx, %rcx
  repe cmpsb
  setnz %al
  ret

memory_move:
memory_copy:
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

flush_tlb:
  mov %cr3, %rax
  mov %rax, %cr3
  ret
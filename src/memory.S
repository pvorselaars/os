.code64

.globl memsetb
.globl memsetw
.globl memsetl
.globl memsetq
.globl memcmp
.globl memmove
.globl memcpy

.section .text

memsetb:
  cld
  mov %rsi, %rax
  mov %rdx, %rcx
  rep stosb
  ret

memsetw:
  cld
  mov %rsi, %rax
  mov %rdx, %rcx
  rep stosw
  ret

memsetl:
  cld
  mov %rsi, %rax
  mov %rdx, %rcx
  rep stosl
  ret

memsetq:
  cld
  mov %rsi, %rax
  mov %rdx, %rcx
  rep stosq
  ret

memcmp:
  cld
  xor %rax, %rax
  mov %rcx, %rdx
  repe cmpsb
  setnz %al
  ret

memmove:
memcpy:
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

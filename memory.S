.code64

.globl memsetb
.globl memsetw
.globl memsetl
.globl memsetq

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

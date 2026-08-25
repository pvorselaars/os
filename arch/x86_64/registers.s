.code64
.globl x86_64_write_msr
.globl x86_64_read_msr

.section .text

x86_64_write_msr:
   movl %edi, %ecx
   movl %esi, %eax
   shrq $32, %rsi
   movl %esi, %edx
   wrmsr
   ret

x86_64_read_msr:
   mov %edi, %ecx
   rdmsr
   ret




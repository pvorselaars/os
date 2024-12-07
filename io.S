.globl inb
.globl inw
.globl outb
.globl outw

inb:
  mov %di, %dx
  in %dx, %al
  ret
   
inw:
  mov %di, %dx
  in %dx, %ax
  ret

outb:
  mov %di, %dx
  mov %si, %ax
  out %al, %dx
  ret

outw:
  mov %di, %dx
  mov %si, %ax
  out %ax, %dx
  ret

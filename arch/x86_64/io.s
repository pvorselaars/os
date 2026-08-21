.globl arch_io_read8
.globl arch_io_read16
.globl arch_io_write8
.globl arch_io_write16

arch_io_read8:
  mov %di, %dx
  in %dx, %al
  ret
   
arch_io_read16:
  mov %di, %dx
  in %dx, %ax
  ret

arch_io_write8:
  mov %di, %dx
  mov %si, %ax
  out %al, %dx
  ret

arch_io_write16:
  mov %di, %dx
  mov %si, %ax
  out %ax, %dx
  ret

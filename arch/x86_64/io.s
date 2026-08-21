.globl pc_io_read8
.globl pc_io_read16
.globl pc_io_write8
.globl pc_io_write16
pc_io_read8:
  mov %di, %dx
  in %dx, %al
  ret
   
pc_io_read16:
  mov %di, %dx
  in %dx, %ax
  ret

pc_io_write8:
  mov %di, %dx
  mov %si, %ax
  out %al, %dx
  ret

pc_io_write16:
  mov %di, %dx
  mov %si, %ax
  out %ax, %dx
  ret

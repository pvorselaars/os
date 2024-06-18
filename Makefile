OBJDIR = obj

V       = @

CC      = i386-elf-gcc
LD      = i386-elf-ld
OBJDUMP = i386-elf-objdump
OBJCOPY = i386-elf-objcopy

CFLAGS  = -Iinclude

os.img: obj/boot/boot
	dd if=/dev/zero of=os.img count=2000
	dd if=obj/boot/boot of=os.img count=1 conv=notrunc

include boot/boot.mk

qemu: os.img
	qemu-system-i386 -drive file=os.img,format=raw

qemu-gdb: os.img
	qemu-system-i386 -drive file=os.img,format=raw -s -S

clean:
	rm -rf obj os.img

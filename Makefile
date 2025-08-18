CFLAGS = -Wall -m64 -s  -pedantic \
                        -ffreestanding \
                        -nostdlib \
                        -fcf-protection=none \
                        -fno-ident \
                        -fno-stack-protector \
                        -fno-unwind-tables \
                        -fno-asynchronous-unwind-tables \
                        -fdata-sections \
                        -fno-builtin \
                        -std=c2x \
						-g

LFLAGS = --gc-sections --no-relax

gdb: boot.bin
	gdb -ex "target remote | qemu-system-x86_64 -gdb stdio -S -bios boot.bin" -ex "set confirm off" -ex "add-symbol-file boot.elf 0xF0000" -ex "file boot.elf" 

run: os.img
	qemu-system-x86_64 -bios boot.bin

os.img: boot.bin kernel.bin
	dd if=/dev/zero of=os.img bs=1024 count=128
	dd if=boot.bin of=os.img conv=notrunc
	dd if=kernel.bin of=os.img conv=notrunc seek=1

boot.bin: boot.o
	ld -Ttext=0x0 $(LFLAGS) -o boot.elf boot.o
	objcopy -S -O binary -j .text boot.elf boot.bin

kernel.bin: kernel.o memory.o io.o console.o string.o utils.o interrupt.o disk.o
	ld -Tkernel.ld $(LFLAGS) -o kernel.elf $^
	objcopy -S -O binary kernel.elf kernel.bin

%.o: %.c %.S %.h
	cc $(CFLAGS) -c $*.c -o $*.c.o
	cc $(CFLAGS) -c $*.S -o $*.S.o
	ld -r $*.c.o $*.S.o -o $@
	@rm $*.c.o $*.S.o

%.o: %.S %.h
	cc $(CFLAGS) -c $<

clean:
	rm -f *.o *.l *.elf *.img *.bin

.PHONY: clean
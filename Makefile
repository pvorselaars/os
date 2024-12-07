CFLAGS = -Wall -m64 -s -ffreestanding -nostdlib -fcf-protection=none -fno-ident -fno-stack-protector -fno-asynchronous-unwind-tables -g

run: os.img
	qemu-system-x86_64 -drive format=raw,file=os.img

gdb: os.img
	gdb -ex "target remote | qemu-system-x86_64 -gdb stdio -S -drive format=raw,file=os.img"

os.img: boot.bin kernel.bin
	dd if=/dev/zero of=os.img bs=1024 count=20
	dd if=boot.bin of=os.img conv=notrunc
	dd if=kernel.bin of=os.img conv=notrunc seek=1

boot.bin: boot.o
	ld -Ttext=0x7c00 -o boot.elf boot.o
	objcopy -S -O binary -j .text boot.elf boot.bin

kernel.bin: kernel.o memory.o
	ld -Tkernel.ld -o kernel.elf $^
	objcopy -S -O binary kernel.elf kernel.bin

%.o: %.c
	cc $(CFLAGS) -Wa,-alh=$^.l -c $^

%.o: %.S
	cc $(CFLAGS) -Wa,-alh=$^.l -c $^

%.o: %.s
	cc $(CFLAGS) -Wa,-alh=$^.l -c $^

clean:
	rm -f *.o *.l *.elf *.img *.bin

.PHONY: clean

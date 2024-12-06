CFLAGS = -Wall -m64 -s -ffreestanding -nostdlib -fcf-protection=none -fno-ident -fno-stack-protector -fno-asynchronous-unwind-tables -Wl,--omit-stack-protector

run: os.img
	qemu-system-x86_64 -drive format=raw,file=os.img

gdb: os.img
	qemu-system-x86_64 -s -S -drive format=raw,file=os.img&
	gdb -ex "target remote :1234"

os.img: boot.bin kernel.bin
	dd if=/dev/zero of=os.img bs=1024 count=20
	dd if=boot.bin of=os.img conv=notrunc
	dd if=kernel.bin of=os.img conv=notrunc seek=1

boot.bin: boot.o
	ld -Ttext=0x7c00 -o boot.elf boot.o
	objcopy -S -O binary -j .text boot.elf boot.bin

boot.o: boot.S
	cc $(CFLAGS) -Wa,-alh=$^.l -c $^

kernel.bin: kernel.o
	ld -Ttext=0x100000 -e kernel -o kernel.elf $^
	objcopy -S -O binary -j .text kernel.elf kernel.bin

%.o: %.c
	cc $(CFLAGS) -Wa,-alh=$^.l -c $^

clean:
	rm -f *.o *.l *.elf *.img *.bin

.PHONY: clean

run: os.img
	qemu-system-x86_64 -drive format=raw,file=os.img

gdb: os.img
	qemu-system-x86_64 -s -S -drive format=raw,file=os.img&
	gdb -ex "target remote localhost:1234" -ex "file boot.elf"

os.img: boot.bin
	dd if=/dev/zero of=os.img bs=512 count=2
	dd if=boot.bin of=os.img bs=512 conv=notrunc

boot.bin: boot.o boot.ld
	ld -T boot.ld -o boot.elf boot.o
	objdump -S boot.elf > boot.l
	objcopy -S -O binary -j .text boot.elf boot.bin

boot.o: boot.S
	cc -Wall -g -c $^

clean:
	rm -f *.o *.l *.elf *.img *.bin

.PHONY: clean

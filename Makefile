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
						-Iinclude \
						-g

LFLAGS = --gc-sections --no-relax

QEMU = qemu-system-x86_64 \
						-nodefaults \
						-bios bin/boot \
						-M pc \
						-cpu qemu64 \
						-m 64M \
						#-vga std \
						-rtc base=localtime \
						-parallel file:lpt.log \
						-fda os.img \
						-netdev user,id=net0 \
						-device ne2k_pci,netdev=net0 \
						-no-acpi \

gdb: bin/boot
	gdb -ex "target remote | $(QEMU) -gdb stdio -S " -ex "set confirm off" -ex "add-symbol-file bin/boot.elf 0xF0000" -ex "file bin/boot.elf" 

run: os.img
	$(QEMU)

os.img: bin/kernel
	dd if=/dev/zero of=os.img bs=1024 count=128
	dd if=bin/kernel of=os.img conv=notrunc

bin/boot: obj/boot.o | dir
	ld -Ttext=0x0 $(LFLAGS) -o bin/boot.elf $<
	objcopy -S -O binary -j .text bin/boot.elf bin/boot

bin/kernel: obj/kernel.o obj/memory.o obj/io.o obj/console.o obj/string.o obj/utils.o obj/interrupt.o obj/disk.o
	ld -Tkernel.ld $(LFLAGS) -o bin/kernel.elf $^
	objcopy -S -O binary bin/kernel.elf bin/kernel

obj/%.o: src/%.c src/%.S include/%.h | dir
	cc $(CFLAGS) -c src/$*.c -o obj/$*.c.o
	cc $(CFLAGS) -c src/$*.S -o obj/$*.S.o
	ld -r obj/$*.c.o obj/$*.S.o -o $@
	@rm obj/$*.c.o obj/$*.S.o

obj/%.o: src/%.c | dir
	cc $(CFLAGS) -c $< -o obj/$*.o

obj/%.o: src/%.S | dir
	cc $(CFLAGS) -c $< -o $@

dir:
	@mkdir -p obj bin

clean:
	rm -rf obj/ bin/ *.l *.img

.PHONY: clean
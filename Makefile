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

LFLAGS = --no-relax

QEMU = qemu-system-x86_64 \
						-nodefaults \
						-bios bin/os \
						-M pc \
						-cpu qemu64 \
						-m 64M \
						#-audiodev pa,id=speaker -machine pcspk-audiodev=speaker \
						-vga std \
						-rtc base=localtime \
						-parallel file:lpt.log \
						-fda os.img \
						-netdev user,id=net0 \
						-device ne2k_pci,netdev=net0 \
						-no-acpi \

gdb: bin/os
	gdb bin/os.elf -q -ex "target remote | $(QEMU) -d int,cpu_reset -gdb stdio -S"

run: bin/os
	$(QEMU)

bin/os: obj/boot.o obj/kernel.o obj/memory.o obj/utils.o obj/string.o obj/console.o obj/io.o obj/interrupt.o | dir
	ld -Tlink.ld $(LFLAGS) -o bin/os.elf $^
	objcopy -X -O binary bin/os.elf $@

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
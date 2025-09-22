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
						-Werror \
						-Wno-error=unused-variable \
						-Wno-error=unused-but-set-variable \
						-g

LFLAGS = --no-relax

QEMU = qemu-system-x86_64 \
						-monitor telnet:127.0.01:1234,server,nowait\
						-nodefaults \
						-machine acpi=off \
						-bios bin/os \
						-M isapc \
						-cpu qemu64,-apic,-x2apic,+pdpe1gb \
						-m 2M \
						-audiodev pa,id=speaker -machine pcspk-audiodev=speaker \
						-serial stdio \
						-parallel file:lpt.log \
						-vga std \
						#-rtc base=localtime \
						-fda os.img \
						-netdev user,id=net0 \
						-device ne2k_pci,netdev=net0 \

SRC := $(wildcard src/*.c src/*.S)
OBJ := $(patsubst src/%,obj/%,$(SRC:.c=.o))
OBJ := $(OBJ:.S=.o)

gdb: bin/os
	tmux new-session -d -s os
	tmux send-keys -t os "$(QEMU) -S -d cpu_reset,int,guest_errors -no-reboot -gdb tcp::1235 " Enter
	tmux new-window -t os
	tmux send-keys -t os "gdb bin/os.elf -q -ex 'target remote :1235'" Enter
	tmux attach-session -t os

stop:
	tmux kill-session -t os

run: bin/os
	$(QEMU)

bin/os: $(OBJ) | link.ld dir
	ld -Tlink.ld $(LFLAGS) -o bin/os.elf $^
	objdump -d bin/os.elf > os.l
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
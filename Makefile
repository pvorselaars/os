ARCH ?= x86_64
BOARD ?= pc

ifeq ($(ARCH),x86_64)
    CC := gcc
    ARCH_CFLAGS := -m64
    ARCH_LFLAGS := 
    VALID_BOARDS := pc
endif

ifeq ($(ARCH),aarch64)
    CC := aarch64-linux-gnu-gcc
    ARCH_CFLAGS := -march=armv8-a+crc -mtune=cortex-a72
    ARCH_LFLAGS := 
    VALID_BOARDS := rpi4
endif

ifeq ($(filter $(BOARD),$(VALID_BOARDS)),)
    $(error Invalid BOARD=$(BOARD) for ARCH=$(ARCH). Valid boards: $(VALID_BOARDS))
endif

CFLAGS = -Wall -s -pedantic \
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
						$(ARCH_CFLAGS) \
						-Iinclude \
						-Iinclude/arch/$(ARCH) \
						-Iinclude/board/$(BOARD) \
						-Werror \
						-Wno-error=unused-variable \
						-Wno-error=unused-but-set-variable \
						-g

LFLAGS = --no-relax $(ARCH_LFLAGS)

ifeq ($(BOARD),pc)
QEMU = qemu-system-x86_64 \
						-monitor telnet:127.0.01:1234,server,nowait\
						-nodefaults \
						-machine acpi=off \
						-drive file=bin/os,format=raw \
						-M isapc \
						-cpu qemu64,-apic,-x2apic,+pdpe1gb \
						-m 2M \
						-audiodev pa,id=speaker -machine pcspk-audiodev=speaker \
						-serial stdio \
						-parallel file:lpt.log \
						-vga std
endif

ifeq ($(BOARD),rpi4)
QEMU = qemu-system-aarch64 \
						-machine raspi4b \
						-cpu cortex-a72 \
						-smp 4 \
						-m 1G \
						-kernel bin/os \
						-serial stdio \
						-display none
endif

SRC_C := $(wildcard kernel/*.c lib/*.c drivers/*/*.c arch/$(ARCH)/*.c arch/$(ARCH)/internal/*.c board/$(BOARD)/*.c tests/tests.c)
SRC_S := $(wildcard kernel/*.s lib/*.s drivers/*/*.s arch/$(ARCH)/*.s arch/$(ARCH)/internal/*.s board/$(BOARD)/*.s)

OBJ_C := $(patsubst %.c,obj/%.o,$(SRC_C))
OBJ_S := $(patsubst %.s,obj/%.s.o,$(SRC_S))
OBJ := $(OBJ_C) $(OBJ_S)

gdb: bin/os
	tmux new-session -d -s os
	tmux send-keys -t os "$(QEMU) -S -d cpu_reset,int,guest_errors -no-reboot -gdb tcp::1235 " Enter
	tmux split-window -h -t os
	tmux send-keys -t os "gdb bin/os.elf -q -ex 'target remote :1235'" Enter
	tmux attach-session -t os

stop:
	tmux kill-session -t os

run: bin/os
	$(QEMU)

bin/os: $(OBJ) | board/$(BOARD)/link.ld dir
	ld -Tboard/$(BOARD)/link.ld $(LFLAGS) -o bin/os.elf $^
	objdump -d bin/os.elf > os.l
	objcopy -X -O binary --only-section=.boot bin/os.elf bin/boot.bin
	objcopy -X -O binary --remove-section=.boot bin/os.elf bin/kernel.bin
	cat bin/boot.bin bin/kernel.bin > $@

obj/%.o: %.c | dir
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

obj/%.s.o: %.s | dir
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@

dir:
	@mkdir -p obj bin
	@mkdir -p obj/kernel obj/lib obj/drivers obj/arch/$(ARCH) obj/arch/$(ARCH)/internal obj/board/$(BOARD)

TEST_CC := gcc
TEST_CFLAGS := -g -std=c2x -Iinclude 

TEST_SRCS := $(wildcard tests/arch/$(ARCH)/*.c)
TEST_BINS := $(patsubst tests/%.c,tests/bin/%,$(TEST_SRCS))

tests/bin:
	@mkdir -p tests/bin

tests/bin/%: tests/%.c | tests/bin
	@mkdir -p $(dir $@)
	@$(TEST_CC) $(TEST_CFLAGS) -o $@ $< tests/tests.c \
		$(if $(wildcard $(patsubst tests/arch/$(ARCH)/%.c,arch/$(ARCH)/%.s,$<)),$(patsubst tests/arch/$(ARCH)/%.c,arch/$(ARCH)/%.s,$<))

.PHONY: test
test: $(TEST_BINS)
	@for t in $(TEST_BINS); do \
		./$$t ; \
	done; \

pc:
	$(MAKE) ARCH=x86_64 BOARD=pc

rpi4:
	$(MAKE) ARCH=aarch64 BOARD=rpi4


clean:
	rm -rf obj/ bin/ *.l

.PHONY: clean info pc rpi4
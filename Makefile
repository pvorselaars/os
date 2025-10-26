PLATFORM ?= pc
ARCH ?= x86_64

ifeq ($(PLATFORM),pc)
    ARCH := x86_64
    CC := gcc
    PLATFORM_CFLAGS := -m64
    PLATFORM_LFLAGS := 
endif

ifeq ($(PLATFORM),raspberrypi4)
    ARCH := aarch64
    CC := aarch64-linux-gnu-gcc
    PLATFORM_CFLAGS := -march=armv8-a+crc -mtune=cortex-a72
    PLATFORM_LFLAGS := 
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
						$(PLATFORM_CFLAGS) \
						-Iinclude \
						-Iinclude/kernel \
						-Iinclude/lib \
						-Iinclude/drivers \
						-Iinclude/arch/$(ARCH) \
						-Iinclude/platform/$(PLATFORM) \
						-Werror \
						-Wno-error=unused-variable \
						-Wno-error=unused-but-set-variable \
						-g

LFLAGS = --no-relax $(PLATFORM_LFLAGS)

ifeq ($(PLATFORM),pc)
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

ifeq ($(PLATFORM),raspberrypi4)
QEMU = qemu-system-aarch64 \
						-machine raspi4b \
						-cpu cortex-a72 \
						-smp 4 \
						-m 1G \
						-kernel bin/os \
						-serial stdio \
						-display none
endif

SRC_KERNEL_C := $(wildcard kernel/*.c)
SRC_KERNEL_S := $(wildcard kernel/*.s)
SRC_LIB_C := $(wildcard lib/*.c)
SRC_LIB_S := $(wildcard lib/*.s)
SRC_DRIVERS_C := $(wildcard drivers/*/*.c)
SRC_DRIVERS_S := $(wildcard drivers/*/*.s)
SRC_ARCH_C := $(wildcard arch/$(ARCH)/*.c)
SRC_ARCH_S := $(wildcard arch/$(ARCH)/*.s)
SRC_ARCH_INTERNAL_C := $(wildcard arch/$(ARCH)/internal/*.c)
SRC_ARCH_INTERNAL_S := $(wildcard arch/$(ARCH)/internal/*.s)
SRC_PLATFORM_C := $(wildcard platform/$(PLATFORM)/*.c)
SRC_PLATFORM_S := $(wildcard platform/$(PLATFORM)/*.s)

OBJ_KERNEL_C := $(patsubst kernel/%.c,obj/kernel/%.o,$(SRC_KERNEL_C))
OBJ_KERNEL_S := $(patsubst kernel/%.s,obj/kernel/%.s.o,$(SRC_KERNEL_S))
OBJ_LIB_C := $(patsubst lib/%.c,obj/lib/%.o,$(SRC_LIB_C))
OBJ_LIB_S := $(patsubst lib/%.s,obj/lib/%.s.o,$(SRC_LIB_S))
OBJ_DRIVERS_C := $(patsubst drivers/%.c,obj/drivers/%.o,$(SRC_DRIVERS_C))
OBJ_DRIVERS_S := $(patsubst drivers/%.s,obj/drivers/%.s.o,$(SRC_DRIVERS_S))
OBJ_ARCH_C := $(patsubst arch/$(ARCH)/%.c,obj/arch/$(ARCH)/%.o,$(SRC_ARCH_C))
OBJ_ARCH_S := $(patsubst arch/$(ARCH)/%.s,obj/arch/$(ARCH)/%.s.o,$(SRC_ARCH_S))
OBJ_ARCH_INTERNAL_C := $(patsubst arch/$(ARCH)/internal/%.c,obj/arch/$(ARCH)/internal/%.o,$(SRC_ARCH_INTERNAL_C))
OBJ_ARCH_INTERNAL_S := $(patsubst arch/$(ARCH)/internal/%.s,obj/arch/$(ARCH)/internal/%.s.o,$(SRC_ARCH_INTERNAL_S))
OBJ_PLATFORM_C := $(patsubst platform/$(PLATFORM)/%.c,obj/platform/$(PLATFORM)/%.o,$(SRC_PLATFORM_C))
OBJ_PLATFORM_S := $(patsubst platform/$(PLATFORM)/%.s,obj/platform/$(PLATFORM)/%.s.o,$(SRC_PLATFORM_S))

OBJ := $(OBJ_KERNEL_C) $(OBJ_KERNEL_S) $(OBJ_LIB_C) $(OBJ_LIB_S) $(OBJ_DRIVERS_C) $(OBJ_DRIVERS_S) $(OBJ_ARCH_C) $(OBJ_ARCH_S) $(OBJ_ARCH_INTERNAL_C) $(OBJ_ARCH_INTERNAL_S) $(OBJ_PLATFORM_C) $(OBJ_PLATFORM_S)

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

bin/os: $(OBJ) | platform/$(PLATFORM)/link.ld dir
	ld -Tplatform/$(PLATFORM)/link.ld $(LFLAGS) -o bin/os.elf $^
	objdump -d bin/os.elf > os.l
	objcopy -X -O binary --only-section=.boot bin/os.elf bin/boot.bin
	objcopy -X -O binary --remove-section=.boot bin/os.elf bin/kernel.bin
	cat bin/boot.bin bin/kernel.bin > $@

obj/kernel/%.o: kernel/%.c | dir
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

obj/kernel/%.s.o: kernel/%.s | dir
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@

obj/lib/%.o: lib/%.c | dir
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

obj/lib/%.s.o: lib/%.s | dir
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@

obj/drivers/%.o: drivers/%.c | dir
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

obj/drivers/%.s.o: drivers/%.s | dir
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@

obj/arch/$(ARCH)/%.o: arch/$(ARCH)/%.c | dir
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

obj/arch/$(ARCH)/%.s.o: arch/$(ARCH)/%.s | dir
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@

obj/arch/$(ARCH)/internal/%.o: arch/$(ARCH)/internal/%.c | dir
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

obj/arch/$(ARCH)/internal/%.s.o: arch/$(ARCH)/internal/%.s | dir
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@

obj/platform/$(PLATFORM)/%.o: platform/$(PLATFORM)/%.c | dir
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

obj/platform/$(PLATFORM)/%.s.o: platform/$(PLATFORM)/%.s | dir
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@

dir:
	@mkdir -p obj bin
	@mkdir -p obj/kernel obj/lib obj/arch/$(ARCH) obj/arch/$(ARCH)/internal obj/platform/$(PLATFORM)

pc:
	$(MAKE) PLATFORM=pc

raspberrypi4:
	$(MAKE) PLATFORM=raspberrypi4

clean:
	rm -rf obj/ bin/ *.l

.PHONY: clean info pc raspberrypi4
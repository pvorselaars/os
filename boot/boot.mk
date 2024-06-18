OBJDIRS += boot

BOOT_OBJS = $(OBJDIR)/boot/boot.o

$(OBJDIR)/boot/%.o: boot/%.S
	@echo + as $<
	@mkdir -p $(@D)
	$(V)$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/boot/boot: $(BOOT_OBJS) boot/boot.ld
	@echo + ld boot/boot
	$(V)$(LD) -T boot/boot.ld -o $@.out $(BOOT_OBJS)
	$(V)$(OBJDUMP) -S $@.out > $@.asm
	$(V)$(OBJCOPY) -S -O binary -j .text $@.out $@

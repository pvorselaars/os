ENTRY(kernel)

BOOT_BASE    = 0x7c00;
KERNEL_PHYS  = 0x10000;
KERNEL_BASE  = 0xFFFFFF8000000000;
KERNEL_VMA   = KERNEL_BASE + KERNEL_PHYS;

SECTIONS
{
    .boot BOOT_BASE : {
        obj/board/pc/boot.s.o(.text)
		BOOT_END = .;
		BOOT_GDT_PHYS = BOOT_BASE + 0x108;  /* GDT physical address */
		BOOT_GDT_VIRT = KERNEL_BASE + BOOT_GDT_PHYS;  /* GDT virtual address */
    }

    .text KERNEL_VMA : AT(KERNEL_PHYS) {
        kernel_start = .;
        *(.text)
    }

    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }

    .data : {
        *(.data)
        *(.data.*)
    }

    .got : {
        *(.got)
    }

    .got.plt : {
        *(.got.plt)
    }
    
    KERNEL_LOAD_END = .;  /* End of sections that need to be loaded from disk */

    .bss : {
        *(.bss)
        *(.bss.*)
    }

    KERNEL_END = .;
    
    /* Calculate kernel size in bytes and sectors (only loaded sections) */
    KERNEL_SIZE = KERNEL_LOAD_END - kernel_start;
    KERNEL_SECTORS = (KERNEL_SIZE + 511) / 512;  /* Round up to next sector */

    /DISCARD/ : {        *(.note.gnu.property)
        *(.gnu.property)
        *(.note.gnu.build-id)
        *(.gnu.hash)
        *(.comment)
        *(.eh_frame)
        *(.eh_frame_hdr)
    }

    .debug_info     0 : { *(.debug_info) }
    .debug_abbrev   0 : { *(.debug_abbrev) }
    .debug_line     0 : { *(.debug_line) }
    .debug_str      0 : { *(.debug_str) }
    .debug_ranges   0 : { *(.debug_ranges) }
    .debug_loc      0 : { *(.debug_loc) }
    .debug_frame    0 : { *(.debug_frame) }
    .debug_aranges  0 : { *(.debug_aranges) }
}

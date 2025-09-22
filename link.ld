ENTRY(reset_vector)

RESET_VECTOR = 0xFFFF0;
BOOT_BASE    = 0xF0000;
KERNEL_VMA   = 0xFFFFFF8000000000 + BOOT_END;

SECTIONS
{
    .reset RESET_VECTOR : {
        *(.reset)
		. = ALIGN(16);
    } = 0x0

    .boot BOOT_BASE : {
        obj/boot.o(.text)
		BOOT_END = .;
    }

    .text KERNEL_VMA : AT(BOOT_END) {
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

    .bss : {
        *(.bss)
        *(.bss.*)
    }

    KERNEL_END = .;

    /DISCARD/ : {
        *(.note.gnu.property)
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

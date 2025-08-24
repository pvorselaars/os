ENTRY(_start)
SECTIONS {

	. = 0xF0000;

	.text : {
		obj/boot.o(.text)
		*(.text)
	}

	.data : {
		*(.data)
	}

	.rodata : {
		*(.rodata)
	}
	
	/* Do not exceed 64KB - 16 */
	ASSERT(. <= 0xffff0, "code section too large")

	. = 0xffff0;
	.reset : {
		*(.reset)
		. = ALIGN(16);
	} = 0x0

	.bss : {
		*(.bss)
	}
	
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

ENTRY(_start)
SECTIONS {

	. = 0xF0000;

	BEGIN = .;

	.text : {
		obj/boot.o(.text)
		*(.text)
	}

	.data : {
		*(.data)
		*(.bss)
	}

	.rodata : {
		*(.rodata)
	}

	END = .;

	. += 0xfff0 - (END - BEGIN);
	.reset : {
		*(.reset)
		. = ALIGN(16);
	} = 0x0
	
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

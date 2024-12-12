ENTRY(kernel)
SECTIONS {
	. = 0xffff800000010000;
	PROVIDE(KERNEL_START = .);

	.text : {
		*(.text)
	}

	.data : {
		*(.data)
	}

	.rodata : {
		*(.rodata)
	}

	.bss : {
		*(.bss)
	}

	PROVIDE(KERNEL_END = .);

}

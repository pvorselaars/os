ENTRY(kernel)
SECTIONS {
	. = 0xffff800000010000;

	__KERNEL_START = .;
	.text : {
		*(.text)
	}

	.data : {
		*(.data)
		*(.bss)
	}

	.rodata : {
		*(.rodata)
	}

	__KERNEL_END = .;

}

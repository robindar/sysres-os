ENTRY(_start)

SECTIONS
{
	/* Starts at LOADER_ADDR. */
	. = 0x00000000;

	__start = .;
	.text ALIGN(0x1000) : AT(ADDR(.text) - 0x00000000)
	{
		KEEP(*(.text.boot))
			*(.text)
			*(.rodata)
	}

	.data ALIGN(0x1000) : AT(ADDR(.data) - 0x00000000)
	{
		*(.data)
	}

	__bss_start = .;
	.bss : AT(ADDR(.bss) - 0x00000000)
	{
		bss = .;
		*(.bss)
	}
	__bss_end = .;
	. = ALIGN(0x200000);
	__TTBR0_EL1_start = .;
	. = . + 0x8000; /* 512 * 64 bits */
	. = ALIGN(0x8000);
	__TTBR1_EL1_start = .;  /* Warning this is positioned on the lvl3 tables ?? */
	. = . + 0x8000; /* 512 * 64 bits */
	. = ALIGN(0x8000);
	__end = .;
}


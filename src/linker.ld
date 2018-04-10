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
	/* Physical memory map : size 512 * 512 * 4 bytes */
	__physical_memory_map = .;
	. = . + 0x100000;
	. = ALIGN(0x200000); /* Ensure bits [20:0] are 0 (cf. ARM ARM 2727) */
	__TTBR0_EL1_start = .;
	. = . + 0x1000; /* 512 * 8 bytes */
	. = ALIGN(0x1000);
	__TTBR1_EL1_start = .;
	. = . + 0x1000; /* 512 * 8 bytes */
	. = ALIGN(0x1000);
	/* LVL3 tables for TTBR0 : size 512 * 512 * 8 bytes */
	. = . + 0x200000;
	/* Align __end to granule size so it fits in a page */
	. = ALIGN(0x1000);
	__end = .;
}


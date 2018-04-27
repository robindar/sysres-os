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
	. = ALIGN(0x200000); /* Ensure bits [20:0] are 0 (cf. ARM ARM 2727) */
	__mmu_tables_start = .;
        . = . + 0x4000000;  /* see doc/mmu.md  = 32 * 512 * GRANULE (this amounts to 10% of available memory)*/
        __mmu_tables_end   = .;
	__physical_memory_map = .;
	. = . + 0x100000;
	__end = .;
}


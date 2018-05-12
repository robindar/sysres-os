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
    }
    /* Alignement constraint */
    . = ALIGN(0x1000);
    /* Outputs an error if the kernel oversteps 0x8000 */
    .rodata 0x8000 : AT(ADDR(.rodata) - 0x00000000)
    {
        __data_start = .;
        /* TODO : put this as read-only */
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
    . = ALIGN(0x1000);
    __bss_end = .;

    . = ALIGN(0x200000); /* Ensure bits [20:0] are 0 (cf. ARM ARM 2727) */
    __mmu_tables_start = .;
    . = . + 0x4000000;  /* see doc/mmu.md  = 32 * 512 * GRANULE (this amounts to 10% of available memory)*/
    __mmu_tables_end   = .;

    __physical_memory_map = .;
    . = . + 0x100000;

    /* Virtual page Bind Counter
     * requires (512 * 512 - n) bytes
     * where n is the size (in pages) of the kernel,
     * i.e. (__end / 0x1000)
     * known to be at least 0x4400
     */
    . = . + 0x40000; /* (0d512 * 0d512 - 0x4400) = 0x3bc00 */

    . = ALIGN(0x200000); /* should be superfluous */
    __end = .;
}


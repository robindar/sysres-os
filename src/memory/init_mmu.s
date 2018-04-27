.globl init_mmu
init_mmu:
	// Enable MMU
	ldr X3, = __mmu_tables_start ;
	msr TTBR0_EL1, X3 // Set TTBR0
        // mov X3, XZR
	// msr TTBR1_EL1, X3 // Set TTBR1 -> deprecated
	// Set TCR
	mrs X27, TCR_EL1
	//orr X3, X3, #(1 << 38) // TBI1 -> Top byte ignored in address calculation
	//orr X3, X3, #(1 << 37) // TBI0 -> idem (cf. ARM ARM : 2129)
	orr X3, X3, #(1 << 36) // AS   -> 16 bits of ASID
	orr X3, X3, #(1 << 31) // TG1  -> 10 : granule size at 4KB
	orr X3, X3, #(1 <<  1) // T0SZ -> 0-5   : 34 = 0b100010
	orr X3, X3, #(1 <<  5)
	orr X3, X3, #(1 << 17) // T1SZ -> 16-21 : 34 = 0b100010
	orr X3, X3, #(1 << 21)
	msr TCR_El1, X3
	// Instruction Synchronization Barrier:
	//  forces the previous changes to be seen
	//  before enabling the MMU
	isb

	mov X28, X30 // Preserve caller-saved link register
	bl uart_init
        mov X0, XZR  //PID to pass as first arg to c_init_mmu
	bl c_init_mmu
	mov X30, X28 // Restore link register

	mrs X3, SCTLR_EL1 // Read  System Control Register configuration data
        mov X0, X3
	orr X3, X3, #1    // Set [M] bit and enable the MMU
	msr SCTLR_EL1, X3 // Write System Control Register configuration data
	// Instruction Synchronization Barrier:
	isb

	ret

// To keep this in the first portion of the binary.
.section ".text.boot"

// Make _start global.
.globl _start

 //Exception level switch : from secure to kernel mode
.globl switch_from_EL3_to_EL1

// Entry point for the kernel.
// r15 -> should begin execution at 0x8000.
// r0 -> 0x00000000
// r1 -> 0x00000C42
// r2 -> 0x00000100 - start of ATAGS
// preserve these registers as argument for kernel_main
_start:
	// Setup the stack.
	mov sp, #0x8000

	// Clear out bss.
	ldr X4, =__bss_start
	ldr X9, =__bss_end
	mov X5, #0
	mov X6, #0
	mov X7, #0
	mov X8, #0
	b       2f

1:
	// store multiple at r4.
	stp X5, X6, [X4], #16
	stp X7, X8, [X4], #16

	// If we are still below bss_end, loop.
2:
	cmp X4, X9
	blo 1b

	//Turn off cpu1,2,3
	//Get cpu id in r3
	//mrc p15,0,r3,c0,c0,5
	mrs X3, MPIDR_EL1
	//Mask
	and X3, X3, #0xFF
	//Compare
	cmp X3, #0
	//Jump if non-zero
	bne halt

	//Tell the system where the Interrupt Tables are
	ldr X3, =Vector_table_el1
	msr VBAR_EL1, X3
	ldr X3, =Vector_table_el2
	msr VBAR_EL2, X3
	ldr X3, =Vector_table_el3
	msr VBAR_EL3, X3

	//Switch from EL3 (secure) to EL1 (kernel)
	bl switch_from_EL3_to_EL1
	//Set up stack again
	mov sp, #0x8000

	// Enable MMU
	//msr TTBR0_EL1, X0 // Set TTBR0
	//msr TTBR1_EL1, X1 // Set TTBR1
	//msr TCR_El1, X2   // Set TCR
	// Instruction Synchronization Barrier:
	//  forces the previous changes to be seen
	//  before enabling the MMU
	//isb

	//mrs X0, SCTLR_EL1 // Read  System Control Register configuration data
	//orr X0, X0, #1    // Set [M] bit and enable the MMU
	//msr SCTLR_EL1, X0 // Write System Control Register configuration data
	//isb

	mov X0, #0
	b kernel_main

	//halt
.globl halt
halt:
	wfe
	b halt


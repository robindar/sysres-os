// To keep this in the first portion of the binary.
.section ".text.boot"

// Make _start global.
.globl _start

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


	b kernel_main

	//halt
halt:
	wfe
	b halt


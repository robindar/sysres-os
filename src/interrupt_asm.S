//See https://community.arm.com/processors/f/discussions/6970/confusion-about-exception-level-of-armv8 for general principle
//See ARM architecure manual for details about registers config
.global switch_from_EL3_to_EL1
switch_from_EL3_to_EL1:
        //Enable SIMD instructions in EL1
        mrs X3, CPACR_EL1
        orr X3, X3, #(1 << 20)
        orr X3, X3, #(1 << 21)
        msr CPACR_EL1, X3

        msr sctlr_el1, xzr //Disable MMU in el1

        mov x3, xzr //We configure lower level scurity setting
        orr x3, x3, #(1 << 10) //Lower level in Aarch64
        msr scr_el3, x3 //Default : secure mode

        //Set state
        mov x3, xzr
        //Default : all execptions are not masked
        //We mask them for debugging purposes first
        orr x3, x3, #(1 << 6)
        orr x3, x3, #(1 << 7)
        orr x3, x3, #(1 << 8)
        orr x3, x3, #(1 << 9)
        //Default : Aarch64 state
        // Set el1h, ie el1 with its own stack pointer
        orr x3, x3, #(1 << 2) //el1
        orr x3, x3, #1 //own stack pointer
        msr spsr_el3, x3

        //We assume desired return adress is in x30 = Link Register (LR)
        //So a simple BL is enough to call this subroutine
        //adrp x3, kernel_main
        //add x3, x3, :lo12:kernel_main
        //bl uart_simple_put_reg
        msr elr_el3, x30
        eret

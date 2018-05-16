## Solved issues ##
# Procs #
- Stack page was allocated using KERNEL permission even for user processes
- Data and bss were mapped using KERNEL permission while we need something similar to DEVICE : ie RW at every level

-Issue with the contiguous bit:
       - a page accessed at EL0 was causing a permission fault even though the permissions were correct
       - a freed page in the identity mapped zone was accessed again without fault
 -> Cause : the contiguousBit which was = 1 in the kernel -> these pqges were reprsented by the first of their group of 16

- GDB not able to read usr code in usr mode
- Free has to executed at EL1
- GCC putting all non static global var at 0x0...

# BCM2837 datatsheet #

https://github.com/raspberrypi/documentation/issues/325#issuecomment-379651504

# Clock interrupts #

TIMER VALUE is documented as RO but must be initialized (no clue what LOAD is for)

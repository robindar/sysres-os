## Solved issues ##
# Procs #
- Stack page was allocated using KERNEL permission even for user processes
- Data and bss were mapped using KERNEL permission while we need something similar to DEVICE : ie RW at every level


# BSS Shit #

Initialize bss segment ONE FUCKING TIME, not one per core

# MMU Conventions #

No blocks on level 2, only tables allowed

WARNING: STAY in fcking SECURE EL !!!! (cf. ARM ARM : 2092)

INFO : SG1 means "Stage 1"

Only accessible EL are Secure. All decodings thus occur in EL1 Secure and EL0 Secure
Stage 2 is not accessible from Secure EL.

INFO : Output address size is 32 bit (4GB) (cf. ARM ARM : 2098)

INFO : Selection between TTBR0 and TTBR1 is done by looking at bits [63:48] of VA

INFO : All pages are non-global and non-dirty by default

# /!\ x definition from ARM ARM /!\ #

Lookup level  |  4KB granule size
--------------+--------------------
Zero          |  IA[47:39] , x = 39
First         |  IA[38:30] , x = 30
Second        |  IA[29:21] , x = 21
Third         |  IA[20:12] , x = 12

cf. ARM ARM : 2108

# Previous notes #

max 48bits for 4KB

Translation table format descriptors : 2144 (Level 0,1,2)
                                       2147 (Level 3)

Attribute fields description : 2150

# Addressing Stuff #

kernel (up to __end) is mapped to identity
uart is paged to identity (GPIO BASE etc)

No guaranties for other pages (see get unbound physical page)

# Shareablity attributes #
The meaning of Inner/Outer shareable (in clock descirptor attributes) is controled by the Cache Level ID register. See ARM ARM 2405
Inner shareable stops somewhere between L1 and L7 caches, includes all cores.
For now inner-shareable shoudl be fine, but maybe to change for multicore ? -> to change to inner

# Caching #
For I/O memory, we use Device policy as writes have side effects (ARM ARM 118) with nGnRnE (ie no optimizations) => this corresponds to an attribute of 0b00000000
For normal memory (ARM ARM 116), write-through cachable is simpler as a write is performed immediatly in the main memory, so we avoid data sync issues -> to change later ?
We use non transient, read-allocate, write-allocate by default but we'll save other profiles in MAIR for later anyway

(se ARM ARM 2610 for the encoding)

Content of MAIR_EL1 :

AttrIndex |  Attribute | Meaning
----------+------------+--------------
     0    | 0000 0000  | Device nGnRnE
     1    | 1011 1011  | Normal memory, Inner/Outer Write-Through Non-transient, Read-Allocate, Write-Allocate
     2    | 0011 0011  | Normal memory, Inner/Outer Write-Through     Transient, Read-Allocate, Write-Allocate
     3    | 1111 1111  | Normal memory, Inner/Outer Write-Back    Non-transient, Read-Allocate, Write-Allocate
     4    | 0111 0111  | Normal memory, Inner/Outer Write-Back        Transient, Read-Allocate, Write-Allocate
     5    | 0100 0100  | Normal memory, Non-cacheable
     6    | Undef
     7    | Undef
For table cachning : it is set in init_cache
(for enconding see ARM ARM 2693, for explanation see ARMv8-A Address Tranlsation : 18)
00 Normal memory, Outer Non-cacheable
01 Normal memory, Outer Write-Back Read-Allocate Write-Allocate Cacheable
10 Normal memory, Outer Write-Through Read-Allocate No Write-Allocate Cacheable
11 Normal memory, Outer Write-Back Read-Allocate No Write-Allocate Cacheable

Currently it is 10

# Process tables #
After a few botched attemps (kmalloc, identity physical page -> fails because of alignement constraints of TTBR0 see http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0500d/BABHEIGF.html), I will try the simplest solution : we reserve a zone large enough at the end of the kernel for every procs. However, lvl2 tables has to be 0X200000 = 512 * GRANULE aligned. Thus, to save space, each lvl2 tables will have at most 511 valid lvl2 entries (as we have space for only 511 lvl3 tables) so that we can fit 1 lvl2 table + the associated 511 lvl3 tables in 0x200000. We this we can map until 0x3fe00000 instead of 0x40000000 which is enough (as we don't use memory after GPIO_BASE).
However having 256 slots uses the whole RAM, so we'll stick to 32 active process for now. (1/16 of 1 Gb)


# Permissions #
We were mapping stack and data/bss pages with KERNEL permissions even for usr processes. Actually we need RW at every level for these zones.

# Contiguous Bit #
ARM ARM 2178
Warning dangerous :
        - Contiguous zones must be 16 pages aligned, same perm, cache policy, same invalid.valid...
        - Thus id mapped pages must not be touched after set up !!!
        - As of the atual commit (3e8fc on proc, 3aafd on debug_proc), we have :
                - Beginning of kernel : 0x0
                - Data start          : 0x6000
                - BSS end             : 0xb000
                - Id_paging_size      : 0x4300000
          -> Thus we don't have 16 entries for kernel/data but it is feasible for tables

# Kernel tables #
For simplicity, we allocate for the kernel the whole of the first lvl3_table, ie 512*4ko (more exaclty 511*4ko)

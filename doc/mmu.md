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

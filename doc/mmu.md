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

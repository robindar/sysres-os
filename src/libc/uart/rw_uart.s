.globl GET32
GET32:
    ldr w0, [x0]
    ret

.globl PUT32
PUT32:
    str w1, [x0]
    ret

.globl dummy
dummy:
    ret

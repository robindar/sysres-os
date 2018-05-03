#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

/* Generic function for printing an integer, warning displays nothing for 0 */
int put_uint(uint64_t x, unsigned int base, bool upper_hexa) {
    if(x == 0) {
        return 0;
    }
    uint64_t y = x % base;
    int written = put_uint(x/base, base, upper_hexa);
    if(y <= 9) putc(48 + y, stdout);
    else if(upper_hexa) putc(55 + y, stdout);
    else putc(87 + y, stdout);
    return written + 1;
}


int put_int(int64_t x, unsigned int base, bool unsign, bool upper_hexa) {
    int written = 0;
    if(x == 0) {
        putc('0', stdout);
        written ++;
    }
    else if (x < 0 && !unsign){
        putc('-', stdout);
        written += 1 + put_uint(1 + ~x, base, upper_hexa);
    }
    else written += put_uint(x, base, upper_hexa);
    return written;
}

//Takes input as hexadecimal
int main(int argc, char** argv) {
    uint64_t esr_eln = strtol(argv[1], NULL, 16);
    //Parse ESR_EL1 (see aarch64, exception and interrupt handling)
    uint64_t exception_class = (esr_eln & 0xfc000000) >> 26;
    bool il = (esr_eln & 0x2000000) >> 25;
    uint16_t instr_specific_syndrom = (esr_eln & 0x1fffff);
    printf(
        "ER_ELn info :\r\nIL : %d\r\nException Class : 0b"
         , il);
    put_int(exception_class, 2, 1, 0);
    printf("\r\nInstruction Specific Syndrom : 0b");
    put_int(instr_specific_syndrom, 2, 1, 0);
    putc('\n', stdout);
}

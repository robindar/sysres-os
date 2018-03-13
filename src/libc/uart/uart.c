#include "uart.h"
#include "../misc.h"

/*
 * The following code was originally written
 * by David Welch, and published unlicensed
 *
 * The full source code is available at
 * http://www.github.com/dwelch67/raspberrypi
 */

extern void PUT32 ( unsigned int, unsigned int );
extern unsigned int GET32 ( unsigned int );
extern void dummy ( unsigned int );

#define GPFSEL1         0x3F200004
#define GPSET0          0x3F20001C
#define GPCLR0          0x3F200028
#define GPPUD           0x3F200094
#define GPPUDCLK0	    0x3F200098

#define AUX_ENABLES     0x3F215004
#define AUX_MU_IO_REG   0x3F215040
#define AUX_MU_IER_REG  0x3F215044
#define AUX_MU_IIR_REG  0x3F215048
#define AUX_MU_LCR_REG  0x3F21504C
#define AUX_MU_MCR_REG  0x3F215050
#define AUX_MU_LSR_REG  0x3F215054
#define AUX_MU_MSR_REG  0x3F215058
#define AUX_MU_SCRATCH  0x3F21505C
#define AUX_MU_CNTL_REG 0x3F215060
#define AUX_MU_STAT_REG 0x3F215064
#define AUX_MU_BAUD_REG 0x3F215068

//GPIO14  TXD0 and TXD1
//GPIO15  RXD0 and RXD1
//alt function 5 for uart1
//alt function 0 for uart0

//((250,000,000/115200)/8)-1 = 270

unsigned int uart_recv ( void )
{
	while(1)
	{
		if(GET32(AUX_MU_LSR_REG)&0x01) break;
	}
	return(GET32(AUX_MU_IO_REG)&0xFF);
}

void uart_send ( unsigned int c )
{
	while(1)
	{
		if(GET32(AUX_MU_LSR_REG)&0x20) break;
	}
	PUT32(AUX_MU_IO_REG,c);
}

void uart_init ( void )
{
	unsigned int ra;

	PUT32(AUX_ENABLES,1);
	PUT32(AUX_MU_IER_REG,0);
	PUT32(AUX_MU_CNTL_REG,0);
	PUT32(AUX_MU_LCR_REG,3);
	PUT32(AUX_MU_MCR_REG,0);
	PUT32(AUX_MU_IER_REG,0);
	PUT32(AUX_MU_IIR_REG,0xC6);
	PUT32(AUX_MU_BAUD_REG,270);
	ra=GET32(GPFSEL1);
	ra&=~(7<<12); //gpio14
	ra|=2<<12;    //alt5
	ra&=~(7<<15); //gpio15
	ra|=2<<15;    //alt5
	PUT32(GPFSEL1,ra);
	PUT32(GPPUD,0);
	for(ra=0;ra<150;ra++) dummy(ra);
	PUT32(GPPUDCLK0,(1<<14)|(1<<15));
	for(ra=0;ra<150;ra++) dummy(ra);
	PUT32(GPPUDCLK0,0);
	PUT32(AUX_MU_CNTL_REG,3);
}

/* End of external code */

void uart_putc(unsigned char c)
{
	uart_send(c);
}

unsigned char uart_getc()
{
	return uart_recv();
}

int uart_puts(const char* str)
{
	int i = 0;
	for (int i = 0; str[i] != '\0'; i ++)
		uart_putc((unsigned char)str[i]);
	return i;
}

/* Generic function for printing an integer, warning displays nothing for 0 */
int uart_put_uint(unsigned int x, unsigned int base, bool upper_hexa){
	if(x == 0) {
		return 0;
	}
	int y = x % base;
	int written = uart_put_uint(x/base, base, upper_hexa);
	if(y >= 0 && y <= 9) uart_putc(48 + y);
	else if(upper_hexa) uart_putc(55 + y);
	else uart_putc(87 + y);
	return written + 1;
}


int uart_put_int(int x, unsigned int base, bool unsign, bool upper_hexa){
	int written = 0;
	if(x == 0) {
		uart_putc('0');
		written ++;
	}
	else if (x < 0 && !unsign){
		uart_putc('-');
		written += 1 + uart_put_uint(1 + ~x, base, upper_hexa);
	}
	else written += uart_put_uint(x, base, upper_hexa);
	return written;
}


/* uart_printf :
 * Aims at mimicking the behaviour of the C printf but for GPIO
 * Doesn't yet support all the options : for now only conversion flags :
 * d,o,u,x,X,c,s,%*/
int uart_printf(const char* format,...){
	va_list adpar;
	va_start(adpar, format);
	int written = 0;
	int i = 0;
	while(format[i]){
		if(format[i] != '%') {
			uart_putc(format[i]);
			written ++;
		}
		else {
			switch(format[i+1]){
				case '%' :
					uart_putc('%');
					break;
				case 'd':
					written += uart_put_int(va_arg(adpar, int), 10, 0, 0);
					break;
				case 'X':
					written += uart_put_int(va_arg(adpar, int), 16, 0, 1);
					break;
				case 'o':
					written += uart_put_int(va_arg(adpar, unsigned int), 8, 1, 0);
					break;
				case 'u':
					written += uart_put_int(va_arg(adpar, unsigned int), 10, 1, 0);
					break;
				case 'x':
					written += uart_put_int(va_arg(adpar, unsigned int), 16, 1, 0);
					break;
				case 'b':
					written += uart_put_int(va_arg(adpar, unsigned int), 2, 1, 0);
					break;
				case 'c':
					uart_putc(va_arg(adpar,unsigned int));
					written ++;
					break;
				case 's':
					written += uart_puts(va_arg(adpar, char*));
					break;
				default:
					/* TODO : set ERRNO ? */
					return -1;
			}
			i ++;
		}
		i ++;
	}
	return written;
}


void uart_simple_put_reg(uint64_t reg){
	uart_printf("Reg : 0x%x\n",reg);
}

#include "uart.h"
#include "../misc.h"

// Memory-Mapped I/O output
static inline void mmio_write(uint64_t reg, uint64_t data)
{
	*(volatile uint64_t *) reg = data;
}

// Memory-Mapped I/O input
static inline uint64_t mmio_read(uint64_t reg)
{
	return *(volatile uint64_t *)reg;
}

void uart_init()
{
	// Disable UART0.
	mmio_write(UART0_CR, 0x00000000);
	// Setup the GPIO pin 14 && 15.

	// Disable pull up/down for all GPIO pins & delay for 150 cycles.
	mmio_write(GPPUD, 0x00000000);
	delay(150);

	// Disable pull up/down for pin 14,15 & delay for 150 cycles.
	mmio_write(GPPUDCLK0, (1 << 14) | (1 << 15));
	delay(150);

	// Write 0 to GPPUDCLK0 to make it take effect.
	mmio_write(GPPUDCLK0, 0x00000000);

	// Clear pending interrupts.
	mmio_write(UART0_ICR, 0x7FF);

	// Set integer & fractional part of baud rate.
	// Divider = UART_CLOCK/(16 * Baud)
	// Fraction part register = (Fractional part * 64) + 0.5
	// UART_CLOCK = 3000000; Baud = 115200.

	// Divider = 3000000 / (16 * 115200) = 1.627 = ~1.
	mmio_write(UART0_IBRD, 1);
	// Fractional part register = (.627 * 64) + 0.5 = 40.6 = ~40.
	mmio_write(UART0_FBRD, 40);

	// Enable FIFO & 8 bit data transmissio (1 stop bit, no parity).
	mmio_write(UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));

	// Mask all interrupts.
	mmio_write(UART0_IMSC, (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) |
			(1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));

	// Enable UART0, receive & transfer part of UART.
	mmio_write(UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
}

void uart_putc(unsigned char c)
{
	// Wait for UART to become ready to transmit.
	while ( mmio_read(UART0_FR) & (1 << 5) ) { }
	mmio_write(UART0_DR, c);
}

unsigned char uart_getc()
{
	// Wait for UART to have received something.
	while ( mmio_read(UART0_FR) & (1 << 4) ) { }
	return mmio_read(UART0_DR);
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

int uart_put_int(int x, unsigned int base, bool upper_hexa){
	int written = 0;
	if(x == 0) {
		uart_putc('0');
		written ++;
	}
	else if (x < 0){
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
					written += uart_put_int(va_arg(adpar, int), 10, 0);
					break;
				case 'X':
					written += uart_put_int(va_arg(adpar, int), 16, 1);
					break;
				case 'o':
					written += uart_put_int(va_arg(adpar, unsigned int), 8, 0);
					break;
				case 'u':
					written += uart_put_int(va_arg(adpar, unsigned int), 10, 0);
					break;
				case 'x':
					written += uart_put_int(va_arg(adpar, unsigned int), 16, 0);
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

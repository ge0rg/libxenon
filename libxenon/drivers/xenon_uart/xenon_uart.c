#include <stdint.h>

#define BASE 0xea001000

void putch(unsigned char c)
{
	if (c=='\n') putch('\r');

	while (!((*(volatile uint32_t*)(BASE + 0x18))&0x02000000));
	*(volatile uint32_t*)(BASE + 0x14) = (c << 24) & 0xFF000000;
}

int kbhit(void)
{
	uint32_t status;
	
	do
		status = *(volatile uint32_t*)(BASE + 0x18);
	while (status & ~0x03000000);
	
	return !!(status & 0x01000000);
}

int getch(void)
{
	while (!kbhit());
	return (*(volatile uint32_t*)(BASE + 0x10)) >> 24;
}

void uart_puts(unsigned char *s)
{
	while(*s) putch(*s++);
}

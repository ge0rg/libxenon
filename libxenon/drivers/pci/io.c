#include <pci/io.h>

uint32_t read32(long addr)
{
	return __builtin_bswap32(*(volatile uint32_t*)addr);
}

uint32_t read32n(long addr)
{
	return *(volatile uint32_t*)addr;
}

void write32(long addr, uint32_t val)
{
	*(volatile uint32_t*)addr = __builtin_bswap32(val);
}

void write32n(long addr, uint32_t val)
{
	*(volatile uint32_t*)addr = val;
}

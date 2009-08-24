#include <pci/io.h>

inline uint32_t bswap32(uint32_t t)
{
	return ((t & 0xFF) << 24) | ((t & 0xFF00) << 8) | ((t & 0xFF0000) >> 8) | ((t & 0xFF000000) >> 24);
}

uint32_t read32(long addr)
{
	return bswap32(*(volatile uint32_t*)addr);
}

uint32_t read32n(long addr)
{
	return *(volatile uint32_t*)addr;
}

void write32(long addr, uint32_t val)
{
	*(volatile uint32_t*)addr = bswap32(val);
}

void write32n(long addr, uint32_t val)
{
	*(volatile uint32_t*)addr = val;
}

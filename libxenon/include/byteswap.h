#ifndef __byteswap_h
#define __byteswap_h

#include <stdint.h>

static inline uint32_t bswap_32(uint32_t b)
{
	return __builtin_bswap32(b);
}

static inline uint16_t bswap_16(uint16_t b)
{
	return ((b>>8)&0xFF) | ((b << 8) & 0xFF00);
}



#endif

#ifndef __drivers_ppc_timebase_h
#define __drivers_ppc_timebase_h

#include <stdint.h>

#define PPC_TIMEBASE_FREQ 50000000L

static inline uint64_t mftb(void)
{
	uint32_t l, u; 
	asm volatile ("mftbl %0" : "=r" (l));
	asm volatile ("mftbu %0" : "=r" (u));
	return (((uint64_t)u) << 32) | l;
}

#endif

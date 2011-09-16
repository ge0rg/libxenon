#ifndef __drivers_ppc_timebase_h
#define __drivers_ppc_timebase_h

#include <stdint.h>

#define PPC_TIMEBASE_FREQ (3192000000LL/64LL)

static inline uint64_t mftb(void)
{
	uint32_t l, u; 
	asm volatile ("mftbl %0" : "=r" (l));
	asm volatile ("mftbu %0" : "=r" (u));
	return (((uint64_t)u) << 32) | l;
}

static inline unsigned long tb_diff_sec(uint64_t end, uint64_t start)
{
	return (end-start)/PPC_TIMEBASE_FREQ;
}

static inline unsigned long tb_diff_msec(uint64_t end, uint64_t start)
{
	return (end-start)/(PPC_TIMEBASE_FREQ/1000);
}

static inline unsigned long tb_diff_usec(uint64_t end, uint64_t start)
{
	return (end-start)/(PPC_TIMEBASE_FREQ/1000000);
}

#endif

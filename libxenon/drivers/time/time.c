#include <ppc/timebase.h>
#include <stdint.h>

static void tdelay(uint64_t i)
{
	uint64_t t = mftb();
	t += i;
	while (mftb() < t) asm volatile("or 31,31,31");
	asm volatile("or 2,2,2");
}

void udelay(int u)
{
	tdelay(((long long)PPC_TIMEBASE_FREQ) * u / 1000000);
}

void mdelay(int u)
{
	tdelay(((long long)PPC_TIMEBASE_FREQ) * u / 1000);
}

void delay(int u)
{
	tdelay(((long long)PPC_TIMEBASE_FREQ) * u);
}

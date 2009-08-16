#include <ppc/timebase.h>
#include <stdint.h>

static void tdelay(uint64_t i)
{
	uint64_t t = mftb();
	t += i;
	while (mftb() < t);
}

void udelay(int u)
{
	tdelay(PPC_TIMEBASE_FREQ * u / 1000000);
}

void mdelay(int u)
{
	tdelay(PPC_TIMEBASE_FREQ * u / 1000);
}

void delay(int u)
{
	tdelay(PPC_TIMEBASE_FREQ * u);
}

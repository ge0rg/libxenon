#include <xenon_smc/xenon_gpio.h>
#include <pci/io.h>

#define SMC_BASE 0xea001000

void xenon_gpio_set_oe(uint32_t clear, uint32_t set)
{
	write32(SMC_BASE + 0x20, (read32(SMC_BASE + 0x20) &~ clear) | set);
}

void xenon_gpio_set(uint32_t clear, uint32_t set)
{
	write32(SMC_BASE + 0x34, (read32(SMC_BASE + 0x34) &~ clear) | set);
}


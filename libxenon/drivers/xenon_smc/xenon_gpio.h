#ifndef __xenon_smc_xenon_gpio_h
#define __xenon_smc_xenon_gpio_h

#include <stdint.h>

void xenon_gpio_control(uint32_t reg, uint32_t clear, uint32_t set);
void xenon_gpio_set_oe(uint32_t clear, uint32_t set);
void xenon_gpio_set(uint32_t clear, uint32_t set);

#endif

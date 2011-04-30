#ifndef __XENON_SMC
#define __XENON_SMC

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


void xenon_smc_send_message(const unsigned char *msg);
int xenon_smc_receive_message(unsigned char *msg);

int xenon_smc_ana_write(uint8_t addr, uint32_t val);
int xenon_smc_ana_read(uint8_t addr, uint32_t *val);
int xenon_smc_i2c_write(uint16_t addr, uint8_t val);
int xenon_smc_i2c_read(uint16_t addr, uint8_t *val);

void xenon_smc_set_led(int override, int value);
void xenon_smc_set_power_led(int override, int state, int startanim);
void xenon_smc_power_shutdown(void);
void xenon_smc_start_bootanim(void);

void xenon_smc_query_sensors(uint16_t *data);
int xenon_smc_read_avpack(void);

#ifdef __cplusplus
};
#endif

#endif

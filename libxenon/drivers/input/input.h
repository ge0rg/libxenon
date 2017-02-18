#ifndef __input_h
#define __input_h

#ifdef __cplusplus
   extern "C" {
#endif /* __cplusplus */

#include <stdint.h>

struct controller_data_s
{
	signed short s1_x, s1_y, s2_x, s2_y;
	int s1_z, s2_z, lb, rb, start, back, a, b, x, y, up, down, left, right;
	unsigned char lt, rt;
	int logo;
};

int get_controller_data(struct controller_data_s *d, int port);

void set_controller_data(int port, const struct controller_data_s *d);

void set_controller_rumble(int port, uint8_t l, uint8_t r);

#ifdef __cplusplus
   }
#endif /* __cplusplus */

#endif

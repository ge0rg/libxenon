#ifndef __input_h
#define __input_h

struct controller_data_s
{
	signed short s1_x, s1_y, s2_x, s2_y;
	int s1_z, s2_z, lb, rb, start, select, a, b, x, y, up, down, left, right;
	unsigned char lt, rt;
	int logo;
};

int get_controller_data(struct controller_data_s *d, int port);

void set_controller_data(int port, const struct controller_data_s *d);

#endif

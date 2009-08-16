#include <xenon_soc/xenon_secotp.h>

uint64_t xenon_secotp_read_line(int nr)
{
	return *(uint64_t*)(0x8000020000020000 + (nr * 0x200));
}

int xenon_secotp_blow_fuse(int nr)
{
		/* you wish :) */
	return -1;
}

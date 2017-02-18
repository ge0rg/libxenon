#include <xenon_soc/xenon_secotp.h>

uint64_t xenon_secotp_read_line(int nr)
{
	if (nr<0 || nr>=12) return -1;
	return *(uint64_t*)(0x20020000 + (nr * 0x200));
}
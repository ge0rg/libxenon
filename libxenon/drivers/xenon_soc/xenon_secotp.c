#include <xenon_soc/xenon_secotp.h>

extern u64 xenon_fuses[12];

uint64_t xenon_secotp_read_line(int nr)
{
	if (nr<0 || nr>=12) return -1;
	
	return xenon_fuses[nr];
}
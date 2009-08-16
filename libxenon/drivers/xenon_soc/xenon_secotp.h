#ifndef __XENON_SECOTP_H
#define __XENON_SECOTP_H

#include <types/stdint.h>

extern uint64_t xenon_secotp_read_line(int nr);
extern int xenon_secotp_blow_fuse(int nr);

#endif

#ifndef __dvd_h
#define __dvd_h

// for backwards compatibility

#include <diskio/ata.h>

static int dvd_init(){
	return xenon_atapi_init();
}

#endif
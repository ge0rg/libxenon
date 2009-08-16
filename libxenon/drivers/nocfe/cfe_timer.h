#ifndef __USB_CFE_TIMER_H
#define __USB_CFE_TIMER_H

#include <time.h>

#define CFE_HZ 1000
static inline void cfe_sleep(int ticks) { mdelay(ticks); }
static inline void cfe_usleep(int ticks) { udelay(ticks); }

#endif

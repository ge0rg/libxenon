#include "lib_types.h"
#include "lib_queue.h"
#include "lib_malloc.h"
#include "cfe_console.h"
#include "addrspace.h"
#include <ppc/cache.h>

#include <string.h>
#include <stdio.h>

typedef void *hsaddr_t;
#define HSADDR2PTR(x) x
#define PTR2HSADDR(x) x

#define CFE_HZ 1000

#define cfe_sleep mdelay

#define xprintf printf

#define hs_memcpy_from_hs memcpy
#define hs_memcpy_to_hs memcpy

static inline void CACHE_DMA_SYNC(void *x, int l)  { memdcbst(x, l); }
static inline void CACHE_DMA_INVAL(void *x, int l)  { memdcbf(x, l); }


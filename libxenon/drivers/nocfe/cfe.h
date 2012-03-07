#include "lib_types.h"
#include "lib_queue.h"
#include "lib_malloc.h"
#include <ppc/cache.h>
#include <pci/io.h>
#include <time/time.h>

#include <string.h>
#include <stdio.h>

typedef void *hsaddr_t;
typedef unsigned int physaddr_t;

#define PHYSADDR(x)  (x &~ 0x80000000)
#define KERNADDR(x)  (x | 0x80000000)
#define UNCADDR(x)   (x | 0x80000000)

#define console_log(fmt, x...) printf(fmt "\n", x)
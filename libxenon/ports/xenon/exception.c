#include <stdint.h>
#include <stdio.h>
#include <cache.h>


void exception(uint64_t srr0, uint64_t srr1, int wherefrom)
{
  printf("DAMN IT, we failed\n");
  printf("srr0=%016lx, srr1=%016lx, code=%x\n", srr0, srr1, wherefrom);
}


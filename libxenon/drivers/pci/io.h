#ifndef __PCI_IO_H
#define __PCI_IO_H

#include <stdint.h>

uint32_t read32(long addr);
uint32_t read32n(long addr);

void write32(long addr, uint32_t val);
void write32n(long addr, uint32_t val);

#endif

#ifndef __drivers_ppc_vm_h
#define __drivers_ppc_vm_h

#include <stdint.h>

void vm_create_mapping(uint32_t virt_addr, uint64_t phys_addr, int wimg);

#endif

#include <ppc/vm.h>

#define WIMG_CACHED 0x190
#define WIMG_GUARDED 0x1B8

/* Goal is to provide a *simple* memory layout with the following features:
	-	addresses at 0 should pagefault,
	- memory should be continously mapped (cached),
	- we need access to PCI space (guarded),
	- we need access to 0x200_x_x cpu regs (guarded)
*/

uint32_t pagetable[] __attribute__ ((section (".pagetable"))) = {
	0, /* zero "page", should pagefault */
	0,
	(0x20000000000ULL >> 10) | WIMG_GUARDED, // CPU stuff
	0, 
	
	0, 0, 0, 0,

	(0x00000000 >> 10) | WIMG_CACHED,
	(0x10000000 >> 10) | WIMG_CACHED,

	(0x00000000 >> 10) | WIMG_GUARDED,
	(0x10000000 >> 10) | WIMG_GUARDED,

	(0xc0000000 >> 10) | WIMG_GUARDED, // Flash (at 0xc8000000)
	(0xd0000000 >> 10) | WIMG_GUARDED, // PCI config space
	(0xe0000000 >> 10) | WIMG_GUARDED, // PCI space
	0,
};

#if 0
void vm_create_mapping(uint32_t virt_addr, uint64_t phys_addr, int wimg)
{
	/* todo */
}
#endif

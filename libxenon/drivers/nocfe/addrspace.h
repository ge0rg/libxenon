#define K0_TO_PHYS(x) (x &~ 0x80000000)
#define PHYS_TO_K0(x) (x | 0x80000000)
#define PHYS_TO_K1(x) (x | 0x80000000)

#define PHYSADDR(x)  K0_TO_PHYS(x)
#define KERNADDR(x)  PHYS_TO_K0(x)
#define UNCADDR(x)   PHYS_TO_K1(x)

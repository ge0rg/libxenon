#ifndef __ppc_register_h
#define __ppc_register_h

#define __stringify(rn) #rn

#define mfdec()   ({unsigned int rval; \
      asm volatile("mfdec %0" : "=r" (rval)); rval;})
#define mtdec(v)  asm volatile("mtdec %0" : : "r" (v))

#define mfspr(rn) ({unsigned int rval; \
      asm volatile("mfspr %0," __stringify(rn) \
             : "=r" (rval)); rval;})
#define mtspr(rn, v)  asm volatile("mtspr " __stringify(rn) ",%0" : : "r" (v))

#endif

#ifndef __drivers_ppc_cache_cache_h
#define __drivers_ppc_cache_cache_h

extern void memdcbt(void *addr, int len);
extern void memdcbst(void *addr, int len);
extern void memdcbf(void *addr, int len);
extern void memicbi(void *addr, int len);

#endif

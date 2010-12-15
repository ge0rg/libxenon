#ifndef __drivers_ppc_atomic_h
#define __drivers_ppc_atomic_h

void atomic_inc(unsigned int * v);
void atomic_dec(unsigned int * v);
void atomic_clearset(unsigned int * v, unsigned int andc, unsigned int or);
void lock(unsigned int * lock);
void unlock(unsigned int * lock);

#endif

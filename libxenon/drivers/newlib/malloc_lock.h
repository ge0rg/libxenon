
// should be a .h but no way to make it override newlib default implementation
// when built with libxenon...
// for now include it *ONCE* from a .c in your app if you need a thread-safe
// malloc

#ifdef __cplusplus
extern "C" {
#endif

#include <ppc/atomic.h>
#include <ppc/register.h>
#include <assert.h>

static unsigned int spinlock=0;
static volatile int lockcount=0;
static volatile unsigned int lockowner=-1;

__attribute__ ((used)) void __malloc_lock ( struct _reent *_r )
{
    assert(lockcount >= 0);

	if( lockcount == 0 || lockowner != mfspr(pir) )
    {
		lock(&spinlock);
		lockowner=mfspr(pir);
    }

	++lockcount;
}

__attribute__ ((used)) void __malloc_unlock ( struct _reent *_r )
{
    assert(lockcount > 0);
	assert(lockowner == mfspr(pir));

	--lockcount;
	
    if ( lockcount == 0 )
    {
		unlock(&spinlock);  
		lockowner=-1;
    }
}

#ifdef __cplusplus
};
#endif
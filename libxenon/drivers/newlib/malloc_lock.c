#include <sys/reent.h>
#include <ppc/atomic.h>
#include <assert.h>

static unsigned int spinlock=0;
static int lockcount=0;

void __malloc_lock ( struct _reent *_r )
{
    assert(lockcount>=0);
    
    if( !lockcount )
    {
        lock(&spinlock);
    }
    
    lockcount++;   
}

void __malloc_unlock ( struct _reent *_r )
{
    assert(lockcount>0);
    
    lockcount--;
    
    if ( !lockcount )
    {
        unlock(&spinlock);  
    }
}


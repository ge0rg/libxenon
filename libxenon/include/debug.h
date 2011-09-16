#ifndef __include_debug_h
#define __include_debug_h

#ifdef __cplusplus
   extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <xenon_uart/xenon_uart.h>

#define BP {printf("[Breakpoint] in function %s, line %d, file %s\n",__FUNCTION__,__LINE__,__FILE__);getch();}
#define TR {printf("[Trace] in function %s, line %d, file %s\n",__FUNCTION__,__LINE__,__FILE__);}

void buffer_dump(void * buf, int size);

#ifdef __cplusplus
   }
#endif /* __cplusplus */

#endif

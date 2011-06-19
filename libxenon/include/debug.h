#ifndef __include_debug_h
#define __include_debug_h

#include <stdio.h>
#include <xenon_uart/xenon_uart.h>

#define BP {printf("[Breakpoint] in function %s, line %d, file %s\n",__FUNCTION__,__LINE__,__FILE__);getch();}

void buffer_dump(void * buf, int size);

#endif

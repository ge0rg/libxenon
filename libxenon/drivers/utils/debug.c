#include <stdlib.h>
#include <debug.h>

void buffer_dump(void * buf, int size){
	int i;
	char* cbuf=(char*) buf;

	printf("---- %p %d\n",buf,size);
	
	while(size>0){
		for(i=0;i<16;++i){
			printf("%02x ",*cbuf);
			++cbuf;
		}
		printf("\n");
		size-=16;
	}
}


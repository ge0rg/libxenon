#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <debug.h>
#include <xenos/xenos.h>
#include <diskio/dvd.h>
#include <input/input.h>
#include <console/console.h>
#include <diskio/diskio.h>
#include <usb/usbmain.h>
#include <time/time.h>
#include <dirent.h>

#define FG_COL -1
#define BG_COL 0

#define MAX_FILES 1000
#define STICK_THRESHOLD 25000
#define MAX_DISPLAYED_ENTRIES 20

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

struct dirent entries[MAX_FILES];
int entrycount=0;

void load_dir(char * path){
	DIR *d = opendir(path);

	entrycount=0;
	
	if (d){
		struct dirent *de;
		de=readdir(d);
		while(de){
			if(strcmp(de->d_name,".")){
				memcpy(&entries[entrycount],de,sizeof(struct dirent));
				++entrycount;
			}
			
			de=readdir(d);
		}
		closedir(d);
	}
}

void append_dir_to_path(char * path,char * dir){
  if (!strcmp(dir,"..")){
    int i=strlen(path);
    int delimcount=0;

    while(i>=0 && delimcount<2){
      if (path[i]=='/'){
        ++delimcount;

        if(delimcount>1){
          path[i+1]='\0';
        }
      }
      --i;
    }
  }else if (!strcmp(dir,".")){
    return;
  }else{
    strcat(path,dir);
    strcat(path,"/");
  }
}

int main(){
	const char * s;
	char path[256];
	
	int handle;
	struct controller_data_s pad;
	int pos=0,ppos=-1,start,count,i;

	xenos_init(VIDEO_MODE_AUTO);
	console_init();

	usb_init();
	usb_do_poll();

	dvd_init();

  	handle=-1;
    handle=bdev_enum(handle,&s);
    if(handle<0) return 0;

	strcpy(path,s);
	strcat(path,":/");	

	load_dir(path);
	
	for(;;){
 		usb_do_poll();		
 		get_controller_data(&pad, 0);
		
		if (pad.s1_y>STICK_THRESHOLD) --pos;
		if (pad.s1_y<-STICK_THRESHOLD) ++pos;
		
		if (pos<0 || pos>=entrycount){
			pos=ppos;
			continue;
		}
		
		if (pad.a && entries[pos].d_type&DT_DIR){
			append_dir_to_path(path,entries[pos].d_name);
			load_dir(path);
			ppos=-1;
			pos=0;
		}
		
		if(pad.select){
			append_dir_to_path(path,"..");
			load_dir(path);
			ppos=-1;
			pos=0;
		}
		
		if(pad.b){
		    do{
				handle=bdev_enum(handle,&s);
			}while(handle<0);
			strcpy(path,s);
			strcat(path,":/");
			load_dir(path);
			ppos=-1;
			pos=0;
		}

		if (ppos==pos) continue;
		
		memset(&pad,0,sizeof(struct controller_data_s));
		
		console_set_colors(BG_COL,FG_COL);
		console_clrscr();
		printf("A: select, B: change disk, Back: parent dir\n\n%s\n\n",path);
		
		start=MAX(0,pos-MAX_DISPLAYED_ENTRIES/2);
		count=MIN(MAX_DISPLAYED_ENTRIES,entrycount-start);
		
		for(i=start;i<start+count;++i){
			struct dirent *de = &entries[i];

			if (i==pos){
				console_set_colors(FG_COL,BG_COL);
			}else{
				console_set_colors(BG_COL,FG_COL);
			}
			
			if (de->d_type&DT_DIR) console_putch('[');

			s=de->d_name;
			while(*s) console_putch(*s++);
			
			if (de->d_type&DT_DIR) console_putch(']');

			console_putch('\r');
			console_putch('\n');
		}
			
		ppos=pos;
		
		do{
	 		usb_do_poll();		
			get_controller_data(&pad, 0);
		}while(pad.a || pad.b || pad.select || pad.s1_y>STICK_THRESHOLD || pad.s1_y<-STICK_THRESHOLD);
	}
	
	
	return 0;
}
#ifndef _dirent_h_
#define _dirent_h_

#include <sys/iosupport.h>
#include <sys/types.h>
#include <sys/syslimits.h>

#define _DIRENT_HAVE_D_TYPE

#define        DT_UNKNOWN       0
#define        DT_FIFO          1
#define        DT_CHR           2
#define        DT_DIR           4
#define        DT_BLK           6
#define        DT_REG           8
#define        DT_LNK          10
#define        DT_SOCK         12
#define        DT_WHT          14

#ifdef __cplusplus
extern "C" {
#endif
       
       struct dirent {
               ino_t   d_ino;
               unsigned char  d_type;
               char    d_name[NAME_MAX+1];
			   time_t d_atime;
               time_t d_mtime;
               time_t d_ctime;
       };
       
       typedef struct {
               long int        position;
               DIR_ITER*       dirData;
               struct dirent   fileData;
       } DIR;
       
       int closedir(DIR *dirp);
       DIR *opendir(const char *dirname);
       struct dirent *readdir(DIR *dirp);
       int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result);
       void rewinddir(DIR *dirp);
       void seekdir(DIR *dirp, long int loc);
       long int telldir(DIR *dirp);
       
#ifdef __cplusplus
}
#endif

#endif // _dirent_h_
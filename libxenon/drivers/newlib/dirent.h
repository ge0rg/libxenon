#ifndef _SYS_DIRENT_H
# define _SYS_DIRENT_H

#ifdef __cplusplus
extern "C" {
#endif

#define NAME_MAX 256

#define DT_UNKNOWN       0
#define DT_DIR           4
#define DT_REG           8

/*
 * This file was written to be compatible with the BSD directory
 * routines, so it looks like it.  But it was written from scratch.
 * Sean Eric Fagan, sef@Kithrup.COM
 *
 * added d_type for libxenon.
 */

typedef struct _dirdesc {
	int	dd_fd;
} DIR;

# define __dirfd(dp)	((dp)->dd_fd)

#include <sys/types.h>

#include <limits.h>

struct dirent {
	long	d_ino;
	off_t	d_off;
	int d_type;
	unsigned short	d_reclen;
	/* we need better syntax for variable-sized arrays */
	unsigned short	d_namlen;
	char		d_name[NAME_MAX + 1];
};


DIR* opendir(const char* dirname);
struct dirent* readdir(DIR* dirp);
int closedir(DIR* dirp);

#ifdef __cplusplus
}
#endif 


#endif

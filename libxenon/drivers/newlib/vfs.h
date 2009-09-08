#ifndef __newlib_vfs_h
#define __newlib_vfs_h

#include <sys/stat.h>

struct vfs_file_s
{
	struct vfs_fileop_s *ops;
	off_t offset, filesize;
	struct mount_s *mount;
	void *priv[8];
};

struct vfs_fileop_s
{
	ssize_t (*read)(struct vfs_file_s *file, void *dst, size_t len);
	ssize_t (*write)(struct vfs_file_s *file, const void *src, size_t len);
	off_t (*lseek)(struct vfs_file_s *file, size_t offset, int whence);
	int (*isatty)(struct vfs_file_s *file);
	void (*close)(struct vfs_file_s *file);
	int (*fstat)(struct vfs_file_s *file, struct stat *buf);
	int (*getdents)(struct vfs_file_s *file, void *dp, int count);
};

struct mount_s
{
	char mountpoint[32];
	int (*vfs_open)(struct vfs_file_s *file, struct mount_s *mount, const char *filename, int oflags, int perm);
	void *priv[8];
};

struct mount_s *mount(const char *mountpoint, int (*vfs_open)(struct vfs_file_s *file, struct mount_s *mount, const char *filename, int oflags, int perm));
void umount(struct mount_s *mount);

#endif

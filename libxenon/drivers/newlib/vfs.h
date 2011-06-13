#ifndef __newlib_vfs_h
#define __newlib_vfs_h

#include <sys/stat.h>
#include <diskio/diskio.h>
#include <newlib/dirent.h>

struct vfs_file_s
{
	struct vfs_fileop_s *ops;
	off_t offset, filesize;
	struct mount_s *mount;
	void *priv[8];
};

struct vfs_dir_s
{
	DIR dir;
	struct vfs_dirop_s *ops;
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
};

struct vfs_dirop_s
{
	int (*closedir)(struct vfs_dir_s *dirp);
	struct dirent* (*readdir)(struct vfs_dir_s *dirp);
};

struct vfs_mountop_s
{
	int (*open)(struct vfs_file_s *file, struct mount_s *mount, const char *filename, int oflags, int perm);
	int (*opendir)(struct vfs_dir_s *dir, struct mount_s *mount, const char *dirname);
	void (*mount)(struct mount_s *mount, struct bdev * device);
	void (*umount)(struct mount_s *mount);
};

struct mount_s
{
	int index;
	char mountpoint[32];

	struct vfs_mountop_s *ops;

	void *priv[8];
};

struct mount_s *mount(const char *mountpoint, struct vfs_mountop_s *ops, struct bdev * device);
void umount(struct mount_s *mount);

#endif

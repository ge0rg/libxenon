#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <newlib/vfs.h>

struct ffs_s
{
	const char *filename;
	int size;
	void *content;
};

struct ffs_s __attribute__((weak)) ffs_files[] = {{0,0,0}};

#define MAXFD    64
#define MAXMOUNT 16

struct mount_s;

size_t vfs_default_lseek(struct vfs_file_s *file, size_t offset, int whence)
{
	switch (whence)
	{
	case 0:
		file->offset = offset;
		break;
	case 1:
		file->offset += offset;
		break;
	case 2:
		file->offset = file->filesize + offset;
		break;
	}
	if (file->offset < 0)
		file->offset = 0;
	if (file->offset >= file->filesize)
		file->offset = file->filesize;
	return file->offset;
}

			/* /dev/console */
void (*stdout_hook)(const char *text, int len) = 0;

extern void putch(unsigned char c);

ssize_t vfs_console_write(struct vfs_file_s *file, const void *src, size_t len)
{
	if (stdout_hook)
		stdout_hook(src, len);
	size_t i;
	for (i = 0; i < len; ++i)
		putch(((const char*)src)[i]);
	return len;
}

int vfs_console_isatty(struct vfs_file_s *file)
{
	return 1;
}

struct vfs_fileop_s vfs_console_ops = {.write = vfs_console_write, .isatty = vfs_console_isatty};

int vfs_console_open(struct vfs_file_s *file, struct mount_s *mount, const char *filename, int oflags, int perm)
{
	if (!strcmp(filename, "console"))
	{
		file->ops = &vfs_console_ops;
		return 0;
	}
	return -ENOENT;
}

static struct mount_s mounts[MAXMOUNT] = {
	{"/dev/", vfs_console_open},
};

struct mount_s *mount(const char *mountpoint, int (*vfs_open)(struct vfs_file_s *file, struct mount_s *mount, const char *filename, int oflags, int perm))
{
	int i;
	for (i = 0; i < MAXMOUNT; ++i)
	{
		if (!mounts[i].vfs_open)
		{
			strcpy(mounts[i].mountpoint, mountpoint);
			mounts[i].vfs_open = vfs_open;
			return &mounts[i];
		}
	}
	return 0;
}

void umount(struct mount_s *mount)
{
	int i = mount - mounts;
	*mounts[i].mountpoint = 0;
}

static struct vfs_file_s fd_array[MAXFD] = { 
	[0] = {.ops = &vfs_console_ops},
	[1] = {.ops = &vfs_console_ops},
	[2] = {.ops = &vfs_console_ops},
};

extern unsigned char heap_begin;
unsigned char *heap_ptr;

void * sbrk(ptrdiff_t incr)
{
	unsigned char *res;
	if (!heap_ptr)
		heap_ptr = &heap_begin;
	res = heap_ptr;
	heap_ptr += incr;
	return res;
}

static inline struct vfs_file_s *is_valid_and_open_fd(int fd)
{
	return (fd >= 0 && fd < MAXFD && fd_array[fd].ops) ? &fd_array[fd] : 0;
}

ssize_t write(int fildes, const void *buf, size_t nbyte)
{
	struct vfs_file_s *file = is_valid_and_open_fd(fildes);
	
	if (!file)
	{
		errno = EBADF;
		return -1;
	}
	
	ssize_t res = file->ops->write ? file->ops->write(file, buf, nbyte) : -EPERM;
	if (res < 0)
	{
		errno = -res;
		return -1;
	} else
		return res;
}

int close(int fildes)
{
	struct vfs_file_s *file = is_valid_and_open_fd(fildes);
	
	if (!file)
	{
		errno = EBADF;
		return -1;
	}
	
	if (file->ops->close)
		file->ops->close(file);

	memset(file, 0, sizeof(*file));

	return 0;
}

int fstat(int fildes, struct stat *buf)
{
	struct vfs_file_s *file = is_valid_and_open_fd(fildes);
	
	if (!file)
	{
		errno = EBADF;
		return -1;
	}
	
	if (file->ops->fstat)
		return file->ops->fstat(file, buf);
	else
	{
		buf->st_mode = S_IFCHR;
		buf->st_blksize = 0;
		return 0;
	}
}

int isatty(int fildes)
{
	struct vfs_file_s *file = is_valid_and_open_fd(fildes);
	
	if (!file)
	{
		errno = EBADF;
		return -1;
	}
	
	if (file->ops->isatty)
		return file->ops->isatty(file);
	else
		return 0;
}

off_t lseek(int fildes, off_t offset, int whence)
{
	struct vfs_file_s *file = is_valid_and_open_fd(fildes);
	
	if (!file)
	{
		errno = EBADF;
		return -1;
	}
	
	if (file->ops->lseek)
		return file->ops->lseek(file, offset, whence);
	else
	{
		errno = ESPIPE;
		return -1;
	}
}

ssize_t read(int fildes, void *buf, size_t nbyte)
{
	struct vfs_file_s *file = is_valid_and_open_fd(fildes);
	
	if (!file)
	{
		errno = EBADF;
		return -1;
	}
	
	if (file->ops->read)
		return file->ops->read(file, buf, nbyte);
	else
	{
		errno = EINVAL;
		return -1;
	}
}

void _exit(int status)
{
	printf("exit: %d\n", status);
	while (1);
}

int open(const char *path, int oflag, ...)
{
	int fd = 0;
	while (fd < MAXFD)
	{
		if (!fd_array[fd].ops)
			break;
		fd++;
	}
	if (fd == MAXFD)
	{
		errno = EMFILE;
		return -1;
	}
	
	int i;
	for (i = 0; i < MAXMOUNT; ++i)
	{
		const char *mpath = mounts[i].mountpoint;
		if (*mpath && !strncmp(path, mpath, strlen(mpath)))
			if (!mounts[i].vfs_open(&fd_array[fd], &mounts[i], path + strlen(mpath), oflag, 0))
				return fd;
	}
	errno = ENOENT;
	return -1;
}

int getdents (int fd, void *dp, int count)
{
	struct vfs_file_s *file = is_valid_and_open_fd(fd);
	
	if (!file)
	{
		errno = EBADF;
		return -1;
	}
	
	if (file->ops->getdents)
		return file->ops->getdents(file, dp, count);
	else
	{
		errno = EINVAL;
		return -1;
	}
}

pid_t getpid(void)
{
	return 0;
}

int kill(pid_t pid, int sig)
{
	return 0;
}

#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

struct ffs_s
{
	const char *filename;
	int size;
	void *content;
};

struct ffs_s __attribute__((weak)) ffs_files[] = {{0,0,0}};

#define MAXFD 32

static struct fd_s
{
	int open;
	int ffs_entry;
	off_t offset;
	int write;
} fd_array[MAXFD] = {{1, -1}, {1, -1}, {1, -1}}; // stdin, stdout, stderr

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

extern void putch(unsigned char c);

static inline int is_valid_and_open_fd(int fd)
{
	return fd >= 0 && fd < MAXFD && fd_array[fd].open;
}

void (*stdout_hook)(const char *text, int len) = 0;

ssize_t write(int fildes, const void *buf, size_t nbyte)
{
//	printf("write(%d, %p, %ld)=", fildes, buf, nbyte);
	if (!is_valid_and_open_fd(fildes))
	{
//		printf("[EBADF]\n");
		errno = EBADF;
		return -1;
	}
	if (isatty(fildes))
	{
		const char *p = buf;

		if (stdout_hook)
			stdout_hook(p, nbyte);

		int i = 0;
		while (nbyte--)
		{
			putch(*p++);
			i++;
		}
//		printf("%d\n", i);
		return i;
	} else
	{
		printf("*** WRITE %d\n", fildes);
		int i;
		for (i = 0; i < nbyte; ++i)
			printf("%02x ", ((unsigned char*)buf)[i]);
		printf("\n");
		return nbyte;
	}
//	printf("[EPERM]\n");
	errno = EPERM;
	return -1;
}

int close(int fildes)
{
//	printf("close(%d)\n", fildes);
	if (!is_valid_and_open_fd(fildes))
	{
		errno = EBADF;
		return -1;
	}
	if (fd_array[fildes].write)
		printf("*** CLOSE %d\n", fildes);

	fd_array[fildes].open = 0;
	return 0;
}

int fstat(int fildes, struct stat *buf)
{
	buf->st_mode = S_IFCHR; 
	buf->st_blksize = 0;
	return 0;
}

int isatty(int fildes)
{
	if (!is_valid_and_open_fd(fildes))
	{
		errno = EBADF;
		return -1;
	}
	return fd_array[fildes].ffs_entry == -1; // stdout
}

off_t lseek(int fildes, off_t offset, int whence)
{
//	printf("lseek(%d, 0x%lx, %d)\n", fildes, offset, whence);
	if (!is_valid_and_open_fd(fildes))
	{
		errno = EBADF;
		return -1;
	}

	if (isatty(fildes))
	{
		errno = ESPIPE;
		return ((off_t)-1);
	}
	
	size_t filesize = ffs_files[fd_array[fildes].ffs_entry].size;
	
	switch (whence)
	{
	case 0:
		fd_array[fildes].offset = offset;
		break;
	case 1:
		fd_array[fildes].offset += offset;
		break;
	case 2:
		fd_array[fildes].offset = filesize + offset;
		break;
	}
	if (fd_array[fildes].offset < 0)
		fd_array[fildes].offset = 0;
	if (fd_array[fildes].offset >= filesize)
		fd_array[fildes].offset = filesize;
	return fd_array[fildes].offset;
}

ssize_t read(int fildes, void *buf, size_t nbyte)
{
//	printf("read(%d, %p, %ld)=", fildes, buf, nbyte);
	if (!is_valid_and_open_fd(fildes))
	{
//		printf("[EBADF]\n");
		errno = EBADF;
		return -1;
	}
	
	if (isatty(fildes))
	{
//		printf("[EIO]\n");
		errno = EIO;
		return -1;
	}

	size_t filesize = ffs_files[fd_array[fildes].ffs_entry].size;
	off_t off = fd_array[fildes].offset;
	
	if (off + nbyte > filesize)
		nbyte = filesize - off;
	
	memcpy(buf, ffs_files[fd_array[fildes].ffs_entry].content + off, nbyte);
	fd_array[fildes].offset += nbyte;
//	printf("%ld [%02x %02x %02x %02x]\n", nbyte, 
//		((unsigned char*)buf)[0], ((unsigned char*)buf)[1], ((unsigned char*)buf)[2], ((unsigned char*)buf)[3]);
	return nbyte;
}

void _exit(int status)
{
	printf("exit: %d\n", status);
	while (1);
}

int open(const char *path, int oflag, ...)
{
	printf("open(\"%s\", 0x%x)=", path, oflag);
	int fd = 0;
	while (fd < MAXFD)
	{
		if (!fd_array[fd].open)
			break;
		fd++;
	}
	if (fd == MAXFD)
	{
		printf("[EMFILE]\n");
		errno = EMFILE;
		return -1;
	}

	fd_array[fd].write = 0;

	if (oflag & O_CREAT)
	{
		printf("\n*** CREATE %d, %s\n", fd, path);
		fd_array[fd].ffs_entry = 0;
		fd_array[fd].offset = 0;
		fd_array[fd].open = 1;
		fd_array[fd].write = 1;
		return fd;
	}
	
	unsigned int i;
	for (i = 0; ffs_files[i].filename; ++i)
		if (!strcmp(path, ffs_files[i].filename))
			break;
	if (!ffs_files[i].filename)
	{
		printf("[ENOENT]\n");
		errno = ENOENT;
		return -1;
	}

	fd_array[fd].ffs_entry = i;
	fd_array[fd].offset = 0;
	fd_array[fd].open = 1;
	printf("%d\n", fd);
	return fd;
}

pid_t getpid(void)
{
	return 0;
}

int kill(pid_t pid, int sig)
{
	return 0;
}

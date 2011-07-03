#include <stdlib.h>
#include <string.h>

#include "fat.h"
#include "fat_rb.h"
#include "file_rb.h"
#include "dir_rb.h"
#include <diskio/disk_rb.h>
#include <newlib/dirent.h>

off_t _fat_lseek(struct vfs_file_s *file, size_t offset, int whence)
{
	int fd = (int)file->priv[0];
	return rb_lseek(fd,offset,whence);
}


int _fat_read(struct vfs_file_s *file, void *dst, size_t len)
{
	int fd = (int)file->priv[0];
	return rb_read(fd,dst,len);
}

int _fat_write(struct vfs_file_s *file, const void *src, size_t len)
{
	int fd = (int)file->priv[0];
	return rb_write(fd,src,len);
}

int _fat_fstat(struct vfs_file_s *file, struct stat *buf)
{
	int fd = (int)file->priv[0];
	buf->st_size = rb_filesize(fd);
	buf->st_mode = S_IFREG;
	buf->st_blksize = 65536;
	return 0;
}

void _fat_close(struct vfs_file_s *file)
{
	int fd = (int)file->priv[0];
	rb_close(fd);
}

struct vfs_fileop_s vfs_fat_file_ops = {.read = _fat_read, .write = _fat_write,  .lseek = _fat_lseek, .fstat = _fat_fstat, .close = _fat_close};

int _fat_closedir(struct vfs_dir_s *dirp)
{
	RB_DIR * rbdir = dirp->priv[0];
	struct dirent * dentry = dirp->priv[1];
	
	free(dentry);
	return rb_closedir(rbdir);
}

struct dirent* _fat_readdir(struct vfs_dir_s *dirp)
{
	RB_DIR * rbdir = dirp->priv[0];
	struct dirent * dentry = dirp->priv[1];
	
	struct rb_dirent * rbdentry = rb_readdir(rbdir);
	
	if (rbdentry)
	{
		memset(dentry,0,sizeof(struct dirent));
		
		dentry->d_namlen=rb_strlen(rbdentry->d_name);
		dentry->d_ino=rbdentry->startcluster;
		dentry->d_type=(rbdentry->attribute&ATTR_DIRECTORY)?DT_DIR:DT_REG;
		rb_strlcpy(dentry->d_name,rbdentry->d_name,NAME_MAX);

		return dentry;
	}
	
	return NULL;
}

struct vfs_dirop_s vfs_fat_dir_ops = {.readdir = _fat_readdir, .closedir = _fat_closedir};

int _fat_open(struct vfs_file_s *file, struct mount_s *mount, const char *filename, int oflags, int perm)
{
	int fd=rb_open(mount->index,filename,oflags);
	
	file->priv[0] = (void *) fd;
	file->ops = &vfs_fat_file_ops;

	return fd < 0;
}

int _fat_opendir(struct vfs_dir_s *dir, struct mount_s *mount, const char *dirname)
{
	RB_DIR * rbdir=rb_opendir(mount->index,dirname);
	
	dir->priv[0] = rbdir;
	dir->ops = &vfs_fat_dir_ops;

	if (rbdir)
	{
		dir->priv[1] = malloc(sizeof(struct dirent));
		memset(dir->priv[1],0,sizeof(struct dirent));
		return 0;
	}

	return -1;
}


void _fat_mount(struct mount_s *mount, struct bdev * device)
{
	mount->priv[0] = device;
	fat_init();
	disk_mount(device->index,mount->index);
}

void _fat_umount(struct mount_s *mount)
{
	fat_unmount(mount->index,true);
}

struct vfs_mountop_s vfs_fat_mount_ops = {.open = _fat_open, .opendir=_fat_opendir, .mount = _fat_mount, .umount = _fat_umount};



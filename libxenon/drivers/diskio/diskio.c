#include <diskio/diskio.h>
#include <diskio/disk_rb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <newlib/vfs.h>
#include <fat/fat_rb.h>
#include <fat/file_rb.h>
#include <iso9660/iso9660.h>
#include <malloc.h>
#include <newlib/dirent.h>
#include <errno.h>

#define MAX_DEVICES NUM_DRIVES

struct bdev devices[MAX_DEVICES];

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

struct vfs_fileop_s vfs_fat_file_ops = {.read = _fat_read, .lseek = _fat_lseek, .fstat = _fat_fstat, .close = _fat_close};

int _fat_open(struct vfs_file_s *file, struct mount_s *mount, const char *filename, int oflags, int perm)
{
	int fd=rb_open(mount->index,filename,oflags);
	
	file->priv[0] = (void *) fd;
	file->ops = &vfs_fat_file_ops;

	return fd < 0;
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

struct vfs_mountop_s vfs_fat_mount_ops = {.open = _fat_open, .mount = _fat_mount, .umount = _fat_umount};

typedef int (vfs_open_call)(struct vfs_file_s *, struct mount_s *, const char *, int, int);

struct vfs_mountop_s *determine_filesystem (struct bdev *dev)
{
	printf(" * trying to make sense of %s, ", dev->name);

	if(!strcmp(dev->name,"dvd")){
		printf("let's assume it's iso9660\n");
		return &vfs_iso9660_mount_ops;
	}else{
		printf("let's assume it's fat\n");
		return &vfs_fat_mount_ops;
	}
}

struct bdev *register_bdev(void *ctx, struct bdev_ops *ops, const char *name)
{
	int i;
	for (i = 0; i < MAX_DEVICES; ++i)
	{
		if (!devices[i].ops)
			break;
	}
	if (i == MAX_DEVICES)
		return 0;

	devices[i].index = i;
	devices[i].ctx = ctx;
	strcpy(devices[i].name, name); /* strlen(name)>=16 and i'll kill you! */
	devices[i].ops = ops;

	printf("registered new device: %s\n", name);
	
	struct bdev *bdev = devices + i;

	char mountpoint[18];
	snprintf(mountpoint, 18, "%s:", name);
	struct vfs_mountop_s *vfs_ops = determine_filesystem(bdev);
	
	struct mount_s *m = mount(mountpoint, vfs_ops, bdev);
	bdev->mount = m;
	
	return bdev;
}

struct bdev *register_bdev_child(struct bdev *parent, lba_t offset, int index)
{
	struct bdev *c = register_bdev(parent->ctx, parent->ops, "");
	if (!c)
		return 0;
	sprintf(c->name, "%s%d", parent->name, index);
	c->offset = offset;
	return c;
}

void unregister_bdev(struct bdev *bdev)
{
	umount(bdev->mount);
	bdev->ops = NULL;
}

struct bdev *bdev_open(const char *name)
{
	int i;
	for (i = 0; i < MAX_DEVICES; ++i)
		if (!strcmp(devices[i].name, name))
			return devices + i;
	return 0;
}

int bdev_enum(int handle, const char **name)
{
    do{
        ++handle;
    }while(handle<MAX_DEVICES && !devices[handle].ops);

    if (handle>=MAX_DEVICES) return -1;

    if (name) *name=devices[handle].name;

    return handle;
}
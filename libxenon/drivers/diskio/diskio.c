#include <diskio/diskio.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <newlib/vfs.h>
#include <iso9660/iso9660.h>
#include <fat/fat.h>
#include <malloc.h>
#include <newlib/dirent.h>
#include <errno.h>

struct bdev devices[MAXDEVICES];

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
	for (i = 0; i < MAXDEVICES; ++i)
	{
		if (!devices[i].ops)
			break;
	}
	if (i == MAXDEVICES)
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
	for (i = 0; i < MAXDEVICES; ++i)
		if (!strcmp(devices[i].name, name))
			return devices + i;
	return 0;
}

int bdev_enum(int handle, const char **name)
{
    do{
        ++handle;
    }while(handle<MAXDEVICES && !devices[handle].ops);

    if (handle>=MAXDEVICES) return -1;

    if (name) *name=devices[handle].name;

    return handle;
}
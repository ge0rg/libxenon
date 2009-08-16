#include <diskio/diskio.h>
#include <string.h>
#include <stdio.h>

#define MAX_DEVICES 16

struct bdev devices[MAX_DEVICES];

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
	devices[i].ctx = ctx;
	strcpy(devices[i].name, name); /* strlen(name)>=16 and i'll kill you! */
	devices[i].ops = ops;
	printf("registered new device: %s\n", name);
	return devices + i;
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
	bdev->disabled = 1;
}

struct bdev *bdev_open(const char *name)
{
	int i;
	for (i = 0; i < MAX_DEVICES; ++i)
		if (!strcmp(devices[i].name, name))
			return devices + i;
	return 0;
}

#ifndef __diskio_h
#define __diskio_h

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long long lba_t;

struct bdev;

struct bdev_ops
{
	int (*read)(struct bdev *dev, void *data, lba_t lba, int num);
};

struct bdev
{
	void *ctx;
	char name[256];
	lba_t offset;
	struct bdev_ops *ops;
	int disabled; /* used when USB device is unplugged */
	void *mount;
};

struct bdev *register_bdev(void *ctx, struct bdev_ops *ops, const char *name);
struct bdev *register_bdev_child(struct bdev *parent, lba_t offset, int index);

void unregister_bdev(struct bdev *bdev);

struct bdev *bdev_open(const char *name);

#ifdef __cplusplus
};
#endif

#endif

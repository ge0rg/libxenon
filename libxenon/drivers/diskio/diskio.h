#ifndef __diskio_h
#define __diskio_h

#ifdef __cplusplus
extern "C" {
#endif

#define DISKIO_ERROR_NO_MEDIA (-8)
	
typedef unsigned long long lba_t;

struct bdev;

struct bdev_ops
{
	int (*read)(struct bdev *dev, void *data, lba_t lba, int num);
};

struct bdev
{
	int index;
	void *ctx;
	char name[32];
	lba_t offset;
	struct bdev_ops *ops;
	void *mount;
};

struct bdev *register_bdev(void *ctx, struct bdev_ops *ops, const char *name);
struct bdev *register_bdev_child(struct bdev *parent, lba_t offset, int index);

void unregister_bdev(struct bdev *bdev);

struct bdev *bdev_open(const char *name);

int bdev_enum(int handle, const char **name); // return value=-1 means last call returned the last item
                                              // first call: pass handle=-1, next calls: pass handle=previous return value
                                              // name: pass the address of a variable that will recieve the name as char *

#ifdef __cplusplus
};
#endif

#endif

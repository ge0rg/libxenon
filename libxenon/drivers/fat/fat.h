#ifndef __fat_fat_h
#define __fat_fat_h

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int fat_read(void *data, uint32_t len);
int fat_open(const char *name);
int fat_init(struct bdev *_dev);

#ifdef __cplusplus
};
#endif

#endif

#ifndef __fat_fat_h
#define __fat_fat_h

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct fat_context
{
	struct bdev *dev;

	uint64_t partition_start_offset;

	uint32_t bytes_per_cluster;
	uint32_t root_entries;
	uint32_t clusters;
	uint32_t fat_type;	// 12, 16, or 32

	uint64_t fat_offset;
	uint64_t root_offset;
	uint64_t data_offset;


	uint64_t extent_offset;
	uint32_t extent_len;
	uint32_t extent_next_cluster;

	uint8_t fat_name[11];

	uint32_t fat_file_size;
};

int fat_read(struct fat_context *fat, void *data, uint32_t len);
int fat_open(struct fat_context *fat, const char *name);
int fat_init(struct fat_context *fat, struct bdev *_dev);

#ifdef __cplusplus
};
#endif

#endif

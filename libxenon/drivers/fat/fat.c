// Copyright 2009  Segher Boessenkool  <segher@kernel.crashing.org>
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
// THE POSSIBILITY OF SUCH DAMAGE.
// 
// hacked to fit into libxenon by Felix Domke <tmbinc@elitedvb.net>

#include <stdint.h>
#include <string.h>
#include <diskio/diskio.h>

static inline uint16_t le16(const uint8_t *p)
{
	return p[0] | (p[1] << 8);
}
	
static inline uint32_t le32(const uint8_t *p)
{
	return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}
		
#include <stdio.h>

#define RAW_BUF 0x200
#define MAX_SECTS 8
static uint8_t raw_buf[RAW_BUF] __attribute__((aligned(32)));

static struct bdev *dev;

static int raw_read(uint32_t sector)
{
	static uint32_t current = -1;

	if (current == sector)
		return 0;
	current = sector;
	
	return dev->ops->read(dev, raw_buf, sector, 1) < 0 ? : 0;
}

static uint64_t partition_start_offset;

static int read(uint8_t *data, uint64_t offset, uint32_t len)
{
	offset += partition_start_offset;

	while (len) {
		uint32_t buf_off = offset % RAW_BUF;
		uint32_t n;
		
		int err;
		if (!buf_off && !((data - raw_buf)&31) && len >= RAW_BUF)
		{	
			n = len / RAW_BUF;
			if (n > MAX_SECTS)
				n = MAX_SECTS;
				/* aligned */
			err = dev->ops->read(dev, data, offset / RAW_BUF, n);
			if (err < n)
				return err;
			n *= RAW_BUF;
		} else
		{
				/* non-aligned */
			n = RAW_BUF - buf_off;
			if (n > len)
				n = len;
			err = raw_read(offset / RAW_BUF);
			if (err)
				return err;

			memcpy(data, raw_buf + buf_off, n);
		}
		
		data += n;
		offset += n;
		len -= n;
	}

	return 0;
}


static uint32_t bytes_per_cluster;
static uint32_t root_entries;
static uint32_t clusters;
static uint32_t fat_type;	// 12, 16, or 32

static uint64_t fat_offset;
static uint64_t root_offset;
static uint64_t data_offset;


static uint32_t get_fat(uint32_t cluster)
{
	uint8_t fat[4];

	uint32_t offset_bits = cluster*fat_type;
	int err = read(fat, fat_offset + offset_bits/8, 4);
	if (err)
		return 0;

	uint32_t res = le32(fat) >> (offset_bits % 8);
	res &= (1 << fat_type) - 1;
	res &= 0x0fffffff;		// for FAT32

	return res;
}


static uint64_t extent_offset;
static uint32_t extent_len;
static uint32_t extent_next_cluster;

static void get_extent(uint32_t cluster)
{
	extent_len = 0;
	extent_next_cluster = 0;

	if (cluster == 0) {	// Root directory.
		if (fat_type != 32) {
			extent_offset = root_offset;
			extent_len = 0x20*root_entries;

			return;
		}
		cluster = root_offset;
	}

	if (cluster - 2 >= clusters)
		return;

	extent_offset = data_offset + (uint64_t)bytes_per_cluster*(cluster - 2);

	for (;;) {
		extent_len += bytes_per_cluster;

		uint32_t next_cluster = get_fat(cluster);

		if (next_cluster - 2 >= clusters)
			break;

		if (next_cluster != cluster + 1) {
			extent_next_cluster = next_cluster;
			break;
		}

		cluster = next_cluster;
	}
}


static int read_extent(uint8_t *data, uint32_t len)
{
	while (len) {
		if (extent_len == 0)
			return -1;

		uint32_t this = len;
		if (this > extent_len)
			this = extent_len;

		int err = read(data, extent_offset, this);
		if (err)
			return err;

		extent_offset += this;
		extent_len -= this;

		data += this;
		len -= this;

		if (extent_len == 0 && extent_next_cluster)
			get_extent(extent_next_cluster);
	}

	return 0;
}


int fat_read(void *data, uint32_t len)
{
	return read_extent(data, len);
}


static uint8_t fat_name[11];

static uint8_t ucase(char c)
{
	if (c >= 'a' && c <= 'z')
		return c - 'a' + 'A';

	return c;
}

static const char *parse_component(const char *path)
{
	uint32_t i = 0;

	while (*path == '/')
		path++;

	while (*path && *path != '/' && *path != '.') {
		if (i < 8)
			fat_name[i++] = ucase(*path);
		path++;
	}

	while (i < 8)
		fat_name[i++] = ' ';

	if (*path == '.')
		path++;

	while (*path && *path != '/') {
		if (i < 11)
			fat_name[i++] = ucase(*path);
		path++;
	}

	while (i < 11)
		fat_name[i++] = ' ';

	if (fat_name[0] == 0xe5)
		fat_name[0] = 0x05;

	return path;
}


uint32_t fat_file_size;

int fat_open(const char *name)
{
	uint32_t cluster = 0;

	while (*name) {
		get_extent(cluster);

		name = parse_component(name);

		while (extent_len) {
			uint8_t dir[0x20];

			int err = read_extent(dir, 0x20);
			if (err)
				return err;

			if (dir[0] == 0)
				return -1;

			if (dir[0x0b] & 0x08)	// volume label or LFN
				continue;
			if (dir[0x00] == 0xe5)	// deleted file
				continue;

			if (!!*name != !!(dir[0x0b] & 0x10))	// dir vs. file
				continue;

			if (memcmp(fat_name, dir, 11) == 0) {
				cluster = le16(dir + 0x1a);
				if (fat_type == 32)
					cluster |= le16(dir + 0x14) << 16;

				if (*name == 0) {
					fat_file_size = le32(dir + 0x1c);
					get_extent(cluster);

					return 0;
				}

				break;
			}
		}
	}

	return -1;
}


#ifdef FAT_TEST
static void print_dir_entry(uint8_t *dir)
{
	int i, n;

	if (dir[0x0b] & 0x08)	// volume label or LFN
		return;
	if (dir[0x00] == 0xe5)	// deleted file
		return;

	if (fat_type == 32) {
		fprintf(stderr, "#%04x", le16(dir + 0x14));
		fprintf(stderr, "%04x  ", le16(dir + 0x1a));
	} else
		fprintf(stderr, "#%04x  ", le16(dir + 0x1a));	// start cluster
	uint16_t date = le16(dir + 0x18);
	fprintf(stderr, "%04d-%02d-%02d ", 1980 + (date >> 9), (date >> 5) & 0x0f, date & 0x1f);
	uint16_t time = le16(dir + 0x16);
	fprintf(stderr, "%02d:%02d:%02d  ", time >> 11, (time >> 5) & 0x3f, 2*(time & 0x1f));
	fprintf(stderr, "%10d  ", le32(dir + 0x1c));	// file size
	uint8_t attr = dir[0x0b];
	for (i = 0; i < 6; i++)
		fprintf(stderr, "%c", (attr & (1 << i)) ? "RHSLDA"[i] : ' ');
	fprintf(stderr, "  ");
	for (n = 8; n && dir[n - 1] == ' '; n--)
		;
	for (i = 0; i < n; i++)
		fprintf(stderr, "%c", dir[i]);
	for (n = 3; n && dir[8 + n - 1] == ' '; n--)
		;
	if (n) {
		fprintf(stderr, ".");
		for (i = 0; i < n; i++)
			fprintf(stderr, "%c", dir[8 + i]);
	}

	fprintf(stderr, "\n");
}


int print_dir(uint32_t cluster)
{
	uint8_t dir[0x20];

	get_extent(cluster);

	while (extent_len) {
		int err = read_extent(dir, 0x20);
		if (err)
			return err;

		if (dir[0] == 0)
			break;

		print_dir_entry(dir);
	}

	return 0;
}
#endif


static int fat_init_fs(const uint8_t *sb)
{
	uint32_t bytes_per_sector = le16(sb + 0x0b);
	uint32_t sectors_per_cluster = sb[0x0d];
	bytes_per_cluster = bytes_per_sector * sectors_per_cluster;

	uint32_t reserved_sectors = le16(sb + 0x0e);
	uint32_t fats = sb[0x10];
	root_entries = le16(sb + 0x11);
	uint32_t total_sectors = le16(sb + 0x13);
	uint32_t sectors_per_fat = le16(sb + 0x16);

	// For FAT16 and FAT32:
	if (total_sectors == 0)
		total_sectors = le32(sb + 0x20);

	// For FAT32:
	if (sectors_per_fat == 0)
		sectors_per_fat = le32(sb + 0x24);

	// XXX: For FAT32, we might want to look at offsets 28, 2a
	// XXX: We _do_ need to look at 2c

	uint32_t fat_sectors = sectors_per_fat * fats;
	uint32_t root_sectors = (0x20*root_entries + bytes_per_sector - 1)
	                   / bytes_per_sector;

	uint32_t fat_start_sector = reserved_sectors;
	uint32_t root_start_sector = fat_start_sector + fat_sectors;
	uint32_t data_start_sector = root_start_sector + root_sectors;

	clusters = (total_sectors - data_start_sector) / sectors_per_cluster;

	if (clusters < 0x0ff5)
		fat_type = 12;
	else if (clusters < 0xfff5)
		fat_type = 16;
	else
		fat_type = 32;

	fat_offset = (uint64_t)bytes_per_sector*fat_start_sector;
	root_offset = (uint64_t)bytes_per_sector*root_start_sector;
	data_offset = (uint64_t)bytes_per_sector*data_start_sector;

	if (fat_type == 32)
		root_offset = le32(sb + 0x2c);

#ifdef FAT_TEST
	fprintf(stderr, "bytes_per_sector    = %08x\n", bytes_per_sector);
	fprintf(stderr, "sectors_per_cluster = %08x\n", sectors_per_cluster);
	fprintf(stderr, "bytes_per_cluster   = %08x\n", bytes_per_cluster);
	fprintf(stderr, "root_entries        = %08x\n", root_entries);
	fprintf(stderr, "clusters            = %08x\n", clusters);
	fprintf(stderr, "fat_type            = %08x\n", fat_type);
	fprintf(stderr, "fat_offset          = %012llx\n", fat_offset);
	fprintf(stderr, "root_offset         = %012llx\n", root_offset);
	fprintf(stderr, "data_offset         = %012llx\n", data_offset);
#endif

	return 0;
}


static int is_fat_fs(const uint8_t *sb)
{
	// Bytes per sector should be 512, 1024, 2048, or 4096
	uint32_t bps = le16(sb + 0x0b);
	if (bps < 0x0200 || bps > 0x1000 || bps & (bps - 1))
		return 0;

	// Media type should be f0 or f8,...,ff
	if (sb[0x15] < 0xf8 && sb[0x15] != 0xf0)
		return 0;

	// If those checks didn't fail, it's FAT.  We hope.
	return 1;
}


int fat_init(struct bdev *_dev)
{
	uint8_t buf[0x200];
	int err;
	
	dev = _dev;

	partition_start_offset = 0;
	err = read(buf, 0, 0x200);
	if (err)
		return err;

	if (le16(buf + 0x01fe) != 0xaa55)	// Not a DOS disk.
		return -1;

	if (is_fat_fs(buf))
		return fat_init_fs(buf);

	// Maybe there's a partition table?  Let's try the first partition.
	if (buf[0x01c2] == 0)
		return -1;

	partition_start_offset = 0x200ULL*le32(buf + 0x01c6);

	err = read(buf, 0, 0x200);
	if (err)
		return err;

	if (is_fat_fs(buf))
		return fat_init_fs(buf);

	return -1;
}

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
#include <fat/fat.h>

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
static uint32_t current = -1;

static struct fat_context *current_ctx;

static int raw_read(struct fat_context * ctx, uint32_t sector)
{
	if (current_ctx == ctx && current == sector)
		return 0;
	current = sector;
	current_ctx = ctx;

	return ctx->dev->ops->read(ctx->dev, raw_buf, sector, 1) < 0 ? : 0;
}

static int read(struct fat_context *ctx, uint8_t *data, uint64_t offset, uint32_t len)
{
	offset += ctx->partition_start_offset;

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
			err = ctx->dev->ops->read(ctx->dev, data, offset / RAW_BUF, n);
			if (err < n)
				return err;
			n *= RAW_BUF;
		} else
		{
				/* non-aligned */
			n = RAW_BUF - buf_off;
			if (n > len)
				n = len;
			err = raw_read(ctx, offset / RAW_BUF);
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


static uint32_t get_fat(struct fat_context *ctx, uint32_t cluster)
{
	uint8_t fat[4];

	uint32_t offset_bits = cluster*ctx->fat_type;
	int err = read(ctx, fat, ctx->fat_offset + offset_bits/8, 4);
	if (err)
		return 0;

	uint32_t res = le32(fat) >> (offset_bits % 8);
	res &= (1 << ctx->fat_type) - 1;
	res &= 0x0fffffff;		// for FAT32

	return res;
}

static void get_extent(struct fat_context *ctx, uint32_t cluster)
{
	ctx->extent_len = 0;
	ctx->extent_next_cluster = 0;

	if (cluster == 0) {	// Root directory.
		if (ctx->fat_type != 32) {
			ctx->extent_offset = ctx->root_offset;
			ctx->extent_len = 0x20*ctx->root_entries;

			return;
		}
		cluster = ctx->root_offset;
	}

	if (cluster - 2 >= ctx->clusters)
		return;

	ctx->extent_offset = ctx->data_offset + (uint64_t)ctx->bytes_per_cluster*(cluster - 2);

	for (;;) {
		ctx->extent_len += ctx->bytes_per_cluster;

		uint32_t next_cluster = get_fat(ctx, cluster);

		if (next_cluster - 2 >= ctx->clusters)
			break;

		if (next_cluster != cluster + 1) {
			ctx->extent_next_cluster = next_cluster;
			break;
		}

		cluster = next_cluster;
	}
}


static int read_extent(struct fat_context *ctx, uint8_t *data, uint32_t len)
{
	while (len) {
		if (ctx->extent_len == 0)
			return -1;

		uint32_t this = len;
		if (this > ctx->extent_len)
			this = ctx->extent_len;

		int err = read(ctx, data, ctx->extent_offset, this);
		if (err)
			return err;

		ctx->extent_offset += this;
		ctx->extent_len -= this;

		data += this;
		len -= this;

		if (ctx->extent_len == 0 && ctx->extent_next_cluster)
			get_extent(ctx, ctx->extent_next_cluster);
	}

	return 0;
}


int fat_read(struct fat_context *ctx, void *data, uint32_t len)
{
	return read_extent(ctx, data, len);
}


static uint8_t ucase(char c)
{
	if (c >= 'a' && c <= 'z')
		return c - 'a' + 'A';

	return c;
}

static const char *parse_component(struct fat_context *ctx, const char *path)
{
	uint32_t i = 0;

	while (*path == '/')
		path++;

	while (*path && *path != '/' && *path != '.') {
		if (i < 8)
			ctx->fat_name[i++] = ucase(*path);
		path++;
	}

	while (i < 8)
		ctx->fat_name[i++] = ' ';

	if (*path == '.')
		path++;

	while (*path && *path != '/') {
		if (i < 11)
			ctx->fat_name[i++] = ucase(*path);
		path++;
	}

	while (i < 11)
		ctx->fat_name[i++] = ' ';

	if (ctx->fat_name[0] == 0xe5)
		ctx->fat_name[0] = 0x05;

	return path;
}

/**
Byte Offset 	Length 	Description
0x00 	1 	Sequence Number
0x01 	10 	Name characters (five UTF-16 characters)
0x0b 	1 	Attributes (always 0x0F)
0x0c 	1 	Reserved (always 0x00)
0x0d 	1 	Checksum of DOS file name
0x0e 	12 	Name characters (six UTF-16 characters)
0x1a 	2 	First cluster (always 0x0000)
0x1c 	4 	Name characters (two UTF-16 characters)
*/
static char oldlfn[256];
static char *getLFN(uint8_t *dir)
{
	int i=1,j;
	int SN=dir[00];
	int end=0;

	if(dir[00]>0x60)
	{
		return;
	} //erreur

	if(SN>0x40)//20 normalement
	{
		SN-=0x40;
		memset (oldlfn,'0',256);
	}
	j=(SN-1)*(5+6+2);//13 caractere * SN

	while(end==0)
	{
		oldlfn[j]=(dir[i]);
		i+=2;
		j++;
		if(dir[i]==0xff)
			end=1;
		if(i==0x0b)
			i=0x0e;
		if(i==0x1a)
			i=0x1c;
		if(i>=0x20)
			end=1;
	}

	if(SN==0x01)
	{
		return oldlfn;
	}
	else if(dir[00]>0x40)
	{
		oldlfn[j++]='\0';
	}
}

char *getfilename(const char *longname)
{
	int i=0;
	char *ret=malloc(256);//[256];

	if(*longname=='/')
		longname++;//si commence par /
	while(*longname)
	{
		if ((*longname == '/'))
		{
			//Se debarasse du repertoire
			//i=0;
			break;
		}
		else
			ret[i++]=*longname;

		longname++;
	}

	ret[i++]='\0';
	return ret;
}


int fat_open(struct fat_context *ctx, const char *name)
{
	uint32_t cluster = 0;
	char longfilename[256],filename[256];
	while (1) {
		get_extent(ctx, cluster);

		if (!*name)
			return 0;

		strcpy(filename,getfilename(name));
		name = parse_component(ctx, name);

		while (ctx->extent_len) {
			uint8_t dir[0x20];

			int err = read_extent(ctx, dir, 0x20);
			if (err)
				return err;

			if (dir[0] == 0)
				return -1;

			if (dir[0x0b] == 0x08)	// volume label or LFN
				continue;
			if (dir[0x00] == 0xe5)	// deleted file
				continue;

			if (dir[0x0b] == 0x0f) //LFN
			{
				strcpy(longfilename,getLFN(dir));// a arranger
				continue;
			}

			//if (memcmp(ctx->fat_name, dir, 11) == 0) {
			if(strcmp(longfilename,filename)==0) {
				cluster = le16(dir + 0x1a);
				if (ctx->fat_type == 32)
					cluster |= le16(dir + 0x14) << 16;

				if (*name == 0) {
					ctx->fat_file_size = le32(dir + 0x1c);
					get_extent(ctx, cluster);

					return 0;
				}

				break;
			}
		}
	}

	return -1;
}

#ifdef FAT_TEST
static void print_dir_entry(struct fat_context *ctx, uint8_t *dir)
{
	int i, n;

	if (dir[0x0b] & 0x08)	// volume label or LFN
		return;
	if (dir[0x00] == 0xe5)	// deleted file
		return;

	if (ctx->fat_type == 32) {
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


int print_dir(struct fat_context *ctx, uint32_t cluster)
{
	uint8_t dir[0x20];

	get_extent(ctx, cluster);

	while (ctx->extent_len) {
		int err = read_extent(ctx, dir, 0x20);
		if (err)
			return err;

		if (dir[0] == 0)
			break;

		print_dir_entry(ctx, dir);
	}

	return 0;
}
#endif


static int fat_init_fs(struct fat_context *ctx, const uint8_t *sb)
{
	uint32_t bytes_per_sector = le16(sb + 0x0b);
	uint32_t sectors_per_cluster = sb[0x0d];
	ctx->bytes_per_cluster = bytes_per_sector * sectors_per_cluster;

	uint32_t reserved_sectors = le16(sb + 0x0e);
	uint32_t fats = sb[0x10];
	ctx->root_entries = le16(sb + 0x11);
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
	uint32_t root_sectors = (0x20*ctx->root_entries + bytes_per_sector - 1)
	                   / bytes_per_sector;

	uint32_t fat_start_sector = reserved_sectors;
	uint32_t root_start_sector = fat_start_sector + fat_sectors;
	uint32_t data_start_sector = root_start_sector + root_sectors;

	ctx->clusters = (total_sectors - data_start_sector) / sectors_per_cluster;

	if (ctx->clusters < 0x0ff5)
		ctx->fat_type = 12;
	else if (ctx->clusters < 0xfff5)
		ctx->fat_type = 16;
	else
		ctx->fat_type = 32;

	ctx->fat_offset = (uint64_t)bytes_per_sector*fat_start_sector;
	ctx->root_offset = (uint64_t)bytes_per_sector*root_start_sector;
	ctx->data_offset = (uint64_t)bytes_per_sector*data_start_sector;

	if (ctx->fat_type == 32)
		ctx->root_offset = le32(sb + 0x2c);

#ifdef FAT_TEST
	fprintf(stderr, "bytes_per_sector    = %08x\n", bytes_per_sector);
	fprintf(stderr, "sectors_per_cluster = %08x\n", sectors_per_cluster);
	fprintf(stderr, "bytes_per_cluster   = %08x\n", bytes_per_cluster);
	fprintf(stderr, "root_entries        = %08x\n", root_entries);
	fprintf(stderr, "clusters            = %08x\n", ctx->clusters);
	fprintf(stderr, "fat_type            = %08x\n", ctx->fat_type);
	fprintf(stderr, "fat_offset          = %012llx\n", ctx->fat_offset);
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


int fat_init(struct fat_context *ctx, struct bdev *_dev)
{
	uint8_t buf[0x200];
	int err;

	ctx->dev = _dev;

	ctx->partition_start_offset = 0;
	err = read(ctx, buf, 0, 0x200);
	if (err)
		return err;

	if (le16(buf + 0x01fe) != 0xaa55)	// Not a DOS disk.
		return -1;

	if (is_fat_fs(buf))
		return fat_init_fs(ctx, buf);

	// Maybe there's a partition table?  Let's try the first partition.
	if (buf[0x01c2] == 0)
		return -1;

	ctx->partition_start_offset = 0x200ULL*le32(buf + 0x01c6);

	err = read(ctx, buf, 0, 0x200);
	if (err)
		return err;

	if (is_fat_fs(buf))
		return fat_init_fs(ctx, buf);

	return -1;
}

#include <diskio/diskio.h>
#include <string.h>
#include <stdio.h>
#include <newlib/vfs.h>
#include <fat/fat.h>
#include <malloc.h>
#include <dirent.h>
#include <errno.h>

#define MAX_DEVICES 16
struct bdev devices[MAX_DEVICES];

int _fat_read(struct vfs_file_s *file, void *dst, size_t len)
{

	struct fat_context * fat = file->priv[0];
	int res = fat_read(fat, dst, len);
	if (!res)
	{
		file->offset += len;
		return len;
	} else
	{
		errno = EIO;
		return -1;
	}
}

int _fat_fstat(struct vfs_file_s *file, struct stat *buf)
{

	struct fat_context * fat = file->priv[0];
	buf->st_size = fat->fat_file_size;
	return 0;
}

void _fat_close(struct vfs_file_s *file)
{

	struct fat_context * fat = file->priv[0];
	free(fat);
}


/**
Information from : http://en.wikipedia.org/wiki/File_Allocation_Table
**/
//(struct fat_context *ctx, uint8_t *data, uint32_t len)

char oldname[256];

static char *getLFN(uint8_t *dir)
{

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
	int err;
	int i=1;
	int j=0;
	int end=0;
	int sequence_number=0;

	char buffname[256];

	if (dir[0x00] == 0xe5)	// deleted file
		return -2;

	sequence_number=dir[00];

	if(dir[00]>0x40)
	{
		sequence_number-=64;
		i=0;
		strcpy(oldname,"");
	}


	//on fabrique l'indice de lettre a partir de la sequence
	i=0x01;
	while(i<0x0b)
	{
		if((dir[i]==0x00)&&(dir[i+1]==0x00))
		{
			end=1;
			break;
		}
		buffname[j]=dir[i];
		//Avance de 2 vu que c de l'utf16
		i+=2;
		j++;
	}
	i=0x0e;

	while((i<0x1a)&&!end)
	{
		if((dir[i]==0x00)&&(dir[i+1]==0x00))
		{
			end=1;
			break;
		}
		buffname[j]=dir[i];
		i+=2;
		j++;
	}

	i=0x1c;
	while((i<0x1F)&&!end)
	{
		if((dir[i]==0x00)&&(dir[i+1]==0x00))
		{
			end=1;
			break;
		}
		buffname[j]=dir[i];
		i+=2;//utf16
		j++;
	}
	buffname[j]='\0';

	strcat(buffname,oldname);

	if(sequence_number==0x01)
	{
		return buffname;
	}
	else
	{
		strcpy(oldname,buffname);
	}
}

//listes les fichiers
int _fat_getdents (struct vfs_file_s *file, void *dp, int count)
{

	struct fat_context * fat = file->priv[0];
	struct dirent *d = dp;

	int ret = 0;
	char oldlongfilename[256];
	strcpy(oldlongfilename,"");

	while (count >= sizeof(*d))
	{
		uint8_t dir[0x20];
//		char name[8+3+1+2];
		char name[256];

		if (fat_read(fat, dir, 0x20))
			break;
		if (dir[0x00] == 0xe5)	// deleted file
			continue;
		if (dir[0x00] == 0)
			break;

		if (dir[0x0b] & 0x0f)	// volume label or LFN
		{
			strcpy(oldlongfilename,getLFN(dir));
			continue;
		}
		else
		{
			if(oldlongfilename[0]>0)
			{
				strcpy(name,oldlongfilename);//continue;
			}
			else
			{
				int n;
				for (n = 8; n && dir[n - 1] == ' '; n--);
				memcpy(name, dir, n);
				name[n] = 0;

				int m;
				for (m = 3; m && dir[8 + m - 1] == ' '; m--);
				if (m)
				{
					name[n] = '.';
					memcpy(name + n + 1, dir + 8, m);
					name[n+m+1] = 0;
				}
			}
		}
		d->d_ino = 1;
		d->d_namlen = strlen(name) + 1;
		d->d_reclen = sizeof(*d); // - NAME_MAX + d->d_namlen;
		d->d_type = (dir[0x0b] & 0x10) ? DT_DIR : DT_REG;
		strcpy(d->d_name, name);

		ret += d->d_reclen;
		d->d_off = ret;
		count -= d->d_reclen;
		d = ((void*)d) + d->d_reclen;
	}

	return ret;
}

struct vfs_fileop_s vfs_fat_ops = {.read = _fat_read, .fstat = _fat_fstat, .close = _fat_close, .getdents = _fat_getdents};

int _fat_open(struct vfs_file_s *file, struct mount_s *mount, const char *filename, int oflags, int perm)
{

	struct fat_context * fat = malloc(sizeof(struct fat_context));
	memset(fat, 0, sizeof(*fat));

	file->priv[0] = fat;
	struct bdev *bdev = mount->priv[0];

	if (fat_init(fat, bdev) < 0)
	{
		free(fat);
		return -1;
	}

	if (fat_open(fat, filename) < 0)
	{
		free(fat);
		return -1;
	}

	file->ops = &vfs_fat_ops;

	return 0;
}

typedef int (vfs_open_call)(struct vfs_file_s *, struct mount_s *, const char *, int, int);

vfs_open_call *determine_filesystem (struct bdev *dev)
{
	printf(" * trying to make sense of %s, let's assume it's FAT\n", dev->name);
	return _fat_open;
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
	devices[i].ctx = ctx;
	strcpy(devices[i].name, name); /* strlen(name)>=16 and i'll kill you! */
	devices[i].ops = ops;
	printf("registered new device: %s\n", name);

	struct bdev *bdev = devices + i;

	char mountpoint[18];
	snprintf(mountpoint, 18, "%s:", name);
	vfs_open_call *vfs_open = determine_filesystem(bdev);

	struct mount_s *m = mount(mountpoint, vfs_open);
	bdev->mount = m;
	m->priv[0] = bdev;

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
	bdev->disabled = 1;
	umount(bdev->mount);
}

struct bdev *bdev_open(const char *name)
{

	int i;
	for (i = 0; i < MAX_DEVICES; ++i)
		if (!strcmp(devices[i].name, name))
			return devices + i;
	return 0;
}

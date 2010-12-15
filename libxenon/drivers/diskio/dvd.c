#include <diskio/dvd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <diskio/diskio.h>
#include <time/time.h>
#include <xetypes.h>

#define ATAPI_ERROR 1
#define ATAPI_DATA 0
#define ATAPI_DRIVE_SELECT 6
#define ATAPI_BYTE_CNT_L 4
#define ATAPI_BYTE_CNT_H 5
#define ATAPI_FEATURES 1
#define ATAPI_COMMAND 7
#define ATAPI_STATUS 7

#define ATAPI_STATUS_DRQ 8
#define ATAPI_STATUS_ERR 1

static const int ide_base=0xea001200;

static inline void ide_out8(int reg, unsigned char d)
{
	*(volatile unsigned char*)(ide_base + reg) = d;
}

static inline void ide_out16(int reg, unsigned short d)
{
	*(volatile unsigned short*)(ide_base + reg) = d;
}

static inline void ide_out32(int reg, unsigned int d)
{
	*(volatile unsigned int*)(ide_base + reg) = d;
}

static inline unsigned char ide_in8(int reg)
{
	return *(volatile unsigned char*)(ide_base + reg);
}

static inline unsigned int ide_in32(int reg)
{
	return *(volatile unsigned int*)(ide_base + reg);
}

int ide_waitready()
{
	while (1)
	{
		int status=ide_in8(ATAPI_STATUS);
		if ((status&0xC0)==0x40) // RDY and /BUSY
			break;
		if (status == 0xFF)
			return -1;
	}
	return 0;
}

int atapi_packet(unsigned char *packet, unsigned char *data, int data_len)
{
	unsigned char * data_ptr;
	int i;
	
	ide_out8(ATAPI_DRIVE_SELECT, 0xA0);

	data_len /= 2;

	ide_out8(ATAPI_BYTE_CNT_L, data_len & 0xFF);
	ide_out8(ATAPI_BYTE_CNT_H, data_len >> 8);

	ide_out8(ATAPI_FEATURES, 0);
	ide_out8(ATAPI_COMMAND, 0xA0);
	
	if (ide_waitready())
	{
		printf(" ! waitready after command failed!\n");
		return -2;
	}
	
	if (ide_in8(ATAPI_STATUS) & ATAPI_STATUS_DRQ)
	{
		ide_out32(ATAPI_DATA, *(unsigned int*)(packet + 0));
		ide_out32(ATAPI_DATA, *(unsigned int*)(packet + 4));
		ide_out32(ATAPI_DATA, *(unsigned int*)(packet + 8));
	} else if (ide_in8(ATAPI_STATUS) & ATAPI_STATUS_ERR)
	{
		printf(" ! error before sending cdb\n");
		return -3; 
	} else
	{
		printf(" ! DRQ not set\n");
		return -4;
	}
	
	if (ide_waitready())
	{
		printf(" ! waitready after CDB failed\n");
		return -5;
	}
	
	data_ptr=data;

	while (data_len)
	{
		unsigned char x = ide_in8(ATAPI_STATUS);
		
		if (x & 0x80) // busy
			continue;

		if (x & 1) // err
		{
			printf(" ! ERROR after CDB [%02x], err=%02x.\n", data_ptr[0], ide_in8(ATAPI_ERROR));
			break;
		}

		for(i=0;i<2048;i+=16)
		{
			*(u32*)data_ptr = *(vu32*)ide_base;
			data_ptr += 4;
			*(u32*)data_ptr = *(vu32*)ide_base;
			data_ptr += 4;
			*(u32*)data_ptr = *(vu32*)ide_base;
			data_ptr += 4;
			*(u32*)data_ptr = *(vu32*)ide_base;
			data_ptr += 4;
		}
		
		data_len -= 2048/2;
	}
	
	if (ide_in8(ATAPI_STATUS) & ATAPI_STATUS_ERR)
	{
		printf(" ! error after reading response\n");
		return -6;
	}
	
	return data_ptr-data;
}

int dvd_request_sense(void)
{
	unsigned char request_sense[12] = {0x03, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0};
	unsigned char sense[24];
	if (atapi_packet(request_sense, sense, 24))
	{
		printf(" ! REQUEST SENSE failed\n");
		return -1;
	}
	return (sense[2] << 16) | (sense[12] << 8) | sense[13];
}

int dvd_read_sector(unsigned char *sector, int lba, int num)
{
	int maxretries = 10;
	int red;
retry:
	if (!maxretries--)
	{
		printf(" ! sorry.\n");
		return -1;
	}
	unsigned char read[12] = {0x28, 0, lba >> 24, lba >> 16, lba >> 8, lba, 0, num >> 8, num, 0, 0, 0};
	
	red=atapi_packet(read, sector, 0x800 * num);
	
	if (red<=0)
	{
		printf(" ! read sector failed!\n");
		int sense = dvd_request_sense();

		printf(" ! sense: %06x\n", sense);
		
		if ((sense >> 16) == 6)
		{
			printf(" * UNIT ATTENTION -> just retry\n");
			goto retry;
		}
		
		if (sense == 0x020401)
		{
			printf(" ! not ready -> retry\n");
			delay(1);
			goto retry;
		}
		
		return -1;
	}
	return red;
}

void dvd_set_modeb(void)
{
	unsigned char res[2048];
	
	unsigned char modeb[12] = {0xE7, 0x48, 0x49, 0x54, 0x30, 0x90, 0x90, 0xd0, 0x01};
	printf("atapi_packet result: %d\n", atapi_packet(modeb, res, 0x30));
}

int dvd_read_op(struct bdev *dev, void *data, lba_t lba, int num){
	
	//printf("[dvd_read_op] lba %d num %d\n",(int)lba,num);
	
	return dvd_read_sector((unsigned char *)data,lba,num);
}

struct bdev_ops dvd_ops =
{
	.read = dvd_read_op
};

void dvd_init(){
	//dvd_set_modeb(); done by Xell ...

	register_bdev(NULL,&dvd_ops,"dvd");
}
/*
 * xenon_config.c
 *
 *  Created on: Mar 4, 2011
 */

#include <xetypes.h>
#include <stdio.h>
#include <string.h>
#include "xenon_config.h"
#include "xenon_sfcx.h"

#define BLOCK_OFFSET 3 //We want the 3rd block of the 4 config blocks)

extern struct sfc sfc;
extern unsigned char sfcx_page[MAX_PAGE_SZ];   //Max known hardware physical page size

struct XCONFIG_SECURED_SETTINGS secured_settings = {0};

static int xenon_config_initialized=0;

void xenon_config_init(void)
{
	if (xenon_config_initialized) return;

	sfcx_init();

	// depends on sfcx already being initialized
	if (sfc.initialized != SFCX_INITIALIZED)
	{
		//TODO: wassup with this...
		printf(" ! config: sfcx not initialized\n");
	}
	else
	{
		//calc our address (specific for our one structure)
		int addr = sfc.addr_config + (BLOCK_OFFSET * sfc.block_sz) + sfc.page_sz;
		int status = sfcx_read_page(sfcx_page, addr, 0);

		//read from nand
		if (SFCX_SUCCESS(status)){
			//TODO: check if we got erased or zeroed nand data
			memcpy(&secured_settings, &sfcx_page[0x0], sizeof secured_settings);
			xenon_config_initialized=1;
		}
	}
}

int xenon_config_get_avregion(void)
{
	unsigned char buf[0x4] = {0x00,0x00,0x00,0x00};

	//read from nand
	memcpy(buf, &secured_settings.AVRegion, 4);

	//check if we got erased nand data
	if (buf[0x0]==0xFF && buf[0x1]==0xFF && buf[0x2]==0xFF)
		return 0;

	//check if we got zeroed nand data
	if (buf[0x0]==0x00 && buf[0x1]==0x00 && buf[0x2]==0x00)
		return 0;



	if (buf[0x2] >= 0x01 && buf[0x2] <= 0x04)
		return buf[0x2];
	else
	{
		printf(" !!! Unknown AVRegion: %02X!!!\n",buf[0x2]);
		return buf[0x2];
	}
}

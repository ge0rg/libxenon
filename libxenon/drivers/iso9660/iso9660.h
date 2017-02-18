/****************************************************************************
 * ISO9660 devoptab
 * 
 * Copyright (C) 2008-2010
 * tipoloski, clava, shagkur, Tantric, joedj
 ****************************************************************************/

#ifndef __ISO9660_H__
#define __ISO9660_H__

#include <xetypes.h>
#include <sys/iosupport.h>
#include <diskio/disc_io.h>

#define MAX_ISO_FILES 8

#ifdef __cplusplus
extern "C" {
#endif

bool ISO9660_Mount(const char* name, const DISC_INTERFACE* disc_interface);
bool ISO9660_Unmount(const char* name);
const char *ISO9660_GetVolumeLabel(const char *name);

#ifdef __cplusplus
}
#endif

#endif


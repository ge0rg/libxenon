/* modified for libxenon */

/* KallistiOS 1.2.0

   dc/fs/iso9660.h
   (c)2000-2001 Dan Potter

   $Id: fs_iso9660.h,v 1.5 2003/04/24 03:14:20 bardtx Exp $
*/

#ifndef __DC_FS_ISO9660_H
#define __DC_FS_ISO9660_H

#include <newlib/vfs.h>

#define MAX_ISO_FILES 8

extern struct vfs_mountop_s vfs_iso9660_mount_ops;

#endif	/* __DC_FS_ISO9660_H */


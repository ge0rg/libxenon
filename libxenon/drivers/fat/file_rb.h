/* modified for libxenon */

/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2002 by Björn Stenberg
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/

#ifndef _FILE_H_
#define _FILE_H_

#undef MAX_PATH /* this avoids problems when building simulator */
#define MAX_PATH 260

#include <sys/types.h>

#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif

#ifndef O_RDONLY
#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR   2
#define O_CREAT  4
#define O_APPEND 8
#define O_TRUNC  0x10
#endif

/*
typedef int (*open_func)(const char* pathname, int flags, ...);
typedef ssize_t (*read_func)(int fd, void *buf, size_t count);
typedef int (*creat_func)(const char *pathname, mode_t mode);
typedef ssize_t (*write_func)(int fd, const void *buf, size_t count);
typedef void (*qsort_func)(void *base, size_t nmemb,  size_t size,
                           int(*_compar)(const void *, const void *));
*/

extern int rb_open(int volume, const char* pathname, int flags);
extern int rb_close(int fd);
extern int rb_fsync(int fd);
extern ssize_t rb_read(int fd, void *buf, size_t count);
extern off_t rb_lseek(int fildes, off_t offset, int whence);
extern ssize_t rb_write(int fd, const void *buf, size_t count);
extern int rb_remove(int volume, const char* pathname);
extern int rb_rename(int volume, const char* path, const char* newname);
extern int rb_ftruncate(int fd, off_t length);
extern off_t rb_filesize(int fd);
extern int release_files(int volume);
//int fdprintf (int fd, const char *fmt, ...) ATTRIBUTE_PRINTF(2, 3);
#endif

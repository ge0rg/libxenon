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
 * Copyright (C) 2002 by Linus Nielsen Feltzing
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

#ifndef FAT_RB_H
#define FAT_RB_H

#include <stdbool.h>

/////////////////////////////////////////////////////////////////////////////////////
// Begin libxenon edits
/////////////////////////////////////////////////////////////////////////////////////

#include <ppc/atomic.h>
#include <diskio/diskio.h>
#include <stdio.h>

/* we have all that */
#define HAVE_FAT16SUPPORT
#define HAVE_MULTIDRIVE
#define HAVE_MULTIVOLUME
#define HAVE_HOTSWAP

/* debug */
//#define DEBUGF printf
#define DEBUGF(args...)
//#define LDEBUGF printf
#define LDEBUGF(args...) 
//#define logf(args...) {printf(args);printf("\n");}
#define logf(args...) 

#define SHAREDBSS_ATTR

/* Drives are things like a disk, a card, a flash chip. They can have volumes on them */
#ifdef HAVE_MULTIDRIVE
#define IF_MD(x) x /* optional drive parameter */
#define IF_MD2(x,y) x,y /* same, for a list of arguments */
#define IF_MD_NONVOID(x) x /* for prototype with sole volume parameter */
#else /* empty definitions if no multi-drive */
#define IF_MD(x)
#define IF_MD2(x,y)
#define IF_MD_NONVOID(x) void
#endif

/* Volumes mean things that have filesystems on them, like partitions */
#ifdef HAVE_MULTIVOLUME
#define IF_MV(x) x /* optional volume parameter */
#define IF_MV2(x,y) x,y /* same, for a list of arguments */
#define IF_MV_NONVOID(x) x /* for prototype with sole volume parameter */
#else /* empty definitions if no multi-volume */
#define IF_MV(x)
#define IF_MV2(x,y)
#define IF_MV_NONVOID(x) void
#endif

/* static endianness conversion */
#define SWAP_16(x) ((typeof(x))(unsigned short)(((unsigned short)(x) >> 8) | \
                                                ((unsigned short)(x) << 8)))

#define SWAP_32(x) ((typeof(x))(unsigned long)( ((unsigned long)(x) >> 24) | \
                                               (((unsigned long)(x) & 0xff0000ul) >> 8) | \
                                               (((unsigned long)(x) & 0xff00ul) << 8) | \
                                                ((unsigned long)(x) << 24)))

#define letoh16(x) SWAP_16(x)
#define letoh32(x) SWAP_32(x)
#define htole16(x) SWAP_16(x)
#define htole32(x) SWAP_32(x)
#define betoh16(x) (x)
#define betoh32(x) (x)
#define htobe16(x) (x)
#define htobe32(x) (x)

/* panic */
#define panicf(s,args...) { printf("[PANIC !!!] ");printf(s,args);exit(1); }


/* Default creation date for files */
#define YEAR  2000
#define MONTH 01
#define DAY   01

/* string redefine */
#define rb_strlen(s) strlen((char *) s)
#define rb_strlcpy(d,s,len) strlcpy((char *)d,(char *)s,len)
#define rb_strncmp(s1,s2,len) strncmp((char *)s1,(char *)s2,len)
#define rb_strcasecmp(s1,s2) strcasecmp((char *)s1,(char *)s2)
#define rb_strcmp(s1,s2) strcmp((char *)s1,(char *)s2)
#define rb_strrchr(s,c)( (unsigned char *)strrchr((char *)s,c))
#define rb_atoi(s) atoi((char *) s)
#define rb_snprintf(s,l,f,args...) snprintf((char *)s,l,f,args)
#define rb_strcpy(d,s) strcpy((char *)d,(char *)s)
#define rb_strcat(d,s) strcat((char *)d,(char *)s)

/////////////////////////////////////////////////////////////////////////////////////
// End libxenon edits
/////////////////////////////////////////////////////////////////////////////////////

/* This value can be overwritten by a target in config-[target].h, but
   that behaviour is still experimental */
#ifndef SECTOR_SIZE
#define SECTOR_SIZE 512
#endif

/* Number of bytes reserved for a file name (including the trailing \0).
   Since names are stored in the entry as UTF-8, we won't be able to
   store all names allowed by FAT. In FAT, a name can have max 255
   characters (not bytes!). Since the UTF-8 encoding of a char may take
   up to 4 bytes, there will be names that we won't be able to store
   completely. For such names, the short DOS name is used. */
#define FAT_FILENAME_BYTES 256

struct fat_direntry
{
    unsigned char name[FAT_FILENAME_BYTES]; /* UTF-8 encoded name plus \0 */
    unsigned short attr;            /* Attributes */
    unsigned char crttimetenth;     /* Millisecond creation
                                       time stamp (0-199) */
    unsigned short crttime;         /* Creation time */
    unsigned short crtdate;         /* Creation date */
    unsigned short lstaccdate;      /* Last access date */
    unsigned short wrttime;         /* Last write time */
    unsigned short wrtdate;         /* Last write date */
    unsigned long filesize;          /* File size in bytes */
    long firstcluster;               /* fstclusterhi<<16 + fstcluslo */
};

#define FAT_ATTR_READ_ONLY   0x01
#define FAT_ATTR_HIDDEN      0x02
#define FAT_ATTR_SYSTEM      0x04
#define FAT_ATTR_VOLUME_ID   0x08
#define FAT_ATTR_DIRECTORY   0x10
#define FAT_ATTR_ARCHIVE     0x20
#define FAT_ATTR_VOLUME      0x40 /* this is a volume, not a real directory */

struct fat_file
{
    long firstcluster;    /* first cluster in file */
    long lastcluster;     /* cluster of last access */
    long lastsector;      /* sector of last access */
    long clusternum;      /* current clusternum */
    long sectornum;       /* sector number in this cluster */
    unsigned int direntry;   /* short dir entry index from start of dir */
    unsigned int direntries; /* number of dir entries used by this file */
    long dircluster;      /* first cluster of dir */
    bool eof;
#ifdef HAVE_MULTIVOLUME
    int volume;          /* file resides on which volume */
#endif
};

struct fat_dir
{
    unsigned int entry;
    unsigned int entrycount;
    long sector;
    struct fat_file file;
    unsigned char sectorcache[SECTOR_SIZE];
    /* There are 2-bytes per characters. We don't want to bother too much, as LFN entries are
     * at much 255 characters longs, that's at most 20 LFN entries. Each entry hold at most
     * 13 characters, that a total of 260 characters. So we keep a buffer of that size.
     * Keep coherent with fat.c code. */
    unsigned char longname[260 * 2];
};

#ifdef HAVE_HOTSWAP
extern void fat_lock(void);
extern void fat_unlock(void);
#endif

extern void fat_init(void);
extern int fat_mount(IF_MV2(int volume,) IF_MD2(int drive,) long startsector);
extern int fat_unmount(int volume, bool flush);
extern void fat_size(IF_MV2(int volume,) /* public for info */
                     unsigned long* size,
                     unsigned long* free);
extern void fat_recalc_free(IF_MV_NONVOID(int volume)); /* public for debug info screen */
extern int fat_create_dir(const char* name,
                          struct fat_dir* newdir,
                          struct fat_dir* dir);
extern int fat_open(IF_MV2(int volume,)
                    long cluster,
                    struct fat_file* ent,
                    const struct fat_dir* dir);
extern int fat_create_file(const char* name,
                           struct fat_file* ent,
                           struct fat_dir* dir);
extern long fat_readwrite(struct fat_file *ent, long sectorcount, 
                         void* buf, bool write );
extern int fat_closewrite(struct fat_file *ent, long size, int attr);
extern int fat_seek(struct fat_file *ent, unsigned long sector );
extern int fat_remove(struct fat_file *ent);
extern int fat_truncate(const struct fat_file *ent);
extern int fat_rename(struct fat_file* file, 
                      struct fat_dir* dir,
                      const unsigned char* newname,
                      long size, int attr);

extern int fat_opendir(IF_MV2(int volume,)
                       struct fat_dir *ent, unsigned long startcluster,
                       const struct fat_dir *parent_dir);
extern int fat_getnext(struct fat_dir *ent, struct fat_direntry *entry);
extern unsigned int fat_get_cluster_size(IF_MV_NONVOID(int volume)); /* public for debug info screen */
extern bool fat_ismounted(int volume);

#endif

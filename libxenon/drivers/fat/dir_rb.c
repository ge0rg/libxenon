/* modified for libxenon */

/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id: dir.c 13741 2007-06-30 02:08:27Z jethead71 $
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
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fat/fat_rb.h>
#include <fat/dir_rb.h>

#if ((defined(MEMORYSIZE) && (MEMORYSIZE > 8)) || MEM > 8)
#define MAX_OPEN_DIRS 12
#else
#define MAX_OPEN_DIRS 8
#endif

static RB_DIR opendirs[MAX_OPEN_DIRS];

#ifdef HAVE_HOTSWAP
// release all dir handles on a given volume "by force", to avoid leaks
int release_dirs(int volume)
{
    RB_DIR* pdir = opendirs;
    int dd;
    int closed = 0;
    for ( dd=0; dd<MAX_OPEN_DIRS; dd++, pdir++)
    {
#ifdef HAVE_MULTIVOLUME
        if (pdir->fatdir.file.volume == volume)
#else
        (void)volume;
#endif
        {
            pdir->busy = false; /* mark as available, no further action */
            closed++;
        }
    }
    return closed; /* return how many we did */
}
#endif /* #ifdef HAVE_HOTSWAP */

RB_DIR* rb_opendir(int volume, const char* name)
{
    char namecopy[MAX_PATH];
    char* part;
    char* end;
    struct fat_direntry entry;
    int dd;
    RB_DIR* pdir = opendirs;

    if ( name[0] != '/' ) {
        DEBUGF("Only absolute paths supported right now\n");
        return NULL;
    }

    /* find a free dir descriptor */
    for ( dd=0; dd<MAX_OPEN_DIRS; dd++, pdir++)
        if ( !pdir->busy )
            break;

    if ( dd == MAX_OPEN_DIRS ) {
        DEBUGF("Too many dirs open\n");
        errno = EMFILE;
        return NULL;
    }

    pdir->busy = true;

    strlcpy(namecopy, name, sizeof(namecopy)); /* just copy */

    if ( fat_opendir(IF_MV2(volume,) &pdir->fatdir, 0, NULL) < 0 ) {
        DEBUGF("Failed opening root dir\n");
        pdir->busy = false;
        return NULL;
    }

    for ( part = strtok_r(namecopy, "/", &end); part;
          part = strtok_r(NULL, "/", &end)) {
        /* scan dir for name */
        while (1) {
            if ((fat_getnext(&pdir->fatdir,&entry) < 0) ||
                (!entry.name[0])) {
                pdir->busy = false;
                return NULL;
            }
            if ( (entry.attr & FAT_ATTR_DIRECTORY) &&
                 (!rb_strcasecmp(part, entry.name)) ) {
                /* In reality, the parent_dir parameter of fat_opendir seems
                 * useless because it's sole purpose it to have a way to
                 * update the file metadata, but here we are only reading
                 * a directory so there's no need for that kind of stuff.
                 * However, the rmdir function uses a ugly hack to
                 * avoid opening a directory twice when deleting it and thus
                 * needs those information. That's why we pass pdir->fatdir both
                 * as the parent directory and the resulting one (this is safe,
                 * in doubt, check fat_open(dir) code) which will allow this kind of
                 * (ugly) things */
                if ( fat_opendir(IF_MV2(volume,)
                                 &pdir->fatdir,
                                 entry.firstcluster,
                                 &pdir->fatdir) < 0 ) {
                    DEBUGF("Failed opening dir '%s' (%ld)\n",
                           part, entry.firstcluster);
                    pdir->busy = false;
                    return NULL;
                }

				break;
            }
        }
    }

    return pdir;
}

int rb_closedir(RB_DIR* dir)
{
    dir->busy=false;
    return 0;
}

struct rb_dirent* rb_readdir(RB_DIR* dir)
{
    struct fat_direntry entry;
    struct rb_dirent* theent = &(dir->theent);

    if (!dir->busy)
        return NULL;

    /* normal directory entry fetching follows here */
    if (fat_getnext(&(dir->fatdir),&entry) < 0)
        return NULL;

    if ( !entry.name[0] )
        return NULL;

    rb_strlcpy(theent->d_name, entry.name, sizeof(theent->d_name));
    theent->attribute = entry.attr;
    theent->size = entry.filesize;
    theent->startcluster = entry.firstcluster;
    theent->wrtdate = entry.wrtdate;
    theent->wrttime = entry.wrttime;

    return theent;
}

int rb_mkdir(int volume, const char *name)
{
    RB_DIR *dir;
    char namecopy[MAX_PATH];
    char* end;
    char *basename;
    char *parent;
    struct rb_dirent *entry;
    struct fat_dir newdir;
    int rc;

    if ( name[0] != '/' ) {
        DEBUGF("mkdir: Only absolute paths supported right now\n");
        return -1;
    }

    strlcpy(namecopy, name, sizeof(namecopy));

    /* Split the base name and the path */
    end = strrchr(namecopy, '/');
    *end = 0;
    basename = end+1;

    if(namecopy == end) /* Root dir? */
        parent = "/";
    else
        parent = namecopy;
        
    DEBUGF("mkdir: parent: %s, name: %s\n", parent, basename);

    dir = rb_opendir(volume, parent);
    
    if(!dir) {
        DEBUGF("mkdir: can't open parent dir\n");
        return -2;
    }    

    if(basename[0] == 0) {
        DEBUGF("mkdir: Empty dir name\n");
        errno = EINVAL;
        return -3;
    }
    
    /* Now check if the name already exists */
    while ((entry = rb_readdir(dir))) {
        if ( !rb_strcasecmp(basename, entry->d_name) ) {
            DEBUGF("mkdir error: file exists\n");
            errno = EEXIST;
            rb_closedir(dir);
            return - 4;
        }
    }

    memset(&newdir, 0, sizeof(struct fat_dir));
    
    rc = fat_create_dir(basename, &newdir, &(dir->fatdir));
    rb_closedir(dir);
    
    return rc;
}

int rb_rmdir(int volume, const char* name)
{
    int rc;
    RB_DIR* dir;
    struct rb_dirent* entry;
    
    dir = rb_opendir(volume, name);
    if (!dir)
    {
        errno = ENOENT; /* open error */
        return -1;
    }

    /* check if the directory is empty */
    while ((entry = rb_readdir(dir)))
    {
        if (rb_strcmp(entry->d_name, ".") &&
            rb_strcmp(entry->d_name, ".."))
        {
            DEBUGF("rmdir error: not empty\n");
            errno = ENOTEMPTY;
            rb_closedir(dir);
            return -2;
        }
    }

    rc = fat_remove(&(dir->fatdir.file));
    if ( rc < 0 ) {
        DEBUGF("Failed removing dir: %d\n", rc);
        errno = EIO;
        rc = rc * 10 - 3;
    }

    rb_closedir(dir);
    return rc;
}

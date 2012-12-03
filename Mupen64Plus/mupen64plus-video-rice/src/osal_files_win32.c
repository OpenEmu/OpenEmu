/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-core - osal_files_win32.c                                 *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2009 Richard Goedeken                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       * 
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
                       
/* This file contains the definitions for the unix-specific file handling
 * functions
 */

#include <windows.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <direct.h>

#include "osal_files.h"

/* global functions */

int osal_is_directory(const char* name)
{
    char DirName[MAX_PATH + 1];
    int namelen = 0;

    /* we must remove any trailing backslash on the end of the pathname, or this will fail */
    strncpy(DirName, name, MAX_PATH);
    DirName[MAX_PATH] = 0;
    namelen = strlen(DirName);
    if (namelen > 0 && DirName[namelen-1] == '\\')
        DirName[namelen-1] = 0;
    return (GetFileAttributes(DirName) & FILE_ATTRIBUTE_DIRECTORY);
}

int osal_mkdirp(const char *dirpath, int mode)
{
    struct _stat fileinfo;
    size_t dirpathlen = strlen(dirpath);
    char *currpath = _strdup(dirpath);

    /* first, remove sub-dirs on the end (by replacing slashes with NULL chars) until we find an existing directory */
    while (strlen(currpath) > 1 && _stat(currpath, &fileinfo) != 0)
    {
        char *lastslash = strrchr(currpath, '\\');
        if (lastslash == NULL)
        {
            free(currpath);
            return 1; /* error: we never found an existing directory, this path is bad */
        }
        *lastslash = 0;
    }

    /* then walk up the path chain, creating directories along the way */
    do
    {
        if (currpath[strlen(currpath)-1] != '\\' && _stat(currpath, &fileinfo) != 0)
        {
            if (_mkdir(currpath) != 0)
            {
                free(currpath);
                return 2;        /* mkdir failed */
            }
        }
        if (strlen(currpath) == dirpathlen)
            break;
        else
            currpath[strlen(currpath)] = '\\';
    } while (1);
    
    free(currpath);        
    return 0;
}

typedef struct {
    HANDLE hFind;
    WIN32_FIND_DATA find_data;
} dir_search_info;

void * osal_search_dir_open(const char *pathname)
{
   char SearchString[MAX_PATH + 1];
   dir_search_info *pInfo = malloc(sizeof(dir_search_info));

   if (pInfo == NULL)
       return NULL;

   pInfo->hFind = INVALID_HANDLE_VALUE;
   pInfo->find_data.cFileName[0] = 0;

   if (pathname[strlen(pathname)-1] == '\\')
       _snprintf(SearchString, MAX_PATH, "%s*", pathname);
   else
       _snprintf(SearchString, MAX_PATH, "%s\\*", pathname);
   SearchString[MAX_PATH] = 0;

   pInfo->hFind = FindFirstFile(SearchString, &pInfo->find_data);
   return (void *) pInfo;
}

const char *osal_search_dir_read_next(void * search_info)
{
    static char last_filename[_MAX_PATH];
    dir_search_info *pInfo = (dir_search_info *) search_info;

    if (pInfo == NULL || pInfo->hFind == INVALID_HANDLE_VALUE || pInfo->find_data.cFileName[0] == 0)
        return NULL;

    strcpy(last_filename, pInfo->find_data.cFileName);

    if (FindNextFile(pInfo->hFind, &pInfo->find_data) == 0)
    {
        pInfo->find_data.cFileName[0] = 0;
    }

    return last_filename;
}

void osal_search_dir_close(void * search_info)
{
    dir_search_info *pInfo = (dir_search_info *) search_info;

    if (pInfo != NULL)
    {
        if (pInfo->hFind != INVALID_HANDLE_VALUE)
            FindClose(pInfo->hFind);
        free(pInfo);
    }
}

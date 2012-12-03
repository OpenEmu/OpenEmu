/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-ui-console - osal_files.h                                 *
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

/* This header file is for all kinds of system-dependent file handling
 *
 */

#if !defined(OSAL_FILES_H)
#define OSAL_FILES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "m64p_types.h"

#if defined(WIN32)
  #define PATH_MAX _MAX_PATH
  #define OSAL_DIR_SEPARATOR_STR       "\\"
  #define OSAL_DIR_SEPARATOR_CHAR      '\\'
#else  /* Not WIN32 */
  #include <limits.h>  // for PATH_MAX
  #define OSAL_DIR_SEPARATOR_STR       "/"
  #define OSAL_DIR_SEPARATOR_CHAR      '/'

  /* PATH_MAX only may be defined by limits.h */
  #ifndef PATH_MAX
    #define PATH_MAX 4096
  #endif
#endif

int osal_is_directory(const char* name);
int osal_mkdirp(const char *dirpath, int mode);

void * osal_search_dir_open(const char *pathname);
const char *osal_search_dir_read_next(void * dir_handle);
void osal_search_dir_close(void * dir_handle);

#ifdef __cplusplus
}
#endif

#endif /* #define OSAL_FILES_H */


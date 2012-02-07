#ifndef MEMGZIO_H
#define MEMGZIO_H

/* gzio.c -- IO on .gz files
 * Copyright (C) 1995-2002 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.h
 *
 * Compile this file with -DNO_DEFLATE to avoid the compression code.
 */

/* memgzio.c - IO on .gz files in memory
 * Adapted from original gzio.c from zlib library by Forgotten
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <zlib.h>

gzFile ZEXPORT memgzopen(char *memory, int available, const char *mode);
int ZEXPORT memgzread(gzFile file, voidp buf, unsigned len);
int ZEXPORT memgzwrite(gzFile file, const voidp buf, unsigned len);
int ZEXPORT memgzclose(gzFile file);
long ZEXPORT memtell(gzFile file);


/******************************************************************************
 *
 * unzip.c
 *
 *   Zip Support
 *
 *   Only partial support is included, in that only the first file within the archive
 *   is considered to be a ROM image.
 *
 *   Softdev (2006)
 *   Eke-Eke (2007,2008) 
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ********************************************************************************/

int IsZipFile (char * buffer);
int UnZipBuffer (unsigned char * outbuffer, FILE * fd);
void get_zipfilename(char * filename, char * outfilename);

#ifdef __cplusplus
}
#endif

#endif // MEMGZIO_H

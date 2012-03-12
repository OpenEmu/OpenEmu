/***************************************************************************

    un7z.h

    7z file management.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

// this is based on unzip.h, with modifications needed to use the 7zip library

/***************************************************************************
	Above notices are from MAME
	
	Updated Feb 2012
	Adapted for use outside MAME by Barry Harris (FB Alpha)
***************************************************************************/

#pragma once

#ifndef __UN_7Z_H__
#define __UN_7Z_H__

#include "driver.h"

#define ARRAY_LENGTH(x)		(sizeof(x) / sizeof(x[0]))

#include "7z.h"
#include "7zCrc.h"
#include "7zVersion.h"


void *SZipAlloc(void *p, size_t size);
void SZipFree(void *p, void *address);
void *SZipAllocTemp(void *p, size_t size);
void SZipFreeTemp(void *p, void *address);

typedef struct
{
	long _7z_currfpos;
	UINT64			_7z_length;
	FILE *			_7z_osdfile;					/* OSD file handle */

} CSzFile;


typedef struct
{
	ISeqInStream s;
	CSzFile file;
} CFileSeqInStream;

void FileSeqInStream_CreateVTable(CFileSeqInStream *p);


typedef struct
{
	ISeekInStream s;
	CSzFile file;
} CFileInStream;

void FileInStream_CreateVTable(CFileInStream *p);


typedef struct
{
	ISeqOutStream s;
	CSzFile file;
} CFileOutStream;

void FileOutStream_CreateVTable(CFileOutStream *p);



/***************************************************************************
    CONSTANTS
***************************************************************************/


/* Error types */
enum __7z_error
{
	_7ZERR_NONE = 0,
	_7ZERR_OUT_OF_MEMORY,
	_7ZERR_FILE_ERROR,
	_7ZERR_BAD_SIGNATURE,
	_7ZERR_DECOMPRESS_ERROR,
	_7ZERR_FILE_TRUNCATED,
	_7ZERR_FILE_CORRUPT,
	_7ZERR_UNSUPPORTED,
	_7ZERR_BUFFER_TOO_SMALL
};
typedef enum __7z_error _7z_error;



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* describes an open _7Z file */
typedef struct __7z_file _7z_file;
struct __7z_file
{
	const char *	filename;				/* copy of _7Z filename (for caching) */

	int	curr_file_idx;						/* current file index */
	UINT64 uncompressed_length;				/* current file uncompressed length */
	UINT64 crc;								/* current file crc */

	CFileInStream archiveStream;
	CLookToRead lookStream;
	CSzArEx db;
	SRes res;
	ISzAlloc allocImp;
	ISzAlloc allocTempImp;
	bool inited;

	// cached stuff for solid blocks
	UInt32 blockIndex;// = 0xFFFFFFFF; /* it can have any value before first call (if outBuffer = 0) */
	Byte *outBuffer;// = 0; /* it must be 0 before first call for each new archive. */
	size_t outBufferSize;// = 0;  /* it can have any value before first call (if outBuffer = 0) */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- _7Z file access ----- */

/* open a _7Z file and parse its central directory */
_7z_error _7z_file_open(const char *filename, _7z_file **_7z);

/* close a _7Z file (may actually be left open due to caching) */
void _7z_file_close(_7z_file *_7z);

/* clear out all open _7Z files from the cache */
void _7z_file_cache_clear(void);


/* ----- contained file access ----- */

/* find a file index by crc, filename or both */
int _7z_search_crc_match(_7z_file *new_7z, UINT32 crc, const char *search_filename, int search_filename_length, bool matchcrc, bool matchname);

/* decompress the most recently found file in the _7Z */
_7z_error _7z_file_decompress(_7z_file *new_7z, void *buffer, UINT32 length, UINT32 *Processed);


#endif	/* __UN_7Z_H__ */

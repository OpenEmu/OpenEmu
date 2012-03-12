/***************************************************************************

    un7z.c

    Functions to manipulate data within 7z files.

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

// this is based on unzip.c, with modifications needed to use the 7zip library

/***************************************************************************
	Above notices are from MAME
	
	Updated Feb 2012
	Adapted for use outside MAME by Barry Harris (FB Alpha)
***************************************************************************/

#include "un7z.h"

#include <ctype.h>
#include <stdlib.h>
#include "zlib.h"

/***************************************************************************
    7Zip Memory / File handling (adapted from 7zfile.c/.h and 7zalloc.c/.h)
***************************************************************************/

void *SZipAlloc(void *, size_t size)
{
	if (size == 0)
		return 0;

	return malloc(size);
}

void SZipFree(void *, void *address)
{
	free(address);
}



void File_Construct(CSzFile *p)
{
	p->_7z_osdfile = NULL;
}

static WRes File_Open(CSzFile *p, const char *, int)
{
	/* we handle this ourselves ... */
	if (!p->_7z_osdfile) return 1;
	else return 0;
}

WRes InFile_Open(CSzFile *p, const char *name) { return File_Open(p, name, 0); }
WRes OutFile_Open(CSzFile *p, const char *name) { return File_Open(p, name, 1); }


WRes File_Close(CSzFile *)
{
	/* we handle this ourselves ... */
	return 0;
}

WRes File_Read(CSzFile *p, void *data, size_t *size)
{
	UINT32 read_length;

	if (!p->_7z_osdfile)
	{
		printf("un7z.c: called File_Read without file\n");
		return 1;
	}

	size_t originalSize = *size;
	if (originalSize == 0)
		return 0;

	fseek(p->_7z_osdfile, p->_7z_currfpos, SEEK_SET);
	*size = read_length = fread(data, 1, originalSize, p->_7z_osdfile);
	p->_7z_currfpos += read_length;

	if (*size == originalSize)
		return 0;

	return 0;
}

WRes File_Write(CSzFile *, const void *, size_t *)
{
	return 0;
}

WRes File_Seek(CSzFile *p, Int64 *pos, ESzSeek origin)
{
	if (origin==0) p->_7z_currfpos = *pos;
	if (origin==1) p->_7z_currfpos = p->_7z_currfpos + *pos;
	if (origin==2) p->_7z_currfpos = p->_7z_length - *pos;

	*pos = p->_7z_currfpos;
	
	return 0;
}

WRes File_GetLength(CSzFile *p, UInt64 *length)
{
	*length = p->_7z_length;
	return 0;
}

/* ---------- FileSeqInStream ---------- */

static SRes FileSeqInStream_Read(void *pp, void *buf, size_t *size)
{
  CFileSeqInStream *p = (CFileSeqInStream *)pp;
  return File_Read(&p->file, buf, size) == 0 ? SZ_OK : SZ_ERROR_READ;
}

void FileSeqInStream_CreateVTable(CFileSeqInStream *p)
{
  p->s.Read = FileSeqInStream_Read;
}


/* ---------- FileInStream ---------- */

static SRes FileInStream_Read(void *pp, void *buf, size_t *size)
{
	CFileInStream *p = (CFileInStream *)pp;
	return (File_Read(&p->file, buf, size) == 0) ? SZ_OK : SZ_ERROR_READ;
}

static SRes FileInStream_Seek(void *pp, Int64 *pos, ESzSeek origin)
{
	CFileInStream *p = (CFileInStream *)pp;
	return File_Seek(&p->file, pos, origin);
}

void FileInStream_CreateVTable(CFileInStream *p)
{
	p->s.Read = FileInStream_Read;
	p->s.Seek = FileInStream_Seek;
}

/* ---------- FileOutStream ---------- */

static size_t FileOutStream_Write(void *, const void *, size_t size)
{
//  CFileOutStream *p = (CFileOutStream *)pp;
//  File_Write(&p->file, data, &size);
  return size;
}

void FileOutStream_CreateVTable(CFileOutStream *p)
{
  p->s.Write = FileOutStream_Write;
}

/***************************************************************************
    CONSTANTS
***************************************************************************/

/* number of open files to cache */
#define _7Z_CACHE_SIZE	8


/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static _7z_file *_7z_cache[_7Z_CACHE_SIZE];

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* cache management */
static void free__7z_file(_7z_file *_7z);


/***************************************************************************
    _7Z FILE ACCESS
***************************************************************************/

/*-------------------------------------------------
    _7z_file_open - opens a _7Z file for reading
-------------------------------------------------*/

int _7z_search_crc_match(_7z_file *new_7z, UINT32 search_crc, const char* search_filename, int search_filename_length, bool matchcrc, bool matchname)
{
	UInt16 *temp = NULL;
	size_t tempSize = 0;

	for (unsigned int i = 0; i < new_7z->db.db.NumFiles; i++)
	{
		const CSzFileItem *f = new_7z->db.db.Files + i;
		size_t len;

		len = SzArEx_GetFileNameUtf16(&new_7z->db, i, NULL);

		// if it's a directory entry we don't care about it..
		if (f->IsDir)
			continue;

		if (len > tempSize)
		{
			SZipFree(NULL, temp);
			tempSize = len;
			temp = (UInt16 *)SZipAlloc(NULL, tempSize * sizeof(temp[0]));
			if (temp == 0)
			{
				return -1; // memory error
			}
		}

		bool crcmatch = false;
		bool namematch = false;

		UINT64 size = f->Size;
		UINT32 crc = f->Crc;

		/* Check for a name match */
		SzArEx_GetFileNameUtf16(&new_7z->db, i, temp);

		if (len == (unsigned int)search_filename_length+1)
		{
			int j;
			for (j=0;j<search_filename_length;j++)
			{
				UINT8 sn = search_filename[j];
				UINT16 zn = temp[j]; // these are utf16

				// MAME filenames are always lowercase so be case insensitive
				if ((zn>=0x41) && (zn<=0x5a)) zn+=0x20;

				if (sn != zn) break;
			}		
			if (j==search_filename_length) namematch = true;
		}

		
		/* Check for a CRC match */
		if (crc==search_crc) crcmatch = true;

		bool found = false;

		if (matchcrc && matchname)
		{
			if (crcmatch && namematch)
				found = true;
		}
		else if (matchcrc)
		{
			if (crcmatch)
				found = true;
		}
		else if (matchname)
		{
			if (namematch)
				found = true;
		}

		if (found) 
		{
		//	printf("found %S %d %08x %08x %08x %s %d\n", temp, len, crc, search_crc, size, search_filename, search_filename_length);
			new_7z->curr_file_idx = i;
			new_7z->uncompressed_length = size;
			new_7z->crc = crc;

			SZipFree(NULL, temp);
			return i;
		}
	}

	SZipFree(NULL, temp);
	return -1;
}


_7z_error _7z_file_open(const char *filename, _7z_file **_7z)
{
	_7z_error _7zerr = _7ZERR_NONE;


	_7z_file *new_7z;
	char *string;
	unsigned int cachenum;

	SRes res;

	/* ensure we start with a NULL result */
	*_7z = NULL;

	/* see if we are in the cache, and reopen if so */
	for (cachenum = 0; cachenum < ARRAY_LENGTH(_7z_cache); cachenum++)
	{
		_7z_file *cached = _7z_cache[cachenum];

		/* if we have a valid entry and it matches our filename, use it and remove from the cache */
		if (cached != NULL && cached->filename != NULL && strcmp(filename, cached->filename) == 0)
		{
			*_7z = cached;
			_7z_cache[cachenum] = NULL;
			return _7ZERR_NONE;
		}
	}

	/* allocate memory for the _7z_file structure */
	new_7z = (_7z_file *)malloc(sizeof(*new_7z));
	if (new_7z == NULL)
		return _7ZERR_OUT_OF_MEMORY;
	memset(new_7z, 0, sizeof(*new_7z));

	new_7z->inited = false;
	new_7z->archiveStream.file._7z_currfpos = 0;
	
	new_7z->archiveStream.file._7z_osdfile = fopen(filename, "rb");
	if (!new_7z->archiveStream.file._7z_osdfile) {
		_7zerr = _7ZERR_FILE_ERROR;
		goto error;
	}
	
	fseek(new_7z->archiveStream.file._7z_osdfile, 0, SEEK_END);
	new_7z->archiveStream.file._7z_length = ftell(new_7z->archiveStream.file._7z_osdfile);
	fseek(new_7z->archiveStream.file._7z_osdfile, 0, SEEK_SET);

	new_7z->allocImp.Alloc = SZipAlloc;
	new_7z->allocImp.Free = SZipFree;

	new_7z->allocTempImp.Alloc = SZipAlloc;
	new_7z->allocTempImp.Free = SZipFree;

	if (InFile_Open(&new_7z->archiveStream.file, filename))
	{
		_7zerr = _7ZERR_FILE_ERROR;
		goto error;
	}

	FileInStream_CreateVTable(&new_7z->archiveStream);
	LookToRead_CreateVTable(&new_7z->lookStream, False);
  
	new_7z->lookStream.realStream = &new_7z->archiveStream.s;
	LookToRead_Init(&new_7z->lookStream);

	CrcGenerateTable();

	SzArEx_Init(&new_7z->db);
	new_7z->inited = true;

	res = SzArEx_Open(&new_7z->db, &new_7z->lookStream.s, &new_7z->allocImp, &new_7z->allocTempImp);
	if (res != SZ_OK)
	{
		_7zerr = _7ZERR_FILE_ERROR;
		goto error;
	}

	new_7z->blockIndex = 0xFFFFFFFF; /* it can have any value before first call (if outBuffer = 0) */
	new_7z->outBuffer = 0; /* it must be 0 before first call for each new archive. */
	new_7z->outBufferSize = 0;  /* it can have any value before first call (if outBuffer = 0) */

	/* make a copy of the filename for caching purposes */
	string = (char *)malloc(strlen(filename) + 1);
	if (string == NULL)
	{
		_7zerr = _7ZERR_OUT_OF_MEMORY;
		goto error;
	}
	strcpy(string, filename);
	new_7z->filename = string;
	*_7z = new_7z;
	return _7ZERR_NONE;

error:
	free__7z_file(new_7z);
	return _7zerr;
}


/*-------------------------------------------------
    _7z_file_close - close a _7Z file and add it
    to the cache
-------------------------------------------------*/

void _7z_file_close(_7z_file *_7z)
{
	unsigned int cachenum;

	/* close the open files */
	if (_7z->archiveStream.file._7z_osdfile != NULL)
		fclose(_7z->archiveStream.file._7z_osdfile);
	_7z->archiveStream.file._7z_osdfile = NULL;

	/* find the first NULL entry in the cache */
	for (cachenum = 0; cachenum < ARRAY_LENGTH(_7z_cache); cachenum++)
		if (_7z_cache[cachenum] == NULL)
			break;

	/* if no room left in the cache, free the bottommost entry */
	if (cachenum == ARRAY_LENGTH(_7z_cache))
		free__7z_file(_7z_cache[--cachenum]);

	/* move everyone else down and place us at the top */
	if (cachenum != 0)
		memmove(&_7z_cache[1], &_7z_cache[0], cachenum * sizeof(_7z_cache[0]));
	_7z_cache[0] = _7z;
}


/*-------------------------------------------------
    _7z_file_cache_clear - clear the _7Z file
    cache and free all memory
-------------------------------------------------*/

void _7z_file_cache_clear(void)
{
	unsigned int cachenum;

	/* clear call cache entries */
	for (cachenum = 0; cachenum < ARRAY_LENGTH(_7z_cache); cachenum++)
		if (_7z_cache[cachenum] != NULL)
		{
			free__7z_file(_7z_cache[cachenum]);
			_7z_cache[cachenum] = NULL;
		}
}


/*-------------------------------------------------
    _7z_file_decompress - decompress a file
    from a _7Z into the target buffer
-------------------------------------------------*/

_7z_error _7z_file_decompress(_7z_file *new_7z, void *buffer, UINT32 length, UINT32 *Processed)
{
	SRes res;
	int index = new_7z->curr_file_idx;

	/* make sure the file is open.. */
	if (new_7z->archiveStream.file._7z_osdfile==NULL)
	{
		new_7z->archiveStream.file._7z_currfpos = 0;
		new_7z->archiveStream.file._7z_osdfile = fopen(new_7z->filename, "rb");
		if (!new_7z->archiveStream.file._7z_osdfile) {
			return _7ZERR_FILE_ERROR;
		}
	}

	size_t offset = 0;
	size_t outSizeProcessed = 0;

	res = SzArEx_Extract(&new_7z->db, &new_7z->lookStream.s, index,
		&new_7z->blockIndex, &new_7z->outBuffer, &new_7z->outBufferSize,
		&offset, &outSizeProcessed,
		&new_7z->allocImp, &new_7z->allocTempImp);
			
	if (res != SZ_OK)
		return _7ZERR_FILE_ERROR;
		
	*Processed = outSizeProcessed;

	memcpy(buffer, new_7z->outBuffer + offset, length);

	return _7ZERR_NONE;
}



/***************************************************************************
    CACHE MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    free__7z_file - free all the data for a
    _7z_file
-------------------------------------------------*/

static void free__7z_file(_7z_file *_7z)
{
	if (_7z != NULL)
	{
		if (_7z->archiveStream.file._7z_osdfile != NULL)
			fclose(_7z->archiveStream.file._7z_osdfile);
		if (_7z->filename != NULL)
			free((void *)_7z->filename);


		if (_7z->outBuffer) IAlloc_Free(&_7z->allocImp, _7z->outBuffer);
		if (_7z->inited) SzArEx_Free(&_7z->db, &_7z->allocImp);
	

		free(_7z);
	}
}

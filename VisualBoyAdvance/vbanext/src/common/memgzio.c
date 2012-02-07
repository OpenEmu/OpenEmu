/* gzio.c -- IO on .gz files
 * Copyright (C) 1995-2002 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.h
 *
 * Compile this file with -DNO_DEFLATE to avoid the compression code.
 */

/* memgzio.c - IO on .gz files in memory
 * Adapted from original gzio.c from zlib library by Forgotten
 */

/* @(#) $Id: memgzio.c,v 1.5 2006/06/06 21:04:20 spacy51 Exp $ */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <zlib.h>

#include "memgzio.h"

#ifndef DEF_MEM_LEVEL
#  define DEF_MEM_LEVEL 8
#endif

#ifndef OS_CODE
#define OS_CODE 3
#endif

#ifndef zmemcpy
#define zmemcpy memcpy
#endif


/*struct internal_state {int dummy;};*/ /* for buggy compilers */

#ifndef Z_BUFSIZE
#  ifdef MAXSEG_64K
#    define Z_BUFSIZE 4096 /* minimize memory usage for 16-bit DOS */
#  else
#    define Z_BUFSIZE 16384
#  endif
#endif
#ifndef Z_PRINTF_BUFSIZE
#  define Z_PRINTF_BUFSIZE 4096
#endif

#define ALLOC(size) malloc(size)
#define TRYFREE(p) {if (p) free(p);}

static int gz_magic[2] = {0x1f, 0x8b}; /* gzip magic header */

/* gzip flag byte */
#define ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define HEAD_CRC     0x02 /* bit 1 set: header CRC present */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define RESERVED     0xE0 /* bits 5..7: reserved */

typedef struct _MemFile {
  char *memory;
  char *next;
  int available;
  int error;
  char mode;
} MEMFILE;

typedef struct mem_stream {
    z_stream stream;
    int      z_err;   /* error code for last stream operation */
    int      z_eof;   /* set if end of input file */
    MEMFILE *file;    /* memoru file */
    Byte     *inbuf;  /* input buffer */
    Byte     *outbuf; /* output buffer */
    uLong    crc;     /* crc32 of uncompressed data */
    char     *msg;    /* error message */
    int      transparent; /* 1 if input file is not a .gz file */
    char     mode;    /* 'w' or 'r' */
    long     startpos; /* start of compressed data in file (header skipped) */
} mem_stream;


static gzFile gz_open      OF((char *memory, const int available, const char *mode));
static int do_flush        OF((gzFile file, int flush));
static int    get_byte     OF((mem_stream *s));
static void   check_header OF((mem_stream *s));
static int    destroy      OF((mem_stream *s));
static void   putLong      OF((MEMFILE *file, uLong x));
static uLong  getLong      OF((mem_stream *s));

static MEMFILE *memOpen(char *memory, int available, char mode)
{
  MEMFILE *f;

  if(available <= 8)
    return NULL;

  if(mode != 'w' && mode != 'r')
    return NULL;

  f = (MEMFILE *)malloc(sizeof(MEMFILE));

  f->memory = memory;
  f->mode = mode;
  f->error = 0;

  if(mode == 'w') {
    f->available = available - 8;
    f->next = memory + 8;
    memory[0] = 'V';
    memory[1] = 'B';
    memory[2] = 'A';
    memory[3] = ' ';
    *((int *)(memory+4)) = 0;
  } else {
    if(memory[0] != 'V' || memory[1] != 'B' || memory[2] != 'A' ||
       memory[3] != ' ') {
      free(f);
      return NULL;
    }
    f->available = *((int *)(memory+4));
    f->next = memory+8;
  }

  return f;
}

static size_t memWrite(const void *buffer, size_t size, size_t count,
                      MEMFILE *file)
{
  size_t total = size*count;

  if(file->mode != 'w') {
    file->error = 1;
    return 0;
  }

  if(total > (size_t)file->available) {
    total = file->available;
  }
  memcpy(file->next, buffer, total);
  file->available -= (int)total;
  file->next += total;
  return total;
}

static size_t memRead(void *buffer, size_t size, size_t count,
                     MEMFILE *file)
{
  size_t total = size*count;

  if(file->mode != 'r') {
    file->error = 1;
    return 0;
  }

  if(file->available == 0)
    return -1;

  if(total > (size_t)file->available) {
    total = file->available;
  }
  memcpy(buffer, file->next, total);
  file->available -= (int)total;
  file->next += total;
  return total;
}

static int memPutc(int c, MEMFILE *file)
{
  if(file->mode != 'w') {
    file->error = 1;
    return -1;
  }

  if(file->available >= 1) {
    *file->next++ = c;
    file->available--;
  } else
    return -1;

  return c;
}

static long memTell(MEMFILE *f)
{
  return (long)(f->next - f->memory) - 8;
}

static int memError(MEMFILE *f)
{
  return f->error;
}

static int memClose(MEMFILE *f)
{
  if(f->mode == 'w') {
    *((int *)(f->memory+4)) = memTell(f);
  }
  free(f);
  return 0;
}

static int memPrintf(MEMFILE *f, const char *format, ...)
{
  char buffer[80];
  va_list list;
  int len;

  va_start(list, format);
  len = vsprintf(buffer, format, list);
  va_end(list);

  return (int)memWrite(buffer, 1, len, f);
}

/* ===========================================================================
     Opens a gzip (.gz) file for reading or writing. The mode parameter
   is as in fopen ("rb" or "wb"). The file is given either by file descriptor
   or path name (if fd == -1).
     gz_open return NULL if the file could not be opened or if there was
   insufficient memory to allocate the (de)compression state; errno
   can be checked to distinguish the two cases (if errno is zero, the
   zlib error is Z_MEM_ERROR).
*/
static gzFile gz_open (memory, available, mode)
    char *memory;
    const int available;
    const char *mode;
{
    int err;
    int level = Z_DEFAULT_COMPRESSION; /* compression level */
    int strategy = Z_DEFAULT_STRATEGY; /* compression strategy */
    char *p = (char*)mode;
    mem_stream *s;
    char fmode[80]; /* copy of mode, without the compression level */
    char *m = fmode;

    s = (mem_stream *)ALLOC(sizeof(mem_stream));
    if (!s) return Z_NULL;

    s->stream.zalloc = (alloc_func)0;
    s->stream.zfree = (free_func)0;
    s->stream.opaque = (voidpf)0;
    s->stream.next_in = s->inbuf = Z_NULL;
    s->stream.next_out = s->outbuf = Z_NULL;
    s->stream.avail_in = s->stream.avail_out = 0;
    s->z_err = Z_OK;
    s->z_eof = 0;
    s->crc = crc32(0L, Z_NULL, 0);
    s->msg = NULL;
    s->transparent = 0;
    s->file = NULL;

    s->mode = '\0';
    do {
        if (*p == 'r') s->mode = 'r';
        if (*p == 'w' || *p == 'a') s->mode = 'w';
        if (*p >= '0' && *p <= '9') {
	    level = *p - '0';
	} else if (*p == 'f') {
	  strategy = Z_FILTERED;
	} else if (*p == 'h') {
	  strategy = Z_HUFFMAN_ONLY;
	} else {
	    *m++ = *p; /* copy the mode */
	}
    } while (*p++ && m != fmode + sizeof(fmode));
    if (s->mode == '\0') return destroy(s), (gzFile)Z_NULL;

    if (s->mode == 'w') {
#ifdef NO_DEFLATE
        err = Z_STREAM_ERROR;
#else
        err = deflateInit2(&(s->stream), level,
                           Z_DEFLATED, -MAX_WBITS, DEF_MEM_LEVEL, strategy);
        /* windowBits is passed < 0 to suppress zlib header */

        s->stream.next_out = s->outbuf = (Byte*)ALLOC(Z_BUFSIZE);
#endif
        if (err != Z_OK || s->outbuf == Z_NULL) {
            return destroy(s), (gzFile)Z_NULL;
        }
    } else {
        s->stream.next_in  = s->inbuf = (Byte*)ALLOC(Z_BUFSIZE);

        err = inflateInit2(&(s->stream), -MAX_WBITS);
        /* windowBits is passed < 0 to tell that there is no zlib header.
         * Note that in this case inflate *requires* an extra "dummy" byte
         * after the compressed stream in order to complete decompression and
         * return Z_STREAM_END. Here the gzip CRC32 ensures that 4 bytes are
         * present after the compressed stream.
         */
        if (err != Z_OK || s->inbuf == Z_NULL) {
            return destroy(s), (gzFile)Z_NULL;
        }
    }
    s->stream.avail_out = Z_BUFSIZE;

    errno = 0;
    s->file = memOpen(memory, available, s->mode);

    if (s->file == NULL) {
        return destroy(s), (gzFile)Z_NULL;
    }

    if (s->mode == 'w') {
        /* Write a very simple .gz header:
         */
        memPrintf(s->file, "%c%c%c%c%c%c%c%c%c%c", gz_magic[0], gz_magic[1],
             Z_DEFLATED, 0 /*flags*/, 0,0,0,0 /*time*/, 0 /*xflags*/, OS_CODE);
	s->startpos = 10L;
	/* We use 10L instead of ftell(s->file) to because ftell causes an
         * fflush on some systems. This version of the library doesn't use
         * startpos anyway in write mode, so this initialization is not
         * necessary.
         */
    } else {
	check_header(s); /* skip the .gz header */
	s->startpos = (memTell(s->file) - s->stream.avail_in);
    }

    return (gzFile)s;
}

/* ===========================================================================
     Opens a gzip (.gz) file for reading or writing.
*/
gzFile ZEXPORT memgzopen (memory, available, mode)
    char *memory;
    int available;
    const char *mode;
{
    return gz_open (memory, available, mode);
}

/* ===========================================================================
     Read a byte from a mem_stream; update next_in and avail_in. Return EOF
   for end of file.
   IN assertion: the stream s has been sucessfully opened for reading.
*/
static int get_byte(s)
    mem_stream *s;
{
    if (s->z_eof) return EOF;
    if (s->stream.avail_in == 0) {
	errno = 0;
	s->stream.avail_in = (uInt)memRead(s->inbuf, 1, Z_BUFSIZE, s->file);
	if (s->stream.avail_in == 0) {
	    s->z_eof = 1;
	    if (memError(s->file)) s->z_err = Z_ERRNO;
	    return EOF;
	}
	s->stream.next_in = s->inbuf;
    }
    s->stream.avail_in--;
    return *(s->stream.next_in)++;
}

/* ===========================================================================
      Check the gzip header of a mem_stream opened for reading. Set the stream
    mode to transparent if the gzip magic header is not present; set s->err
    to Z_DATA_ERROR if the magic header is present but the rest of the header
    is incorrect.
    IN assertion: the stream s has already been created sucessfully;
       s->stream.avail_in is zero for the first time, but may be non-zero
       for concatenated .gz files.
*/
static void check_header(s)
    mem_stream *s;
{
    int method; /* method byte */
    int flags;  /* flags byte */
    uInt len;
    int c;

    /* Check the gzip magic header */
    for (len = 0; len < 2; len++) {
	c = get_byte(s);
	if (c != gz_magic[len]) {
	    if (len != 0) s->stream.avail_in++, s->stream.next_in--;
	    if (c != EOF) {
		s->stream.avail_in++, s->stream.next_in--;
		s->transparent = 1;
	    }
	    s->z_err = s->stream.avail_in != 0 ? Z_OK : Z_STREAM_END;
	    return;
	}
    }
    method = get_byte(s);
    flags = get_byte(s);
    if (method != Z_DEFLATED || (flags & RESERVED) != 0) {
	s->z_err = Z_DATA_ERROR;
	return;
    }

    /* Discard time, xflags and OS code: */
    for (len = 0; len < 6; len++) (void)get_byte(s);

    if ((flags & EXTRA_FIELD) != 0) { /* skip the extra field */
	len  =  (uInt)get_byte(s);
	len += ((uInt)get_byte(s))<<8;
	/* len is garbage if EOF but the loop below will quit anyway */
	while (len-- != 0 && get_byte(s) != EOF) ;
    }
    if ((flags & ORIG_NAME) != 0) { /* skip the original file name */
	while ((c = get_byte(s)) != 0 && c != EOF) ;
    }
    if ((flags & COMMENT) != 0) {   /* skip the .gz file comment */
	while ((c = get_byte(s)) != 0 && c != EOF) ;
    }
    if ((flags & HEAD_CRC) != 0) {  /* skip the header crc */
	for (len = 0; len < 2; len++) (void)get_byte(s);
    }
    s->z_err = s->z_eof ? Z_DATA_ERROR : Z_OK;
}

 /* ===========================================================================
 * Cleanup then free the given mem_stream. Return a zlib error code.
   Try freeing in the reverse order of allocations.
 */
static int destroy (s)
    mem_stream *s;
{
    int err = Z_OK;

    if (!s) return Z_STREAM_ERROR;

    TRYFREE(s->msg);

    if (s->stream.state != NULL) {
	if (s->mode == 'w') {
#ifdef NO_DEFLATE
	    err = Z_STREAM_ERROR;
#else
	    err = deflateEnd(&(s->stream));
#endif
	} else if (s->mode == 'r') {
	    err = inflateEnd(&(s->stream));
	}
    }
    if (s->file != NULL && memClose(s->file)) {
#ifdef ESPIPE
	if (errno != ESPIPE) /* fclose is broken for pipes in HP/UX */
#endif
	    err = Z_ERRNO;
    }
    if (s->z_err < 0) err = s->z_err;

    TRYFREE(s->inbuf);
    TRYFREE(s->outbuf);
    TRYFREE(s);
    return err;
}

/* ===========================================================================
     Reads the given number of uncompressed bytes from the compressed file.
   gzread returns the number of bytes actually read (0 for end of file).
*/
int ZEXPORT memgzread (file, buf, len)
    gzFile file;
    voidp buf;
    unsigned len;
{
    mem_stream *s = (mem_stream*)file;
    Bytef *start = (Bytef*)buf; /* starting point for crc computation */
    Byte  *next_out; /* == stream.next_out but not forced far (for MSDOS) */

    if (s == NULL || s->mode != 'r') return Z_STREAM_ERROR;

    if (s->z_err == Z_DATA_ERROR || s->z_err == Z_ERRNO) return -1;
    if (s->z_err == Z_STREAM_END) return 0;  /* EOF */

    next_out = (Byte*)buf;
    s->stream.next_out = (Bytef*)buf;
    s->stream.avail_out = len;

    while (s->stream.avail_out != 0) {

	if (s->transparent) {
	    /* Copy first the lookahead bytes: */
	    uInt n = s->stream.avail_in;
	    if (n > s->stream.avail_out) n = s->stream.avail_out;
	    if (n > 0) {
		zmemcpy(s->stream.next_out, s->stream.next_in, n);
		next_out += n;
		s->stream.next_out = next_out;
		s->stream.next_in   += n;
		s->stream.avail_out -= n;
		s->stream.avail_in  -= n;
	    }
	    if (s->stream.avail_out > 0) {
		s->stream.avail_out -= (uInt)memRead(next_out, 1, s->stream.avail_out, s->file);
	    }
	    len -= s->stream.avail_out;
	    s->stream.total_in  += (uLong)len;
	    s->stream.total_out += (uLong)len;
            if (len == 0) s->z_eof = 1;
	    return (int)len;
	}
        if (s->stream.avail_in == 0 && !s->z_eof) {

            errno = 0;
            s->stream.avail_in = (uInt)memRead(s->inbuf, 1, Z_BUFSIZE, s->file);
            if (s->stream.avail_in == 0) {
                s->z_eof = 1;
		if (memError(s->file)) {
		    s->z_err = Z_ERRNO;
		    break;
		}
            }
            s->stream.next_in = s->inbuf;
        }
        s->z_err = inflate(&(s->stream), Z_NO_FLUSH);

	if (s->z_err == Z_STREAM_END) {
	    /* Check CRC and original size */
	    s->crc = crc32(s->crc, start, (uInt)(s->stream.next_out - start));
	    start = s->stream.next_out;

	    if (getLong(s) != s->crc) {
		s->z_err = Z_DATA_ERROR;
	    } else {
	        (void)getLong(s);
                /* The uncompressed length returned by above getlong() may
                 * be different from s->stream.total_out) in case of
		 * concatenated .gz files. Check for such files:
		 */
		check_header(s);
		if (s->z_err == Z_OK) {
		    uLong total_in = s->stream.total_in;
		    uLong total_out = s->stream.total_out;

		    inflateReset(&(s->stream));
		    s->stream.total_in = total_in;
		    s->stream.total_out = total_out;
		    s->crc = crc32(0L, Z_NULL, 0);
		}
	    }
	}
	if (s->z_err != Z_OK || s->z_eof) break;
    }
    s->crc = crc32(s->crc, start, (uInt)(s->stream.next_out - start));

    return (int)(len - s->stream.avail_out);
}


#ifndef NO_DEFLATE
/* ===========================================================================
     Writes the given number of uncompressed bytes into the compressed file.
   gzwrite returns the number of bytes actually written (0 in case of error).
*/
int ZEXPORT memgzwrite (file, buf, len)
    gzFile file;
    const voidp buf;
    unsigned len;
{
    mem_stream *s = (mem_stream*)file;

    if (s == NULL || s->mode != 'w') return Z_STREAM_ERROR;

    s->stream.next_in = (Bytef*)buf;
    s->stream.avail_in = len;

    while (s->stream.avail_in != 0) {

        if (s->stream.avail_out == 0) {

            s->stream.next_out = s->outbuf;
            if (memWrite(s->outbuf, 1, Z_BUFSIZE, s->file) != Z_BUFSIZE) {
                s->z_err = Z_ERRNO;
                break;
            }
            s->stream.avail_out = Z_BUFSIZE;
        }
        s->z_err = deflate(&(s->stream), Z_NO_FLUSH);
        if (s->z_err != Z_OK) break;
    }
    s->crc = crc32(s->crc, (const Bytef *)buf, len);

    return (int)(len - s->stream.avail_in);
}
#endif
/* ===========================================================================
     Flushes all pending output into the compressed file. The parameter
   flush is as in the deflate() function.
*/
static int do_flush (file, flush)
    gzFile file;
    int flush;
{
    uInt len;
    int done = 0;
    mem_stream *s = (mem_stream*)file;

    if (s == NULL || s->mode != 'w') return Z_STREAM_ERROR;

    s->stream.avail_in = 0; /* should be zero already anyway */

    for (;;) {
        len = Z_BUFSIZE - s->stream.avail_out;

        if (len != 0) {
            if ((uInt)memWrite(s->outbuf, 1, len, s->file) != len) {
                s->z_err = Z_ERRNO;
                return Z_ERRNO;
            }
            s->stream.next_out = s->outbuf;
            s->stream.avail_out = Z_BUFSIZE;
        }
        if (done) break;
        s->z_err = deflate(&(s->stream), flush);

	/* Ignore the second of two consecutive flushes: */
	if (len == 0 && s->z_err == Z_BUF_ERROR) s->z_err = Z_OK;

        /* deflate has finished flushing only when it hasn't used up
         * all the available space in the output buffer:
         */
        done = (s->stream.avail_out != 0 || s->z_err == Z_STREAM_END);

        if (s->z_err != Z_OK && s->z_err != Z_STREAM_END) break;
    }
    return  s->z_err == Z_STREAM_END ? Z_OK : s->z_err;
}

/* ===========================================================================
   Outputs a long in LSB order to the given file
*/
static void putLong (file, x)
    MEMFILE *file;
    uLong x;
{
    int n;
    for (n = 0; n < 4; n++) {
        memPutc((int)(x & 0xff), file);
        x >>= 8;
    }
}

/* ===========================================================================
   Reads a long in LSB order from the given mem_stream. Sets z_err in case
   of error.
*/
static uLong getLong (s)
    mem_stream *s;
{
    uLong x = (uLong)get_byte(s);
    int c;

    x += ((uLong)get_byte(s))<<8;
    x += ((uLong)get_byte(s))<<16;
    c = get_byte(s);
    if (c == EOF) s->z_err = Z_DATA_ERROR;
    x += ((uLong)c)<<24;
    return x;
}

/* ===========================================================================
     Flushes all pending output if necessary, closes the compressed file
   and deallocates all the (de)compression state.
*/
int ZEXPORT memgzclose (file)
    gzFile file;
{
    int err;
    mem_stream *s = (mem_stream*)file;

    if (s == NULL) return Z_STREAM_ERROR;

    if (s->mode == 'w') {
#ifdef NO_DEFLATE
return Z_STREAM_ERROR;
#else
err = do_flush (file, Z_FINISH);
if (err != Z_OK) return destroy((mem_stream*)file);

putLong (s->file, s->crc);
putLong (s->file, s->stream.total_in);
#endif
    }
    return destroy((mem_stream*)file);
}

long ZEXPORT memtell(file)
     gzFile file;
{
    mem_stream *s = (mem_stream*)file;

    if (s == NULL) return Z_STREAM_ERROR;

    return memTell(s->file);
}

/*
 * PKWare Zip Header - adopted into zip standard
 */
#define PKZIPID 0x504b0304
//#define MAXROM 0x500000
#define ZIPCHUNK 2048
#define MAXROMSIZE 64*1024*1024

/*
 * Zip file header definition
 */
typedef struct
{
  unsigned int zipid;  /* 0x04034b50 */
  unsigned short zipversion;
  unsigned short zipflags;
  unsigned short compressionMethod;
  unsigned short lastmodtime;
  unsigned short lastmoddate;
  unsigned int crc32;
  unsigned int compressedSize;
  unsigned int uncompressedSize;
  unsigned short filenameLength;
  unsigned short extraDataLength;
} PKZIPHEADER;

/*
 * Zip files are stored little endian
 * Support functions for short and int types
 */
static unsigned int FLIP32 (unsigned int b)
{
  unsigned int c;
  c = (b & 0xff000000) >> 24;
  c |= (b & 0xff0000) >> 8;
  c |= (b & 0xff00) << 8;
  c |= (b & 0xff) << 24;
  return c;
}

static unsigned short FLIP16 (unsigned short b)
{
  unsigned short c;
  c = (b & 0xff00) >> 8;
  c |= (b & 0xff) << 8;
  return c;
}

/****************************************************************************
 * IsZipFile
 *
 * Returns TRUE when PKZIPID is first four characters of buffer
 ****************************************************************************/
int IsZipFile (char *buffer)
{
	unsigned int *check;
	check = (unsigned int *) buffer;
	if (check[0] == PKZIPID) return 1;
	return 0;
}

void get_zipfilename(char * filename, char * outfilename)
{
	char in[ZIPCHUNK];

	/* Open file */
	FILE *fd = fopen(filename, "rb");
	if (!fd) return;

	/* Read first 2 bytes  */
	fread(in, 2, 1, fd);

	/* Detect Zip file */
	if (memcmp(in, "PK", 2) == 0)
	{
		int len;
		PKZIPHEADER *pkzip;
		/* Read remaining header data */
		fread(in + 2, sizeof(PKZIPHEADER) - 2, 1, fd);

		/* Zip header pointer */
		pkzip = (PKZIPHEADER *) in;

		/* Return compressed file name */
		len = FLIP16(pkzip->filenameLength);
		if (len >= 1024) len = 1024 - 1;
		fread(outfilename, len, 1, fd);
		outfilename[len] = 0;
	}

	fclose(fd);
}


/*****************************************************************************
 * UnZipBuffer
 *
 * It should be noted that there is a limit of 5MB total size for any ROM
 ******************************************************************************/
int UnZipBuffer (unsigned char * outbuffer, FILE * fd)
{
	PKZIPHEADER pkzip;
	int zipoffset = 0;
	int zipchunk = 0;
	char out[ZIPCHUNK];
	z_stream zs;
	int res;
	int bufferoffset = 0;
	int have = 0;
	char readbuffer[ZIPCHUNK];
	char msg[64];

	/*** Read Zip Header ***/
	fread(readbuffer, ZIPCHUNK,  1, fd);

	/*** Copy PKZip header to static, used as info ***/
	memcpy (&pkzip, &readbuffer, sizeof (PKZIPHEADER));

	if (FLIP32 (pkzip.uncompressedSize) > MAXROMSIZE)
	{
		printf("Error","File is too large !");
		return 0;
	}

	sprintf (msg, "Unzipping %d bytes ...", FLIP32 (pkzip.uncompressedSize));
	printf("Information",msg,1);

	/*** Prepare the zip stream ***/
	memset (&zs, 0, sizeof (z_stream));
	zs.zalloc = Z_NULL;
	zs.zfree = Z_NULL;
	zs.opaque = Z_NULL;
	zs.avail_in = 0;
	zs.next_in = Z_NULL;
	res = inflateInit2 (&zs, -MAX_WBITS);

	if (res != Z_OK)
	{
		printf("Error","Unable to unzip file !");
		return 0;
	}

	/*** Set ZipChunk for first pass ***/
	zipoffset = (sizeof (PKZIPHEADER) + FLIP16 (pkzip.filenameLength) + FLIP16 (pkzip.extraDataLength));
	zipchunk = ZIPCHUNK - zipoffset;

	/*** Now do it! ***/
	do
	{
		zs.avail_in = zipchunk;
		zs.next_in = (Bytef *) & readbuffer[zipoffset];

		/*** Now inflate until input buffer is exhausted ***/
		do
		{
			zs.avail_out = ZIPCHUNK;
			zs.next_out = (Bytef *) & out;
			res = inflate (&zs, Z_NO_FLUSH);

			if (res == Z_MEM_ERROR)
			{
				inflateEnd (&zs);
				printf("Error","Unable to unzip file !");
				return 0;
			}

			have = ZIPCHUNK - zs.avail_out;
			if (have)
			{
				/*** Copy to normal block buffer ***/
				memcpy (&outbuffer[bufferoffset], &out, have);
				bufferoffset += have;
			}
		}
		while (zs.avail_out == 0);

		/*** Readup the next 2k block ***/
		zipoffset = 0;
		zipchunk = ZIPCHUNK;
		fread(readbuffer, ZIPCHUNK, 1, fd);
	}
	while (res != Z_STREAM_END);

	inflateEnd (&zs);

	if (res == Z_STREAM_END)
	{
		if (FLIP32 (pkzip.uncompressedSize) == (unsigned int) bufferoffset)
			return bufferoffset;
		else
			return FLIP32 (pkzip.uncompressedSize);
	}

	return 0;
}

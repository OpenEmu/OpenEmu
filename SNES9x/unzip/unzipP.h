#ifndef _UNZIPP_H_
#define _UNZIPP_H_

#include "unzip.h"

#ifndef local
#define local static
#endif
/* compile with -Dlocal if your debugger can't find static symbols */



#if !defined(unix) && !defined(CASESENSITIVITYDEFAULT_YES) && \
                      !defined(CASESENSITIVITYDEFAULT_NO)
#define CASESENSITIVITYDEFAULT_NO
#endif


#ifndef UNZ_BUFSIZE
#define UNZ_BUFSIZE (16384)
#endif

#ifndef UNZ_MAXFILENAMEINZIP
#define UNZ_MAXFILENAMEINZIP (256)
#endif

#ifndef ALLOC
#define ALLOC(size) (malloc(size))
#endif
#ifndef TRYFREE
#define TRYFREE(p) {if (p) free(p);}
#endif

#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)


/* I've found an old Unix (a SunOS 4.1.3_U1) without all SEEK_* defined.... */

#ifndef SEEK_CUR
#define SEEK_CUR    1
#endif

#ifndef SEEK_END
#define SEEK_END    2
#endif

#ifndef SEEK_SET
#define SEEK_SET    0
#endif

/* unz_file_info_interntal contain internal info about a file in zipfile */
typedef struct unz_file_info_internal_s
{
    uLong           offset_curfile;	/* relative offset of local header 4
					 * bytes */
}               unz_file_info_internal;


/*
 * file_in_zip_read_info_s contain internal information about a file in
 * zipfile, when reading and decompress it
 */
typedef struct
{
    char           *read_buffer;/* internal buffer for compressed data */
    z_stream        stream;	/* zLib stream structure for inflate */

    uLong           pos_in_zipfile;	/* position in byte on the zipfile,
					 * for fseek */
    uLong           stream_initialised;	/* flag set if stream structure is
					 * initialised */

    uLong           offset_local_extrafield;	/* offset of the local extra
						 * field */
    uInt            size_local_extrafield;	/* size of the local extra
						 * field */
    uLong           pos_local_extrafield;	/* position in the local
						 * extra field in read */

    uLong           crc32;	/* crc32 of all data uncompressed */
    uLong           crc32_wait;	/* crc32 we must obtain after decompress all */
    uLong           rest_read_compressed;	/* number of byte to be
						 * decompressed */
    uLong           rest_read_uncompressed;	/* number of byte to be
						 * obtained after decomp */
    FILE           *file;	/* io structore of the zipfile */
    uLong           compression_method;	/* compression method (0==store) */
    uLong           byte_before_the_zipfile;	/* byte before the zipfile,
						 * (>0 for sfx) */
}               file_in_zip_read_info_s;


/*
 * unz_s contain internal information about the zipfile
 */
typedef struct
{
    FILE           *file;	/* io structore of the zipfile */
    unz_global_info gi;		/* public global information */
    uLong           byte_before_the_zipfile;	/* byte before the zipfile,
						 * (>0 for sfx) */
    uLong           num_file;	/* number of the current file in the zipfile */
    uLong           pos_in_central_dir;	/* pos of the current file in the
					 * central dir */
    uLong           current_file_ok;	/* flag about the usability of the
					 * current file */
    uLong           central_pos;/* position of the beginning of the central
				 * dir */

    uLong           size_central_dir;	/* size of the central directory  */
    uLong           offset_central_dir;	/* offset of start of central
					 * directory with respect to the
					 * starting disk number */

    unz_file_info   cur_file_info;	/* public info about the current file
					 * in zip */
    unz_file_info_internal cur_file_info_internal;	/* private info about it */
    file_in_zip_read_info_s *pfile_in_zip_read;	/* structure about the
						 * current file if we are
						 * decompressing it */
}               unz_s;

#endif

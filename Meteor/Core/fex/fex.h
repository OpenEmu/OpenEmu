/** Uniform access to zip, gzip, 7-zip, and RAR compressed archives \file */

/* File_Extractor 1.0.0 */
#ifndef FEX_H
#define FEX_H

#include <stddef.h>

#ifdef __cplusplus
	extern "C" {
#endif


/** First parameter of most functions is fex_t*, or const fex_t* if nothing is
changed. Once one of these functions returns an error, the archive should not
be used any further, other than to close it. One exception is
fex_error_file_eof; the archive may still be used after this. */
typedef struct fex_t fex_t;

/** Pointer to error, or NULL if function was successful. See error functions
below. */
#ifndef fex_err_t /* (#ifndef allows better testing of library) */
	typedef const char* fex_err_t;
#endif


/**** File types ****/

/** Archive file type identifier. Can also hold NULL. */
typedef const struct fex_type_t_* fex_type_t;

/** Array of supported types, with NULL at end */
const fex_type_t* fex_type_list( void );

/** Name of this archive type, e.g. "ZIP archive", "file" */
const char* fex_type_name( fex_type_t );

/** Usual file extension for type, e.g. ".zip", ".7z". For binary file type,
returns "", since it can open any file. */
const char* fex_type_extension( fex_type_t );


/**** Wide-character file paths ****/

/** Converts wide-character path to form suitable for use with fex functions. */
char* fex_wide_to_path( const wchar_t* wide );

/** Frees converted path. OK to pass NULL. */
void fex_free_path( char* );


/**** Identification ****/

/** True if str ends in extension. If extension is "", always returns true.
Converts str to lowercase before comparison, so extension should ALREADY be
lowercase (i.e. pass ".zip", NOT ".ZIP"). */
int fex_has_extension( const char str [], const char extension [] );

/** Determines type based on first fex_identify_header_size bytes of file.
Returns usual file extension this should have (e.g. ".zip", ".gz", etc.).
Returns "" if file header is not recognized. */
const char* fex_identify_header( const void* header );
enum { fex_identify_header_size = 16 };

/** Determines type based on extension of a file path, or just a lone extension
(must include '.', e.g. ".zip", NOT just "zip"). Returns NULL if extension is
for an unsupported type (e.g. ".lzh"). */
fex_type_t fex_identify_extension( const char path_or_extension [] );

/** Determines type based on filename extension and/or file header. Sets *out
to determined type, or NULL if type is not supported. */
fex_err_t fex_identify_file( fex_type_t* out, const char path [] );

/** Type of an already-opened archive */
fex_type_t fex_type( const fex_t* );


/**** Open/close ****/

/** Initializes static tables used by library. Automatically called by
fex_open(). OK to call more than once. */
fex_err_t fex_init( void );

/** Opens archive and points *out at it. If error, sets *out to NULL. */
fex_err_t fex_open( fex_t** out, const char path [] );

/** Opens archive of specified type and sets *out. Returns error if file is not
of that archive type. If error, sets *out to NULL. */
fex_err_t fex_open_type( fex_t** out, const char path [], fex_type_t );

/** Closes archive and frees memory. OK to pass NULL. */
void fex_close( fex_t* );


/**** Scanning ****/

/** True if at end of archive. Must be called after fex_open() or fex_rewind(),
as an archive might contain no files. */
int fex_done( const fex_t* );

/** Goes to next file in archive. If there are no more files, fex_done() will
now return true. */
fex_err_t fex_next( fex_t* );

/** Goes back to first file in archive, as if it were just opened with
fex_open() */
fex_err_t fex_rewind( fex_t* );

/** Saved position in archive. Can also store zero. */
typedef int fex_pos_t;

/** Position of current file in archive. Never returns zero. */
fex_pos_t fex_tell_arc( const fex_t* );

/** Returns to file at previously-saved position */
fex_err_t fex_seek_arc( fex_t*, fex_pos_t );


/**** Info ****/

/** Name of current file */
const char* fex_name( const fex_t* );

/** Wide-character name of current file, or NULL if unavailable */
const wchar_t* fex_wname( const fex_t* );

/** Makes further information available for file */
fex_err_t fex_stat( fex_t* );

/** Size of current file. fex_stat() or fex_data() must have been called. */
int fex_size( const fex_t* );

/** Modification date of current file (MS-DOS format), or 0 if unavailable.
fex_stat() must have been called. */
unsigned int fex_dos_date( const fex_t* );

/** CRC-32 checksum of current file's contents, or 0 if unavailable. Doesn't
require calculation; simply gets it from file's header. fex_stat() must have
been called. */
unsigned int fex_crc32( const fex_t* );


/**** Extraction ****/

/** Reads n bytes from current file. Reading past end of file results in
fex_err_file_eof. */
fex_err_t fex_read( fex_t*, void* out, int n );

/** Number of bytes read from current file */
int fex_tell( const fex_t* );

/** Points *out at current file's data in memory. Pointer is valid until
fex_next(), fex_rewind(), fex_seek_arc(), or fex_close() is called. Pointer
must NOT be freed(); library frees it automatically. If error, sets *out to
NULL. */
fex_err_t fex_data( fex_t*, const void** out );


/**** Errors ****/

/** Error string associated with err. Returns "" if err is NULL. Returns err
unchanged if it isn't a fex_err_t returned by library. */
const char* fex_err_str( fex_err_t err );

/** Details of error beyond main cause, or "" if none or err is NULL. Returns
err unchanged if it isn't a fex_err_t returned by library. */
const char* fex_err_details( fex_err_t err );

/** Numeric code corresponding to err. Returns fex_ok if err is NULL. Returns
fex_err_generic if err isn't a fex_err_t returned by library. */
int fex_err_code( fex_err_t err );

enum {
	fex_ok               =    0,/**< Successful call. Guaranteed to be zero. */
	fex_err_generic      = 0x01,/**< Error of unspecified type */
	fex_err_memory       = 0x02,/**< Out of memory */
	fex_err_caller       = 0x03,/**< Caller called function with bad args */
	fex_err_internal     = 0x04,/**< Internal problem, bug, etc. */
	fex_err_limitation   = 0x05,/**< Exceeded program limit */
	
	fex_err_file_missing = 0x20,/**< File not found at specified path */
	fex_err_file_read    = 0x21,/**< Couldn't open file for reading */
	fex_err_file_io      = 0x23,/**< Read/write error */
	fex_err_file_eof     = 0x25,/**< Tried to read past end of file */
	
	fex_err_file_type    = 0x30,/**< File is of wrong type */
	fex_err_file_feature = 0x32,/**< File requires unsupported feature */
	fex_err_file_corrupt = 0x33 /**< File is corrupt */
};

/** fex_err_t corresponding to numeric code. Note that this might not recover
the original fex_err_t before it was converted to a numeric code; in
particular, fex_err_details(fex_code_to_err(code)) will be "" in most cases. */
fex_err_t fex_code_to_err( int code );


/* Deprecated */
typedef fex_t File_Extractor;

#ifdef __cplusplus
	}
#endif

#endif

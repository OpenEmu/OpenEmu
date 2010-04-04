/* Compressed file archive C interface (also usable from C++) */

/* File_Extractor 0.4.3 */
#ifndef FEX_H
#define FEX_H

#ifdef __cplusplus
	extern "C" {
#endif

/* Error string returned by library functions, or NULL if no error (success).
If function takes fex_err_t* err_out, it sets *err_out to NULL on success,
otherwise error string, or you can pass NULL if you don't care about exact
cause of error (these functions still report error by returning NULL). */
typedef const char* fex_err_t;

/* First parameter of most extractor_ functions is a pointer to the
File_Extractor being acted on. */
typedef struct File_Extractor File_Extractor;


/**** Basics ****/

/* Opens archive and returns pointer to it, or NULL if error. */
File_Extractor* fex_open( const char* path, fex_err_t* err_out );

/* True if at end of archive. */
int fex_done( File_Extractor const* );

/* Name of current file. */
const char* fex_name( File_Extractor* );

/* Size of current file. */
long fex_size( File_Extractor const* );

/* Extracts n bytes and writes them to *out. Returns error if all n
bytes couldn't be extracted (due to end of file or read error). */
fex_err_t fex_read( File_Extractor*, void* out, long n );

/* Goes to next file in archive (skips directories). */
fex_err_t fex_next( File_Extractor* );

/* Closes archive and frees memory. */
void fex_close( File_Extractor* );


/**** Advanced ****/

/* Goes back to first file in archive. */
fex_err_t fex_rewind( File_Extractor* );

/* Hints to fex_next() that no file extraction will occur, speeding scanning
of some archive types. */
void fex_scan_only( File_Extractor* );

/* Modification date of current file (MS-DOS format). */
unsigned long fex_dos_date( File_Extractor const* );

/* Number of bytes remaining to be read from current file. */
long fex_remain( File_Extractor const* );

/* Reads at most n bytes and returns number actually read, or negative if error. */
long fex_read_avail( File_Extractor*, void* out, long n );

/* Extracts first n bytes and ignores rest. Faster than a normal read since it
doesn't need to read any more data. Must not be called twice in a row. */
fex_err_t fex_read_once( File_Extractor*, void* out, long n );

/* Loads file data into memory (if not already) and returns pointer to it, or
NULL if error. Pointer is valid until fex_next(), fex_rewind(), or fex_close() are
called. Will return same pointer if called more than once. */
const unsigned char* fex_data( File_Extractor*, fex_err_t* err_out );


/**** Archive types ****/

/* fex_type_t is a pointer to this structure. For example, fex_zip_type->extension is
"ZIP" and ex_zip_type->new_fex() is equilvant to 'new Zip_Extractor' (in C++). */
struct fex_type_t_
{
	const char* extension;      /* file extension/type */
	File_Extractor* (*new_fex)();
};

/* Archive type constants for each supported file type */
extern struct fex_type_t_ const
		fex_7z_type  [1],   /* .7z (7-zip) */
		fex_gz_type  [1],   /* .gz (gzip) */
		/*fex_rar_type [1],*/   /* .rar */
		fex_zip_type [1],   /* .zip */
		fex_bin_type [1];   /* binary file, possibly gzipped */
typedef struct fex_type_t_ const* fex_type_t;

/* Array of supported archive types, with NULL entry at end. */
fex_type_t const* fex_type_list();

/* Type of archive this extractor handles. */
fex_type_t fex_type( File_Extractor const* );


/******** Advanced opening ********/

/* Error returned if file is wrong type */
extern const char fex_wrong_file_type [29];

/* Determines likely archive type based on first four bytes of file. Returns string
containing proper file suffix (i.e. "ZIP", "GZ", etc.) or "" (empty string) if file
header is not recognized. */
const char* fex_identify_header( void const* header );

/* Gets corresponding archive type for file path or extension passed in. Returns NULL
if type isn't recognized. */
fex_type_t fex_identify_extension( const char* path_or_extension );

/* Determines file type based on filename extension, or file header (if extension
isn't recognized). Returns NULL if unrecognized or error. */
fex_type_t fex_identify_file( const char* path, fex_err_t* err_out );

/* Opens archive of specific type and returns pointer to it, or NULL if error. */
File_Extractor* fex_open_type( fex_type_t, const char* path, fex_err_t* err_out );


/******** User data ********/

/* Sets/gets pointer to data you want to associate with this extractor.
You can use this for whatever you want. */
void  fex_set_user_data( File_Extractor*, void* new_user_data );
void* fex_user_data( File_Extractor const* );

/* Registers cleanup function to be called when closing extractor, or NULL to
clear it. Passes user_data (see above) to cleanup function. */
typedef void (*fex_user_cleanup_t)( void* user_data );
void fex_set_user_cleanup( File_Extractor*, fex_user_cleanup_t func );


#ifdef __cplusplus
	}
#endif

#endif

// Error strings and conversion functions

// File_Extractor 1.0.0
#ifndef BLARGG_ERRORS_H
#define BLARGG_ERRORS_H

#ifndef BLARGG_COMMON_H
	#include "blargg_common.h"
#endif

typedef const char blargg_err_def_t [];

// Basic errors
extern blargg_err_def_t blargg_err_generic;
extern blargg_err_def_t blargg_err_memory;
extern blargg_err_def_t blargg_err_caller;
extern blargg_err_def_t blargg_err_internal;
extern blargg_err_def_t blargg_err_limitation;

// File low-level
extern blargg_err_def_t blargg_err_file_missing; // not found
extern blargg_err_def_t blargg_err_file_read;
extern blargg_err_def_t blargg_err_file_write;
extern blargg_err_def_t blargg_err_file_io;
extern blargg_err_def_t blargg_err_file_full;
extern blargg_err_def_t blargg_err_file_eof;

// File high-level
extern blargg_err_def_t blargg_err_file_type;   // wrong file type
extern blargg_err_def_t blargg_err_file_feature;
extern blargg_err_def_t blargg_err_file_corrupt;

// C string describing error, or "" if err == NULL
const char* blargg_err_str( blargg_err_t err );

// True iff error is of given type, or false if err == NULL
bool blargg_is_err_type( blargg_err_t, const char type [] );

// Details of error without describing main cause, or "" if err == NULL
const char* blargg_err_details( blargg_err_t err );

// Converts error string to integer code using mapping table. Calls blargg_is_err_type()
// for each str and returns code on first match. Returns 0 if err == NULL.
struct blargg_err_to_code_t {
	const char* str;
	int code;
};
int blargg_err_to_code( blargg_err_t err, blargg_err_to_code_t const [] );

// Converts error code back to string. If code == 0, returns NULL. If not in table,
// returns blargg_err_generic.
blargg_err_t blargg_code_to_err( int code, blargg_err_to_code_t const [] );

// Generates error string literal with details of cause
#define BLARGG_ERR( type, str ) (type "; " str)

// Extra space to make it clear when blargg_err_str() isn't called to get
// printable version of error. At some point, I might prefix error strings
// with a code, to speed conversion to a code.
#define BLARGG_ERR_TYPE( str )  " " str

// Error types to pass to BLARGG_ERR macro
#define BLARGG_ERR_GENERIC      BLARGG_ERR_TYPE( "operation failed" )
#define BLARGG_ERR_MEMORY       BLARGG_ERR_TYPE( "out of memory" )
#define BLARGG_ERR_CALLER       BLARGG_ERR_TYPE( "internal usage bug" )
#define BLARGG_ERR_INTERNAL     BLARGG_ERR_TYPE( "internal bug" )
#define BLARGG_ERR_LIMITATION   BLARGG_ERR_TYPE( "exceeded limitation" )

#define BLARGG_ERR_FILE_MISSING BLARGG_ERR_TYPE( "file not found" )
#define BLARGG_ERR_FILE_READ    BLARGG_ERR_TYPE( "couldn't open file" )
#define BLARGG_ERR_FILE_WRITE   BLARGG_ERR_TYPE( "couldn't modify file" )
#define BLARGG_ERR_FILE_IO      BLARGG_ERR_TYPE( "read/write error" )
#define BLARGG_ERR_FILE_FULL    BLARGG_ERR_TYPE( "disk full" )
#define BLARGG_ERR_FILE_EOF     BLARGG_ERR_TYPE( "truncated file" )

#define BLARGG_ERR_FILE_TYPE    BLARGG_ERR_TYPE( "wrong file type" )
#define BLARGG_ERR_FILE_FEATURE BLARGG_ERR_TYPE( "unsupported file feature" )
#define BLARGG_ERR_FILE_CORRUPT BLARGG_ERR_TYPE( "corrupt file" )

#endif

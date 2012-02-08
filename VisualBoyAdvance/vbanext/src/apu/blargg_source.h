/* Included at the beginning of library source files, AFTER all other #include lines.
Sets up helpful macros and services used in my source code. Since this is only "active"
in my source code, I don't have to worry about polluting the global namespace with
unprefixed names. */

// Gb_Snd_Emu 0.2.0
#ifndef BLARGG_SOURCE_H
#define BLARGG_SOURCE_H

// If expr yields non-NULL error string, returns it from current function,
// otherwise continues normally.
#undef  RETURN_ERR
#define RETURN_ERR( expr ) do {                         \
		const char * blargg_return_err_ = (expr);               \
		if ( blargg_return_err_ ) return blargg_return_err_;    \
	} while ( 0 )

/* If ptr is NULL, returns "Out of memory" error string, otherwise continues normally.*/
#undef  CHECK_ALLOC
#define CHECK_ALLOC( ptr ) do { if ( (ptr) == 0 ) return "Out of memory"; } while ( 0 )

#endif

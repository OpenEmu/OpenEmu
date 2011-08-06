// Library configuration. Modify this file as necessary.

#ifndef BLARGG_CONFIG_H
#define BLARGG_CONFIG_H

// Uncomment to use zlib for transparent decompression of gzipped files
//#define HAVE_ZLIB_H

// Uncomment to support only the listed game music types. See gme_type_list.cpp
// for a list of all types.
//#define GME_TYPE_LIST gme_nsf_type, gme_gbs_type

// Uncomment to enable platform-specific optimizations
//#define BLARGG_NONPORTABLE 1

// Uncomment to use faster, lower quality sound synthesis
//#define BLIP_BUFFER_FAST 1

// Uncomment if automatic byte-order determination doesn't work
//#define BLARGG_BIG_ENDIAN 1

// Uncomment if you get errors in the bool section of blargg_common.h
//#define BLARGG_COMPILER_HAS_BOOL 1

// Use standard config.h if present
#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#endif

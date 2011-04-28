#ifndef QLZ_HEADER
#define QLZ_HEADER

// Version 1.31 final
#define QLZ_VERSION_MAJOR 1
#define QLZ_VERSION_MINOR 3
#define QLZ_VERSION_REVISION 1

// Set following flags according to the manual
#define COMPRESSION_LEVEL 0
//#define STREAMING_MODE 2000000
#define test_rle
#define speedup_incompressible
//#define memory_safe

// Public functions of QuickLZ

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t qlz_decompress(const char *source, void *destination, char *scratch);
size_t qlz_compress(const void *source, char *destination, size_t size, char *scratch);
size_t qlz_size_decompressed(const char *source);
size_t qlz_size_compressed(const char *source);
int qlz_get_setting(int setting);

#if (defined(__X86__) || defined(__i386__) || defined(i386) || defined(_M_IX86) || defined(__386__) || defined(__x86_64__) || defined(_M_X64))
	#define X86X64
#endif

// Compute SCRATCH_COMPRESS, SCRATCH_DECOMPRESS and constants used internally
#if COMPRESSION_LEVEL == 0 && defined(memory_safe)
	#error memory_safe flag cannot be used with COMPRESSION_LEVEL 0
#endif

#define HASH_ENTRIES 4096

#if (COMPRESSION_LEVEL == 0 || COMPRESSION_LEVEL == 1 || COMPRESSION_LEVEL == 2)
	#define AND 1
#elif (COMPRESSION_LEVEL == 3)
	#define AND 0x7
#else
	#error COMPRESSION_LEVEL must be 0, 1, 2 or 3
#endif

#define HASH_SIZE (AND + 1)*HASH_ENTRIES*sizeof(unsigned char *)

#ifdef STREAMING_MODE
	#define STREAMING_MODE_VALUE STREAMING_MODE
#else
	#define STREAMING_MODE_VALUE 0
#endif

#define STREAMING_MODE_ROUNDED ((STREAMING_MODE_VALUE >> 3) << 3)

#if (COMPRESSION_LEVEL > 1)
	#define SCRATCH_COMPRESS HASH_SIZE + STREAMING_MODE_VALUE + 16 + HASH_ENTRIES
#else
	#define SCRATCH_COMPRESS HASH_SIZE + STREAMING_MODE_VALUE + 16
#endif

#if (COMPRESSION_LEVEL == 0)
	#define SCRATCH_DECOMPRESS HASH_ENTRIES*sizeof(unsigned char *) + 16 + STREAMING_MODE_VALUE
#else
	#define SCRATCH_DECOMPRESS 16 + STREAMING_MODE_VALUE
#endif


#ifdef __cplusplus
}
#endif


#endif






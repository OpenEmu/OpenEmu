/* Fast data compressor for keeping data compressed in memory. Data format subject
to change, so not suitable for long-term storage or transfer between computers. */

/* blz 0.9.1 */
#ifndef BLZ_H
#define BLZ_H

#ifdef __cplusplus
	extern "C" {
#endif

/* All functions are thread-safe */

/* Temporary buffer type that you must allocate. Does NOT need to be cleared
before calling blz_pack(), and can be freed immediately afterwards. */
typedef struct blz_pack_t { unsigned char const* dict [0x1000]; } blz_pack_t;

/* Compresses data_size bytes from data_in to packed_out and returns packed size,
at most data_size + blz_worst_case. Requires data_size + blz_pack_extra bytes
allocated to packed_out. Requires that data_size be 0 to 0x7FFFFFFF (2 GB).
If temp is NULL, stores data uncompressed. */
enum { blz_pack_extra = 320 };
enum { blz_worst_case =   4 };
int blz_pack( void const* data_in, int data_size, void* packed_out, blz_pack_t* temp );

/* Decompresses from packed_in to data_out and returns original data size.
Does NOT do any consistency checking, so corrupt data can crash program. */
int blz_unpack( void const* packed_in, void* data_out );

/* Size of original data before compression */
int blz_size( void const* packed_in );

/* String describing version of blz. Currently either "fast" or "slow". */
extern char const blz_version [];

#ifdef __cplusplus
	}
#endif

#endif

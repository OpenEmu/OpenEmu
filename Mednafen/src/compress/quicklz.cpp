/*                   QuickLZ 1.20 data compression library
                 Copyright (C) 2006-2007 Lasse Mikkel Reinhold

QuickLZ can be used for free under the GPL-1 or GPL-2 license (where anything 
released into public must be open source) or under a commercial license if such 
has been acquired (see http://www.quicklz.com/order.html). The commercial license 
does not cover derived or ported versions created by third parties under GPL.

Caution! qlz_compress expects the size of the destination buffer to be exactly 
"uncompressed size" + 36000 bytes or more and qlz_compress_packet expects 
"uncompressed size" + 400.

Caution! The x86x64_only flag generates code which is not compatible with other 
architectures than x86 and x64 */

#include "../types.h"
#include <memory.h>
#include "quicklz.h"

#if defined(__amd64__) || defined(__x86_64__) || defined(_M_AMD64) || defined(__386__) || defined(__i386__) || defined(__i386) || defined(_M_IX86) || defined(_M_I386)
#define UNALIGNED_MEMORY_ACCESSES_OK	1
#endif

#if defined(LSB_FIRST) && defined(UNALIGNED_MEMORY_ACCESSES_OK)
#define x86x64_only
#endif

//#define memory_safe
//#define no_time_overhead
#define test_rle
#define speedup_incompressible
#define STREAM_BUFFER_SIZE 1000000

uint32 qlz_compress(const void *source, char *destination, uint32 size);
uint32 qlz_decompress(const char *source, void *destination);
uint32 qlz_size_decompressed(const char *source);
uint32 qlz_size_compressed(const char *source);
uint32 qlz_compress_packet(const void *source, char *destination, uint32 size, char *buffer);
uint32 qlz_decompress_packet(const char *source, void *destination, char *buffer);

#if STREAM_BUFFER_SIZE < 120000
	#error STREAM_BUFFER_SIZE must be at least 120000
#endif

static ALWAYS_INLINE uint32 fast_read(void const *src, uint32 bytes)
{
#ifndef x86x64_only
	unsigned char *p = (unsigned char*)src;
	switch (bytes)
	{
		case 4:
			return(*p | *(p + 1) << 8 | *(p + 2) << 16 | *(p + 3) << 24);
		case 3: 
			return(*p | *(p + 1) << 8 | *(p + 2) << 16);
		case 2:
			return(*p | *(p + 1) << 8);
		case 1: 
			return(*p);
	}
	return 0;
#else
	if (bytes >= 1 && bytes <= 4)
		return *((uint32*)src);
	else
		return 0;
#endif
}

static ALWAYS_INLINE void fast_write(uint32 f, void *dst, uint32 bytes)
{
#ifndef x86x64_only
	unsigned char *p = (unsigned char*)dst;
	switch (bytes)
	{
		case 4: 
			*p = f;
			*(p + 1) = f >> 8;
			*(p + 2) = f >> 16;
			*(p + 3) = f >> 24;
			return;
		case 3:
			*p = f;
			*(p + 1) = f >> 8;
			*(p + 2) = f >> 16;
			return;
		case 2:
			*p = f;
			*(p + 1) = f >> 8;
			return;
		case 1:
			*p = f;
			return;
	}
#else
	switch (bytes)
	{
		case 4: 
			*((uint32*)dst) = f;
			return;
		case 3:
			*((uint32*)dst) = f;
			return;
		case 2:
			*((uint32*)dst) = f;
			return;
		case 1:
			*((unsigned char*)dst) = (char)f;
			return;
	}
#endif
}

static ALWAYS_INLINE void memcpy_up(char *dst, const char *src, uint32 n)
{
	// cannot be replaced by overlap handling of memmove() due to LZSS algorithm
#ifndef x86x64_only

	if(n > 8 && src + n < dst)
		memcpy(dst, src, n);
	else
	{
		char *end = dst + n;
		while(dst < end)
		{
			*dst = *src;
			dst++;
			src++;
		}
	}
#else
	if (n < 5)
		*((uint32*)dst) = *((uint32*)src);
	else
	{
		char *end = dst + n;
		while(dst < end)
		{
			*((uint32*)dst) = *((uint32*)src);
			dst = dst + 4;
			src = src + 4;
		}
	}
#endif
}

static ALWAYS_INLINE uint32 fast_read_safe(void const *src, uint32 bytes, const char *invalid)
{
#ifdef memory_safe 
	if ((const char *)src + 4 > (const char *)invalid)
		return 0;
#endif
	invalid = invalid;
	return fast_read(src, bytes);
}

enum HEADERFIELDS {QCLZ = 0, QCLZ_VERSION = 1, COMPSIZE = 2, UNCOMPSIZE = 3, COMPRESSIBLE = 4, RESERVED1 = 5, RESERVED2 = 6, RESERVED3 = 7};


uint32 qlz_compress_core(const void *source, char *destination, uint32 size, const char **hashtable, const char *first_valid)
{
	const char *source_c = (const char*)source;
	char *destination_c = (char*)destination;
	const char *last_byte = source_c + size - 1;
	const char *src = source_c + 1;
	char *cword_ptr = destination_c;
	char *dst = destination_c + 4 + 1;
	char *prev_dst = dst;
	const char *prev_src = src;
	uint32 cword_val = 1U << 30;
	const char *guarantee_uncompressed = last_byte - 7;

#ifdef no_time_overhead
#define VALIDATE_OFFSET (o > first_valid) &&
#else
#define VALIDATE_OFFSET 
#endif
	
	first_valid = first_valid;

	// save first byte uncompressed
	*(destination_c + 4) = *source_c;

	while(src < guarantee_uncompressed)
	{
		uint32 fetch;
		if ((cword_val & 1) == 1)
		{
			// check if destinationc pointer could exceed destination buffer
			if (dst > destination_c + size)
				return 0;

			// store control word
			fast_write((cword_val >> 1) | (1U << 31), cword_ptr, 4);
			cword_ptr = dst;
			dst += 4;
			cword_val = 1U << 31;

#ifdef speedup_incompressible
			// check if source chunk is compressible
			if (dst - prev_dst > src - prev_src && src > source_c + 1000)
			{
				int q;
				for(q = 0; q < 20 && src + 31 < guarantee_uncompressed && dst + 35 < destination_c + size; q++)
				{
					fast_write((1U << 31), dst - 4, 4);
					memcpy(dst, src, 4*8 - 1);
					dst += 4*8 - 1 + 4;
					src += 4*8 - 1;
					prev_src = src;
					prev_dst = dst;
					cword_ptr = dst - 4;
				}
			} 
#endif
		}
#ifdef test_rle
		// check for rle sequence
		if (fast_read(src, 4) == fast_read(src + 1, 4))
		{
			const char *orig_src; 
			fetch = fast_read(src, 4);
			orig_src = src;
			do src = src + 4; while (src <= guarantee_uncompressed - 4 && fetch == fast_read(src, 4));
			if((src - orig_src) <= 2047) 
			{			
				fast_write(((fetch & 0xff) << 16) | (uint32)((src - orig_src) << 4) | 15, dst, 4);
				dst = dst + 3;
			}
			else
			{
				fast_write(((fetch & 0xff) << 16) | 15, dst, 4);
				fast_write((uint32)(src - orig_src), dst + 3, 4);
				dst = dst + 7;
			}
			cword_val = (cword_val >> 1) | (1 << (4*8 - 1));
		}
		else
#endif
			{
			const char *o;
			uint32 hash;
			// fetch source data and update hash table
			fetch = fast_read(src, 4);
			hash = ((fetch >> 12) ^ fetch) & 0x0fff;
			o = hashtable[hash];
			hashtable[hash] = src;
#ifndef x86x64_only
			if (VALIDATE_OFFSET src - o <= 131071 && src - o > 3 && *src == *o && *(src + 1) == *(o + 1) && *(src + 2) == *(o + 2))
#else
			if (VALIDATE_OFFSET src - o <= 131071 && src - o > 3 && (((*(uint32*)o) ^ (*(const uint32*)src)) & 0xffffff) == 0)
#endif
			{
				uint32 offset;
				uint32 matchlen;

				offset = (uint32)(src - o);
#ifndef x86x64_only
				if (*(o + 3) != *(src + 3))
#else
				if ((*(uint32*)o) != (*(uint32*)src))
#endif
				{
					if(offset <= 63)
					{
						// encode lz match
						*dst = (unsigned char)(offset << 2);
						cword_val = (cword_val >> 1) | (1U << 31);
						src += 3;
						dst++;
					}
					else if (offset <= 16383)
					{
						// encode lz match
						uint32 f = (offset << 2) | 1;
						fast_write(f, dst, 2);
						cword_val = (cword_val >> 1) | (1U << 31);
						src += 3;
						dst += 2;
					}
					else
					{
						// encode literal
						*dst = *src;
						src++;
						dst++;
						cword_val = (cword_val >> 1);
					}
				}
				else
				{
					// encode lz match
					const char *old_src = src;

					cword_val = (cword_val >> 1) | (1U << 31);
					src += 3;
					while(*(o + (src - old_src)) == *src && src < guarantee_uncompressed)
						src++;
					matchlen = (uint32)(src - old_src);

					if (matchlen <= 18 && offset <= 1023)
					{
						uint32 f = ((matchlen - 3) << 2) | (offset << 6) | 2;
						fast_write(f, dst, 2);
						dst += 2;
					}

					else if(matchlen <= 34 && offset <= 65535)
					{
						uint32 f = ((matchlen - 3) << 3) | (offset << 8) | 3;
						fast_write(f, dst, 3);
						dst += 3;
					}
					else if (matchlen >= 3)
					{
						if (matchlen <= 2050)
						{
							uint32 f = ((matchlen - 3) << 4) | (offset << 15) | 7;
							fast_write(f, dst, 4);
							dst += 4;
						}
						else
						{
							fast_write(7, dst, 4);
							fast_write(matchlen, dst + 4, 4);
							fast_write(offset, dst + 8, 4);
							dst += 12;							
						} 
					}
				}
			}
			else
			{
				// encode literal
				*dst = *src;
				src++;
				dst++;
				cword_val = (cword_val >> 1);
			}
		}
	}

	*(dst + 15) = (char)((size_t)(source_c + size) % 8);

// save last source bytes as literals
	while (src <= last_byte)
	{
		if ((cword_val & 1) == 1)
		{
			fast_write((cword_val >> 1) | (1U << 31), cword_ptr, 4);
			cword_ptr = dst;
			dst += 4;
			cword_val = 1U << 31;
		}
		*dst = *src;
		src++;
		dst++;
		cword_val = (cword_val >> 1);
	}

	while((cword_val & 1) != 1)
		cword_val = (cword_val >> 1);

	fast_write((cword_val >> 1) | (1U << 31), cword_ptr, 4);

	// min. size must be 9 bytes so that the qlz_size functions can take 9
	// bytes as argument without having to rewind linear storage devices later
	if (dst - destination_c < 9)
		return 9;
	else
		return (uint32)(dst - destination_c);
}

void qlz_decompress_core(const char *source, void *destination, uint32 size, uint32 source_size, char *first_valid)
{
	const char *source_c = (const char*)source;
	char *destination_c = (char*)destination;
	const char *src = source_c;
	char *dst = destination_c;
	const char* last_byte_successor = destination_c + size;
	uint32 cword_val = 1;
	const uint32 bitlut[16] = {4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0};
	const char *guaranteed_uncompressed = last_byte_successor - 3;

	first_valid = first_valid;

	if (size == 0 || source_size == 0)
		return;
	
	// prevent spurious memory read on a source with size < 4
	if (dst >= guaranteed_uncompressed)
	{
		src += 4;
		while(dst < last_byte_successor)
		{
			*dst = *src;
			dst++;
			src++;
		}
		return;
	}

	for(;;) 
	{
		uint32 fetch;

		if (cword_val == 1)
		{
			// fetch control word
			cword_val = fast_read_safe(src, 4, source_c + source_size) | (1U << 31);
			src += 4;
		}
		
		fetch = fast_read_safe(src, 4, source_c + source_size);

		// check if we must decode lz match
		if ((cword_val & 1) == 1)
		{
			uint32 offset;
			uint32 matchlen;

			cword_val = cword_val >> 1;
			if ((fetch & 3) == 0)
			{
				offset = (fetch & 0xff) >> 2;
#ifdef memory_safe	
				if (3 > (uint32)(guaranteed_uncompressed - dst) || offset > (uint32)(dst - first_valid))
					return;
#endif
				memcpy_up(dst, dst - offset, 3);
				dst += 3;
				src++;
			}
			else if ((fetch & 2) == 0)
			{
				offset = (fetch & 0xffff) >> 2;
#ifdef memory_safe	
				if (3 > (uint32)(guaranteed_uncompressed - dst) || offset > (uint32)(dst - first_valid)) 
					return;
#endif
				memcpy_up(dst, dst - offset, 3);
				dst += 3;
				src += 2;
			}
			else if ((fetch & 1) == 0)
			{
				offset = (fetch & 0xffff) >> 6;
				matchlen = ((fetch >> 2) & 15) + 3;
#ifdef memory_safe	
				if (matchlen > (uint32)(guaranteed_uncompressed - dst) || offset > (uint32)(dst - first_valid)) 
					return;
#endif
				memcpy_up(dst, dst - offset, matchlen);
				src += 2;
				dst += matchlen;
			}
			else if ((fetch & 4) == 0)
			{
				offset = (fetch & 0xffffff) >> 8;
				matchlen = ((fetch >> 3) & (4*8 -1)) + 3;
#ifdef memory_safe	
				if (matchlen > (uint32)(guaranteed_uncompressed - dst) || offset > (uint32)(dst - first_valid)) 
					return;
#endif
				memcpy_up(dst, dst - offset, matchlen);
				src += 3;
				dst += matchlen;
			}
			else if ((fetch & 8) == 0)
			{
				offset = (fetch >> 15);
				if (offset != 0)
				{
					matchlen = ((fetch >> 4) & 2047) + 3;
					src += 4;
				}
				else
				{
					matchlen = fast_read_safe(src + 4, 4, source_c + source_size);
					offset = fast_read_safe(src + 8, 4, source_c + source_size);
					src += 12;
				}
#ifdef memory_safe	
				if (matchlen > (uint32)(guaranteed_uncompressed - dst) || offset > (uint32)(dst - first_valid)) 
					return;
#endif
				memcpy_up(dst, dst - offset, matchlen);
				dst += matchlen;
			}
			else
			{
				// decode rle sequence
				unsigned char rle_char;

				rle_char = (unsigned char)(fetch >> 16);
				matchlen = ((fetch >> 4) & 0xfff);

				if(matchlen != 0)
					src += 3;
				else
				{
					matchlen = fast_read_safe(src + 3, 4, source_c + source_size);
					src += 7;
				}

#ifdef memory_safe	
				if(matchlen > (uint32)(guaranteed_uncompressed - dst)) 
					return;
#endif
				memset(dst, rle_char, matchlen);
				dst += matchlen;
			}
		}
		else
		{
			// decode literal
#ifdef memory_safe 
			if (4 > destination_c + size - dst || src > source_c + source_size + 4) 
				return;
#endif
			memcpy_up(dst, src, 4);
			dst += bitlut[cword_val & 0xf];
			src += bitlut[cword_val & 0xf];
			cword_val = cword_val >> (bitlut[cword_val & 0xf]);

			if (dst >= guaranteed_uncompressed)
			{
				// decode last literals and exit
				while(dst < last_byte_successor)
				{
					if (cword_val == 1)
					{
						src += 4;
						cword_val = 1U << 31;
					}
					if (1 > destination_c + size - dst)
						return;
					*dst = *src;
					dst++;
					src++;
					cword_val = cword_val >> 1;
				}
				return;
			}
		}
	}
}

uint32 qlz_size_decompressed(const char *source)
{
	uint32 *header, n, r;
	header = (uint32*)source;
	if (memcmp(&header[QCLZ], "QCLZ", 4) != 0)
	{
		n = (((*source) & 2) == 2) ? 4 : 1;
		r = fast_read(source + 1 + n, n);
		r = r & (0xffffffff >> ((4 - n)*8));
		return r;
	}
	else
		return fast_read(&header[UNCOMPSIZE], 4);
}

uint32 qlz_size_compressed(const char *source)
{
	uint32 *header, n, r;
	header = (uint32*)source;
	if (memcmp(&header[QCLZ], "QCLZ", 4) != 0)
	{
		n = (((*source) & 2) == 2) ? 4 : 1;
		r = fast_read(source + 1, n);
		r = r & (0xffffffff >> ((4 - n)*8));
		return r;
	}
	else
		return fast_read(&header[COMPSIZE], 4);
}

uint32 qlz_compress(const void *source, char *destination, uint32 size)
{
#ifndef no_time_overhead
	uint32 i;
#endif
	uint32 r, base;
	uint32 compressed;
	const char **hashtable = (const char**)(destination + size + 36000 - sizeof(char *)*4096 - (((size_t)(destination + size)) % 8));

	if(size < 216)
		base = 3;
	else
		base = 9;

#ifndef no_time_overhead
		for (i = 0; i < 4096; i++)
			hashtable[i] = (const char *)source;
#endif
	r = base + qlz_compress_core(source, destination + base, size, hashtable, (const char*)source);

	if(r == base)
	{
		memcpy(destination + base, source, size);
		r = size + base;
		compressed = 0;
	}
	else
		compressed = 1;

	hashtable[0] = 0;

	if(base == 3)
	{
		*destination = (unsigned char)(0 | compressed);
		*(destination + 1) = (unsigned char)r;
		*(destination + 2) = (unsigned char)size;
	}
	else
	{
		*destination = (unsigned char)(2 | compressed);
		fast_write(r, destination + 1, 4);
		fast_write(size, destination + 5, 4);
	}
	return r;
}

uint32 qlz_decompress(const char *source, void *destination)
{
	uint32 size_decompressed = qlz_size_decompressed(source);
	uint32 size_compressed = qlz_size_compressed(source);
	uint32 n = (((*source) & 2) == 2) ? 4 : 1;
	uint32 base = 2*n + 1;
	uint32 *header = (uint32*)source;

	if (memcmp(&header[QCLZ], "QCLZ", 4) == 0) 
	{
		if (fast_read(&header[COMPRESSIBLE], 4) != 1)
			memcpy(destination, source + 32, size_decompressed);
		else
			qlz_decompress_core(source + 32, destination, size_decompressed, size_compressed, (char*)destination);
		return size_decompressed;		
	}
	else if (((*source) & 0xfc) == 0)
	{
		if((*source & 1) == 1)		
			qlz_decompress_core(source + base, destination, size_decompressed, size_compressed, (char*)destination);
		else
			memcpy(destination, source + base, size_decompressed);
		return size_decompressed;
	}
	else
		return 0;
}

uint32 qlz_compress_packet(const void *source, char *destination, uint32 size, char *buffer)
{
	// 0-7 bytes for aligning; 4 bytes for buffersize; 32768 byte hashtable; STREAM_BUFFER_SIZE - 40000 bytes streambuffer
	char *buffer_aligned = buffer + 8 - (((size_t)buffer) % 8);
	const char **hashtable = (const char **)(buffer_aligned + 4);
	uint32 *buffersize = (uint32 *)buffer_aligned;
	char *streambuffer = buffer_aligned + 4 + 32768;
	uint32 r, i;
	uint32 compressed, base;

	if(size < 216)
		base = 3;
	else
		base = 9;

	if (hashtable[0] == 0)
	{

		for (i = 0; i < 4096; i++)
			hashtable[i] = streambuffer;

		*buffersize = 0;
	}

	if (*buffersize + size > STREAM_BUFFER_SIZE - 40000) 
	{
		for (i = 0; i < 4096; i++)
			hashtable[i] = (const char *)source;
		r = base + qlz_compress_core(source, destination + base, size, hashtable, (const char*)source);

		if(r == base)
		{
			memcpy(destination + base, source, size);
			r = size + base;
			compressed = 0;
		}
		else
			compressed = 1;
	
		hashtable[0] = 0;
	}
	else
	{
		memcpy(streambuffer + *buffersize, source, size);
		r = base + qlz_compress_core(streambuffer + *buffersize, destination + base, size, hashtable, streambuffer);

		if(r == base)
		{
			memcpy(destination + base, streambuffer + *buffersize, size);
			r = size + base;
			compressed = 0;
		}
		else
			compressed = 1;

		*buffersize += size;
	}

	if(base == 3)
	{
		*destination = (unsigned char)(0 | compressed);
		*(destination + 1) = (unsigned char)r;
		*(destination + 2) = (unsigned char)size;
	}
	else
	{
		*destination = (unsigned char)(2 | compressed);
		fast_write(r, destination + 1, 4);
		fast_write(size, destination + 5, 4);
	}
	return r;
}

uint32 qlz_decompress_packet(const char *source, void *destination, char *buffer)
{
	// 0-7 bytes for aligning; 4 bytes for buffersize; STREAM_BUFFER_SIZE - 40000 bytes streambuffer
	char *buffer_aligned = buffer + 8 - (((size_t)buffer) % 8);
	uint32 *buffersize = (uint32 *)buffer_aligned;
	char *streambuffer = buffer_aligned + 4;
	uint32 size_decompressed = qlz_size_decompressed(source);
	uint32 size_compressed = qlz_size_compressed(source);
	uint32 n = (((*source) & 2) == 2) ? 4 : 1;
	uint32 base = 2*n + 1;

	if (*buffersize + size_decompressed > STREAM_BUFFER_SIZE - 40000) 
	{		
		if((*source & 1) == 1)		
			qlz_decompress_core(source + base, destination, size_decompressed, size_compressed, (char*)destination);
		else
			memcpy(destination, source + base, size_decompressed);
		*buffersize = 0;
	}
	else
	{
		if((*source & 1) == 1)		
			qlz_decompress_core(source + base, streambuffer + *buffersize, size_decompressed, size_compressed, streambuffer);
		else
			memcpy(streambuffer + *buffersize, source + base, size_decompressed);
		memcpy(destination, streambuffer + *buffersize, size_decompressed);
		*buffersize += size_decompressed;
	}
	return size_decompressed;
}



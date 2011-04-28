// QuickLZ data compression library
// Copyright (C) 2006-2008 Lasse Mikkel Reinhold
// lar@quicklz.com
//
// QuickLZ can be used for free under the GPL-1 or GPL-2 license (where anything 
// released into public must be open source) or under a commercial license if such 
// has been acquired (see http://www.quicklz.com/order.html). The commercial license 
// does not cover derived or ported versions created by third parties under GPL.

#include "quicklz.h"

int qlz_get_setting(int setting)
{
	switch (setting)
	{
		case 0: return COMPRESSION_LEVEL;
		case 1: return SCRATCH_COMPRESS;
		case 2: return SCRATCH_DECOMPRESS;
		case 3: return STREAMING_MODE_VALUE;
#ifdef test_rle
		case 4: return 1;
#else
		case 4: return 0;
#endif
#ifdef speedup_incompressible
		case 5: return 1;
#else
		case 5: return 0;
#endif
#ifdef memory_safe
		case 6: return 1;
#else
		case 6: return 0;
#endif
		case 7: return QLZ_VERSION_MAJOR;
		case 8: return QLZ_VERSION_MINOR;
		case 9: return QLZ_VERSION_REVISION;
	}
	return -1;
}

__inline unsigned int hash_func(unsigned int i)
{
	return ((i >> 12) ^ i) & 0x0fff;
}

__inline unsigned int fast_read(void const *src, unsigned int bytes)
{
#ifndef X86X64
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
		return *((unsigned int*)src);
	else
		return 0;
#endif
}

__inline void fast_write(unsigned int f, void *dst, unsigned int bytes)
{
#ifndef X86X64
	unsigned char *p = (unsigned char*)dst;

	switch (bytes)
	{
		case 4: 
			*p = (unsigned char)f;
			*(p + 1) = (unsigned char)(f >> 8);
			*(p + 2) = (unsigned char)(f >> 16);
			*(p + 3) = (unsigned char)(f >> 24);
			return;
		case 3:
			*p = (unsigned char)f;
			*(p + 1) = (unsigned char)(f >> 8);
			*(p + 2) = (unsigned char)(f >> 16);
			return;
		case 2:
			*p = (unsigned char)f;
			*(p + 1) = (unsigned char)(f >> 8);
			return;
		case 1:
			*p = (unsigned char)f;
			return;
	}
#else
	switch (bytes)
	{
		case 4: 
			*((unsigned int*)dst) = f;
			return;
		case 3:
			*((unsigned int*)dst) = f;
			return;
		case 2:
#if COMPRESSION_LEVEL == 0
// 2 byte writes are common in level 0
			*((unsigned short int*)dst) = (unsigned short int)f;
#else
			*((unsigned int*)dst) = f;
#endif
			return;
		case 1:
			*((unsigned char*)dst) = (unsigned char)f;
			return;
	}
#endif
}

__inline void memcpy_up(unsigned char *dst, const unsigned char *src, unsigned int n)
{
	// cannot be replaced by overlap handling of memmove() due to LZSS algorithm
#ifndef X86X64

	if(n > 8 && src + n < dst)
		memcpy(dst, src, n);
	else
	{
		unsigned char *end = dst + n;
		while(dst < end)
		{
			*dst = *src;
			dst++;
			src++;
		}
	}
#else
	if (n < 5)
		*((unsigned int*)dst) = *((unsigned int*)src);
	else
	{
		unsigned char *end = dst + n;
		while(dst < end)
		{
			*((unsigned int*)dst) = *((unsigned int*)src);
			dst = dst + 4;
			src = src + 4;
		}
	}
#endif
}

__inline unsigned int fast_read_safe(void const *src, unsigned int bytes, const unsigned char *invalid)
{
#ifdef memory_safe 
	if ((const unsigned char *)src + 4 > (const unsigned char *)invalid)
		return 0xffffffff;
#endif
	invalid = invalid;
	return fast_read(src, bytes);
}

unsigned int qlz_compress_core(const void *source, unsigned char *destination, unsigned int size,  const unsigned char *hashtable[][AND + 1], const unsigned char *first_valid, unsigned char *hash_counter)
{
	const unsigned char *source_c = (const unsigned char*)source;
	unsigned char *destination_c = (unsigned char*)destination;
	const unsigned char *last_byte = source_c + size - 1;
	const unsigned char *src = source_c;
	unsigned char *cword_ptr = destination_c;
	unsigned char *dst = destination_c + 4;
	unsigned int cword_val = 1U << 31;
	const unsigned char *guarantee_uncompressed = last_byte - 8;

#ifdef speedup_incompressible
		unsigned char *prev_dst = dst;
		const unsigned char *prev_src = src;
#endif

	hash_counter = hash_counter;
	first_valid = first_valid;

	// save first 4 bytes uncompressed
	while(src < source_c + 4 && src < guarantee_uncompressed)
	{
		cword_val = (cword_val >> 1);
		*dst = *src;
		dst++;
		src++;
	}
	
	while(src < guarantee_uncompressed)
	{
		unsigned int fetch;
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
				for(q = 0; q < 30 && src + 31 < guarantee_uncompressed && dst + 35 < destination_c + size; q++)
				{

#if(COMPRESSION_LEVEL == 0)
					int w;
					for(w = 0; w < 31; w++)
					{
						fetch = fast_read(src + w, 4);
						*(unsigned int*)&hashtable[hash_func(fetch)][0] = fast_read(src + w, 4);
						hashtable[hash_func(fetch)][1] = src + w;
					}
#endif
					fast_write((1U << 31), dst - 4, 4);
					memcpy(dst, src, 31);

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
			const unsigned char *orig_src; 
			fetch = fast_read(src, 4);
			orig_src = src;
			do src = src + 4; while (src <= guarantee_uncompressed - 4 && fetch == fast_read(src, 4));
			if((src - orig_src) <= 2047) 
			{			
				fast_write(((fetch & 0xff) << 16) | (unsigned int)((src - orig_src) << 4) | 15, dst, 4);
				dst = dst + 3;
			}
			else
			{
				fast_write(((fetch & 0xff) << 16) | 15, dst, 4);
				fast_write((unsigned int)(src - orig_src), dst + 3, 4);
				dst = dst + 7;
			}
			cword_val = (cword_val >> 1) | (1 << 31);
		}
		else
#endif
		{
			const unsigned char *o;
			unsigned int hash, matchlen;

#if(COMPRESSION_LEVEL < 2)
			unsigned int cached;

			fetch = fast_read(src, 4);
			hash = hash_func(fetch);

			cached = fetch ^ *(unsigned int*)&hashtable[hash][0];
			*(unsigned int*)&hashtable[hash][0] = fetch;

			o = hashtable[hash][1];
			hashtable[hash][1] = src;

#else
			unsigned char c;
			unsigned int k, m;
			const unsigned char *offset2 = 0;
			
			fetch = fast_read(src, 4);
			hash = hash_func(fetch);

			matchlen = 0;
			c = hash_counter[hash];			
			for(k = 0; k < AND + 1; k++)
			{
				o = hashtable[hash][(c - k) & AND];
				if(o > first_valid && o < src - 3 && *(src + matchlen) == *(o + matchlen) && (fast_read(o, 3) & 0xffffff) == (fetch & 0xffffff) && src - o < 131071)
				{	
					size_t remaining;
					remaining = guarantee_uncompressed - src;
					m = 3;
					if (fast_read(o, 4) == fetch)
					{
						while(*(o + m) == *(src + m) && m < remaining)
							m++;
					}
					if (m > matchlen)
					{
						matchlen = m;
						offset2 = o;
					}
				}
			}
			o = offset2;			
			c = (hash_counter[hash] + 1) & AND;
			hash_counter[hash] = c;			
			hashtable[hash][c] = src;
#endif

#if(COMPRESSION_LEVEL == 0)
			if (o != 0 && (cached & 0xffffff) == 0 && src - o > 3)
#elif(COMPRESSION_LEVEL == 1)
			if ((cached & 0xffffff) == 0 && o > first_valid && o < src - 3 && ((fast_read(o, 3) ^ fast_read(src, 3)) & 0xffffff) == 0 && src - o < 131071)
#elif(COMPRESSION_LEVEL > 1)
			if(matchlen == 3)
#endif
			{
				unsigned int offset;
				offset = (unsigned int)(src - o);
					
#if(COMPRESSION_LEVEL < 2)
				if (cached & 0xffffffff)
#endif
				{
#if (COMPRESSION_LEVEL > 2)
					unsigned int u;
					for(u = 1; u < 3; u++)
					{
						hash = hash_func(fast_read(src + u, 4));
						c = (hash_counter[hash] + 1) & AND;
						hash_counter[hash] = c;
						hashtable[hash][c] = src + u;
					}	
#endif

#if (COMPRESSION_LEVEL == 0)					
					cword_val = (cword_val >> 1) | (1U << 31);
					fast_write(3 | (hash << 4), dst, 2);
					src += 3;
					dst += 2;
#else

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
						unsigned int f = (offset << 2) | 1;
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
#endif
				}		
#if(COMPRESSION_LEVEL > 1)
			}								
			else if(matchlen > 3)
			{
#elif(COMPRESSION_LEVEL < 2)
				else
#endif
				{
					// encode lz match
					unsigned int offset;
					
#if(COMPRESSION_LEVEL < 2)
					const unsigned char *old_src = src;
					offset = (unsigned int)(src - o);
					cword_val = (cword_val >> 1) | (1U << 31);

					src += 3;
					while(*(o + (src - old_src)) == *src && src < guarantee_uncompressed)
						src++;
					matchlen = (unsigned int)(src - old_src);
#else
					unsigned int u;
					offset = (unsigned int)(src - o);
					cword_val = (cword_val >> 1) | (1U << 31);

#if (COMPRESSION_LEVEL > 2)
					for(u = 1; u < matchlen; u++)
#else
					for(u = 1; u < matchlen && u < 5; u++)
#endif
					{
						hash = hash_func(fast_read(src + u, 4));
						c = (hash_counter[hash] + 1) & AND;
						hash_counter[hash] = c;
						hashtable[hash][c] = src + u;
					}
					src += matchlen;			
#endif					

#if (COMPRESSION_LEVEL == 0)
					if (matchlen < 15)
					{
						fast_write(matchlen | (hash << 4), dst, 2);
						dst += 2;
					} 
					else if (matchlen < 255)
					{
						fast_write(hash << 4, dst, 2);
						*(dst + 2) = (unsigned char)matchlen;
						dst += 3;
					}
					else
					{
						fast_write(hash << 4, dst, 2);
						*(dst + 2) = 0;
						fast_write(matchlen, dst + 3, 4);
						dst += 7;
					}					
#else
					if (matchlen <= 18 && offset <= 1023)
					{
						unsigned int f = ((matchlen - 3) << 2) | (offset << 6) | 2;
						fast_write(f, dst, 2);
						dst += 2;
					}

					else if(matchlen <= 34 && offset <= 65535)
					{
						unsigned int f = ((matchlen - 3) << 3) | (offset << 8) | 3;
						fast_write(f, dst, 3);
						dst += 3;
					}
					else if (matchlen >= 3)
					{
						if (matchlen <= 2050)
						{
							unsigned int f = ((matchlen - 3) << 4) | (offset << 15) | 7;
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
#endif
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

		if (src < last_byte - 2 && src > source_c + 3)
		{		
			hashtable[hash_func(fast_read(src, 4))][1] = src;
			*(unsigned int*)&hashtable[hash_func(fast_read(src, 4))][0] = fast_read(src, 4);
		}
		*dst = *src;
		src++;
		dst++;

		cword_val = (cword_val >> 1);
	}

	while((cword_val & 1) != 1)
		cword_val = (cword_val >> 1);

	fast_write((cword_val >> 1) | (1U << 31), cword_ptr, 4);

	// min. size must be 9 bytes so that the qlz_size functions can take 9 bytes as argument
	if (dst - destination_c < 9)
		return 9;
	else
		return (unsigned int)(dst - destination_c);
}

size_t qlz_decompress_core(const unsigned char *source, void *destination, size_t size, size_t source_size, unsigned char *first_valid, const unsigned char *hashtable[])
{
	const unsigned char *source_c = (const unsigned char*)source;
	unsigned char *destination_c = (unsigned char*)destination;
	const unsigned char *src = source_c;
	unsigned char *dst = destination_c;
	const unsigned char* last_byte_successor = destination_c + size;
	unsigned int cword_val = 1;
	const unsigned int bitlut[16] = {4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0};
	const unsigned char *guaranteed_uncompressed = last_byte_successor - 4;
	unsigned char *last_hashed = destination_c + 3;

	first_valid = first_valid;
	last_hashed = last_hashed;
	hashtable = hashtable;
		
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

		return (unsigned int)(dst - destination_c);
	}


	for(;;) 
	{
		unsigned int fetch;

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
			unsigned int matchlen;

#if(COMPRESSION_LEVEL == 0)
			unsigned int hash;
			const unsigned char *offset2;

			cword_val = cword_val >> 1;			

			if((fetch & 0xf) != 15)
			{
				hash = (fetch >> 4) & 0xfff;
				offset2 = hashtable[hash];

				if((fetch & 0xf) != 0)
				{
					matchlen = (fetch & 0xf);
					src += 2;			
				}
				else if((fetch & 0x00ff0000) != 0)
				{
					matchlen = *(src + 2);
					src += 3;							
				}
				else
				{
					matchlen = fast_read(src + 3, 4);
					src += 7;
				}
				memcpy_up(dst, offset2, matchlen);
				while(last_hashed < dst)
				{
					last_hashed++;
					hashtable[hash_func(fast_read(last_hashed, 4))] = last_hashed;
				}				
				dst += matchlen;
				last_hashed = dst - 1;				
			}
				
#else		
			unsigned int offset;
			cword_val = cword_val >> 1;			

			if ((fetch & 3) == 0)
			{
				offset = (fetch & 0xff) >> 2;
#ifdef memory_safe	
				if (3 > (unsigned int)(guaranteed_uncompressed - dst) || offset > (unsigned int)(dst - first_valid))
					return 0;
#endif
				memcpy_up(dst, dst - offset, 3);
				dst += 3;
				src++;
			}
			else if ((fetch & 2) == 0)
			{
				offset = (fetch & 0xffff) >> 2;
#ifdef memory_safe	
				if (3 > (unsigned int)(guaranteed_uncompressed - dst) || offset > (unsigned int)(dst - first_valid)) 
					return 0;
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
				if (matchlen > (unsigned int)(guaranteed_uncompressed - dst) || offset > (unsigned int)(dst - first_valid)) 
					return 0;
#endif
				memcpy_up(dst, dst - offset, matchlen);
				src += 2;
				dst += matchlen;
			}
			else if ((fetch & 4) == 0)
			{
				offset = (fetch & 0xffffff) >> 8;
				matchlen = ((fetch >> 3) & 31) + 3;
#ifdef memory_safe	
				if (matchlen > (unsigned int)(guaranteed_uncompressed - dst) || offset > (unsigned int)(dst - first_valid)) 
					return 0;
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
				if (matchlen > (unsigned int)(guaranteed_uncompressed - dst) || offset > (unsigned int)(dst - first_valid)) 
					return 0;
#endif
				memcpy_up(dst, dst - offset, matchlen);
				dst += matchlen;
			}
#endif
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
				if(matchlen > (unsigned int)(guaranteed_uncompressed - dst)) 
					return 0;
#endif
				memset(dst, rle_char, matchlen);

#if(COMPRESSION_LEVEL == 0)
				while(last_hashed < dst - 1)
				{
					last_hashed++;
					hashtable[hash_func(fast_read(last_hashed, 4))] = last_hashed;
				}				
				last_hashed = dst - 1 + matchlen;
#endif
				dst += matchlen; 
			}
		}
		else
		{
			// decode literal
#ifdef memory_safe 
			if (4 > destination_c + size - dst || src > source_c + source_size + 4) 
				return 0;
#endif
			memcpy_up(dst, src, 4);

			dst += bitlut[cword_val & 0xf];
			src += bitlut[cword_val & 0xf];
			cword_val = cword_val >> (bitlut[cword_val & 0xf]);

#if(COMPRESSION_LEVEL == 0)
			while(last_hashed < dst - 3)
			{
				last_hashed++;
				hashtable[hash_func(fast_read(last_hashed, 4))] = last_hashed;
			}
#endif
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
						return 0;

					*dst = *src;
					dst++;
					src++;
					cword_val = cword_val >> 1;
				}

#if(COMPRESSION_LEVEL == 0)
				while(last_hashed < last_byte_successor - 4)
				{
					last_hashed++;
					hashtable[hash_func(fast_read(last_hashed, 4))] = last_hashed;
				}
#endif
				if((src - 1) - source_c > 8) // 8 bytes comp. size excessive len is ok
					return 0;
				else if(dst - destination_c - size == 0)
					return size;
				else
					return 0;
			}
		}
	}
}

size_t qlz_size_decompressed(const char *source)
{
	unsigned int n, r;
	n = (((*source) & 2) == 2) ? 4 : 1;
	r = fast_read(source + 1 + n, n);
	r = r & (0xffffffff >> ((4 - n)*8));
	return r;
}

size_t qlz_size_compressed(const char *source)
{
	unsigned int n, r;
	n = (((*source) & 2) == 2) ? 4 : 1;
	r = fast_read(source + 1, n);
	r = r & (0xffffffff >> ((4 - n)*8));
	return r;
}

size_t qlz_compress(const void *source, char *destination, size_t size, char *scratch)
{
	// 1-8 bytes for aligning (not 0-7!); 8 bytes for buffersize (padds on 32 bit cpu); HASH_SIZE hash table; STREAMING_MODE_ROUNDED bytes streambuffer; optional HASH_ENTRIES byte hash counter
	unsigned char *buffer_aligned = (unsigned char *)scratch + 8 - (((size_t)scratch) % 8);
	const unsigned char *(*hashtable)[AND + 1] = (const unsigned char *(*)[AND + 1])(buffer_aligned + 8);
	size_t *buffersize = (size_t *)buffer_aligned;
	unsigned char *streambuffer = buffer_aligned + 8 + HASH_SIZE;
	unsigned int r;
	unsigned int compressed, base;
	unsigned char *hash_counter = streambuffer + STREAMING_MODE_ROUNDED;

	if(size == 0 || size > 0xffffffff)
		return 0;

#if (COMPRESSION_LEVEL == 0 && STREAMING_MODE_ROUNDED == 0)
		memset((void *)hashtable, 0, HASH_SIZE);
#endif

	if(size < 216)
		base = 3;
	else
		base = 9;

// if not STREAMING_MODE, then STREAMING_MODE_ROUNDED == 0 and first case (streaming buffer full) is executed unconditionally, functioning as block comp.
	if (*buffersize + size - 1 >= STREAMING_MODE_ROUNDED)
	{
#if (COMPRESSION_LEVEL == 0 && STREAMING_MODE_ROUNDED != 0)
		memset((void *)hashtable, 0, HASH_SIZE);
#endif

		r = base + qlz_compress_core(source, (unsigned char*)destination + base, (unsigned int)size, hashtable, (const unsigned char*)source, hash_counter);
#if (COMPRESSION_LEVEL == 0 && STREAMING_MODE_ROUNDED != 0)
		memset((void *)hashtable, 0, HASH_SIZE);
#endif

		if(r == base)
		{
			memcpy(destination + base, source, size);
			r = (unsigned int)size + base;
			compressed = 0;
		}
		else
			compressed = 1;
		*buffersize = 0;
	}
	else
	{
		memcpy(streambuffer + *buffersize, source, size);
		r = base + qlz_compress_core(streambuffer + *buffersize, (unsigned char*)destination + base, (unsigned int)size, hashtable, streambuffer, hash_counter);

		if(r == base)
		{
			memcpy(destination + base, streambuffer + *buffersize, size);
			r = (unsigned int)size + base;
			compressed = 0;
				
			memset((void*)hashtable, 0, HASH_SIZE);
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
		fast_write((unsigned int)size, destination + 5, 4);
	}

#if (COMPRESSION_LEVEL == 0)
		*destination = (*destination) | 4;
#endif 

	return (size_t)r;
}



size_t qlz_decompress(const char *source, void *destination, char *scratch)
{
	// 1-8 bytes for aligning (not 0-7!); 8 bytes for buffersize (padds on 32bit cpu); STREAMING_MODE_ROUNDED streambuffer; HASH_SIZE hash table
	unsigned char *buffer_aligned = (unsigned char *)scratch + 8 - (((size_t)scratch) % 8);
	size_t *buffersize = (size_t *)buffer_aligned;
	unsigned int headerlen = 2*((((*source) & 2) == 2) ? 4 : 1) + 1; // get header len

	unsigned char *streambuffer = buffer_aligned + 8;
	const unsigned char **hashtable = (const unsigned char **)(streambuffer + STREAMING_MODE_ROUNDED);

	size_t dsiz = qlz_size_decompressed((char *)source);
	size_t csiz = qlz_size_compressed((char *)source);
	if (*buffersize + qlz_size_decompressed((char *)source) - 1 >= STREAMING_MODE_ROUNDED) 
	{	
		if((*source & 1) == 1)		
			qlz_decompress_core((const unsigned char *)source + headerlen, destination, dsiz, csiz, (unsigned char*)destination, hashtable);
		else
			memcpy(destination, source + headerlen, dsiz);
		*buffersize = 0;
	}
	else
	{
		if((*source & 1) == 1)
			qlz_decompress_core((const unsigned char *)source + headerlen, streambuffer + *buffersize, dsiz, csiz, streambuffer, hashtable);
		else		
			memcpy(streambuffer + *buffersize, source + headerlen, dsiz);
		memcpy(destination, streambuffer + *buffersize, dsiz);
		*buffersize += dsiz;
	}
	return dsiz;
}

#ifndef _MDFN_MEMORY_H

// These functions can be used from driver code or from internal Mednafen code.

#define MDFN_malloc(size, purpose) MDFN_malloc_real(size, purpose, __FILE__, __LINE__)
#define MDFN_calloc(nmemb, size, purpose) MDFN_calloc_real(nmemb, size, purpose, __FILE__, __LINE__)
#define MDFN_realloc(ptr, size, purpose) MDFN_realloc_real(ptr, size, purpose, __FILE__, __LINE__)

void *MDFN_malloc_real(uint32 size, const char *purpose, const char *_file, const int _line);
void *MDFN_calloc_real(uint32 nmemb, uint32 size, const char *purpose, const char *_file, const int _line);
void *MDFN_realloc_real(void *ptr, uint32 size, const char *purpose, const char *_file, const int _line);
void MDFN_free(void *ptr);

static INLINE void MDFN_FastU32MemsetM8(uint32 *array, uint32 value_32, unsigned int u32len)
{
 // gcc's optimizer is smart enough to use the floating-point unit to do this memset
 // on non-64-bit processors, at least on x86.
 uint64 value = (uint64)value_32 | ((uint64)value_32 << 32);
 uint64 *newarray = (uint64 *)array;

 u32len >>= 1;
 for(unsigned int i = 0; i < u32len; i++)
  *newarray++ = value;
}

static INLINE void MDFN_FastMemcpyM8(void *dest, void *src, unsigned int bytelen)
{
 unsigned int len = bytelen >> 3;
 uint64 *dest_64 = (uint64 *)dest;
 uint64 *src_64 = (uint64 *)src;

 do
 {
  *dest_64++ = *src_64++;
  len--;
 } while(len);
}


#define _MDFN_MEMORY_H
#endif

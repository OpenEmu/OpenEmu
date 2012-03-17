/**
 * GENS: Byteswapping functions.
 * These functions were ported from x86 assembler to C,
 * since they don't really need assembly optimizations.
 */


#ifndef GENS_BYTESWAP_H
#define GENS_BYTESWAP_H

#ifdef __cplusplus
extern "C" {
#endif


// Endianness defines ported from libsdl.
#define GENS_LIL_ENDIAN 1234
#define GENS_BIG_ENDIAN 4321
#ifndef GENS_BYTEORDER
#if defined(__hppa__) || \
    defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
    (defined(__MIPS__) && defined(__MIPSEB__)) || \
    defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) || \
    defined(__SPARC__)
#define GENS_BYTEORDER GENS_BIG_ENDIAN
#else
#define GENS_BYTEORDER GENS_LIL_ENDIAN
#endif
#endif


// 16-bit byteswap function.
void __byte_swap_16_array(void *ptr, int n);

// TODO: Optimize out ?e??_to_cpu_from_ptr on appropriate architectures.
#define be16_to_cpu_from_ptr(ptr) \
	(((ptr)[0] << 8) | ((ptr)[1]))
#define le16_to_cpu_from_ptr(ptr) \
	(((ptr)[1] << 8) | ((ptr)[0]))
#define be32_to_cpu_from_ptr(ptr) \
	(((ptr)[0] << 24) | ((ptr)[1] << 16) | ((ptr)[2] << 8) | ((ptr)[3]))
#define le32_to_cpu_from_ptr(ptr) \
	(((ptr)[3] << 24) | ((ptr)[2] << 16) | ((ptr)[1] << 8) | ((ptr)[0]))
#define cpu_to_le16_ucptr(ptr, val) \
	(ptr)[0] = ((val) & 0xFF);  \
	(ptr)[1] = (((val) >> 8) & 0xFF);
#define cpu_to_be16_ucptr(ptr, val) \
	(ptr)[0] = (((val) >> 8) & 0xFF);  \
	(ptr)[1] = ((val) & 0xFF);
#define cpu_to_le32_ucptr(ptr, val) \
	(ptr)[0] = ((val) & 0xFF);  \
	(ptr)[1] = (((val) >> 8) & 0xFF);  \
	(ptr)[2] = (((val) >> 16) & 0xFF); \
	(ptr)[3] = (((val) >> 24) & 0xFF);
#define cpu_to_be32_ucptr(ptr, val) \
	(ptr)[0] = (((val) >> 24) & 0xFF); \
	(ptr)[1] = (((val) >> 16) & 0xFF); \
	(ptr)[2] = (((val) >> 8) & 0xFF);  \
	(ptr)[3] = ((val) & 0xFF);

#if GENS_BYTEORDER == GENS_LIL_ENDIAN
#define be16_to_cpu_array(ptr, n) __byte_swap_16_array((ptr), (n));
#define le16_to_cpu_array(ptr, n)
#define cpu_to_be16_array(ptr, n) __byte_swap_16_array((ptr), (n));
#define cpu_to_le16_array(ptr, n)
#else // GENS_BYTEORDER == GENS_BIG_ENDIAN
#define be16_to_cpu_array(ptr, n)
#define le16_to_cpu_array(ptr, n) __byte_swap_16_array((ptr), (n));
#define cpu_to_be16_array(ptr, n)
#define cpu_to_le16_array(ptr, n) __byte_swap_16_array((ptr), (n));
#endif


#ifdef __cplusplus
}
#endif

#endif /* GENS_BYTESWAP_H */

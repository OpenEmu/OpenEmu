#ifndef _EMUTYPES_H_
#define _EMUTYPES_H_

#ifndef INLINE

#if defined(_MSC_VER)
#define INLINE __forceinline
#elif defined(__GNUC__)
#define INLINE __inline__
#elif defined(_MWERKS_)
#define INLINE inline
#else
#define INLINE
#endif
#endif

#if defined(EMU_DLL_IMPORTS)
#define EMU2149_DLL_IMPORTS
#define EMU2212_DLL_IMPORTS
#define EMU2413_DLL_IMPORTS
#define EMU8950_DLL_IMPORTS
#define EMU76489_DLL_IMPORTS
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int e_uint;
typedef signed int e_int;

typedef unsigned char e_uint8 ;
typedef signed char e_int8 ;

typedef unsigned short e_uint16 ;
typedef signed short e_int16 ;

typedef unsigned int e_uint32 ;
typedef signed int e_int32 ;

#ifdef __cplusplus
}
#endif
#endif

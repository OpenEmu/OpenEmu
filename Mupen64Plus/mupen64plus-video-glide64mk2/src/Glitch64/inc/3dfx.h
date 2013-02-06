/*
** THIS SOFTWARE IS SUBJECT TO COPYRIGHT PROTECTION AND IS OFFERED ONLY
** PURSUANT TO THE 3DFX GLIDE GENERAL PUBLIC LICENSE. THERE IS NO RIGHT
** TO USE THE GLIDE TRADEMARK WITHOUT PRIOR WRITTEN PERMISSION OF 3DFX
** INTERACTIVE, INC. A COPY OF THIS LICENSE MAY BE OBTAINED FROM THE 
** DISTRIBUTOR OR BY CONTACTING 3DFX INTERACTIVE INC(info@3dfx.com). 
** THIS PROGRAM IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER 
** EXPRESSED OR IMPLIED. SEE THE 3DFX GLIDE GENERAL PUBLIC LICENSE FOR A
** FULL TEXT OF THE NON-WARRANTY PROVISIONS.  
** 
** USE, DUPLICATION OR DISCLOSURE BY THE GOVERNMENT IS SUBJECT TO
** RESTRICTIONS AS SET FORTH IN SUBDIVISION (C)(1)(II) OF THE RIGHTS IN
** TECHNICAL DATA AND COMPUTER SOFTWARE CLAUSE AT DFARS 252.227-7013,
** AND/OR IN SIMILAR OR SUCCESSOR CLAUSES IN THE FAR, DOD OR NASA FAR
** SUPPLEMENT. UNPUBLISHED RIGHTS RESERVED UNDER THE COPYRIGHT LAWS OF
** THE UNITED STATES.  
** 
** COPYRIGHT 3DFX INTERACTIVE, INC. 1999, ALL RIGHTS RESERVED
**
** $Revision: 1.3.4.2 $
** $Date: 2003/05/05 06:50:41 $
*/
#ifndef __3DFX_H__
#define __3DFX_H__

/*
** basic data types
*/
typedef unsigned char   FxU8;
typedef signed   char   FxI8;
typedef unsigned short  FxU16;
typedef signed   short  FxI16;
#if defined(__alpha__) || defined (__LP64__)
typedef signed   int    FxI32;
typedef unsigned int    FxU32;
#else
typedef signed   long   FxI32;
typedef unsigned long   FxU32;
#endif
typedef unsigned long   AnyPtr;
typedef int             FxBool;
typedef float           FxFloat;
typedef double          FxDouble;

/*
** color types
*/
typedef unsigned long                FxColor_t;
typedef struct { float r, g, b, a; } FxColor4;

/*
** fundamental types
*/
#define FXTRUE    1
#define FXFALSE   0

/*
** helper macros
*/
#define FXUNUSED( a ) ((void)(a))
#define FXBIT( i )    ( 1L << (i) )

/*
** export macros
*/

#if defined(__MSC__) || defined(_MSC_VER)
#  if defined (MSVC16)
#    define FX_ENTRY 
#    define FX_CALL
#  else
#    define FX_ENTRY __declspec(dllexport) extern
#    define FX_CALL  __stdcall
#  endif
#elif defined(__WATCOMC__)
#  define FX_ENTRY extern
#  define FX_CALL  __stdcall
#elif defined (__IBMC__) || defined (__IBMCPP__)
   /*  IBM Visual Age C/C++: */
#  define FX_ENTRY extern
#  define FX_CALL  __stdcall
#elif defined(__DJGPP__)
#  define FX_ENTRY extern
#  define FX_CALL
#elif defined(__MINGW32__)
#  define FX_ENTRY extern
#  define FX_CALL  __stdcall
#elif defined(__unix__)
#  define FX_ENTRY extern
#  define FX_CALL
#elif defined(__APPLE__)
#  define FX_ENTRY extern
#  define FX_CALL
#elif defined(__MWERKS__)
#  if macintosh
#    define FX_ENTRY extern
#    define FX_CALL
#  else /* !macintosh */
#    error "Unknown MetroWerks target platform"
#  endif /* !macintosh */
#else
#  warning define FX_ENTRY & FX_CALL for your compiler
#  define FX_ENTRY extern
#  define FX_CALL
#endif

/*
** x86 compiler specific stuff
*/
#if defined(__BORLANDC_)
#  define REALMODE

#  define REGW( a, b ) ((a).x.b)
#  define REGB( a, b ) ((a).h.b)
#  define INT86( a, b, c ) int86(a,b,c)
#  define INT86X( a, b, c, d ) int86x(a,b,c,d)

#  define RM_SEG( a ) FP_SEG( a )
#  define RM_OFF( a ) FP_OFF( a )
#elif defined(__WATCOMC__)
#  undef FP_SEG
#  undef FP_OFF

#  define REGW( a, b ) ((a).w.b)
#  define REGB( a, b ) ((a).h.b)
#  define INT86( a, b, c ) int386(a,b,c)
#  define INT86X( a, b, c, d ) int386x(a,b,c,d)

#  define RM_SEG( a )  ( ( ( ( FxU32 ) (a) ) & 0x000F0000 ) >> 4 )
#  define RM_OFF( a )  ( ( FxU16 ) (a) )
#endif

#endif /* !__3DFX_H__ */

/*******************************************************************************
*                                                                              *
*  Define size independent data types and operations.                          *
*                                                                              *
*   The following types must be supported by all platforms:                    *
*                                                                              *
*  UINT8  - Unsigned 8-bit Integer    INT8  - Signed 8-bit integer             *
*  UINT16 - Unsigned 16-bit Integer  INT16 - Signed 16-bit integer             *
*  UINT32 - Unsigned 32-bit Integer  INT32 - Signed 32-bit integer             *
*                                                                              *
*******************************************************************************/

#ifndef OSD_CPU_H
#define OSD_CPU_H

#undef TRUE
#undef FALSE
#define TRUE  1
#define FALSE 0

typedef unsigned char   UINT8;
typedef unsigned short  UINT16;
typedef unsigned int    UINT32;
typedef signed char     INT8;
typedef signed short    INT16;
typedef signed int      INT32;

/******************************************************************************
 * Union of UINT8, UINT16 and UINT32 in native endianess of the target
 * This is used to access bytes and words in a machine independent manner.
 * The upper bytes h2 and h3 normally contain zero (16 bit CPU cores)
 * thus PAIR.d can be used to pass arguments to the memory system
 * which expects 'int' really.
 ******************************************************************************/

typedef union {
#ifdef LSB_FIRST
  struct { UINT8 l,h,h2,h3; } b;
  struct { UINT16 l,h; } w;
#else
  struct { UINT8 h3,h2,h,l; } b;
  struct { UINT16 h,l; } w;
#endif
  UINT32 d;
}  PAIR;

#endif  /* defined OSD_CPU_H */

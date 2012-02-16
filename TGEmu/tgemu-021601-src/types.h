/* OpenEmu change
 #undef uint8
 #undef uint16
 #undef uint32
 
 #define uint8  unsigned char
 #define uint16 unsigned short
 #define uint32 unsigned int
 #define int8  signed char
 #define int16 signed short
 #define int32 signed long int
 */

#ifndef _TYPES_H_
#define _TYPES_H_

#include <stdint.h>

#undef uint8
#undef uint16
#undef uint32

#undef uint8
#undef uint16
#undef uint32

#define uint8  uint8_t
#define uint16 uint16_t
#define uint32 uint32_t
#define int8  int8_t
#define int16 int16_t
#define int32 int32_t

#endif /* _TYPES_H_ */

/*
#ifndef _TYPES_H_
#define _TYPES_H_

typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned long int uint32;

typedef signed char int8;
typedef signed short int int16;
typedef signed long int int32;

#endif*/ /* _TYPES_H_ */

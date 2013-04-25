#ifndef IDE_H_
#define IDE_H_

#include "atari.h"
#ifdef HAVE_INTTYPES_H
#  include <inttypes.h>
#else
   typedef signed char int8_t;
   typedef unsigned char  uint8_t;
   typedef short int16_t;
   typedef unsigned short uint16_t;
   typedef int  int32_t;
   typedef unsigned uint32_t;
   typedef long long int64_t;
   typedef unsigned long long uint64_t;
#endif

extern int IDE_enabled;

int     IDE_Initialise(int *argc, char *argv[]);
void IDE_Exit(void);
uint8_t IDE_GetByte(uint16_t addr, int no_side_effects);
void    IDE_PutByte(uint16_t addr, uint8_t byte);

#endif

#ifndef __STELLA_H
#define __STELLA_H

#include "../mednafen.h"
#include "../state.h"
#include "../general.h"

//#include "interrupt.h"

namespace MDFN_IEN_STELLA
{

#define  mBCD(value) (((value)/10)<<4)|((value)%10)

extern          uint32 rom_size;
extern          int wsc;

}

#endif

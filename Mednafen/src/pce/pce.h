#ifndef _PCE_H

#include "../types.h"
#include "../mednafen.h"
#include "../state.h"
#include "../general.h"
#include "../memory.h"

#define DECLFR(x) uint8 x (uint32 A)
#define DECLFW(x) void x (uint32 A, uint8 V)

#include "huc6280.h"


extern bool PCE_InDebug;
extern bool PCE_ACEnabled; // Arcade Card emulation enabled?
void PCE_Power(void);

extern readfunc PCERead[0x100];
extern writefunc PCEWrite[0x100];
extern uint8 PCEIODataBuffer;
extern int pce_overclocked;

#define _PCE_H
#endif

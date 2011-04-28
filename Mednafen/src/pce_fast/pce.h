#ifndef _PCE_H

#include "../types.h"
#include "../mednafen.h"
#include "../state.h"
#include "../general.h"
#include "../memory.h"

#define PCE_MASTER_CLOCK        21477272.727273

#define DECLFR(x) uint8 MDFN_FASTCALL x (uint32 A)
#define DECLFW(x) void MDFN_FASTCALL x (uint32 A, uint8 V)

namespace PCE_Fast
{
extern uint8 ROMSpace[0x88 * 8192 + 8192];

typedef void (MDFN_FASTCALL *writefunc)(uint32 A, uint8 V);
typedef uint8 (MDFN_FASTCALL *readfunc)(uint32 A);

extern uint8 PCEIODataBuffer;

bool PCE_InitCD(void);

};

#include "huc6280.h"

namespace PCE_Fast
{
extern bool PCE_ACEnabled; // Arcade Card emulation enabled?
void PCE_Power(void);

extern readfunc PCERead[0x100];
extern writefunc PCEWrite[0x100];
extern int pce_overclocked;

extern uint8 BaseRAM[32768 + 8192];

};

using namespace PCE_Fast;

#define _PCE_H
#endif

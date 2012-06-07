#ifndef _PCE_H

#include "../mednafen.h"
#include "../state.h"
#include "../general.h"

namespace MDFN_IEN_PCE
{

#define PCE_MASTER_CLOCK	21477272.727273

#define DECLFR(x) uint8 x (uint32 A)
#define DECLFW(x) void x (uint32 A, uint8 V)

};

#include "huc6280/huc6280.h"

namespace MDFN_IEN_PCE
{
extern HuC6280 *HuCPU;

extern uint32 PCE_InDebug;
extern bool PCE_ACEnabled; // Arcade Card emulation enabled?
void PCE_Power(void);

void PCE_MidSync(void);

uint8 PCE_PeekMainRAM(uint32 A);
void PCE_PokeMainRAM(uint32 A, uint8 V);

};

#define _PCE_H
#endif

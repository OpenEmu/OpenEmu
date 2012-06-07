#ifndef __MDFN_PCE_HUC_H
#define __MDFN_PCE_HUC_H

namespace MDFN_IEN_PCE
{

typedef enum
{
 SYSCARD_NONE = 0,
 SYSCARD_1,
 SYSCARD_2,
 SYSCARD_3,
 SYSCARD_ARCADE		// 3.0 + extras
} SysCardType;

int HuCLoad(const uint8 *data, uint32 len, uint32 crc32, bool DisableBRAM = false, SysCardType syscard = SYSCARD_NONE);

void HuC_EndFrame(int32 timestamp);

void HuCClose(void);
int HuC_StateAction(StateMem *sm, int load, int data_only);

void HuC_Power(void);

DECLFR(PCE_ACRead);
DECLFW(PCE_ACWrite);

extern bool PCE_IsCD;
extern bool IsTsushin;

// Debugger support functions.
bool HuC_IsBRAMAvailable(void);
uint8 HuC_PeekBRAM(uint32 A);
void HuC_PokeBRAM(uint32 A, uint8 V);
};

#endif

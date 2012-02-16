#ifndef _FCEU_CHEAT_H
#define _FCEU_CHEAT_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void FCEU_CheatResetRAM(void);
void FCEU_CheatAddRAM(int s, uint32 A, uint8 *p);

void FCEU_LoadGameCheats(FILE *override);
void FCEU_FlushGameCheats(FILE *override, int nosave);
void FCEU_ApplyPeriodicCheats(void);
void FCEU_PowerCheats(void);
const char * FCEUI_GetCheatLabel(unsigned int which);

#ifdef __cplusplus
}
#endif

#endif

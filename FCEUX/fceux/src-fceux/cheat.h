#ifndef __FCEU_CHEAT_H
#define __FCEU_CHEAT_H

void FCEU_CheatResetRAM(void);
void FCEU_CheatAddRAM(int s, uint32 A, uint8 *p);

void FCEU_LoadGameCheats(FILE *override);
void FCEU_FlushGameCheats(FILE *override, int nosave);
void FCEU_ApplyPeriodicCheats(void);
void FCEU_PowerCheats(void);

const char * FCEUI_GetCheatLabel(uint32 which);
int FCEU_CheatGetByte(uint32 A);
void FCEU_CheatSetByte(uint32 A, uint8 V);
void FCEUI_DisableAllCheats(void);

#endif

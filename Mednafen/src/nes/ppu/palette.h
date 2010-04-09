#ifndef _PPU_PALETTEH
#define _PPU_PALETTEH

#include "palette-driver.h"

extern MDFNPalStruct *palo;
extern MDFNPalStruct MDFNPalette[256];
extern int MDFNPaletteChanged;

void MDFN_ResetPalette(void);
void MDFN_ResetMessages();
void MDFN_LoadGamePalette(void);

void MDFN_SetPPUPalette(int fishie);

#endif

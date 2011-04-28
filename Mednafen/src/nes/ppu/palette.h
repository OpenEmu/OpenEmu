#ifndef _PPU_PALETTEH
#define _PPU_PALETTEH

typedef struct {
        uint8 r,g,b;
} MDFNPalStruct;

extern MDFNPalStruct ActiveNESPalette[0x200];

void MDFN_InitPalette(const unsigned int which);

#endif

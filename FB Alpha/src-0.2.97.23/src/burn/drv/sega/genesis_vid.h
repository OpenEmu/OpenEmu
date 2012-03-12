#include "driver.h"

extern UINT16 GenesisPaletteBase;
extern UINT16 GenesisBgPalLookup[4];
extern UINT16 GenesisSpPalLookup[4];

INT32 StartGenesisVDP(INT32 ScreenNum, UINT32* pal);
void GenesisVDPExit();
void GenesisVDPScan();
UINT16 GenesisVDPRead(UINT32 offset);
void GenesisVDPWrite(UINT32 offset, UINT16 data);
void vdp_drawline(UINT16 *bitmap, UINT32 line, INT32 bgfill);

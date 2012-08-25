#include "burnint.h"
#include "m68000_intf.h"
#include "z80_intf.h"
#include "eeprom.h"

#define CAVE_REFRESHRATE (15625.0 / 271.5)

inline static void CaveClearOpposites(UINT16* nJoystickInputs)
{
	if ((*nJoystickInputs & 0x0003) == 0x0003) {
		*nJoystickInputs &= ~0x0003;
	}
	if ((*nJoystickInputs & 0x000C) == 0x000C) {
		*nJoystickInputs &= ~0x000C;
	}
}

// cave.cpp
extern INT32 nCaveXSize, nCaveYSize;
extern INT32 nCaveXOffset, nCaveYOffset;
extern INT32 nCaveExtraXOffset, nCaveExtraYOffset;
extern INT32 nCaveRowModeOffset;

INT32 CaveScanGraphics();
void CaveClearScreen(UINT32 nColour);

// cave_palette.cpp
extern UINT32* CavePalette;

extern UINT8* CavePalSrc;
extern UINT8 CaveRecalcPalette;

INT32 CavePalInit(INT32 nPalSize);
INT32 CavePalExit();
INT32 CavePalUpdate4Bit(INT32 nOffset, INT32 nNumPalettes);
INT32 CavePalUpdate8Bit(INT32 nOffset, INT32 nNumPalettes);

void CavePalWriteByte(UINT32 nAddress, UINT8 byteValue);
void CavePalWriteWord(UINT32 nAddress, UINT16 wordValue);

// cave_tiles.cpp
extern UINT8* CaveTileROM[4];
extern UINT8* CaveTileRAM[4];

extern UINT32 CaveTileReg[4][3];
extern INT32 nCaveTileBank;

INT32 CaveTileRender(INT32 nMode);
void CaveTileExit();
INT32 CaveTileInit();
INT32 CaveTileInitLayer(INT32 nLayer, INT32 nROMSize, INT32 nBitdepth, INT32 nOffset);

// cave_sprite.cpp
extern INT32 CaveSpriteVisibleXOffset;

extern UINT8* CaveSpriteROM;
extern UINT8* CaveSpriteRAM;

extern INT32 nCaveSpriteBank;
extern INT32 nCaveSpriteBankDelay;

extern INT32 (*CaveSpriteBuffer)();
extern INT32 CaveSpriteRender(INT32 nLowPriority, INT32 nHighPriority);
void CaveSpriteExit();
INT32 CaveSpriteInit(INT32 nType, INT32 nROMSize);

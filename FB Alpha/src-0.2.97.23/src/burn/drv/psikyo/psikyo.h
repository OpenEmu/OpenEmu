#include "burnint.h"
#include "m68000_intf.h"
#include "z80_intf.h"

// General
enum {
	PSIKYO_HW_SAMURAIA,
	PSIKYO_HW_GUNBIRD,
	PSIKYO_HW_S1945,
	PSIKYO_HW_TENGAI
};

extern INT32 PsikyoHardwareVersion;

// palette
INT32 PsikyoPalInit();
INT32 PsikyoPalExit();
INT32 PsikyoPalUpdate();

void PsikyoPalWriteByte(UINT32 nAddress, UINT8 byteValue);
void PsikyoPalWriteWord(UINT32 nAddress, UINT16 wordValue);

extern UINT8* PsikyoPalSrc;
extern UINT8 PsikyoRecalcPalette;
extern UINT32* PsikyoPalette;

// Tile rendering
INT32 PsikyoTileRender();
void PsikyoSetTileBank(INT32 nLayer, INT32 nBank);
INT32 PsikyoTileInit(UINT32 nROMSize);
void PsikyoTileExit();

extern UINT8* PsikyoTileROM;
extern UINT8* PsikyoTileRAM[3];

extern bool bPsikyoClearBackground;

// Sprite rendering
INT32 PsikyoSpriteInit(INT32 nROMSize);
void PsikyoSpriteExit();
INT32 PsikyoSpriteBuffer();
INT32 PsikyoSpriteRender(INT32 nLowPriority, INT32 nHighPriority);

extern UINT8* PsikyoSpriteROM;
extern UINT8* PsikyoSpriteRAM;
extern UINT8* PsikyoSpriteLUT;

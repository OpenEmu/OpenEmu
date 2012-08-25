#include "burnint.h"
#include "m68000_intf.h"
#include "z80_intf.h"
#include "msm6295.h"
#include "burn_ym2151.h"
#include "burn_ym3812.h"

// Uncomment to have the driver perform the screen rotation (faster unless 3D hardware is used for blitting).
// #undef DRIVER_ROTATION
// #define DRIVER_ROTATION

const INT32 TOA_68K_SPEED = 16000000;
const INT32 TOA_Z80_SPEED = 4000000;
const INT32 TOA_VBLANK_LINES = 22;

extern INT32 Hellfire;
extern INT32 Rallybik;

// toaplan.cpp
extern INT32 nToaCyclesScanline;
extern INT32 nToaCyclesDisplayStart;
extern INT32 nToaCyclesVBlankStart;

INT32 ToaLoadCode(UINT8 *Rom, INT32 nStart, INT32 nCount);
INT32 ToaLoadGP9001Tiles(UINT8* pDest, INT32 nStart, INT32 nNumFiles, INT32 nROMSize, bool bSwap = false);

void ToaClearScreen(INT32 PalOffset);

void ToaZExit();

extern UINT8* RomZ80;
extern UINT8* RamZ80;

extern INT32 nCyclesDone[2], nCyclesTotal[2];
extern INT32 nCyclesSegment;

#ifdef DRIVER_ROTATION
 extern bool bToaRotateScreen;
 #define TOA_ROTATE_GRAPHICS_CCW 0
#else
 const bool bToaRotateScreen = false;
 #define TOA_ROTATE_GRAPHICS_CCW BDF_ORIENTATION_VERTICAL
#endif

extern UINT8* pBurnBitmap;
extern INT32 nBurnColumn;
extern INT32 nBurnRow;

inline void ToaGetBitmap()
{
	if (bToaRotateScreen) {
		pBurnBitmap = pBurnDraw + 319 * nBurnPitch;
		nBurnColumn = -nBurnPitch;
		nBurnRow = nBurnBpp;
	} else {
		pBurnBitmap = pBurnDraw;
		nBurnColumn = nBurnBpp;
		nBurnRow = nBurnPitch;
	}
}

inline void ToaClearOpposites(UINT8* nJoystickInputs)
{
	if ((*nJoystickInputs & 0x03) == 0x03) {
		*nJoystickInputs &= ~0x03;
	}
	if ((*nJoystickInputs & 0x0C) == 0x0C) {
		*nJoystickInputs &= ~0x0C;
	}
}

// toa_pal.cpp
extern UINT8 *ToaPalSrc;
extern UINT32* ToaPalette;
extern UINT8 ToaRecalcPalette;
extern INT32 nToaPalLen;

INT32 ToaPalInit();
INT32 ToaPalExit();
INT32 ToaPalUpdate();

// toa_gp9001.cpp
extern UINT8* GP9001ROM[2];
extern UINT32 nGP9001ROMSize[2];

extern UINT32 GP9001TileBank[8];

extern UINT8* GP9001RAM[2];
extern UINT16* GP9001Reg[2];

extern INT32 nSpriteXOffset, nSpriteYOffset, nSpritePriority;

extern INT32 nLayer0XOffset, nLayer0YOffset, nLayer0Priority;
extern INT32 nLayer1XOffset, nLayer1YOffset, nLayer1Priority;
extern INT32 nLayer2XOffset, nLayer2YOffset, nLayer2Priority;
extern INT32 nLayer3XOffset, nLayer3YOffset;

INT32 ToaBufferGP9001Sprites();
INT32 ToaRenderGP9001();
INT32 ToaInitGP9001(INT32 n = 1);
INT32 ToaExitGP9001();
INT32 ToaScanGP9001(INT32 nAction, INT32* pnMin);

inline static void ToaGP9001SetRAMPointer(UINT32 wordValue, const INT32 nController = 0)
{
	extern UINT8* GP9001Pointer[2];

	wordValue &= 0x1FFF;
	GP9001Pointer[nController] = GP9001RAM[nController] + (wordValue << 1);
}

inline static void ToaGP9001WriteRAM(const UINT16 wordValue, const INT32 nController)
{
	extern UINT8* GP9001Pointer[2];

	*((UINT16*)(GP9001Pointer[nController])) = BURN_ENDIAN_SWAP_INT16(wordValue);
	GP9001Pointer[nController] += 2;
}

inline static UINT16 ToaGP9001ReadRAM_Hi(const INT32 nController)
{
	extern UINT8* GP9001Pointer[2];

	return BURN_ENDIAN_SWAP_INT16(*((UINT16*)(GP9001Pointer[nController])));
}

inline static UINT16 ToaGP9001ReadRAM_Lo(const INT32 nController)
{
	extern UINT8* GP9001Pointer[2];

	return BURN_ENDIAN_SWAP_INT16(*((UINT16*)(GP9001Pointer[nController] + 2)));
}

inline static void ToaGP9001SelectRegister(const UINT16 wordValue, const INT32 nController = 0)
{
	extern INT32 GP9001Regnum[2];

	GP9001Regnum[nController] = wordValue & 0xFF;
}

inline static void ToaGP9001WriteRegister(const UINT16 wordValue, const INT32 nController = 0)
{
	extern INT32 GP9001Regnum[2];

	GP9001Reg[nController][GP9001Regnum[nController]] = wordValue;
}

inline static UINT16 ToaVBlankRegister()
{
	INT32 nCycles = SekTotalCycles();

	if (nCycles >= nToaCyclesVBlankStart) {
		return 1;
	} else {
		if (nCycles < nToaCyclesDisplayStart) {
			return 1;
		}
	}
	return 0;
}

inline static UINT16 ToaScanlineRegister()
{
	static INT32 nPreviousScanline;
	UINT16 nFlags = 0xFE00;
	INT32 nCurrentScanline = SekCurrentScanline();

#if 0
	// None of the games actually use this
	INT32 nCurrentBeamPosition = SekTotalCycles() % nToaCyclesScanline;
	if (nCurrentBeamPosition < 64) {
		nFlags &= ~0x4000;
	}
#endif

	if (nCurrentScanline != nPreviousScanline) {
		nPreviousScanline = nCurrentScanline;
		nFlags &= ~0x8000;

//		bprintf(PRINT_NORMAL, _T("  - line %3i, PC 0x%08X\n"), nCurrentScanline, SekGetPC(-1));

	}

	return nFlags | nCurrentScanline;
}

// toa_extratext.cpp
extern UINT8* ExtraTROM;
extern UINT8* ExtraTRAM;
extern UINT8* ExtraTScroll;
extern UINT8* ExtraTSelect;
extern INT32 nExtraTXOffset;

INT32 ToaExtraTextLayer();
INT32 ToaExtraTextInit();
void ToaExtraTextExit();

// toa_bcu2.cpp
extern INT32 ToaOpaquePriority;
extern UINT8* ToaPalSrc2;
extern UINT32* ToaPalette2;

extern UINT8* BCU2ROM;
extern UINT8* FCU2ROM;

extern UINT8* BCU2RAM;
extern UINT8* FCU2RAM;
extern UINT8* FCU2RAMSize;

extern UINT32 BCU2Pointer;
extern UINT32 FCU2Pointer;

extern UINT32 nBCU2ROMSize;
extern UINT32 nFCU2ROMSize;

extern UINT16 BCU2Reg[8];

extern INT32 nBCU2TileXOffset;
extern INT32 nBCU2TileYOffset;

INT32 ToaPal2Update();
INT32 ToaInitBCU2();
INT32 ToaExitBCU2();
void ToaBufferFCU2Sprites();
INT32 ToaRenderBCU2();

// toaplan1.cpp
extern INT32 nToa1Cycles68KSync;

INT32 ToaLoadTiles(UINT8* pDest, INT32 nStart, INT32 nROMSize);
void toaplan1FMIRQHandler(INT32, INT32 nStatus);
INT32 toaplan1SynchroniseStream(INT32 nSoundRate);
UINT8 __fastcall toaplan1ReadByteZ80RAM(UINT32 sekAddress);
UINT16 __fastcall toaplan1ReadWordZ80RAM(UINT32 sekAddress);
void __fastcall toaplan1WriteByteZ80RAM(UINT32 sekAddress, UINT8 byteValue);
void __fastcall toaplan1WriteWordZ80RAM(UINT32 sekAddress, UINT16 wordValue);

inline void ToaBCU2SetRAMPointer(UINT32 wordValue)
{
	BCU2Pointer = (wordValue & 0x3FFF) << 1;
}

inline UINT16 ToaBCU2GetRAMPointer()
{
	return (BCU2Pointer >> 1) & 0x3FFF;
}

inline void ToaBCU2WriteRAM(const UINT16 wordValue)
{
	((UINT16*)BCU2RAM)[BCU2Pointer & 0x7FFF] = wordValue;
	BCU2Pointer++;
}

inline UINT16 ToaBCU2ReadRAM_Hi()
{
	return ((UINT16*)BCU2RAM)[BCU2Pointer & 0x7FFF];
}

inline UINT16 ToaBCU2ReadRAM_Lo()
{
	return ((UINT16*)BCU2RAM)[(BCU2Pointer & 0x7FFF) + 1];
}

inline void ToaFCU2SetRAMPointer(UINT32 wordValue)
{
	FCU2Pointer = wordValue & 0x03FF;
}

inline UINT16 ToaFCU2GetRAMPointer()
{
	return FCU2Pointer & 0x03FF;
}

inline void ToaFCU2WriteRAM(const UINT16 wordValue)
{
	((UINT16*)FCU2RAM)[FCU2Pointer & 0x03FF] = wordValue;
	FCU2Pointer++;
}

inline void ToaFCU2WriteRAMSize(const UINT16 wordValue)
{
	((UINT16*)FCU2RAMSize)[FCU2Pointer & 0x003F] = wordValue;
	FCU2Pointer++;
}

inline UINT16 ToaFCU2ReadRAM()
{
	return ((UINT16*)FCU2RAM)[FCU2Pointer & 0x03FF];
}

inline UINT16 ToaFCU2ReadRAMSize()
{
	return ((UINT16*)FCU2RAMSize)[FCU2Pointer & 0x003F];
}

// d_battleg.cpp
extern INT32 Bgareggabl;

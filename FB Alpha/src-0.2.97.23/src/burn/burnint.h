// Burn - Arcade emulator library - internal code

// Standard headers
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#if defined(__LIBSNES__) && defined(_MSC_VER)
#include <tchar.h>
#else
#include "tchar.h"
#endif

#include "burn.h"

#ifdef LSB_FIRST
typedef union
{
	struct { UINT8 l,h,h2,h3; } b;
	struct { UINT16 l,h; } w;
	UINT32 d;
} PAIR;

#define BURN_ENDIAN_SWAP_INT8(x)				x
#define BURN_ENDIAN_SWAP_INT16(x)				x
#define BURN_ENDIAN_SWAP_INT32(x)				x
#define BURN_ENDIAN_SWAP_INT64(x)				x
#else
// define the above union and BURN_ENDIAN_SWAP macros in the following platform specific header
#include "burn_endian.h"
#endif

// ---------------------------------------------------------------------------
// Driver information

struct BurnDriver {
	char* szShortName;			// The filename of the zip file (without extension)
	char* szParent;				// The filename of the parent (without extension, NULL if not applicable)
	char* szBoardROM;			// The filename of the board ROMs (without extension, NULL if not applicable)
	char* szSampleName;			// The filename of the samples zip file (without extension, NULL if not applicable)
	char* szDate;

	// szFullNameA, szCommentA, szManufacturerA and szSystemA should always contain valid info
	// szFullNameW, szCommentW, szManufacturerW and szSystemW should be used only if characters or scripts are needed that ASCII can't handle
	char*    szFullNameA; char*    szCommentA; char*    szManufacturerA; char*    szSystemA;
	wchar_t* szFullNameW; wchar_t* szCommentW; wchar_t* szManufacturerW; wchar_t* szSystemW;

	INT32 Flags;			// See burn.h
	INT32 Players;		// Max number of players a game supports (so we can remove single player games from netplay)
	INT32 Hardware;		// Which type of hardware the game runs on
	INT32 Genre;
	INT32 Family;
	INT32 (*GetZipName)(char** pszName, UINT32 i);				// Function to get possible zip names
	INT32 (*GetRomInfo)(struct BurnRomInfo* pri, UINT32 i);		// Function to get the length and crc of each rom
	INT32 (*GetRomName)(char** pszName, UINT32 i, INT32 nAka);	// Function to get the possible names for each rom
	INT32 (*GetSampleInfo)(struct BurnSampleInfo* pri, UINT32 i);		// Function to get the sample flags
	INT32 (*GetSampleName)(char** pszName, UINT32 i, INT32 nAka);	// Function to get the possible names for each sample
	INT32 (*GetInputInfo)(struct BurnInputInfo* pii, UINT32 i);	// Function to get the input info for the game
	INT32 (*GetDIPInfo)(struct BurnDIPInfo* pdi, UINT32 i);		// Function to get the input info for the game
	INT32 (*Init)(); INT32 (*Exit)(); INT32 (*Frame)(); INT32 (*Redraw)(); INT32 (*AreaScan)(INT32 nAction, INT32* pnMin);
	UINT8* pRecalcPal; UINT32 nPaletteEntries;										// Set to 1 if the palette needs to be fully re-calculated
	INT32 nWidth, nHeight; INT32 nXAspect, nYAspect;					// Screen width, height, x/y aspect
};

#define BurnDriverD BurnDriver		// Debug status
#define BurnDriverX BurnDriver		// Exclude from build

// Standard functions for dealing with ROM and input info structures
#include "stdfunc.h"

// ---------------------------------------------------------------------------

// burn.cpp
INT32 BurnSetRefreshRate(double dRefreshRate);
INT32 BurnByteswap(UINT8* pm,INT32 nLen);
INT32 BurnClearScreen();

// load.cpp
INT32 BurnLoadRom(UINT8* Dest, INT32 i, INT32 nGap);
INT32 BurnXorRom(UINT8* Dest, INT32 i, INT32 nGap);
INT32 BurnLoadBitField(UINT8* pDest, UINT8* pSrc, INT32 nField, INT32 nSrcLen);

// ---------------------------------------------------------------------------
// Colour-depth independant image transfer

extern UINT16* pTransDraw;

void BurnTransferClear();
INT32 BurnTransferCopy(UINT32* pPalette);
void BurnTransferExit();
INT32 BurnTransferInit();

// ---------------------------------------------------------------------------
// Plotting pixels

inline static void PutPix(UINT8* pPix, UINT32 c)
{
	if (nBurnBpp >= 4) {
		*((UINT32*)pPix) = c;
	} else {
		if (nBurnBpp == 2) {
			*((UINT16*)pPix) = (UINT16)c;
		} else {
			pPix[0] = (UINT8)(c >>  0);
			pPix[1] = (UINT8)(c >>  8);
			pPix[2] = (UINT8)(c >> 16);
		}
	}
}

// ---------------------------------------------------------------------------
// Setting up cpus for cheats

struct cpu_core_config {
	void (*open)(INT32);		// cpu open
	void (*close)();		// cpu close

	UINT8 (*read)(UINT32);		// read
	void (*write)(UINT32, UINT8);	// write
	INT32 (*active)();		// active cpu
	INT32 (*totalcycles)();		// total cycles
	void (*newframe)();		// new frame

	INT32 (*run)(INT32);		// execute cycles
	void (*runend)();		// end run
	void (*reset)();		// reset cpu

	UINT64 nMemorySize;		// how large is our memory range?
	UINT32 nAddressXor;		// fix endianness for some cpus
};

void CpuCheatRegister(INT32 type, cpu_core_config *config);

// burn_memory.cpp
void BurnInitMemoryManager();
UINT8 *BurnMalloc(INT32 size);
void _BurnFree(void *ptr);
#define BurnFree(x)		_BurnFree(x); x = NULL;
void BurnExitMemoryManager();

// ---------------------------------------------------------------------------
// Sound clipping macro
#define BURN_SND_CLIP(A) ((A) < -0x8000 ? -0x8000 : (A) > 0x7fff ? 0x7fff : (A))

// ---------------------------------------------------------------------------
// Debug Tracker

extern UINT8 Debug_BurnTransferInitted;
extern UINT8 Debug_BurnGunInitted;
extern UINT8 Debug_BurnLedInitted;
extern UINT8 Debug_HiscoreInitted;
extern UINT8 Debug_GenericTilesInitted;

extern UINT8 DebugDev_8255PPIInitted;
extern UINT8 DebugDev_EEPROMInitted;
extern UINT8 DebugDev_PandoraInitted;
extern UINT8 DebugDev_SeibuSndInitted;
extern UINT8 DebugDev_TimeKprInitted;

extern UINT8 DebugSnd_AY8910Initted;
extern UINT8 DebugSnd_Y8950Initted;
extern UINT8 DebugSnd_YM2151Initted;
extern UINT8 DebugSnd_YM2203Initted;
extern UINT8 DebugSnd_YM2413Initted;
extern UINT8 DebugSnd_YM2608Initted;
extern UINT8 DebugSnd_YM2610Initted;
extern UINT8 DebugSnd_YM2612Initted;
extern UINT8 DebugSnd_YM3526Initted;
extern UINT8 DebugSnd_YM3812Initted;
extern UINT8 DebugSnd_YMF278BInitted;
extern UINT8 DebugSnd_DACInitted;
extern UINT8 DebugSnd_ES5506Initted;
extern UINT8 DebugSnd_ES8712Initted;
extern UINT8 DebugSnd_ICS2115Initted;
extern UINT8 DebugSnd_IremGA20Initted;
extern UINT8 DebugSnd_K007232Initted;
extern UINT8 DebugSnd_K051649Initted;
extern UINT8 DebugSnd_K053260Initted;
extern UINT8 DebugSnd_K054539Initted;
extern UINT8 DebugSnd_MSM5205Initted;
extern UINT8 DebugSnd_MSM6295Initted;
extern UINT8 DebugSnd_NamcoSndInitted;
extern UINT8 DebugSnd_RF5C68Initted;
extern UINT8 DebugSnd_SAA1099Initted;
extern UINT8 DebugSnd_SamplesInitted;
extern UINT8 DebugSnd_SegaPCMInitted;
extern UINT8 DebugSnd_SN76496Initted;
extern UINT8 DebugSnd_UPD7759Initted;
extern UINT8 DebugSnd_X1010Initted;
extern UINT8 DebugSnd_YMZ280BInitted;

extern UINT8 DebugCPU_ARM7Initted;
extern UINT8 DebugCPU_ARMInitted;
extern UINT8 DebugCPU_H6280Initted;
extern UINT8 DebugCPU_HD6309Initted;
extern UINT8 DebugCPU_KonamiInitted;
extern UINT8 DebugCPU_M6502Initted;
extern UINT8 DebugCPU_M6800Initted;
extern UINT8 DebugCPU_M6805Initted;
extern UINT8 DebugCPU_M6809Initted;
extern UINT8 DebugCPU_S2650Initted;
extern UINT8 DebugCPU_SekInitted;
extern UINT8 DebugCPU_VezInitted;
extern UINT8 DebugCPU_ZetInitted;

extern UINT8 DebugCPU_I8039Initted;
extern UINT8 DebugCPU_SH2Initted;

void DebugTrackerExit();

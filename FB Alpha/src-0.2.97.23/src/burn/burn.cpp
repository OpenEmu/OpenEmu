// Burn - Drivers module

#include "version.h"
#include "burnint.h"
#include "burn_sound.h"
#include "driverlist.h"

// filler function, used if the application is not printing debug messages
static INT32 __cdecl BurnbprintfFiller(INT32, TCHAR* , ...) { return 0; }
// pointer to burner printing function
INT32 (__cdecl *bprintf)(INT32 nStatus, TCHAR* szFormat, ...) = BurnbprintfFiller;

INT32 nBurnVer = BURN_VERSION;		// Version number of the library

UINT32 nBurnDrvCount = 0;		// Count of game drivers
UINT32 nBurnDrvActive = ~0U;	// Which game driver is selected
UINT32 nBurnDrvSelect[8] = { ~0U, ~0U, ~0U, ~0U, ~0U, ~0U, ~0U, ~0U }; // Which games are selected (i.e. loaded but not necessarily active)
									
bool bBurnUseMMX;
#if defined BUILD_A68K
bool bBurnUseASMCPUEmulation = true;
#else
bool bBurnUseASMCPUEmulation = false;
#endif

#if defined (FBA_DEBUG)
 clock_t starttime = 0;
#endif

UINT32 nCurrentFrame;			// Framecount for emulated game

UINT32 nFramesEmulated;		// Counters for FPS	display
UINT32 nFramesRendered;		//
bool bForce60Hz = false;
INT32 nBurnFPS = 6000;
INT32 nBurnCPUSpeedAdjust = 0x0100;	// CPU speed adjustment (clock * nBurnCPUSpeedAdjust / 0x0100)

// Burn Draw:
UINT8* pBurnDraw = NULL;	// Pointer to correctly sized bitmap
INT32 nBurnPitch = 0;					// Pitch between each line
INT32 nBurnBpp;						// Bytes per pixel (2, 3, or 4)

INT32 nBurnSoundRate = 0;				// sample rate of sound or zero for no sound
INT32 nBurnSoundLen = 0;				// length in samples per frame
INT16* pBurnSoundOut = NULL;		// pointer to output buffer

INT32 nInterpolation = 1;				// Desired interpolation level for ADPCM/PCM sound
INT32 nFMInterpolation = 0;			// Desired interpolation level for FM sound

UINT8 nBurnLayer = 0xFF;	// Can be used externally to select which layers to show
UINT8 nSpriteEnable = 0xFF;	// Can be used externally to select which layers to show

INT32 nMaxPlayers;

bool bSaveCRoms = 0;

UINT32 *pBurnDrvPalette;

bool BurnCheckMMXSupport()
{
#if defined BUILD_X86_ASM
	UINT32 nSignatureEAX = 0, nSignatureEBX = 0, nSignatureECX = 0, nSignatureEDX = 0;

	CPUID(1, nSignatureEAX, nSignatureEBX, nSignatureECX, nSignatureEDX);

	return (nSignatureEDX >> 23) & 1;						// bit 23 of edx indicates MMX support
#else
	return 0;
#endif
}

extern "C" INT32 BurnLibInit()
{
	BurnLibExit();
	nBurnDrvCount = sizeof(pDriver) / sizeof(pDriver[0]);	// count available drivers

	cmc_4p_Precalc();
	bBurnUseMMX = BurnCheckMMXSupport();

	return 0;
}

extern "C" INT32 BurnLibExit()
{
	nBurnDrvCount = 0;

	return 0;
}

INT32 BurnGetZipName(char** pszName, UINT32 i)
{
	static char szFilename[MAX_PATH];
	char* pszGameName = NULL;

	if (pszName == NULL) {
		return 1;
	}

	if (i == 0) {
		pszGameName = pDriver[nBurnDrvActive]->szShortName;
	} else {
		INT32 nOldBurnDrvSelect = nBurnDrvActive;
		UINT32 j = pDriver[nBurnDrvActive]->szBoardROM ? 1 : 0;

		// Try BIOS/board ROMs first
		if (i == 1 && j == 1) {										// There is a BIOS/board ROM
			pszGameName = pDriver[nBurnDrvActive]->szBoardROM;
		}

		if (pszGameName == NULL) {
			// Go through the list to seek out the parent
			while (j < i) {
				char* pszParent = pDriver[nBurnDrvActive]->szParent;
				pszGameName = NULL;

				if (pszParent == NULL) {							// No parent
					break;
				}

				for (nBurnDrvActive = 0; nBurnDrvActive < nBurnDrvCount; nBurnDrvActive++) {
		            if (strcmp(pszParent, pDriver[nBurnDrvActive]->szShortName) == 0) {	// Found parent
						pszGameName = pDriver[nBurnDrvActive]->szShortName;
						break;
					}
				}

				j++;
			}
		}

		nBurnDrvActive = nOldBurnDrvSelect;
	}

	if (pszGameName == NULL) {
		*pszName = NULL;
		return 1;
	}

	strcpy(szFilename, pszGameName);

	*pszName = szFilename;

	return 0;
}

// ----------------------------------------------------------------------------
// Static functions which forward to each driver's data and functions

INT32 BurnStateMAMEScan(INT32 nAction, INT32* pnMin);
void BurnStateExit();
INT32 BurnStateInit();

// Get the text fields for the driver in TCHARs
extern "C" TCHAR* BurnDrvGetText(UINT32 i)
{
	char* pszStringA = NULL;
	wchar_t* pszStringW = NULL;
	static char* pszCurrentNameA;
	static wchar_t* pszCurrentNameW;

#if defined (_UNICODE)

	static wchar_t szShortNameW[32];
	static wchar_t szDateW[32];
	static wchar_t szFullNameW[256];
	static wchar_t szCommentW[256];
	static wchar_t szManufacturerW[256];
	static wchar_t szSystemW[256];
	static wchar_t szParentW[32];
	static wchar_t szBoardROMW[32];
	static wchar_t szSampleNameW[32];

#else

	static char szShortNameA[32];
	static char szDateA[32];
	static char szFullNameA[256];
	static char szCommentA[256];
	static char szManufacturerA[256];
	static char szSystemA[256];
	static char szParentA[32];
	static char szBoardROMA[32];
	static char szSampleNameA[32];

#endif

	if (!(i & DRV_ASCIIONLY)) {
		switch (i & 0xFF) {
			case DRV_FULLNAME:
				pszStringW = pDriver[nBurnDrvActive]->szFullNameW;
				
				if (i & DRV_NEXTNAME) {
					if (pszCurrentNameW && pDriver[nBurnDrvActive]->szFullNameW) {
						pszCurrentNameW += wcslen(pszCurrentNameW) + 1;
						if (!pszCurrentNameW[0]) {
							return NULL;
						}
						pszStringW = pszCurrentNameW;
					}
				} else {

#if !defined (_UNICODE)

					// Ensure all of the Unicode titles are printable in the current locale
					pszCurrentNameW = pDriver[nBurnDrvActive]->szFullNameW;
					if (pszCurrentNameW && pszCurrentNameW[0]) {
						INT32 nRet;

						do {
							nRet = wcstombs(szFullNameA, pszCurrentNameW, 256);
							pszCurrentNameW += wcslen(pszCurrentNameW) + 1;
						} while	(nRet >= 0 && pszCurrentNameW[0]);

						// If all titles can be printed, we can use the Unicode versions
						if (nRet >= 0) {
							pszStringW = pszCurrentNameW = pDriver[nBurnDrvActive]->szFullNameW;
						}
					}

#else

					pszStringW = pszCurrentNameW = pDriver[nBurnDrvActive]->szFullNameW;

#endif

				}
				break;
			case DRV_COMMENT:
				pszStringW = pDriver[nBurnDrvActive]->szCommentW;
				break;
			case DRV_MANUFACTURER:
				pszStringW = pDriver[nBurnDrvActive]->szManufacturerW;
				break;
			case DRV_SYSTEM:
				pszStringW = pDriver[nBurnDrvActive]->szSystemW;
		}

#if defined (_UNICODE)

		if (pszStringW && pszStringW[0]) {
			return pszStringW;
		}

#else

		switch (i & 0xFF) {
			case DRV_NAME:
				pszStringA = szShortNameA;
				break;
			case DRV_DATE:
				pszStringA = szDateA;
				break;
			case DRV_FULLNAME:
				pszStringA = szFullNameA;
				break;
			case DRV_COMMENT:
				pszStringA = szCommentA;
				break;
			case DRV_MANUFACTURER:
				pszStringA = szManufacturerA;
				break;
			case DRV_SYSTEM:
				pszStringA = szSystemA;
				break;
			case DRV_PARENT:
				pszStringA = szParentA;
				break;
			case DRV_BOARDROM:
				pszStringA = szBoardROMA;
				break;
			case DRV_SAMPLENAME:
				pszStringA = szSampleNameA;
				break;
		}

		if (pszStringW && pszStringA && pszStringW[0]) {
			if (wcstombs(pszStringA, pszStringW, 256) != -1U) {
				return pszStringA;
			}

		}

		pszStringA = NULL;

#endif

	}

	if (i & DRV_UNICODEONLY) {
		return NULL;
	}

	switch (i & 0xFF) {
		case DRV_NAME:
			pszStringA = pDriver[nBurnDrvActive]->szShortName;
			break;
		case DRV_DATE:
			pszStringA = pDriver[nBurnDrvActive]->szDate;
			break;
		case DRV_FULLNAME:
			pszStringA = pDriver[nBurnDrvActive]->szFullNameA;

			if (i & DRV_NEXTNAME) {
				if (!pszCurrentNameW && pDriver[nBurnDrvActive]->szFullNameA) {
					pszCurrentNameA += strlen(pszCurrentNameA) + 1;
					if (!pszCurrentNameA[0]) {
						return NULL;
					}
					pszStringA = pszCurrentNameA;
				}
			} else {
				pszStringA = pszCurrentNameA = pDriver[nBurnDrvActive]->szFullNameA;
				pszCurrentNameW = NULL;
			}
			break;
		case DRV_COMMENT:
			pszStringA = pDriver[nBurnDrvActive]->szCommentA;
			break;
		case DRV_MANUFACTURER:
			pszStringA = pDriver[nBurnDrvActive]->szManufacturerA;
			break;
		case DRV_SYSTEM:
			pszStringA = pDriver[nBurnDrvActive]->szSystemA;
			break;
		case DRV_PARENT:
			pszStringA = pDriver[nBurnDrvActive]->szParent;
			break;
		case DRV_BOARDROM:
			pszStringA = pDriver[nBurnDrvActive]->szBoardROM;
			break;
		case DRV_SAMPLENAME:
			pszStringA = pDriver[nBurnDrvActive]->szSampleName;
	}

#if defined (_UNICODE)

	switch (i & 0xFF) {
		case DRV_NAME:
			pszStringW = szShortNameW;
			break;
		case DRV_DATE:
			pszStringW = szDateW;
			break;
		case DRV_FULLNAME:
			pszStringW = szFullNameW;
			break;
		case DRV_COMMENT:
			pszStringW = szCommentW;
			break;
		case DRV_MANUFACTURER:
			pszStringW = szManufacturerW;
			break;
		case DRV_SYSTEM:
			pszStringW = szSystemW;
			break;
		case DRV_PARENT:
			pszStringW = szParentW;
			break;
		case DRV_BOARDROM:
			pszStringW = szBoardROMW;
			break;
		case DRV_SAMPLENAME:
			pszStringW = szSampleNameW;
			break;
	}

	if (pszStringW && pszStringA && pszStringA[0]) {
		if (mbstowcs(pszStringW, pszStringA, 256) != -1U) {
			return pszStringW;
		}
	}

#else

	if (pszStringA && pszStringA[0]) {
		return pszStringA;
	}

#endif

	return NULL;
}


// Get the ASCII text fields for the driver in ASCII format;
extern "C" char* BurnDrvGetTextA(UINT32 i)
{
	switch (i) {
		case DRV_NAME:
			return pDriver[nBurnDrvActive]->szShortName;
		case DRV_DATE:
			return pDriver[nBurnDrvActive]->szDate;
		case DRV_FULLNAME:
			return pDriver[nBurnDrvActive]->szFullNameA;
		case DRV_COMMENT:
			return pDriver[nBurnDrvActive]->szCommentA;
		case DRV_MANUFACTURER:
			return pDriver[nBurnDrvActive]->szManufacturerA;
		case DRV_SYSTEM:
			return pDriver[nBurnDrvActive]->szSystemA;
		case DRV_PARENT:
			return pDriver[nBurnDrvActive]->szParent;
		case DRV_BOARDROM:
			return pDriver[nBurnDrvActive]->szBoardROM;
		case DRV_SAMPLENAME:
			return pDriver[nBurnDrvActive]->szSampleName;
		default:
			return NULL;
	}
}

#if defined (_UNICODE)
void BurnLocalisationSetName(char *szName, TCHAR *szLongName)
{
	for (UINT32 i = 0; i < nBurnDrvCount; i++) {
		nBurnDrvActive = i;
		if (!strcmp(szName, pDriver[i]->szShortName)) {
			pDriver[i]->szFullNameW = szLongName;
		}
	}
}
#endif

// Get the zip names for the driver
extern "C" INT32 BurnDrvGetZipName(char** pszName, UINT32 i)
{
	if (pDriver[nBurnDrvActive]->GetZipName) {									// Forward to drivers function
		return pDriver[nBurnDrvActive]->GetZipName(pszName, i);
	}

	return BurnGetZipName(pszName, i);											// Forward to general function
}

extern "C" INT32 BurnDrvGetRomInfo(struct BurnRomInfo* pri, UINT32 i)		// Forward to drivers function
{
	return pDriver[nBurnDrvActive]->GetRomInfo(pri, i);
}

extern "C" INT32 BurnDrvGetRomName(char** pszName, UINT32 i, INT32 nAka)		// Forward to drivers function
{
	return pDriver[nBurnDrvActive]->GetRomName(pszName, i, nAka);
}

extern "C" INT32 BurnDrvGetInputInfo(struct BurnInputInfo* pii, UINT32 i)	// Forward to drivers function
{
	return pDriver[nBurnDrvActive]->GetInputInfo(pii, i);
}

extern "C" INT32 BurnDrvGetDIPInfo(struct BurnDIPInfo* pdi, UINT32 i)
{
	if (pDriver[nBurnDrvActive]->GetDIPInfo) {									// Forward to drivers function
		return pDriver[nBurnDrvActive]->GetDIPInfo(pdi, i);
	}

	return 1;																	// Fail automatically
}

extern "C" INT32 BurnDrvGetSampleInfo(struct BurnSampleInfo* pri, UINT32 i)		// Forward to drivers function
{
	return pDriver[nBurnDrvActive]->GetSampleInfo(pri, i);
}

extern "C" INT32 BurnDrvGetSampleName(char** pszName, UINT32 i, INT32 nAka)		// Forward to drivers function
{
	return pDriver[nBurnDrvActive]->GetSampleName(pszName, i, nAka);
}

// Get the screen size
extern "C" INT32 BurnDrvGetVisibleSize(INT32* pnWidth, INT32* pnHeight)
{
	*pnWidth =pDriver[nBurnDrvActive]->nWidth;
	*pnHeight=pDriver[nBurnDrvActive]->nHeight;

	return 0;
}

extern "C" INT32 BurnDrvGetVisibleOffs(INT32* pnLeft, INT32* pnTop)
{
	*pnLeft = 0;
	*pnTop = 0;

	return 0;
}

extern "C" INT32 BurnDrvGetFullSize(INT32* pnWidth, INT32* pnHeight)
{
	if (pDriver[nBurnDrvActive]->Flags & BDF_ORIENTATION_VERTICAL) {
		*pnWidth =pDriver[nBurnDrvActive]->nHeight;
		*pnHeight=pDriver[nBurnDrvActive]->nWidth;
	} else {
		*pnWidth =pDriver[nBurnDrvActive]->nWidth;
		*pnHeight=pDriver[nBurnDrvActive]->nHeight;
	}

	return 0;
}

// Get screen aspect ratio
extern "C" INT32 BurnDrvGetAspect(INT32* pnXAspect, INT32* pnYAspect)
{
	*pnXAspect = pDriver[nBurnDrvActive]->nXAspect;
	*pnYAspect = pDriver[nBurnDrvActive]->nYAspect;

	return 0;
}

extern "C" INT32 BurnDrvSetVisibleSize(INT32 pnWidth, INT32 pnHeight)
{
	if (pDriver[nBurnDrvActive]->Flags & BDF_ORIENTATION_VERTICAL) {
		pDriver[nBurnDrvActive]->nHeight = pnWidth;
		pDriver[nBurnDrvActive]->nWidth = pnHeight;
	} else {
		pDriver[nBurnDrvActive]->nWidth = pnWidth;
		pDriver[nBurnDrvActive]->nHeight = pnHeight;
	}
	
	return 0;
}

extern "C" INT32 BurnDrvSetAspect(INT32 pnXAspect,INT32 pnYAspect)
{
	pDriver[nBurnDrvActive]->nXAspect = pnXAspect;
	pDriver[nBurnDrvActive]->nYAspect = pnYAspect;

	return 0;	
}

// Get the hardware code
extern "C" INT32 BurnDrvGetHardwareCode()
{
	return pDriver[nBurnDrvActive]->Hardware;
}

// Get flags, including BDF_GAME_WORKING flag
extern "C" INT32 BurnDrvGetFlags()
{
	return pDriver[nBurnDrvActive]->Flags;
}

// Return BDF_WORKING flag
extern "C" bool BurnDrvIsWorking()
{
	return pDriver[nBurnDrvActive]->Flags & BDF_GAME_WORKING;
}

// Return max. number of players
extern "C" INT32 BurnDrvGetMaxPlayers()
{
	return pDriver[nBurnDrvActive]->Players;
}

// Return genre flags
extern "C" INT32 BurnDrvGetGenreFlags()
{
	return pDriver[nBurnDrvActive]->Genre;
}

// Return family flags
extern "C" INT32 BurnDrvGetFamilyFlags()
{
	return pDriver[nBurnDrvActive]->Family;
}

// Init game emulation (loading any needed roms)
extern "C" INT32 BurnDrvInit()
{
	INT32 nReturnValue;

	if (nBurnDrvActive >= nBurnDrvCount) {
		return 1;
	}

#if defined (FBA_DEBUG)
	{
		TCHAR szText[1024] = _T("");
		TCHAR* pszPosition = szText;
		TCHAR* pszName = BurnDrvGetText(DRV_FULLNAME);
		INT32 nName = 1;

		while ((pszName = BurnDrvGetText(DRV_NEXTNAME | DRV_FULLNAME)) != NULL) {
			nName++;
		}

		// Print the title

		bprintf(PRINT_IMPORTANT, _T("*** Starting emulation of %s - %s.\n"), BurnDrvGetText(DRV_NAME), BurnDrvGetText(DRV_FULLNAME));

		// Then print the alternative titles

		if (nName > 1) {
			bprintf(PRINT_IMPORTANT, _T("    Alternative %s "), (nName > 2) ? _T("titles are") : _T("title is"));
			pszName = BurnDrvGetText(DRV_FULLNAME);
			nName = 1;
			while ((pszName = BurnDrvGetText(DRV_NEXTNAME | DRV_FULLNAME)) != NULL) {
				if (pszPosition + _tcslen(pszName) - 1022 > szText) {
					break;
				}
				if (nName > 1) {
					bprintf(PRINT_IMPORTANT, _T(SEPERATOR_1));
				}
				bprintf(PRINT_IMPORTANT, _T("%s"), pszName);
				nName++;
			}
			bprintf(PRINT_IMPORTANT, _T(".\n"));
		}
	}
#endif

	BurnSetRefreshRate(60.0);

	CheatInit();
	HiscoreInit();
	BurnStateInit();	
	BurnInitMemoryManager();

	nReturnValue = pDriver[nBurnDrvActive]->Init();	// Forward to drivers function

	nMaxPlayers = pDriver[nBurnDrvActive]->Players;
	
#if defined (FBA_DEBUG)
	if (!nReturnValue) {
		starttime = clock();
		nFramesEmulated = 0;
		nFramesRendered = 0;
		nCurrentFrame = 0;
	} else {
		starttime = 0;
	}
#endif

	return nReturnValue;
}

// Exit game emulation
extern "C" INT32 BurnDrvExit()
{
#if defined (FBA_DEBUG)
	if (starttime) {
		clock_t endtime;
		clock_t nElapsedSecs;

		endtime = clock();
		nElapsedSecs = (endtime - starttime);
		bprintf(PRINT_IMPORTANT, _T(" ** Emulation ended (running for %.2f seconds).\n"), (float)nElapsedSecs / CLOCKS_PER_SEC);
		bprintf(PRINT_IMPORTANT, _T("    %.2f%% of frames rendered (%d out of a total %d).\n"), (float)nFramesRendered / nFramesEmulated * 100, nFramesRendered, nFramesEmulated);
		bprintf(PRINT_IMPORTANT, _T("    %.2f frames per second (average).\n"), (float)nFramesRendered / nFramesEmulated * nBurnFPS / 100);
		bprintf(PRINT_NORMAL, _T("\n"));
	}
#endif

	CheatExit();
	CheatSearchExit();
	HiscoreExit();
	BurnStateExit();
	
	nBurnCPUSpeedAdjust = 0x0100;
	
	pBurnDrvPalette = NULL;	
	
	INT32 nRet = pDriver[nBurnDrvActive]->Exit();			// Forward to drivers function
	
	BurnExitMemoryManager();
#if defined FBA_DEBUG
	DebugTrackerExit();
#endif

	return nRet;
}

INT32 (__cdecl* BurnExtCartridgeSetupCallback)(BurnCartrigeCommand nCommand) = NULL;

INT32 BurnDrvCartridgeSetup(BurnCartrigeCommand nCommand)
{
	if (nBurnDrvActive >= nBurnDrvCount || BurnExtCartridgeSetupCallback == NULL) {
		return 1;
	}

	if (nCommand == CART_EXIT) {
		return pDriver[nBurnDrvActive]->Exit();
	}

	if (nCommand != CART_INIT_END && nCommand != CART_INIT_START) {
		return 1;
	}

	BurnExtCartridgeSetupCallback(CART_INIT_END);

#if defined FBA_DEBUG
		bprintf(PRINT_NORMAL, _T("  * Loading"));
#endif

	if (BurnExtCartridgeSetupCallback(CART_INIT_START)) {
		return 1;
	}

	if (nCommand == CART_INIT_START) {
		return pDriver[nBurnDrvActive]->Init();
	}

	return 0;
}

// Do one frame of game emulation
extern "C" INT32 BurnDrvFrame()
{
	CheatApply();									// Apply cheats (if any)
	HiscoreApply();
	return pDriver[nBurnDrvActive]->Frame();		// Forward to drivers function
}

// Force redraw of the screen
extern "C" INT32 BurnDrvRedraw()
{
	if (pDriver[nBurnDrvActive]->Redraw) {
		return pDriver[nBurnDrvActive]->Redraw();	// Forward to drivers function
	}

	return 1;										// No funtion provide, so simply return
}

// Refresh Palette
extern "C" INT32 BurnRecalcPal()
{
	if (nBurnDrvActive < nBurnDrvCount) {
		UINT8* pr = pDriver[nBurnDrvActive]->pRecalcPal;
		if (pr == NULL) return 1;
		*pr = 1;									// Signal for the driver to refresh it's palette
	}

	return 0;
}

extern "C" INT32 BurnDrvGetPaletteEntries()
{
	return pDriver[nBurnDrvActive]->nPaletteEntries;
}

// ----------------------------------------------------------------------------

INT32 (__cdecl *BurnExtProgressRangeCallback)(double fProgressRange) = NULL;
INT32 (__cdecl *BurnExtProgressUpdateCallback)(double fProgress, const TCHAR* pszText, bool bAbs) = NULL;

INT32 BurnSetProgressRange(double fProgressRange)
{
	if (BurnExtProgressRangeCallback) {
		return BurnExtProgressRangeCallback(fProgressRange);
	}

	return 1;
}

INT32 BurnUpdateProgress(double fProgress, const TCHAR* pszText, bool bAbs)
{
	if (BurnExtProgressUpdateCallback) {
		return BurnExtProgressUpdateCallback(fProgress, pszText, bAbs);
	}

	return 1;
}

// ----------------------------------------------------------------------------

INT32 BurnSetRefreshRate(double dFrameRate)
{
	if (!bForce60Hz) {
		nBurnFPS = (INT32)(100.0 * dFrameRate);
	}

	return 0;
}

inline static INT32 BurnClearSize(INT32 w, INT32 h)
{
	UINT8 *pl;
	INT32 y;

	w *= nBurnBpp;

	// clear the screen to zero
	for (pl = pBurnDraw, y = 0; y < h; pl += nBurnPitch, y++) {
		memset(pl, 0x00, w);
	}

	return 0;
}

INT32 BurnClearScreen()
{
	struct BurnDriver* pbd = pDriver[nBurnDrvActive];

	if (pbd->Flags & BDF_ORIENTATION_VERTICAL) {
		BurnClearSize(pbd->nHeight, pbd->nWidth);
	} else {
		BurnClearSize(pbd->nWidth, pbd->nHeight);
	}

	return 0;
}

// Byteswaps an area of memory
INT32 BurnByteswap(UINT8* pMem, INT32 nLen)
{
	nLen >>= 1;
	for (INT32 i = 0; i < nLen; i++, pMem += 2) {
		UINT8 t = pMem[0];
		pMem[0] = pMem[1];
		pMem[1] = t;
	}

	return 0;
}

// Application-defined rom loading function:
INT32 (__cdecl *BurnExtLoadRom)(UINT8 *Dest, INT32 *pnWrote, INT32 i) = NULL;

// Application-defined colour conversion function
static UINT32 __cdecl BurnHighColFiller(INT32, INT32, INT32, INT32) { return (UINT32)(~0); }
UINT32 (__cdecl *BurnHighCol) (INT32 r, INT32 g, INT32 b, INT32 i) = BurnHighColFiller;

// ----------------------------------------------------------------------------
// Colour-depth independant image transfer

UINT16* pTransDraw = NULL;

static INT32 nTransWidth, nTransHeight;

void BurnTransferClear()
{
#if defined FBA_DEBUG
	if (!Debug_BurnTransferInitted) bprintf(PRINT_ERROR, _T("BurnTransferClear called without init\n"));
#endif

	memset((void*)pTransDraw, 0, nTransWidth * nTransHeight * sizeof(UINT16));
}

INT32 BurnTransferCopy(UINT32* pPalette)
{
#if defined FBA_DEBUG
	if (!Debug_BurnTransferInitted) bprintf(PRINT_ERROR, _T("BurnTransferCopy called without init\n"));
#endif

	UINT16* pSrc = pTransDraw;
	UINT8* pDest = pBurnDraw;
	
	pBurnDrvPalette = pPalette;

	switch (nBurnBpp) {
		case 2: {
			for (INT32 y = 0; y < nTransHeight; y++, pSrc += nTransWidth, pDest += nBurnPitch) {
				for (INT32 x = 0; x < nTransWidth; x ++) {
					((UINT16*)pDest)[x] = pPalette[pSrc[x]];
				}
			}
			break;
		}
		case 3: {
			for (INT32 y = 0; y < nTransHeight; y++, pSrc += nTransWidth, pDest += nBurnPitch) {
				for (INT32 x = 0; x < nTransWidth; x++) {
					UINT32 c = pPalette[pSrc[x]];
					*(pDest + (x * 3) + 0) = c & 0xFF;
					*(pDest + (x * 3) + 1) = (c >> 8) & 0xFF;
					*(pDest + (x * 3) + 2) = c >> 16;

				}
			}
			break;
		}
		case 4: {
			for (INT32 y = 0; y < nTransHeight; y++, pSrc += nTransWidth, pDest += nBurnPitch) {
				for (INT32 x = 0; x < nTransWidth; x++) {
					((UINT32*)pDest)[x] = pPalette[pSrc[x]];
				}
			}
			break;
		}
	}

	return 0;
}

void BurnTransferExit()
{
#if defined FBA_DEBUG
	if (!Debug_BurnTransferInitted) bprintf(PRINT_ERROR, _T("BurnTransferClear called without init\n"));
#endif

	if (pTransDraw) {
		free(pTransDraw);
		pTransDraw = NULL;
	}
	
	Debug_BurnTransferInitted = 0;
}

INT32 BurnTransferInit()
{
	Debug_BurnTransferInitted = 1;
	
	if (BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) {
		BurnDrvGetVisibleSize(&nTransHeight, &nTransWidth);
	} else {
		BurnDrvGetVisibleSize(&nTransWidth, &nTransHeight);
	}

	pTransDraw = (UINT16*)malloc(nTransWidth * nTransHeight * sizeof(UINT16));
	if (pTransDraw == NULL) {
		return 1;
	}

	BurnTransferClear();

	return 0;
}



// ----------------------------------------------------------------------------
// Savestate support

// Application-defined callback for processing the area
static INT32 __cdecl DefAcb (struct BurnArea* /* pba */) { return 1; }
INT32 (__cdecl *BurnAcb) (struct BurnArea* pba) = DefAcb;

// Scan driver data
INT32 BurnAreaScan(INT32 nAction, INT32* pnMin)
{
	INT32 nRet = 0;

	// Handle any MAME-style variables
	if (nAction & ACB_DRIVER_DATA) {
		nRet = BurnStateMAMEScan(nAction, pnMin);
	}

	// Forward to the driver
	if (pDriver[nBurnDrvActive]->AreaScan) {
		nRet |= pDriver[nBurnDrvActive]->AreaScan(nAction, pnMin);
	}

	return nRet;
}

// ----------------------------------------------------------------------------
// Wrappers for MAME-specific function calls

#include "driver.h"

// ----------------------------------------------------------------------------
// Wrapper for MAME logerror calls

#if defined (FBA_DEBUG) && defined (MAME_USE_LOGERROR)
void logerror(char* szFormat, ...)
{
	static char szLogMessage[1024];

	va_list vaFormat;
	va_start(vaFormat, szFormat);

	_vsnprintf(szLogMessage, 1024, szFormat, vaFormat);

	va_end(vaFormat);

	bprintf(PRINT_ERROR, _T("%hs"), szLogMessage);

	return;
}
#endif

// ----------------------------------------------------------------------------
// Wrapper for MAME state_save_register_* calls

struct BurnStateEntry { BurnStateEntry* pNext; BurnStateEntry* pPrev; char szName[256]; void* pValue; UINT32 nSize; };

static BurnStateEntry* pStateEntryAnchor = NULL;
typedef void (*BurnPostloadFunction)();
static BurnPostloadFunction BurnPostload[8];

static void BurnStateRegister(const char* module, INT32 instance, const char* name, void* val, UINT32 size)
{
	// Allocate new node
	BurnStateEntry* pNewEntry = (BurnStateEntry*)malloc(sizeof(BurnStateEntry));
	if (pNewEntry == NULL) {
		return;
	}

	memset(pNewEntry, 0, sizeof(BurnStateEntry));

	// Link the new node
	pNewEntry->pNext = pStateEntryAnchor;
	if (pStateEntryAnchor) {
		pStateEntryAnchor->pPrev = pNewEntry;
	}
	pStateEntryAnchor = pNewEntry;

	sprintf(pNewEntry->szName, "%s:%s %i", module, name, instance);

	pNewEntry->pValue = val;
	pNewEntry->nSize = size;
}

void BurnStateExit()
{
	if (pStateEntryAnchor) {
		BurnStateEntry* pCurrentEntry = pStateEntryAnchor;
		BurnStateEntry* pNextEntry;

		do {
			pNextEntry = pCurrentEntry->pNext;
			if (pCurrentEntry) {
				free(pCurrentEntry);
			}
		} while ((pCurrentEntry = pNextEntry) != 0);
	}

	pStateEntryAnchor = NULL;

	for (INT32 i = 0; i < 8; i++) {
		BurnPostload[i] = NULL;
	}
}

INT32 BurnStateInit()
{
	BurnStateExit();

	return 0;
}

INT32 BurnStateMAMEScan(INT32 nAction, INT32* pnMin)
{
	if (nAction & ACB_VOLATILE) {

		if (pnMin && *pnMin < 0x029418) {						// Return minimum compatible version
			*pnMin = 0x029418;
		}

		if (pStateEntryAnchor) {
			struct BurnArea ba;
			BurnStateEntry* pCurrentEntry = pStateEntryAnchor;

			do {
			   	ba.Data		= pCurrentEntry->pValue;
				ba.nLen		= pCurrentEntry->nSize;
				ba.nAddress = 0;
				ba.szName	= pCurrentEntry->szName;
				BurnAcb(&ba);

			} while ((pCurrentEntry = pCurrentEntry->pNext) != 0);
		}

		if (nAction & ACB_WRITE) {
			for (INT32 i = 0; i < 8; i++) {
				if (BurnPostload[i]) {
					BurnPostload[i]();
				}
			}
		}
	}

	return 0;
}

// wrapper functions

extern "C" void state_save_register_func_postload(void (*pFunction)())
{
	for (INT32 i = 0; i < 8; i++) {
		if (BurnPostload[i] == NULL) {
			BurnPostload[i] = pFunction;
			break;
		}
	}
}

extern "C" void state_save_register_INT8(const char* module, INT32 instance, const char* name, INT8* val, UINT32 size)
{
	BurnStateRegister(module, instance, name, (void*)val, size * sizeof(INT8));
}

extern "C" void state_save_register_UINT8(const char* module, INT32 instance, const char* name, UINT8* val, UINT32 size)
{
	BurnStateRegister(module, instance, name, (void*)val, size * sizeof(UINT8));
}

extern "C" void state_save_register_INT16(const char* module, INT32 instance, const char* name, INT16* val, UINT32 size)
{
	BurnStateRegister(module, instance, name, (void*)val, size * sizeof(INT16));
}

extern "C" void state_save_register_UINT16(const char* module, INT32 instance, const char* name, UINT16* val, UINT32 size)
{
	BurnStateRegister(module, instance, name, (void*)val, size * sizeof(UINT16));
}

extern "C" void state_save_register_INT32(const char* module, INT32 instance, const char* name, INT32* val, UINT32 size)
{
	BurnStateRegister(module, instance, name, (void*)val, size * sizeof(INT32));
}

extern "C" void state_save_register_UINT32(const char* module, INT32 instance, const char* name, UINT32* val, UINT32 size)
{
	BurnStateRegister(module, instance, name, (void*)val, size * sizeof(UINT32));
}

extern "C" void state_save_register_int(const char* module, INT32 instance, const char* name, INT32* val)
{
	BurnStateRegister(module, instance, name, (void*)val, sizeof(INT32));
}

extern "C" void state_save_register_float(const char* module, INT32 instance, const char* name, float* val, UINT32 size)
{
	BurnStateRegister(module, instance, name, (void*)val, size * sizeof(float));
}

extern "C" void state_save_register_double(const char* module, INT32 instance, const char* name, double* val, UINT32 size)
{
	BurnStateRegister(module, instance, name, (void*)val, size * sizeof(double));
}

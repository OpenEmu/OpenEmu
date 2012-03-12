// Burner Zip module
#include "burner.h"

int nBzipError = 0;												// non-zero if there is a problem with the opened romset

static TCHAR* szBzipName[BZIP_MAX] = { NULL, };					// Zip files to search through

struct RomFind { int nState; int nZip; int nPos; };				// State is non-zero if found. 1 = found totally okay.
static struct RomFind* RomFind = NULL;
static int nRomCount = 0; static int nTotalSize = 0;
static struct ZipEntry* List = NULL; static int nListCount = 0;	// List of entries for current zip file
static int nCurrentZip = -1;									// Zip which is currently open

// ----------------------------------------------------------------------------

static void BzipListFree()
{
	if (List) {
		for (int i = 0; i < nListCount; i++) {
			if (List[i].szName) {
				free(List[i].szName);
				List[i].szName = NULL;
			}
		}
		free(List);
		List = NULL;
	}

	nListCount = 0;
}

static char* GetFilenameA(char* szFull)
{
	int nLen = strlen(szFull);

	if (nLen <= 0) {
		return szFull;
	}
	for (int i = nLen - 1; i >= 0; i--) {
		if (szFull[i] == '\\' || szFull[i] == '/') {
			return szFull + i + 1;
		}
	}

	return szFull;
}

static TCHAR* GetFilenameW(TCHAR* szFull)
{
	int nLen = _tcslen(szFull);

	if (nLen <= 0) {
		return szFull;
	}
	for (int i = nLen - 1; i >= 0; i--) {
		if (szFull[i] == _T('\\') || szFull[i] == _T('/')) {
			return szFull + i + 1;
		}
	}

	return szFull;
}

static int FindRomByName(TCHAR* szName)
{
	struct ZipEntry* pl;
	int i;

	// Find the rom named szName in the List
	for (i = 0, pl = List; i < nListCount; i++, pl++) {
		TCHAR szCurrentName[MAX_PATH];
		if (_tcsicmp(szName, GetFilenameW(ANSIToTCHAR(pl->szName, szCurrentName, MAX_PATH))) == 0) {
			return i;
		}
	}
	return -1;													// couldn't find the rom
}

static int FindRomByCrc(unsigned int nCrc)
{
	struct ZipEntry* pl;
	int i;

	// Find the rom named szName in the List
	for (i = 0, pl = List; i< nListCount; i++, pl++)	{
		if (nCrc == pl->nCrc) {
			return i;
		}
	}

	return -1;													// couldn't find the rom
}

// Find rom number i from the pBzipDriver game
static int FindRom(int i)
{
	struct BurnRomInfo ri;
	int nRet;

	memset(&ri, 0, sizeof(ri));

	nRet = BurnDrvGetRomInfo(&ri, i);
	if (nRet != 0) {											// Failure: no such rom
		return -2;
	}

	if (ri.nCrc) {												// Search by crc first
		nRet = FindRomByCrc(ri.nCrc);
		if (nRet >= 0) {
			return nRet;
		}
	}

	for (int nAka = 0; nAka < 0x10000; nAka++) {				// Failing that, search for possible names
		char *szPossibleName = NULL;

		nRet = BurnDrvGetRomName(&szPossibleName, i, nAka);
		if (nRet) {												// No more rom names
			break;
		}
		nRet = FindRomByName(ANSIToTCHAR(szPossibleName, NULL, 0));
		if (nRet >= 0) {
			return nRet;
		}
	}

	return -1;													// Couldn't find the rom
}

static int RomDescribe(struct BurnRomInfo* pri)
{
	if (nBzipError == 0) {
		nBzipError |= 0x8000;

		FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_INVALID));
	}

	FBAPopupAddText(PUF_TEXT_DEFAULT, _T(" ") _T(SEPERATOR_1));
	if (pri->nType & BRF_ESS) {
		FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_DET_ESS));
	}
	if (pri->nType & BRF_BIOS) {
		FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_DET_BIOS));
	}
	if (pri->nType & BRF_PRG) {
		FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_DET_PRG));
	}
	if (pri->nType & BRF_GRA) {
		FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_DET_GRA));
	}
	if (pri->nType & BRF_SND) {
		FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_DET_SND));
	}
	FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_DET_ROM));

	return 0;
}

static int CheckRomsBoot()
{
	int nRet = 0;

	for (int i = 0; i < nRomCount; i++) {
		struct BurnRomInfo ri;
		int nState;

		memset(&ri, 0, sizeof(ri));
		BurnDrvGetRomInfo(&ri, i);			// Find information about the wanted rom
		nState = RomFind[i].nState;			// Get the state of the rom in the zip file

		if (nState != 1 && ri.nType && ri.nCrc) {
			if (!(ri.nType & BRF_OPT) && !(ri.nType & BRF_NODUMP)) {
				return 1;
			}
			nRet = 2;
		}
	}

	return nRet;
}

static int GetBZipError(int nState)
{
	switch (nState) {
		case 1:								// OK
			return 0x0000;
		case 0:								// Not present
			return 0x0001;
		case 3:								// Incomplete
			return 0x0001;
		default:							// CRC wrong or too large
			return 0x0100;
	}

	return 0x0100;
}

// Check the roms to see if they code, graphics etc are complete
static int CheckRoms()
{
	for (int i = 0; i < nRomCount; i++) {
		struct BurnRomInfo ri;

		memset(&ri, 0, sizeof(ri));
		BurnDrvGetRomInfo(&ri, i);							// Find information about the wanted rom
		if ( /*ri.nCrc &&*/ (ri.nType & BRF_OPT) == 0 && (ri.nType & BRF_NODUMP) == 0) {
			int nState = RomFind[i].nState;					// Get the state of the rom in the zip file
			int nError = GetBZipError(nState);

			if (nState == 0 && ri.nType) {					// (A type of 0 means empty slot - no rom)
				char* szName = "Unknown";
				RomDescribe(&ri);
				BurnDrvGetRomName(&szName, i, 0);
				FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_NOTFOUND), szName);
			}

			if (nError == 0) {
				nBzipError |= 0x2000;
			}

			if (ri.nType & BRF_ESS) {						// essential rom - without it the game may not run at all
				nBzipError |= nError << 0;
			}
			if (ri.nType & BRF_PRG) {						// rom which contains program information
				nBzipError |= nError << 1;
			}
			if (ri.nType & BRF_GRA) {						// rom which contains graphics information
				nBzipError |= nError << 2;
			}
			if (ri.nType & BRF_SND) {						// rom which contains sound information
				nBzipError |= nError << 3;
			}
		}
	}

	if (nBzipError & 0x0F0F) {
		nBzipError |= 0x4000;
	}

	return 0;
}

// ----------------------------------------------------------------------------

static int __cdecl BzipBurnLoadRom(unsigned char* Dest, int* pnWrote, int i)
{
#if defined (BUILD_WIN32)
	MSG Msg;
#endif

	struct BurnRomInfo ri;
	int nWantZip = 0;
	TCHAR szText[128];
	char* pszRomName = NULL;
	int nRet = 0;

	if (i < 0 || i >= nRomCount) {
		return 1;
	}

	ri.nLen = 0;
	BurnDrvGetRomInfo(&ri, i);								// Get info

	// show what we're doing
	BurnDrvGetRomName(&pszRomName, i, 0);
	if (pszRomName == NULL) {
		TCHAR szTempName[100];
		_stprintf(szTempName, _T("%s"), FBALoadStringEx(hAppInst, IDS_ERR_UNKNOWN, true));
		sprintf(pszRomName, "%s", TCHARToANSI(szTempName, NULL, 0));
	}
	
	TCHAR szTempLoading[100];
	_stprintf(szTempLoading, _T("%s"), FBALoadStringEx(hAppInst, IDS_PROGRESS_LOADING_ONLY, true));
	_stprintf(szText, _T("%s"), szTempLoading);
	
	if (ri.nType & (BRF_PRG | BRF_GRA | BRF_SND | BRF_BIOS)) {
		if (ri.nType & BRF_BIOS) {
			TCHAR szTempBios[100];
			_stprintf(szTempBios, _T("%s"), FBALoadStringEx(hAppInst, IDS_ERR_LOAD_DET_BIOS, true));
			_stprintf(szText + _tcslen(szText), _T(" %s"), szTempBios);
		}
		if (ri.nType & BRF_PRG) {
			TCHAR szTempPrg[100];
			_stprintf(szTempPrg, _T("%s"), FBALoadStringEx(hAppInst, IDS_ERR_LOAD_DET_PRG, true));
			_stprintf(szText + _tcslen(szText), _T(" %s"), szTempPrg);
		}
		if (ri.nType & BRF_GRA) {
			TCHAR szTempGra[100];
			_stprintf(szTempGra, _T("%s"), FBALoadStringEx(hAppInst, IDS_ERR_LOAD_DET_GRA, true));
			_stprintf (szText + _tcslen(szText), _T(" %s"), szTempGra);
		}
		if (ri.nType & BRF_SND) {
			TCHAR szTempSnd[100];
			_stprintf(szTempSnd, _T("%s"), FBALoadStringEx(hAppInst, IDS_ERR_LOAD_DET_SND, true));
			_stprintf (szText + _tcslen(szText), _T(" %s"), szTempSnd);
		}
		_stprintf(szText + _tcslen(szText), _T("(%hs)..."), pszRomName);
	} else {
		_stprintf(szText + _tcslen(szText), _T(" %hs..."), pszRomName);
	}
	ProgressUpdateBurner(ri.nLen ? 1.0 / ((double)nTotalSize / ri.nLen) : 0, szText, 0);

#if defined (BUILD_WIN32)
	// Check for messages:
	while (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE)) {
		DispatchMessage(&Msg);
	}
#endif

	if (RomFind[i].nState == 0) {							// Rom not found in zip at all
		return 1;
	}

	nWantZip = RomFind[i].nZip;								// Which zip file it is in
	if (nCurrentZip != nWantZip) {							// If we haven't got the right zip file currently open
		ZipClose();
		nCurrentZip = -1;
		if (ZipOpen(TCHARToANSI(szBzipName[nWantZip], NULL, 0))) {
			return 1;
		}
		nCurrentZip = nWantZip;
	}

	// Read in file and return how many bytes we read
	if (ZipLoadFile(Dest, ri.nLen, pnWrote, RomFind[i].nPos)) {

		// Error loading from the zip file
		FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(nRet == 2 ? IDS_ERR_LOAD_DISK_CRC : IDS_ERR_LOAD_DISK), pszRomName, GetFilenameW(szBzipName[nCurrentZip]));
		FBAPopupDisplay(PUF_TYPE_WARNING);

		return 1;
	}

	return 0;
}

// ----------------------------------------------------------------------------

int BzipStatus()
{
	if (!(nBzipError & 0x0F0F)) {
		return BZIP_STATUS_OK;
	}
	if (nBzipError & 1) {
		return BZIP_STATUS_ERROR;
	}

	return BZIP_STATUS_BADDATA;
}

int BzipOpen(bool bootApp)
{
	int nMemLen;														// Zip name number

	nTotalSize = 0;
	nBzipError = 0;

	if (szBzipName == NULL) {
		return 1;
	}

	BzipClose();														// Make sure nothing is open

	// Count the number of roms needed
	for (nRomCount = 0; ; nRomCount++) {
		if (BurnDrvGetRomInfo(NULL, nRomCount)) {
			break;
		}
	}
	if (nRomCount <= 0) {
		return 1;
	}

	// Create an array for holding lookups for each rom -> zip entries
	nMemLen = nRomCount * sizeof(struct RomFind);
	RomFind = (struct RomFind*)malloc(nMemLen);
	if (RomFind == NULL) {
		return 1;
	}
	memset(RomFind, 0, nMemLen);

	for (int y = 0; y < BZIP_MAX; y++) {
		if (szBzipName[y]) {
			free(szBzipName[y]);
			szBzipName[y] = NULL;
		}
	}

	// Locate each zip file
	for (int y = 0, z = 0; y < BZIP_MAX && z < BZIP_MAX; y++) {
		char* szName = NULL;
		bool bFound = false;

		if (BurnDrvGetZipName(&szName, y)) {
			break;
		}

		for (int d = 0; d < DIRS_MAX; d++) {
			TCHAR szFullName[MAX_PATH];

			_stprintf(szFullName, _T("%s%hs"), szAppRomPaths[d], szName);

			if (ZipOpen(TCHARToANSI(szFullName, NULL, 0)) == 0) {		// Open the rom zip file
				ZipClose();

				bFound = true;

				szBzipName[z] = (TCHAR*)malloc(MAX_PATH * sizeof(TCHAR));
				_tcscpy(szBzipName[z], szFullName);

				if (!bootApp) {
					FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_FOUND), szName, szBzipName[z]);
				}

				z++;
				if (z >= BZIP_MAX) break;

				// Look further in the last ten paths specified, so you can put files with ROMs
				// used only by FB Alpha there without causing problems with dat files
				if (d < DIRS_MAX - 11) {
					d = DIRS_MAX - 11;
				} else {
					if (d >= DIRS_MAX - 1) {
						break;
					}
				}
			}
		}

		if (!bootApp && !bFound) {
			FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_NOTFOUND), szName);
		}
	}

	if (!bootApp) {
		FBAPopupAddText(PUF_TEXT_DEFAULT, _T("\n"));
	}

	// Locate the ROM data in the zip files
	for (int z = 0; z < BZIP_MAX; z++) {

		if (szBzipName[z] == NULL) {
			continue;
		}

		if (ZipOpen(TCHARToANSI(szBzipName[z], NULL, 0)) == 0) {		// Open the rom zip file
			nCurrentZip = z;

			ZipGetList(&List, &nListCount);								// Get the list of entries

			for (int i = 0; i < nRomCount; i++) {
				struct BurnRomInfo ri;
				int nFind;

				if (RomFind[i].nState == 1) {							// Already found this and it's okay
					continue;
				}

				memset(&ri, 0, sizeof(ri));

				nFind = FindRom(i);

				if (nFind < 0) {										// Couldn't find this rom at all
					continue;
				}

				RomFind[i].nZip = z;									// Remember which zip file it is in
				RomFind[i].nPos = nFind;
				RomFind[i].nState = 1;									// Set to found okay

				BurnDrvGetRomInfo(&ri, i);								// Get info about the rom

				if ((ri.nType & BRF_OPT) == 0 && (ri.nType & BRF_NODUMP) == 0)	{
					nTotalSize += ri.nLen;
				}

				if (List[nFind].nLen == ri.nLen) {
					if (ri.nCrc) {										// If we know the CRC
						if (List[nFind].nCrc != ri.nCrc) {				// Length okay, but CRC wrong
							RomFind[i].nState = 2;
						}
					}
				} else {
					if (List[nFind].nLen < ri.nLen) {
						RomFind[i].nState = 3;							// Too small
					} else {
						RomFind[i].nState = 4;							// Too big
					}
				}

				if (!bootApp) {
					if (RomFind[i].nState != 1) {
						RomDescribe(&ri);

						if (RomFind[i].nState == 2) {
							FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_CRC), GetFilenameA(List[nFind].szName), List[nFind].nCrc, ri.nCrc);
						}
						if (RomFind[i].nState == 3) {
							FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_SMALL), GetFilenameA(List[nFind].szName), List[nFind].nLen >> 10, ri.nLen >> 10);
						}
						if (RomFind[i].nState == 4) {
							FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_LARGE), GetFilenameA(List[nFind].szName), List[nFind].nLen >> 10, ri.nLen >> 10);
						}
					}
				}
			}

			BzipListFree();
		}

		ZipClose();														// Close the last zip file if open
		nCurrentZip = -1;
	}

	if (!bootApp) {
		// Check the roms to see if the code, graphics etc are complete
		CheckRoms();

		if (nBzipError & 0x2000) {
			if (!(nBzipError & 0x0F0F)) {
				FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_OK));
			} else {
				FBAPopupAddText(PUF_TEXT_DEFAULT, _T("\n"));
				FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_PROBLEM));
			}

			if (nBzipError & 0x0101) {
				FBAPopupAddText(PUF_TEXT_DEFAULT, _T("\n ") _T(SEPERATOR_1));
				if (nBzipError & 0x0001) {
					FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_ESS_MISS));
				} else {
					FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_ESS_BAD));
				}
			}
			if (nBzipError & 0x0202) {
				FBAPopupAddText(PUF_TEXT_DEFAULT, _T("\n ") _T(SEPERATOR_1));
				FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_DET_PRG));
				if (nBzipError & 0x0002) {
					FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_DATA_MISS));
				} else {
					FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_DATA_BAD));
				}
			}
			if (nBzipError & 0x0404) {
				FBAPopupAddText(PUF_TEXT_DEFAULT, _T("\n ") _T(SEPERATOR_1));
				FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_DET_GRA));
				if (nBzipError & 0x0004) {
					FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_DATA_MISS));
				} else {
					FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_DATA_BAD));
				}
			}
			if (nBzipError & 0x0808) {
				FBAPopupAddText(PUF_TEXT_DEFAULT, _T("\n ") _T(SEPERATOR_1));
				FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_DET_SND));
				if (nBzipError & 0x0008) {
					FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_DATA_MISS));
				} else {
					FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_DATA_BAD));
				}
			}

			// Catch non-categorised ROMs
			if ((nBzipError & 0x0F0F) == 0) {
				if (nBzipError & 0x0010) {
					FBAPopupAddText(PUF_TEXT_DEFAULT, _T("\n ") _T(SEPERATOR_1));
					if (nBzipError & 0x1000) {
						FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_DATA_MISS));
					} else {
						FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_DATA_BAD));
					}
				}
			}
		} else {
			FBAPopupAddText(PUF_TEXT_DEFAULT, _T("\n"));
			FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_NODATA));
		}

		BurnExtLoadRom = BzipBurnLoadRom;								// Okay to call our function to load each rom

	} else {
		return CheckRomsBoot();
	}

	return 0;
}

int BzipClose()
{
	ZipClose();
	nCurrentZip = -1;													// Close the last zip file if open

	BurnExtLoadRom = NULL;												// Can't call our function to load each rom anymore
	nBzipError = 0;														// reset romset errors

	if (RomFind) {
		free(RomFind);
		RomFind = NULL;
	}
	nRomCount = 0;

	for (int z = 0; z < BZIP_MAX; z++) {
		if (szBzipName[z]) {
			free(szBzipName[z]);
			szBzipName[z] = NULL;
		}
	}

	return 0;
}

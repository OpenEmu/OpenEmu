#include "burner.h"

static bool SkipComma(TCHAR** s)
{
	while (**s && **s != _T(',')) {
		(*s)++;
	}

	if (**s == _T(',')) {
		(*s)++;
	}

	if (**s) {
		return true;
	}

	return false;
}

static void CheatError(TCHAR* pszFilename, INT32 nLineNumber, CheatInfo* pCheat, TCHAR* pszInfo, TCHAR* pszLine)
{
#if defined (BUILD_WIN32)
	FBAPopupAddText(PUF_TEXT_NO_TRANSLATE, _T("Cheat file %s is malformed.\nPlease remove or repair the file.\n\n"), pszFilename);
	if (pCheat) {
		FBAPopupAddText(PUF_TEXT_NO_TRANSLATE, _T("Parse error at line %i, in cheat \"%s\".\n"), nLineNumber, pCheat->szCheatName);
	} else {
		FBAPopupAddText(PUF_TEXT_NO_TRANSLATE, _T("Parse error at line %i.\n"), nLineNumber);
	}

	if (pszInfo) {
		FBAPopupAddText(PUF_TEXT_NO_TRANSLATE, _T("Problem:\t%s.\n"), pszInfo);
	}
	if (pszLine) {
		FBAPopupAddText(PUF_TEXT_NO_TRANSLATE, _T("Text:\t%s\n"), pszLine);
	}

	FBAPopupDisplay(PUF_TYPE_ERROR);
#endif
}

static INT32 ConfigParseFile(TCHAR* pszFilename)
{
#define INSIDE_NOTHING (0xFFFF & (1 << ((sizeof(TCHAR) * 8) - 1)))

	TCHAR szLine[8192];
	TCHAR* s;
	TCHAR* t;
	INT32 nLen;

	INT32 nLine = 0;
	TCHAR nInside = INSIDE_NOTHING;

	CheatInfo* pCurrentCheat = NULL;

	FILE* h = _tfopen(pszFilename, _T("rt"));
	if (h == NULL) {
		return 1;
	}

	while (1) {
		if (_fgetts(szLine, sizeof(szLine), h) == NULL) {
			break;
		}

		nLine++;

		nLen = _tcslen(szLine);
		// Get rid of the linefeed at the end
		while (szLine[nLen - 1] == 0x0A || szLine[nLen - 1] == 0x0D) {
			szLine[nLen - 1] = 0;
			nLen--;
		}

		s = szLine;													// Start parsing

		if (s[0] == _T('/') && s[1] == _T('/')) {					// Comment
			continue;
		}

		if ((t = LabelCheck(s, _T("include"))) != 0) {				// Include a file
			s = t;

			TCHAR szFilename[MAX_PATH] = _T("");

			// Read name of the cheat file
			TCHAR* szQuote = NULL;
			QuoteRead(&szQuote, NULL, s);

			_stprintf(szFilename, _T("%s%s.dat"), szAppCheatsPath, szQuote);

			if (ConfigParseFile(szFilename)) {
				_stprintf(szFilename, _T("%s%s.ini"), szAppCheatsPath, szQuote);
				if (ConfigParseFile(szFilename)) {
					CheatError(pszFilename, nLine, NULL, _T("included file doesn't exist"), szLine);
				}
			}

			continue;
		}

		if ((t = LabelCheck(s, _T("cheat"))) != 0) {				// Add new cheat
			s = t;

			// Read cheat name
			TCHAR* szQuote = NULL;
			TCHAR* szEnd = NULL;

			QuoteRead(&szQuote, &szEnd, s);

			s = szEnd;

			if ((t = LabelCheck(s, _T("advanced"))) != 0) {			// Advanced cheat
				s = t;
			}

			SKIP_WS(s);

			if (nInside == _T('{')) {
				CheatError(pszFilename, nLine, pCurrentCheat, _T("missing closing bracket"), NULL);
				break;
			}
#if 0
			if (*s != _T('\0') && *s != _T('{')) {
				CheatError(pszFilename, nLine, NULL, _T("malformed cheat declaration"), szLine);
				break;
			}
#endif
			nInside = *s;

			// Link new node into the list
			CheatInfo* pPreviousCheat = pCurrentCheat;
			pCurrentCheat = (CheatInfo*)malloc(sizeof(CheatInfo));
			if (pCheatInfo == NULL) {
				pCheatInfo = pCurrentCheat;
			}

			memset(pCurrentCheat, 0, sizeof(CheatInfo));
			pCurrentCheat->pPrevious = pPreviousCheat;
			if (pPreviousCheat) {
				pPreviousCheat->pNext = pCurrentCheat;
			}

			// Fill in defaults
			pCurrentCheat->nType = 0;								// Default to cheat type 0 (apply each frame)
			pCurrentCheat->nStatus = -1;							// Disable cheat

			memcpy(pCurrentCheat->szCheatName, szQuote, QUOTE_MAX);

			continue;
		}

		if ((t = LabelCheck(s, _T("type"))) != 0) {					// Cheat type
			if (nInside == INSIDE_NOTHING || pCurrentCheat == NULL) {
				CheatError(pszFilename, nLine, pCurrentCheat, _T("rogue cheat type"), szLine);
				break;
			}
			s = t;

			// Set type
			pCurrentCheat->nType = _tcstol(s, NULL, 0);

			continue;
		}

		if ((t = LabelCheck(s, _T("default"))) != 0) {				// Default option
			if (nInside == INSIDE_NOTHING || pCurrentCheat == NULL) {
				CheatError(pszFilename, nLine, pCurrentCheat, _T("rogue default"), szLine);
				break;
			}
			s = t;

			// Set default option
			pCurrentCheat->nDefault = _tcstol(s, NULL, 0);

			continue;
		}

		INT32 n = _tcstol(s, &t, 0);
		if (t != s) {				   								// New option

			if (nInside == INSIDE_NOTHING || pCurrentCheat == NULL) {
				CheatError(pszFilename, nLine, pCurrentCheat, _T("rogue option"), szLine);
				break;
			}

			// Link a new Option structure to the cheat
			if (n < CHEAT_MAX_OPTIONS) {
				s = t;

				// Read option name
				TCHAR* szQuote = NULL;
				TCHAR* szEnd = NULL;
				if (QuoteRead(&szQuote, &szEnd, s)) {
					CheatError(pszFilename, nLine, pCurrentCheat, _T("option name omitted"), szLine);
					break;
				}
				s = szEnd;

				if (pCurrentCheat->pOption[n] == NULL) {
					pCurrentCheat->pOption[n] = (CheatOption*)malloc(sizeof(CheatOption));
				}
				memset(pCurrentCheat->pOption[n], 0, sizeof(CheatOption));

				memcpy(pCurrentCheat->pOption[n]->szOptionName, szQuote, QUOTE_MAX * sizeof(TCHAR));

				INT32 nCurrentAddress = 0;
				bool bOK = true;
				while (nCurrentAddress < CHEAT_MAX_ADDRESS) {
					INT32 nCPU = 0, nAddress = 0, nValue = 0;

					if (SkipComma(&s)) {
						nCPU = _tcstol(s, &t, 0);		// CPU number
						if (t == s) {
							CheatError(pszFilename, nLine, pCurrentCheat, _T("CPU number omitted"), szLine);
							bOK = false;
							break;
						}
						s = t;

						SkipComma(&s);
						nAddress = _tcstol(s, &t, 0);	// Address
						if (t == s) {
							bOK = false;
							CheatError(pszFilename, nLine, pCurrentCheat, _T("address omitted"), szLine);
							break;
						}
						s = t;

						SkipComma(&s);
						nValue = _tcstol(s, &t, 0);		// Value
						if (t == s) {
							bOK = false;
							CheatError(pszFilename, nLine, pCurrentCheat, _T("value omitted"), szLine);
							break;
						}
					} else {
						if (nCurrentAddress) {			// Only the first option is allowed no address
							break;
						}
						if (n) {
							bOK = false;
							CheatError(pszFilename, nLine, pCurrentCheat, _T("CPU / address / value omitted"), szLine);
							break;
						}
					}

					pCurrentCheat->pOption[n]->AddressInfo[nCurrentAddress].nCPU = nCPU;
					pCurrentCheat->pOption[n]->AddressInfo[nCurrentAddress].nAddress = nAddress;
					pCurrentCheat->pOption[n]->AddressInfo[nCurrentAddress].nValue = nValue;
					nCurrentAddress++;
				}

				if (!bOK) {
					break;
				}

			}

			continue;
		}

		SKIP_WS(s);
		if (*s == _T('}')) {
			if (nInside != _T('{')) {
				CheatError(pszFilename, nLine, pCurrentCheat, _T("missing opening bracket"), NULL);
				break;
			}

			nInside = INSIDE_NOTHING;
		}

		// Line isn't (part of) a valid cheat
#if 0
		if (*s) {
			CheatError(pszFilename, nLine, NULL, _T("rogue line"), szLine);
			break;
		}
#endif

	}

	if (h) {
		fclose(h);
	}

	return 0;
}


//TODO: make cross platform
static INT32 ConfigParseNebulaFile(TCHAR* pszFilename)
{

#if defined (BUILD_WIN32)
	FILE *fp = _tfopen(pszFilename, _T("rt"));
	if (fp == NULL) {
		return 1;
	}

	INT32 nLen;
	INT32 i, j, n = 0;
	TCHAR tmp[32];
	TCHAR szLine[1024];

	CheatInfo* pCurrentCheat = NULL;

	while (1)
	{
		if (_fgetts(szLine, 1024, fp) == NULL)
			break;

		nLen = _tcslen(szLine);

		if (nLen < 3 || szLine[0] == '[') continue;

		if (!_tcsncmp (_T("Name="), szLine, 5))
		{
			n = 0;

			// Link new node into the list
			CheatInfo* pPreviousCheat = pCurrentCheat;
			pCurrentCheat = (CheatInfo*)malloc(sizeof(CheatInfo));
			if (pCheatInfo == NULL) {
				pCheatInfo = pCurrentCheat;
			}

			memset(pCurrentCheat, 0, sizeof(CheatInfo));
			pCurrentCheat->pPrevious = pPreviousCheat;
			if (pPreviousCheat) {
				pPreviousCheat->pNext = pCurrentCheat;
			}

			// Fill in defaults
			pCurrentCheat->nType = 0;							// Default to cheat type 0 (apply each frame)
			pCurrentCheat->nStatus = -1;							// Disable cheat
			pCurrentCheat->nDefault = 0;							// Set default option

			_tcsncpy (pCurrentCheat->szCheatName, szLine + 5, QUOTE_MAX);
			pCurrentCheat->szCheatName[nLen-6] = '\0';

			continue;
		}

		if (!_tcsncmp (_T("Default="), szLine, 8) && n >= 0)
		{
			_tcsncpy (tmp, szLine + 8, nLen-9);
			tmp[nLen-9] = '\0';
			_stscanf (tmp, _T("%d"), &(pCurrentCheat->nDefault));
			continue;
		}


		i = 0, j = 0;
		while (i < nLen)
		{
			if (szLine[i] == '=' && i < 4) j = i+1;
			if (szLine[i] == ',' || szLine[i] == '\n')
			{
				if (pCurrentCheat->pOption[n] == NULL) {
					pCurrentCheat->pOption[n] = (CheatOption*)malloc(sizeof(CheatOption));
				}
				memset(pCurrentCheat->pOption[n], 0, sizeof(CheatOption));

				_tcsncpy (pCurrentCheat->pOption[n]->szOptionName, szLine + j, QUOTE_MAX * sizeof(TCHAR));
				pCurrentCheat->pOption[n]->szOptionName[i-j] = '\0';

				i++; j = i;
				break;
			}
			i++;
		}

		INT32 nAddress = -1, nValue = 0, nCurrentAddress = 0;
		while (nCurrentAddress < CHEAT_MAX_ADDRESS)
		{
			if (i == nLen) break;

			if (szLine[i] == ',' || szLine[i] == '\n')
			{
				_tcsncpy (tmp, szLine + j, i-j);
				tmp[i-j] = '\0';

				if (nAddress == -1) {
					_stscanf (tmp, _T("%x"), &nAddress);
				} else {
					_stscanf (tmp, _T("%x"), &nValue);

					pCurrentCheat->pOption[n]->AddressInfo[nCurrentAddress].nCPU = 0; 	// Always
					pCurrentCheat->pOption[n]->AddressInfo[nCurrentAddress].nAddress = nAddress ^ 1;
					pCurrentCheat->pOption[n]->AddressInfo[nCurrentAddress].nValue = nValue;
					nCurrentAddress++;

					nAddress = -1;
					nValue = 0;
				}
				j = i+1;
			}
			i++;
		}
		n++;
	}

	fclose (fp);
#endif
	return 0;
}


//TODO: make cross platform
static INT32 ConfigParseMAMEFile()
{
#if defined (BUILD_WIN32)

#define AddressInfo()	\
	INT32 k = (flags >> 20) & 3;	\
	for (INT32 i = 0; i < k+1; i++) {	\
		pCurrentCheat->pOption[n]->AddressInfo[nCurrentAddress].nCPU = 0;	\
		pCurrentCheat->pOption[n]->AddressInfo[nCurrentAddress].nAddress = nAddress + i;	\
		pCurrentCheat->pOption[n]->AddressInfo[nCurrentAddress].nValue = (nValue >> ((k*8)-(i*8))) & 0xff;	\
		nCurrentAddress++;	\
	}	\

#define OptionName(a)	\
	if (pCurrentCheat->pOption[n] == NULL) {						\
		pCurrentCheat->pOption[n] = (CheatOption*)malloc(sizeof(CheatOption));		\
	}											\
	memset(pCurrentCheat->pOption[n], 0, sizeof(CheatOption));				\
	_tcsncpy (pCurrentCheat->pOption[n]->szOptionName, a, QUOTE_MAX * sizeof(TCHAR));	\

#define tmpcpy(a)	\
	_tcsncpy (tmp, szLine + c0[a] + 1, c0[a+1] - (c0[a]+1));	\
	tmp[c0[a+1] - (c0[a]+1)] = '\0';				\

	TCHAR szFileName[MAX_PATH] = _T("");
	_stprintf(szFileName, _T("%scheat.dat"), szAppCheatsPath);
	
	FILE *fz = _tfopen(szFileName, _T("rt"));
	if (fz == NULL) {
		return 1;
	}

	TCHAR tmp[256];
	TCHAR gName[64];
	TCHAR szLine[1024];

	INT32 nLen;
	INT32 n = 0;
	INT32 menu = 0;
	INT32 nFound = 0;
	INT32 nCurrentAddress = 0;
	UINT32 flags = 0;
	UINT32 nAddress = 0;
	UINT32 nValue = 0;

	CheatInfo* pCurrentCheat = NULL;
	_stprintf(gName, _T(":%s:"), BurnDrvGetText(DRV_NAME));

	while (1)
	{
		if (_fgetts(szLine, 1024, fz) == NULL)
			break;

		nLen = _tcslen (szLine);

		if (szLine[0] == ';') continue;

		if (_tcsncmp (szLine, gName, lstrlen(gName))) {
			if (nFound) break;
			else continue;
		}

		nFound = 1;

		INT32 c0[16], c1 = 0;					// find colons / break
		for (INT32 i = 0; i < nLen; i++)
			if (szLine[i] == ':' || szLine[i] == '\n')
				c0[c1++] = i;

		tmpcpy(1);						// control flags
		_stscanf (tmp, _T("%x"), &flags);

		tmpcpy(2);						// cheat address
		_stscanf (tmp, _T("%x"), &nAddress);

		tmpcpy(3);						// cheat value
		_stscanf (tmp, _T("%x"), &nValue);

		tmpcpy(5);						// cheat name

		if (flags & 0x80007f00) continue;			// skip various cheats

		// controls how many bytes we're going to patch (only allow single bytes for now)
	//	if (flags & 0x00300000) continue;
	//	nValue &= 0x000000ff;			// only use a single byte

		if ( flags & 0x00008000 || (flags & 0x0001000 && !menu)) {
			if (nCurrentAddress < CHEAT_MAX_ADDRESS) {
				AddressInfo();
			}

			continue;
		}

		if (~flags & 0x00010000) {
			n = 0;
			menu = 0;
			nCurrentAddress = 0;

			// Link new node into the list
			CheatInfo* pPreviousCheat = pCurrentCheat;
			pCurrentCheat = (CheatInfo*)malloc(sizeof(CheatInfo));
			if (pCheatInfo == NULL) {
				pCheatInfo = pCurrentCheat;
			}

			memset(pCurrentCheat, 0, sizeof(CheatInfo));
			pCurrentCheat->pPrevious = pPreviousCheat;
			if (pPreviousCheat) {
				pPreviousCheat->pNext = pCurrentCheat;
			}

			// Fill in defaults
			pCurrentCheat->nType = 0;							// Default to cheat type 0 (apply each frame)
			pCurrentCheat->nStatus = -1;							// Disable cheat
			pCurrentCheat->nDefault = 0;							// Set default option

			_tcsncpy (pCurrentCheat->szCheatName, tmp, QUOTE_MAX);

			if (lstrlen(tmp) <= 0 || flags == 0x60000000) {
				n++;
				continue;
			}

			OptionName(_T("Disabled"));

			if (nAddress) {
				n++;

				OptionName(tmp);	
				AddressInfo();
			} else {
				menu = 1;
			}

			continue;
		}

		if ( flags & 0x00010000 && menu) {
			n++;
			nCurrentAddress = 0;

			OptionName(tmp);
			AddressInfo();
			
			continue;
		}
	}

	fclose (fz);
#endif
	return 0;
}


INT32 ConfigCheatLoad()
{
	TCHAR szFilename[MAX_PATH] = _T("");

	_stprintf(szFilename, _T("%s%s.ini"), szAppCheatsPath, BurnDrvGetText(DRV_NAME));
	if (ConfigParseFile(szFilename)) {
		_stprintf(szFilename, _T("%s%s.dat"), szAppCheatsPath, BurnDrvGetText(DRV_NAME));
		if (ConfigParseNebulaFile(szFilename)) {
			if (ConfigParseMAMEFile()) {
				return 1;
			}
		}
	}

	if (pCheatInfo) {
		INT32 nCurrentCheat = 0;
		while (CheatEnable(nCurrentCheat, -1) == 0) {
			nCurrentCheat++;
		}

		CheatUpdate();
	}

	return 0;
}



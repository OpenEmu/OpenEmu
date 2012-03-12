#include "burner.h"
#include <ddeml.h>

// #define PRINT_TRANSLATION_INFO

bool bLocalisationActive = false;
TCHAR szLocalisationTemplate[MAX_PATH] = _T("");

static const unsigned int nMaxResources = 2000;

static int nFBACodepage;

struct LocaliseControlInfo { unsigned int nID; wchar_t szCaption[QUOTE_MAX]; };
struct LocaliseResourceInfo { unsigned int nID; wchar_t szCaption[QUOTE_MAX]; LocaliseControlInfo* pControlInfo[1024]; };

#define RES_DEALLOCATE		(1 << 0)
#define RES_ISTRANSLATION	(1 << 1)

struct FBAResourceInfo_t { int nResourceFlags; void* pResourceTranslation; };
static FBAResourceInfo_t* FBAResourceInfo = NULL;

typedef struct {
	WORD dlgVer;
	WORD signature;
	DWORD helpID;
	DWORD exStyle;
	DWORD style;
	WORD cDlgItems;
	short x;
	short y;
	short cx;
	short cy;
//	sz_Or_Ord menu;
//	sz_Or_Ord windowClass;
//	WCHAR title[titleLen];
//	WORD pointsize;
//	WORD weight;
//	BYTE italic;
//	BYTE charset;
//	WCHAR typeface[stringLen];
} DLGTEMPLATEEX;

typedef struct {
	DWORD helpID;
	DWORD exStyle;
	DWORD style;
	short x;
	short y;
	short cx;
	short cy;
	WORD id;
//	sz_Or_Ord windowClass;
//	sz_Or_Ord title;
//	WORD extraCount;
} DLGITEMTEMPLATEEX;

// ----------------------------------------------------------------------------

#define WSKIP_WS(s) while (iswspace(*s)) { s++; }			// Skip whitespace
#define WFIND_WS(s) while (*s && !iswspace(*s)) { s++; }	// Find whitespace
#define WFIND_QT(s) while (*s && *s != L'\"') { s++; }		// Find quote

#if defined (UNICODE)
#define QuoteReadW QuoteRead
#define LabelCheckW LabelCheck
#else
int QuoteReadW(wchar_t** ppszQuote, wchar_t** ppszEnd, wchar_t* pszSrc)
{
	static wchar_t szQuote[QUOTE_MAX];
	wchar_t* s = pszSrc;
	wchar_t* e;

	// Skip whitespace
	WSKIP_WS(s);

	e = s;

	if (*s == _T('\"')) {										// Quoted string
		s++;
		e++;
		// Find end quote
		WFIND_QT(e);
		wcsncpy(szQuote, s, e - s);
		// Zero-terminate
		szQuote[e - s] = _T('\0');
		e++;
	} else {													// Non-quoted string
		// Find whitespace
		WFIND_WS(e);
		wcsncpy(szQuote, s, e - s);
		// Zero-terminate
		szQuote[e - s] = _T('\0');
	}

	if (ppszQuote) {
		*ppszQuote = szQuote;
	}
	if (ppszEnd)	{
		*ppszEnd = e;
	}

	return 0;
}

wchar_t* LabelCheckW(wchar_t* s, wchar_t* pszLabel)
{
	int nLen;

	if (s == NULL) {
		return NULL;
	}
	if (pszLabel == NULL) {
		return NULL;
	}
	nLen = wcslen(pszLabel);

	WSKIP_WS(s);												// Skip whitespace

	if (wcsncmp(s, pszLabel, nLen)){							// Doesn't match
		return NULL;
	}

	return s + nLen;
}
#endif

// ----------------------------------------------------------------------------

int EscapeString(wchar_t* pwszStringOut, wchar_t* pwszStringIn, int max)
{
	int in = 0, out = 0;

	while (pwszStringIn[in] && out < (max - 1)) {
		switch (pwszStringIn[in]) {
			case L'\n':
				pwszStringOut[out++] = L'\\';
				pwszStringOut[out++] = L'n';
				break;
			case L'\t':
				pwszStringOut[out++] = L'\\';
				pwszStringOut[out++] = L't';
				break;
			case L'\'':
				pwszStringOut[out++] = L'\\';
				pwszStringOut[out++] = L'\'';
				break;
			case L'\"':
				pwszStringOut[out++] = L'\\';
				pwszStringOut[out++] = L'\"';
				break;
			case L'\\':
				pwszStringOut[out++] = L'\\';
				pwszStringOut[out++] = L'\\';
				break;

			default:
				pwszStringOut[out++] = pwszStringIn[in];
		}

		in++;
	}

	pwszStringOut[out] = L'\0';

	return out;
}

int UnEscapeString(wchar_t* pwszStringOut, wchar_t* pwszStringIn, int max)
{
	int in = 0, out = 0;

	while (pwszStringIn[in] && in < max) {
		if (pwszStringIn[in] == L'\\') {
			in++;
			switch (pwszStringIn[in++]) {
				case L'n':
					pwszStringOut[out++] = L'\n';
					break;
				case L't':
					pwszStringOut[out++] = L'\t';
					break;
				case L'\'':
					pwszStringOut[out++] = L'\'';
					break;
				case L'\"':
					pwszStringOut[out++] = L'\"';
					break;
				case L'\\':
					pwszStringOut[out++] = L'\\';
					break;

				default:
					pwszStringOut[out++] = pwszStringIn[in];
			}
		} else {
			pwszStringOut[out++] = pwszStringIn[in++];
		}
	}

	pwszStringOut[out] = L'\0';

	return out;
}

// ----------------------------------------------------------------------------
// Translation template creation

int BuildTemplateDlgTemplateEx(const DLGTEMPLATEEX* pTemplate, LPTSTR lpszName, FILE* fp)
{
	wchar_t wszBuffer[5120];
	wchar_t* pszCaption = L"";
	int nControls = 0;

	// 26 is sizeof(DLGTEMPLATEEX)
	char* pTemplateDataIn = (char*)(((INT_PTR)pTemplate + 26 + sizeof(WORD) - 1) & ~(sizeof(WORD) - 1));

	// Menu
	switch (*((WORD*)pTemplateDataIn)) {
		case 0x0000:
			pTemplateDataIn += 1 * sizeof(WORD);
			break;
		case 0xFFFF:
			pTemplateDataIn += 2 * sizeof(WORD);
			break;
		default:
			pTemplateDataIn += wcslen((wchar_t*)pTemplateDataIn) * sizeof(wchar_t) + sizeof(wchar_t);
			break;
	}

	// Class
	switch (*((WORD*)pTemplateDataIn)) {
		case 0x0000:
			pTemplateDataIn += 1 * sizeof(WORD);
			break;
		case 0xFFFF:
			pTemplateDataIn += 2 * sizeof(WORD);
			break;
		default:
			pTemplateDataIn += wcslen((wchar_t*)pTemplateDataIn) * sizeof(wchar_t) + sizeof(wchar_t);
			break;
	}

	// Caption
	switch (*((WORD*)pTemplateDataIn)) {
		case 0x0000:
			pTemplateDataIn += 1 * sizeof(WORD);
			break;
		case 0xFFFF:
			pTemplateDataIn += 2 * sizeof(WORD);
			break;
		default:
			pszCaption = (wchar_t*)pTemplateDataIn;
			pTemplateDataIn += wcslen((wchar_t*)pTemplateDataIn) * sizeof(wchar_t) + sizeof(wchar_t);
			break;
	}

	if (fp) {
		EscapeString(wszBuffer, pszCaption, 5120);
		_ftprintf(fp, _T("        //-- \"%ls\"\n"), wszBuffer);
		if (IS_INTRESOURCE(lpszName)) {
			_ftprintf(fp, _T("dialog\t%4i "), (INT_PTR)lpszName);
		} else {
#if defined (UNICODE)
			EscapeString(wszBuffer, lpszName, 5120);
#else
			wchar_t szTemp[5120] = L"";

			MultiByteToWideChar(nFBACodepage, 0, lpszName, -1, szTemp, 5120);
			EscapeString(wszBuffer, szTemp, 5120);
#endif
			_ftprintf(fp, _T("dialog\t%s "), wszBuffer);
		}

		_ftprintf(fp, _T("\"\" {\n"));
	}

	// Font
	if (pTemplate->style & (DS_SETFONT | DS_SHELLFONT)) {

#if 0
		if (fp) {
			EscapeString(wszBuffer, (wchar_t*)(pTemplateDataIn + 2 + 2 * sizeof(WORD)), 5120);
			_ftprintf(fp, _T("\tfont %2i \"%ls\"\n"), *((WORD*)pTemplateDataIn), wszBuffer);
		}
#endif

		pTemplateDataIn += 2 + 2 * sizeof(WORD);
		pTemplateDataIn += wcslen((wchar_t*)pTemplateDataIn) * sizeof(wchar_t) + sizeof(wchar_t);
	}

	// Controls
	for (int i = 0; i < pTemplate->cDlgItems; i++) {
		bool bExcludeControl = false;
		DWORD dwStyle;
		WORD wID;

		pTemplateDataIn = (char*)(((INT_PTR)pTemplateDataIn + sizeof(DWORD) - 1) & ~(sizeof(DWORD) - 1));

		dwStyle = ((DLGITEMTEMPLATEEX*)pTemplateDataIn)->style;
		wID = ((DLGITEMTEMPLATEEX*)pTemplateDataIn)->id;

		pTemplateDataIn = (char*)(((INT_PTR)pTemplateDataIn + sizeof(DLGITEMTEMPLATEEX) + sizeof(WORD) - 1) & ~(sizeof(WORD) - 1));

		// Class
		switch (*((WORD*)pTemplateDataIn)) {
			case 0x0000:
				pTemplateDataIn += 1 * sizeof(WORD);
				break;
			case 0xFFFF:
				if ( ((WORD*)pTemplateDataIn)[1] == 0x0081 ||
					(((WORD*)pTemplateDataIn)[1] == 0x0082 && (dwStyle & SS_BITMAP))) {

					bExcludeControl = true;
				}

				pTemplateDataIn += 2 * sizeof(WORD);
				break;
			default:
				if ( !wcsicmp((wchar_t*)pTemplateDataIn, L"SysTreeView32") ||
					 !wcsicmp((wchar_t*)pTemplateDataIn, L"SysListView32") ||
					 !wcsicmp((wchar_t*)pTemplateDataIn, L"Edit")		   ||
					 !wcsnicmp((wchar_t*)pTemplateDataIn, L"RichEdit", 8)  ||
					(!wcsicmp((wchar_t*)pTemplateDataIn, L"Static") && (dwStyle & SS_BITMAP))) {

					bExcludeControl = true;
				}

				pTemplateDataIn += wcslen((wchar_t*)pTemplateDataIn) * sizeof(wchar_t) + sizeof(wchar_t);
				break;
		}

		if (*((WORD*)pTemplateDataIn) == 0xFFFF) {
			pTemplateDataIn += 4;
		}

		if (wcslen((wchar_t*)pTemplateDataIn) && !bExcludeControl) {
			nControls++;

			if (fp) {
				EscapeString(wszBuffer, (wchar_t*)pTemplateDataIn, 5120);
				_ftprintf(fp, _T("        //-- \"%ls\"\n"), wszBuffer);
				_ftprintf(fp, _T("  %5i\t%4i \"\"\n"), wID, i);
			}
		}

		pTemplateDataIn += wcslen((wchar_t*)pTemplateDataIn) * sizeof(wchar_t) + sizeof(wchar_t);
		pTemplateDataIn += sizeof(WORD) + *((WORD*)pTemplateDataIn);
	}

	if (fp) {
		_ftprintf(fp, _T("}\n\n"));
	}

	return nControls;
}

int BuildTemplateMenuTemplate(const MENUTEMPLATE* pTemplate, LPTSTR lpszName, FILE* fp)
{
	wchar_t wszBuffer[5120];
	TCHAR szIndent[] = _T("                ");

	char* pTemplateDataIn = (char*)pTemplate + sizeof(MENUITEMTEMPLATEHEADER);
	bool bLastPopup = false, bLastItem = false;
	int nIndent = 0;
	int bracketcnt = 0; // popup menu counter (BEGIN...END)
	int i = 0;

	if (fp) {
		if (IS_INTRESOURCE(lpszName)) {
			_ftprintf(fp, _T("menu   %4i {\n"), (INT_PTR)lpszName);
		} else {
#if defined (UNICODE)
			EscapeString(wszBuffer, lpszName, 5120);
#else
			wchar_t szTemp[5120] = L"";

			MultiByteToWideChar(nFBACodepage, 0, lpszName, -1, szTemp, 5120);
			EscapeString(wszBuffer, szTemp, 5120);
#endif
			_ftprintf(fp, _T("menu   %s {\n"), wszBuffer);
		}
	}

	do {
		wchar_t pszTitle[1024] = L"";
		WORD wOption = ((MENUITEMTEMPLATE*)pTemplateDataIn)->mtOption;
		WORD wID = 0;
		int l;

		if (((MENUITEMTEMPLATE*)pTemplateDataIn)->mtOption & MF_POPUP) {
			bLastPopup = false;
			bLastItem = false;
			bracketcnt++;
			if (((MENUITEMTEMPLATE*)pTemplateDataIn)->mtOption & MF_END) {
				bLastPopup = true;
				bracketcnt--;
			}
			pTemplateDataIn += sizeof(WORD);
		} else {
			if (((MENUITEMTEMPLATE*)pTemplateDataIn)->mtOption & MF_END) {
				bLastItem = true;
				bracketcnt--;
			}
			wID = ((MENUITEMTEMPLATE*)pTemplateDataIn)->mtID;
			pTemplateDataIn += sizeof(WORD) * 2;
		}

		for (l = 0; l < 1023 && ((wchar_t*)pTemplateDataIn)[l] && ((wchar_t*)pTemplateDataIn)[l] != _T('\t'); l++) { }

		wcsncpy(pszTitle, (wchar_t*)pTemplateDataIn, l);

		if (fp) {
			if (wcslen((wchar_t*)pTemplateDataIn)) {
				EscapeString(wszBuffer, pszTitle, 5120);
				_ftprintf(fp, _T("%s        //-- \"%ls\"\n"), szIndent + (8 - nIndent) * 2, wszBuffer);
				_ftprintf(fp, _T("%s"), szIndent + (8 - nIndent) * 2);
				if (wOption & MF_POPUP) {
					_ftprintf(fp, _T("  popup %4i \"\" {\n"), i);
					nIndent++;
				} else {
					_ftprintf(fp, _T("  %5i %4i \"\"\n"), wID, i);
				}
			}

			if ((wOption & MF_END) && !(wOption & MF_POPUP)) {
				nIndent--;
				if (nIndent < 0) {
					nIndent = 0;
				}
				if (nIndent > 8) {
					nIndent = 8;
				}

				if (fp) {
					_ftprintf(fp, _T("%s  }\n"), szIndent + (8 - nIndent) * 2);
				}
			}
		}
		pTemplateDataIn += wcslen((wchar_t*)pTemplateDataIn) * sizeof(wchar_t) + sizeof(wchar_t);

	} while (i++ < 1024 && (!(bLastPopup && bLastItem) || bracketcnt > 0));

	if (fp) {
		while (nIndent) {
			nIndent--;
			_ftprintf(fp, _T("%s  }\n"), szIndent + (8 - nIndent) * 2);
		}

		_ftprintf(fp, _T("}\n\n"));
	}

	return i;
}

static BOOL CALLBACK FBALocaliseEnumResourceNamesDialog(HMODULE /* hModule */, LPCTSTR /* lpszType */, LPTSTR lpszName, LONG_PTR lParam)
{
	LPCDLGTEMPLATE pTemplate = (LPCDLGTEMPLATE)LoadResource(hAppInst, FindResource(hAppInst, lpszName, RT_DIALOG));

	if (LockResource((HGLOBAL)pTemplate)) {
		if (((DLGTEMPLATEEX*)pTemplate)->signature == 0xFFFF && ((DLGTEMPLATEEX*)pTemplate)->dlgVer == 1) {
			if (BuildTemplateDlgTemplateEx((DLGTEMPLATEEX*)pTemplate, lpszName, NULL)) {
				BuildTemplateDlgTemplateEx((DLGTEMPLATEEX*)pTemplate, lpszName, (FILE*)lParam);
			}
		}
	}

	return TRUE;
}

static BOOL CALLBACK FBALocaliseEnumResourceNamesMenu(HMODULE /* hModule */, LPCTSTR /* lpszType */, LPTSTR lpszName, LONG_PTR lParam)
{
	MENUTEMPLATE* pTemplate = (MENUTEMPLATE*)LoadResource(hAppInst, FindResource(hAppInst, lpszName, RT_MENU));

	if (LockResource((HGLOBAL)pTemplate)) {
		if (((MENUITEMTEMPLATEHEADER*)pTemplate)->versionNumber == 0) {
			BuildTemplateMenuTemplate((MENUITEMTEMPLATEHEADER*)pTemplate, lpszName, (FILE*)lParam);
		}
	}

	return TRUE;
}

static BOOL CALLBACK FBALocaliseEnumResourceNamesString(HMODULE /* hModule */, LPCTSTR /* lpszType */, LPTSTR lpszName, LONG_PTR lParam)
{
	wchar_t* pwsz = (wchar_t*)LoadResource(hAppInst, FindResource(hAppInst, lpszName, RT_STRING));

	if (LockResource(pwsz)) {
		wchar_t wszBuffer[5120];

		// Locate the string in the bundle
		for (int i = 0; i < 16; i++) {
			if (*pwsz) {
				EscapeString(wszBuffer, pwsz + 1, 5120);
				_ftprintf((FILE*)lParam, _T("        //-- \"%ls\"\n"), wszBuffer);
				_ftprintf((FILE*)lParam, _T("string %5i \"\"\n\n"), (((INT_PTR)lpszName) - 1) * 16 + i);
			}
			pwsz += *pwsz + 1;
		}
	}

	return TRUE;
}

static BOOL CALLBACK FBALocaliseEnumResTypeProc(HMODULE /* hModule */, LPTSTR lpszType, LONG_PTR lParam)
{
	if (lpszType == RT_DIALOG) {
		EnumResourceNames(NULL, lpszType, &FBALocaliseEnumResourceNamesDialog, lParam);
	}
	if (lpszType == RT_MENU) {
		EnumResourceNames(NULL, lpszType, &FBALocaliseEnumResourceNamesMenu, lParam);
	}
	if (lpszType == RT_STRING) {
		EnumResourceNames(NULL, lpszType, &FBALocaliseEnumResourceNamesString, lParam);
	}

	return TRUE;
}

static int FBALocaliseWriteTemplate(TCHAR* pszTemplate)
{
	FILE* fp = _tfopen(pszTemplate, _T("wt"));
	if (fp == NULL) {
		return 1;
	}

	_ftprintf(fp, _T("// translation template for FB Alpha\n\n"));
	_ftprintf(fp, _T("version 0x%06X\n\n"), nBurnVer);

	_ftprintf(fp, _T("// codepage 1252\n\n"));

	EnumResourceTypes(NULL, (ENUMRESTYPEPROC)FBALocaliseEnumResTypeProc, (LONG_PTR)fp);

	fclose(fp);

	return 0;
}

// -----------------------------------------------------------------------------

#define CATCH_UP { int nCopySize = pTemplateDataIn - pTemplateDataInSync;								\
				   int nTotalSize = pTemplateDataOutSync - pTemplateDataOut;							\
				   pTemplateDataOut = (char*)realloc(pTemplateDataOut, nTotalSize + nCopySize);			\
				   memcpy(pTemplateDataOut + nTotalSize, pTemplateDataInSync, nCopySize);				\
				   pTemplateDataInSync = pTemplateDataIn;												\
				   pTemplateDataOutSync = pTemplateDataOut + nTotalSize + nCopySize; }

#define ALIGN(a) { int nTotalSize = (pTemplateDataOutSync - pTemplateDataOut + a - 1) & ~(a - 1);		\
				   pTemplateDataOut = (char*)realloc(pTemplateDataOut, nTotalSize);						\
				   pTemplateDataOutSync = pTemplateDataOut + nTotalSize; }

#define ADD_STRING(s) { int nTotalSize = pTemplateDataOutSync - pTemplateDataOut;						\
						int nExtraSize = wcslen((wchar_t*)s) * sizeof(wchar_t) + sizeof(wchar_t);		\
						pTemplateDataOut = (char*)realloc(pTemplateDataOut, nTotalSize + nExtraSize);	\
						wcscpy((wchar_t*)(pTemplateDataOut + nTotalSize), s);							\
						pTemplateDataOutSync = pTemplateDataOut + nTotalSize + nExtraSize; }

#ifdef PRINT_TRANSLATION_INFO
void ParseDlgTemplateEx(const DLGTEMPLATEEX* pTemplate)
{
	// 26 is sizeof(DLGTEMPLATEEX)
	char* pTemplateDataIn = (char*)(((UINT32)pTemplate + 26 + sizeof(WORD) - 1) & ~(sizeof(WORD) - 1));

#if 0
	dprintf(_T("\n"));
	char* pData = (char*)pTemplate;
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
				unsigned int n = *pData;
				dprintf(_T("%02X"), n & 255);
				pData++;
			}
			dprintf(_T(" "));
		}
		dprintf(_T("\n"));
	}
	dprintf(_T("\n"));
#endif

	// Menu
	switch (*((WORD*)pTemplateDataIn)) {
		case 0x0000:
			pTemplateDataIn += 1 * sizeof(WORD);
			break;
		case 0xFFFF:
			dprintf(_T("   menu is %04X.\n"), *((UINT16*)pTemplateDataIn + sizeof(WORD)));
			pTemplateDataIn += 2 * sizeof(WORD);
			break;
		default:
			dprintf(_T("   menu is \"%ls\".\n"), (wchar_t*)pTemplateDataIn);
			pTemplateDataIn += wcslen((wchar_t*)pTemplateDataIn) * sizeof(wchar_t) + sizeof(wchar_t);
			break;
	}

//	pTemplateDataIn = (char*)(((UINT32)pTemplateDataIn + sizeof(WORD) - 1) & ~(sizeof(WORD) - 1));

	// Class
	switch (*((WORD*)pTemplateDataIn)) {
		case 0x0000:
			pTemplateDataIn += 1 * sizeof(WORD);
			break;
		case 0xFFFF:
			dprintf(_T("   class is %04X.\n"), *((UINT16*)pTemplateDataIn + sizeof(WORD)));
			pTemplateDataIn += 2 * sizeof(WORD);
			break;
		default:
			dprintf(_T("   class is \"%ls\".\n"), (wchar_t*)pTemplateDataIn);
			pTemplateDataIn += wcslen((wchar_t*)pTemplateDataIn) * sizeof(wchar_t) + sizeof(wchar_t);
			break;
	}

//	pTemplateDataIn = (char*)(((UINT32)pTemplateDataIn + sizeof(WORD) - 1) & ~(sizeof(WORD) - 1));

	// Caption
	dprintf(_T("   "));
	switch (*((WORD*)pTemplateDataIn)) {
		case 0x0000:
			pTemplateDataIn += 1 * sizeof(WORD);
			break;
		case 0xFFFF:
			dprintf(_T("icon is %04X, "), *((UINT16*)pTemplateDataIn + sizeof(WORD)));
			pTemplateDataIn += 2 * sizeof(WORD);
			break;
		default:
			dprintf(_T("caption is \"%ls\".\n"), (wchar_t*)pTemplateDataIn);
			pTemplateDataIn += wcslen((wchar_t*)pTemplateDataIn) * sizeof(wchar_t) + sizeof(wchar_t);
			break;
	}

	// Font
	if (pTemplate->style & (DS_SETFONT | DS_SHELLFONT)) {
		dprintf(_T("   Font: \"%ls\", %i pt.\n"), (wchar_t*)(pTemplateDataIn + 2 + 2 * sizeof(WORD)), *((WORD*)pTemplateDataIn));
		pTemplateDataIn += 2 + 2 * sizeof(WORD);
		pTemplateDataIn += wcslen((wchar_t*)pTemplateDataIn) * sizeof(wchar_t) + sizeof(wchar_t);
	}

	// Controls
	dprintf(_T("   %i controls used.\n"), pTemplate->cDlgItems);
	for (int i = 0; i < pTemplate->cDlgItems; i++) {
		pTemplateDataIn = (char*)(((UINT32)pTemplateDataIn + sizeof(DWORD) - 1) & ~(sizeof(DWORD) - 1));

		dprintf(_T("      control %02i, ID is %05i, pos %03ix%03i, size %03ix%03i\n"), i, ((DLGITEMTEMPLATEEX*)pTemplateDataIn)->id, ((DLGITEMTEMPLATEEX*)pTemplateDataIn)->x, ((DLGITEMTEMPLATEEX*)pTemplateDataIn)->y, ((DLGITEMTEMPLATEEX*)pTemplateDataIn)->cx, ((DLGITEMTEMPLATEEX*)pTemplateDataIn)->cy);

		pTemplateDataIn = (char*)(((UINT32)pTemplateDataIn + sizeof(DLGITEMTEMPLATEEX) + sizeof(WORD) - 1) & ~(sizeof(WORD) - 1));

		dprintf(_T("         "));

		// Class
		switch (*((WORD*)pTemplateDataIn)) {
			case 0x0000:
				pTemplateDataIn += 1 * sizeof(WORD);
				break;
			case 0xFFFF:
				dprintf(_T("class is %04X, "), *((UINT16*)pTemplateDataIn + sizeof(WORD)));
				pTemplateDataIn += 2 * sizeof(WORD);
				break;
			default:
				dprintf(_T("class is \"%ls\", "), (wchar_t*)pTemplateDataIn);
				pTemplateDataIn += wcslen((wchar_t*)pTemplateDataIn) * sizeof(wchar_t) + sizeof(wchar_t);
				break;
		}

//		pTemplateDataIn = (char*)(((UINT32)pTemplateDataIn + sizeof(WORD) - 1) & ~(sizeof(WORD) - 1));

		if (*((WORD*)pTemplateDataIn) == 0xFFFF) {
			pTemplateDataIn += 4;
		}

		dprintf(_T("title is \"%ls\".\n"), (wchar_t*)pTemplateDataIn);
		pTemplateDataIn += wcslen((wchar_t*)pTemplateDataIn) * sizeof(wchar_t) + sizeof(wchar_t);
		if (*((WORD*)pTemplateDataIn)) {
			dprintf(_T("         %i bytes of extra data present.\n"), *((WORD*)pTemplateDataIn));
		}
		pTemplateDataIn += sizeof(WORD) + *((WORD*)pTemplateDataIn);
	}
}
#endif

DLGTEMPLATE* TranslateDlgTemplateEx(const DLGTEMPLATEEX* pTemplate, const LocaliseResourceInfo* pLocaliseInfo)
{
	// 26 is sizeof(DLGTEMPLATEEX)
	char* pTemplateDataIn = (char*)(((INT_PTR)pTemplate + 26 + sizeof(WORD) - 1) & ~(sizeof(WORD) - 1));
	char* pTemplateDataOut = NULL;
	char* pTemplateDataInSync = NULL;
	char* pTemplateDataOutSync = NULL;

#ifdef PRINT_TRANSLATION_INFO
	dprintf(_T("Translating dialogex template:\n"));
#endif

	pTemplateDataOut = (char*)malloc(26);
	memcpy(pTemplateDataOut, (char*)pTemplate, 26);
	pTemplateDataInSync = pTemplateDataIn;
	pTemplateDataOutSync = pTemplateDataOut + 26;

#ifdef PRINT_TRANSLATION_INFO
 #if 0
	dprintf(_T("\n"));
	char* pData = (char*)pTemplate;
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
				unsigned int n = *pData;
				dprintf(_T("%02X"), n & 255);
				pData++;
			}
			dprintf(_T(" "));
		}
		dprintf(_T("\n"));
	}
	dprintf(_T("\n"));
 #endif
#endif

	// Menu
	switch (*((WORD*)pTemplateDataIn)) {
		case 0x0000:
			pTemplateDataIn += 1 * sizeof(WORD);
			break;
		case 0xFFFF:
#ifdef PRINT_TRANSLATION_INFO
			dprintf(_T("   menu is %04X.\n"), *((UINT16*)pTemplateDataIn + sizeof(WORD)));
#endif
			pTemplateDataIn += 2 * sizeof(WORD);
			break;
		default:
#ifdef PRINT_TRANSLATION_INFO
			dprintf(_T("   menu is \"%ls\".\n"), (wchar_t*)pTemplateDataIn);
#endif
			pTemplateDataIn += wcslen((wchar_t*)pTemplateDataIn) * sizeof(wchar_t) + sizeof(wchar_t);
			break;
	}

//	pTemplateDataIn = (char*)(((UINT32)pTemplateDataIn + sizeof(WORD) - 1) & ~(sizeof(WORD) - 1));

	// Class
	switch (*((WORD*)pTemplateDataIn)) {
		case 0x0000:
			pTemplateDataIn += 1 * sizeof(WORD);
			break;
		case 0xFFFF:
#ifdef PRINT_TRANSLATION_INFO
			dprintf(_T("   class is %04X.\n"), *((UINT16*)pTemplateDataIn + sizeof(WORD)));
#endif
			pTemplateDataIn += 2 * sizeof(WORD);
			break;
		default:
#ifdef PRINT_TRANSLATION_INFO
			dprintf(_T("   class is \"%ls\".\n"), (wchar_t*)pTemplateDataIn);
#endif
			pTemplateDataIn += wcslen((wchar_t*)pTemplateDataIn) * sizeof(wchar_t) + sizeof(wchar_t);
			break;
	}

//	pTemplateDataIn = (char*)(((UINT32)pTemplateDataIn + sizeof(WORD) - 1) & ~(sizeof(WORD) - 1));

	// Caption
#ifdef PRINT_TRANSLATION_INFO
	dprintf(_T("   "));
#endif
	if (*((WORD*)pTemplateDataIn) == 0xFFFF) {
#ifdef PRINT_TRANSLATION_INFO
		dprintf(_T("icon is %04X, "), *((UINT16*)pTemplateDataIn + sizeof(WORD)));
#endif
		pTemplateDataIn += 2 * sizeof(WORD);
	}
	CATCH_UP;
	if (pLocaliseInfo->szCaption[0]) {
		ADD_STRING(pLocaliseInfo->szCaption);
	} else {
		ADD_STRING((wchar_t*)pTemplateDataIn);
	}

	switch (*((WORD*)pTemplateDataIn)) {
		case 0x0000:
			pTemplateDataIn += 1 * sizeof(WORD);
			break;
		default:
#ifdef PRINT_TRANSLATION_INFO
			dprintf(_T("caption is \"%ls\".\n"), (wchar_t*)pTemplateDataIn);
#endif
			pTemplateDataIn += wcslen((wchar_t*)pTemplateDataIn) * sizeof(wchar_t) + sizeof(wchar_t);
			break;
	}
	pTemplateDataInSync = pTemplateDataIn;

	// Font
	if (pTemplate->style & (DS_SETFONT | DS_SHELLFONT)) {
#ifdef PRINT_TRANSLATION_INFO
		dprintf(_T("   Font: \"%ls\", %i pt.\n"), (wchar_t*)(pTemplateDataIn + 2 + 2 * sizeof(WORD)), *((WORD*)pTemplateDataIn));
#endif
		pTemplateDataIn += 2 + 2 * sizeof(WORD);

#if 0
		CATCH_UP;
		ADD_STRING(L"Times New Roman");
		pTemplateDataIn += wcslen((wchar_t*)pTemplateDataIn) * sizeof(wchar_t) + sizeof(wchar_t);
		pTemplateDataInSync = pTemplateDataIn;
#else
		pTemplateDataIn += wcslen((wchar_t*)pTemplateDataIn) * sizeof(wchar_t) + sizeof(wchar_t);
#endif

	}

	// Controls
#ifdef PRINT_TRANSLATION_INFO
	dprintf(_T("   %i controls used.\n"), pTemplate->cDlgItems);
#endif
	for (int i = 0; i < pTemplate->cDlgItems; i++) {

		CATCH_UP;
		ALIGN(sizeof(DWORD));

		pTemplateDataIn = pTemplateDataInSync = (char*)(((INT_PTR)pTemplateDataIn + sizeof(DWORD) - 1) & ~(sizeof(DWORD) - 1));

#ifdef PRINT_TRANSLATION_INFO
		dprintf(_T("      control %02i, ID is %05i, pos %03ix%03i, size %03ix%03i\n"), i, ((DLGITEMTEMPLATEEX*)pTemplateDataIn)->id, ((DLGITEMTEMPLATEEX*)pTemplateDataIn)->x, ((DLGITEMTEMPLATEEX*)pTemplateDataIn)->y, ((DLGITEMTEMPLATEEX*)pTemplateDataIn)->cx, ((DLGITEMTEMPLATEEX*)pTemplateDataIn)->cy);
#endif

		pTemplateDataIn = (char*)(((INT_PTR)pTemplateDataIn + sizeof(DLGITEMTEMPLATEEX) + sizeof(WORD) - 1) & ~(sizeof(WORD) - 1));

#ifdef PRINT_TRANSLATION_INFO
		dprintf(_T("         "));
#endif

		// Class
		switch (*((WORD*)pTemplateDataIn)) {
			case 0x0000:
				pTemplateDataIn += 1 * sizeof(WORD);
				break;
			case 0xFFFF:
#ifdef PRINT_TRANSLATION_INFO
				dprintf(_T("class is %04X, "), *((UINT16*)pTemplateDataIn + sizeof(WORD)));
#endif
				pTemplateDataIn += 2 * sizeof(WORD);
				break;
			default:
#ifdef PRINT_TRANSLATION_INFO
				dprintf(_T("class is \"%ls\", "), (wchar_t*)pTemplateDataIn);
#endif
				pTemplateDataIn += wcslen((wchar_t*)pTemplateDataIn) * sizeof(wchar_t) + sizeof(wchar_t);
				break;
		}

//		pTemplateDataIn = (char*)(((UINT32)pTemplateDataIn + sizeof(WORD) - 1) & ~(sizeof(WORD) - 1));

		if (*((WORD*)pTemplateDataIn) == 0xFFFF) {
			pTemplateDataIn += 4;
		}

		CATCH_UP;
		if (pLocaliseInfo->pControlInfo[i] && pLocaliseInfo->pControlInfo[i]->szCaption[0]) {
			ADD_STRING(pLocaliseInfo->pControlInfo[i]->szCaption);
		} else {
			ADD_STRING((wchar_t*)pTemplateDataIn);
		}

#ifdef PRINT_TRANSLATION_INFO
		dprintf(_T("title is \"%ls\".\n"), (wchar_t*)pTemplateDataIn);
#endif
		pTemplateDataIn += wcslen((wchar_t*)pTemplateDataIn) * sizeof(wchar_t) + sizeof(wchar_t);
		pTemplateDataInSync = pTemplateDataIn;
#ifdef PRINT_TRANSLATION_INFO
		if (*((WORD*)pTemplateDataIn)) {
			dprintf(_T("         %i bytes of extra data present.\n"), *((WORD*)pTemplateDataIn));
		}
#endif
		pTemplateDataIn += sizeof(WORD) + *((WORD*)pTemplateDataIn);
	}

	CATCH_UP;

#ifdef PRINT_TRANSLATION_INFO
	dprintf(_T("Parsing translated template:\n"));

	if (((DLGTEMPLATEEX*)pTemplate)->signature == 0xFFFF) {
		ParseDlgTemplateEx((DLGTEMPLATEEX*)pTemplateDataOut);
	}

	dprintf(_T("Done.\n"));
#endif

	return (DLGTEMPLATE*)pTemplateDataOut;
}

INT_PTR FBADialogBox(HINSTANCE hInstance, LPTSTR lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc)
{
	// Try to load the translated dialog
	if (bLocalisationActive && hInstance == hAppInst && (UINT_PTR)lpTemplate < nMaxResources && FBAResourceInfo[(UINT_PTR)lpTemplate].pResourceTranslation) {
		return DialogBoxIndirect(hInstance, (LPCDLGTEMPLATE)FBAResourceInfo[(INT_PTR)lpTemplate].pResourceTranslation, hWndParent, lpDialogFunc);
	}

	// Localisation disabled or couldn't lock resource, so use normal function
	return DialogBox(hInstance, lpTemplate, hWndParent, lpDialogFunc);
}

HWND FBACreateDialog(HINSTANCE hInstance, LPCTSTR lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc)
{
	// Try to load the translated dialog
	if (bLocalisationActive && hInstance == hAppInst && (UINT_PTR)lpTemplate < nMaxResources && FBAResourceInfo[(UINT_PTR)lpTemplate].pResourceTranslation) {
		return CreateDialogIndirect(hInstance, (LPCDLGTEMPLATE)FBAResourceInfo[(INT_PTR)lpTemplate].pResourceTranslation, hWndParent, lpDialogFunc);
	}

	// Localisation disabled or couldn't lock resource, so use normal function
	return CreateDialog(hInstance, lpTemplate, hWndParent, lpDialogFunc);
}

// ----------------------------------------------------------------------------

#ifdef PRINT_TRANSLATION_INFO
void ParseMenuTemplate(const MENUTEMPLATE* pTemplate)
{
	// 26 is sizeof(DLGTEMPLATEEX)
	char* pTemplateDataIn = (char*)pTemplate + sizeof(MENUITEMTEMPLATEHEADER);

#if 0
	dprintf(_T("\n"));
	char* pData = (char*)pTemplate;
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
				unsigned int n = *pData;
				dprintf(_T("%02X"), n & 255);
				pData++;
			}
			dprintf(_T(" "));
		}
		dprintf(_T("\n"));
	}
	dprintf(_T("\n"));
#endif

	int i = 0;
	int bracketcnt = 0; // popup menu counter (BEGIN...END)
	bool bLastPopup = false, bLastItem = false;
	do {
		if (((MENUITEMTEMPLATE*)pTemplateDataIn)->mtOption & MF_POPUP) {
			bLastPopup = false;
			bLastItem = false;
			bracketcnt++;
			if (((MENUITEMTEMPLATE*)pTemplateDataIn)->mtOption & MF_END) {
				bLastPopup = true;
				bracketcnt--;
			}
			dprintf(_T("   popup %03i "), i);
			pTemplateDataIn += sizeof(WORD);
		} else {
			if (((MENUITEMTEMPLATE*)pTemplateDataIn)->mtOption & MF_END) {
				bLastItem = true;
				bracketcnt--;
			}
			dprintf(_T("   item %03i ID is %05i "), i, ((MENUITEMTEMPLATE*)pTemplateDataIn)->mtID);
			pTemplateDataIn += sizeof(WORD) * 2;
		}

		dprintf(_T("\"%ls\".\n"), ((wchar_t*)pTemplateDataIn));
		pTemplateDataIn += wcslen((wchar_t*)pTemplateDataIn) * sizeof(wchar_t) + sizeof(wchar_t);
	} while (i++ < 1024 && (!(bLastPopup && bLastItem) || bracketcnt > 0));
}
#endif

MENUTEMPLATE* TranslateMenuTemplate(const MENUTEMPLATE* pTemplate, const LocaliseResourceInfo* pLocaliseInfo)
{
	// 26 is sizeof(DLGTEMPLATEEX)
	char* pTemplateDataIn = (char*)pTemplate + sizeof(MENUITEMTEMPLATEHEADER);
	char* pTemplateDataOut = NULL;
	char* pTemplateDataInSync = NULL;
	char* pTemplateDataOutSync = NULL;

#ifdef PRINT_TRANSLATION_INFO
	dprintf(_T("Translating menu template:\n"));
#endif

	pTemplateDataOut = (char*)malloc(sizeof(MENUITEMTEMPLATEHEADER));
	memcpy(pTemplateDataOut, (char*)pTemplate, sizeof(MENUITEMTEMPLATEHEADER) + ((MENUITEMTEMPLATEHEADER*)pTemplate)->offset);
	pTemplateDataInSync = pTemplateDataIn;
	pTemplateDataOutSync = pTemplateDataOut + sizeof(MENUITEMTEMPLATEHEADER) + ((MENUITEMTEMPLATEHEADER*)pTemplate)->offset;

#ifdef PRINT_TRANSLATION_INFO
 #if 0
	dprintf(_T("\n"));
	char* pData = (char*)pTemplate;
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
				unsigned int n = *pData;
				dprintf(_T("%02X"), n & 255);
				pData++;
			}
			dprintf(_T(" "));
		}
		dprintf(_T("\n"));
	}
	dprintf(_T("\n"));
 #endif
#endif

	int i = 0;
	int bracketcnt = 0; // popup menu counter (BEGIN...END)
	bool bLastPopup = false, bLastItem = false;
	do {
		if (((MENUITEMTEMPLATE*)pTemplateDataIn)->mtOption & MF_POPUP) {
			bLastPopup = false;
			bLastItem = false;
			bracketcnt++;
			if (((MENUITEMTEMPLATE*)pTemplateDataIn)->mtOption & MF_END) {
				bLastPopup = true;
				bracketcnt--;
			}
#ifdef PRINT_TRANSLATION_INFO
			dprintf(_T("   popup %03i "), i);
#endif
			pTemplateDataIn += sizeof(WORD);
		} else {
			if (((MENUITEMTEMPLATE*)pTemplateDataIn)->mtOption & MF_END) {
				bLastItem = true;
				bracketcnt--;
			}
#ifdef PRINT_TRANSLATION_INFO
			dprintf(_T("   item %03i ID is %05i "), i, ((MENUITEMTEMPLATE*)pTemplateDataIn)->mtID);
#endif
			pTemplateDataIn += sizeof(WORD) * 2;
		}

		CATCH_UP;
#ifdef PRINT_TRANSLATION_INFO
		dprintf(_T("\"%ls\".\n"), (wchar_t*)pTemplateDataIn);
#endif
		if (pLocaliseInfo->pControlInfo[i] && pLocaliseInfo->pControlInfo[i]->szCaption[0]) {
			wchar_t* pszHotkey = NULL;

			for (pszHotkey = (wchar_t*)pTemplateDataIn; *pszHotkey && *pszHotkey != _T('\t'); pszHotkey++) { }

			ADD_STRING(pLocaliseInfo->pControlInfo[i]->szCaption);
			if (*pszHotkey) {
				pTemplateDataOutSync -= sizeof(wchar_t);
				ADD_STRING(pszHotkey);
			}
		} else {
			ADD_STRING((wchar_t*)pTemplateDataIn);
		}

		pTemplateDataIn += wcslen((wchar_t*)pTemplateDataIn) * sizeof(wchar_t) + sizeof(wchar_t);
		pTemplateDataInSync = pTemplateDataIn;
	} while (i++ < 1024 && (!(bLastPopup && bLastItem) || bracketcnt > 0));

	CATCH_UP;

#ifdef PRINT_TRANSLATION_INFO
	dprintf(_T("Parsing translated template:\n"));
	ParseMenuTemplate((MENUTEMPLATE*)pTemplateDataOut);
	dprintf(_T("Done.\n"));
#endif

	return (MENUTEMPLATE*)pTemplateDataOut;

}

HMENU FBALoadMenu(HINSTANCE hInstance, LPTSTR lpMenuName)
{
	// Try to load the translated menu
	if (bLocalisationActive && hInstance == hAppInst && (UINT_PTR)lpMenuName < nMaxResources && FBAResourceInfo[(UINT_PTR)lpMenuName].pResourceTranslation) {
		return LoadMenuIndirect((MENUTEMPLATE*)FBAResourceInfo[(INT_PTR)lpMenuName].pResourceTranslation);
	}

	// Translation unavailable, so use normal function
	return LoadMenu(hInstance, lpMenuName);
}

// ----------------------------------------------------------------------------

TCHAR* FBALoadStringEx(HINSTANCE hInstance, UINT uID, bool bTranslate)
{
	if (bLocalisationActive && bTranslate && uID < nMaxResources) {
		if (FBAResourceInfo[uID].pResourceTranslation) {

#if 1 && defined (PRINT_TRANSLATION_INFO)
			dprintf(_T("string %5i: \"%ls\"\n"), uID, (TCHAR*)FBAResourceInfo[uID].pResourceTranslation);
#endif

			return (TCHAR*)FBAResourceInfo[uID].pResourceTranslation;
		}
	}

	{
		// Convert the string ID into a bundle number
		wchar_t* pwsz = (wchar_t*)LoadResource(hInstance, FindResource(hInstance, MAKEINTRESOURCE(uID / 16 + 1), RT_STRING));

		if ((pwsz = (wchar_t*)LockResource(pwsz)) != NULL) {
			// Locate the string in the bundle
			for (unsigned int i = 0; i < (uID & 15); i++) {
				pwsz += *pwsz + 1;
			}
		}

#if 1 && defined (PRINT_TRANSLATION_INFO)
		dprintf(_T("string %5i: \"%ls\"\n"), uID, *pwsz ? pwsz + 1 : pwsz);
#endif

#if defined (UNICODE) && defined (_MSC_VER)
		return *pwsz ? pwsz + 1 : pwsz;
#else
		{
#if !defined (UNICODE)
			static char szStringBuffer[5120];
//			memset(szStringBuffer, 0, sizeof(szStringBuffer));

			if (WideCharToMultiByte(CP_ACP, 0, *pwsz ? pwsz + 1 : pwsz, *pwsz, szStringBuffer, 5120, NULL, NULL)) {
				szStringBuffer[*pwsz] = '\0';
				return szStringBuffer;
			}
#else
			static wchar_t szStringBuffer[5120];

			// When using GCC, copy the string since Windres can't zero-terminate strings in resource files
			if (*pwsz) {
				wcsncpy(szStringBuffer, pwsz + 1, *pwsz);
			}
			szStringBuffer[*pwsz] = '\0';
			return szStringBuffer;

#endif
			return NULL;
		}
#endif
	}
}

int FBALoadString(HINSTANCE hInstance, UINT uID, LPTSTR lpBuffer, int nBufferMax)
{
	if (bLocalisationActive && uID < nMaxResources) {
		if (FBAResourceInfo[uID].pResourceTranslation) {
			int nLen = _tcslen((TCHAR*)FBAResourceInfo[uID].pResourceTranslation);

#if defined (UNICODE)
			_tcsncpy(lpBuffer, (TCHAR*)FBAResourceInfo[uID].pResourceTranslation, nBufferMax - 1);
#else
			strncpy(lpBuffer, (char*)FBAResourceInfo[uID].pResourceTranslation, nBufferMax - 1);
#endif
			return ((nLen < nBufferMax) ? nLen : nBufferMax) - 1;
		}
	}

	return LoadString(hInstance, uID, lpBuffer, nBufferMax);
}

// ----------------------------------------------------------------------------

static void FBALocaliseError(TCHAR* pszFilename, int nLineNumber, TCHAR* pszInfo, wchar_t* pszLine)
{
	FBAPopupAddText(PUF_TEXT_NO_TRANSLATE, _T("Language file %s is malformed.\nPlease remove or repair the file.\n\n"), pszFilename);
	FBAPopupAddText(PUF_TEXT_NO_TRANSLATE, _T("Parse error at line %i.\n"), nLineNumber);

	if (pszInfo) {
		FBAPopupAddText(PUF_TEXT_NO_TRANSLATE, _T("Problem:\t%s.\n"), pszInfo);
	}
	if (pszLine) {
		FBAPopupAddText(PUF_TEXT_NO_TRANSLATE, _T("Text:\t%ls\n"), pszLine);
	}

	FBAPopupDisplay(PUF_TYPE_ERROR);
}

static int FBALocaliseParseFile(TCHAR* pszFilename)
{
	LocaliseResourceInfo CurrentResource;
	wchar_t wszBuffer[5120];

	int nTemplateVersion = 0;

	wchar_t szLine[5120];
	wchar_t* s;
	wchar_t* t;
	int nLen;

	int nLine = 0;
	int nInside = 0;
	TCHAR* nResourceType = 0;

	if (pszFilename == 0 || _tcslen(pszFilename) == 0) {
		return -1;
	}

	memset(&CurrentResource, 0, sizeof(LocaliseResourceInfo));

	FILE* h = _tfopen(pszFilename, _T("rb"));
	if (h == NULL) {
		return 1;
	}

	{
		unsigned char szBOM[4] = { 0, };

		fread(szBOM, 1, sizeof(szBOM), h);

		// See if it's a UTF-8 file
		if (szBOM[0] == 0xEF && szBOM[1] == 0xBB && szBOM[2] == 0xBF) {
			nFBACodepage = CP_UTF8;
		}

#ifdef _UNICODE
		// See if it's a UTF-16 file
		if (szBOM[0] == 0xFF && szBOM[1] == 0xFE) {
			nFBACodepage = CP_WINUNICODE;

			fseek(h, 2, SEEK_SET);
		}
#endif

	}

	if (nFBACodepage != CP_WINUNICODE) {
		fclose(h);
		h = _tfopen(pszFilename, _T("rt"));
		if (h == NULL) {
			return 1;
		}

		if (nFBACodepage == CP_UTF8) {
			fseek(h, 3, SEEK_SET);
		}

	}

	while (1) {
		char szTemp[1024];

#ifdef _UNICODE
		if (nFBACodepage != CP_WINUNICODE) {
#endif
			if (fgets(szTemp, sizeof(szTemp), h) == NULL) {
				break;
			}
			MultiByteToWideChar(nFBACodepage, 0, szTemp, -1, szLine, sizeof(szLine) / sizeof(TCHAR));
#ifdef _UNICODE
		} else {
			if (_fgetts(szLine, sizeof(szLine), h) == NULL) {
				break;
			}
		}
#endif

		nLine++;

		nLen = wcslen(szLine);
		// Get rid of the linefeed at the end
		while (szLine[nLen - 1] == 0x0A || szLine[nLen - 1] == 0x0D) {
			szLine[nLen - 1] = 0;
			nLen--;
		}

		s = szLine;													// Start parsing

		WSKIP_WS(s);

		if (s[0] == _T('/') && s[1] == _T('/')) {					// Comment
			continue;
		}

		if ((t = LabelCheckW(s, L"version")) != 0) {				// Version
			s = t;

			WSKIP_WS(s);

			nTemplateVersion = wcstol(s, &t, 0);

			if (s != t) {
				if (nTemplateVersion != nBurnVer) {
					break;
				}
			}

#ifdef PRINT_TRANSLATION_INFO
			dprintf(_T("version\n"), nTemplateVersion);
#endif

			s = t;

			continue;
		}

		if ((t = LabelCheckW(s, L"codepage")) != 0) {				// Set codepage
			s = t;

			WSKIP_WS(s);

			nFBACodepage = wcstol(s, &t, 0);

#ifdef PRINT_TRANSLATION_INFO
			dprintf(_T("codepage\n"), nFBACodepage);
#endif

			s = t;

			continue;
		}

		if ((t = LabelCheckW(s, L"menu")) != 0) {
			s = t;

			nResourceType = RT_MENU;

			WSKIP_WS(s);

			unsigned int nID = wcstol(s, &t, 0);

#ifdef PRINT_TRANSLATION_INFO
			dprintf(_T("menu %i\n"), nID);
#endif
			s = t;

			WSKIP_WS(s);

			if (nInside) {
				FBALocaliseError(pszFilename, nLine, _T("missing closing bracket"), NULL);
				break;
			}
			if (*s != _T('{')) {
				FBALocaliseError(pszFilename, nLine, _T("missing opening bracket"), NULL);
				break;
			}
			nInside = 1;

			CurrentResource.nID = nID;

			continue;
		}

		if ((t = LabelCheckW(s, L"dialog")) != 0) {				// Add new resource
			s = t;

			nResourceType = RT_DIALOG;

			WSKIP_WS(s);

			unsigned int nID = wcstol(s, &t, 0);

#ifdef PRINT_TRANSLATION_INFO
			dprintf(_T("dialog %i\n"), nID);
#endif
			s = t;

			// Read dialog caption
			wchar_t* szQuote = NULL;
			wchar_t* szEnd = NULL;

			QuoteReadW(&szQuote, &szEnd, s);

			s = szEnd;

			WSKIP_WS(s);

			if (nInside) {
				FBALocaliseError(pszFilename, nLine, _T("missing closing bracket"), NULL);
				break;
			}
			if (*s != L'{') {
				FBALocaliseError(pszFilename, nLine, _T("missing opening bracket"), NULL);
				break;
			}
			nInside = 1;

			CurrentResource.nID = nID;

			if (wcslen(szQuote)) {
				memcpy(CurrentResource.szCaption, szQuote, QUOTE_MAX * sizeof(TCHAR));
			}

			continue;
		}

		if ((t = LabelCheckW(s, L"string")) != 0) {
			s = t;

			if (nInside) {
				FBALocaliseError(pszFilename, nLine, _T("missing closing bracket"), NULL);
				break;
			}

			WSKIP_WS(s);

			unsigned int nID = wcstol(s, &t, 0);

			s = t;

			// Read dialog caption
			WFIND_QT(s);
			s++;
			wchar_t* szQuote = s;
			WFIND_QT(s);
			wchar_t* szEnd = s;

			if (nID < nMaxResources) {

#ifdef PRINT_TRANSLATION_INFO
				{
					TCHAR szFormat[256];
					_stprintf(szFormat, _T("string %%5i \"%%.%ils\"\n"), szEnd - szQuote);
					dprintf(szFormat, nID, szQuote);
				}
#endif

				if (szEnd - szQuote > 0) {
					UnEscapeString(wszBuffer, szQuote, szEnd - szQuote);
					FBAResourceInfo[nID].nResourceFlags = RES_DEALLOCATE;
#ifdef _UNICODE
					FBAResourceInfo[nID].pResourceTranslation = malloc((wcslen(wszBuffer) + 1) * sizeof(wchar_t));
					wcscpy((wchar_t*)FBAResourceInfo[nID].pResourceTranslation, wszBuffer);
#else
					{
						char szStringBuffer[5120];
						memset(szStringBuffer, 0, sizeof(szStringBuffer));

						WideCharToMultiByte(CP_ACP, 0, wszBuffer, -1, szStringBuffer, 5120, NULL, NULL);

						FBAResourceInfo[nID].pResourceTranslation = malloc((strlen(szStringBuffer) + 1));
						strcpy((char*)FBAResourceInfo[nID].pResourceTranslation, szStringBuffer);
					}
#endif
				}
			}
		}

		int n = wcstol(s, &t, 0);
		bool bPopup = false;
		if (t == s) {
			t = LabelCheckW(s, L"popup");
			bPopup = true;
		}
		if (t && t != s) {				   							// New control

			if (nInside == 0) {
				FBALocaliseError(pszFilename, nLine, _T("rogue control statement"), szLine);
				break;
			}

			s = t;
			n = wcstol(s, &t, 0);

			// Link a new control info structure
			if (n < 256) {
				s = t;

				// Read option name
				wchar_t* szQuote = NULL;
				wchar_t* szEnd = NULL;
				if (QuoteReadW(&szQuote, &szEnd, s)) {
					FBALocaliseError(pszFilename, nLine, _T("control name omitted"), szLine);
					break;
				}
				s = szEnd;

				if (bPopup) {
					WSKIP_WS(s);

					if (*s != L'{') {
						FBALocaliseError(pszFilename, nLine, _T("missing opening bracket"), NULL);
						break;
					}
					nInside++;
				}

				if (wcslen(szQuote)) {
					if (CurrentResource.pControlInfo[n] == NULL) {
						CurrentResource.pControlInfo[n] = (LocaliseControlInfo*)malloc(sizeof(LocaliseControlInfo));
					}
					memset(CurrentResource.pControlInfo[n], 0, sizeof(LocaliseControlInfo));
					memcpy(CurrentResource.pControlInfo[n]->szCaption, szQuote, QUOTE_MAX * sizeof(TCHAR));
				}

//				dprintf(_T("   - %ls\n"), pCurrentResource->pControlInfo[n]->szCaption);

			}

			continue;
		}

		WSKIP_WS(s);
		if (*s == L'}') {
			if (nInside == 0) {
				FBALocaliseError(pszFilename, nLine, _T("rogue closing bracket"), NULL);
				break;
			}

			nInside--;

			if (nInside == 0) {

				if (CurrentResource.nID < nMaxResources) {
					if (nResourceType == RT_MENU) {
						MENUTEMPLATE* pTemplate;

						pTemplate = (MENUTEMPLATE*)LoadResource(hAppInst, FindResource(hAppInst, MAKEINTRESOURCE(CurrentResource.nID), RT_MENU));
						if (LockResource((HGLOBAL)pTemplate)) {
							if (((MENUITEMTEMPLATEHEADER*)pTemplate)->versionNumber == 0) {

								// Translate the structure
								FBAResourceInfo[CurrentResource.nID].pResourceTranslation = TranslateMenuTemplate((MENUTEMPLATE*)pTemplate, &CurrentResource);
								FBAResourceInfo[CurrentResource.nID].nResourceFlags = RES_DEALLOCATE;
							}
						}
					}
					if (nResourceType == RT_DIALOG) {
						LPCDLGTEMPLATE pTemplate;

						pTemplate = (LPCDLGTEMPLATE)LoadResource(hAppInst, FindResource(hAppInst, MAKEINTRESOURCE(CurrentResource.nID), RT_DIALOG));
						if (LockResource((HGLOBAL)pTemplate)) {
							if (((DLGTEMPLATEEX*)pTemplate)->signature == 0xFFFF && ((DLGTEMPLATEEX*)pTemplate)->dlgVer == 1) {

								// Translate the structure
								FBAResourceInfo[CurrentResource.nID].pResourceTranslation = TranslateDlgTemplateEx((DLGTEMPLATEEX*)pTemplate, &CurrentResource);
								FBAResourceInfo[CurrentResource.nID].nResourceFlags = RES_DEALLOCATE;
							}
						}
					}
				}

				for (int i = 0; i < 1024; i++) {
					if (CurrentResource.pControlInfo[i]) {
						free(CurrentResource.pControlInfo[i]);
						CurrentResource.pControlInfo[i] = NULL;
					}
				}
				memset(&CurrentResource, 0, sizeof(LocaliseResourceInfo));
			}
		}

		// Line isn't (part of) a valid cheat
#if 0
		if (*s) {
			FBALocaliseError(pszFilename, nLine, _T("rogue line"), szLine);
			break;
		}
#endif

	}

	for (int i = 0; i < 1024; i++) {
		if (CurrentResource.pControlInfo[i]) {
			free(CurrentResource.pControlInfo[i]);
			CurrentResource.pControlInfo[i] = NULL;
		}
	}

	if (h) {
		fclose(h);
	}

	if (nTemplateVersion != nBurnVer) {
		if (nTemplateVersion == 0) {
			return -1;
		}
		return -2;
	}

	return 0;
}

void FBALocaliseExit()
{
	bLocalisationActive = false;

	if (FBAResourceInfo) {
		for (unsigned int i = 0; i < nMaxResources; i++) {
			if (FBAResourceInfo[i].nResourceFlags & RES_DEALLOCATE) {
				if (FBAResourceInfo[i].pResourceTranslation) {
					free(FBAResourceInfo[i].pResourceTranslation);
					FBAResourceInfo[i].pResourceTranslation = NULL;
				}
			}
		}

		if (FBAResourceInfo) {
			free(FBAResourceInfo);
			FBAResourceInfo = NULL;
		}
	}

	return;
}

int FBALocaliseInit(TCHAR* pszTemplate)
{
	int nRet;

	FBALocaliseExit();
	nFBACodepage = GetACP();

	if (pszTemplate == NULL || _tcslen(pszTemplate) == 0) {

#ifdef PRINT_DEBUG_INFO
		dprintf(_T(" ** Translation disabled\n"));
#endif

		szLocalisationTemplate[0] = _T('\0');

		return 0;
	}

	FBAResourceInfo = (FBAResourceInfo_t*)malloc(nMaxResources * sizeof(FBAResourceInfo_t));
	if (FBAResourceInfo == NULL) {
		return 1;
	}

	memset(FBAResourceInfo, 0, nMaxResources * sizeof(FBAResourceInfo_t));

	nRet = FBALocaliseParseFile(pszTemplate);
	if (nRet > 0) {

#ifdef PRINT_DEBUG_INFO
		dprintf(_T(" ** Translation initialisation failed\n"));
#endif

		return 1;
	}

	if (nRet < 0) {

#ifdef PRINT_DEBUG_INFO
		dprintf(_T(" ** Translation disabled\n"));
#endif

		FBALocaliseExit();
		nFBACodepage = GetACP();

		szLocalisationTemplate[0] = _T('\0');

		return 0;
	}

#ifdef PRINT_DEBUG_INFO
	dprintf(_T(" ** Translation initialised\n"));
#endif

	if (pszTemplate) {
		_tcsncpy(szLocalisationTemplate, pszTemplate, sizeof(szLocalisationTemplate) / sizeof(TCHAR));
	}

	bLocalisationActive = true;

	return 0;
}

// ----------------------------------------------------------------------------
// Dialog box to load/save a template

static TCHAR szFilter[100];

static void MakeOfn()
{
	_stprintf(szFilter, _T("%s"), FBALoadStringEx(hAppInst, IDS_LOCAL_FILTER, true));
	memcpy(szFilter + _tcslen(szFilter), _T(" (*.flt)\0*.flt\0\0"), 16 * sizeof(TCHAR));
			
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hScrnWnd;
	ofn.lpstrFilter = szFilter;
	ofn.lpstrFile = szChoice;
	ofn.nMaxFile = sizeof(szChoice) / sizeof(TCHAR);
	ofn.lpstrInitialDir = _T(".\\config\\localisation");
	ofn.Flags = OFN_NOCHANGEDIR | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = _T("flt");

	return;
}

int FBALocaliseLoadTemplate()
{
	_stprintf(szChoice, _T("template"));
	MakeOfn();
	TCHAR szTitle[100];
	_stprintf(szTitle, _T("%s"), FBALoadStringEx(hAppInst, IDS_LOCAL_SELECT, true));
	ofn.lpstrTitle = szTitle;
	ofn.Flags |= OFN_OVERWRITEPROMPT;

	int bOldPause = bRunPause;
	bRunPause = 1;
	int nRet = GetOpenFileName(&ofn);
	bRunPause = bOldPause;

	if (nRet == 0) {
		return 1;
	}

	return FBALocaliseInit(szChoice);
}

int FBALocaliseCreateTemplate()
{
	_stprintf(szChoice, _T("template"));
	MakeOfn();
	TCHAR szTitle[100];
	_stprintf(szTitle, _T("%s"), FBALoadStringEx(hAppInst, IDS_LOCAL_CREATE, true));
	ofn.lpstrTitle = szTitle;
	ofn.Flags |= OFN_OVERWRITEPROMPT;

	int bOldPause = bRunPause;
	bRunPause = 1;
	int nRet = GetSaveFileName(&ofn);
	bRunPause = bOldPause;

	if (nRet == 0) {
		return 1;
	}

	return FBALocaliseWriteTemplate(szChoice);
}

#include "burner.h"

bool bIsWindowsXPorGreater = FALSE;

// Detect if we are using Windows XP/Vista/7
BOOL DetectWindowsVersion()
{
    OSVERSIONINFO osvi;
    BOOL bIsWindowsXPorLater;

    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&osvi);
    
	// osvi.dwMajorVersion returns the windows version: 5 = XP 6 = Vista/7
    // osvi.dwMinorVersion returns the minor version, XP and 7 = 1, Vista = 0
    bIsWindowsXPorLater = ((osvi.dwMajorVersion > 5) || ( (osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion >= 1)));
    
    return bIsWindowsXPorLater; 
}

// Set the current directory to be the application's directory
int AppDirectory()
{
	TCHAR szPath[MAX_PATH] = _T("");
	int nLen = 0;
	TCHAR *pc1, *pc2;
	TCHAR* szCmd = GetCommandLine();

	// Find the end of the "c:\directory\program.exe" bit
	if (szCmd[0] == _T('\"')) {						// Filename is enclosed in quotes
		szCmd++;
		for (pc1 = szCmd; *pc1; pc1++) {
			if (*pc1 == _T('\"')) break;			// Find the last "
		}
	} else {
		for (pc1 = szCmd; *pc1; pc1++) {
			if (*pc1 == _T(' ')) break;				// Find the first space
		}
	}
	// Find the last \ or /
	for (pc2 = pc1; pc2 >= szCmd; pc2--) {
		if (*pc2 == _T('\\')) break;
		if (*pc2 == _T('/')) break;
	}

	// Copy the name of the executable into a variable
	nLen = pc1 - pc2 - 1;
	if (nLen > EXE_NAME_SIZE) {
		nLen = EXE_NAME_SIZE;
	}
	_tcsncpy(szAppExeName, pc2 + 1, nLen);
	szAppExeName[nLen] = 0;

	// strip .exe
	if ((pc1 = _tcschr(szAppExeName, _T('.'))) != 0) {
		*pc1 = 0;
	}

	nLen = pc2 - szCmd;
	if (nLen <= 0) return 1;			// No path

	// Now copy the path into a new buffer
	_tcsncpy(szPath, szCmd, nLen);
	SetCurrentDirectory(szPath);		// Finally set the current directory to be the application's directory

	dprintf(szPath);
	dprintf(_T("\n"));

	return 0;
}

void UpdatePath(TCHAR* path)
{
	int pathlen = _tcslen(path);
	if (pathlen) {
		DWORD attrib = INVALID_FILE_ATTRIBUTES;
		TCHAR curdir[MAX_PATH] = _T("");
		int curlen = 0;

		attrib = GetFileAttributes(path);
		if (attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY) && path[pathlen - 1] != _T('\\')) {
			path[pathlen] = _T('\\');
			path[pathlen + 1] = _T('\0');

			pathlen++;
		}

		GetCurrentDirectory(sizeof(curdir), curdir);
		curlen = _tcslen(curdir);

		if (_tcsnicmp(curdir, path, curlen) == 0 && path[curlen] == _T('\\')) {
			TCHAR newpath[MAX_PATH];

			_tcscpy(newpath, path + curlen + 1);
			_tcscpy(path, newpath);
		}
	}
}

// ---------------------------------------------------------------------------

static void MyRegCreateKeys(int nDepth, TCHAR* pNames[], HKEY* pResult)
{
	for (int i = 0; i < nDepth; i++) {
		pResult[i] = NULL;
		RegCreateKeyEx((i ? pResult[i - 1] : HKEY_CLASSES_ROOT), pNames[i], 0, _T(""), 0, KEY_WRITE, NULL, &pResult[i], NULL);
	}
}

static void MyRegCloseKeys(int nDepth, HKEY* pKeys)
{
	for (int i = nDepth - 1; i >= 0; i--) {
		if (pKeys[i]) {
			RegCloseKey(pKeys[i]);
		}
	}
}

static void MyRegDeleteKeys(int nDepth, TCHAR* pNames[], HKEY* pKeys)
{
	for (int i = 0; i < nDepth - 1; i++) {
		pKeys[i] = NULL;
		RegOpenKeyEx((i ? pKeys[i - 1] : HKEY_CLASSES_ROOT), pNames[i], 0, 0, &pKeys[i]);
	}
	for (int i = nDepth - 1; i >= 0; i--) {
		RegDeleteKey((i ? pKeys[i - 1] : HKEY_CLASSES_ROOT), pNames[i]);
		if (i) {
			RegCloseKey(pKeys[i - 1]);
		}
	}
}

void RegisterExtensions(bool bCreateKeys)
{
	HKEY myKeys[4];

	TCHAR* myKeynames1[1] = { _T(".fr") };
	TCHAR* myKeynames2[1] = { _T(".fs") };
	TCHAR* myKeynames3[4] = { _T("FBAlpha"), _T("shell"), _T("open"), _T("command") };
	TCHAR* myKeynames4[2] = { _T("FBAlpha"), _T("DefaultIcon") };
	TCHAR myKeyValue[MAX_PATH + 32] = _T("");

	if (bCreateKeys) {
		TCHAR szExename[MAX_PATH] = _T("");
		GetModuleFileName(NULL, szExename, MAX_PATH);

		MyRegCreateKeys(1, myKeynames1, myKeys);
		_stprintf(myKeyValue, _T("FBAlpha"));
		RegSetValueEx(myKeys[0], NULL, 0, REG_SZ, (BYTE*)myKeyValue, (_tcslen(myKeyValue) + 1) * sizeof(TCHAR));
		MyRegCloseKeys(2, myKeys);

		MyRegCreateKeys(1, myKeynames2, myKeys);
		_stprintf(myKeyValue, _T("FBAlpha"));
		RegSetValueEx(myKeys[0], NULL, 0, REG_SZ, (BYTE*)myKeyValue, (_tcslen(myKeyValue) + 1) * sizeof(TCHAR));
		MyRegCloseKeys(2, myKeys);

		MyRegCreateKeys(4, myKeynames3, myKeys);
		_stprintf(myKeyValue, _T("\"%s\" \"%%1\" -w"), szExename);
		RegSetValueEx(myKeys[3], NULL, 0, REG_SZ, (BYTE*)myKeyValue, (_tcslen(myKeyValue) + 1) * sizeof(TCHAR));
		_stprintf(myKeyValue, _T("FB Alpha file"));
		RegSetValueEx(myKeys[0], NULL, 0, REG_SZ, (BYTE*)myKeyValue, (_tcslen(myKeyValue) + 1) * sizeof(TCHAR));
		MyRegCloseKeys(4, myKeys);

		MyRegCreateKeys(2, myKeynames4, myKeys);
		_stprintf(myKeyValue, _T("\"%s\", 0"), szExename);
		RegSetValueEx(myKeys[1], NULL, 0, REG_SZ, (BYTE*)myKeyValue, (_tcslen(myKeyValue) + 1) * sizeof(TCHAR));
		MyRegCloseKeys(2, myKeys);
	} else {
		MyRegDeleteKeys(2, myKeynames4, myKeys);
		MyRegDeleteKeys(4, myKeynames3, myKeys);
		MyRegDeleteKeys(1, myKeynames2, myKeys);
		MyRegDeleteKeys(1, myKeynames1, myKeys);
	}

	return;
}

// ---------------------------------------------------------------------------

// Get the position of the client area of a window on the screen
int GetClientScreenRect(HWND hWnd, RECT *pRect)
{
	POINT Corner = {0, 0};

	GetClientRect(hWnd, pRect);
	ClientToScreen(hWnd, &Corner);

	pRect->left += Corner.x;
	pRect->right += Corner.x;
	pRect->top += Corner.y;
	pRect->bottom += Corner.y;

	return 0;
}

// Put a window in the middle of another window
int WndInMid(HWND hMid, HWND hBase)
{
	RECT MidRect = {0, 0, 0, 0};
	int mw = 0, mh = 0;
	RECT BaseRect = {0, 0, 0, 0};
	int bx = 0, by = 0;

	// Find the height and width of the Mid window
	GetWindowRect(hMid, &MidRect);
	mw = MidRect.right - MidRect.left;
	mh = MidRect.bottom - MidRect.top;

	// Find the center of the Base window
	if (hBase && IsWindowVisible(hBase)) {
		GetWindowRect(hBase, &BaseRect);
		if (hBase == hScrnWnd) {
			// For the main window, center in the client area.
			BaseRect.left += GetSystemMetrics(SM_CXSIZEFRAME);
			BaseRect.right -= GetSystemMetrics(SM_CXSIZEFRAME);
			BaseRect.top += GetSystemMetrics(SM_CYSIZEFRAME);
			if (bMenuEnabled) {
				BaseRect.top += GetSystemMetrics(SM_CYCAPTION) + nMenuHeight;
			}
			BaseRect.bottom -= GetSystemMetrics(SM_CYSIZEFRAME);
		}
	} else {
		SystemParametersInfo(SPI_GETWORKAREA, 0, &BaseRect, 0);
	}

	bx = BaseRect.left + BaseRect.right;
	bx = (bx - mw) >> 1;
	by = BaseRect.top + BaseRect.bottom;
	by = (by - mh) >> 1;

	if (hBase) {
		SystemParametersInfo(SPI_GETWORKAREA, 0, &SystemWorkArea, 0);

		if (bx + mw > SystemWorkArea.right) {
			bx = SystemWorkArea.right - mw;
		}
		if (by + mh > SystemWorkArea.bottom) {
			by = SystemWorkArea.bottom - mh;
		}
		if (bx < SystemWorkArea.left) {
			bx = SystemWorkArea.left;
		}
		if (by < SystemWorkArea.top) {
			by = SystemWorkArea.top;
		}
	}

	// Center the window
	SetWindowPos(hMid, NULL, bx, by, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	return 0;
}

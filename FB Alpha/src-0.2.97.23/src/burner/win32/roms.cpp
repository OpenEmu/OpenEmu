#include "burner.h"
#include <shlobj.h>
#include <process.h>

static HWND hTabControl = NULL;

HWND hRomsDlg = NULL;
static HWND hParent = NULL;

char* gameAv = NULL;
bool avOk = false;

static unsigned ScanThreadId = 0;
static HANDLE hScanThread = NULL;
static int nOldSelect = 0;

static HANDLE hEvent = NULL;

static void CreateRomDatName(TCHAR* szRomDat)
{
	_stprintf(szRomDat, _T("config/%s.roms.dat"), szAppExeName);

	return;
}

//Select Directory Dialog//////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK DefInpProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	int var;
	static bool chOk;

	switch (Msg) {
		case WM_INITDIALOG: {
			chOk = false;

			// Setup edit controls values (ROMs Paths)
			for(int x = 0; x < 20; x++) {
				SetDlgItemText(hDlg, IDC_ROMSDIR_EDIT1 + x, szAppRomPaths[x]);
			}

			// Setup the tabs
			hTabControl = GetDlgItem(hDlg, IDC_ROMPATH_TAB);

			TC_ITEM tcItem; 
			tcItem.mask = TCIF_TEXT;

			UINT idsString[20] = { IDS_ROMPATH_1,IDS_ROMPATH_2,IDS_ROMPATH_3,IDS_ROMPATH_4,IDS_ROMPATH_5,IDS_ROMPATH_6,IDS_ROMPATH_7,IDS_ROMPATH_8,IDS_ROMPATH_9,IDS_ROMPATH_10,IDS_ROMPATH_11,IDS_ROMPATH_12,IDS_ROMPATH_13,IDS_ROMPATH_14,IDS_ROMPATH_15,IDS_ROMPATH_16,IDS_ROMPATH_17,IDS_ROMPATH_18,IDS_ROMPATH_19,IDS_ROMPATH_20 };

			for(int nIndex = 0; nIndex < 20; nIndex++) {
				tcItem.pszText = FBALoadStringEx(hAppInst, idsString[nIndex], true);
				TabCtrl_InsertItem(hTabControl, nIndex, &tcItem);
			}

			int TabPage = TabCtrl_GetCurSel(hTabControl);
			
			// hide all controls excluding the selected controls
			for(int x = 0; x < 20; x++) {
				if(x != TabPage) {
					ShowWindow(GetDlgItem(hDlg, IDC_ROMSDIR_BR1 + x), SW_HIDE);		// browse buttons
					ShowWindow(GetDlgItem(hDlg, IDC_ROMSDIR_EDIT1 + x), SW_HIDE);	// edit controls
				}
			}

			// Show the proper controls
			ShowWindow(GetDlgItem(hDlg, IDC_ROMSDIR_BR1 + TabPage), SW_SHOW);		// browse buttons
			ShowWindow(GetDlgItem(hDlg, IDC_ROMSDIR_EDIT1 + TabPage), SW_SHOW);		// edit controls

			UpdateWindow(hDlg);

			WndInMid(hDlg, hParent);
			SetFocus(hDlg);											// Enable Esc=close
			break;
		}
		case WM_NOTIFY: {
			NMHDR* pNmHdr = (NMHDR*)lParam;

			if (pNmHdr->code == TCN_SELCHANGE) {

				int TabPage = TabCtrl_GetCurSel(hTabControl);
				
				// hide all controls excluding the selected controls
				for(int x = 0; x < 20; x++) {
					if(x != TabPage) {
						ShowWindow(GetDlgItem(hDlg, IDC_ROMSDIR_BR1 + x), SW_HIDE);		// browse buttons
						ShowWindow(GetDlgItem(hDlg, IDC_ROMSDIR_EDIT1 + x), SW_HIDE);	// edit controls
					}
				}

				// Show the proper controls
				ShowWindow(GetDlgItem(hDlg, IDC_ROMSDIR_BR1 + TabPage), SW_SHOW);		// browse buttons
				ShowWindow(GetDlgItem(hDlg, IDC_ROMSDIR_EDIT1 + TabPage), SW_SHOW);		// edit controls
				
				UpdateWindow(hDlg);

				return FALSE;
			}
			break;
		}
		case WM_COMMAND: {
			LPMALLOC pMalloc = NULL;
			BROWSEINFO bInfo;
			ITEMIDLIST* pItemIDList = NULL;
			TCHAR buffer[MAX_PATH];

			if (LOWORD(wParam) == IDOK) {

				for (int i = 0; i < 20; i++) {
//					if (GetDlgItemText(hDlg, IDC_ROMSDIR_EDIT1 + i, buffer, sizeof(buffer)) && lstrcmp(szAppRomPaths[i], buffer)) {
					GetDlgItemText(hDlg, IDC_ROMSDIR_EDIT1 + i, buffer, sizeof(buffer));
					if (lstrcmp(szAppRomPaths[i], buffer)) chOk = true;
					lstrcpy(szAppRomPaths[i], buffer);
//					}
				}

				SendMessage(hDlg, WM_CLOSE, 0, 0);
				break;
			} else {
				if (LOWORD(wParam) >= IDC_ROMSDIR_BR1 && LOWORD(wParam) <= IDC_ROMSDIR_BR20) {
					var = IDC_ROMSDIR_EDIT1 + LOWORD(wParam) - IDC_ROMSDIR_BR1;
				} else {
					if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDCANCEL) {
						SendMessage(hDlg, WM_CLOSE, 0, 0);
					}
					break;
				}
			}

		    SHGetMalloc(&pMalloc);

			memset(&bInfo, 0, sizeof(bInfo));
			bInfo.hwndOwner = hDlg;
			bInfo.pszDisplayName = buffer;
			bInfo.lpszTitle = FBALoadStringEx(hAppInst, IDS_ROMS_SELECT_DIR, true);
			bInfo.ulFlags = BIF_EDITBOX | BIF_RETURNONLYFSDIRS;

			pItemIDList = SHBrowseForFolder(&bInfo);

			if (pItemIDList) {
				if (SHGetPathFromIDList(pItemIDList, buffer)) {
					int strLen = _tcslen(buffer);
					if (strLen) {
						if (buffer[strLen - 1] != _T('\\')) {
							buffer[strLen] = _T('\\');
							buffer[strLen + 1] = _T('\0');
						}
						SetDlgItemText(hDlg, var, buffer);
					}
				}
				pMalloc->Free(pItemIDList);
			}
			pMalloc->Release();

			break;
		}
		case WM_CLOSE: {
			hParent = NULL;
			EndDialog(hDlg, 0);
			if (chOk) {
				bRescanRoms = true;
				CreateROMInfo(hDlg);
			}
		}
	}

	return 0;
}


int RomsDirCreate(HWND hParentWND)
{
	hParent = hParentWND;
	
	FBADialogBox(hAppInst, MAKEINTRESOURCE(IDD_ROMSDIR), hParent, (DLGPROC)DefInpProc);
	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//Check Romsets Dialog/////////////////////////////////////////////////////////////////////////////

static int WriteGameAvb()
{
	TCHAR szRomDat[MAX_PATH];
	FILE* h;

	CreateRomDatName(szRomDat);

	if ((h = _tfopen(szRomDat, _T("wt"))) == NULL) {
		return 1;
	}

	_ftprintf(h, _T(APP_TITLE) _T(" v%.20s ROMs"), szAppBurnVer);	// identifier
	_ftprintf(h, _T(" 0x%04X "), nBurnDrvCount);					// no of games

	for (unsigned int i = 0; i < nBurnDrvCount; i++) {
		if (gameAv[i] & 2) {
			_fputtc(_T('*'), h);
		} else {
			if (gameAv[i] & 1) {
				_fputtc(_T('+'), h);
			} else {
				_fputtc(_T('-'), h);
			}
		}
	}

	_ftprintf(h, _T(" END"));									// end marker

	fclose(h);

	return 0;
}

static int DoCheck(TCHAR* buffPos)
{
	TCHAR label[256];

	// Check identifier
	memset(label, 0, sizeof(label));
	_stprintf(label, _T(APP_TITLE) _T(" v%.20s ROMs"), szAppBurnVer);
	if ((buffPos = LabelCheck(buffPos, label)) == NULL) {
		return 1;
	}

	// Check no of supported games
	memset(label, 0, sizeof(label));
	memcpy(label, buffPos, 16);
	buffPos += 8;
	unsigned int n = _tcstol(label, NULL, 0);
	if (n != nBurnDrvCount) {
		return 1;
	}

	for (unsigned int i = 0; i < nBurnDrvCount; i++) {
		if (*buffPos == _T('*')) {
			gameAv[i] = 3;
		} else {
			if (*buffPos == _T('+')) {
				gameAv[i] = 1;
			} else {
				if (*buffPos == _T('-')) {
					gameAv[i] = 0;
				} else {
					return 1;
				}
			}
		}

		buffPos++;
	}

	memset(label, 0, sizeof(label));
	_stprintf(label, _T(" END"));
	if (LabelCheck(buffPos, label) == NULL) {
		avOk = true;
		return 0;
	} else {
		return 1;
	}
}

int CheckGameAvb()
{
	TCHAR szRomDat[MAX_PATH];
	FILE* h;
	int bOK;
	int nBufferSize = nBurnDrvCount + 256;
	TCHAR* buffer = (TCHAR*)malloc(nBufferSize * sizeof(TCHAR));
	if (buffer == NULL) {
		return 1;
	}

	memset(buffer, 0, nBufferSize * sizeof(TCHAR));
	CreateRomDatName(szRomDat);

	if ((h = _tfopen(szRomDat, _T("r"))) == NULL) {
		if (buffer) 
		{
			free(buffer);
			buffer = NULL;
		}
		return 1;
	}

	_fgetts(buffer, nBufferSize, h);
	fclose(h);

	bOK = DoCheck(buffer);

	if (buffer) {
		free(buffer);
		buffer = NULL;
	}
	return bOK;
}

static int QuitRomsScan()
{
	DWORD dwExitCode;

	GetExitCodeThread(hScanThread, &dwExitCode);

	if (dwExitCode == STILL_ACTIVE) {

		// Signal the scan thread to abort
		SetEvent(hEvent);

		// Wait for the thread to finish
		if (WaitForSingleObject(hScanThread, 10000) != WAIT_OBJECT_0) {
			// If the thread doesn't finish within 10 seconds, forcibly kill it
			TerminateThread(hScanThread, 1);
		}

		CloseHandle(hScanThread);
	}

	CloseHandle(hEvent);

	hEvent = NULL;

	hScanThread = NULL;
	ScanThreadId = 0;

	BzipClose();

	nBurnDrvActive = nOldSelect;
	nOldSelect = 0;
	bRescanRoms = false;

	if (avOk) {
		WriteGameAvb();
	}

	return 1;
}

static unsigned __stdcall AnalyzingRoms(void*)
{
	for (unsigned int z = 0; z < nBurnDrvCount; z++) {
		nBurnDrvActive = z;

		// See if we need to abort
		if (WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0) {
			ExitThread(0);
		}

		SendDlgItemMessage(hRomsDlg, IDC_WAIT_PROG, PBM_STEPIT, 0, 0);

		switch (BzipOpen(TRUE))	{
			case 0:
				gameAv[z] = 3;
				break;
			case 2:
				gameAv[z] = 1;
				break;
			case 1:
				gameAv[z] = 0;
		}
		BzipClose();
   }

	avOk = true;

	PostMessage(hRomsDlg, WM_CLOSE, 0, 0);

	return 0;
}

static INT_PTR CALLBACK WaitProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM)		// LPARAM lParam
{
	switch (Msg) {
		case WM_INITDIALOG:
			hRomsDlg = hDlg;
			nOldSelect = nBurnDrvActive;
			memset(gameAv, 0, sizeof(gameAv));
			SendDlgItemMessage(hDlg, IDC_WAIT_PROG, PBM_SETRANGE, 0, MAKELPARAM(0, nBurnDrvCount));
			SendDlgItemMessage(hDlg, IDC_WAIT_PROG, PBM_SETSTEP, (WPARAM)1, 0);

			ShowWindow(GetDlgItem(hDlg, IDC_WAIT_LABEL_A), TRUE);
			SendMessage(GetDlgItem(hDlg, IDC_WAIT_LABEL_A), WM_SETTEXT, (WPARAM)0, (LPARAM)FBALoadStringEx(hAppInst, IDS_SCANNING_ROMS, true));
			ShowWindow(GetDlgItem(hDlg, IDCANCEL), TRUE);

			avOk = false;
			hScanThread = (HANDLE)_beginthreadex(NULL, 0, AnalyzingRoms, NULL, 0, &ScanThreadId);

			hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			
			if (hParent == NULL) {
				RECT rect;
				int x, y;

				SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);

				x = 315 + GetSystemMetrics(SM_CXDLGFRAME) * 2 + 6;
				y = 74 + GetSystemMetrics(SM_CYDLGFRAME) * 2 + 6;

				SetForegroundWindow(hDlg);
				SetWindowPos(hDlg, HWND_TOPMOST, (rect.right - rect.left) / 2 - x / 2, (rect.bottom - rect.top) / 2 - y / 2, x, y, 0);
				RedrawWindow(hDlg, NULL, NULL, 0);
				ShowWindow(hDlg, SW_SHOWNORMAL);
			} else {
				WndInMid(hDlg, hParent);
				SetFocus(hDlg);		// Enable Esc=close
			}

			break;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDCANCEL) {
				PostMessage(hDlg, WM_CLOSE, 0, 0);
			}
			break;

		case WM_CLOSE:
			QuitRomsScan();
			EndDialog(hDlg, 0);
			hRomsDlg = NULL;
			hParent = NULL;

	}

	return 0;
}

int CreateROMInfo(HWND hParentWND)
{
	hParent = hParentWND;
	
	if (gameAv == NULL) {
		gameAv = (char*)malloc(nBurnDrvCount);
		memset(gameAv, 0, nBurnDrvCount);
	}

	if (gameAv) {
		if (CheckGameAvb() || bRescanRoms) {
			FBADialogBox(hAppInst, MAKEINTRESOURCE(IDD_WAIT), hParent, (DLGPROC)WaitProc);
		}
	}

	return 1;
}

void FreeROMInfo()
{
	if (gameAv) {
		free(gameAv);
		gameAv = NULL;
	}
}

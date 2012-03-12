#include "burner.h"
#include <shlobj.h>

static HWND hTabControl 	= NULL;
static int nInitTabSelect 	= 0;
static HWND hParent			= NULL;

TCHAR szAppPreviewsPath[MAX_PATH]	= _T("support/previews/");
TCHAR szAppTitlesPath[MAX_PATH]		= _T("support/titles/");
TCHAR szAppCheatsPath[MAX_PATH]		= _T("support/cheats/");
TCHAR szAppHiscorePath[MAX_PATH]	= _T("support/hiscores/");
TCHAR szAppSamplesPath[MAX_PATH]	= _T("support/samples/");
TCHAR szAppIpsPath[MAX_PATH]		= _T("support/ips/");
TCHAR szAppIconsPath[MAX_PATH]		= _T("support/icons/");

static TCHAR szCheckIconsPath[MAX_PATH];

static void IconsDirPathChanged() {
	if(bEnableIcons && bIconsLoaded) {
		// unload icons
		UnloadDrvIcons();
		bIconsLoaded = 0;
		// load icons
		LoadDrvIcons();
		bIconsLoaded = 1;
	}
	if(bEnableIcons && !bIconsLoaded) {
		// load icons
		LoadDrvIcons();
		bIconsLoaded = 1;
	}	
}

static INT_PTR CALLBACK DefInpProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	int var;

	switch (Msg) {
		case WM_INITDIALOG: {
			SetDlgItemText(hDlg, IDC_SUPPORTDIR_EDIT1, szAppPreviewsPath);
			SetDlgItemText(hDlg, IDC_SUPPORTDIR_EDIT2, szAppTitlesPath);
			SetDlgItemText(hDlg, IDC_SUPPORTDIR_EDIT3, szAppIconsPath);
			SetDlgItemText(hDlg, IDC_SUPPORTDIR_EDIT4, szAppCheatsPath);
			SetDlgItemText(hDlg, IDC_SUPPORTDIR_EDIT5, szAppHiscorePath);
			SetDlgItemText(hDlg, IDC_SUPPORTDIR_EDIT6, szAppSamplesPath);
			SetDlgItemText(hDlg, IDC_SUPPORTDIR_EDIT7, szAppIpsPath);
			SetDlgItemText(hDlg, IDC_SUPPORTDIR_EDIT8, szNeoCDGamesDir);
			SetDlgItemText(hDlg, IDC_SUPPORTDIR_EDIT9, szNeoCDCoverDir);

			// Setup the tabs
			hTabControl = GetDlgItem(hDlg, IDC_SPATH_TAB);

			TC_ITEM tcItem; 
			tcItem.mask = TCIF_TEXT;

			UINT idsString[9] = { IDS_SPATH_PREVIEW, IDS_SPATH_TITLES, IDS_SPATH_ICONS, IDS_SPATH_CHEATS, IDS_SPATH_HISCORE, IDS_SPATH_SAMPLES, IDS_SPATH_IPS, IDS_SPATH_NGCD_ISOS, IDS_SPATH_NGCD_COVERS };
			
			for(int nIndex = 0; nIndex < 9; nIndex++) {
				tcItem.pszText = FBALoadStringEx(hAppInst, idsString[nIndex], true);
				TabCtrl_InsertItem(hTabControl, nIndex, &tcItem);
			}

			int TabPage = TabCtrl_GetCurSel(hTabControl);
			
			// hide all controls excluding the selected controls
			for(int x = 0; x < 9; x++) {
				if(x != TabPage) {
					ShowWindow(GetDlgItem(hDlg, IDC_SUPPORTDIR_BR1 + x), SW_HIDE);		// browse buttons
					ShowWindow(GetDlgItem(hDlg, IDC_SUPPORTDIR_EDIT1 + x), SW_HIDE);	// edit controls
				}
			}

			// Show the proper controls
			ShowWindow(GetDlgItem(hDlg, IDC_SUPPORTDIR_BR1 + TabPage), SW_SHOW);		// browse buttons
			ShowWindow(GetDlgItem(hDlg, IDC_SUPPORTDIR_EDIT1 + TabPage), SW_SHOW);		// edit controls

			WndInMid(hDlg, hParent);
			SetFocus(hDlg);											// Enable Esc=close
			
			if (nInitTabSelect) {
				SendMessage(hTabControl, TCM_SETCURSEL, nInitTabSelect, 0);
				// hide all controls excluding the selected controls
				for(int x = 0; x < 9; x++) {
					if(x != nInitTabSelect) {
						ShowWindow(GetDlgItem(hDlg, IDC_SUPPORTDIR_BR1 + x), SW_HIDE);		// browse buttons
						ShowWindow(GetDlgItem(hDlg, IDC_SUPPORTDIR_EDIT1 + x), SW_HIDE);	// edit controls
					} else {
						ShowWindow(GetDlgItem(hDlg, IDC_SUPPORTDIR_BR1 + x), SW_SHOW);		// browse buttons
						ShowWindow(GetDlgItem(hDlg, IDC_SUPPORTDIR_EDIT1 + x), SW_SHOW);	// edit controls
					}
				}
			}
				
			break;
		}

		case WM_NOTIFY:
		{
			NMHDR* pNmHdr = (NMHDR*)lParam;

			if (pNmHdr->code == TCN_SELCHANGE) {

				int TabPage = TabCtrl_GetCurSel(hTabControl);
				
				// hide all controls excluding the selected controls
				for(int x = 0; x < 9; x++) {
					if(x != TabPage) {
						ShowWindow(GetDlgItem(hDlg, IDC_SUPPORTDIR_BR1 + x), SW_HIDE);		// browse buttons
						ShowWindow(GetDlgItem(hDlg, IDC_SUPPORTDIR_EDIT1 + x), SW_HIDE);	// edit controls
					}
				}

				// Show the proper controls
				ShowWindow(GetDlgItem(hDlg, IDC_SUPPORTDIR_BR1 + TabPage), SW_SHOW);		// browse buttons
				ShowWindow(GetDlgItem(hDlg, IDC_SUPPORTDIR_EDIT1 + TabPage), SW_SHOW);		// edit controls
				
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
				GetDlgItemText(hDlg, IDC_SUPPORTDIR_EDIT1, szAppPreviewsPath,	sizeof(szAppPreviewsPath));
				GetDlgItemText(hDlg, IDC_SUPPORTDIR_EDIT2, szAppTitlesPath,		sizeof(szAppTitlesPath));
				GetDlgItemText(hDlg, IDC_SUPPORTDIR_EDIT3, szAppIconsPath,		sizeof(szAppIconsPath));
				GetDlgItemText(hDlg, IDC_SUPPORTDIR_EDIT4, szAppCheatsPath,	sizeof(szAppCheatsPath));
				GetDlgItemText(hDlg, IDC_SUPPORTDIR_EDIT5, szAppHiscorePath,	sizeof(szAppHiscorePath));
				GetDlgItemText(hDlg, IDC_SUPPORTDIR_EDIT6, szAppSamplesPath,	sizeof(szAppSamplesPath));
				GetDlgItemText(hDlg, IDC_SUPPORTDIR_EDIT7, szAppIpsPath,	sizeof(szAppIpsPath));
				GetDlgItemText(hDlg, IDC_SUPPORTDIR_EDIT8, szNeoCDGamesDir,	sizeof(szNeoCDGamesDir));
				GetDlgItemText(hDlg, IDC_SUPPORTDIR_EDIT9, szNeoCDCoverDir,	sizeof(szNeoCDCoverDir));

				SendMessage(hDlg, WM_CLOSE, 0, 0);
				break;
			} else {
				if (LOWORD(wParam) >= IDC_SUPPORTDIR_BR1 && LOWORD(wParam) <= IDC_SUPPORTDIR_BR9) {
					var = IDC_SUPPORTDIR_EDIT1 + LOWORD(wParam) - IDC_SUPPORTDIR_BR1;
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
							buffer[strLen]		= _T('\\');
							buffer[strLen + 1]	= _T('\0');
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
			EndDialog(hDlg, 0);
			// If Icons directory path has been changed do the proper action
			if(_tcscmp(szCheckIconsPath, szAppIconsPath)) {
				IconsDirPathChanged();
			}
			break;
		}
	}

	return 0;
}

int SupportDirCreate(HWND hParentWND)
{
	hParent = hParentWND;
	
	_stprintf(szCheckIconsPath, szAppIconsPath);
	
	return FBADialogBox(hAppInst, MAKEINTRESOURCE(IDD_SUPPORTDIR), hParentWND, (DLGPROC)DefInpProc);
}

int SupportDirCreateTab(int nTab, HWND hParentWND)
{
	nInitTabSelect = nTab - IDC_SUPPORTDIR_EDIT1;
	
	hParent = hParentWND;
	
	_stprintf(szCheckIconsPath, szAppIconsPath);
	
	return FBADialogBox(hAppInst, MAKEINTRESOURCE(IDD_SUPPORTDIR), hParentWND, (DLGPROC)DefInpProc);
}

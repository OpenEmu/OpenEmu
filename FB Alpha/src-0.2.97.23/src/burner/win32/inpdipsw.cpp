// Burner DipSwitches Dialog module
#include "burner.h"

HWND hInpDIPSWDlg = NULL;									// Handle to the DIPSW Dialog
static HWND hInpDIPSWList = NULL;

static unsigned char nPrevDIPSettings[4];

static unsigned int nDIPGroup;
static int nDIPOffset;

static bool bOK;

static void InpDIPSWGetOffset()
{
	BurnDIPInfo bdi;

	nDIPOffset = 0;
	for (int i = 0; BurnDrvGetDIPInfo(&bdi, i) == 0; i++) {
		if (bdi.nFlags == 0xF0) {
			nDIPOffset = bdi.nInput;
			break;
		}
	}
}

void InpDIPSWResetDIPs()
{
	int i = 0;
	BurnDIPInfo bdi;
	struct GameInp* pgi;

	InpDIPSWGetOffset();

	while (BurnDrvGetDIPInfo(&bdi, i) == 0) {
		if (bdi.nFlags == 0xFF) {
			pgi = GameInp + bdi.nInput + nDIPOffset;
			pgi->Input.Constant.nConst = (pgi->Input.Constant.nConst & ~bdi.nMask) | (bdi.nSetting & bdi.nMask);
		}
		i++;
	}
}

static int InpDIPSWListBegin()
{
	LVCOLUMN LvCol;
	if (hInpDIPSWList == NULL) {
		return 1;
	}

	// Full row select style:
	SendMessage(hInpDIPSWList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	// Make column headers
	memset(&LvCol, 0, sizeof(LvCol));
	LvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	LvCol.cx = 158;
	LvCol.pszText = FBALoadStringEx(hAppInst, IDS_DIP_GROUP, true);
	SendMessage(hInpDIPSWList, LVM_INSERTCOLUMN, 0, (LPARAM)&LvCol);
	LvCol.cx = 215;
	LvCol.pszText = FBALoadStringEx(hAppInst, IDS_DIP_SETTING, true);
	SendMessage(hInpDIPSWList, LVM_INSERTCOLUMN, 1, (LPARAM)&LvCol);

	return 0;
}

static bool CheckSetting(int i)
{
	BurnDIPInfo bdi;
	BurnDrvGetDIPInfo(&bdi, i);
	struct GameInp* pgi = GameInp + bdi.nInput + nDIPOffset;

	if ((pgi->Input.Constant.nConst & bdi.nMask) == bdi.nSetting) {
		unsigned char nFlags = bdi.nFlags;
		if ((nFlags & 0x0F) <= 1) {
			return true;
		} else {
			for (int j = 1; j < (nFlags & 0x0F); j++) {
				BurnDrvGetDIPInfo(&bdi, i + j);
				pgi = GameInp + bdi.nInput + nDIPOffset;
				if (nFlags & 0x80) {
					if ((pgi->Input.Constant.nConst & bdi.nMask) == bdi.nSetting) {
						return false;
					}
				} else {
					if ((pgi->Input.Constant.nConst & bdi.nMask) != bdi.nSetting) {
						return false;
					}
				}
			}
			return true;
		}
	}
	return false;
}

// Make a list view of the DIPswitches
static int InpDIPSWListMake()
{
	if (hInpDIPSWList == NULL) {
		return 1;
	}
	SendMessage(hInpDIPSWList, LVM_DELETEALLITEMS, 0, 0);

	BurnDIPInfo bdi;
	unsigned int i = 0, j = 0, k = 0;
	char* pDIPGroup = NULL;
	while (BurnDrvGetDIPInfo(&bdi, i) == 0) {
		if ((bdi.nFlags & 0xF0) == 0xF0) {
		   	if (bdi.nFlags == 0xFE || bdi.nFlags == 0xFD) {
				pDIPGroup = bdi.szText;
				k = i;
			}
			i++;
		} else {
			if (CheckSetting(i)) {
				LVITEM LvItem;
				memset(&LvItem, 0, sizeof(LvItem));
				LvItem.mask = LVIF_TEXT | LVIF_PARAM;
				LvItem.iItem = j;
				LvItem.iSubItem = 0;
				LvItem.pszText = ANSIToTCHAR(pDIPGroup, NULL, 0);
				LvItem.lParam = (LPARAM)k;
				SendMessage(hInpDIPSWList, LVM_INSERTITEM, 0, (LPARAM)&LvItem);
				LvItem.mask = LVIF_TEXT;
				LvItem.iSubItem = 1;
				LvItem.pszText = ANSIToTCHAR(bdi.szText, NULL, 0);
				SendMessage(hInpDIPSWList, LVM_SETITEM, 0, (LPARAM)&LvItem);
				j++;
			}
			i += (bdi.nFlags & 0x0F);
		}
	}

	return 0;
}

static int InpDIPSWInit()
{
	BurnDIPInfo bdi;
	struct GameInp *pgi;

	InpDIPSWGetOffset();

	hInpDIPSWList = GetDlgItem(hInpDIPSWDlg, IDC_INPCHEAT_LIST);
	InpDIPSWListBegin();
	InpDIPSWListMake();

	for (int i = 0, j = 0; BurnDrvGetDIPInfo(&bdi, i) == 0; i++) {
		if (bdi.nInput >= 0  && bdi.nFlags == 0xFF) {
			pgi = GameInp + bdi.nInput + nDIPOffset;
			nPrevDIPSettings[j] = pgi->Input.Constant.nConst;
			j++;
		}
	}

	return 0;
}

static int InpDIPSWExit()
{
	hInpDIPSWList = NULL;
	hInpDIPSWDlg = NULL;
	if(!bAltPause && bRunPause) {
		bRunPause = 0;
	}
	GameInpCheckMouse();
	return 0;
}

static void InpDIPSWCancel()
{
	if (!bOK) {
		int i = 0, j = 0;
		BurnDIPInfo bdi;
		struct GameInp *pgi;
		while (BurnDrvGetDIPInfo(&bdi, i) == 0) {
			if (bdi.nInput >= 0 && bdi.nFlags == 0xFF) {
				pgi = GameInp + bdi.nInput + nDIPOffset;
				pgi->Input.Constant.nConst = nPrevDIPSettings[j];
				j++;
			}
			i++;
		}
	}
}

// Create the list of possible values for a DIPswitch
static void InpDIPSWSelect()
{
	SendMessage(GetDlgItem(hInpDIPSWDlg, IDC_INPCX1_VALUE), CB_RESETCONTENT, 0, 0);

	int nSel = SendMessage(hInpDIPSWList, LVM_GETNEXTITEM, (WPARAM)-1, LVNI_SELECTED);
	if (nSel >= 0) {
		LVITEM LvItem;
		memset(&LvItem, 0, sizeof(LvItem));
		LvItem.mask = LVIF_PARAM;
		LvItem.iItem = nSel;
		SendMessage(hInpDIPSWList, LVM_GETITEM, 0, (LPARAM)&LvItem);

		nDIPGroup = LvItem.lParam;

		BurnDIPInfo bdiGroup;
		BurnDrvGetDIPInfo(&bdiGroup, nDIPGroup);

		int nCurrentSetting = 0;
		for (int i = 0, j = 0; i < bdiGroup.nSetting; i++) {
			TCHAR szText[256];
			BurnDIPInfo bdi;

			do {
				BurnDrvGetDIPInfo(&bdi, nDIPGroup + 1 + j++);
			} while (bdi.nFlags == 0);

			if (bdiGroup.szText) {
				_stprintf(szText, _T("%hs: %hs"), bdiGroup.szText, bdi.szText);
			} else {
				_stprintf(szText, _T("%hs"), bdi.szText);
			}
			SendMessage(GetDlgItem(hInpDIPSWDlg, IDC_INPCX1_VALUE), CB_ADDSTRING, 0, (LPARAM)szText);

			if (CheckSetting(nDIPGroup + j)) {
				nCurrentSetting = i;
			}
		}
		SendMessage(GetDlgItem(hInpDIPSWDlg, IDC_INPCX1_VALUE), CB_SETCURSEL, (WPARAM)nCurrentSetting, 0);
	}
}

static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg == WM_INITDIALOG) {

//		EnableWindow(hScrnWnd, FALSE);

		hInpDIPSWDlg = hDlg;
		InpDIPSWInit();
		if (!kNetGame && bAutoPause) {
			bRunPause = 1;
		}
		
		WndInMid(hDlg, hScrnWnd);
		SetFocus(hDlg);											// Enable Esc=close
		
		return TRUE;
	}

    if (Msg == WM_CLOSE) {
		EnableWindow(hScrnWnd, TRUE);
		DestroyWindow(hInpDIPSWDlg);
		return 0;
	}

	if (Msg == WM_DESTROY) {
		InpDIPSWCancel();
		InpDIPSWExit();
		return 0;
	}

	if (Msg == WM_COMMAND) {
		int Id = LOWORD(wParam);
		int Notify = HIWORD(wParam);

		if (Id == IDOK && Notify == BN_CLICKED) {			// OK button
			bOK = true;
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			return 0;
		}
		if (Id == IDCANCEL && Notify == BN_CLICKED) {		// cancel = close
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			return 0;
		}

		// New DIPswitch value selected
		if (Id == IDC_INPCX1_VALUE && Notify == CBN_SELCHANGE) {
			BurnDIPInfo bdi = {0, 0, 0, 0, NULL};
			struct GameInp *pgi;
			int nSel = SendMessage(GetDlgItem(hInpDIPSWDlg, IDC_INPCX1_VALUE), CB_GETCURSEL, 0, 0);
			int j = 0;
			for (int i = 0; i <= nSel; i++) {
				do {
					BurnDrvGetDIPInfo(&bdi, nDIPGroup + 1 + j++);
				} while (bdi.nFlags == 0);
			}
			pgi = GameInp + bdi.nInput + nDIPOffset;
			pgi->Input.Constant.nConst = (pgi->Input.Constant.nConst & ~bdi.nMask) | (bdi.nSetting & bdi.nMask);
			if (bdi.nFlags & 0x40) {
				while (BurnDrvGetDIPInfo(&bdi, nDIPGroup + 1 + j++) == 0) {
					if (bdi.nFlags == 0) {
						pgi = GameInp + bdi.nInput + nDIPOffset;
						pgi->Input.Constant.nConst = (pgi->Input.Constant.nConst & ~bdi.nMask) | (bdi.nSetting & bdi.nMask);
					} else {
						break;
					}
				}
			}

			InpDIPSWListMake();
			return 0;
		}

		// New DIPswitch selected
		if (Id == IDC_INPC_RESET && Notify == BN_CLICKED) {

			InpDIPSWResetDIPs();

			InpDIPSWListMake();								// refresh view
			SendMessage(GetDlgItem(hInpDIPSWDlg, IDC_INPCX1_VALUE), CB_RESETCONTENT, 0, 0);
			return 0;
	   }

	}

	if (Msg == WM_NOTIFY && lParam) {
		int Id = LOWORD(wParam);
		NMHDR* pnm = (NMHDR*)lParam;

		if (Id == IDC_INPCHEAT_LIST && pnm->code == LVN_ITEMCHANGED) {
			if (((NM_LISTVIEW*)lParam)->uNewState & LVIS_SELECTED) {
				InpDIPSWSelect();
			}
			return 0;
		}

		if (Id == IDC_INPCHEAT_LIST && pnm->code == NM_CUSTOMDRAW) {
			NMLVCUSTOMDRAW* plvcd = (NMLVCUSTOMDRAW*)lParam;

			switch (plvcd->nmcd.dwDrawStage) {
				case CDDS_PREPAINT: {
                    SetWindowLongPtr(hInpDIPSWDlg, DWLP_MSGRESULT, CDRF_NOTIFYITEMDRAW);
					return 1;
				}
				case CDDS_ITEMPREPAINT: {
					BurnDIPInfo bdi;
					BurnDrvGetDIPInfo(&bdi, plvcd->nmcd.lItemlParam);
					if (bdi.nFlags == 0xFD) {
						plvcd->clrTextBk = RGB(0xFF, 0xDF, 0xBB);
    					SetWindowLongPtr(hInpDIPSWDlg, DWLP_MSGRESULT, CDRF_NEWFONT);
					}

					return 1;
				}
			}
		}
	}

	return 0;
}

int InpDIPSWCreate()
{
	if (bDrvOkay == 0) {									// No game is loaded
		return 1;
	}

	bOK = false;

	FBADialogBox(hAppInst, MAKEINTRESOURCE(IDD_INPDIP), hScrnWnd, (DLGPROC)DialogProc);

	return 0;
}

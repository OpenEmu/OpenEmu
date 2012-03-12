#include "burner.h"

HWND hInpCheatDlg = NULL;				// Handle to the Cheat Dialog

static HWND hInpCheatList = NULL;

static bool bOK;

static int nCurrentCheat;
static int* nPrevCheatSettings = NULL;

static int InpCheatListBegin()
{
	LVCOLUMN LvCol;
	if (hInpCheatList == NULL) {
		return 1;
	}

	// Full row select style:
	SendMessage(hInpCheatList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	// Make column headers
	memset(&LvCol, 0, sizeof(LvCol));
	LvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	LvCol.cx = 0xA4;
	LvCol.pszText = FBALoadStringEx(hAppInst, IDS_CHEAT_NAME, true);
	SendMessage(hInpCheatList, LVM_INSERTCOLUMN, 0, (LPARAM)&LvCol);
	LvCol.cx = 0xA4;
	LvCol.pszText = FBALoadStringEx(hAppInst, IDS_CHEAT_STATUS, true);
	SendMessage(hInpCheatList, LVM_INSERTCOLUMN, 1, (LPARAM)&LvCol);

	return 0;
}

// Make a list view of the DIPswitches
static int InpCheatListMake()
{
	if (hInpCheatList == NULL) {
		return 1;
	}

	SendMessage(hInpCheatList, LVM_DELETEALLITEMS, 0, 0);

	int i = 0;
	CheatInfo* pCurrentCheat = pCheatInfo;

	while (pCurrentCheat) {

		LVITEM LvItem;
		memset(&LvItem, 0, sizeof(LvItem));
		LvItem.mask = LVIF_TEXT;
		LvItem.iItem = i;
		LvItem.iSubItem = 0;
		LvItem.pszText = pCurrentCheat->szCheatName;
		SendMessage(hInpCheatList, LVM_INSERTITEM, 0, (LPARAM)&LvItem);
		LvItem.mask = LVIF_TEXT;
		LvItem.iSubItem = 1;
		LvItem.pszText = pCurrentCheat->pOption[pCurrentCheat->nCurrent]->szOptionName;
		SendMessage(hInpCheatList, LVM_SETITEM, 0, (LPARAM)&LvItem);

		pCurrentCheat = pCurrentCheat->pNext;
		i++;
	}

	return 0;
}

static int InpCheatInit()
{
	hInpCheatList = GetDlgItem(hInpCheatDlg, IDC_INPCHEAT_LIST);
	InpCheatListBegin();
	InpCheatListMake();

	// Save old cheat settings
	CheatInfo* pCurrentCheat = pCheatInfo;
	nCurrentCheat = 0;
	while (pCurrentCheat) {
		pCurrentCheat = pCurrentCheat->pNext;
		nCurrentCheat++;
	}

	nPrevCheatSettings = (int*)malloc(nCurrentCheat * sizeof(int));

	pCurrentCheat = pCheatInfo;
	nCurrentCheat = 0;
	while (pCurrentCheat) {
		nPrevCheatSettings[nCurrentCheat] = pCurrentCheat->nCurrent;
		pCurrentCheat = pCurrentCheat->pNext;
		nCurrentCheat++;
	}

	return 0;
}

static int InpCheatExit()
{
	if (nPrevCheatSettings) {
		free(nPrevCheatSettings);
		nPrevCheatSettings = NULL;
	}

	hInpCheatList = NULL;
	hInpCheatDlg = NULL;
	if(!bAltPause && bRunPause) {
		bRunPause = 0;
	}
	GameInpCheckMouse();
	return 0;
}

static void InpCheatCancel()
{
	if (!bOK) {
		CheatInfo* pCurrentCheat = pCheatInfo;
		nCurrentCheat = 0;
		while (pCurrentCheat) {
			CheatEnable(nCurrentCheat, nPrevCheatSettings[nCurrentCheat]);
			pCurrentCheat = pCurrentCheat->pNext;
			nCurrentCheat++;
		}
	}
}

static void InpCheatSelect()
{
	SendMessage(GetDlgItem(hInpCheatDlg, IDC_INPCX1_VALUE), CB_RESETCONTENT, 0, 0);

	int nSel = SendMessage(hInpCheatList, LVM_GETNEXTITEM, (WPARAM)-1, LVNI_SELECTED);
	if (nSel >= 0) {
		LVITEM LvItem;
		memset(&LvItem, 0, sizeof(LvItem));
		LvItem.mask = LVIF_PARAM;
		LvItem.iItem = nSel;
		SendMessage(hInpCheatList, LVM_GETITEM, 0, (LPARAM)&LvItem);

		CheatInfo* pCurrentCheat = pCheatInfo;
		nCurrentCheat = 0;
		while (pCurrentCheat && nCurrentCheat < nSel) {
			pCurrentCheat = pCurrentCheat->pNext;
			nCurrentCheat++;
		}

		for (int i = 0; pCurrentCheat->pOption[i]; i++) {
			TCHAR szText[256];
			_stprintf(szText, _T("%s: %s"), pCurrentCheat->szCheatName, pCurrentCheat->pOption[i]->szOptionName);
			SendMessage(GetDlgItem(hInpCheatDlg, IDC_INPCX1_VALUE), CB_ADDSTRING, 0, (LPARAM)szText);
		}

		SendMessage(GetDlgItem(hInpCheatDlg, IDC_INPCX1_VALUE), CB_SETCURSEL, (WPARAM)pCurrentCheat->nCurrent, 0);
	}
}

static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg == WM_INITDIALOG) {

//		EnableWindow(hScrnWnd, FALSE);

		hInpCheatDlg = hDlg;
		InpCheatInit();
		if (!kNetGame && bAutoPause) {
			bRunPause = 1;
		}
		
		WndInMid(hDlg, hScrnWnd);
		SetFocus(hDlg);											// Enable Esc=close
		
		return TRUE;
	}

    if (Msg == WM_CLOSE) {
		EnableWindow(hScrnWnd, TRUE);
		DestroyWindow(hInpCheatDlg);
		return 0;
	}

	if (Msg == WM_DESTROY) {
		InpCheatCancel();
		InpCheatExit();
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

		if (Id == IDC_INPCX1_VALUE && Notify == CBN_SELCHANGE) {

			int nSel = SendMessage(GetDlgItem(hInpCheatDlg, IDC_INPCX1_VALUE), CB_GETCURSEL, 0, 0);

			CheatEnable(nCurrentCheat, nSel);

			InpCheatListMake();
			return 0;
		}

		if (Id == IDC_INPC_RESET && Notify == BN_CLICKED) {

			CheatInfo* pCurrentCheat = pCheatInfo;
			nCurrentCheat = 0;
			while (pCurrentCheat) {
				CheatEnable(nCurrentCheat, -1);
				pCurrentCheat = pCurrentCheat->pNext;
				nCurrentCheat++;
			}

			InpCheatListMake();								// refresh view
			SendMessage(GetDlgItem(hInpCheatDlg, IDC_INPCX1_VALUE), CB_RESETCONTENT, 0, 0);
			return 0;
	   }

	}

	if (Msg == WM_NOTIFY && lParam) {
		int Id = LOWORD(wParam);
		NMHDR *pnm = (NMHDR*)lParam;

		if (Id == IDC_INPCHEAT_LIST && pnm->code == LVN_ITEMCHANGED) {
			if (((NM_LISTVIEW*)lParam)->uNewState & LVIS_SELECTED) {
				InpCheatSelect();
			}
			return 0;
		}
		
		if (Id == IDC_INPCHEAT_LIST && ((pnm->code == NM_DBLCLK) || (pnm->code == NM_RDBLCLK))) {
			// Select the next item of the currently selected one.
			int nSel_Dbl = SendMessage(GetDlgItem(hInpCheatDlg, IDC_INPCX1_VALUE), CB_GETCURSEL, 0, 0);
			int nCount = SendMessage(GetDlgItem(hInpCheatDlg, IDC_INPCX1_VALUE), CB_GETCOUNT, 0, 0);
			if ((nSel_Dbl != LB_ERR) && (nCount > 1)) {
				if (pnm->code == NM_DBLCLK) {
					if (++nSel_Dbl >= nCount) nSel_Dbl = 0;
				} else {
					if (--nSel_Dbl < 0) nSel_Dbl = nCount - 1;
				}
				SendMessage(GetDlgItem(hInpCheatDlg, IDC_INPCX1_VALUE), CB_SETCURSEL, nSel_Dbl, 0);
				CheatEnable(nCurrentCheat, nSel_Dbl);
				InpCheatListMake();
			}
			return 0;
		}
	}

	return 0;
}

int InpCheatCreate()
{
	if (bDrvOkay == 0) {									// No game is loaded
		return 1;
	}

	bOK = false;

//	DestroyWindow(hInpCheatDlg);							// Make sure exitted

//	hInpCheatDlg = FBACreateDialog(hAppInst, MAKEINTRESOURCE(IDD_INPCHEAT), hScrnWnd, DialogProc);
//	if (hInpCheatDlg == NULL) {
//		return 1;
//	}

//	WndInMid(hInpCheatDlg, hScrnWnd);
//	ShowWindow(hInpCheatDlg, SW_NORMAL);

	FBADialogBox(hAppInst, MAKEINTRESOURCE(IDD_INPCHEAT), hScrnWnd, (DLGPROC)DialogProc);

	return 0;
}


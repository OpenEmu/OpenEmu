// Burner Input Dialog module
#include "burner.h"

HWND hInpdDlg = NULL;							// Handle to the Input Dialog
static HWND hInpdList = NULL;
static unsigned char *LastVal = NULL;			// Last input values/defined
static int bLastValDefined = 0;					//

static HWND hInpdGi = NULL, hInpdPci = NULL, hInpdAnalog = NULL;	// Combo boxes

// Update which input is using which PC input
static int InpdUseUpdate()
{
	unsigned int i, j = 0;
	struct GameInp* pgi = NULL;
	if (hInpdList == NULL) {
		return 1;
	}

	// Update the values of all the inputs
	for (i = 0, pgi = GameInp; i < nGameInpCount; i++, pgi++) {
		LVITEM LvItem;
		TCHAR* pszVal = NULL;

		if (pgi->Input.pVal == NULL) {
			continue;
		}

		pszVal = InpToDesc(pgi);

		memset(&LvItem, 0, sizeof(LvItem));
		LvItem.mask = LVIF_TEXT;
		LvItem.iItem = j;
		LvItem.iSubItem = 1;
		LvItem.pszText = pszVal;

		SendMessage(hInpdList, LVM_SETITEM, 0, (LPARAM)&LvItem);

		j++;
	}

	for (i = 0, pgi = GameInp + nGameInpCount; i < nMacroCount; i++, pgi++) {
		LVITEM LvItem;
		TCHAR* pszVal = NULL;

		if (pgi->nInput & GIT_GROUP_MACRO) {
			pszVal = InpMacroToDesc(pgi);

			memset(&LvItem, 0, sizeof(LvItem));
			LvItem.mask = LVIF_TEXT;
			LvItem.iItem = j;
			LvItem.iSubItem = 1;
			LvItem.pszText = pszVal;

			SendMessage(hInpdList, LVM_SETITEM, 0, (LPARAM)&LvItem);
		}

		j++;
	}

	return 0;
}

int InpdUpdate()
{
	unsigned int i, j = 0;
	struct GameInp* pgi = NULL;
	unsigned char* plv = NULL;
	unsigned short nThisVal;
	if (hInpdList == NULL) {
		return 1;
	}
	if (LastVal == NULL) {
		return 1;
	}

	// Update the values of all the inputs
	for (i = 0, pgi = GameInp, plv = LastVal; i < nGameInpCount; i++, pgi++, plv++) {
		LVITEM LvItem;
		TCHAR szVal[16];

		if (pgi->nType == 0) {
			continue;
		}

		if (pgi->nType & BIT_GROUP_ANALOG) {
			if (bRunPause) {														// Update LastVal
				nThisVal = pgi->Input.nVal;
			} else {
				nThisVal = *pgi->Input.pShortVal;
			}

			if (bLastValDefined && (pgi->nType != BIT_ANALOG_REL || nThisVal) && pgi->Input.nVal == *((unsigned short*)plv)) {
				j++;
				continue;
			}

			*((unsigned short*)plv) = nThisVal;
		} else {
			if (bRunPause) {														// Update LastVal
				nThisVal = pgi->Input.nVal;
			} else {
				nThisVal = *pgi->Input.pVal;
			}

			if (bLastValDefined && pgi->Input.nVal == *plv) {						// hasn't changed
				j++;
				continue;
			}

			*plv = nThisVal;
		}

		switch (pgi->nType) {
			case BIT_DIGITAL: {
				if (nThisVal == 0) {
					szVal[0] = 0;
				} else {
					if (nThisVal == 1) {
						_tcscpy(szVal, _T("ON"));
					} else {
						_stprintf(szVal, _T("0x%02X"), nThisVal);
					}
				}
				break;
			}
			case BIT_ANALOG_ABS: {
				_stprintf(szVal, _T("0x%02X"), nThisVal >> 8);
				break;
			}
			case BIT_ANALOG_REL: {
				if (nThisVal == 0) {
					szVal[0] = 0;
				}
				if ((short)nThisVal < 0) {
					_stprintf(szVal, _T("%d"), ((short)nThisVal) >> 8);
				}
				if ((short)nThisVal > 0) {
					_stprintf(szVal, _T("+%d"), ((short)nThisVal) >> 8);
				}
				break;
			}
			default: {
				_stprintf(szVal, _T("0x%02X"), nThisVal);
			}
		}

		memset(&LvItem, 0, sizeof(LvItem));
		LvItem.mask = LVIF_TEXT;
		LvItem.iItem = j;
		LvItem.iSubItem = 2;
		LvItem.pszText = szVal;

		SendMessage(hInpdList, LVM_SETITEM, 0, (LPARAM)&LvItem);

		j++;
	}

	bLastValDefined = 1;										// LastVal is now defined

	return 0;
}

static int InpdListBegin()
{
	LVCOLUMN LvCol;
	if (hInpdList == NULL) {
		return 1;
	}

	// Full row select style:
	SendMessage(hInpdList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);

	// Make column headers
	memset(&LvCol, 0, sizeof(LvCol));
	LvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

	LvCol.cx = 0xa0;
	LvCol.pszText = FBALoadStringEx(hAppInst, IDS_INPUT_INPUT, true);
	SendMessage(hInpdList, LVM_INSERTCOLUMN, 0, (LPARAM)&LvCol);

	LvCol.cx = 0xa0;
	LvCol.pszText = FBALoadStringEx(hAppInst, IDS_INPUT_MAPPING, true);
	SendMessage(hInpdList, LVM_INSERTCOLUMN, 1, (LPARAM)&LvCol);

	LvCol.cx = 0x38;
	LvCol.pszText = FBALoadStringEx(hAppInst, IDS_INPUT_STATE, true);
	SendMessage(hInpdList, LVM_INSERTCOLUMN, 2, (LPARAM)&LvCol);

	return 0;
}

// Make a list view of the game inputs
int InpdListMake(int bBuild)
{
	unsigned int j = 0;

	if (hInpdList == NULL) {
		return 1;
	}

	bLastValDefined = 0;
	if (bBuild)	{
		SendMessage(hInpdList, LVM_DELETEALLITEMS, 0, 0);
	}

	// Add all the input names to the list
	for (unsigned int i = 0; i < nGameInpCount; i++) {
		struct BurnInputInfo bii;
		LVITEM LvItem;

		// Get the name of the input
		bii.szName = NULL;
		BurnDrvGetInputInfo(&bii, i);

		// skip unused inputs
		if (bii.pVal == NULL) {
			continue;
		}
		if (bii.szName == NULL)	{
			bii.szName = "";
		}

		memset(&LvItem, 0, sizeof(LvItem));
		LvItem.mask = LVIF_TEXT | LVIF_PARAM;
		LvItem.iItem = j;
		LvItem.iSubItem = 0;
		LvItem.pszText = ANSIToTCHAR(bii.szName, NULL, 0);
		LvItem.lParam = (LPARAM)i;

		SendMessage(hInpdList, bBuild ? LVM_INSERTITEM : LVM_SETITEM, 0, (LPARAM)&LvItem);

		j++;
	}

	struct GameInp* pgi = GameInp + nGameInpCount;
	for (unsigned int i = 0; i < nMacroCount; i++, pgi++) {
		LVITEM LvItem;

		if (pgi->nInput & GIT_GROUP_MACRO) {
			memset(&LvItem, 0, sizeof(LvItem));
			LvItem.mask = LVIF_TEXT | LVIF_PARAM;
			LvItem.iItem = j;
			LvItem.iSubItem = 0;
			LvItem.pszText = ANSIToTCHAR(pgi->Macro.szName, NULL, 0);
			LvItem.lParam = (LPARAM)j;

			SendMessage(hInpdList, bBuild ? LVM_INSERTITEM : LVM_SETITEM, 0, (LPARAM)&LvItem);
		}

		j++;
	}

	InpdUseUpdate();

	return 0;
}

static void DisablePresets()
{
	EnableWindow(hInpdPci, FALSE);
	EnableWindow(hInpdAnalog, FALSE);
	EnableWindow(GetDlgItem(hInpdDlg, IDC_INPD_DEFAULT), FALSE);
	EnableWindow(GetDlgItem(hInpdDlg, IDC_INPD_USE), FALSE);
}

static void InitComboboxes()
{
	TCHAR szLabel[1024];
	HANDLE search;
	WIN32_FIND_DATA findData;

	for (int i = 0; i < 4; i++) {
		_stprintf(szLabel, FBALoadStringEx(hAppInst, IDS_INPUT_INP_PLAYER, true), i + 1);
		SendMessage(hInpdGi, CB_ADDSTRING, 0, (LPARAM)szLabel);
	}

	SendMessage(hInpdPci, CB_ADDSTRING, 0, (LPARAM)FBALoadStringEx(hAppInst, IDS_INPUT_INP_KEYBOARD, true));
	for (int i = 0; i < 3; i++) {
		_stprintf(szLabel, FBALoadStringEx(hAppInst, IDS_INPUT_INP_JOYSTICK, true), i);
		SendMessage(hInpdPci, CB_ADDSTRING, 0, (LPARAM)szLabel);
	}
	SendMessage(hInpdPci, CB_ADDSTRING, 0, (LPARAM)FBALoadStringEx(hAppInst, IDS_INPUT_INP_XARCADEL, true));
	SendMessage(hInpdPci, CB_ADDSTRING, 0, (LPARAM)FBALoadStringEx(hAppInst, IDS_INPUT_INP_XARCADER, true));
	SendMessage(hInpdPci, CB_ADDSTRING, 0, (LPARAM)FBALoadStringEx(hAppInst, IDS_INPUT_INP_HOTRODL, true));
	SendMessage(hInpdPci, CB_ADDSTRING, 0, (LPARAM)FBALoadStringEx(hAppInst, IDS_INPUT_INP_HOTRODR, true));

	// Scan presets directory for .ini files and add them to the list
	if ((search = FindFirstFile(_T("config/presets/*.ini"), &findData)) != INVALID_HANDLE_VALUE) {
		do {
			findData.cFileName[_tcslen(findData.cFileName) - 4] = 0;
			SendMessage(hInpdPci, CB_ADDSTRING, 0, (LPARAM)findData.cFileName);
		} while (FindNextFile(search, &findData) != 0);

		FindClose(search);
	}
}

static int InpdInit()
{
	int nMemLen;

	hInpdList = GetDlgItem(hInpdDlg, IDC_INPD_LIST);

	// Allocate a last val array for the last input values
	nMemLen = nGameInpCount * sizeof(char);
	LastVal = (unsigned char*)malloc(nMemLen);
	if (LastVal == NULL) {
		return 1;
	}
	memset(LastVal, 0, nMemLen);

	InpdListBegin();
	InpdListMake(1);

	// Init the Combo boxes
	hInpdGi = GetDlgItem(hInpdDlg, IDC_INPD_GI);
	hInpdPci = GetDlgItem(hInpdDlg, IDC_INPD_PCI);
	hInpdAnalog = GetDlgItem(hInpdDlg, IDC_INPD_ANALOG);
	InitComboboxes();

	DisablePresets();

	return 0;
}

static int InpdExit()
{
	// Exit the Combo boxes
	hInpdPci = NULL;
	hInpdGi = NULL;
	hInpdAnalog = NULL;

	if (LastVal != NULL) {
		free(LastVal);
		LastVal = NULL;
	}
	hInpdList = NULL;
	hInpdDlg = NULL;
	if (!bAltPause && bRunPause) {
		bRunPause=0;
	}
	GameInpCheckMouse();

	return 0;
}

static void GameInpConfigOne(int nPlayer, int nPcDev, int nAnalog, struct GameInp* pgi, char* szi)
{
	switch (nPcDev) {
		case  0:
			GamcPlayer(pgi, szi, nPlayer, -1);						// Keyboard
			GamcAnalogKey(pgi, szi, nPlayer, nAnalog);
			GamcMisc(pgi, szi, nPlayer);
			break;
		case  1:
			GamcPlayer(pgi, szi, nPlayer, 0);						// Joystick 1
			GamcAnalogJoy(pgi, szi, nPlayer, 0, nAnalog);
			GamcMisc(pgi, szi, nPlayer);
			break;
		case  2:
			GamcPlayer(pgi, szi, nPlayer, 1);						// Joystick 2
			GamcAnalogJoy(pgi, szi, nPlayer, 1, nAnalog);
			GamcMisc(pgi, szi, nPlayer);
			break;
		case  3:
			GamcPlayer(pgi, szi, nPlayer, 2);						// Joystick 3
			GamcAnalogJoy(pgi, szi, nPlayer, 2, nAnalog);
			GamcMisc(pgi, szi, nPlayer);
			break;
		case  4:
			GamcPlayerHotRod(pgi, szi, nPlayer, 0x10, nAnalog);		// X-Arcade left side
			GamcMisc(pgi, szi, -1);
			break;
		case  5:
			GamcPlayerHotRod(pgi, szi, nPlayer, 0x11, nAnalog);		// X-Arcade right side
			GamcMisc(pgi, szi, -1);
			break;
		case  6:
			GamcPlayerHotRod(pgi, szi, nPlayer, 0x00, nAnalog);		// HotRod left side
			GamcMisc(pgi, szi, -1);
			break;
		case  7:
			GamcPlayerHotRod(pgi, szi, nPlayer, 0x01, nAnalog);		// HotRod right size
			GamcMisc(pgi, szi, -1);
			break;
	}
}

// Configure some of the game input
static int GameInpConfig(int nPlayer, int nPcDev, int nAnalog)
{
	struct GameInp* pgi = NULL;
	unsigned int i;

	for (i = 0, pgi = GameInp; i < nGameInpCount; i++, pgi++) {
		struct BurnInputInfo bii;

		// Get the extra info about the input
		bii.szInfo = NULL;
		BurnDrvGetInputInfo(&bii, i);
		if (bii.pVal == NULL) {
			continue;
		}
		if (bii.szInfo == NULL) {
			bii.szInfo = "";
		}
		GameInpConfigOne(nPlayer, nPcDev, nAnalog, pgi, bii.szInfo);
	}

	for (i = 0; i < nMacroCount; i++, pgi++) {
		GameInpConfigOne(nPlayer, nPcDev, nAnalog, pgi, pgi->Macro.szName);
	}

	GameInpCheckLeftAlt();

	return 0;
}

// List item activated; find out which one
static int ListItemActivate()
{
	struct BurnInputInfo bii;
	LVITEM LvItem;

	int nSel = SendMessage(hInpdList, LVM_GETNEXTITEM, (WPARAM)-1, LVNI_SELECTED);
	if (nSel < 0) {
		return 1;
	}

	// Get the corresponding input
	LvItem.mask = LVIF_PARAM;
	LvItem.iItem = nSel;
	LvItem.iSubItem = 0;
	SendMessage(hInpdList, LVM_GETITEM, 0, (LPARAM)&LvItem);
	nSel = LvItem.lParam;

	if (nSel >= (int)(nGameInpCount + nMacroCount)) {	// out of range
		return 1;
	}

	bii.nType = 0;
	BurnDrvGetInputInfo(&bii, nSel);
	if (bii.pVal == NULL) {
		return 1;
	}

	DestroyWindow(hInpsDlg);							// Make sure any existing dialogs are gone
	DestroyWindow(hInpcDlg);							//

	if (bii.nType & BIT_GROUP_CONSTANT) {
		// Dip switch is a constant - change it
		nInpcInput = nSel;
		InpcCreate();
	} else {
		if (GameInp[nSel].nInput == GIT_MACRO_CUSTOM) {
#if 0
			InpMacroCreate(nSel);
#endif
		} else {
			// Assign to a key
			nInpsInput = nSel;
			InpsCreate();
		}
	}

	GameInpCheckLeftAlt();

	return 0;
}

#if 0
static int NewMacroButton()
{
	LVITEM LvItem;
	int nSel;

	DestroyWindow(hInpsDlg);							// Make sure any existing dialogs are gone
	DestroyWindow(hInpcDlg);							//

	nSel = SendMessage(hInpdList, LVM_GETNEXTITEM, (WPARAM)-1, LVNI_SELECTED);
	if (nSel < 0) {
		nSel = -1;
	}

	// Get the corresponding input
	LvItem.mask = LVIF_PARAM;
	LvItem.iItem = nSel;
	LvItem.iSubItem = 0;
	SendMessage(hInpdList, LVM_GETITEM, 0, (LPARAM)&LvItem);
	nSel = LvItem.lParam;

	if (nSel >= (int)nGameInpCount && nSel < (int)(nGameInpCount + nMacroCount)) {
		if (GameInp[nSel].nInput != GIT_MACRO_CUSTOM) {
			nSel = -1;
		}
	} else {
		nSel = -1;
	}

	InpMacroCreate(nSel);

	return 0;
}
#endif

static int DeleteInput(unsigned int i)
{
	struct BurnInputInfo bii;

	if (i >= nGameInpCount) {

		if (i < nGameInpCount + nMacroCount) {	// Macro
			GameInp[i].Macro.nMode = 0;
		} else { 								// out of range
			return 1;
		}
	} else {									// "True" input
		bii.nType = BIT_DIGITAL;
		BurnDrvGetInputInfo(&bii, i);
		if (bii.pVal == NULL) {
			return 1;
		}
		if (bii.nType & BIT_GROUP_CONSTANT) {	// Don't delete dip switches
			return 1;
		}

		GameInp[i].nInput = 0;
	}

	GameInpCheckLeftAlt();

	return 0;
}

// List item(s) deleted; find out which one(s)
static int ListItemDelete()
{
	int nStart = -1;
	LVITEM LvItem;
	int nRet;

	while ((nRet = SendMessage(hInpdList, LVM_GETNEXTITEM, (WPARAM)nStart, LVNI_SELECTED)) != -1) {
		nStart = nRet;

		// Get the corresponding input
		LvItem.mask = LVIF_PARAM;
		LvItem.iItem = nRet;
		LvItem.iSubItem = 0;
		SendMessage(hInpdList, LVM_GETITEM, 0, (LPARAM)&LvItem);
		nRet = LvItem.lParam;

		DeleteInput(nRet);
	}

	InpdListMake(0);							// refresh view
	return 0;
}

static int InitAnalogOptions(int nGi, int nPci)
{
	// init analog options dialog
	int nAnalog = -1;
	if (nPci == (nPlayerDefaultControls[nGi] & 0x0F)) {
		nAnalog = nPlayerDefaultControls[nGi] >> 4;
	}

	SendMessage(hInpdAnalog, CB_RESETCONTENT, 0, 0);
	if (nPci >= 1 && nPci <= 3) {
		// Absolute mode only for joysticks
		SendMessage(hInpdAnalog, CB_ADDSTRING, 0, (LPARAM)(LPARAM)FBALoadStringEx(hAppInst, IDS_INPUT_ANALOG_ABS, true));
	} else {
		if (nAnalog > 0) {
			nAnalog--;
		}
	}
	SendMessage(hInpdAnalog, CB_ADDSTRING, 0, (LPARAM)(LPARAM)FBALoadStringEx(hAppInst, IDS_INPUT_ANALOG_AUTO, true));
	SendMessage(hInpdAnalog, CB_ADDSTRING, 0, (LPARAM)(LPARAM)FBALoadStringEx(hAppInst, IDS_INPUT_ANALOG_NORMAL, true));

	SendMessage(hInpdAnalog, CB_SETCURSEL, (WPARAM)nAnalog, 0);

	return 0;
}

static void SaveHardwarePreset()
{
	TCHAR *szDefaultCpsFile = _T("config\\presets\\cps.ini");
	TCHAR *szDefaultNeogeoFile = _T("config\\presets\\neogeo.ini");
	TCHAR *szDefaultPgmFile = _T("config\\presets\\pgm.ini");
	TCHAR *szFileName = _T("config\\presets\\preset.ini");
	TCHAR *szHardwareString = _T("Generic hardware");
	
	int nHardwareFlag = (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK);

	if (nHardwareFlag == HARDWARE_CAPCOM_CPS1 || nHardwareFlag == HARDWARE_CAPCOM_CPS1_QSOUND || nHardwareFlag == HARDWARE_CAPCOM_CPS1_GENERIC || nHardwareFlag == HARDWARE_CAPCOM_CPSCHANGER || nHardwareFlag == HARDWARE_CAPCOM_CPS2 || nHardwareFlag == HARDWARE_CAPCOM_CPS3) {
		szFileName = szDefaultCpsFile;
		szHardwareString = _T("CPS-1/CPS-2/CPS-3 hardware");
	}
	
	if (nHardwareFlag == HARDWARE_SNK_NEOGEO) {
		szFileName = szDefaultNeogeoFile;
		szHardwareString = _T("Neo-Geo hardware");
	}
	
	if (nHardwareFlag == HARDWARE_IGS_PGM) {
		szFileName = szDefaultPgmFile;
		szHardwareString = _T("PGM hardware");
	}
	
	FILE *fp = _tfopen(szFileName, _T("wt"));
	if (fp) {
		_ftprintf(fp, _T(APP_TITLE) _T(" - Hardware Default Preset\n\n"));
		_ftprintf(fp, _T("%s\n\n"), szHardwareString);
		_ftprintf(fp, _T("version 0x%06X\n\n"), nBurnVer);
		GameInpWrite(fp);
		fclose(fp);
	}
	
	FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_PRESET_SAVED), szFileName);
	FBAPopupDisplay(PUF_TYPE_INFO);
}

int UsePreset(bool bMakeDefault)
{
	int nGi, nPci, nAnalog = 0;
	TCHAR szFilename[MAX_PATH] = _T("config\\presets\\");

	nGi = SendMessage(hInpdGi, CB_GETCURSEL, 0, 0);
	if (nGi == CB_ERR) {
		return 1;
	}
	nPci = SendMessage(hInpdPci, CB_GETCURSEL, 0, 0);
	if (nPci == CB_ERR) {
		return 1;
	}
	if (nPci <= 7) {
		// Determine analog option
		nAnalog = SendMessage(hInpdAnalog, CB_GETCURSEL, 0, 0);
		if (nAnalog == CB_ERR) {
			return 1;
		}

		if (nPci == 0 || nPci > 3) {				// No "Absolute" option for keyboard or X-Arcade/HotRod controls
			nAnalog++;
		}

		GameInpConfig(nGi, nPci, nAnalog);			// Re-configure inputs
	} else {
		// Find out the filename of the preset ini
		SendMessage(hInpdPci, CB_GETLBTEXT, nPci, (LPARAM)(szFilename + _tcslen(szFilename)));
		_tcscat(szFilename, _T(".ini"));

		GameInputAutoIni(nGi, szFilename, true);	// Read inputs from file

		// Make sure all inputs are defined
		for (unsigned int i = 0, j = 0; i < nGameInpCount; i++) {
			if (GameInp[i].Input.pVal == NULL) {
				continue;
			}

			if (GameInp[i].nInput == 0) {
				DeleteInput(j);
			}

			j++;
		}

		nPci = 0x0F;
	}

	SendMessage(hInpdAnalog, CB_SETCURSEL, (WPARAM)-1, 0);
	SendMessage(hInpdPci, CB_SETCURSEL, (WPARAM)-1, 0);
	SendMessage(hInpdGi, CB_SETCURSEL, (WPARAM)-1, 0);

	DisablePresets();

	if (bMakeDefault) {
		nPlayerDefaultControls[nGi] = nPci | (nAnalog << 4);
		_tcscpy(szPlayerDefaultIni[nGi], szFilename);
	}

	GameInpCheckLeftAlt();

	return 0;
}

static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg == WM_INITDIALOG) {
		hInpdDlg = hDlg;
		InpdInit();
		if (!kNetGame && bAutoPause) {
			bRunPause = 1;
		}
		
		return TRUE;
	}

	if (Msg == WM_CLOSE) {
		EnableWindow(hScrnWnd, TRUE);
		DestroyWindow(hInpdDlg);
		return 0;
	}

	if (Msg == WM_DESTROY) {
		InpdExit();
		return 0;
	}

	if (Msg == WM_COMMAND) {
		int Id = LOWORD(wParam);
		int Notify = HIWORD(wParam);

		if (Id == IDOK && Notify == BN_CLICKED) {
			ListItemActivate();
			return 0;
		}
		if (Id == IDCANCEL && Notify == BN_CLICKED) {
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			return 0;
		}

		if (Id == IDC_INPD_NEWMACRO && Notify == BN_CLICKED) {

//			NewMacroButton();

			return 0;
		}
		
		if (Id == IDC_INPD_SAVE_AS_PRESET && Notify == BN_CLICKED) {
			SaveHardwarePreset();
			return 0;
		}

		if (Id == IDC_INPD_USE && Notify == BN_CLICKED) {

			UsePreset(false);

			InpdListMake(0);								// refresh view

			return 0;
		}

		if (Id == IDC_INPD_DEFAULT && Notify == BN_CLICKED) {

			UsePreset(true);

			InpdListMake(0);								// refresh view

			return 0;
		}

		if (Id == IDC_INPD_GI && Notify == CBN_SELCHANGE) {
			int nGi;
			nGi = SendMessage(hInpdGi, CB_GETCURSEL, 0, 0);
			if (nGi == CB_ERR) {
				SendMessage(hInpdPci, CB_SETCURSEL, (WPARAM)-1, 0);
				SendMessage(hInpdAnalog, CB_SETCURSEL, (WPARAM)-1, 0);

				DisablePresets();

				return 0;
			}
			int nPci = nPlayerDefaultControls[nGi] & 0x0F;
			SendMessage(hInpdPci, CB_SETCURSEL, nPci, 0);
			EnableWindow(hInpdPci, TRUE);

			if (nPci > 5) {
				SendMessage(hInpdAnalog, CB_SETCURSEL, (WPARAM)-1, 0);
				EnableWindow(hInpdAnalog, FALSE);
			} else {
				InitAnalogOptions(nGi, nPci);
				EnableWindow(hInpdAnalog, TRUE);
			}

			EnableWindow(GetDlgItem(hInpdDlg, IDC_INPD_DEFAULT), TRUE);
			EnableWindow(GetDlgItem(hInpdDlg, IDC_INPD_USE), TRUE);

			return 0;
		}

		if (Id == IDC_INPD_PCI && Notify == CBN_SELCHANGE) {
			int nGi, nPci;
			nGi = SendMessage(hInpdGi, CB_GETCURSEL, 0, 0);
			if (nGi == CB_ERR) {
				return 0;
			}
			nPci = SendMessage(hInpdPci, CB_GETCURSEL, 0, 0);
			if (nPci == CB_ERR) {
				return 0;
			}

			if (nPci > 7) {
				EnableWindow(GetDlgItem(hInpdDlg, IDC_INPD_DEFAULT), TRUE);
				EnableWindow(GetDlgItem(hInpdDlg, IDC_INPD_USE), TRUE);

				SendMessage(hInpdAnalog, CB_SETCURSEL, (WPARAM)-1, 0);
				EnableWindow(hInpdAnalog, FALSE);
			} else {
				EnableWindow(hInpdAnalog, TRUE);
				InitAnalogOptions(nGi, nPci);

				if (SendMessage(hInpdAnalog, CB_GETCURSEL, 0, 0) != CB_ERR) {
					EnableWindow(GetDlgItem(hInpdDlg, IDC_INPD_DEFAULT), TRUE);
					EnableWindow(GetDlgItem(hInpdDlg, IDC_INPD_USE), TRUE);
				} else {
					EnableWindow(GetDlgItem(hInpdDlg, IDC_INPD_DEFAULT), FALSE);
					EnableWindow(GetDlgItem(hInpdDlg, IDC_INPD_USE), FALSE);
				}
			}

			return 0;
		}

		if (Id == IDC_INPD_ANALOG && Notify == CBN_SELCHANGE) {
			if (SendMessage(hInpdAnalog, CB_GETCURSEL, 0, 0) != CB_ERR) {
				EnableWindow(GetDlgItem(hInpdDlg, IDC_INPD_DEFAULT), TRUE);
				EnableWindow(GetDlgItem(hInpdDlg, IDC_INPD_USE), TRUE);
			}

			return 0;
		}

	}

	if (Msg == WM_NOTIFY && lParam != 0) {
		int Id = LOWORD(wParam);
		NMHDR* pnm = (NMHDR*)lParam;

		if (Id == IDC_INPD_LIST && pnm->code == LVN_ITEMACTIVATE) {
			ListItemActivate();
		}
		if (Id == IDC_INPD_LIST && pnm->code == LVN_KEYDOWN) {
			NMLVKEYDOWN *pnmkd = (NMLVKEYDOWN*)lParam;
			if (pnmkd->wVKey == VK_DELETE) {
				ListItemDelete();
			}
		}

		if (Id == IDC_INPD_LIST && pnm->code == NM_CUSTOMDRAW) {
			NMLVCUSTOMDRAW* plvcd = (NMLVCUSTOMDRAW*)lParam;

			switch (plvcd->nmcd.dwDrawStage) {
				case CDDS_PREPAINT:
                    SetWindowLongPtr(hInpdDlg, DWLP_MSGRESULT, CDRF_NOTIFYITEMDRAW);
					return 1;
				case CDDS_ITEMPREPAINT:
					if (plvcd->nmcd.dwItemSpec < nGameInpCount) {
						if (GameInp[plvcd->nmcd.dwItemSpec].nType & BIT_GROUP_CONSTANT) {

							if (GameInp[plvcd->nmcd.dwItemSpec].nInput == 0) {
								plvcd->clrTextBk = RGB(0xDF, 0xDF, 0xDF);

								SetWindowLongPtr(hInpdDlg, DWLP_MSGRESULT, CDRF_NEWFONT);
								return 1;
							}

							if (GameInp[plvcd->nmcd.dwItemSpec].nType == BIT_DIPSWITCH) {
								plvcd->clrTextBk = RGB(0xFF, 0xEF, 0xD7);

								SetWindowLongPtr(hInpdDlg, DWLP_MSGRESULT, CDRF_NEWFONT);
								return 1;
							}
						}
					}

					if (plvcd->nmcd.dwItemSpec >= nGameInpCount) {
						if (GameInp[plvcd->nmcd.dwItemSpec].Macro.nMode) {
							plvcd->clrTextBk = RGB(0xFF, 0xCF, 0xCF);
						} else {
							plvcd->clrTextBk = RGB(0xFF, 0xEF, 0xEF);
						}

						SetWindowLongPtr(hInpdDlg, DWLP_MSGRESULT, CDRF_NEWFONT);
						return 1;
					}
					return 1;
			}
		}
		return 0;
	}

	return 0;
}

int InpdCreate()
{
	if (bDrvOkay == 0) {
		return 1;
	}

	DestroyWindow(hInpdDlg);										// Make sure exitted

	hInpdDlg = FBACreateDialog(hAppInst, MAKEINTRESOURCE(IDD_INPD), hScrnWnd, (DLGPROC)DialogProc);
	if (hInpdDlg == NULL) {
		return 1;
	}

	WndInMid(hInpdDlg, hScrnWnd);
	ShowWindow(hInpdDlg, SW_NORMAL);
	
	return 0;
}



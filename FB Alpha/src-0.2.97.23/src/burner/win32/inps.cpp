// Burner Input Set dialog module
#include "burner.h"

HWND hInpsDlg = NULL;						// Handle to the Input Set Dialog
static HBRUSH hWhiteBGBrush;

unsigned int nInpsInput = 0;				// The input number we are redefining
static struct BurnInputInfo bii;			// Info about the input
static int nDlgState = 0;
static int nInputCode = -1;					// If in state 3, code N was nInputCode
static int nCounter = 0;					// Counter of frames since the dialog was made
static struct GameInp* pgi = NULL;			// Current GameInp
static struct GameInp OldInp;				// Old GameInp
static int bOldPush = 0;					// 1 if the push button was pressed last time

static bool bGrabMouse = false;

static bool bOldLeftAltkeyMapped;

static int InpsInit()
{
	TCHAR szTitle[128];
	memset(&OldInp, 0, sizeof(OldInp));

	pgi = NULL;
	if (nInpsInput >= nGameInpCount + nMacroCount) {		// input out of range
		return 1;
	}
	pgi = GameInp + nInpsInput;

	memset(&bii,0,sizeof(bii));
	BurnDrvGetInputInfo(&bii, nInpsInput);

	if (bii.nType & BIT_GROUP_CONSTANT) {					// This dialog doesn't handle constants
		return 1;
	}

	OldInp = *pgi;
	bOldPush = 0;

	bGrabMouse = false;

	bOldLeftAltkeyMapped = bLeftAltkeyMapped;
	bLeftAltkeyMapped = true;

	// Set the dialog title
	if (nInpsInput >= nGameInpCount) {
		// Macro
		_stprintf(szTitle, FBALoadStringEx(hAppInst, IDS_INPSET_MOVENAME, true), pgi->Macro.szName);
	} else {
		// Normal input
		if (bii.szName == NULL || bii.szName[0] == _T('\0')) {
			_stprintf(szTitle, FBALoadStringEx(hAppInst, IDS_INPSET_MOVE, true));
		} else {
			_stprintf(szTitle, FBALoadStringEx(hAppInst, IDS_INPSET_MOVENAME, true), bii.szName);
		}
	}
	SetWindowText(hInpsDlg, szTitle);

	InputFind(2);

	nDlgState = 4;
	nInputCode = -1;
	nCounter = 0;

	return 0;
}

static int InpsExit()
{
	bOldPush = 0;
	if (pgi != NULL) {
		*pgi=OldInp;
	}
	memset(&OldInp, 0, sizeof(OldInp));

	bLeftAltkeyMapped = bOldLeftAltkeyMapped;

	nDlgState = 0;

	return 0;
}

static int SetInput(int nCode)
{
	if ((pgi->nInput & GIT_GROUP_MACRO) == 0) {

		if (bii.nType & BIT_GROUP_CONSTANT) {							// Don't change dip switches!
			DestroyWindow(hInpsDlg);
			return 0;
		}

		if ((bii.nType & BIT_GROUP_ANALOG) && (nCode & 0xFF) < 0x10) {	// Analog controls
			if (strcmp(bii.szInfo + 4, "-axis-neg") == 0 || strcmp(bii.szInfo + 4, "-axis-pos") == 0) {
				if ((nCode & 0xF000) == 0x4000) {
					if (nCode & 1) {
						pgi->nInput = GIT_JOYAXIS_POS;
					} else {
						pgi->nInput = GIT_JOYAXIS_NEG;
					}
					pgi->Input.JoyAxis.nJoy = (nCode & 0x0F00) >> 8;
					pgi->Input.JoyAxis.nAxis = (nCode & 0x0F) >> 1;
				}
			} else {													// Map entire axis
				if ((nCode & 0xF000) == 0x4000) {
					pgi->nInput = GIT_JOYAXIS_FULL;
					pgi->Input.JoyAxis.nJoy = (nCode & 0x0F00) >> 8;
					pgi->Input.JoyAxis.nAxis = (nCode & 0x0F) >> 1;
				} else {
					pgi->nInput = GIT_MOUSEAXIS;
					pgi->Input.MouseAxis.nMouse = (nCode & 0x0F00) >> 8;
					pgi->Input.MouseAxis.nAxis = (nCode & 0x0F) >> 1;
				}
			}
		} else {
			pgi->nInput = GIT_SWITCH;
			pgi->Input.Switch.nCode = (unsigned short)nCode;
		}
	} else {
		pgi->Macro.nMode = 0x01;										// Mark macro as in use
		pgi->Macro.Switch.nCode = (unsigned short)nCode;				// Assign switch
	}

	OldInp = *pgi;

	InpdListMake(0);													// Update list with new input type

	return 0;
}

static int InpsPushUpdate()
{
	int nPushState = 0;

	if (pgi == NULL || nInpsInput >= nGameInpCount) {
		return 0;
	}

	// See if the push button is pressed
	nPushState = SendDlgItemMessage(hInpsDlg, IDC_INPS_PUSH, BM_GETSTATE, 0, 0);
	if (nPushState & BST_PUSHED) {
		nPushState = 1;
	} else {
		nPushState = 0;
	}

	if (nPushState) {
		switch (OldInp.nType) {
			case BIT_DIGITAL:								// Set digital inputs to 1
				pgi->nInput = GIT_CONSTANT;
				pgi->Input.Constant.nConst = 1;
				break;
			case BIT_DIPSWITCH:								// Set dipswitch block to 0xFF
				pgi->nInput = GIT_CONSTANT;
				pgi->Input.Constant.nConst = 0xFF;
				break;
		}
	} else {
		// Change back
		*pgi = OldInp;
	}
	if (nPushState != bOldPush) {							// refresh view
		InpdListMake(0);
	}

	bOldPush = nPushState;

	return nPushState;
}

static void InpsUpdateControl(int nCode)
{
	TCHAR szString[MAX_PATH] = _T("");
	TCHAR szDevice[MAX_PATH] = _T("");
	TCHAR szControl[MAX_PATH] = _T("");

	_stprintf(szString, _T("%s"), InputCodeDesc(nCode));
	SetWindowText(GetDlgItem(hInpsDlg, IDC_INPS_CONTROL), szString);

	InputGetControlName(nCode, szDevice, szControl);
	_sntprintf(szString, MAX_PATH, _T("%s %s"), szDevice, szControl);
	SetWindowText(GetDlgItem(hInpsDlg, IDC_INPS_CONTROL_NAME), szString);
}

int InpsUpdate()
{
	TCHAR szTemp[MAX_PATH] = _T("");
	int nButtonState;
	int nFind = -1;

	if (hInpsDlg == NULL) {										// Don't do anything if the dialog isn't created
		return 1;
	}
	if (nCounter < 0x100000) {									// advance frames since dialog was created
		nCounter++;
	}

	if (InpsPushUpdate()) {
		return 0;
	}

	nButtonState = SendDlgItemMessage(hInpsDlg, IDC_INPS_GRABMOUSE, BM_GETSTATE, 0, 0);
	if (bGrabMouse) {
		if ((nButtonState & BST_CHECKED) == 0) {
			bGrabMouse = false;
			nDlgState = 2;
			return 0;
		}
	} else {
		if (nButtonState & BST_CHECKED) {
			bGrabMouse = true;
			nDlgState = 4;
			return 0;
		}
	}
	// This doesn't work properly
	if (nButtonState & BST_PUSHED) {
		return 0;
	}

	nButtonState = SendDlgItemMessage(hInpsDlg, IDCANCEL, BM_GETSTATE, 0, 0);
	if (nButtonState & BST_PUSHED) {
		return 0;
	}

	nFind = InputFind(nDlgState);

	if (nDlgState & 4) {										//  4 = Waiting for all controls to be released

		if (bGrabMouse ? nFind >= 0 : (nFind >= 0 && nFind < 0x8000)) {

			if (nCounter >= 60) {

				// Alert the user that a control is stuck

				_stprintf(szTemp, FBALoadStringEx(hAppInst, IDS_INPSET_WAITING, true), InputCodeDesc(nFind));
				SetWindowText(GetDlgItem(hInpsDlg, IDC_INPS_CONTROL), szTemp);

				nCounter = 0;
			}
			return 0;
		}

		// All keys released
		SetWindowText(GetDlgItem(hInpsDlg, IDC_INPS_CONTROL), _T(""));
		SetWindowText(GetDlgItem(hInpsDlg, IDC_INPS_CONTROL_NAME), _T(""));
		nDlgState = 8;
	}

	if (nDlgState & 8) {										//  8 = Waiting for a control to be activated

		if (bGrabMouse ? nFind < 0 : (nFind < 0 || nFind >= 0x8000)) {
			return 0;
		}

		// Key pressed

		nInputCode = nFind;
		InpsUpdateControl(nInputCode);

		nDlgState = 16;
	}

	if (nDlgState & 16) {										// 16 = waiting for control to be released

		if (bGrabMouse ? nFind >= 0 : (nFind >= 0 && nFind < 0x8000)) {
			if (nInputCode != nFind) {
				nInputCode = nFind;
				InpsUpdateControl(nInputCode);
			}
			return 0;
		}

		// Key released
		SetInput(nInputCode);

		nDlgState = 0;
		DestroyWindow(hInpsDlg);								// Quit dialog
	}

	return 0;
}

static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg == WM_INITDIALOG) {
		hInpsDlg = hDlg;
		hWhiteBGBrush = CreateSolidBrush(RGB(0xFF,0xFF,0xFF));
		if (InpsInit()) {
			DestroyWindow(hInpsDlg);
			return FALSE;
		}
		SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hDlg, IDC_INPS_CONTROL), TRUE);
		return FALSE;
	}
	if (Msg == WM_CLOSE) {
		DestroyWindow(hInpsDlg);
		return 0;
	}
	if (Msg == WM_DESTROY) {
		DeleteObject(hWhiteBGBrush);
		InpsExit();
		hInpsDlg = NULL;
		return 0;
	}
	if (Msg == WM_COMMAND) {
		int Id = LOWORD(wParam);
		int Notify = HIWORD(wParam);
		if (Id == IDCANCEL && Notify == BN_CLICKED) {
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			return 0;
		}
	}
	if (Msg == WM_CTLCOLORSTATIC) {
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_INPS_CONTROL)) {
			return (INT_PTR)hWhiteBGBrush;
		}
	}
	return 0;
}

int InpsCreate()
{
	DestroyWindow(hInpsDlg);					// Make sure exitted
	hInpsDlg = NULL;

	hInpsDlg = FBACreateDialog(hAppInst, MAKEINTRESOURCE(IDD_INPS), hInpdDlg, (DLGPROC)DialogProc);
	if (hInpsDlg == NULL) {
		return 1;
	}

	WndInMid(hInpsDlg, hInpdDlg);
	ShowWindow(hInpsDlg, SW_NORMAL);

	return 0;
}

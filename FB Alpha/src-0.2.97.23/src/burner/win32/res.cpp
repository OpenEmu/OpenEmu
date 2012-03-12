#include "burner.h"

#define		HORIZONTAL_ORIENTED_RES		0
#define		VERTICAL_ORIENTED_RES		1
int			nOrientation;

static INT_PTR CALLBACK ResProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM)	// LPARAM lParam
{
	static bool bOK;

	switch (Msg) {
		case WM_INITDIALOG: {
			DEVMODE devMode;
			devMode.dmSize = sizeof(DEVMODE);
			devMode.dmDriverExtra = 0;

			// Run through all possible screenmodes and fill the combobox
			for (int i = 0; EnumDisplaySettings(NULL, i, &devMode); i++) {

				// We're only interested in 16, 24, or 32 bit modes.
				if (devMode.dmBitsPerPel & 0x30) {
					long nItem = 0, nItemValue;
					long nNewRes = (devMode.dmPelsWidth << 16) | devMode.dmPelsHeight;

					// See if the resolution is already in the combobox
					do {
						nItemValue = SendDlgItemMessage(hDlg, IDC_CHOOSE_LIST, CB_GETITEMDATA, nItem, 0);
						nItem++;
					} while ((nItemValue != CB_ERR) && (nItemValue != nNewRes));

					// If not, add it
					if (nItemValue == CB_ERR) {
						TCHAR szTemp[32];
						_stprintf(szTemp, _T(" %li x %li"), devMode.dmPelsWidth, devMode.dmPelsHeight);
						nItem = SendDlgItemMessage(hDlg, IDC_CHOOSE_LIST, CB_ADDSTRING, 0, (LPARAM)&szTemp);
						SendDlgItemMessage(hDlg, IDC_CHOOSE_LIST, CB_SETITEMDATA, nItem, nNewRes);
					}
				}
			}

			SendDlgItemMessage(hDlg, IDC_CHOOSE_LIST, CB_SETCURSEL, 0, 0);

			bOK = 0;
			WndInMid(hDlg, hScrnWnd);

			return TRUE;
		}
		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK) {
				bOK = 1;
				SendMessage(hDlg, WM_CLOSE, 0, 0);
			}
			if (LOWORD(wParam) == IDCANCEL) {
				SendMessage(hDlg, WM_CLOSE, 0, 0);
			}
			break;

		case WM_CLOSE: {

			if (bOK) {
				long nItem = SendDlgItemMessage(hDlg, IDC_CHOOSE_LIST, CB_GETCURSEL, 0, 0);
				long nItemValue = SendDlgItemMessage(hDlg, IDC_CHOOSE_LIST, CB_GETITEMDATA, nItem, 0);

				if(nOrientation == HORIZONTAL_ORIENTED_RES) {
					nVidHorWidth	= nItemValue >> 16;
					nVidHorHeight	= nItemValue & 0xFFFF;
				} 
				if(nOrientation == VERTICAL_ORIENTED_RES)	{
					nVidVerWidth	= nItemValue >> 16;
					nVidVerHeight	= nItemValue & 0xFFFF;
				}
				if(bDrvOkay) {
					if (BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) {
						nVidWidth	= nVidVerWidth;
						nVidHeight	= nVidVerHeight;
					} else {
						nVidWidth	= nVidHorWidth;
						nVidHeight	= nVidHorHeight;
					}
				}
			}

			EndDialog(hDlg, 0);
		}
	}

	return 0;
}


int ResCreate(int nResOrientation)
{
	nOrientation = nResOrientation;

	FBADialogBox(hAppInst,MAKEINTRESOURCE(IDD_CHOOSERES),hScrnWnd,(DLGPROC)ResProc);
	return 0;
}


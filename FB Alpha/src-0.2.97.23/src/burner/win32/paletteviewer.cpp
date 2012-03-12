#include "burner.h"

static HWND hPaletteViewerDlg	= NULL;
static HWND hParent		= NULL;
static HWND PaletteControl[256] = {NULL,};
static HBRUSH PaletteBrush[256] = {NULL,};

static int nPalettePosition;
static int nPaletteEntries;

static void CalcBrushes(int nStartColour)
{
	int Colour, r, g, b;
	
	for (int i = 0; i < 256; i++) {
		DeleteObject(PaletteBrush[i]);
		PaletteBrush[i] = NULL;
		
		if (i + nStartColour < nPaletteEntries) {
			Colour = pBurnDrvPalette[i + nStartColour];
			r = (Colour & 0x7c00) >> 7;
			g = (Colour & 0x3e0) >> 2;
			b = (Colour & 0x1f) << 3;
			PaletteBrush[i] = CreateSolidBrush(RGB(r, g, b));
		}
	}
}

static void UpdateLabels()
{
	TCHAR szItemText[10];
	
	for (int i = 0; i < 16; i++) {
		int nLabel = nPalettePosition + (i * 16);
		szItemText[0] = _T('\0');
		_stprintf(szItemText, _T("%05X"), nLabel);
		SendMessage(GetDlgItem(hPaletteViewerDlg, IDC_GFX_VIEWER_VERT_1 + i), WM_SETTEXT, (WPARAM)0, (LPARAM)szItemText);
	}
}

static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg == WM_INITDIALOG) {
		hPaletteViewerDlg = hDlg;

		if (bDrvOkay) {
			if (!kNetGame && bAutoPause) bRunPause = 1;
			AudSoundStop();
		}
		
		for (int y = 0; y < 16; y++) {
			for (int x = 0; x < 16; x++) {
				PaletteControl[(y * 16) + x] = CreateWindowEx(0, _T("STATIC"), NULL, WS_CHILD | WS_VISIBLE | SS_NOTIFY, (x * 21) + 38, (y * 21) + 21, 20, 20, hPaletteViewerDlg, NULL, NULL, NULL);
			}
		}
		
		nPaletteEntries = BurnDrvGetPaletteEntries();
		nPalettePosition = 0x0000;
		CalcBrushes(nPalettePosition);
		
		WndInMid(hDlg, hParent);
		SetFocus(hDlg);

		return TRUE;
	}
	
	if (Msg == WM_CTLCOLORSTATIC) {
		for (int i = 0; i < 256; i++) {
			if ((HWND)lParam == PaletteControl[i]) {
				return (INT_PTR)PaletteBrush[i];
			}

		}
	}
	
	if (Msg == WM_CLOSE) {
		for (int i = 0; i < 256; i++) {
			DeleteObject(PaletteBrush[i]);
			PaletteBrush[i] = NULL;
			PaletteControl[i] = NULL;
		}
		
		nPalettePosition = 0;
		nPaletteEntries = 0;
		
		EndDialog(hPaletteViewerDlg, 0);
		
		EnableWindow(hScrnWnd, TRUE);
		DestroyWindow(hPaletteViewerDlg);
		
		if (bDrvOkay) {
			if(!bAltPause && bRunPause) bRunPause = 0;
			AudSoundPlay();
		}
		
		return 0;
	}

	if (Msg == WM_COMMAND) {
		int Id = LOWORD(wParam);
		int Notify = HIWORD(wParam);
		
		if (Notify == STN_CLICKED) {
			TCHAR szText[50];
			
			for (int i = 0; i < 256; i++) {
				if ((HWND)lParam == PaletteControl[i]) {
					int Colour, r, g, b;
					
					Colour = pBurnDrvPalette[i + nPalettePosition];
					r = (Colour & 0x7c00) >> 7;
					g = (Colour & 0x3e0) >> 2;
					b = (Colour & 0x1f) << 3;
					
					szText[0] = _T('\0');
					_stprintf(szText, _T("Selected colour: #%05X RGB #%02X%02X%02X"), i + nPalettePosition, r, g, b);
					SendMessage(GetDlgItem(hPaletteViewerDlg, IDC_LABELCOMMENT), WM_SETTEXT, (WPARAM)0, (LPARAM)szText);					
					return 0;
				}
			}
		}
		
		if (Id == IDCANCEL && Notify == BN_CLICKED) {
			SendMessage(hPaletteViewerDlg, WM_CLOSE, 0, 0);
			return 0;
		}
		
		if (Id == IDC_GFX_VIEWER_PREV && Notify == BN_CLICKED) {
			nPalettePosition -= 0x100;
			if (nPalettePosition < 0) nPalettePosition = nPaletteEntries - 0x100;
			CalcBrushes(nPalettePosition);
			RedrawWindow(hPaletteViewerDlg, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
			UpdateLabels();			
			return 0;
		}
		
		if (Id == IDC_GFX_VIEWER_NEXT && Notify == BN_CLICKED) {
			nPalettePosition += 0x100;
			if (nPalettePosition >= nPaletteEntries) nPalettePosition = 0x0000;
			CalcBrushes(nPalettePosition);
			RedrawWindow(hPaletteViewerDlg, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
			UpdateLabels();			
			return 0;
		}
	}
	
	return 0;
}

int PaletteViewerDialogCreate(HWND hParentWND)
{
	if (pBurnDrvPalette == NULL) return 1;
	
	hParent = hParentWND;
	FBADialogBox(hAppInst, MAKEINTRESOURCE(IDD_PALETTEVIEWER), hParent, (DLGPROC)DialogProc);
	
	hParent = NULL;
	hPaletteViewerDlg = NULL;

	return 0;
}

// Display a dialog containing error messages
#include "burner.h"
#include <richedit.h>

static TCHAR* pszBufferEnglish = NULL;
static TCHAR* pszBufferLocal = NULL;

static int nBufferEnglishSize = 0;
static int nBufferLocalSize = 0;

static int nPopupFlags;

static HFONT hEditFont;
static int nIcon;

// ----------------------------------------------------------------------------
// Log the popup message to the debug log/console

static int FBAPopupLog()
{
	TCHAR* pszTypeEnglish;

	switch (nPopupFlags & 7) {
		case PUF_TYPE_ERROR:
			pszTypeEnglish = FBALoadStringEx(hAppInst, IDS_ERR_ERROR, false);
			break;
		case PUF_TYPE_WARNING:
			pszTypeEnglish = FBALoadStringEx(hAppInst, IDS_ERR_WARNING, false);
			break;
		default:
			pszTypeEnglish = FBALoadStringEx(hAppInst, IDS_ERR_INFORMATION, false);
			break;
	}

	dprintf(_T("*** %s message:\n"), pszTypeEnglish);

	for (TCHAR* szText = pszBufferEnglish; ; ) {
		int nLen;

		for (nLen = 0; szText[nLen] && szText[nLen] != _T('\n'); nLen++) { }

		if (nLen) {
			TCHAR szFormat[16];
			_stprintf(szFormat, _T("    %%.%is\n"), nLen);
			dprintf(szFormat, szText);
		}

		if (!szText[nLen]) {
			break;
		}

		szText += nLen + 1;
	}

	return 0;
}

// ----------------------------------------------------------------------------

static INT_PTR CALLBACK FBAPopupProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg) {

		// Grab the handle to the font we need to use
		case WM_SETFONT: {
			hEditFont = (HFONT)wParam;
			break;
		}

		case WM_DRAWITEM: {
			if (((LPDRAWITEMSTRUCT)lParam)->CtlID == IDC_MESSAGE_ICON) {
				DrawIconEx(((LPDRAWITEMSTRUCT)lParam)->hDC, 0, 0, (HICON)LoadImage(NULL, MAKEINTRESOURCE(nIcon), IMAGE_ICON, 0, 0, LR_LOADTRANSPARENT | LR_SHARED), 32, 32, 0, NULL, DI_NORMAL | DI_DEFAULTSIZE);
			}

			break;
		}

		case WM_INITDIALOG: {
			int nBeep = -1;
			TCHAR szCaption[1024] = _T("");
			TCHAR* pszTypeEnglish;
			TCHAR* pszTypeLocal;

			nIcon = 0;

			switch (nPopupFlags & 7) {
				case PUF_TYPE_ERROR:
					nIcon = OIC_ERROR;
					nBeep = MB_ICONHAND;
					pszTypeEnglish = FBALoadStringEx(hAppInst, IDS_ERR_ERROR, false);
					pszTypeLocal = FBALoadStringEx(hAppInst, IDS_ERR_ERROR, true);
					break;
				case PUF_TYPE_WARNING:
					nIcon = OIC_WARNING;
					nBeep = MB_ICONEXCLAMATION;
					pszTypeEnglish = FBALoadStringEx(hAppInst, IDS_ERR_WARNING, false);
					pszTypeLocal = FBALoadStringEx(hAppInst, IDS_ERR_WARNING, true);
					break;
				default:
					nIcon = OIC_INFORMATION;
					nBeep = MB_ICONASTERISK;
					pszTypeEnglish = FBALoadStringEx(hAppInst, IDS_ERR_INFORMATION, false);
					pszTypeLocal = FBALoadStringEx(hAppInst, IDS_ERR_INFORMATION, true);
					break;
			}

			// Set the icon
			SendDlgItemMessage(hDlg, IDC_MESSAGE_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadImage(NULL, MAKEINTRESOURCE(nIcon), IMAGE_ICON, 32, 32, LR_SHARED));

			// Set the caption
			_sntprintf(szCaption, 1024, _T(APP_TITLE) _T(" %s"), pszTypeLocal);
			SetWindowText(hDlg, szCaption);

			{
				SIZE sizee = { 0, 0 };
				SIZE sizel = { 0, 0 };
				POINT p = { 0, 0 };
				RECT rect;
				HWND hWnd;

				{
					hWnd = GetDlgItem(hDlg, IDC_MESSAGE_EDIT_ENG);
					HDC hDC = GetDC(hWnd);
					HFONT hFont = (HFONT)SelectObject(hDC, hEditFont);

					for (TCHAR* szText = pszBufferEnglish; ; ) {

						SIZE line;
						int nLen;

						for (nLen = 0; szText[nLen] && szText[nLen] != _T('\n'); nLen++) { }

						GetTextExtentPoint32(hDC, szText, nLen ? nLen : 1, &line);

						if (sizee.cx < line.cx) {
							sizee.cx = line.cx;
						}
						sizee.cy += line.cy;

						if (!szText[nLen]) {
							break;
						}

						szText += nLen + 1;
					}

					if (bLocalisationActive && pszBufferLocal) {
						for (TCHAR* szText = pszBufferLocal; ; ) {

							SIZE line;
							int nLen;

							for (nLen = 0; szText[nLen] && szText[nLen] != _T('\n'); nLen++) { }

							GetTextExtentPoint32(hDC, szText, nLen ? nLen : 1, &line);

							if (sizel.cx < line.cx) {
								sizel.cx = line.cx;
							}
							sizel.cy += line.cy;

							if (!szText[nLen]) {
								break;
							}

							szText += nLen + 1;
						}
					}

					SelectObject(hDC, hFont);
					ReleaseDC(hWnd, hDC);
				}

				// Set minimum size if needed
				hWnd = GetDlgItem(hDlg, IDOK);
				GetWindowRect(hWnd, &rect);
				if (sizee.cx < (rect.right - rect.left)) {
					sizee.cx = rect.right - rect.left;
				}

				ClientToScreen(hDlg, &p);
				GetWindowRect(hDlg, &rect);

				// Size dialog and edit control containing message text
				if (bLocalisationActive && pszBufferLocal) {

					if (sizel.cx < sizee.cx) {
						sizel.cx = sizee.cx;
					} else {
						sizee.cx = sizel.cx;
					}
					if (sizel.cy < 32) {
						sizel.cy = 32;
					}

					MoveWindow(hDlg, rect.left, rect.top, rect.right - rect.left + sizee.cx, rect.bottom - rect.top + sizee.cy + sizel.cy + 12, FALSE);

					hWnd = GetDlgItem(hDlg, IDC_MESSAGE_EDIT_LOCAL);
					GetWindowRect(hWnd, &rect);
					SetWindowPos(hWnd, NULL, 0, 0, rect.right - rect.left + sizel.cx, rect.bottom - rect.top + sizel.cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW);
					SendMessage(hWnd, WM_SETTEXT, (WPARAM)0, (LPARAM)pszBufferLocal);
					SendMessage(hWnd, EM_SETMARGINS, EC_LEFTMARGIN, 3);
					ShowWindow(hWnd, SW_SHOW);

					hWnd = GetDlgItem(hDlg, IDC_MESSAGE_BACK);
					SetWindowPos(hWnd, NULL, 0, 0, 9999, rect.bottom - p.y + sizel.cy + 6, SWP_NOZORDER | SWP_NOREDRAW);
					ShowWindow(hWnd, SW_SHOW);

					p.y -= rect.bottom - rect.top + sizel.cy + 12;
				} else {
					if (sizee.cy < 32) {
						sizee.cy = 32;
					}

					MoveWindow(hDlg, rect.left, rect.top, rect.right - rect.left + sizee.cx, rect.bottom - rect.top + sizee.cy, FALSE);
				}

				hWnd = GetDlgItem(hDlg, IDC_MESSAGE_EDIT_ENG);
				SendMessage(hWnd, EM_SETBKGNDCOLOR, 0, GetSysColor(COLOR_3DFACE));
				GetWindowRect(hWnd, &rect);
				SetWindowPos(hWnd, NULL, rect.left - p.x, rect.top - p.y, rect.right - rect.left + sizee.cx, rect.bottom - rect.top + sizee.cy, (pszBufferLocal ? 0 : SWP_NOMOVE) | SWP_NOZORDER | SWP_NOREDRAW);
				SendMessage(hWnd, WM_SETTEXT, (WPARAM)0, (LPARAM)pszBufferEnglish);
				SendMessage(hWnd, EM_SETMARGINS, EC_LEFTMARGIN, 3);

				// Center button
				GetClientRect(hDlg, &rect);
				p.x = (rect.right - rect.left) / 2;
				hWnd = GetDlgItem(hDlg, IDOK);
				GetWindowRect(hWnd, &rect);
				SetWindowPos(hWnd, NULL, p.x - (rect.right - rect.left) / 2, rect.top - p.y + sizee.cy, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
			}

			WndInMid(hDlg, hScrnWnd);
			SetForegroundWindow(hDlg);
			MessageBeep(nBeep);

			return TRUE;
		}
		case WM_COMMAND: {
			if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDOK) {
				SendMessage(hDlg, WM_CLOSE, 0, 0);
			}
			break;
		}
		case WM_CLOSE: {
			EndDialog(hDlg, 0);
			break;
		}
	}

	return 0;
}

// ----------------------------------------------------------------------------

int FBAPopupAddText(int nFlags, TCHAR* pszFormat, ...)
{
	TCHAR szString[1024] = _T("");
	TCHAR* pszStringEnglish;

	va_list vaEnglish;
	va_start(vaEnglish, pszFormat);

	if (IS_INTRESOURCE(pszFormat)) {
		pszStringEnglish = FBALoadStringEx(hAppInst, (INT_PTR)pszFormat, false);
	} else {
		pszStringEnglish = pszFormat;
	}

	// See if we need to load strings from reources
	if (nFlags & PUF_TEXT_TRANSLATE) {
		TCHAR* pszStringLocal;

		va_list vaLocal;
		va_start(vaLocal, pszFormat);

		if (IS_INTRESOURCE(pszFormat)) {
			pszStringLocal = FBALoadStringEx(hAppInst, (INT_PTR)pszFormat, true);
		} else {
			pszStringLocal = pszFormat;
		}

		// Add the translated string if present
		if (bLocalisationActive && (nFlags & PUF_TEXT_TRANSLATE) &&  pszStringLocal && *pszStringLocal) {
			// Add the translated string
			int nLen = _vsntprintf(szString, 1024, pszStringLocal, vaLocal);
			if (nLen > 0) {
				TCHAR* pszNewBuffer = (TCHAR*)realloc(pszBufferLocal, (nLen + nBufferLocalSize + 1) * sizeof(TCHAR));
				if (pszNewBuffer) {
					pszBufferLocal = pszNewBuffer;
					_tcsncpy(pszBufferLocal + nBufferLocalSize, szString, nLen);
					nBufferLocalSize += nLen;
					pszBufferLocal[nBufferLocalSize] = 0;
				}
			}
		}

		va_end(vaLocal);
	}

	// Add the English language string
	if (pszStringEnglish && *pszStringEnglish) {
		int nLen = _vsntprintf(szString, 1024, pszStringEnglish, vaEnglish);
		if (nLen > 0) {
			TCHAR* pszNewBuffer = (TCHAR*)realloc(pszBufferEnglish, (nLen + nBufferEnglishSize + 1) * sizeof(TCHAR));
			if (pszNewBuffer) {
				pszBufferEnglish = pszNewBuffer;
				_tcsncpy(pszBufferEnglish + nBufferEnglishSize, szString, nLen);
				nBufferEnglishSize += nLen;
				pszBufferEnglish[nBufferEnglishSize] = 0;
			}
		}
	}

	va_end(vaEnglish);

	return 0;
}

int FBAPopupDestroyText()
{
	if (pszBufferEnglish) {
		free(pszBufferEnglish);
		pszBufferEnglish = NULL;
	}
	nBufferEnglishSize = 0;

	if (pszBufferLocal) {
		free(pszBufferLocal);
		pszBufferLocal = NULL;
	}
	nBufferLocalSize = 0;

	return 0;
}

// ----------------------------------------------------------------------------

int FBAPopupDisplay(int nFlags)
{
	HMODULE hRiched = NULL;
	
#if defined (_UNICODE)
	hRiched = LoadLibrary(L"RICHED20.DLL");
#else
	hRiched = LoadLibrary("RICHED20.DLL");
#endif

	nPopupFlags = nFlags;

	SplashDestroy(true);

	FBAPopupLog();

	if (!(nPopupFlags & PUF_TYPE_LOGONLY) && hRiched) {
		DialogBox(hAppInst, MAKEINTRESOURCE(IDD_POPUP), hScrnWnd, (DLGPROC)FBAPopupProc);
		FreeLibrary(hRiched);
		hRiched = NULL;
	}

	FBAPopupDestroyText();

	return 1;
}

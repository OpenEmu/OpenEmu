#include "burner.h"
#include <process.h>

static HWND hProgressDlg = NULL;
static HANDLE hProgressThread = NULL;
static unsigned ProgressThreadID = 0;

int nProgressPosBurn, nProgressPosBurner;
int nProgressMin, nProgressMax;

static HANDLE hEvent = NULL;

// ----------------------------------------------------------------------------
// Callbacks for burn library

static int __cdecl ProgressSetRangeBurn(double dProgressRange)
{
	if (hProgressThread == NULL || hProgressDlg == NULL) {
		return 1;
	}

	nProgressMin = -(int)((double)nProgressMax * dProgressRange);
	SendDlgItemMessage(hProgressDlg, IDC_WAIT_PROG, PBM_SETRANGE32, nProgressMin, nProgressMax);

	return 0;
}

static int __cdecl ProgressUpdateBurn(double dProgress, const TCHAR* pszText, bool bAbs)
{
	if (hProgressThread == NULL || hProgressDlg == NULL) {
		return 1;
	}

	if (pszText) {
		SendDlgItemMessage(hProgressDlg, IDC_WAIT_LABEL_B2, WM_SETTEXT, 0, (LPARAM)pszText);
		SetWindowText(hScrnWnd, pszText);
	}

	if (bAbs) {
		nProgressPosBurn = (int)((double)(-nProgressMin) * dProgress);
		if (nProgressPosBurn > -nProgressMin) {
			nProgressPosBurn = -nProgressMin;
		}
		SendDlgItemMessage(hProgressDlg, IDC_WAIT_PROG, PBM_SETPOS, (WPARAM)(nProgressMin + nProgressPosBurn + nProgressPosBurner), 0);
	} else {
		if (dProgress) {
			nProgressPosBurn += (int)((double)(-nProgressMin) * dProgress);
			if (nProgressPosBurn > -nProgressMin) {
				nProgressPosBurn = -nProgressMin;
			}
			SendDlgItemMessage(hProgressDlg, IDC_WAIT_PROG, PBM_SETPOS, (WPARAM)(nProgressMin + nProgressPosBurn + nProgressPosBurner), 0);
		}
	}

	return 0;
}

// ----------------------------------------------------------------------------
// Callback for burner

int ProgressUpdateBurner(double dProgress, const TCHAR* pszText, bool bAbs)
{
	if (hProgressThread == NULL || hProgressDlg == NULL) {
		return 1;
	}

	if (pszText) {
		SendDlgItemMessage(hProgressDlg, IDC_WAIT_LABEL_B2, WM_SETTEXT, 0, (LPARAM)pszText);
		SetWindowText(hScrnWnd, pszText);
	}

	if (bAbs) {
		nProgressPosBurner = (int)((double)nProgressMax * dProgress);
		if (nProgressPosBurner > nProgressMax) {
			nProgressPosBurner = nProgressMax;
		}
		SendDlgItemMessage(hProgressDlg, IDC_WAIT_PROG, PBM_SETPOS, (WPARAM)(nProgressMin + nProgressPosBurn + nProgressPosBurner), 0);
	} else {
		if (dProgress) {
			if (nProgressPosBurner > nProgressMax) {
				nProgressPosBurner = nProgressMax;
			}
			nProgressPosBurner += (int)((double)nProgressMax * dProgress);
			SendDlgItemMessage(hProgressDlg, IDC_WAIT_PROG, PBM_SETPOS, (WPARAM)(nProgressMin + nProgressPosBurn + nProgressPosBurner), 0);
		}
	}

	return 0;
}

// ----------------------------------------------------------------------------
// Code that runs in a seperate thread

static INT_PTR CALLBACK ProgressProc(HWND hDlg, UINT Msg, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (Msg == WM_INITDIALOG) {
		TCHAR szText[128] = _T("");

		hProgressDlg = hDlg;

		SendDlgItemMessage(hDlg, IDC_WAIT_PROG, PBM_SETRANGE32, nProgressMin, nProgressMax);

		_stprintf(szText, FBALoadStringEx(hAppInst, IDS_PROGRESS_LOADING, true), BurnDrvGetText(DRV_NAME));
		SendDlgItemMessage(hDlg, IDC_WAIT_LABEL_B1, WM_SETTEXT, 0, (LPARAM)szText);

		ShowWindow(GetDlgItem(hDlg, IDC_WAIT_LABEL_B1), TRUE);
		ShowWindow(GetDlgItem(hDlg, IDC_WAIT_LABEL_B2), TRUE);

		WndInMid(hDlg, hScrnWnd);
		SetForegroundWindow(hDlg);
		SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

		return TRUE;
	}

	return 0;
}

static unsigned __stdcall DoProgress(void*)
{
	MSG msg;

	// Raise the thread priority for this thread
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

	nProgressMin = 0;
	nProgressMax = 1 << 30;

	nProgressPosBurn = 0;
	nProgressPosBurner = 0;

	BurnExtProgressRangeCallback = ProgressSetRangeBurn;
	BurnExtProgressUpdateCallback = ProgressUpdateBurn;

	FBACreateDialog(hAppInst, MAKEINTRESOURCE(IDD_WAIT), NULL, (DLGPROC)ProgressProc);

	if (hEvent) {
		SetEvent(hEvent);
	}

	while (GetMessage(&msg, NULL, 0, 0)) {

		// See if we need to end the thread
		if (msg.message == (WM_APP + 0)) {
			break;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	DestroyWindow(hProgressDlg);

	return 0;
}

// ----------------------------------------------------------------------------

int ProgressCreate()
{
	if (hProgressDlg || hProgressThread) {
		return 1;
	}

	hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	hProgressThread = (HANDLE)_beginthreadex(NULL, 0, DoProgress, NULL, 0, &ProgressThreadID);

	WaitForSingleObject(hEvent, 15000);

	CloseHandle(hEvent);
	hEvent = NULL;

	return 0;
}

int ProgressDestroy()
{
	if (hProgressThread) {

		// Signal the prgress thread to end
		PostThreadMessage(ProgressThreadID, WM_APP + 0, 0, 0);

		// Wait for the thread to finish
		if (WaitForSingleObject(hProgressThread, 15000) != WAIT_OBJECT_0) {
			// If the thread doesn't finish within 15 seconds, forcibly kill it
			TerminateThread(hProgressThread, 1);
		}

		BurnExtProgressRangeCallback = NULL;
		BurnExtProgressUpdateCallback = NULL;

		hProgressDlg = NULL;

		CloseHandle(hProgressThread);

		hProgressThread = NULL;
		ProgressThreadID = 0;
	}

	return 0;
}

#include "burner.h"
#include <ctype.h>

const double PI = 3.14159265358979323846f;			// Pi
const double DEGTORAD = 0.01745329251994329547f;	// Degrees to Radians
const double RADTODEG = 57.29577951308232286465f;	// Radians to Degrees

static int nExitStatus;

// -----------------------------------------------------------------------------

static INT_PTR CALLBACK DefInpProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM)
{
 	int nRet = 0;
	BOOL fTrue = FALSE;

	switch (Msg) {
		case WM_INITDIALOG:
			nRet = nAudSegCount;
			SetWindowText(hDlg, FBALoadStringEx(hAppInst, IDS_NUMDIAL_NUM_FRAMES, true));
			SetDlgItemInt(hDlg, IDC_VALUE_EDIT, nRet, TRUE);
			return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == ID_VALUE_CLOSE) {
				SendMessage(hDlg, WM_CLOSE, 0, 0);
			} else {
				if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDCANCEL) {
					SendMessage(hDlg, WM_CLOSE, 0, 0);
				}
			}
			break;

		case WM_CLOSE:
			nRet = GetDlgItemInt(hDlg, IDC_VALUE_EDIT, &fTrue, TRUE);
			if (fTrue) {
				nAudSegCount = nRet;
			}
			EndDialog(hDlg, 0);
	}

	return 0;
}

int NumDialCreate(int)
{
	FBADialogBox(hAppInst, MAKEINTRESOURCE(IDD_VALUE), hScrnWnd, (DLGPROC)DefInpProc);

	return 1;
}

// -----------------------------------------------------------------------------
// Gamma dialog

static INT_PTR CALLBACK GammaProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM)	// LPARAM lParam
{
	static double nPrevGamma;

	switch (Msg) {
		case WM_INITDIALOG: {
			TCHAR szText[16];
			int nPos;

			nPrevGamma = nGamma;
			nExitStatus = 0;

			WndInMid(hDlg, hScrnWnd);

			// Initialise gamma slider
			SendDlgItemMessage(hDlg, IDC_GAMMA_SLIDER, TBM_SETRANGE, (WPARAM)0, (LPARAM)MAKELONG(1, 20000));
			SendDlgItemMessage(hDlg, IDC_GAMMA_SLIDER, TBM_SETLINESIZE, (WPARAM)0, (LPARAM)200);
			SendDlgItemMessage(hDlg, IDC_GAMMA_SLIDER, TBM_SETPAGESIZE, (WPARAM)0, (LPARAM)250);
			SendDlgItemMessage(hDlg, IDC_GAMMA_SLIDER, TBM_SETTIC, (WPARAM)0, (LPARAM)7500);
			SendDlgItemMessage(hDlg, IDC_GAMMA_SLIDER, TBM_SETTIC, (WPARAM)0, (LPARAM)10001);
			SendDlgItemMessage(hDlg, IDC_GAMMA_SLIDER, TBM_SETTIC, (WPARAM)0, (LPARAM)12500);

			// Set slider to value of nGamma
			if (nGamma > 1.0) {		// Gamma > 1
				nPos = int((nGamma - 1.0) * 10000.0 + 10001.0);
			} else {				// Gamma < 1
				nPos = int(10000.0f - ((1.0 / nGamma) * 10000.0 - 10000.0));
			}
			SendDlgItemMessage(hDlg, IDC_GAMMA_SLIDER, TBM_SETPOS, (WPARAM)true, (LPARAM)nPos);

			// Set the edit control to the value of nGamma
			_stprintf(szText, _T("%0.2f"), nGamma);
			SendDlgItemMessage(hDlg, IDC_GAMMA_EDIT, WM_SETTEXT, (WPARAM)0, (LPARAM)szText);

			// Update the screen
			ComputeGammaLUT();
			if (bVidOkay) {
				VidRedraw();
				VidPaint(0);
			}

			return TRUE;
		}
		case WM_COMMAND: {
			switch (HIWORD(wParam)) {
				case BN_CLICKED: {
					if (LOWORD(wParam) == IDOK) {
						nExitStatus = 1;
						SendMessage(hDlg, WM_CLOSE, 0, 0);
					}
					if (LOWORD(wParam) == IDCANCEL) {
						nExitStatus = -1;
						SendMessage(hDlg, WM_CLOSE, 0, 0);
					}
					break;
				}
				case EN_UPDATE: {
					if (nExitStatus == 0) {
						TCHAR szText[16] = _T("");
						bool bPoint = 0;
						bool bValid = 1;

						if (SendDlgItemMessage(hDlg, IDC_GAMMA_EDIT, WM_GETTEXTLENGTH, (WPARAM)0, (LPARAM)0) < 16) {
							SendDlgItemMessage(hDlg, IDC_GAMMA_EDIT, WM_GETTEXT, (WPARAM)16, (LPARAM)szText);
						}

						// Scan string in the edit control for illegal characters
						for (int i = 0; szText[i]; i++) {
							if (szText[i] == _T('.')) {
								if (bPoint) {
									bValid = 0;
									break;
								}
							} else {
								if (!_istdigit(szText[i])) {
									bValid = 0;
									break;
								}
							}
						}

						if (bValid) {
							int nPos;

							nGamma = _tcstod(szText, NULL);

							// Set slider to value of nGamma
							if (nGamma > 1.0) {		// Gamma > 1
								nPos = int((nGamma - 1.0) * 10000.0 + 10001.0);
							} else {				// Gamma < 1
								nPos = int(10000.0 - ((1.0 / nGamma) * 10000.0 - 10000.0));
							}
							SendDlgItemMessage(hDlg, IDC_GAMMA_SLIDER, TBM_SETPOS, (WPARAM)true, (LPARAM)nPos);

							// Update the screen
							ComputeGammaLUT();
							if (bVidOkay) {
								VidRedraw();
								VidPaint(0);
							}
						}
					}
					break;
				}
				break;
			}
		}

		case WM_HSCROLL: {
			switch (LOWORD(wParam)) {
				case TB_BOTTOM:
				case TB_ENDTRACK:
				case TB_LINEDOWN:
				case TB_LINEUP:
				case TB_PAGEDOWN:
				case TB_PAGEUP:
				case TB_THUMBPOSITION:
				case TB_THUMBTRACK:
				case TB_TOP: {
					if (nExitStatus == 0) {
						TCHAR szText[16];
						int nPos;

						// Update the contents of the edit control
						nPos = SendDlgItemMessage(hDlg, IDC_GAMMA_SLIDER, TBM_GETPOS, (WPARAM)0, (LPARAM)0);
						if (nPos > 10000) {	// Gamma > 1
							nGamma = 1.0 + (nPos - 10000.0) / 10000.0;
						} else {			// Gamma < 1
							nGamma = 1.0 / (1.0 + ((10001.0 - nPos) / 10000.0));
						}
						_stprintf(szText, _T("%0.2f"), nGamma);
						SendDlgItemMessage(hDlg, IDC_GAMMA_EDIT, WM_SETTEXT, (WPARAM)0, (LPARAM)szText);

						// Update the screen
						ComputeGammaLUT();
						if (bVidOkay) {
							VidRedraw();
							VidPaint(0);
						}
					}
					break;
				}
			}
			break;
		}

		case WM_CLOSE:
			if (nExitStatus != 1) {
				nGamma = nPrevGamma;
			}
			EndDialog(hDlg, 0);
	}

	return 0;
}

void GammaDialog()
{
	FBADialogBox(hAppInst, MAKEINTRESOURCE(IDD_GAMMA), hScrnWnd, (DLGPROC)GammaProc);
}

// -----------------------------------------------------------------------------
// Scanline intensity dialog

static INT_PTR CALLBACK ScanlineProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM)	// LPARAM lParam
{
	static int nPrevIntensity;

	switch (Msg) {
		case WM_INITDIALOG: {
			TCHAR szText[16];
			nPrevIntensity = nVidScanIntensity;
			nExitStatus = 0;

			WndInMid(hDlg, hScrnWnd);

			// Initialise slider
			SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_SETRANGE, (WPARAM)0, (LPARAM)MAKELONG(0, 255));
			SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_SETPAGESIZE, (WPARAM)0, (LPARAM)16);
			SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_SETTIC, (WPARAM)0, (LPARAM)191);
			SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_SETTIC, (WPARAM)0, (LPARAM)127);
			SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_SETTIC, (WPARAM)0, (LPARAM)63);

			// Set slider to current value
			SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_SETPOS, (WPARAM)true, (LPARAM)nVidScanIntensity & 0xFF);

			// Set the edit control to current value
			_stprintf(szText, _T("%i"), nVidScanIntensity & 0xFF);
			SendDlgItemMessage(hDlg, IDC_SCANLINE_EDIT, WM_SETTEXT, (WPARAM)0, (LPARAM)szText);

			// Update the screen
			if (bDrvOkay) {
				VidPaint(2);
			}

			return TRUE;
		}
		case WM_COMMAND: {
			switch (HIWORD(wParam)) {
				case BN_CLICKED: {
					if (LOWORD(wParam) == IDOK) {
						nExitStatus = 1;
						SendMessage(hDlg, WM_CLOSE, 0, 0);
					}
					if (LOWORD(wParam) == IDCANCEL) {
						nExitStatus = -1;
						SendMessage(hDlg, WM_CLOSE, 0, 0);
					}
					break;
				}
				case EN_UPDATE: {
					if (nExitStatus == 0) {
						TCHAR szText[16] = _T("");
						bool bValid = 1;

						if (SendDlgItemMessage(hDlg, IDC_SCANLINE_EDIT, WM_GETTEXTLENGTH, (WPARAM)0, (LPARAM)0) < 16) {
							SendDlgItemMessage(hDlg, IDC_SCANLINE_EDIT, WM_GETTEXT, (WPARAM)16, (LPARAM)szText);
						}

						// Scan string in the edit control for illegal characters
						for (int i = 0; szText[i]; i++) {
							if (!_istdigit(szText[i])) {
								bValid = 0;
								break;
							}
						}

						if (bValid) {
							nVidScanIntensity = _tcstol(szText, NULL, 0);
							if (nVidScanIntensity < 0) {
								nVidScanIntensity = 0;
							} else {
								if (nVidScanIntensity > 255) {
									nVidScanIntensity = 255;
								}
							}

							// Set slider to current value
							SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_SETPOS, (WPARAM)true, (LPARAM)nVidScanIntensity);

							nVidScanIntensity |= (nVidScanIntensity << 8) | (nVidScanIntensity << 16);

							// Update the screen
							if (bVidOkay) {
								VidPaint(2);
							}
						}
					}
					break;
				}
			}
			break;
		}

		case WM_HSCROLL: {
			switch (LOWORD(wParam)) {
				case TB_BOTTOM:
				case TB_ENDTRACK:
				case TB_LINEDOWN:
				case TB_LINEUP:
				case TB_PAGEDOWN:
				case TB_PAGEUP:
				case TB_THUMBPOSITION:
				case TB_THUMBTRACK:
				case TB_TOP: {
					if (nExitStatus == 0) {
						TCHAR szText[16];

						// Update the contents of the edit control
						nVidScanIntensity = SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_GETPOS, (WPARAM)0, (LPARAM)0);
						_stprintf(szText, _T("%i"), nVidScanIntensity);
						SendDlgItemMessage(hDlg, IDC_SCANLINE_EDIT, WM_SETTEXT, (WPARAM)0, (LPARAM)szText);

						nVidScanIntensity |= (nVidScanIntensity << 8) | (nVidScanIntensity << 16);
						// Update the screen
						if (bVidOkay) {
//							VidRedraw();
							VidPaint(2);
						}
					}
					break;
				}
			}
			break;
		}

		case WM_CLOSE:
			if (nExitStatus != 1) {
				nVidScanIntensity = nPrevIntensity;
			}
			EndDialog(hDlg, 0);
			break;
	}

	return 0;
}

void ScanlineDialog()
{
	FBADialogBox(hAppInst, MAKEINTRESOURCE(IDD_SCANLINE), hScrnWnd, (DLGPROC)ScanlineProc);
}

// -----------------------------------------------------------------------------
// Feedback intensity dialog
static INT_PTR CALLBACK PhosphorProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)	// LPARAM lParam
{
	static int nPrevIntensity;
	static int nPrevSaturation;

	switch (Msg) {
		case WM_INITDIALOG: {
			TCHAR szText[16];
			nPrevIntensity = nVidFeedbackIntensity;
			nPrevSaturation = nVidFeedbackOverSaturation;
			nExitStatus = 0;

			WndInMid(hDlg, hScrnWnd);

			// Initialise sliders
			SendDlgItemMessage(hDlg, IDC_PHOSPHOR_1_SLIDER, TBM_SETRANGE, (WPARAM)0, (LPARAM)MAKELONG(0, 255));
			SendDlgItemMessage(hDlg, IDC_PHOSPHOR_1_SLIDER, TBM_SETPAGESIZE, (WPARAM)0, (LPARAM)8);
			SendDlgItemMessage(hDlg, IDC_PHOSPHOR_1_SLIDER, TBM_SETTIC, (WPARAM)0, (LPARAM)127);
			SendDlgItemMessage(hDlg, IDC_PHOSPHOR_1_SLIDER, TBM_SETTIC, (WPARAM)0, (LPARAM)63);
			SendDlgItemMessage(hDlg, IDC_PHOSPHOR_1_SLIDER, TBM_SETTIC, (WPARAM)0, (LPARAM)31);
			SendDlgItemMessage(hDlg, IDC_PHOSPHOR_1_SLIDER, TBM_SETTIC, (WPARAM)0, (LPARAM)15);

			SendDlgItemMessage(hDlg, IDC_PHOSPHOR_2_SLIDER, TBM_SETRANGE, (WPARAM)0, (LPARAM)MAKELONG(0, 127));
			SendDlgItemMessage(hDlg, IDC_PHOSPHOR_2_SLIDER, TBM_SETPAGESIZE, (WPARAM)0, (LPARAM)4);
			SendDlgItemMessage(hDlg, IDC_PHOSPHOR_2_SLIDER, TBM_SETTIC, (WPARAM)0, (LPARAM)63);
			SendDlgItemMessage(hDlg, IDC_PHOSPHOR_2_SLIDER, TBM_SETTIC, (WPARAM)0, (LPARAM)31);
			SendDlgItemMessage(hDlg, IDC_PHOSPHOR_2_SLIDER, TBM_SETTIC, (WPARAM)0, (LPARAM)15);

			// Set slider to current values
			SendDlgItemMessage(hDlg, IDC_PHOSPHOR_1_SLIDER, TBM_SETPOS, (WPARAM)true, (LPARAM)nVidFeedbackIntensity);
			SendDlgItemMessage(hDlg, IDC_PHOSPHOR_2_SLIDER, TBM_SETPOS, (WPARAM)true, (LPARAM)nVidFeedbackOverSaturation);

			// Set the edit control to current values
			_stprintf(szText, _T("%i"), nVidFeedbackIntensity);
			SendDlgItemMessage(hDlg, IDC_PHOSPHOR_1_EDIT, WM_SETTEXT, (WPARAM)0, (LPARAM)szText);
			_stprintf(szText, _T("%i"), nVidFeedbackOverSaturation);
			SendDlgItemMessage(hDlg, IDC_PHOSPHOR_2_EDIT, WM_SETTEXT, (WPARAM)0, (LPARAM)szText);

			return TRUE;
		}
		case WM_COMMAND: {
			switch (HIWORD(wParam)) {
				case BN_CLICKED: {
					if (LOWORD(wParam) == IDOK) {
						nExitStatus = 1;
						SendMessage(hDlg, WM_CLOSE, 0, 0);
					}
					if (LOWORD(wParam) == IDCANCEL) {
						nExitStatus = -1;
						SendMessage(hDlg, WM_CLOSE, 0, 0);
					}
					break;
				}
				case EN_UPDATE: {
					if (nExitStatus == 0) {
						TCHAR szText[16] = _T("");
						bool bValid = 1;

						switch (LOWORD(wParam)) {
							case IDC_PHOSPHOR_1_EDIT:
								if (SendDlgItemMessage(hDlg, IDC_PHOSPHOR_1_EDIT, WM_GETTEXTLENGTH, (WPARAM)0, (LPARAM)0) < 16) {
									SendDlgItemMessage(hDlg, IDC_PHOSPHOR_1_EDIT, WM_GETTEXT, (WPARAM)16, (LPARAM)szText);
								}

								// Scan string in the edit control for illegal characters
								for (int i = 0; szText[i]; i++) {
									if (!_istdigit(szText[i])) {
										bValid = 0;
										break;
									}
								}

								if (bValid) {
									nVidFeedbackIntensity = _tcstol(szText, NULL, 0);
									if (nVidFeedbackIntensity < 0) {
										nVidFeedbackIntensity = 0;
									} else {
										if (nVidFeedbackIntensity > 255) {
											nVidFeedbackIntensity = 255;
										}
									}

									// Set slider to current value
									SendDlgItemMessage(hDlg, IDC_PHOSPHOR_1_SLIDER, TBM_SETPOS, (WPARAM)true, (LPARAM)nVidFeedbackIntensity);
								}
								break;
							case IDC_PHOSPHOR_2_EDIT:
								if (SendDlgItemMessage(hDlg, IDC_PHOSPHOR_2_EDIT, WM_GETTEXTLENGTH, (WPARAM)0, (LPARAM)0) < 16) {
									SendDlgItemMessage(hDlg, IDC_PHOSPHOR_2_EDIT, WM_GETTEXT, (WPARAM)16, (LPARAM)szText);
								}

								// Scan string in the edit control for illegal characters
								for (int i = 0; szText[i]; i++) {
									if (!_istdigit(szText[i])) {
										bValid = 0;
										break;
									}
								}

								if (bValid) {
									nVidFeedbackOverSaturation = _tcstol(szText, NULL, 0);
									if (nVidFeedbackOverSaturation < 0) {
										nVidFeedbackOverSaturation = 0;
									} else {
										if (nVidFeedbackOverSaturation > 255) {
											nVidFeedbackOverSaturation = 255;
										}
									}

									// Set slider to current value
									SendDlgItemMessage(hDlg, IDC_PHOSPHOR_2_SLIDER, TBM_SETPOS, (WPARAM)true, (LPARAM)nVidFeedbackOverSaturation);

									// Update the screen
									if (bVidOkay) {
										VidPaint(2);
									}
								}
								break;
						}
					}
					break;
				}
			}
			break;
		}

		case WM_HSCROLL: {
			switch (LOWORD(wParam)) {
				case TB_BOTTOM:
				case TB_ENDTRACK:
				case TB_LINEDOWN:
				case TB_LINEUP:
				case TB_PAGEDOWN:
				case TB_PAGEUP:
				case TB_THUMBPOSITION:
				case TB_THUMBTRACK:
				case TB_TOP: {
					if (nExitStatus == 0) {
						TCHAR szText[16];

						// Update the contents of the edit control
						switch (GetDlgCtrlID((HWND)lParam)) {
							case IDC_PHOSPHOR_1_SLIDER:
								nVidFeedbackIntensity = SendDlgItemMessage(hDlg, IDC_PHOSPHOR_1_SLIDER, TBM_GETPOS, (WPARAM)0, (LPARAM)0);
								_stprintf(szText, _T("%i"), nVidFeedbackIntensity);
								SendDlgItemMessage(hDlg, IDC_PHOSPHOR_1_EDIT, WM_SETTEXT, (WPARAM)0, (LPARAM)szText);
								break;
							case IDC_PHOSPHOR_2_SLIDER:
								nVidFeedbackOverSaturation = SendDlgItemMessage(hDlg, IDC_PHOSPHOR_2_SLIDER, TBM_GETPOS, (WPARAM)0, (LPARAM)0);
								_stprintf(szText, _T("%i"), nVidFeedbackOverSaturation);
								SendDlgItemMessage(hDlg, IDC_PHOSPHOR_2_EDIT, WM_SETTEXT, (WPARAM)0, (LPARAM)szText);
								break;
						}
					}
					break;
				}
			}
			break;
		}

		case WM_CLOSE:
			if (nExitStatus != 1) {
				nVidFeedbackIntensity = nPrevIntensity;
				nVidFeedbackOverSaturation = nPrevSaturation;
			}
			EndDialog(hDlg, 0);
			break;
	}

	return 0;
}

void PhosphorDialog()
{
	FBADialogBox(hAppInst, MAKEINTRESOURCE(IDD_PHOSPHOR), hScrnWnd, (DLGPROC)PhosphorProc);
}

// -----------------------------------------------------------------------------
// Screen Angle dialog
static INT_PTR CALLBACK ScreenAngleProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	static float fPrevScreenAngle, fPrevScreenCurvature;
	static HWND hScreenAngleSlider, hScreenAngleEdit;

	switch (Msg) {
		case WM_INITDIALOG: {
			TCHAR szText[16];
			fPrevScreenAngle = fVidScreenAngle;
			fPrevScreenCurvature = fVidScreenCurvature;
			hScreenAngleSlider = GetDlgItem(hDlg, IDC_SCREENANGLE_SLIDER);
			hScreenAngleEdit = GetDlgItem(hDlg, IDC_SCREENANGLE_EDIT);
			nExitStatus = 0;

			WndInMid(hDlg, hScrnWnd);

			// Initialise sliders
			SendDlgItemMessage(hDlg, IDC_SCREENANGLE_SLIDER, TBM_SETRANGE, (WPARAM)0, (LPARAM)MAKELONG(0, 12000));
			SendDlgItemMessage(hDlg, IDC_SCREENANGLE_SLIDER, TBM_SETLINESIZE, (WPARAM)0, (LPARAM)100);
			SendDlgItemMessage(hDlg, IDC_SCREENANGLE_SLIDER, TBM_SETPAGESIZE, (WPARAM)0, (LPARAM)250);
			SendDlgItemMessage(hDlg, IDC_SCREENANGLE_SLIDER, TBM_SETTIC, (WPARAM)0, (LPARAM)1000);
			SendDlgItemMessage(hDlg, IDC_SCREENANGLE_SLIDER, TBM_SETTIC, (WPARAM)0, (LPARAM)2250);
			SendDlgItemMessage(hDlg, IDC_SCREENANGLE_SLIDER, TBM_SETTIC, (WPARAM)0, (LPARAM)6000);

			SendDlgItemMessage(hDlg, IDC_SCREENCURVATURE_SLIDER, TBM_SETRANGE, (WPARAM)0, (LPARAM)MAKELONG(0, 12000));
			SendDlgItemMessage(hDlg, IDC_SCREENCURVATURE_SLIDER, TBM_SETLINESIZE, (WPARAM)0, (LPARAM)150);
			SendDlgItemMessage(hDlg, IDC_SCREENCURVATURE_SLIDER, TBM_SETPAGESIZE, (WPARAM)0, (LPARAM)375);
			SendDlgItemMessage(hDlg, IDC_SCREENCURVATURE_SLIDER, TBM_SETTIC, (WPARAM)0, (LPARAM)3000);
			SendDlgItemMessage(hDlg, IDC_SCREENCURVATURE_SLIDER, TBM_SETTIC, (WPARAM)0, (LPARAM)4500);
			SendDlgItemMessage(hDlg, IDC_SCREENCURVATURE_SLIDER, TBM_SETTIC, (WPARAM)0, (LPARAM)6000);

			// Set sliders to current value
			SendDlgItemMessage(hDlg, IDC_SCREENANGLE_SLIDER, TBM_SETPOS, (WPARAM)true, (LPARAM)(fVidScreenAngle * RADTODEG * 100.0f));
			SendDlgItemMessage(hDlg, IDC_SCREENCURVATURE_SLIDER, TBM_SETPOS, (WPARAM)true, (LPARAM)(fVidScreenCurvature * RADTODEG * 150.0f));

			// Set the edit controls to current value
			_stprintf(szText, _T("%0.2f"), fVidScreenAngle * RADTODEG);
			SendDlgItemMessage(hDlg, IDC_SCREENANGLE_EDIT, WM_SETTEXT, (WPARAM)0, (LPARAM)szText);

			_stprintf(szText, _T("%0.2f"), fVidScreenCurvature * RADTODEG);
			SendDlgItemMessage(hDlg, IDC_SCREENCURVATURE_EDIT, WM_SETTEXT, (WPARAM)0, (LPARAM)szText);

			// Update the screen
			if (bVidOkay) {
				VidPaint(2);
			}

			return TRUE;
		}
		case WM_COMMAND: {
			switch (HIWORD(wParam)) {
				case BN_CLICKED: {
					if (LOWORD(wParam) == IDOK) {
						nExitStatus = 1;
						SendMessage(hDlg, WM_CLOSE, 0, 0);
					}
					if (LOWORD(wParam) == IDCANCEL) {
						nExitStatus = -1;
						SendMessage(hDlg, WM_CLOSE, 0, 0);
					}
					break;
				}
				case EN_UPDATE: {
					if (nExitStatus == 0) {
						TCHAR szText[16] = _T("");
						bool bPoint = 0;
						bool bValid = 1;

						if ((HWND)lParam == hScreenAngleEdit) {
							if (SendDlgItemMessage(hDlg, IDC_SCREENANGLE_EDIT, WM_GETTEXTLENGTH, (WPARAM)0, (LPARAM)0) < 16) {
								SendDlgItemMessage(hDlg, IDC_SCREENANGLE_EDIT, WM_GETTEXT, (WPARAM)16, (LPARAM)szText);
							}
						} else {
							if (SendDlgItemMessage(hDlg, IDC_SCREENCURVATURE_EDIT, WM_GETTEXTLENGTH, (WPARAM)0, (LPARAM)0) < 16) {
								SendDlgItemMessage(hDlg, IDC_SCREENCURVATURE_EDIT, WM_GETTEXT, (WPARAM)16, (LPARAM)szText);
							}
						}

						// Scan string in the edit control for illegal characters
						for (int i = 0; szText[i]; i++) {
							if (szText[i] == '.') {
								if (bPoint) {
									bValid = 0;
									break;
								}
							} else {
								if (!_istdigit(szText[i])) {
									bValid = 0;
									break;
								}
							}
						}

						if (bValid) {
							if ((HWND)lParam == hScreenAngleEdit) {
								fVidScreenAngle = _tcstod(szText, NULL) * DEGTORAD;
								if (fVidScreenAngle < 0.0f) {
									fVidScreenAngle = 0.0f;
								} else {
									if (fVidScreenAngle > PI / 1.5f) {
										fVidScreenAngle = PI / 1.5f;
									}
								}

								// Set slider to current value
								SendDlgItemMessage(hDlg, IDC_SCREENANGLE_SLIDER, TBM_SETPOS, (WPARAM)true, (LPARAM)(fVidScreenAngle * RADTODEG * 100.0f));
							} else {
								fVidScreenCurvature = _tcstod(szText, NULL) * DEGTORAD;
								if (fVidScreenCurvature < 0.0f) {
									fVidScreenCurvature = 0.0f;
								} else {
									if (fVidScreenCurvature > PI / 2.25f) {
										fVidScreenCurvature = PI / 2.25f;
									}
								}

								// Set slider to current value
								SendDlgItemMessage(hDlg, IDC_SCREENCURVATURE_SLIDER, TBM_SETPOS, (WPARAM)true, (LPARAM)(fVidScreenCurvature * RADTODEG * 150.0f));
							}

							// Update the screen
							if (bVidOkay) {
								VidPaint(2);
							}
						}
					}
					break;
				}
			}
			break;
		}

		case WM_HSCROLL: {
			switch (LOWORD(wParam)) {
				case TB_BOTTOM:
				case TB_ENDTRACK:
				case TB_LINEDOWN:
				case TB_LINEUP:
				case TB_PAGEDOWN:
				case TB_PAGEUP:
				case TB_THUMBPOSITION:
				case TB_THUMBTRACK:
				case TB_TOP: {
					if (nExitStatus == 0) {
						TCHAR szText[16];

						// Update the contents of the edit control
						if ((HWND)lParam == hScreenAngleSlider) {
							fVidScreenAngle = (float)SendDlgItemMessage(hDlg, IDC_SCREENANGLE_SLIDER, TBM_GETPOS, (WPARAM)0, (LPARAM)0) * DEGTORAD / 100.0f;
							_stprintf(szText, _T("%0.2f"), fVidScreenAngle * RADTODEG);
							SendDlgItemMessage(hDlg, IDC_SCREENANGLE_EDIT, WM_SETTEXT, (WPARAM)0, (LPARAM)szText);
						} else {
							fVidScreenCurvature = (float)SendDlgItemMessage(hDlg, IDC_SCREENCURVATURE_SLIDER, TBM_GETPOS, (WPARAM)0, (LPARAM)0) * DEGTORAD / 150.0f;
							_stprintf(szText, _T("%0.2f"), fVidScreenCurvature * RADTODEG);
							SendDlgItemMessage(hDlg, IDC_SCREENCURVATURE_EDIT, WM_SETTEXT, (WPARAM)0, (LPARAM)szText);
						}

						// Update the screen
						if (bVidOkay) {
							VidPaint(0);
						}
					}
					break;
				}
			}
			break;
		}

		case WM_CLOSE:
			if (nExitStatus != 1) {
				fVidScreenAngle = fPrevScreenAngle;
				fVidScreenCurvature = fPrevScreenCurvature;
			}
			EndDialog(hDlg, 0);
			break;
	}

	return 0;
}

void ScreenAngleDialog()
{
	FBADialogBox(hAppInst, MAKEINTRESOURCE(IDD_SCREENANGLE), hScrnWnd, (DLGPROC)ScreenAngleProc);
}

// -----------------------------------------------------------------------------
// CPU clock dialog

static INT_PTR CALLBACK CPUClockProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM)	// LPARAM lParam
{
	switch (Msg) {
		case WM_INITDIALOG: {
			TCHAR szText[16];
			nExitStatus = 0;

			WndInMid(hDlg, hScrnWnd);

			// Initialise slider
			SendDlgItemMessage(hDlg, IDC_CPUCLOCK_SLIDER, TBM_SETRANGE, (WPARAM)0, (LPARAM)MAKELONG(0x80, 0x0200));
			SendDlgItemMessage(hDlg, IDC_CPUCLOCK_SLIDER, TBM_SETLINESIZE, (WPARAM)0, (LPARAM)0x05);
			SendDlgItemMessage(hDlg, IDC_CPUCLOCK_SLIDER, TBM_SETPAGESIZE, (WPARAM)0, (LPARAM)0x10);
			SendDlgItemMessage(hDlg, IDC_CPUCLOCK_SLIDER, TBM_SETTIC, (WPARAM)0, (LPARAM)0x0100);
			SendDlgItemMessage(hDlg, IDC_CPUCLOCK_SLIDER, TBM_SETTIC, (WPARAM)0, (LPARAM)0x0120);
			SendDlgItemMessage(hDlg, IDC_CPUCLOCK_SLIDER, TBM_SETTIC, (WPARAM)0, (LPARAM)0x0140);
			SendDlgItemMessage(hDlg, IDC_CPUCLOCK_SLIDER, TBM_SETTIC, (WPARAM)0, (LPARAM)0x0180);

			// Set slider to current value
			SendDlgItemMessage(hDlg, IDC_CPUCLOCK_SLIDER, TBM_SETPOS, (WPARAM)true, (LPARAM)nBurnCPUSpeedAdjust);

			// Set the edit control to current value
			_stprintf(szText, _T("%i"), nBurnCPUSpeedAdjust * 100 / 256);
			SendDlgItemMessage(hDlg, IDC_CPUCLOCK_EDIT, WM_SETTEXT, (WPARAM)0, (LPARAM)szText);

			return TRUE;
		}
		case WM_COMMAND: {
			switch (HIWORD(wParam)) {
				case BN_CLICKED: {
					if (LOWORD(wParam) == IDOK) {
						nExitStatus = 1;
						SendMessage(hDlg, WM_CLOSE, 0, 0);
					}
					if (LOWORD(wParam) == IDCANCEL) {
						nExitStatus = -1;
						SendMessage(hDlg, WM_CLOSE, 0, 0);
					}
					break;
				}
				case EN_UPDATE: {
					if (nExitStatus == 0) {
						TCHAR szText[16] = _T("");
						bool bValid = 1;
						int nValue;

						if (SendDlgItemMessage(hDlg, IDC_CPUCLOCK_EDIT, WM_GETTEXTLENGTH, (WPARAM)0, (LPARAM)0) < 16) {
							SendDlgItemMessage(hDlg, IDC_CPUCLOCK_EDIT, WM_GETTEXT, (WPARAM)16, (LPARAM)szText);
						}

						// Scan string in the edit control for illegal characters
						for (int i = 0; szText[i]; i++) {
							if (!_istdigit(szText[i])) {
								bValid = 0;
								break;
							}
						}

						if (bValid) {
							nValue = _tcstol(szText, NULL, 0);
							if (nValue < 25) {
								nValue = 25;
							} else {
								if (nValue > 400) {
									nValue = 400;
								}
							}

							nValue = (int)((double)nValue * 256.0 / 100.0 + 0.5);

							// Set slider to current value
							SendDlgItemMessage(hDlg, IDC_CPUCLOCK_SLIDER, TBM_SETPOS, (WPARAM)true, (LPARAM)nValue);
						}
					}
					break;
				}
			}
			break;
		}

		case WM_HSCROLL: {
			switch (LOWORD(wParam)) {
				case TB_BOTTOM:
				case TB_ENDTRACK:
				case TB_LINEDOWN:
				case TB_LINEUP:
				case TB_PAGEDOWN:
				case TB_PAGEUP:
				case TB_THUMBPOSITION:
				case TB_THUMBTRACK:
				case TB_TOP: {
					if (nExitStatus == 0) {
						TCHAR szText[16];
						int nValue;

						// Update the contents of the edit control
						nValue = SendDlgItemMessage(hDlg, IDC_CPUCLOCK_SLIDER, TBM_GETPOS, (WPARAM)0, (LPARAM)0);
						nValue = (int)((double)nValue * 100.0 / 256.0 + 0.5);
						_stprintf(szText, _T("%i"), nValue);
						SendDlgItemMessage(hDlg, IDC_CPUCLOCK_EDIT, WM_SETTEXT, (WPARAM)0, (LPARAM)szText);
					}
					break;
				}
			}
			break;
		}

		case WM_CLOSE:
			if (nExitStatus == 1) {
				TCHAR szText[16] = _T("");

				SendDlgItemMessage(hDlg, IDC_CPUCLOCK_EDIT, WM_GETTEXT, (WPARAM)16, (LPARAM)szText);
				nBurnCPUSpeedAdjust = _tcstol(szText, NULL, 0);
				if (nBurnCPUSpeedAdjust < 25) {
					nBurnCPUSpeedAdjust = 25;
				} else {
					if (nBurnCPUSpeedAdjust > 400) {
						nBurnCPUSpeedAdjust = 400;
					}
				}

				nBurnCPUSpeedAdjust = (int)((double)nBurnCPUSpeedAdjust * 256.0 / 100.0 + 0.5);
			}
			EndDialog(hDlg, 0);
			break;
	}

	return 0;
}

void CPUClockDialog()
{
	FBADialogBox(hAppInst, MAKEINTRESOURCE(IDD_CPUCLOCK), hScrnWnd, (DLGPROC)CPUClockProc);
}

// -----------------------------------------------------------------------------
// Cubic filter quality dialog

static INT_PTR CALLBACK CubicProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM)	// LPARAM lParam
{
	static double dPrevB, dPrevC;

	switch (Msg) {
		case WM_INITDIALOG: {
			TCHAR szText[16];
			dPrevB = dVidCubicB;
			dPrevC = dVidCubicC;
			nExitStatus = 0;

			WndInMid(hDlg, hScrnWnd);

			// Initialise slider
			SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_SETRANGE, (WPARAM)0, (LPARAM)MAKELONG(0, 10000));
			SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_SETLINESIZE, (WPARAM)0, (LPARAM)100);
			SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_SETPAGESIZE, (WPARAM)0, (LPARAM)500);
			SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_SETTIC, (WPARAM)0, (LPARAM)3333);
			SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_SETTIC, (WPARAM)0, (LPARAM)5000);
			SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_SETTIC, (WPARAM)0, (LPARAM)7500);

			// Set slider to current value
			SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_SETPOS, (WPARAM)true, (LPARAM)((1.0 - dVidCubicB) * 10000));

			// Set the edit control to current value
			_stprintf(szText, _T("%.3lf"), 1.0 - dVidCubicB);
			SendDlgItemMessage(hDlg, IDC_SCANLINE_EDIT, WM_SETTEXT, (WPARAM)0, (LPARAM)szText);

			SetWindowText(hDlg, FBALoadStringEx(hAppInst, IDS_NUMDIAL_FILTER_SHARP, true));

			// Update the screen
			if (bDrvOkay) {
				VidPaint(2);
			}

			return TRUE;
		}
		case WM_COMMAND: {
			switch (HIWORD(wParam)) {
				case BN_CLICKED: {
					if (LOWORD(wParam) == IDOK) {
						nExitStatus = 1;
						SendMessage(hDlg, WM_CLOSE, 0, 0);
					}
					if (LOWORD(wParam) == IDCANCEL) {
						nExitStatus = -1;
						SendMessage(hDlg, WM_CLOSE, 0, 0);
					}
					break;
				}
				case EN_UPDATE: {
					if (nExitStatus == 0) {
						TCHAR szText[16] = _T("");
						bool bPoint = 0;
						bool bValid = 1;

						if (SendDlgItemMessage(hDlg, IDC_SCANLINE_EDIT, WM_GETTEXTLENGTH, (WPARAM)0, (LPARAM)0) < 16) {
							SendDlgItemMessage(hDlg, IDC_SCANLINE_EDIT, WM_GETTEXT, (WPARAM)16, (LPARAM)szText);
						}

						// Scan string in the edit control for illegal characters
						for (int i = 0; szText[i]; i++) {
							if (szText[i] == _T('.')) {
								if (bPoint) {
									bValid = 0;
									break;
								}
							} else {
								if (!_istdigit(szText[i])) {
									bValid = 0;
									break;
								}
							}
						}

						if (bValid) {
							dVidCubicB = 1.0 - _tcstod(szText, NULL);
							if (dVidCubicB < 0.0) {
								dVidCubicB = 0.0;
							} else {
								if (dVidCubicB > 1.0) {
									dVidCubicB = 1.0;
								}
							}

							// Set slider to current value
							SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_SETPOS, (WPARAM)true, (LPARAM)((1.0 - dVidCubicB) * 10000));

							// Update the screen
							if (bVidOkay) {
								VidPaint(2);
							}
						}
					}
					break;
				}
			}
			break;
		}

		case WM_HSCROLL: {
			switch (LOWORD(wParam)) {
				case TB_BOTTOM:
				case TB_ENDTRACK:
				case TB_LINEDOWN:
				case TB_LINEUP:
				case TB_PAGEDOWN:
				case TB_PAGEUP:
				case TB_THUMBPOSITION:
				case TB_THUMBTRACK:
				case TB_TOP: {
					if (nExitStatus == 0) {
						TCHAR szText[16];

						// Update the contents of the edit control
						dVidCubicB = 1.0 - (double)SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_GETPOS, (WPARAM)0, (LPARAM)0) / 10000;
						_stprintf(szText, _T("%.3lf"), 1.0 - dVidCubicB);
						SendDlgItemMessage(hDlg, IDC_SCANLINE_EDIT, WM_SETTEXT, (WPARAM)0, (LPARAM)szText);

						// Update the screen
						if (bVidOkay) {
//							VidRedraw();
							VidPaint(2);
						}
					}
					break;
				}
			}
			break;
		}

		case WM_CLOSE:
			if (nExitStatus != 1) {
				dVidCubicB = dPrevB;
				dVidCubicC = dPrevC;
			}
			EndDialog(hDlg, 0);
			break;
	}

	return 0;
}

void CubicSharpnessDialog()
{
	FBADialogBox(hAppInst, MAKEINTRESOURCE(IDD_SCANLINE), hScrnWnd, (DLGPROC)CubicProc);
}


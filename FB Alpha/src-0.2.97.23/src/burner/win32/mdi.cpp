/*
	================================================================================================
	FB Alpha MDI (Multiple Document Interface) (by CaptainCPS-X / Jezer Andino)
	================================================================================================
	------------------------------------------------------------------------------------------------
	Notes:
	------------------------------------------------------------------------------------------------

		At this point MDI basic features are working fine, since there is no need to minimize
		the main Video Window (child window), I left it to be maximized always. When switching 
		to FullScreen mode DirectX will then use hScrnWnd instead of hVideoWindow (for now, 
		until a fullscreen child window is properly implemented).

	------------------------------------------------------------------------------------------------
	Technical Notes:
	------------------------------------------------------------------------------------------------

		Based on Microsoft Developer Network documentation, most of the MDI functions and features
		will require a minimum operative system of Windows 2000. Maybe it will work on previous
		versions of Windows but that's what is documented on MSDN.

	------------------------------------------------------------------------------------------------
	TODO:
	------------------------------------------------------------------------------------------------

		* ...

	================================================================================================
*/

#include "burner.h"

const TCHAR	szMDIFrameClass[]	= _T("MDICLIENT");		// <- (don't change this) 
const TCHAR szVidWndClass[]		= _T("VidWndClass");
const TCHAR szVidWndTitle[]		= _T("\0");

HWND hWndChildFrame				= NULL;
HWND hVideoWindow				= NULL;

// Prototypes
LRESULT CALLBACK	VideoWndProc		(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
BOOL				RegNewMDIChild		();
HWND				CreateNewMDIChild	(HWND hMDIClient);
void				DestroyBurnerMDI	(int nAction);
bool				GetThemeStatus		();

// Window message handling stuff
static bool bDrag = false;
static int nOldWindowX, nOldWindowY;
static bool bRDblClick = false;
static float nDownX, nDownY, nMoveX, nMoveY;
HBITMAP hbitmap = NULL;
void StretchBmForClient(HDC, HWND, HBITMAP);

//static int OnCreate			(HWND, LPCREATESTRUCT);
//static void OnSize			(HWND, UINT state, int, int);
static void OnMDIActivate	(HWND, BOOL, HWND, HWND);
static void OnPaint         (HWND);							
static int OnMouseMove		(HWND, int, int, UINT);
static int OnLButtonDown	(HWND, BOOL, int, int, UINT);
static int OnLButtonUp		(HWND, int, int, UINT);
static int OnLButtonDblClk	(HWND, BOOL, int, int, UINT);
static int OnRButtonUp		(HWND, int, int, UINT);
static int OnRButtonDown	(HWND, BOOL, int, int, UINT);
static int OnSysCommand		(HWND, UINT, int, int);

int InitBurnerMDI (HWND hParentWnd)
{
	CLIENTCREATESTRUCT ccs;
	ccs.hWindowMenu		= NULL;
	ccs.idFirstChild	= ID_MDI_START_CHILD;
	UINT nWindowStyle	= WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE;
	UINT nWindowStyleEx	= 0;

	hWndChildFrame		= CreateWindowEx( nWindowStyleEx, szMDIFrameClass, NULL, nWindowStyle, 0, 0, 0, 0, hParentWnd, NULL, hAppInst, (LPVOID)&ccs );
	
	//if(!hWndChildFrame) MessageBox(hWnd, _T("Could not create MDI client."), _T("Error"), MB_OK | MB_ICONERROR);

	hVideoWindow		= nVidFullscreen ? NULL : CreateNewMDIChild(hWndChildFrame);

	return 0;
}

LRESULT CALLBACK VideoWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg) 
	{
		//HANDLE_MSG(hWnd, WM_CREATE,				OnCreate);
		//HANDLE_MSG(hWnd, WM_SIZE,			OnSize);
		HANDLE_MSG(hWnd, WM_MDIACTIVATE,	OnMDIActivate);
		HANDLE_MSGB(hWnd, WM_PAINT,			OnPaint);
		HANDLE_MSG(hWnd, WM_MOUSEMOVE,		OnMouseMove);
		HANDLE_MSG(hWnd, WM_LBUTTONUP,		OnLButtonUp);
		HANDLE_MSG(hWnd, WM_LBUTTONDOWN,	OnLButtonDown);
		HANDLE_MSG(hWnd, WM_LBUTTONDBLCLK,	OnLButtonDblClk);
		HANDLE_MSG(hWnd, WM_RBUTTONUP,		OnRButtonUp);
		HANDLE_MSG(hWnd, WM_RBUTTONDBLCLK,	OnRButtonDown);
		HANDLE_MSG(hWnd, WM_RBUTTONDOWN,	OnRButtonDown);

		// We can't use the macro from windowsx.h macro for this one
		case WM_SYSCOMMAND: 
		{
			if (OnSysCommand(hWnd,(UINT)wParam,(int)(short)LOWORD(lParam),(int)(short)HIWORD(lParam))) {
				return 0;
			}
			break;
		}

		default:
			return DefMDIChildProc(hWnd, Msg, wParam, lParam);
	}
	return 0;
}

BOOL RegNewMDIChild()
{
	if(nVidFullscreen) {
		return FALSE;
	}

	WNDCLASSEX wcex;
	wcex.cbSize				= sizeof(WNDCLASSEX);
	wcex.style				= CS_DBLCLKS;										// handle double click
	wcex.lpfnWndProc		= VideoWndProc;										// video window process callback
	wcex.cbClsExtra			= 0;
	wcex.cbWndExtra			= 0;
	wcex.hInstance			= hAppInst;											// fba instance handle
	wcex.hIcon				= LoadIcon(hAppInst, MAKEINTRESOURCE(IDI_APP));		// fba icon
	wcex.hCursor			= LoadCursor(NULL, IDC_ARROW);						// regular cursor
	wcex.hbrBackground		= CreateSolidBrush(0);								// black video window background
	wcex.lpszMenuName		= NULL;												// no menu
	wcex.lpszClassName		= szVidWndClass;									// video window class (MDI child)
	wcex.hIconSm			= LoadIcon(hAppInst, MAKEINTRESOURCE(IDI_APP));

	if(!RegisterClassEx(&wcex))	
	{
		//MessageBox(NULL, _T("There was a problem registering the window class"), _T("Error"), MB_OK);
		return FALSE;
	} else {
		return TRUE;
	}
}

HWND CreateNewMDIChild(HWND hMDIClient)
{
	HWND hNewWnd = NULL;
	hNewWnd = CreateMDIWindow(szVidWndClass, szVidWndTitle, MDIS_ALLCHILDSTYLES | CS_DBLCLKS | WS_DLGFRAME | WS_MAXIMIZE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hMDIClient, hAppInst, 0);
	//if(!hNewWnd) MessageBox(NULL, _T("Error creating child window"), _T("Creation Error"), MB_OK);
	return hNewWnd;
}

void DestroyBurnerMDI(int nAction) 
{
	switch(nAction) 
	{
		// OnDestroy
		case 0:
		{
			hVideoWindow	= NULL;
			hWndChildFrame	= NULL;
			break;
		}

		// ScrnExit()
		case 1:
		{
			if(hVideoWindow) 
			{
				SendMessage(hWndChildFrame, WM_MDIDESTROY, (INT_PTR) hVideoWindow, 0L);
				hVideoWindow = NULL;
			}
			if(hWndChildFrame) 
			{
				DestroyWindow(hWndChildFrame);
				hWndChildFrame = NULL;
			}
			UnregisterClass(szVidWndClass, hAppInst);
			break;
		}
	}
	DeleteObject(hbitmap);
}

// Check if Windows has a Theme active (Windows XP+)
bool GetThemeStatus() 
{
	bool bThemeActive = false;

	typedef BOOL WINAPI ISTHEMEACTIVE();
	ISTHEMEACTIVE* pISTHEMEACTIVE = NULL;

	HMODULE hMod = LoadLibrary(_T("uxtheme.dll"));

	if(hMod) {
		pISTHEMEACTIVE = reinterpret_cast<ISTHEMEACTIVE*>(GetProcAddress(hMod,"IsThemeActive"));
		if(pISTHEMEACTIVE) {
			bThemeActive = pISTHEMEACTIVE();
		}
		FreeLibrary(hMod);
	}

	return bThemeActive;
}

//static int OnCreate(HWND, LPCREATESTRUCT) 
//{
//	// ...
//	return 1;
//}

//static void OnSize(HWND hwnd, UINT /*state*/, int /*cx*/, int /*cy*/) 
//{
//	// ...
//}

static void OnMDIActivate(HWND /*hwnd*/, BOOL /*fActive*/, HWND /*hwndActivate*/, HWND /*hwndDeactivate*/) 
{
	// ...
	return;
}

// Needed for the non-preview mode
void StretchBmForClient(HDC hDC, HWND hWnd, HBITMAP hBitmap)
{
	HDC memDC; BITMAP bm; RECT re;

	memDC = CreateCompatibleDC(hDC);
	HGDIOBJ hOldObj = SelectObject(memDC, hBitmap);
	
	GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bm);
	GetClientRect(hWnd, &re);
	
	StretchBlt(hDC, 0, 0, re.right, re.bottom, memDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
	
	SelectObject(memDC, hOldObj);
	DeleteDC(memDC);
}

static void OnPaint(HWND hwnd)
{
	if(!nVidFullscreen) 
	{
		// This was formerly an alternative for the fail in using the "Preview blitter"
		// option in Win9x plattaforms, now, this is used just to complement the option
		// if you don't like the preview enabled.
		if (!bDrvOkay) {
			PAINTSTRUCT ps;

			//hbitmap = (HBITMAP)LoadImage(hAppInst, _T("BMP_SPLASH"), IMAGE_BITMAP, 304, 224, 0);
			hbitmap = (HBITMAP)LoadImage(hAppInst, MAKEINTRESOURCE(BMP_SPLASH), IMAGE_BITMAP, 304, 224, 0);

			HDC hDC = BeginPaint(hwnd, &ps);
			StretchBmForClient(hDC, hwnd, hbitmap);

			return;
		}

		PAINTSTRUCT ps;
		HDC hdc = NULL;
		hdc = BeginPaint (hwnd, &ps);
		EndPaint (hwnd, &ps);
		
		VidPaint(1);
	}
}

// MOUSE MOVE (DRAG)
static int OnMouseMove(HWND /*hwnd*/, int /*x*/, int /*y*/, UINT keyIndicators)
{
	if(bDrag && keyIndicators == MK_LBUTTON &&  !nVidFullscreen && !bMenuEnabled) 
	{
		AudSoundStop();

		// If UxThemes are active (Windows XP+) or DWM Composition is active
		if(GetThemeStatus() || IsCompositeOn()) {
			// Redraw everything on the MDI frame to get accurate calculatations
			RedrawWindow(hWndChildFrame, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
		}

		POINT pointer;
		memset(&pointer, 0, sizeof(POINT));
        
		GetCursorPos(&pointer);
		
		nMoveX = pointer.x;
		nMoveY = pointer.y;

		// negative (left / up) [-]
		if((nMoveX < nDownX && nMoveX != 0) || (nMoveY < nDownY && nMoveY != 0)) {
			nWindowPosX = nOldWindowX - (int)(nDownX - nMoveX);
			nWindowPosY = nOldWindowY - (int)(nDownY - nMoveY);	
			SetWindowPos(hScrnWnd, NULL, nWindowPosX, nWindowPosY, 0, 0, SWP_NOREPOSITION | SWP_NOSIZE);
		}

		// positive (right / down) [+]
		if((nMoveX > nDownX && nMoveX != 0) || (nMoveY > nDownY && nMoveY != 0)) {
			nWindowPosX = nOldWindowX + (int)(nMoveX - nDownX);
			nWindowPosY = nOldWindowY + (int)(nMoveY - nDownY);	
			SetWindowPos(hScrnWnd, NULL, nWindowPosX, nWindowPosY, 0, 0, SWP_NOREPOSITION | SWP_NOSIZE);
		}

		AudSoundPlay();
		return 0;
	}
	return 1;
}

// MOUSE LEFT BUTTON (DOWN)
static int OnLButtonDown(HWND /*hwnd*/, BOOL bDouble, int /*x*/, int /*y*/, UINT)
{
	if (!bDrag && !nVidFullscreen && !bMenuEnabled && !bDouble) 
	{
		POINT pointer;
		memset(&pointer, 0, sizeof(POINT));

        GetCursorPos(&pointer);

        nDownX = pointer.x;
        nDownY = pointer.y;

		RECT cr;
		GetWindowRect(hScrnWnd, &cr);

		nOldWindowX = cr.left;
		nOldWindowY = cr.top;

		bDrag = true;

		return 0;
	}
	return 1;
}

// MOUSE LEFT BUTTON (UP)
static int OnLButtonUp(HWND /*hwnd*/, int /*x*/, int /*y*/, UINT)
{
	RECT cr;
	GetWindowRect(hScrnWnd, &cr);

	if(bDrag && !nVidFullscreen && !bMenuEnabled) 
	{
		if((nDownX != nMoveX || nDownY != nMoveY) && (nMoveX != 0 && nMoveY != 0)) {				
			// window moved	- NO MENU
		} else {
			// window was not moved - SHOW MENU
			TrackPopupMenuEx(hMenuPopup, TPM_LEFTALIGN | TPM_TOPALIGN, (int)nDownX, (int)nDownY, hScrnWnd, NULL);
		}

		bDrag = false;
		nDownX = nDownY = nMoveX = nMoveY = 0;

		return 0;
	}
	return 1;
}

// MOUSE LEFT BUTTON (DOUBLE CLICK)
static int OnLButtonDblClk(HWND /*hwnd*/, BOOL, int, int, UINT)
{
	if (bDrvOkay) 
	{
		nVidFullscreen = !nVidFullscreen;
		POST_INITIALISE_MESSAGE;
		return 0;
	}
	return 1;
}

// MOUSE RIGHT BUTTON (DOWN)
static int OnRButtonDown(HWND hwnd, BOOL /*bDouble*/, int, int, UINT)
{
	return (hwnd == hVideoWindow && !nVidFullscreen) ? 0 : 1;
}

// MOUSE RIGHT BUTTON (UP)
static int OnRButtonUp(HWND /*hwnd*/, int, int, UINT)
{
	// If not fullscreen and this event is not related to 'toggle fullscreen' right double-click event
	if (!nVidFullscreen && !bRDblClick) 
	{
		bMenuEnabled = !bMenuEnabled;
		POST_INITIALISE_MESSAGE;
		return 0;
	}
	return 1;
}

// Block screensaver and windows menu if needed
static int OnSysCommand(HWND, UINT sysCommand, int, int)
{
	switch (sysCommand) {
		case SC_MONITORPOWER:
		case SC_SCREENSAVE: {
			if (!bRunPause && bDrvOkay) {
				return 1;
			}
			break;
		}
		case SC_KEYMENU:
		case SC_MOUSEMENU: {
			if (kNetGame && !bModelessMenu) {
				return 1;
			}
			break;
		}
	}
	return 0;
}

/***************************************************************************
 * Gens: (Win32) Country Code Window.                                      *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#include "country_code_window.h"
#include "country_code_window_callbacks.h"
#include "gens/gens_window.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "emulator/g_main.hpp"

#include <windowsx.h>

// Gens Win32 resources
#include "ui/win32/resource.h"

// Win32 common controls
#include <commctrl.h>

// WINE's headers don't include BUTTON_IMAGELIST or related macros and constants.
#ifndef BUTTON_IMAGELIST
typedef struct
{
    HIMAGELIST himl;
    RECT margin;
    UINT uAlign;
} BUTTON_IMAGELIST, *PBUTTON_IMAGELIST;
#endif /* BUTTON_IMAGELIST */
#ifndef BUTTON_IMAGELIST_ALIGN_LEFT
#define BUTTON_IMAGELIST_ALIGN_LEFT 0
#endif
#ifndef BCM_FIRST
#define BCM_FIRST 0x1600
#endif
#ifndef BCM_SETIMAGELIST
#define BCM_SETIMAGELIST (BCM_FIRST + 0x0002)
#endif
#ifndef Button_SetImageList
#define Button_SetImageList(hWnd, pbuttonImageList) \
	SendMessage((hWnd), BCM_SETIMAGELIST, 0, (LPARAM)(pbuttonImageList))
#endif

static WNDCLASS WndClass;
HWND country_code_window = NULL;

// Controls
HWND cc_lstCountryCodes = NULL;

// Image lists for arrow buttons on XP and later.
HIMAGELIST cc_imglArrowUp = NULL;
HIMAGELIST cc_imglArrowDown = NULL;

// Country codes
// TODO: Move this somewhere else?
const char* Country_Code_String[3] = {"USA", "Japan", "Europe"};

// Window size.
static const unsigned short wndWidth = 256;
static const unsigned short wndHeight = 144;


/**
 * create_country_code_window(): Create the Country Code Window.
 * @return Country Code Window.
 */
HWND create_country_code_window(void)
{
	if (country_code_window)
	{
		// Country Code window is already created. Set focus.
		// TODO: Figure out how to do this.
		ShowWindow(country_code_window, 1);
		return NULL;
	}
	
	// Create the window class.
	WndClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = Country_Code_Window_WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = ghInstance;
	WndClass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_SONIC_HEAD));
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = "Gens_Country_Code";
	
	RegisterClass(&WndClass);
	
	// Create the window.
	country_code_window = CreateWindowEx(NULL, "Gens_Country_Code", "Country Code Order",
					     WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
					     CW_USEDEFAULT, CW_USEDEFAULT,
					     wndWidth, wndHeight,
					     Gens_hWnd, NULL, ghInstance, NULL);
	
	// Set the actual window size.
	// NOTE: This is done in Country_Code_Window_CreateChildWindows to compensate for listbox variations.
	//Win32_setActualWindowSize(country_code_window, wndWidth, wndHeight);
	
	// Center the window on the Gens window.
	// NOTE: This is done in Country_Code_Window_CreateChildWindows to compensate for listbox variations.
	//Win32_centerOnGensWindow(country_code_window);
	
	UpdateWindow(country_code_window);
	return country_code_window;
}


void Country_Code_Window_CreateChildWindows(HWND hWnd)
{
	// Up/Down button size (square).
	const unsigned short btnSize = 24;
	
	// Group box size.
	const unsigned short grpBox_Width = wndWidth-16;
	const unsigned short grpBox_Height = wndHeight-16-24-8;
	const unsigned short lstBox_Width = grpBox_Width-16-btnSize-8;
	const unsigned short lstBox_Height = grpBox_Height-16-8;
	
	// Create the group box.
	HWND grpBox;
	grpBox = CreateWindow(WC_BUTTON, "Country Code Order",
			      WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
			      8, 8, grpBox_Width, grpBox_Height,
			      hWnd, NULL, ghInstance, NULL);
	SetWindowFont(grpBox, fntMain, TRUE);
	
	// Create the listbox.
	cc_lstCountryCodes = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTBOX, NULL,
					    WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					    8+8, 8+16, lstBox_Width, lstBox_Height,
					    hWnd, NULL, ghInstance, NULL);
	SetWindowFont(cc_lstCountryCodes, fntMain, TRUE);
	
	// Check what the listbox's actual height is.
	RECT r;
	GetWindowRect(cc_lstCountryCodes, &r);
	const unsigned short lstBox_Height_New = r.bottom - r.top;
	
	// Adjust the height of the groupbox.
	const unsigned short grpBox_Height_New = grpBox_Height - (lstBox_Height - lstBox_Height_New);
	SetWindowPos(grpBox, 0, 0, 0, grpBox_Width, grpBox_Height_New, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
	
	// Adjust the window height and position.
	Win32_setActualWindowSize(hWnd, wndWidth, grpBox_Height_New+16+24+8);
	Win32_centerOnGensWindow(hWnd);
	
	// Create the up/down buttons.
	// TODO: BS_ICON apparently doesn't work on NT4 and earlier.
// 	// See http://support.microsoft.com/kb/142226
	HWND btnUp, btnDown;
	HICON icoUp, icoDown;
	const unsigned short icoSize = 16;
	
	icoUp = (HICON)LoadImage(ghInstance, MAKEINTRESOURCE(IDI_ARROW_UP),
				 IMAGE_ICON, icoSize, icoSize, 0);
	icoDown = (HICON)LoadImage(ghInstance, MAKEINTRESOURCE(IDI_ARROW_DOWN),
				   IMAGE_ICON, icoSize, icoSize, 0);
	
	btnUp = CreateWindow(WC_BUTTON, NULL,
			     WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_ICON,
			     8+8+lstBox_Width+8, 8+16, btnSize, btnSize,
			     hWnd, (HMENU)IDC_COUNTRY_CODE_UP, ghInstance, NULL);
	SetWindowFont(btnUp, fntMain, TRUE);
	
	btnDown = CreateWindow(WC_BUTTON, NULL,
			       WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_ICON,
			       8+8+lstBox_Width+8, 8+16+lstBox_Height_New-btnSize, btnSize, btnSize,
			       hWnd, (HMENU)IDC_COUNTRY_CODE_DOWN, ghInstance, NULL);
	SetWindowFont(btnDown, fntMain, TRUE);
	
	if (winVersion.dwMajorVersion > 5 ||
	    (winVersion.dwMajorVersion == 5 && winVersion.dwMinorVersion >= 1))
	{
		// Windows XP or later. Use BUTTON_IMAGELIST and Button_SetImageList().
		// This ensures that visual styles are applied correctly.
		cc_imglArrowUp = ImageList_Create(icoSize, icoSize, ILC_MASK | ILC_COLOR4, 1, 1);
		cc_imglArrowDown = ImageList_Create(icoSize, icoSize, ILC_MASK | ILC_COLOR4, 1, 1);
		ImageList_SetBkColor(cc_imglArrowUp, CLR_NONE);
		ImageList_SetBkColor(cc_imglArrowDown, CLR_NONE);
		ImageList_AddIcon(cc_imglArrowUp, icoUp);
		ImageList_AddIcon(cc_imglArrowDown, icoDown);
		
		BUTTON_IMAGELIST bimglBtn;
		bimglBtn.uAlign = BUTTON_IMAGELIST_ALIGN_LEFT;
		bimglBtn.margin.top = 0;
		bimglBtn.margin.bottom = 0;
		bimglBtn.margin.left = 1;
		bimglBtn.margin.right = 0;
		
		// Up button
		bimglBtn.himl = cc_imglArrowUp;
		Button_SetImageList(btnUp, &bimglBtn);
		
		// Down button
		bimglBtn.himl = cc_imglArrowDown;
		Button_SetImageList(btnDown, &bimglBtn);
	}
	else
	{
		// Windows 2000 or earlier. Use BM_SETIMAGE.
		SetWindowText(btnUp, "Up");
		SetWindowText(btnDown, "Down");
		SendMessage(btnUp, BM_SETIMAGE, IMAGE_ICON, (LPARAM)icoUp);
		SendMessage(btnDown, BM_SETIMAGE, IMAGE_ICON, (LPARAM)icoDown);
	}
	
	// Buttons
	const unsigned short btnTop = 8+grpBox_Height_New+8;
	const unsigned short btnWidth = 75;
	const int btnLeft = ((wndWidth - (btnWidth+8+btnWidth+8+btnWidth)) / 2);
	HWND btnOK, btnApply, btnCancel;
	
	btnOK = CreateWindow(WC_BUTTON, "&OK", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
			     btnLeft, btnTop, btnWidth, 23,
			     hWnd, (HMENU)IDC_BTN_OK, ghInstance, NULL);
	SetWindowFont(btnOK, fntMain, TRUE);
	
	btnApply = CreateWindow(WC_BUTTON, "&Apply", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				btnLeft+btnWidth+8, btnTop, btnWidth, 23,
				hWnd, (HMENU)IDC_BTN_APPLY, ghInstance, NULL);
	SetWindowFont(btnApply, fntMain, TRUE);
	
	btnCancel = CreateWindow(WC_BUTTON, "&Cancel", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				 btnLeft+btnWidth+8+btnWidth+8, btnTop, btnWidth, 23,
				 hWnd, (HMENU)IDC_BTN_CANCEL, ghInstance, NULL);
	SetWindowFont(btnCancel, fntMain, TRUE);
	
	// Set focus to the listbox.
	SetFocus(cc_lstCountryCodes);
}

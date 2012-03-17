/***************************************************************************
 * Gens: (Win32) Color Adjustment Window.                                  *
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

#include "color_adjust_window.h"
#include "color_adjust_window_callbacks.h"
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

static WNDCLASS WndClass;
HWND color_adjust_window = NULL;

// Controls
static HWND ca_lblContrastDesc;
HWND ca_trkContrast;
HWND ca_lblContrastVal;
static HWND ca_lblBrightnessDesc;
HWND ca_trkBrightness;
HWND ca_lblBrightnessVal;


/**
 * create_color_adjust_window(): Create the Color Adjustment Window.
 * @return Color Adjustment Window.
 */
HWND create_color_adjust_window(void)
{
	if (color_adjust_window)
	{
		// Color Adjustment window is already created. Set focus.
		// TODO: Figure out how to do this.
		ShowWindow(color_adjust_window, 1);
		return NULL;
	}
	
	// Create the window class.
	WndClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = Color_Adjust_Window_WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = ghInstance;
	WndClass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_SONIC_HEAD));
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = "Gens_Color_Adjust";
	
	RegisterClass(&WndClass);
	
	// Create the window.
	color_adjust_window = CreateWindowEx(NULL, "Gens_Color_Adjust", "Color Adjustment",
					     WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
					     CW_USEDEFAULT, CW_USEDEFAULT,
					     296, 112,
					     Gens_hWnd, NULL, ghInstance, NULL);
	
	// Set the actual window size.
	Win32_setActualWindowSize(color_adjust_window, 296, 112);
	
	// Center the window on the Gens window.
	Win32_centerOnGensWindow(color_adjust_window);
	
	UpdateWindow(color_adjust_window);
	return color_adjust_window;
}


void Color_Adjust_Window_CreateChildWindows(HWND hWnd)
{
	unsigned int trkStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | TBS_HORZ | TBS_BOTTOM;
	
	// Contrast
	ca_lblContrastDesc = CreateWindow(WC_STATIC, "Contrast", WS_CHILD | WS_VISIBLE | SS_LEFT,
					  8, 16, 56, 16, hWnd, NULL, ghInstance, NULL);
	SetWindowFont(ca_lblContrastDesc, fntMain, TRUE);
	
	ca_trkContrast = CreateWindow(TRACKBAR_CLASS, "", trkStyle,
				      8+56, 14, 192, 24, hWnd,
				      (HMENU)IDC_TRK_CA_CONTRAST, ghInstance, NULL);
	SendMessage(ca_trkContrast, TBM_SETPAGESIZE, 0, 10);
	SendMessage(ca_trkContrast, TBM_SETTICFREQ, 25, 0);
	SendMessage(ca_trkContrast, TBM_SETRANGE, TRUE, MAKELONG(-100, 100));
	SendMessage(ca_trkContrast, TBM_SETPOS, TRUE, 0);
	SetFocus(ca_trkContrast);
	
	ca_lblContrastVal = CreateWindow(WC_STATIC, "0", WS_CHILD | WS_VISIBLE | SS_LEFT,
					 8+56+192+8, 16, 32, 16, hWnd, NULL, ghInstance, NULL);
	SetWindowFont(ca_lblContrastVal, fntMain, TRUE);
	
	// Brightness
	ca_lblBrightnessDesc = CreateWindow(WC_STATIC, "Brightness", WS_CHILD | WS_VISIBLE | SS_LEFT,
					    8, 48, 56, 16, hWnd, NULL, ghInstance, NULL);
	SetWindowFont(ca_lblBrightnessDesc, fntMain, TRUE);
	
	ca_trkBrightness = CreateWindow(TRACKBAR_CLASS, "", trkStyle,
					8+56, 14+32, 192, 24, hWnd,
					(HMENU)IDC_TRK_CA_BRIGHTNESS, ghInstance, NULL);
	SendMessage(ca_trkBrightness, TBM_SETPAGESIZE, 0, 10);
	SendMessage(ca_trkBrightness, TBM_SETTICFREQ, 25, 0);
	SendMessage(ca_trkBrightness, TBM_SETRANGE, TRUE, MAKELONG(-100, 100));
	SendMessage(ca_trkBrightness, TBM_SETPOS, TRUE, 0);
	
	ca_lblBrightnessVal = CreateWindow(WC_STATIC, "0", WS_CHILD | WS_VISIBLE | SS_LEFT,
					   8+56+192+8, 48, 32, 16, hWnd, NULL, ghInstance, NULL);
	SetWindowFont(ca_lblBrightnessVal, fntMain, TRUE);
	
	// Buttons
	HWND btnOK, btnApply, btnCancel;
	
	btnOK = CreateWindow(WC_BUTTON, "&OK", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
			     28, 80, 75, 23,
			     hWnd, (HMENU)IDC_BTN_OK, ghInstance, NULL);
	SetWindowFont(btnOK, fntMain, TRUE);
	
	btnApply = CreateWindow(WC_BUTTON, "&Apply", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				28+75+8, 80, 75, 23,
				hWnd, (HMENU)IDC_BTN_APPLY, ghInstance, NULL);
	SetWindowFont(btnApply, fntMain, TRUE);
	
	btnCancel = CreateWindow(WC_BUTTON, "&Cancel", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				 28+75+8+75+8, 80, 75, 23,
				 hWnd, (HMENU)IDC_BTN_CANCEL, ghInstance, NULL);
	SetWindowFont(btnCancel, fntMain, TRUE);
}

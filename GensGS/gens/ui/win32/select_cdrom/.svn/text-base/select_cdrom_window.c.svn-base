/***************************************************************************
 * Gens: (Win32) Select CD-ROM Drive Window.                               *
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

#include "emulator/g_main.hpp"

#include "select_cdrom_window.h"
#include "select_cdrom_window_callbacks.h"
#include "gens/gens_window.hpp"

// SegaCD
#include "emulator/g_mcd.hpp"
#include "segacd/cd_aspi.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include <windowsx.h>

// Gens Win32 resources
#include "ui/win32/resource.h"

// Win32 common controls
#include <commctrl.h>

static WNDCLASS WndClass;
HWND select_cdrom_window = NULL;

// Controls
HWND SelCD_cdromDropdownBox = NULL;
HWND SelCD_btnOK = NULL;
HWND SelCD_btnApply = NULL;

/**
 * create_select_cdrom_window(): Create the Select CD-ROM Drive Window.
 * @return Select CD-ROM Drive Window.
 */
HWND create_select_cdrom_window(void)
{
	if (select_cdrom_window)
	{
		// Select CD-ROM Drive window is already created. Set focus.
		// TODO: Figure out how to do this.
		ShowWindow(select_cdrom_window, 1);
		return NULL;
	}
	
	// Create the window class.
	WndClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = Select_CDROM_Window_WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = ghInstance;
	WndClass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_SONIC_HEAD));
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = "Gens_Select_CDROM";
	
	RegisterClass(&WndClass);
	
	// Create the window.
	select_cdrom_window = CreateWindowEx(NULL, "Gens_Select_CDROM", "Select CD-ROM Drive",
					     WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
					     CW_USEDEFAULT, CW_USEDEFAULT,
					     320, 72,
					     Gens_hWnd, NULL, ghInstance, NULL);
	
	// Set the actual window size.
	Win32_setActualWindowSize(select_cdrom_window, 320, 72);
	
	// Center the window on the Gens window.
	Win32_centerOnGensWindow(select_cdrom_window);
	
	UpdateWindow(select_cdrom_window);
	return select_cdrom_window;
}


void Select_CDROM_Window_CreateChildWindows(HWND hWnd)
{
	HWND cdromDriveTitle;
	
	// CD-ROM Drive title
	cdromDriveTitle = CreateWindow(WC_STATIC, "CD-ROM Drive:",
				       WS_CHILD | WS_VISIBLE | SS_LEFT,
				       8, 8+3, 96, 16,
				       hWnd, NULL, ghInstance, NULL);
	SetWindowFont(cdromDriveTitle, fntMain, TRUE);
	
	// CD-ROM Drive dropdown box
	SelCD_cdromDropdownBox = CreateWindow(WC_COMBOBOX, NULL,
					      WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
					      16+96-8, 8, 320-8-96-16+8, 23*5,
					      hWnd, NULL, ghInstance, NULL);
	SetWindowFont(SelCD_cdromDropdownBox, fntMain, TRUE);
	
	// Buttons
	const unsigned short btnTop = 40;
	const unsigned short btnLeft = (320-(8+75+8+75+8+75+8))/2;
	HWND btnCancel;
	
	SelCD_btnOK = CreateWindow(WC_BUTTON, "&OK", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
				   btnLeft+8, btnTop, 75, 23,
				   hWnd, (HMENU)IDC_BTN_OK, ghInstance, NULL);
	SetWindowFont(SelCD_btnOK, fntMain, TRUE);
	
	SelCD_btnApply = CreateWindow(WC_BUTTON, "&Apply", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				      btnLeft+8+75+8, btnTop, 75, 23,
				      hWnd, (HMENU)IDC_BTN_APPLY, ghInstance, NULL);
	SetWindowFont(SelCD_btnApply, fntMain, TRUE);
	
	btnCancel = CreateWindow(WC_BUTTON, "&Cancel", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				 btnLeft+8+75+8+75+8, btnTop, 75, 23,
				 hWnd, (HMENU)IDC_BTN_CANCEL, ghInstance, NULL);
	SetWindowFont(btnCancel, fntMain, TRUE);
	
	if (Num_CD_Drive == 0)
	{
		// No CD-ROM drives detected.
		ComboBox_AddString(SelCD_cdromDropdownBox, "No CD-ROM drives detected.");
		Button_Enable(SelCD_btnOK, FALSE);
		Button_Enable(SelCD_btnApply, FALSE);
		
		return;
	}
	
	// Populate the dropdown box.
	char driveName[100];
	int i;
	for (i = 0; i < Num_CD_Drive; i++)
	{
		ASPI_Get_Drive_Info(i, (unsigned char*)driveName);
		ComboBox_AddString(SelCD_cdromDropdownBox, &driveName[8]);
	}
}

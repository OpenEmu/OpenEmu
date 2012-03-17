/***************************************************************************
 * Gens: (Win32) BIOS/Misc Files Window.                                   *
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

#include "bios_misc_files_window.hpp"
#include "bios_misc_files_window_callbacks.h"
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
HWND bios_misc_files_window = NULL;

// Frame width.
static const int frameWidth = 376;

// All textboxes to be displayed on the BIOS/Misc Files window are defined here.
const unsigned short BIOSMiscFiles_Count = 13;
const struct BIOSMiscFileEntry_t BIOSMiscFiles[BIOSMiscFiles_Count + 1] =
{
	{"Genesis BIOS File", "md_bios", (FileFilterType)0, NULL},
	{"Genesis", "md_bios", ROMFile, BIOS_Filenames.MD_TMSS},
	{"32X BIOS Files", "32x_bios", (FileFilterType)0, NULL},
	{"MC68000", "mc68000", ROMFile, BIOS_Filenames._32X_MC68000},
	{"Master SH2", "msh2", ROMFile, BIOS_Filenames._32X_MSH2},
	{"Slave SH2", "ssh2", ROMFile, BIOS_Filenames._32X_SSH2},
	{"SegaCD BIOS Files", "mcd_bios", (FileFilterType)0, NULL},
	{"USA", "mcd_bios_usa", ROMFile, BIOS_Filenames.SegaCD_US},
	{"Europe", "mcd_bios_eur", ROMFile, BIOS_Filenames.MegaCD_EU},
	{"Japan", "mcd_bios_jap", ROMFile, BIOS_Filenames.MegaCD_JP},
	{"Compression Utilities", "compression", (FileFilterType)0, NULL},
	{"7z Binary", "_7z", AnyFile, Misc_Filenames._7z_Binary},
	{"RAR Binary", "rar", AnyFile, Misc_Filenames.RAR_Binary},
	{NULL, NULL, (FileFilterType)0, NULL},
};

// Handles for the Win32 textboxes.
HWND bmf_txtEntry[BIOSMiscFiles_Count];


/**
 * create_bios_misc_files_window(): Create the BIOS/Misc Files Window.
 * @return BIOS/Misc Files Window.
 */
HWND create_bios_misc_files_window(void)
{
	if (bios_misc_files_window)
	{
		// BIOS/Misc Files window is already created. Set focus.
		// TODO: Figure out how to do this.
		ShowWindow(bios_misc_files_window, 1);
		return NULL;
	}
	
	// Create the window class.
	WndClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = BIOS_Misc_Files_Window_WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = ghInstance;
	WndClass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_SONIC_HEAD));
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = "Gens_BIOS_Misc_Files";
	
	RegisterClass(&WndClass);
	
	// Create the window.
	bios_misc_files_window = CreateWindowEx(NULL, "Gens_BIOS_Misc_Files", "Configure BIOS/Misc Files",
						WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
						CW_USEDEFAULT, CW_USEDEFAULT,
						frameWidth + 16, 384,
						Gens_hWnd, NULL, ghInstance, NULL);
	
	// Set the actual window size.
	Win32_setActualWindowSize(bios_misc_files_window, frameWidth + 16, 384);
	
	// Center the window on the Gens window.
	Win32_centerOnGensWindow(bios_misc_files_window);
	
	UpdateWindow(bios_misc_files_window);
	return bios_misc_files_window;
}


void BIOS_Misc_Files_Window_CreateChildWindows(HWND hWnd)
{
	// Create all frames. This will be fun!
	HWND grpBox = NULL;
	HWND lblTitle, txtEntry, btnChange;
	
	// Positioning.
	const int grpBox_Left = 8;
	int grpBox_Top = 0, grpBox_Height = 0, grpBox_Entry = 0;
	int entryTop;
	int file = 0;
	
	while (BIOSMiscFiles[file].title)
	{
		if (!BIOSMiscFiles[file].entry)
		{
			// No entry buffer. This is a new frame.
			grpBox_Top += grpBox_Height + 8;
			grpBox_Height = 24;
			grpBox_Entry = 0;
			bmf_txtEntry[file] = NULL;
			
			grpBox = CreateWindow(WC_BUTTON, BIOSMiscFiles[file].title,
					      WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
					      grpBox_Left, grpBox_Top, frameWidth, grpBox_Height,
					      hWnd, NULL, ghInstance, NULL);
			
			// Set the font for the groupbox title.
			SetWindowFont(grpBox, fntMain, TRUE);
		}
		else
		{
			// File entry.
			grpBox_Height += 24;
			entryTop = 20 + (grpBox_Entry * 24);
			SetWindowPos(grpBox, NULL, 0, 0, frameWidth, grpBox_Height, SWP_NOMOVE | SWP_NOZORDER);
			
			// Create the label for the title.
			lblTitle = CreateWindow(WC_STATIC, BIOSMiscFiles[file].title,
						WS_CHILD | WS_VISIBLE | SS_LEFT,
						grpBox_Left + 8, grpBox_Top + entryTop, 64, 16,
						hWnd, NULL, ghInstance, NULL);
						
			// Set the font for the label.
			SetWindowFont(lblTitle, fntMain, TRUE);
			
			// Create the textbox for the entry.
			txtEntry = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, BIOSMiscFiles[file].entry,
						  WS_CHILD | WS_VISIBLE | WS_TABSTOP | SS_LEFT | ES_AUTOHSCROLL,
						  grpBox_Left + 8+56+8, grpBox_Top + entryTop,
						  frameWidth - (8+64+8+72+8), 20,
						  hWnd, NULL, ghInstance, NULL);
			
			// Set the font for the entry.
			SetWindowFont(txtEntry, fntMain, TRUE);
			
			// Create the change button for the entry.
			btnChange = CreateWindow(WC_BUTTON, "Change...",
						 WS_CHILD | WS_VISIBLE | WS_TABSTOP,
						 grpBox_Left + frameWidth - (8+72), grpBox_Top + entryTop, 72, 20,
						 hWnd, (HMENU)(IDC_BTN_CHANGE + file), ghInstance, NULL);
						 
			// Set the font for the button.
			SetWindowFont(btnChange, fntMain, TRUE);
			
			// Save the text entry hWnd for later.
			bmf_txtEntry[file] = txtEntry;
			
			// Next entry.
			grpBox_Entry++;
		}
		
		// Next file.
		file++;
	}
	
	// Buttons
	int btnTop = grpBox_Top + grpBox_Height + 8;
	const int btnLeft = ((frameWidth - (75+8+75+8+75)) / 2) + 8;
	HWND btnOK, btnApply, btnCancel;
	
	btnOK = CreateWindow(WC_BUTTON, "&OK", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
			     btnLeft, btnTop, 75, 23,
			     hWnd, (HMENU)IDC_BTN_OK, ghInstance, NULL);
	SetWindowFont(btnOK, fntMain, TRUE);
	
	btnApply = CreateWindow(WC_BUTTON, "&Apply", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				btnLeft+75+8, btnTop, 75, 23,
				hWnd, (HMENU)IDC_BTN_APPLY, ghInstance, NULL);
	SetWindowFont(btnApply, fntMain, TRUE);
	
	btnCancel = CreateWindow(WC_BUTTON, "&Cancel", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				 btnLeft+75+8+75+8, btnTop, 75, 23,
				 hWnd, (HMENU)IDC_BTN_CANCEL, ghInstance, NULL);
	SetWindowFont(btnCancel, fntMain, TRUE);
	
	// Set focus to the first textbox.
	// (Index 0 is the frame; Index 1 is the textbox.)
	SetFocus(bmf_txtEntry[1]);
}

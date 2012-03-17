/***************************************************************************
 * Gens: (Win32) Directory Configuration Window.                           *
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

#include "directory_config_window.h"
#include "directory_config_window_callbacks.h"
#include "gens/gens_window.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <windowsx.h>

// Gens Win32 resources
#include "ui/win32/resource.h"

// Win32 common controls
#include <commctrl.h>

#include "emulator/g_main.hpp"

// Includes with directory defines.
// TODO: Consolidate these into Settings.PathNames
#include "util/file/save.hpp"
#include "util/gfx/imageutil.hpp"
#include "util/file/ggenie.h"
#include "util/file/rom.hpp"


static WNDCLASS WndClass;
HWND directory_config_window = NULL;


// Frame width.
static const int frameWidth = 384;


// All textboxes to be displayed on the BIOS/Misc Files window are defined here.
const unsigned short DirEntries_Count = 8;
const struct DirEntry_t DirEntries[] =
{
	{"Savestates", "savestates", State_Dir},
	{"SRAM Backup", "sram", SRAM_Dir},
	{"BRAM Backup", "bram", BRAM_Dir},
	{"WAV Dump", "wavdump", PathNames.Dump_WAV_Dir},
	{"GYM Dump", "gymdump", PathNames.Dump_GYM_Dir},
	{"Screenshots", "screenshots", PathNames.Screenshot_Dir},
	{"PAT patches", "pat_patches", Patch_Dir},
	{"IPS patches", "ips_patches", IPS_Dir},
	{NULL, NULL, NULL},
};
HWND dc_txtEntry[8];


/**
 * create_directory_config_window(): Create the Directory Configuration Window.
 * @return Directory Configuration Window.
 */
HWND create_directory_config_window(void)
{
	if (directory_config_window)
	{
		// Directory Configuration window is already created. Set focus.
		// TODO: Figure out how to do this.
		ShowWindow(directory_config_window, 1);
		return NULL;
	}
	
	// Create the window class.
	WndClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = Directory_Config_Window_WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = ghInstance;
	WndClass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_SONIC_HEAD));
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = "Gens_Directory_Config";
	
	RegisterClass(&WndClass);
	
	// Create the window.
	directory_config_window = CreateWindowEx(NULL, "Gens_Directory_Config", "Configure Directories",
						 WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
						 CW_USEDEFAULT, CW_USEDEFAULT,
						 frameWidth + 16, 264,
						 Gens_hWnd, NULL, ghInstance, NULL);	
	
	// Set the actual window size.
	Win32_setActualWindowSize(directory_config_window, frameWidth + 16, 264);
	
	// Center the window on the Gens window.
	Win32_centerOnGensWindow(directory_config_window);
	
	UpdateWindow(directory_config_window);
	return directory_config_window;
}


void Directory_Config_Window_CreateChildWindows(HWND hWnd)
{
	// Unlike BIOS/Misc Files, there's only one frame.
	HWND grpBox = NULL;
	HWND lblTitle, txtEntry, btnChange;
	
	// Positioning.
	const int grpBox_Left = 8;
	int grpBox_Top = 8, grpBox_Height = 24;
	int entryTop;
	int dir = 0;
	
	// Create the frame.
	grpBox = CreateWindow(WC_BUTTON, "Configure Directories",
			      WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
			      grpBox_Left, grpBox_Top, frameWidth, grpBox_Height,
			      hWnd, NULL, ghInstance, NULL);
	
	// Set the font for the groupbox title.
	SetWindowFont(grpBox, fntMain, TRUE);
	
	// Add the directory entries.
	while (DirEntries[dir].title)
	{
		grpBox_Height += 24;
		entryTop = 20 + (dir * 24);
		
		// Create the label for the title.
		lblTitle = CreateWindow(WC_STATIC, DirEntries[dir].title,
					WS_CHILD | WS_VISIBLE | SS_LEFT,
					grpBox_Left + 8, grpBox_Top + entryTop, 72, 16,
					hWnd, NULL, ghInstance, NULL);
		
		// Set the font for the label.
		SetWindowFont(lblTitle, fntMain, TRUE);
		
		// Create the textbox for the entry.
		txtEntry = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, DirEntries[dir].entry,
					  WS_CHILD | WS_VISIBLE | WS_TABSTOP | SS_LEFT | ES_AUTOHSCROLL,
					  grpBox_Left+8+72+8, grpBox_Top+entryTop,
					  frameWidth-(8+72+16+72+8), 20,
					  hWnd, NULL, ghInstance, NULL);
		
		// Set the font for the entry.
		SetWindowFont(txtEntry, fntMain, TRUE);
		
		// Create the change button for the entry.
		btnChange = CreateWindow(WC_BUTTON, "Change...",
					 WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					 grpBox_Left+frameWidth-(72+8), grpBox_Top+entryTop, 72, 20,
					 hWnd, (HMENU)(IDC_BTN_CHANGE + dir), ghInstance, NULL);
		
		// Set the font for the button.
		SetWindowFont(btnChange, fntMain, TRUE);
		
		// Save the text entry hWnd for later.
		dc_txtEntry[dir] = txtEntry;
		
		// Next directory.
		dir++;
	}
	
	// Increase the height of the group box.
	SetWindowPos(grpBox, NULL, 0, 0, frameWidth, grpBox_Height, SWP_NOMOVE | SWP_NOZORDER);
	
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
	SetFocus(dc_txtEntry[0]);
}

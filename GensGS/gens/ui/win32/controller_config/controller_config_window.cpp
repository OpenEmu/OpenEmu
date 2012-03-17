/***************************************************************************
 * Gens: (Win32) Controller Configuration Window.                          *
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

#include "controller_config_window.hpp"
#include "controller_config_window_callbacks.hpp"
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

// DirectInput access is needed.
#include "input/input_dinput.hpp"

// Controls
HWND cc_chkTeamPlayer[2];
HWND cc_lblPlayer[8];
HWND cc_cboControllerType[8];
HWND cc_btnReconfigure[8];
HWND cc_lblSettingKeys;

static WNDCLASS WndClass;
HWND controller_config_window = NULL;

// Internal copy of Keys_Def[], which is copied when OK is clicked.
struct KeyMap keyConfig[8];

// Controller Group Box size.
static const unsigned short grpBox_Height = 140;
static const unsigned short grpBox_Width = 248;

// Note Group Box size.
static const unsigned short grpBox_Note_Width = 288;
static const unsigned short grpBox_Note_Height = 168;

// Window size.
static const unsigned short wndWidth = 8+grpBox_Width+8+grpBox_Note_Width+8;
static const unsigned short wndHeight = (grpBox_Height*2)+24;

/**
 * create_controller_config_window(): Create the Controller Configuration Window.
 * @return Game Genie Window.
 */
HWND create_controller_config_window(void)
{
	if (controller_config_window)
	{
		// Controller Configuration window is already created. Set focus.
		// TODO: Figure out how to do this.
		ShowWindow(controller_config_window, 1);
		return NULL;
	}
	
	// Create the window class.
	WndClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = Controller_Config_Window_WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = ghInstance;
	WndClass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_SONIC_HEAD));
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = "Gens_Controller_Config";
	
	RegisterClass(&WndClass);
	
	// Create the window.
	controller_config_window = CreateWindowEx(NULL, "Gens_Controller_Config", "Controller Configuration",
						  WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
						  CW_USEDEFAULT, CW_USEDEFAULT,
						  wndWidth, wndHeight,
						  Gens_hWnd, NULL, ghInstance, NULL);
	
	// Set the actual window size.
	Win32_setActualWindowSize(controller_config_window, wndWidth, wndHeight);
	
	// Center the window on the Gens window.
	Win32_centerOnGensWindow(controller_config_window);
	
	UpdateWindow(controller_config_window);
	return controller_config_window;
}


void Controller_Config_Window_CreateChildWindows(HWND hWnd)
{
	// Create the two port boxes.
	unsigned short i, j, k;
	char tmp[64];
	HWND grpBox, lblNote;
	
	const unsigned short controllerOrder[2][4] = {{0, 2, 3, 4}, {1, 5, 6, 7}};
	unsigned short curOrder;
	
	// TODO: Move this somewhere else?
	const char* PadTypes[2] =
	{
		"3 buttons",
		"6 buttons",
	};
	
	unsigned short grpBox_Top = 8;
	for (i = 0; i < 2; i++)
	{
		// Create the groupbox.
		sprintf(tmp, "Port %d", i + 1);
		grpBox = CreateWindow(WC_BUTTON, tmp,
				      WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
				      8, grpBox_Top, grpBox_Width, grpBox_Height,
				      hWnd, NULL, ghInstance, NULL);
		SetWindowFont(grpBox, fntMain, TRUE);
		
		// Create the Teamplayer checkbox.
		cc_chkTeamPlayer[i] = CreateWindow(WC_BUTTON, "Use Teamplayer",
						   WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
						   8+8, grpBox_Top+16, 96, 16,
						   hWnd, (HMENU)(IDC_CONTROLLER_CONFIG_CHKTEAMPLAYER + (i+1)),
						   ghInstance, NULL);
		SetWindowFont(cc_chkTeamPlayer[i], fntMain, TRUE);
		
		// Create the four players.
		for (j = 0; j < 4; j++)
		{
			curOrder = controllerOrder[i][j];
			
			// Label
			sprintf(tmp, "Player %d%c", i + 1, (j == 0 ? 0x00 : 'A' + j));
			cc_lblPlayer[curOrder] = CreateWindow(
							WC_STATIC, tmp,
							WS_CHILD | WS_VISIBLE | SS_LEFT,
							8+8, grpBox_Top+16+16+4+(j*24)+4, 48, 16,
							hWnd, NULL, ghInstance, NULL);
			SetWindowFont(cc_lblPlayer[curOrder], fntMain, TRUE);
			
			// Controller type dropdown
			cc_cboControllerType[curOrder] = CreateWindow(
								WC_COMBOBOX, tmp,
								WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,
								8+8+48+8, grpBox_Top+16+16+4+(j*24), 80, 23*2,
								hWnd, NULL, ghInstance, NULL);
			SetWindowFont(cc_cboControllerType[curOrder], fntMain, TRUE);
			
			for (k = 0; k < 2; k++)
			{
				ComboBox_AddString(cc_cboControllerType[curOrder], PadTypes[k]);
			}
			
			// Reconfigure button
			cc_btnReconfigure[curOrder] = CreateWindow(
								WC_BUTTON, "Reconfigure",
								WS_CHILD | WS_VISIBLE | WS_TABSTOP,
								8+8+48+8+80+8, grpBox_Top+16+16+4+(j*24), 75+8, 23,
								hWnd, (HMENU)(IDD_CONTROLLER_CONFIG_RECONFIGURE + curOrder),
								ghInstance, NULL);
			SetWindowFont(cc_btnReconfigure[curOrder], fntMain, TRUE);
		}
		
		// Next group box.
		grpBox_Top += grpBox_Height + 8;
	}
	
	// Note.
	const char* strNote =
			"Players 1B, 1C, and 1D are enabled only if\n"
			"teamplayer is enabled on Port 1.\n\n"
			"Players 2B, 2C, and 2D are enabled only if\n"
			"teamplayer is enabled on Port 2.\n\n"
			"Only a few games support teamplayer (games which\n"
			"have 4 player support), so don't forget to use the\n"
			"\"load config\" and \"save config\" options. :)\n\n"
			"Controller configuration is applied when OK is clicked.";
	
	// Create the Note group box.
	grpBox = CreateWindow(WC_BUTTON, "Note",
			      WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
			      8+grpBox_Width+8, 8, grpBox_Note_Width, grpBox_Note_Height,
			      hWnd, NULL, ghInstance, NULL);
	SetWindowFont(grpBox, fntMain, TRUE);
	
	// Create the note label inside of the group box.
	lblNote = CreateWindow(WC_STATIC, strNote,
			       WS_CHILD | WS_VISIBLE | SS_LEFT,
			       8, 16, grpBox_Note_Width-8-8, grpBox_Note_Height-16-8,
			       grpBox, NULL, ghInstance, NULL);
	SetWindowFont(lblNote, fntMain, TRUE);
	
	// Create the Setting Keys group box.
	grpBox = CreateWindow(WC_BUTTON, "Setting Keys",
			      WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
			      8+grpBox_Width+8, 8+grpBox_Note_Height+8, grpBox_Note_Width, 64,
			      hWnd, NULL, ghInstance, NULL);
	SetWindowFont(grpBox, fntMain, TRUE);
	
	// Create the Setting Keys label.
	cc_lblSettingKeys = CreateWindow(WC_STATIC, NULL,
					 WS_CHILD | WS_VISIBLE | SS_CENTER,
					 8, 16, grpBox_Note_Width-8-8, 36,
					 grpBox, NULL, ghInstance, NULL);
	SetWindowFont(cc_lblSettingKeys, fntMain, TRUE);
	
	// Buttons
	const unsigned short btnTop = wndHeight-8-23;
	const unsigned short btnLeft = wndWidth-8-75-8-75-8-75;
	HWND btnOK, btnApply, btnCancel;
	
	btnOK = CreateWindow(WC_BUTTON, "&OK",
			     WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
			     btnLeft, btnTop, 75, 23,
			     hWnd, (HMENU)IDC_BTN_OK, ghInstance, NULL);
	SetWindowFont(btnOK, fntMain, TRUE);
	
	btnApply = CreateWindow(WC_BUTTON, "&Apply",
				WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				btnLeft+75+8, btnTop, 75, 23,
				hWnd, (HMENU)IDC_BTN_APPLY, ghInstance, NULL);
	SetWindowFont(btnApply, fntMain, TRUE);
	
	btnCancel = CreateWindow(WC_BUTTON, "&Cancel",
				 WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				 btnLeft+75+8+75+8, btnTop, 75, 23,
				 hWnd, (HMENU)IDC_BTN_CANCEL, ghInstance, NULL);
	SetWindowFont(btnCancel, fntMain, TRUE);
	
	// Set focus to Player 1's "Reconfigure" button.
	SetFocus(cc_btnReconfigure[0]);
}

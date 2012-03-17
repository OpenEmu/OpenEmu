/***************************************************************************
 * Gens: (Win32) Game Genie Window.                                        *
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

#include "game_genie_window.h"
#include "game_genie_window_callbacks.h"
#include "gens/gens_window.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "emulator/gens.hpp"
#include "emulator/g_main.hpp"

#include <windowsx.h>

// Gens Win32 resources
#include "ui/win32/resource.h"

// Win32 common controls
#include <commctrl.h>

static WNDCLASS WndClass;
HWND game_genie_window = NULL;

// Controls
HWND gg_txtCode = NULL;
HWND gg_txtName = NULL;
HWND gg_lstvCodes = NULL;

// Old window procedures for gg_txtCode and gg_txtName.
WNDPROC gg_txtCode_oldProc;
WNDPROC gg_txtName_oldProc;

// Window width.
static const int wndWidth = 438;


/**
 * create_game_genie_window(): Create the Game Genie Window.
 * @return Game Genie Window.
 */
HWND create_game_genie_window(void)
{
	if (game_genie_window)
	{
		// Game Genie window is already created. Set focus.
		// TODO: Figure out how to do this.
		ShowWindow(game_genie_window, 1);
		return NULL;
	}
	
	// Create the window class.
	WndClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = Game_Genie_Window_WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = ghInstance;
	WndClass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_SONIC_HEAD));
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = "Gens_Game_Genie";
	
	RegisterClass(&WndClass);
	
	// Create the window.
	game_genie_window = CreateWindowEx(NULL, "Gens_Game_Genie", "Game Genie",
					   WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
					   CW_USEDEFAULT, CW_USEDEFAULT,
					   wndWidth, 316,
					   Gens_hWnd, NULL, ghInstance, NULL);
	
	// Set the actual window size.
	Win32_setActualWindowSize(game_genie_window, wndWidth, 316);
	
	// Center the window on the Gens window.
	Win32_centerOnGensWindow(game_genie_window);
	
	UpdateWindow(game_genie_window);
	return game_genie_window;
}


void Game_Genie_Window_CreateChildWindows(HWND hWnd)
{
	// Description labels.
	HWND lblInfoTitle, lblInfo;
	
	// Strings
	const char* strInfoTitle = "Information about Game Genie / Patch codes";
	const char* strInfo =
			"Both Game Genie codes and Patch codes are supported.\n"
			"Check the box next to the code to activate it.\n"
			"Syntax for Game Genie codes: XXXX-YYYY\n"
			"Syntax for Patch codes: AAAAAA-DDDD (address-data)";
	
	// Info Title
	lblInfoTitle = CreateWindow(WC_STATIC, strInfoTitle,
				    WS_CHILD | WS_VISIBLE | SS_LEFT,
				    8, 8, 256, 16, hWnd, NULL, ghInstance, NULL);
	SetWindowFont(lblInfoTitle, fntTitle, TRUE);
	
	// Info
	lblInfo = CreateWindow(WC_STATIC, strInfo,
			       WS_CHILD | WS_VISIBLE | SS_LEFT,
			       8, 24, wndWidth-16, 68, hWnd, NULL, ghInstance, NULL);
	SetWindowFont(lblInfo, fntMain, TRUE);
	
	// Code and Name boxes, plus "Add Code" button.
	HWND lblCode, btnAddCode;
	HWND lblName;
	
	// Code label
	lblCode = CreateWindow(WC_STATIC, "Code",
			       WS_CHILD | WS_VISIBLE | SS_LEFT,
			       8, 24+68+8, 32, 16, hWnd, NULL, ghInstance, NULL);
	SetWindowFont(lblCode, fntMain, TRUE);
	
	// Code entry
	gg_txtCode = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, NULL,
				    WS_CHILD | WS_VISIBLE | WS_TABSTOP | SS_LEFT | ES_AUTOHSCROLL,
				    8+32+8, 24+68+8, wndWidth - (8+32+8+64+8+8+16), 20,
				    hWnd, NULL, ghInstance, NULL);
	SetWindowFont(gg_txtCode, fntMain, TRUE);
	gg_txtCode_oldProc = (WNDPROC)SetWindowLongPtr(gg_txtCode, GWL_WNDPROC, (LONG_PTR)Game_Genie_TextBox_WndProc);
	
	// Name label
	lblName = CreateWindow(WC_STATIC, "Name",
			       WS_CHILD | WS_VISIBLE | SS_LEFT,
			       8, 24+68+8+24, 32, 16,
			       hWnd, NULL, ghInstance, NULL);
	SetWindowFont(lblName, fntMain, TRUE);
	
	// Name entry
	gg_txtName = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, NULL,
				    WS_CHILD | WS_VISIBLE | WS_TABSTOP | SS_LEFT | ES_AUTOHSCROLL,
				    8+32+8, 24+68+8+24, wndWidth - (8+32+8+64+8+8+16), 20,
				    hWnd, NULL, ghInstance, NULL);
	SetWindowFont(gg_txtName, fntMain, TRUE);
	gg_txtName_oldProc = (WNDPROC)SetWindowLongPtr(gg_txtName, GWL_WNDPROC, (LONG_PTR)Game_Genie_TextBox_WndProc);
	
	// Add Code
	btnAddCode = CreateWindow(WC_BUTTON, "&Add Code",
				  WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				  wndWidth - (64+8+16), 24+68+8, 63+16, 20,
				  hWnd, IDC_BTN_ADD, ghInstance, NULL);
	SetWindowFont(btnAddCode, fntMain, TRUE);
	
	// ListView
	gg_lstvCodes = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, "",
				      WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_REPORT,
				      8, 24+68+8+24+24, wndWidth - (8+8), 128,
				      hWnd, NULL, ghInstance, NULL);
	SetWindowFont(gg_lstvCodes, fntMain, TRUE);
	SendMessage(gg_lstvCodes, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
	
	// Create the ListView columns.
	LV_COLUMN lvCol;
	memset(&lvCol, 0, sizeof(lvCol));
	lvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	
	// Code
	lvCol.pszText = "Code";
	lvCol.cx = 128;
	SendMessage(gg_lstvCodes, LVM_INSERTCOLUMN, 0, (LPARAM)&lvCol);
	
	// Name
	lvCol.pszText = "Name";
	lvCol.cx = 256;
	SendMessage(gg_lstvCodes, LVM_INSERTCOLUMN, 1, (LPARAM)&lvCol);
	
	// Buttons
	const int btnTop = 24+68+8+24+24+128+8;
	HWND btnOK, btnApply, btnCancel, btnDeactivateAll, btnDelete;
	
	btnOK = CreateWindow(WC_BUTTON, "&OK", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
			     8, btnTop, 75, 23,
			     hWnd, (HMENU)IDC_BTN_OK, ghInstance, NULL);
	SetWindowFont(btnOK, fntMain, TRUE);
	
	btnApply = CreateWindow(WC_BUTTON, "&Apply", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				8+75+8, btnTop, 75, 23,
				hWnd, (HMENU)IDC_BTN_APPLY, ghInstance, NULL);
	SetWindowFont(btnApply, fntMain, TRUE);
	
	btnCancel = CreateWindow(WC_BUTTON, "&Cancel", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				 8+75+8+75+8, btnTop, 75, 23,
				 hWnd, (HMENU)IDC_BTN_CANCEL, ghInstance, NULL);
	SetWindowFont(btnCancel, fntMain, TRUE);
	
	btnDelete = CreateWindow(WC_BUTTON, "&Delete", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				 8+75+8+75+8+75+8, btnTop, 75, 23,
				 hWnd, (HMENU)IDC_BTN_DELETE, ghInstance, NULL);
	SetWindowFont(btnDelete, fntMain, TRUE);
	
	btnDeactivateAll = CreateWindow(WC_BUTTON, "Deac&tivate All", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					8+75+8+75+8+75+8+75+8, btnTop, 75+16, 23,
					hWnd, (HMENU)IDC_BTN_DEACTIVATEALL, ghInstance, NULL);
	SetWindowFont(btnDeactivateAll, fntMain, TRUE);
	
	// Set focus to the Code textbox.
	SetFocus(gg_txtCode);
}

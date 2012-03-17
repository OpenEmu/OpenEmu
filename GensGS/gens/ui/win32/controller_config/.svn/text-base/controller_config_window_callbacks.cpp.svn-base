/***************************************************************************
 * Gens: (Win32) Controller Configuration Window - Callback Functions.     *
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

#include <stdio.h>
#include <string.h>

#include "controller_config_window.hpp"
#include "controller_config_window_callbacks.hpp"
#include "controller_config_window_misc.hpp"

#include "emulator/gens.hpp"
#include "emulator/g_main.hpp"

#include <windows.h>
#include <windowsx.h>

// Gens Win32 resources
#include "ui/win32/resource.h"

// DirectInput access is needed.
#include "input/input_dinput.hpp"


LRESULT CALLBACK Controller_Config_Window_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_CREATE:
			Controller_Config_Window_CreateChildWindows(hWnd);
			break;
		
		case WM_CLOSE:
			DestroyWindow(controller_config_window);
			return 0;
		
		case WM_MENUSELECT:
		case WM_ENTERSIZEMOVE:
		case WM_NCLBUTTONDOWN:
		case WM_NCRBUTTONDOWN:
			// Prevent audio stuttering when one of the following events occurs:
			// - Menu is opened.
			// - Window is resized.
			// - Left/Right mouse button down on title bar.
			audio->clearSoundBuffer();
			break;
		
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
			{
				// Set the DirectInput cooperative level.
				reinterpret_cast<Input_DInput*>(input)->setCooperativeLevel(hWnd);
				
				// Initialize joysticks.
				reinterpret_cast<Input_DInput*>(input)->initJoysticks(hWnd);
			}
			break;
		
		case WM_COMMAND:
			// Button press, or Enter pressed in textbox
			switch (LOWORD(wParam))
			{
				case IDC_CONTROLLER_CONFIG_CHKTEAMPLAYER1:
				case IDC_CONTROLLER_CONFIG_CHKTEAMPLAYER2:
					// Teamplayer. Adjust the teamplayer settings.
					adjustTeamplayer((LOWORD(wParam) & 0x0F) - 1);
					break;
				
				case IDOK: // Standard dialog button ID
				case IDC_BTN_OK:
				case IDC_BTN_SAVE:
					Controller_Config_Save();
					DestroyWindow(hWnd);
					break;
				
				case IDC_BTN_APPLY:
					Controller_Config_Save();
					break;
				
				case IDCANCEL: // Standard dialog button ID
				case IDC_BTN_CANCEL:
					DestroyWindow(hWnd);
					break;
				
				default:
					if ((LOWORD(wParam) & 0xFF00) == IDD_CONTROLLER_CONFIG_RECONFIGURE)
					{
						// Reconfigure a controller.
						int player = LOWORD(wParam) & 0xFF;
						Reconfigure_Input(player, ComboBox_GetCurSel(cc_cboControllerType[player]));
					}
					break;
			}
			break;
		
		case WM_DESTROY:
			if (hWnd != controller_config_window)
				break;
			
			controller_config_window = NULL;
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}


/**
 * adjustTeamplayer(): Adjust Teamplayer settings.
 * @param player 0 == player 1; 1 == player 2
 */
void adjustTeamplayer(unsigned short player)
{
	if (player >= 2)
		return;
	
	// Enable/Disable the appropriate controller options depending on the port number.
	
	// Get whether this checkbox is checked or not.
	bool thisChecked = (Button_GetCheck(cc_chkTeamPlayer[player]) == BST_CHECKED);
	
	// Loop for players xB through xD.
	unsigned short tpPlayer;
	const unsigned short tpPlayerMin = (player == 0 ? 2 : 5);
	const unsigned short tpPlayerMax = (player == 0 ? 4 : 7);
	for (tpPlayer = tpPlayerMin; tpPlayer <= tpPlayerMax; tpPlayer++)
	{
		Static_Enable(cc_lblPlayer[tpPlayer], thisChecked);
		ComboBox_Enable(cc_cboControllerType[tpPlayer], thisChecked);
		Button_Enable(cc_btnReconfigure[tpPlayer], thisChecked);
	}
}

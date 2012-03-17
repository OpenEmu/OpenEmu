/***************************************************************************
 * Gens: (GTK+) Directory Configuration Window - Callback Functions.       *
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
#include "directory_config_window_misc.hpp"

#include "emulator/gens.hpp"
#include "gens_ui.hpp"

// Gens Win32 resources
#include "ui/win32/resource.h"


LRESULT CALLBACK Directory_Config_Window_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
			Directory_Config_Window_CreateChildWindows(hWnd);
			break;
		
		case WM_CLOSE:
			DestroyWindow(directory_config_window);
			return 0;
		
		case WM_MENUSELECT:
		case WM_ENTERSIZEMOVE:
		case WM_NCLBUTTONDOWN:
		case WM_NCRBUTTONDOWN:
			// Prevent audio stuttering when one of the following events occurs:
			// - Menu is opened.
			// - Window is resized.
			// - Left/Right mouse button down on title bar.
			Win32_ClearSoundBuffer();
			break;
		
		case WM_COMMAND:
			// Button press
			switch (LOWORD(wParam))
			{
				case IDOK: // Standard dialog button ID
				case IDC_BTN_OK:
				case IDC_BTN_SAVE:
					Directory_Config_Save();
					DestroyWindow(hWnd);
					break;
				
				case IDC_BTN_APPLY:
					Directory_Config_Save();
					break;
				
				case IDCANCEL: // Standard dialog button ID
				case IDC_BTN_CANCEL:
					DestroyWindow(hWnd);
					break;
				
				default:
					if ((LOWORD(wParam) & 0xFF00) == IDC_BTN_CHANGE)
					{
						// Change a directory.
						Directory_Config_Change(LOWORD(wParam) & 0xFF);
					}
					break;
			}
			break;
		
		case WM_DESTROY:
			if (hWnd != directory_config_window)
				break;
			
			directory_config_window = NULL;
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}

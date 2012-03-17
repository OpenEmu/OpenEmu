/***************************************************************************
 * Gens: (Win32) BIOS/Misc Files Window - Callback Functions.              *
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
#include "bios_misc_files_window_misc.hpp"

#include "emulator/gens.hpp"

// Gens Win32 resources
#include "ui/win32/resource.h"


LRESULT CALLBACK BIOS_Misc_Files_Window_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_CREATE:
			BIOS_Misc_Files_Window_CreateChildWindows(hWnd);
			break;
		
		case WM_CLOSE:
			DestroyWindow(bios_misc_files_window);
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
					BIOS_Misc_Files_Save();
					DestroyWindow(hWnd);
					break;
				
				case IDC_BTN_APPLY:
					BIOS_Misc_Files_Save();
					break;
				
				case IDCANCEL: // Standard dialog button ID
				case IDC_BTN_CANCEL:
					DestroyWindow(hWnd);
					break;
				
				default:
					if ((LOWORD(wParam) & 0xFF00) == IDC_BTN_CHANGE)
					{
						// Change a file.
						BIOS_Misc_Files_Change(LOWORD(wParam) & 0xFF);
					}
					break;
			}
			break;
		
		case WM_DESTROY:
			if (hWnd != bios_misc_files_window)
				break;
			
			bios_misc_files_window = NULL;
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/***************************************************************************
 * Gens: (GTK+) Color Adjustment Window - Callback Functions.              *
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

#include "color_adjust_window.h"
#include "color_adjust_window_callbacks.h"
#include "color_adjust_window_misc.h"

#include "emulator/gens.hpp"
#include "emulator/g_main.hpp"
#include "gens_ui.hpp"

// Gens Win32 resources
#include "ui/win32/resource.h"

// Win32 common controls
#include <commctrl.h>


LRESULT CALLBACK Color_Adjust_Window_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	char buf[16];
	int scrlPos;
	
	switch(message)
	{
		case WM_CREATE:
			Color_Adjust_Window_CreateChildWindows(hWnd);
			break;
		
		case WM_CLOSE:
			DestroyWindow(color_adjust_window);
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
					CA_Save();
					DestroyWindow(hWnd);
					break;
				
				case IDC_BTN_APPLY:
					CA_Save();
					break;
				
				case IDCANCEL: // Standard dialog button ID
				case IDC_BTN_CANCEL:
					DestroyWindow(hWnd);
					break;
			}
			break;
		
		case WM_HSCROLL:
			// Trackbar scroll
			switch (LOWORD(wParam))
			{
				case TB_THUMBPOSITION:
				case TB_THUMBTRACK:
					// Scroll position is in the high word of wParam.
					scrlPos = (signed short)HIWORD(wParam);
					break;
				
				default:
					// Send TBM_GETPOS to the trackbar to get the position.
					scrlPos = SendMessage((HWND)lParam, TBM_GETPOS, 0, 0);
					break;
			}
			
			// Convert the scroll position to a string.
			sprintf(buf, "%d", scrlPos);
			
			// Set the value label.
			if ((HWND)lParam == ca_trkContrast)
				SetWindowText(ca_lblContrastVal, buf);
			else if ((HWND)lParam == ca_trkBrightness)
				SetWindowText(ca_lblBrightnessVal, buf);
			
			break;
		
		case WM_DESTROY:
			if (hWnd != color_adjust_window)
				break;
			
			color_adjust_window = NULL;
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}

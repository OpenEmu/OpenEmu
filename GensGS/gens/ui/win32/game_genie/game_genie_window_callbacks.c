/***************************************************************************
 * Gens: (Win32) Game Genie Window - Callback Functions.                   *
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

#include <string.h>

#include "game_genie_window.h"
#include "game_genie_window_callbacks.h"
#include "game_genie_window_misc.h"

#include "emulator/gens.hpp"
#include "gens_ui.hpp"

#include <windows.h>

// Gens Win32 resources
#include "ui/win32/resource.h"

void AddCode_Signal(HWND hWnd);


LRESULT CALLBACK Game_Genie_Window_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_CREATE:
			Game_Genie_Window_CreateChildWindows(hWnd);
			break;
		
		case WM_CLOSE:
			DestroyWindow(game_genie_window);
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
			// Button press, or Enter pressed in textbox
			switch (LOWORD(wParam))
			{
				case IDOK: // Standard dialog button ID
				case IDC_BTN_OK:
				case IDC_BTN_SAVE:
					GG_SaveCodes();
					DestroyWindow(hWnd);
					break;
				
				case IDC_BTN_APPLY:
					GG_SaveCodes();
					break;
				
				case IDCANCEL: // Standard dialog button ID
				case IDC_BTN_CANCEL:
					DestroyWindow(hWnd);
					break;
				
				case IDC_BTN_ADD:
					AddCode_Signal(hWnd);
					break;
				
				case IDC_BTN_DELETE:
					GG_DelSelectedCode();
					break;
				
				case IDC_BTN_DEACTIVATEALL:
					GG_DeactivateAllCodes();
					break;
			}
			break;
		
		case WM_DESTROY:
			if (hWnd != game_genie_window)
				break;
			
			game_genie_window = NULL;
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}


LRESULT CALLBACK Game_Genie_TextBox_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Game Genie textbox subclassing
	// This is only used to capture the Enter keypress.
	
	// Note: WM_GETDLGCODE may show up instead of WM_KEYDOWN,
	// since this window handles dialog messages.
	if ((message == WM_KEYDOWN || message == WM_GETDLGCODE) && wParam == VK_RETURN)
	{
		// Enter is pressed. Add the code.
		AddCode_Signal(game_genie_window);
		if (message == WM_GETDLGCODE)
			return DLGC_WANTALLKEYS;
		else //if (message == WM_KEYDOWN)
			return TRUE;
	}
	
	// Not Enter. Run the regular procedure.
	if (hWnd == gg_txtCode)
		return CallWindowProc(gg_txtCode_oldProc, hWnd, message, wParam, lParam);
	else if (hWnd == gg_txtName)
		return CallWindowProc(gg_txtName_oldProc, hWnd, message, wParam, lParam);
	
	// Unknown control.
	return FALSE;
}


void AddCode_Signal(HWND hWnd)
{
	if (hWnd != game_genie_window)
		return;
	
	int length;
	char code[64], name[256];
	
	// Get the text.
	GetWindowText(gg_txtCode, code, sizeof(code));
	GetWindowText(gg_txtName, name, sizeof(name));
	
	// Get the length of the code.
	// Game Genie codes are 9 characters long. ("XXXX-YYYY")
	// Patch codes are 11 characters long. ("AAAAAA-DDDD")
	// TODO: Better verification of valid codes.
	// TODO: Automatically add a hyphen for 8-char Game Genie codes and a colon for 10-char Patch codes.
	length = strlen(code);
	if (length == 9 || length == 11)
		GG_AddCode(name, code, 0);
	
	// Clear the contents of the entry widgets.
	SetWindowText(gg_txtCode, "");
	SetWindowText(gg_txtName, "");
	
	// Set the focus to the code entry widget.
	SetFocus(gg_txtCode);
}


#if 0
/**
 * Code was toggled.
 */
void on_treeview_gg_list_item_toggled(GtkCellRendererToggle *cell_renderer,
				      gchar *path, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(cell_renderer);
	
	// Toggle the state of this item.
	GtkTreeIter iter;
	gboolean curState;
	
	// Get the iterator pointing to this list item.
	gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(user_data), &iter, path);
	
	// Get the current value.
	gtk_tree_model_get(GTK_TREE_MODEL(user_data), &iter, 0, &curState, -1);
	
	// Set the inverted value.
	gtk_list_store_set(GTK_LIST_STORE(user_data), &iter, 0, !curState, -1);
}


/**
 * Apply
 */
void on_button_gg_Apply_clicked(GtkButton *button, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(button);
	GENS_UNUSED_PARAMETER(user_data);
	
	GG_SaveCodes();
}


/**
 * Save
 */
void on_button_gg_Save_clicked(GtkButton *button, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(button);
	GENS_UNUSED_PARAMETER(user_data);
	
	GG_SaveCodes();
	gtk_widget_destroy(game_genie_window);
	game_genie_window = NULL;
}
#endif

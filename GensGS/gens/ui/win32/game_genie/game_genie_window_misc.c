/***************************************************************************
 * Gens: (Win32) Game Genie Window - Miscellaneous Functions.              *
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
#include "gens/gens_window.hpp"
#include "util/file/ggenie.h"
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/vdp/vdp_io.h"

#include "emulator/g_main.hpp"

#include <windows.h>

// Win32 common controls
#include <commctrl.h>

// For whatever reason, Wine's headers don't include the ListView_(Set|Get)CheckState macros.
#ifndef ListView_SetCheckState
#define ListView_SetCheckState(hwnd,iIndex,bCheck) \
	ListView_SetItemState(hwnd,iIndex,INDEXTOSTATEIMAGEMASK((bCheck)+1),LVIS_STATEIMAGEMASK)
#endif
#ifndef ListView_GetCheckState
#define ListView_GetCheckState(hwnd,iIndex) \
	((((UINT)(ListView_GetItemState(hwnd,iIndex,LVIS_STATEIMAGEMASK)))>>12)-1)
#endif

/**
 * Open_Game_Genie(): Opens the Game Genie window.
 */
void Open_Game_Genie(void)
{
	HWND gg = create_game_genie_window();
	if (!gg)
	{
		// Either an error occurred while creating the Game Genie window,
		// or the Game Genie window is already created.
		return;
	}
	
	// TODO: Make the window modal.
	//gtk_window_set_transient_for(GTK_WINDOW(bmf), GTK_WINDOW(gens_window));
	
	int i;
	ice = 0;
	
	// Go through the list of codes and add them to the listview.
	for (i = 0; i < 256; i++)
	{
		if (Game_Genie_Codes[i].code[0] == 0)
			continue;
		
		GG_AddCode(Game_Genie_Codes[i].name, Game_Genie_Codes[i].code, Game_Genie_Codes[i].active);
		
		// If the ROM is loaded, and this code applies to ROM data, apply the code.
		// Or something.
		// TODO: Figure out what this actually does.
		if ((Game_Genie_Codes[i].restore != 0xFFFFFFFF) &&
		    (Game_Genie_Codes[i].addr < Rom_Size) &&
		    (Genesis_Started))
		{
			Rom_Data[Game_Genie_Codes[i].addr] = (unsigned char)(Game_Genie_Codes[i].restore & 0xFF);
			Rom_Data[Game_Genie_Codes[i + 1].addr] = (unsigned char)((Game_Genie_Codes[i].restore & 0xFF00) >> 8);
		}
	}
	
	// Show the Game Genie window.
	ShowWindow(gg, 1);
}


/**
 * GG_AddCode(): Add a code to the Game Genie treeview.
 * @param name Name of the code.
 * @param code Actual code.
 * @param enabled If non-zero, the code is enabled.
 */
void GG_AddCode(const char *name, const char *code, int enabled)
{
	char upperCode[16];
	int i;
	
	// Convert the code to uppercase.
	for (i = 0; i < 16; i++)
	{
		if (code[i] >= 'a' && code[i] <= 'z')
			upperCode[i] = code[i] - ('a' - 'A');
		else
			upperCode[i] = code[i];
		
		if (upperCode[i] == 0x00)
			break;
	}
	
	// Get the number of items in the listview.
	int lvItems = SendMessage(gg_lstvCodes, LVM_GETITEMCOUNT, 0, 0);
	
	// Add the listview item.
	LV_ITEM lviCode;
	memset(&lviCode, 0x00, sizeof(lviCode));
	lviCode.mask = LVIF_TEXT;
	lviCode.cchTextMax = 256;
	lviCode.iItem = lvItems;
	
	// First column: Code
	lviCode.iSubItem = 0;
	lviCode.pszText = upperCode;
	SendMessage(gg_lstvCodes, LVM_INSERTITEM, 0, (LPARAM)&lviCode);
	
	// Second column: Name
	lviCode.iSubItem = 1;
	lviCode.pszText = name;
	SendMessage(gg_lstvCodes, LVM_SETITEM, 0, (LPARAM)&lviCode);
	
	// Set the checkbox state.
	ListView_SetCheckState(gg_lstvCodes, lvItems, enabled);
}


/**
 * GG_DelSelectedCode(): Delete the selected code.
 */
void GG_DelSelectedCode(void)
{
	// Delete all selected codes.
	int lvItems = SendMessage(gg_lstvCodes, LVM_GETITEMCOUNT, 0, 0);
	unsigned int state;
	int i;
	
	for (i = lvItems - 1; i >= 0; i--)
	{
		state = ListView_GetItemState(gg_lstvCodes, i, LVIS_SELECTED);
		if (state & LVIS_SELECTED)
		{
			// Selected item. Delete it.
			SendMessage(gg_lstvCodes, LVM_DELETEITEM, i, 0);
		}
	}
}


/**
 * GG_DeactivateAllCodes(): Deactivate all codes.
 */
void GG_DeactivateAllCodes(void)
{
	int lvItems = SendMessage(gg_lstvCodes, LVM_GETITEMCOUNT, 0, 0);
	int i;
	
	for (i = 0; i < lvItems; i++)
	{
		ListView_SetCheckState(gg_lstvCodes, i, FALSE);
	}
}


/**
 * GG_SaveCodes(): Save the codes from the ListView to Game_Genie_Codes[].
 */
void GG_SaveCodes(void)
{
	int lvItems, i, state;
	LV_ITEM lviCode;
	
	// Reinitialize the Game Genie array.
	Init_GameGenie();
	
	// Get the number of items in the listview.
	lvItems = SendMessage(gg_lstvCodes, LVM_GETITEMCOUNT, 0, 0);
	
	// Set up the LV_ITEM.
	memset(&lviCode, 0x00, sizeof(lviCode));
	lviCode.mask = LVIF_TEXT;
	
	for (i = 0; i < lvItems; i++)
	{
		lviCode.iItem = i;
		
		// Code
		lviCode.iSubItem = 0;
		lviCode.pszText = Game_Genie_Codes[i].code;
		lviCode.cchTextMax = sizeof(Game_Genie_Codes[i].code) - 1;
		SendMessage(gg_lstvCodes, LVM_GETITEMTEXT, i, (LPARAM)&lviCode);
		
		// Name
		lviCode.iSubItem = 1;
		lviCode.pszText = Game_Genie_Codes[i].name;
		lviCode.cchTextMax = sizeof(Game_Genie_Codes[i].name) - 1;
		SendMessage(gg_lstvCodes, LVM_GETITEMTEXT, i, (LPARAM)&lviCode);
		
		// State
		state = ListView_GetCheckState(gg_lstvCodes, i);
		Game_Genie_Codes[i].active = (state ? 1 : 0);
	}
	
	// Decode and apply Game Genie codes.
	// TODO: Move this somewhere else?
	for (i = 0; i < 256; i++)
	{
		if ((Game_Genie_Codes[i].code[0] != 0) &&
		    (Game_Genie_Codes[i].addr == 0xFFFFFFFF) &&
		    (Game_Genie_Codes[i].data == 0))
		{
			// Decode this entry.
			decode(Game_Genie_Codes[i].code, (struct patch*)(&(Game_Genie_Codes[i].addr)));
			
			if ((Game_Genie_Codes[i].restore = 0xFFFFFFFF) &&
			    (Game_Genie_Codes[i].addr < Rom_Size) &&
			    (Genesis_Started))
			{
				Game_Genie_Codes[i].restore = (unsigned int)(Rom_Data[Game_Genie_Codes[i].addr] & 0xFF) +
						      (unsigned int)((Rom_Data[Game_Genie_Codes[i].addr + 1] & 0xFF) << 8);
			}
		}
	}
}

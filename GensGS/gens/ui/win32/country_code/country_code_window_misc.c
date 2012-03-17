/***************************************************************************
 * Gens: (Win32) Country Code Window - Miscellaneous Functions.            *
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

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "country_code_window.h"
#include "country_code_window_callbacks.h"
#include "country_code_window_misc.h"
#include "gens/gens_window.hpp"

#include "emulator/g_main.hpp"

#include <windows.h>
#include <windowsx.h>

// Win32 common controls
#include <commctrl.h>


/**
 * Open_Country_Code(): Opens the Country Code window.
 */
void Open_Country_Code(void)
{
	HWND Country = create_country_code_window();
	if (!Country)
	{
		// Either an error occurred while creating the Country Code window,
		// or the Country Code window is already created.
		return;
	}
	
	// TODO: Make the window modal.
	//gtk_window_set_transient_for(GTK_WINDOW(Country), GTK_WINDOW(gens_window));
	
	// Show the Country Code window.
	ShowWindow(Country, 1);
	
	// Set up the country order treeview.
	// Elements in Country_Order[3] can have one of three values:
	// - 0 [USA]
	// - 1 [JAP]
	// - 2 [EUR]
	
	// Make sure the country code order is valid.
	Check_Country_Order();
	
	// Add the country codes to the listbox in the appropriate order.
	int i;
	for (i = 0; i < 3; i++)
	{
		ListBox_InsertString(cc_lstCountryCodes, i, Country_Code_String[Country_Order[i]]);
		ListBox_SetItemData(cc_lstCountryCodes, i, Country_Order[i]);
	}
}


/**
 * Country_Save(): Save the settings.
 */
void Country_Save(void)
{
	// Save settings.
	int i, countryID;
	
	for (i = 0; i < 3; i++)
	{
		Country_Order[i] = ListBox_GetItemData(cc_lstCountryCodes, i);
	}
	
	// Validate the country code order.
	Check_Country_Order();
}


/**
 * Country_MoveUp(): Move the selected country up.
 */
void Country_MoveUp(void)
{
	int curIndex = ListBox_GetCurSel(cc_lstCountryCodes);
	if (curIndex == -1 || curIndex == 0)
		return;
	
	// Swap the current item and the one above it.
	char aboveItem[32]; int aboveItemData;
	char curItem[32];   int curItemData;
	
	// Get the current text and data for each item.
	ListBox_GetText(cc_lstCountryCodes, curIndex - 1, aboveItem);
	ListBox_GetText(cc_lstCountryCodes, curIndex, curItem);
	aboveItemData = ListBox_GetItemData(cc_lstCountryCodes, curIndex - 1);
	curItemData = ListBox_GetItemData(cc_lstCountryCodes, curIndex);
	
	// Swap the strings and data.
	ListBox_DeleteString(cc_lstCountryCodes, curIndex - 1);
	ListBox_InsertString(cc_lstCountryCodes, curIndex - 1, curItem);
	ListBox_SetItemData(cc_lstCountryCodes, curIndex - 1, curItemData);
	
	ListBox_DeleteString(cc_lstCountryCodes, curIndex);
	ListBox_InsertString(cc_lstCountryCodes, curIndex, aboveItem);
	ListBox_SetItemData(cc_lstCountryCodes, curIndex, aboveItemData);
	
	// Set the current selection.
	ListBox_SetCurSel(cc_lstCountryCodes, curIndex - 1);
}


/**
 * Country_MoveDown(): Move the selected country down.
 */
void Country_MoveDown(void)
{
	int curIndex = ListBox_GetCurSel(cc_lstCountryCodes);
	if (curIndex == -1 || curIndex >= (ListBox_GetCount(cc_lstCountryCodes) - 1))
		return;
	
	// Swap the current item and the one below it.
	char curItem[32];   int curItemData;
	char belowItem[32]; int belowItemData;
	
	// Get the current text and data for each item.
	ListBox_GetText(cc_lstCountryCodes, curIndex, curItem);
	ListBox_GetText(cc_lstCountryCodes, curIndex + 1, belowItem);
	curItemData = ListBox_GetItemData(cc_lstCountryCodes, curIndex);
	belowItemData = ListBox_GetItemData(cc_lstCountryCodes, curIndex + 1);
	
	// Swap the strings and data.
	ListBox_DeleteString(cc_lstCountryCodes, curIndex);
	ListBox_InsertString(cc_lstCountryCodes, curIndex, belowItem);
	ListBox_SetItemData(cc_lstCountryCodes, curIndex, belowItemData);
	
	ListBox_DeleteString(cc_lstCountryCodes, curIndex + 1);
	ListBox_InsertString(cc_lstCountryCodes, curIndex + 1, curItem);
	ListBox_SetItemData(cc_lstCountryCodes, curIndex + 1, curItemData);
	
	// Set the current selection.
	ListBox_SetCurSel(cc_lstCountryCodes, curIndex + 1);
}

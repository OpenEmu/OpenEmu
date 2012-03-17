/***************************************************************************
 * Gens: (Win32) Main Window.                                              *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gens_window.hpp"
#include "gens_window_sync.hpp"
#include "gens_window_callbacks.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "emulator/g_main.hpp"

#ifdef GENS_DEBUGGER
#include "debugger/debugger.hpp"
#endif /* GENS_DEBUGGER */

WNDCLASS WndClass;
HWND Gens_hWnd = NULL;

// Gens Win32 resources
#include "ui/win32/resource.h"

// New menu handler.
#include "ui/common/gens/gens_menu.h"
#include "ui/common/gens/gens_menu_callbacks.hpp"
static void Win32_ParseMenu(GensMenuItem_t *menu, HMENU container);

// Accelerator table for the main Gens window. [Menu commands.]
HACCEL hAccelTable_Menu = NULL;

// Hash table containing all the menu items.
// Key is the menu ID.
win32MenuMap gensMenuMap;

// C++ includes
#include <vector>
using std::vector;

// Main menu.
HMENU MainMenu;
void create_gens_window_menubar(void);


/**
 * initGens_hWnd(): Initialize the Gens window.
 * @return hWnd.
 */
HWND initGens_hWnd(void)
{
	// This function simply initializes the base window.
	// It's needed because DirectX needs the window handle in order to set cooperative levels.
	
	WndClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = Gens_Window_WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = ghInstance;
	WndClass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_SONIC_HEAD));
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = NULL;
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = "Gens";
	
	RegisterClass(&WndClass);
	
	Gens_hWnd = CreateWindowEx(NULL, "Gens", "Gens", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
				   320 * 2, 240 * 2, NULL, NULL, ghInstance, NULL);
	
	// Accept dragged files.
	DragAcceptFiles(Gens_hWnd, TRUE);
	
	return Gens_hWnd;
}


HWND create_gens_window(void)
{
	// Create the menu bar.
	create_gens_window_menubar();
	
	return Gens_hWnd;
}


/**
 * create_gens_window_menubar(): Create the menu bar.
 */
void create_gens_window_menubar(void)
{
	// TODO: Popup menu if fullscreen.
	DestroyMenu(MainMenu);
	
	// Create the main menu.
	if (draw->fullScreen())
		MainMenu = CreatePopupMenu();
	else
		MainMenu = CreateMenu();
	
	// Menus
	Win32_ParseMenu(&gmiMain[0], MainMenu);
	
	// Set the menu bar.
	if (draw->fullScreen())
		SetMenu(Gens_hWnd, NULL);
	else
		SetMenu(Gens_hWnd, MainMenu);
}


/**
 * Win32_ParseMenu(): Parse the menu structs.
 * @param menu First item of the array of menu structs to parse.
 * @param container Container to add the menu items to.
 */
static void Win32_ParseMenu(GensMenuItem_t *menu, HMENU container)
{
	HMENU mnuSubMenu;
	string sMenuText;
	
	// Win32 InsertMenu() parameters.
	unsigned int uFlags;
	UINT_PTR uIDNewItem;
	
	// Vector of accelerators.
	static vector<ACCEL> vAccel;
	ACCEL curAccel;
	
	// If this is the first invokation of Win32_ParseMenu,
	// clear the vector of accelerators and the menu map.
	if (container == MainMenu)
	{
		vAccel.clear();
		gensMenuMap.clear();
	}
	
	while (menu->id != 0)
	{
		// Check what type of menu item this is.
		switch ((menu->flags & GMF_ITEM_MASK))
		{
			case GMF_ITEM_SEPARATOR:
				// Separator.
				uFlags = MF_BYPOSITION | MF_SEPARATOR;
				uIDNewItem = (UINT_PTR)menu->id;
				break;
			
			case GMF_ITEM_SUBMENU:
				// Submenu.
				if (!menu->submenu)
				{
					// No submenu specified. Create a normal menu item for now.
					uFlags = MF_BYPOSITION | MF_STRING;
					uIDNewItem = (UINT_PTR)menu->id;
				}
				else
				{
					// Submenu specified.
					mnuSubMenu = CreatePopupMenu();
					Win32_ParseMenu(menu->submenu, mnuSubMenu);
					uFlags = MF_BYPOSITION | MF_STRING | MF_POPUP;
					uIDNewItem = (UINT_PTR)mnuSubMenu;
					
					// Add the submenu to the menu map.
					gensMenuMap.insert(win32MenuMapItem(menu->id, mnuSubMenu));
				}
				break;
			
			case GMF_ITEM_CHECK:
			case GMF_ITEM_RADIO:
			default:
				// Menu item. (Win32 doesn't treat check or radio items as different types.)
				uFlags = MF_BYPOSITION | MF_STRING;
				uIDNewItem = (UINT_PTR)menu->id;
				break;
		}
		
		// Set the menu text.
		if (menu->text)
			sMenuText = menu->text;
		else
			sMenuText.clear();
		
		// Check for an accelerator.
		if (menu->accelKey != 0)
		{
			// Accelerator specified.
			// TODO: Add the accelerator to the accelerator table.
			curAccel.fVirt = FVIRTKEY | FNOINVERT;
			
			sMenuText += "\t";
			
			// Determine the modifier.
			if (menu->accelModifier & GMAM_CTRL)
			{
				curAccel.fVirt |= FCONTROL;
				sMenuText += "Ctrl+";
			}
			if (menu->accelModifier & GMAM_ALT)
			{
				curAccel.fVirt |= FALT;
				sMenuText += "Alt+";
			}
			if (menu->accelModifier & GMAM_SHIFT)
			{
				curAccel.fVirt |= FSHIFT;
				sMenuText += "Shift+";
			}
			
			// Determine the key.
			// TODO: Add more special keys.
			char tmpKey[8];
			switch (menu->accelKey)
			{
				case GMAK_BACKSPACE:
					curAccel.key = VK_BACK;
					sMenuText += "Backspace";
					break;
				
				case GMAK_ENTER:
					curAccel.key = VK_RETURN;
					sMenuText += "Enter";
					break;
				
				case GMAK_TAB:
					curAccel.key = VK_TAB;
					sMenuText += "Tab";
					break;
				
				case GMAK_F1: case GMAK_F2:  case GMAK_F3:  case GMAK_F4:
				case GMAK_F5: case GMAK_F6:  case GMAK_F7:  case GMAK_F8:
				case GMAK_F9: case GMAK_F10: case GMAK_F11: case GMAK_F12:
					curAccel.key = (menu->accelKey - GMAK_F1) + VK_F1;
					sprintf(tmpKey, "F%d", (menu->accelKey - GMAK_F1 + 1));
					sMenuText += string(tmpKey);
					break;
					
				default:
					curAccel.key = toupper(menu->accelKey);
					sMenuText += (char)(curAccel.key);
					break;
			}
			
			// Add the accelerator.
			if ((menu->flags & GMF_ITEM_MASK) != GMF_ITEM_SUBMENU)
			{
				curAccel.cmd = menu->id;
				vAccel.push_back(curAccel);
			}
		}
		
		// Add the menu item to the container.
		InsertMenu(container, -1, uFlags, uIDNewItem, sMenuText.c_str());
		
		// Next menu item.
		menu++;
	}
	
	// If this is the first invokation of Win32_ParseMenu, create the accelerator table.
	if (container == MainMenu)
	{
		if (hAccelTable_Menu)
			DestroyAcceleratorTable(hAccelTable_Menu);
		
		// Create the accelerator table.
		hAccelTable_Menu = CreateAcceleratorTable(&vAccel[0], vAccel.size());
		
		// Clear the vector of accelerators.
		vAccel.clear();
	}
}


/**
 * findMenuItem(): Find a submenu in the menu map.
 * @param id Submenu ID.
 * @return Submenu.
 */
HMENU findMenuItem(uint16_t id)
{
	// TODO: Make this a common function.
	
	win32MenuMap::iterator mnuIter;
	
	mnuIter = gensMenuMap.find(id & 0xFF00);
	if (mnuIter == gensMenuMap.end())
		return NULL;
	
	return (*mnuIter).second;
}

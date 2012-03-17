/***************************************************************************
 * Gens: (GTK+) Directory Configuration Window - Miscellaneous Functions.  *
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

#include "directory_config_window.h"
#include "directory_config_window_callbacks.h"
#include "directory_config_window_misc.hpp"
#include "gens/gens_window.hpp"

#include "emulator/g_main.hpp"
#include "ui/gens_ui.hpp"

#include <windows.h>


/**
 * Open_Directory_Config(): Opens the Directory Configuration window.
 */
void Open_Directory_Config(void)
{
	HWND dc = create_directory_config_window();
	if (!dc)
	{
		// Either an error occurred while creating the Controller Configuration window,
		// or the Controller Configuration window is already created.
		return;
	}
	
	// TODO: Make the window modal.
	//gtk_window_set_transient_for(GTK_WINDOW(dc), GTK_WINDOW(gens_window));
	
	// Show the Directory Configuration window.
	ShowWindow(dc, 1);
}


/**
 * Directory_Config_Change(): Change a directory.
 * @param dir Directory ID.
 */
void Directory_Config_Change(int dir)
{
	char curDir[GENS_PATH_MAX];
	char tmp[64];
	string newDir;
	
	// Get the currently entered directory.
	GetWindowText(dc_txtEntry[dir], curDir, sizeof(curDir));
	
	// Request a new file.
	sprintf(tmp, "Select %s Directory", DirEntries[dir].title);
	newDir = GensUI::selectDir(tmp, curDir, directory_config_window);
	
	// If Cancel was selected, don't do anything.
	if (newDir.length() == 0)
		return;
	
	// Set the new directory.
	SetWindowText(dc_txtEntry[dir], newDir.c_str());
}


/**
 * Directory_Config_Save(): Save the Directories.
 */
void Directory_Config_Save(void)
{
	int dir = 0;
	
	// Go through the DirEntries[] struct.
	while (DirEntries[dir].title)
	{
		// Get the entry text.
		GetWindowText(dc_txtEntry[dir], DirEntries[dir].entry, GENS_PATH_MAX);
		
		// Make sure the end of the directory has a backslash.
		// TODO: Do this in functions that use pathnames.
		int len = strlen(DirEntries[dir].entry);
		if (len > 0 && DirEntries[dir].entry[len - 1] != GENS_DIR_SEPARATOR_CHR)
			strcat(DirEntries[dir].entry, GENS_DIR_SEPARATOR_STR);
		
		// Increment the directory counter.
		dir++;
	}
}

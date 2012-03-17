/***************************************************************************
 * Gens: (GTK+) BIOS/Misc Files Window - Miscellaneous Functions.          *
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

#include "bios_misc_files_window.hpp"
#include "bios_misc_files_window_callbacks.h"
#include "bios_misc_files_window_misc.hpp"
#include "gens/gens_window.hpp"

#include "emulator/g_main.hpp"

#include <windows.h>


/**
 * Open_BIOS_Misc_Files(): Opens the BIOS/Misc Files window.
 */
void Open_BIOS_Misc_Files(void)
{
	HWND bmf = create_bios_misc_files_window();
	if (!bmf)
	{
		// Either an error occurred while creating the BIOS/Misc Files window,
		// or the BIOS/Misc Files window is already created.
		return;
	}
	
	// TODO: Make the window modal.
	//gtk_window_set_transient_for(GTK_WINDOW(bmf), GTK_WINDOW(gens_window));
	
	// Show the BIOS/Misc Files window.
	ShowWindow(bmf, 1);
}


/**
 * BIOS_Misc_Files_Change(): Change a filename.
 * @param file File ID.
 */
void BIOS_Misc_Files_Change(int file)
{
	char curFile[GENS_PATH_MAX];
	char tmp[64];
	string newFile;
	
	// Get the currently entered filename.
	GetWindowText(bmf_txtEntry[file], curFile, sizeof(curFile));
	
	// Request a new file.
	sprintf(tmp, "Select %s File", BIOSMiscFiles[file].title);
	newFile = GensUI::openFile(tmp, curFile, BIOSMiscFiles[file].filter, bios_misc_files_window);
	
	// If Cancel was selected, don't do anything.
	if (newFile.length() == 0)
		return;
	
	// Set the new file.
	SetWindowText(bmf_txtEntry[file], newFile.c_str());
}


/**
 * BIOS_Misc_Files_Save(): Save the BIOS/Misc Files.
 */
void BIOS_Misc_Files_Save(void)
{
	int file = 0;
	
	// Go through the BIOSMiscFiles[] struct.
	while (BIOSMiscFiles[file].title)
	{
		if (!BIOSMiscFiles[file].entry)
		{
			// Skip frame headers here. We just want to get file entries.
			file++;
			continue;
		}
		
		// Get the entry text.
		GetWindowText(bmf_txtEntry[file], BIOSMiscFiles[file].entry, GENS_PATH_MAX);
		
		// Increment the file counter.
		file++;
	}
}

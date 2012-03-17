/***************************************************************************
 * Gens: (Win32) Select CD-ROM Drive Window - Miscellaneous Functions.     *
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

#include "select_cdrom_window.h"
#include "select_cdrom_window_callbacks.h"
#include "select_cdrom_window_misc.hpp"
#include "gens/gens_window.hpp"

#include "ui/gens_ui.hpp"

#include "util/file/rom.hpp"
#include "gens_core/vdp/vdp_io.h"

// SegaCD
#include "emulator/g_mcd.hpp"
#include "segacd/cd_aspi.hpp"

// Synchronization.
#include "gens/gens_window_sync.hpp"

#include <windows.h>
#include <windowsx.h>


/**
 * Open_Select_CDROM(): Opens the Select CD-ROM Drive Window.
 */
void Open_Select_CDROM(void)
{
	HWND SelCD = create_select_cdrom_window();
	if (!SelCD)
	{
		// Either an error occurred while creating the Select CD-ROM Drive Window,
		// or the Select CD-ROM Drive Window is already created.
		return;
	}
	
	// TODO: Make the window modal.
	//gtk_window_set_transient_for(GTK_WINDOW(SelCD), GTK_WINDOW(gens_window));
	
	// Set the current CD-ROM drive.
	ComboBox_SetCurSel(SelCD_cdromDropdownBox, cdromDeviceID);
	if (ComboBox_GetCurSel(SelCD_cdromDropdownBox) == -1)
	{
		// Invalid CD-ROM drive ID. Select the first drive.
		ComboBox_SetCurSel(SelCD_cdromDropdownBox, 0);
	}
	
	// Show the Select CD-ROM Drive window.
	ShowWindow(SelCD, 1);
}


/**
 * SelCD_Save(): Save the settings.
 * @return 1 if settings were saved; 0 on error.
 */
int SelCD_Save(void)
{
	// Save settings.
	int tmpDrive;
	bool restartCD = false;
	
	// CD-ROM drive
	tmpDrive = ComboBox_GetCurSel(SelCD_cdromDropdownBox);
	
	// Check if the drive ID was changed.
	if (tmpDrive == cdromDeviceID)
	{
		// Drive ID wasn't changed. Don't do anything.
		return 1;
	}
	
	// Drive ID was changed.
	// This will cause a reset of the ASPI subsystem.
	if (SegaCD_Started && (CD_Load_System == CDROM_))
	{
		// ASPI is currently in use. Ask the user if they want to restart emulation.
		GensUI::MsgBox_Response response;
		response = GensUI::msgBox(
			"The CD-ROM drive is currently in use.\n"
			"Chanigng the CD-ROM drive will force the emulator to reset.\n"
			"Are you sure you want to do this?",
			"CD-ROM Drive in Use",
			GensUI::MSGBOX_ICON_WARNING | GensUI::MSGBOX_BUTTONS_YES_NO,
			select_cdrom_window);
		
		if (response == GensUI::MSGBOX_RESPONSE_NO)
		{
			// Don't change anything.
			return 0;
		}
		
		// Stop SegaCD emulation.
		// TODO: Don't allow this if Netplay is enabled.
		restartCD = true;
		ROM::freeROM(Game);
	}
	
	// Stop ASPI.
	ASPI_End();
	
	cdromDeviceID = tmpDrive;
	
	// TODO: Drive speed (maybe)
	
	// Restart ASPI.
	ASPI_Init();
	
	if (restartCD)
	{
		// Restart SegaCD emulation.
		SegaCD_Started = Init_SegaCD(NULL);
		Sync_Gens_Window();
	}
	
	// Settings saved.
	return 1;
}

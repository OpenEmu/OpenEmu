/***************************************************************************
 * Gens: (GTK+) Select CD-ROM Drive Window - Miscellaneous Functions.      *
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

#include <gtk/gtk.h>
#include "gtk-misc.h"

#include "ui/gens_ui.hpp"

#include "util/file/rom.hpp"
#include "gens_core/vdp/vdp_io.h"

// SegaCD
#include "emulator/g_mcd.hpp"
#include "segacd/cd_aspi.hpp"

// Synchronization.
#include "gens/gens_window_sync.hpp"

// CD-ROM device prefixes.
const char* CDROM_Prefix[4] = {"/dev/cdrom", "/dev/scd", "/dev/sr", NULL};


/**
 * Open_Select_CDROM(): Opens the Select CD-ROM Drive Window.
 */
void Open_Select_CDROM(void)
{
	GtkWidget *SelCD;
	GtkWidget *combo_drive, *combo_speed;
	int curPrefix, driveSpeed, i; char tmp[64];
	struct stat fileStat;
	
	SelCD = create_select_cdrom_window();
	if (!SelCD)
	{
		// Either an error occurred while creating the Select CD-ROM Drive Window,
		// or the Select CD-ROM Drive Window is already created.
		return;
	}
	gtk_window_set_transient_for(GTK_WINDOW(SelCD), GTK_WINDOW(gens_window));
	
	// Load settings.
	
	// CD-ROM drive
	combo_drive = lookup_widget(SelCD, "combo_drive");
	gtk_entry_set_text(GTK_ENTRY(GTK_BIN(combo_drive)->child), cdromDeviceName);
	
	// Add CD-ROM drives to the dropdown.
	// TODO: Improve this using udev or something.
	curPrefix = 0;
	while (CDROM_Prefix[curPrefix])
	{
		for (i = -1; i <= 9; i++)
		{
			if (i == -1)
				strcpy(tmp, CDROM_Prefix[curPrefix]);
			else
				sprintf(tmp, "%s%d", CDROM_Prefix[curPrefix], i);
				
			if (lstat(tmp, &fileStat))
			{
				// Error occurred while stat'ing this file. Skip it.
				continue;
			}
			
			// File exists.
			
			// If this isn't "/dev/cdrom*", don't add it if it's a symlink.
			if (curPrefix != 0 && S_ISLNK(fileStat.st_mode))
				continue;
			
			// If this is neither a symlink nor a block device, don't add it.
			if (!S_ISLNK(fileStat.st_mode) && !S_ISBLK(fileStat.st_mode))
				continue;
			
			// Add the device file.
			gtk_combo_box_append_text(GTK_COMBO_BOX(combo_drive), tmp);
		}
		
		// Next prefix.
		curPrefix++;
	}
	
	// Drive speed
	driveSpeed = 0;
	for (i = 0; i < 14; i++)
	{
		if (CD_DriveSpeed[i] < 0)
			break;
		else if (CD_DriveSpeed[i] == cdromSpeed)
		{
			driveSpeed = i;
			break;
		}
	}
	combo_speed = lookup_widget(SelCD, "combo_speed");
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo_speed), driveSpeed);
	
	// Show the Select CD-ROM Drive Window.
	gtk_widget_show_all(SelCD);
}


/**
 * SelCD_Save(): Save the settings.
 * @return 1 if settings were saved; 0 on error.
 */
int SelCD_Save(void)
{
	GtkWidget *combo_drive, *entry_combo_drive, *combo_speed;
	gchar *tmpDrive; int driveSpeed;
	
	// Save settings.
	bool restartASPI = false;
	bool restartCD = false;
	
	// CD-ROM drive
	combo_drive = lookup_widget(select_cdrom_window, "combo_drive");
	
	entry_combo_drive = gtk_bin_get_child(GTK_BIN(combo_drive));
	tmpDrive = strdup(gtk_entry_get_text(GTK_ENTRY(entry_combo_drive)));
	if (strlen(tmpDrive))
	{
		// Check if the drive name was changed.
		if (strncmp(tmpDrive, cdromDeviceName, sizeof(cdromDeviceName)) != 0)
		{
			// Drive name changed.
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
		}
		
		// Stop ASPI.
		ASPI_End();
		restartASPI = true;
		
		strncpy(cdromDeviceName, tmpDrive, sizeof(cdromDeviceName) - 1);
		cdromDeviceName[63] = 0x00;
	}
	free(tmpDrive);
	
	// Drive speed
	combo_speed = lookup_widget(select_cdrom_window, "combo_speed");
	driveSpeed = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_speed));
	if (driveSpeed < 0 || driveSpeed >= 14)
		driveSpeed = 0;
	cdromSpeed = CD_DriveSpeed[driveSpeed];
	
	if (restartASPI)
	{
		// Restart ASPI.
		ASPI_Init();
	}
	
	if (restartCD)
	{
		// Restart SegaCD emulation.
		SegaCD_Started = Init_SegaCD(NULL);
		Sync_Gens_Window();
	}
	
	// Settings saved.
	return 1;
}

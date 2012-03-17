/***************************************************************************
 * Gens: (GTK+) BIOS/Misc Files Window.                                    *
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
#include "gens/gens_window.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

// GENS GTK+ miscellaneous functions
#include "gtk-misc.h"

#include "emulator/g_main.hpp"

GtkWidget *bios_misc_files_window = NULL;

static GtkAccelGroup *accel_group;


// All textboxes to be displayed on the BIOS/Misc Files window are defined here.
const struct BIOSMiscFileEntry_t BIOSMiscFiles[] =
{
	{"<b><i>Genesis BIOS File</i></b>", "md_bios", (FileFilterType)0, NULL},
	{"Genesis", "md_bios", ROMFile, BIOS_Filenames.MD_TMSS},
	{"<b><i>32X BIOS Files</i></b>", "32x_bios", (FileFilterType)0, NULL},
	{"MC68000", "mc68000", ROMFile, BIOS_Filenames._32X_MC68000},
	{"Master SH2", "msh2", ROMFile, BIOS_Filenames._32X_MSH2},
	{"Slave SH2", "ssh2", ROMFile, BIOS_Filenames._32X_SSH2},
	{"<b><i>SegaCD BIOS Files</i></b>", "mcd_bios", (FileFilterType)0, NULL},
	{"USA", "mcd_bios_usa", ROMFile, BIOS_Filenames.SegaCD_US},
	{"Europe", "mcd_bios_eur", ROMFile, BIOS_Filenames.MegaCD_EU},
	{"Japan", "mcd_bios_jap", ROMFile, BIOS_Filenames.MegaCD_JP},
	{"<b><i>Miscellaneous Files</i></b>", "misc", (FileFilterType)0, NULL},
	{"7z Binary", "_7z", AnyFile, Misc_Filenames._7z_Binary},
	{"RAR Binary", "rar", AnyFile, Misc_Filenames.RAR_Binary},
	{NULL, NULL, (FileFilterType)0, NULL},
};


/**
 * create_bios_misc_files_window(): Create the BIOS/Misc Files Window.
 * @return BIOS/Misc Files Window.
 */
GtkWidget* create_bios_misc_files_window(void)
{
	GdkPixbuf *bios_misc_files_window_icon_pixbuf;
	GtkWidget *vbox_bmf;
	GtkWidget *frame_file = NULL, *label_frame_file = NULL, *table_frame_file = NULL;
	GtkWidget *label_file = NULL, *entry_file = NULL, *button_file = NULL;
	GtkWidget *hbutton_box_bmf_buttonRow;
	GtkWidget *button_bmf_Cancel, *button_bmf_Apply, *button_bmf_Save;
	
	char tmp[64];
	int file = 0, table_row = 0;
	
	if (bios_misc_files_window)
	{
		// BIOS/Misc Files window is already created. Set focus.
		gtk_widget_grab_focus(bios_misc_files_window);
		return NULL;
	}
	
	accel_group = gtk_accel_group_new();
	
	// Create the BIOS/Misc Files window.
	CREATE_GTK_WINDOW(bios_misc_files_window,
			  "bios_misc_files_window",
			  "Configure BIOS/Misc Files",
			  bios_misc_files_window_icon_pixbuf, "Gens2.ico");
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)bios_misc_files_window, "delete_event",
			 G_CALLBACK(on_bios_misc_files_window_close), NULL);
	g_signal_connect((gpointer)bios_misc_files_window, "destroy_event",
			 G_CALLBACK(on_bios_misc_files_window_close), NULL);
	
	// Create the main VBox.
	vbox_bmf = gtk_vbox_new(FALSE, 10);
	gtk_widget_set_name(vbox_bmf, "vbox_bmf");
	gtk_widget_show(vbox_bmf);
	gtk_container_add(GTK_CONTAINER(bios_misc_files_window), vbox_bmf);
	GLADE_HOOKUP_OBJECT(bios_misc_files_window, vbox_bmf, "vbox_bmf");
	
	// Create all frames. This will be fun!
	while (BIOSMiscFiles[file].title)
	{
		if (!BIOSMiscFiles[file].entry)
		{
			// No entry buffer. This is a new frame.
			sprintf(tmp, "frame_%s", BIOSMiscFiles[file].tag);
			frame_file = gtk_frame_new(NULL);
			gtk_widget_set_name(frame_file, tmp);
			gtk_container_set_border_width(GTK_CONTAINER(frame_file), 4);
			gtk_frame_set_shadow_type(GTK_FRAME(frame_file), GTK_SHADOW_ETCHED_IN);
			gtk_widget_show(frame_file);
			gtk_box_pack_start(GTK_BOX(vbox_bmf), frame_file, TRUE, TRUE, 0);
			GLADE_HOOKUP_OBJECT(bios_misc_files_window, frame_file, tmp);
			
			// Add the frame label.
			sprintf(tmp, "label_frame_%s", BIOSMiscFiles[file].tag);
			label_frame_file = gtk_label_new(BIOSMiscFiles[file].title);
			gtk_widget_set_name(label_frame_file, tmp);
			gtk_label_set_use_markup(GTK_LABEL(label_frame_file), TRUE);
			gtk_widget_show(label_frame_file);
			gtk_frame_set_label_widget(GTK_FRAME(frame_file), label_frame_file);
			GLADE_HOOKUP_OBJECT(bios_misc_files_window, label_frame_file, tmp);
			
			// Add the frame table.
			sprintf(tmp, "table_frame_%s", BIOSMiscFiles[file].tag);
			table_frame_file = gtk_table_new(1, 3, FALSE);
			gtk_widget_set_name(table_frame_file, tmp);
			gtk_container_set_border_width(GTK_CONTAINER(table_frame_file), 8);
			gtk_table_set_row_spacings(GTK_TABLE(table_frame_file), 4);
			gtk_table_set_col_spacings(GTK_TABLE(table_frame_file), 4);
			gtk_widget_show(table_frame_file);
			gtk_container_add(GTK_CONTAINER(frame_file), table_frame_file);
			GLADE_HOOKUP_OBJECT(bios_misc_files_window, table_frame_file, tmp);
			table_row = 0;
		}
		else
		{
			// File entry.
			
			// Check if the table needs to be resized.
			if (table_row > 0)
				gtk_table_resize(GTK_TABLE(table_frame_file), table_row + 1, 3);
			
			// Create the label for this file.
			sprintf(tmp, "label_%s", BIOSMiscFiles[file].tag);
			label_file = gtk_label_new(BIOSMiscFiles[file].title);
			gtk_widget_set_name(label_file, tmp);
			gtk_widget_set_size_request(label_file, 85, -1);
			gtk_misc_set_alignment(GTK_MISC(label_file), 0, 0.5);
			gtk_widget_show(label_file);
			gtk_table_attach(GTK_TABLE(table_frame_file), label_file,
					 0, 1, table_row, table_row + 1,
					 (GtkAttachOptions)(GTK_FILL),
					 (GtkAttachOptions)(0), 0, 0);
			GLADE_HOOKUP_OBJECT(bios_misc_files_window, label_file, tmp);
			
			// Create the entry for this file.
			sprintf(tmp, "entry_%s", BIOSMiscFiles[file].tag);
			entry_file = gtk_entry_new();
			gtk_widget_set_name(entry_file, tmp);
			gtk_entry_set_max_length(GTK_ENTRY(entry_file), GENS_PATH_MAX - 1);
			gtk_entry_set_text(GTK_ENTRY(entry_file), BIOSMiscFiles[file].entry);
			gtk_widget_set_size_request(entry_file, 250, -1);
			gtk_widget_show(entry_file);
			gtk_table_attach(GTK_TABLE(table_frame_file), entry_file,
					 1, 2, table_row, table_row + 1,
					 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
					 (GtkAttachOptions)(0), 0, 0);
			GLADE_HOOKUP_OBJECT(bios_misc_files_window, entry_file, tmp);
			
			// Create the button for this file.
			// TODO: Use an icon?
			sprintf(tmp, "button_%s", BIOSMiscFiles[file].tag);
			button_file = gtk_button_new_with_label("Change...");
			gtk_widget_set_name(button_file, tmp);
			gtk_widget_show(button_file);
			gtk_table_attach(GTK_TABLE(table_frame_file), button_file,
					 2, 3, table_row, table_row + 1,
					 (GtkAttachOptions)(GTK_FILL),
					 (GtkAttachOptions)(0), 0, 0);
			g_signal_connect((gpointer)button_file, "clicked",
					 G_CALLBACK(on_button_bmf_Change_clicked),
					 GINT_TO_POINTER(file));
			GLADE_HOOKUP_OBJECT(bios_misc_files_window, button_file, tmp);
			
			table_row++;
		}
		file++;
	}
	
	// HButton Box for the row of buttons on the bottom of the window
	hbutton_box_bmf_buttonRow = gtk_hbutton_box_new();
	gtk_widget_set_name(hbutton_box_bmf_buttonRow, "hbutton_box_bmf_buttonRow");
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbutton_box_bmf_buttonRow), GTK_BUTTONBOX_END);
	gtk_widget_show(hbutton_box_bmf_buttonRow);
	gtk_box_pack_start(GTK_BOX(vbox_bmf), hbutton_box_bmf_buttonRow, FALSE, FALSE, 0);
	GLADE_HOOKUP_OBJECT(bios_misc_files_window, hbutton_box_bmf_buttonRow, "hbutton_box_bmf_buttonRow");
	
	// Cancel
	button_bmf_Cancel = gtk_button_new_from_stock("gtk-cancel");
	gtk_widget_set_name(button_bmf_Cancel, "button_bmf_cancel");
	gtk_widget_show(button_bmf_Cancel);
	gtk_box_pack_start(GTK_BOX(hbutton_box_bmf_buttonRow), button_bmf_Cancel, FALSE, FALSE, 0);
	gtk_widget_add_accelerator(button_bmf_Cancel, "activate", accel_group,
				   GDK_Escape, (GdkModifierType)(0), (GtkAccelFlags)(0));
	AddButtonCallback_Clicked(button_bmf_Cancel, on_button_bmf_Cancel_clicked);
	GLADE_HOOKUP_OBJECT(bios_misc_files_window, button_bmf_Cancel, "button_bmf_Cancel");
	
	// Apply
	button_bmf_Apply = gtk_button_new_from_stock("gtk-apply");
	gtk_widget_set_name(button_bmf_Apply, "button_bmf_Apply");
	gtk_widget_show(button_bmf_Apply);
	gtk_box_pack_start(GTK_BOX(hbutton_box_bmf_buttonRow), button_bmf_Apply, FALSE, FALSE, 0);
	AddButtonCallback_Clicked(button_bmf_Apply, on_button_bmf_Apply_clicked);
	GLADE_HOOKUP_OBJECT(bios_misc_files_window, button_bmf_Apply, "button_bmf_Apply");
	
	// Save
	button_bmf_Save = gtk_button_new_from_stock("gtk-save");
	gtk_widget_set_name(button_bmf_Save, "button_bmf_Save");
	gtk_widget_show(button_bmf_Save);
	gtk_box_pack_start(GTK_BOX(hbutton_box_bmf_buttonRow), button_bmf_Save, FALSE, FALSE, 0);
	AddButtonCallback_Clicked(button_bmf_Save, on_button_bmf_Save_clicked);
	gtk_widget_add_accelerator(button_bmf_Save, "activate", accel_group,
				   GDK_Return, (GdkModifierType)(0), (GtkAccelFlags)(0));
	gtk_widget_add_accelerator(button_bmf_Save, "activate", accel_group,
				   GDK_KP_Enter, (GdkModifierType)(0), (GtkAccelFlags)(0));
	GLADE_HOOKUP_OBJECT(bios_misc_files_window, button_bmf_Save, "button_bmf_Save");
	
	// Add the accel group.
	gtk_window_add_accel_group(GTK_WINDOW(bios_misc_files_window), accel_group);
	
	return bios_misc_files_window;
}

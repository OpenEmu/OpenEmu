/***************************************************************************
 * Gens: (GTK+) Directory Configuration Window.                            *
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

#include "directory_config_window.h"
#include "directory_config_window_callbacks.h"
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

// Includes with directory defines.
// TODO: Consolidate these into Settings.PathNames
#include "util/file/save.hpp"
#include "util/gfx/imageutil.hpp"
#include "util/file/ggenie.h"
#include "util/file/rom.hpp"


GtkWidget *directory_config_window = NULL;

GtkAccelGroup *accel_group;


// All textboxes to be displayed on the BIOS/Misc Files window are defined here.
const struct DirEntry_t DirEntries[] =
{
	{"Savestates", "savestates", State_Dir},
	{"SRAM Backup", "sram", SRAM_Dir},
	{"BRAM Backup", "bram", BRAM_Dir},
	{"WAV Dump", "wavdump", PathNames.Dump_WAV_Dir},
	{"GYM Dump", "gymdump", PathNames.Dump_GYM_Dir},
	{"Screenshots", "screenshots", PathNames.Screenshot_Dir},
	{"PAT patches", "pat_patches", Patch_Dir},
	{"IPS patches", "ips_patches", IPS_Dir},
	{NULL, NULL, NULL},
};


/**
 * create_directory_config_window(): Create the Directory Configuration Window.
 * @return Directory Configuration Window.
 */
GtkWidget* create_directory_config_window(void)
{
	GdkPixbuf *directory_config_window_icon_pixbuf;
	GtkWidget *vbox_dc;
	GtkWidget *frame_direntry = NULL, *label_frame_direntry = NULL, *table_frame_direntry = NULL;
	GtkWidget *label_direntry = NULL, *entry_direntry = NULL, *button_direntry = NULL;
	GtkWidget *hbutton_box_dc_buttonRow;
	GtkWidget *button_dc_Cancel, *button_dc_Apply, *button_dc_Save;
	
	char tmp[64];
	int dir = 0;
	
	if (directory_config_window)
	{
		// Directory Configuration window is already created. Set focus.
		gtk_widget_grab_focus(directory_config_window);
		return NULL;
	}
	
	accel_group = gtk_accel_group_new();
	
	// Create the Directory Configuration window.
	CREATE_GTK_WINDOW(directory_config_window,
			  "directory_config_window",
			  "Configure Directories",
			  directory_config_window_icon_pixbuf, "Gens2.ico");
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)directory_config_window, "delete_event",
			 G_CALLBACK(on_directory_config_window_close), NULL);
	g_signal_connect((gpointer)directory_config_window, "destroy_event",
			 G_CALLBACK(on_directory_config_window_close), NULL);
	
	// Create the main VBox.
	vbox_dc = gtk_vbox_new(FALSE, 10);
	gtk_widget_set_name(vbox_dc, "vbox_dc");
	gtk_widget_show(vbox_dc);
	gtk_container_add(GTK_CONTAINER(directory_config_window), vbox_dc);
	GLADE_HOOKUP_OBJECT(directory_config_window, vbox_dc, "vbox_dc");
	
	// Create the directory entry frame.
	frame_direntry = gtk_frame_new(NULL);
	gtk_widget_set_name(frame_direntry, "frame_direntry");
	gtk_container_set_border_width(GTK_CONTAINER(frame_direntry), 4);
	gtk_frame_set_shadow_type(GTK_FRAME(frame_direntry), GTK_SHADOW_ETCHED_IN);
	gtk_widget_show(frame_direntry);
	gtk_box_pack_start(GTK_BOX(vbox_dc), frame_direntry, TRUE, TRUE, 0);
	GLADE_HOOKUP_OBJECT(directory_config_window, frame_direntry, "frame_direntry");
	
	// Add the frame label.
	label_frame_direntry = gtk_label_new("<b><i>Configure Directories</i></b>");
	gtk_widget_set_name(label_frame_direntry, "label_frame_direntry");
	gtk_label_set_use_markup(GTK_LABEL(label_frame_direntry), TRUE);
	gtk_widget_show(label_frame_direntry);
	gtk_frame_set_label_widget(GTK_FRAME(frame_direntry), label_frame_direntry);
	GLADE_HOOKUP_OBJECT(directory_config_window, label_frame_direntry, "label_frame_direntry");
	
	// Add the frame table.
	table_frame_direntry = gtk_table_new(1, 3, FALSE);
	gtk_widget_set_name(table_frame_direntry, "table_frame_direntry");
	gtk_container_set_border_width(GTK_CONTAINER(table_frame_direntry), 8);
	gtk_table_set_row_spacings(GTK_TABLE(table_frame_direntry), 4);
	gtk_table_set_col_spacings(GTK_TABLE(table_frame_direntry), 4);
	gtk_widget_show(table_frame_direntry);
	gtk_container_add(GTK_CONTAINER(frame_direntry), table_frame_direntry);
	GLADE_HOOKUP_OBJECT(directory_config_window, table_frame_direntry, "table_frame_direntry");
	
	// Create all directory entries.
	while (DirEntries[dir].title)
	{
		// Check if the table needs to be resized.
		if (dir > 0)
			gtk_table_resize(GTK_TABLE(table_frame_direntry), dir + 1, 3);
		
		// Create the label for this file.
		sprintf(tmp, "label_%s", DirEntries[dir].tag);
		label_direntry = gtk_label_new(DirEntries[dir].title);
		gtk_widget_set_name(label_direntry, tmp);
		gtk_widget_set_size_request(label_direntry, 100, -1);
		gtk_misc_set_alignment(GTK_MISC(label_direntry), 0, 0.5);
		gtk_widget_show(label_direntry);
		gtk_table_attach(GTK_TABLE(table_frame_direntry), label_direntry,
					0, 1, dir, dir + 1,
					(GtkAttachOptions)(GTK_FILL),
					(GtkAttachOptions)(0), 0, 0);
		GLADE_HOOKUP_OBJECT(directory_config_window, label_direntry, tmp);
		
		// Create the entry for this file.
		sprintf(tmp, "entry_%s", DirEntries[dir].tag);
		entry_direntry = gtk_entry_new();
		gtk_widget_set_name(entry_direntry, tmp);
		gtk_entry_set_max_length(GTK_ENTRY(entry_direntry), GENS_PATH_MAX - 1);
		gtk_entry_set_text(GTK_ENTRY(entry_direntry), DirEntries[dir].entry);
		gtk_widget_set_size_request(entry_direntry, 250, -1);
		gtk_widget_show(entry_direntry);
		gtk_table_attach(GTK_TABLE(table_frame_direntry), entry_direntry,
					1, 2, dir, dir + 1,
					(GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
					(GtkAttachOptions)(0), 0, 0);
		GLADE_HOOKUP_OBJECT(directory_config_window, entry_direntry, tmp);
		
		// Create the button for this file.
		// TODO: Use an icon?
		sprintf(tmp, "button_%s", DirEntries[dir].tag);
		button_direntry = gtk_button_new_with_label("Change...");
		gtk_widget_set_name(button_direntry, tmp);
		gtk_widget_show(button_direntry);
		gtk_table_attach(GTK_TABLE(table_frame_direntry), button_direntry,
				 2, 3, dir, dir + 1,
				 (GtkAttachOptions)(GTK_FILL),
				 (GtkAttachOptions)(0), 0, 0);
		g_signal_connect((gpointer)button_direntry, "clicked",
				 G_CALLBACK(on_button_dc_Change_clicked),
				 GINT_TO_POINTER(dir));
		GLADE_HOOKUP_OBJECT(directory_config_window, button_direntry, tmp);
		
		dir++;
	}
	
	// HButton Box for the row of buttons on the bottom of the window
	hbutton_box_dc_buttonRow = gtk_hbutton_box_new();
	gtk_widget_set_name(hbutton_box_dc_buttonRow, "hbutton_box_dc_buttonRow");
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbutton_box_dc_buttonRow), GTK_BUTTONBOX_END);
	gtk_widget_show(hbutton_box_dc_buttonRow);
	gtk_box_pack_start(GTK_BOX(vbox_dc), hbutton_box_dc_buttonRow, FALSE, FALSE, 0);
	GLADE_HOOKUP_OBJECT(directory_config_window, hbutton_box_dc_buttonRow, "hbutton_box_dc_buttonRow");
	
	// Cancel
	button_dc_Cancel = gtk_button_new_from_stock("gtk-cancel");
	gtk_widget_set_name(button_dc_Cancel, "button_dc_cancel");
	gtk_widget_show(button_dc_Cancel);
	gtk_box_pack_start(GTK_BOX(hbutton_box_dc_buttonRow), button_dc_Cancel, FALSE, FALSE, 0);
	gtk_widget_add_accelerator(button_dc_Cancel, "activate", accel_group,
				   GDK_Escape, (GdkModifierType)(0), (GtkAccelFlags)(0));
	AddButtonCallback_Clicked(button_dc_Cancel, on_button_dc_Cancel_clicked);
	GLADE_HOOKUP_OBJECT(directory_config_window, button_dc_Cancel, "button_dc_Cancel");
	
	// Apply
	button_dc_Apply = gtk_button_new_from_stock("gtk-apply");
	gtk_widget_set_name(button_dc_Apply, "button_dc_Apply");
	gtk_widget_show(button_dc_Apply);
	gtk_box_pack_start(GTK_BOX(hbutton_box_dc_buttonRow), button_dc_Apply, FALSE, FALSE, 0);
	gtk_widget_add_accelerator(button_dc_Apply, "activate", accel_group,
				   GDK_Escape, (GdkModifierType)(0), (GtkAccelFlags)(0));
	AddButtonCallback_Clicked(button_dc_Apply, on_button_dc_Apply_clicked);
	GLADE_HOOKUP_OBJECT(directory_config_window, button_dc_Apply, "button_dc_Apply");
	
	// Save
	button_dc_Save = gtk_button_new_from_stock("gtk-save");
	gtk_widget_set_name(button_dc_Save, "button_dc_Save");
	gtk_widget_show(button_dc_Save);
	gtk_box_pack_start(GTK_BOX(hbutton_box_dc_buttonRow), button_dc_Save, FALSE, FALSE, 0);
	gtk_widget_add_accelerator(button_dc_Save, "activate", accel_group,
				   GDK_Return, (GdkModifierType)(0), (GtkAccelFlags)(0));
	gtk_widget_add_accelerator(button_dc_Save, "activate", accel_group,
				   GDK_KP_Enter, (GdkModifierType)(0), (GtkAccelFlags)(0));
	AddButtonCallback_Clicked(button_dc_Save, on_button_dc_Save_clicked);
	GLADE_HOOKUP_OBJECT(directory_config_window, button_dc_Save, "button_dc_Save");
	
	// Add the accel group.
	gtk_window_add_accel_group(GTK_WINDOW(directory_config_window), accel_group);
	
	return directory_config_window;
}

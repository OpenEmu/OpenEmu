/***************************************************************************
 * Gens: (GTK+) Select CD-ROM Drive Window.                                *
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

#include "select_cdrom_window.h"
#include "select_cdrom_window_callbacks.h"
#include "gens/gens_window.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gtk/gtk.h>

// GENS GTK+ miscellaneous functions
#include "gtk-misc.h"


// Drive speed definitions. 0 == automatic; -1 == end of list
int CD_DriveSpeed[15] = {0, 1, 2, 4, 8, 10, 12, 20, 32, 36, 40, 48, 50, 52, -1};

GtkWidget *select_cdrom_window = NULL;

GtkAccelGroup *accel_group;


/**
 * create_select_cdrom_window(): Create the Select CD-ROM Drive Window.
 * @return Select CD-ROM Drive Window.
 */
GtkWidget* create_select_cdrom_window(void)
{
	GdkPixbuf *select_cdrom_window_icon_pixbuf;
	GtkWidget *vbox_SelCD;
	GtkWidget *frame_drive, *label_frame_drive;
	GtkWidget *hbox_drive, *label_drive, *combo_drive;
	GtkWidget *hbox_speed, *label_speed, *combo_speed;
	GtkWidget *hbutton_box_bottomRow;
	GtkWidget *button_SelCD_Cancel, *button_SelCD_Apply, *button_SelCD_Save;
	int i; char tmp[64];
	
	if (select_cdrom_window)
	{
		// Select CD-ROM Drive Window is already created. Set focus.
		gtk_widget_grab_focus(select_cdrom_window);
		return NULL;
	}
	
	accel_group = gtk_accel_group_new();
	
	// Create the Select CD-ROM Drive Window.
	CREATE_GTK_WINDOW(select_cdrom_window,
			  "select_cdrom_window",
			  "Select CD-ROM Drive",
			  select_cdrom_window_icon_pixbuf, "Gens2.ico");
	
	// Disable resizing.
	gtk_window_set_resizable(GTK_WINDOW(select_cdrom_window), FALSE);
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)select_cdrom_window, "delete_event",
			 G_CALLBACK(on_select_cdrom_window_close), NULL);
	g_signal_connect((gpointer)select_cdrom_window, "destroy_event",
			 G_CALLBACK(on_select_cdrom_window_close), NULL);
	
	// Create the main VBox.
	vbox_SelCD = gtk_vbox_new(FALSE, 5);
	gtk_widget_set_name(vbox_SelCD, "vbox_SelCD");
	gtk_widget_show(vbox_SelCD);
	gtk_container_add(GTK_CONTAINER(select_cdrom_window), vbox_SelCD);
	GLADE_HOOKUP_OBJECT(select_cdrom_window, vbox_SelCD, "vbox_SelCD");
	
	// Add a frame for drive selection.
	frame_drive = gtk_frame_new(NULL);
	gtk_widget_set_name(frame_drive, "frame_drive");
	gtk_container_set_border_width(GTK_CONTAINER(frame_drive), 5);
	gtk_frame_set_shadow_type(GTK_FRAME(frame_drive), GTK_SHADOW_NONE);
	gtk_widget_show(frame_drive);
	gtk_box_pack_start(GTK_BOX(vbox_SelCD), frame_drive, FALSE, FALSE, 0);
	GLADE_HOOKUP_OBJECT(select_cdrom_window, frame_drive, "frame_drive");
	
	// Add a label to the drive selection frame.
	label_frame_drive = gtk_label_new(
		"Typical values are <i>/dev/cdrom</i>, "
		"<i>/dev/sr0</i>, or <i>/dev/hdc</i>"
		);
	gtk_widget_set_name(label_frame_drive, "label_frame_drive");
	gtk_label_set_use_markup(GTK_LABEL(label_frame_drive), TRUE);
	gtk_widget_show(label_frame_drive);
	gtk_frame_set_label_widget(GTK_FRAME(frame_drive), label_frame_drive);
	GLADE_HOOKUP_OBJECT(select_cdrom_window, label_frame_drive, "label_frame_drive");
	
	// HBox for the CD drive label and combo box.
	hbox_drive = gtk_hbox_new(FALSE, 0);
	gtk_widget_set_name(hbox_drive, "hbox_drive");
	gtk_widget_show(hbox_drive);
	gtk_container_add(GTK_CONTAINER(frame_drive), hbox_drive);
	GLADE_HOOKUP_OBJECT(select_cdrom_window, hbox_drive, "hbox_drive");
	
	// CD drive label
	label_drive = gtk_label_new_with_mnemonic("CD-_ROM drive:");
	gtk_widget_set_name(label_drive, "label_drive");
	gtk_widget_show(label_drive);
	gtk_box_pack_start(GTK_BOX(hbox_drive), label_drive, FALSE, FALSE, 0);
	GLADE_HOOKUP_OBJECT(select_cdrom_window, label_drive, "label_drive");
	
	// Add an editable combo box for the CD drive.
	combo_drive = gtk_combo_box_entry_new_text();
	gtk_widget_set_name(combo_drive, "combo_drive");
	gtk_widget_show(combo_drive);
	gtk_box_pack_start(GTK_BOX(hbox_drive), combo_drive, TRUE, TRUE, 0);
	gtk_entry_set_max_length(GTK_ENTRY(GTK_BIN(combo_drive)->child), 63);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label_drive), combo_drive);
	GLADE_HOOKUP_OBJECT(select_cdrom_window, combo_drive, "combo_drive");
	
	// Add an HBox for speed selection.
	hbox_speed = gtk_hbox_new(FALSE, 0);
	gtk_widget_set_name(hbox_speed, "hbox_speed");
	gtk_widget_show(hbox_speed);
	gtk_box_pack_start(GTK_BOX(vbox_SelCD), hbox_speed, FALSE, FALSE, 0);
	GLADE_HOOKUP_OBJECT(select_cdrom_window, hbox_speed, "hbox_speed");
	
	// Speed label
	label_speed = gtk_label_new_with_mnemonic(
		"Manual CD-ROM speed selection.\n"
		"You may need to restart Gens for\n"
		"this setting to take effect."
		);
	gtk_widget_show(label_speed);
	gtk_box_pack_start(GTK_BOX(hbox_speed), label_speed, TRUE, TRUE, 0);
	GLADE_HOOKUP_OBJECT(select_cdrom_window, label_speed, "label_speed");
	
	// Add the dropdown for speed selection.
	combo_speed = gtk_combo_box_new_text();
	gtk_widget_set_name(combo_speed, "combo_speed");
	gtk_widget_set_size_request(combo_speed, 100, -1);
	for (i = 0; i < 14; i++)
	{
		if (CD_DriveSpeed[i] < 0)
			break;
		else if (CD_DriveSpeed[i] == 0)
			strcpy(tmp, "Auto");
		else
			sprintf(tmp, "%dx", CD_DriveSpeed[i]);
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo_speed), tmp);
	}
	gtk_widget_show(combo_speed);
	gtk_box_pack_start(GTK_BOX(hbox_speed), combo_speed, FALSE, FALSE, 0);
	GLADE_HOOKUP_OBJECT(select_cdrom_window, combo_speed, "combo_speed");
	
	// Create an HButton Box for the buttons on the bottom.
	hbutton_box_bottomRow = gtk_hbutton_box_new();
	gtk_widget_set_name(hbutton_box_bottomRow, "hbutton_box_bottomRow");
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbutton_box_bottomRow), GTK_BUTTONBOX_END);
	gtk_widget_show(hbutton_box_bottomRow);
	gtk_box_pack_start(GTK_BOX(vbox_SelCD), hbutton_box_bottomRow, FALSE, FALSE, 0);
	GLADE_HOOKUP_OBJECT(select_cdrom_window, hbutton_box_bottomRow, "hbutton_box_bottomRow");
	
	// Cancel
	button_SelCD_Cancel = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_widget_set_name(button_SelCD_Cancel, "button_SelCD_Cancel");
	gtk_widget_show(button_SelCD_Cancel);
	gtk_box_pack_start(GTK_BOX(hbutton_box_bottomRow), button_SelCD_Cancel, FALSE, FALSE, 0);
	gtk_widget_add_accelerator(button_SelCD_Cancel, "activate", accel_group,
				   GDK_Escape, (GdkModifierType)(0), (GtkAccelFlags)(0));
	AddButtonCallback_Clicked(button_SelCD_Cancel, on_button_SelCD_Cancel_clicked);
	GLADE_HOOKUP_OBJECT(select_cdrom_window, button_SelCD_Cancel, "button_SelCD_Cancel");
	
	// Apply
	button_SelCD_Apply = gtk_button_new_from_stock(GTK_STOCK_APPLY);
	gtk_widget_set_name(button_SelCD_Apply, "button_SelCD_Apply");
	gtk_widget_show(button_SelCD_Apply);
	gtk_box_pack_start(GTK_BOX(hbutton_box_bottomRow), button_SelCD_Apply, FALSE, FALSE, 0);
	AddButtonCallback_Clicked(button_SelCD_Apply, on_button_SelCD_Apply_clicked);
	GLADE_HOOKUP_OBJECT(select_cdrom_window, button_SelCD_Apply, "button_SelCD_Apply");
	
	// Save
	button_SelCD_Save = gtk_button_new_from_stock(GTK_STOCK_SAVE);
	gtk_widget_set_name(button_SelCD_Save, "button_SelCD_Save");
	gtk_widget_show(button_SelCD_Save);
	gtk_box_pack_start(GTK_BOX(hbutton_box_bottomRow), button_SelCD_Save, FALSE, FALSE, 0);
	gtk_widget_add_accelerator(button_SelCD_Save, "activate", accel_group,
				   GDK_Return, (GdkModifierType)(0), (GtkAccelFlags)(0));
	gtk_widget_add_accelerator(button_SelCD_Save, "activate", accel_group,
				   GDK_KP_Enter, (GdkModifierType)(0), (GtkAccelFlags)(0));
	AddButtonCallback_Clicked(button_SelCD_Save, on_button_SelCD_Save_clicked);
	GLADE_HOOKUP_OBJECT(select_cdrom_window, button_SelCD_Save, "button_SelCD_Save");
	
	// Add the accel group.
	gtk_window_add_accel_group(GTK_WINDOW(select_cdrom_window), accel_group);
	
	return select_cdrom_window;
}

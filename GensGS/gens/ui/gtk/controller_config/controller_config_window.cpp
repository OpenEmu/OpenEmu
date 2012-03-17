/***************************************************************************
 * Gens: (GTK+) Controller Configuration Window.                           *
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

#include "controller_config_window.hpp"
#include "controller_config_window_callbacks.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gtk/gtk.h>

// GENS GTK+ miscellaneous functions
#include "gtk-misc.h"

GtkWidget *controller_config_window = NULL;


// Internal copy of Keys_Def[], which is copied when OK is clicked.
struct KeyMap keyConfig[8];


static void AddControllerVBox(GtkWidget *vbox, int port);


/**
 * create_controller_config_window(): Create the Controller Configuration Window.
 * @return Game Genie Window.
 */
GtkWidget* create_controller_config_window(void)
{
	GdkPixbuf *controller_config_window_icon_pixbuf;
	GtkWidget *frame_port_1, *frame_port_2;
	GtkWidget *frame_note;
	GtkWidget *label_note_heading, *label_note;
	GtkWidget *frame_setting_keys;
	GtkWidget *label_setting_keys, *label_cc_echo;
	GtkWidget *hbutton_box_savecancel;
	GtkWidget *button_cc_Cancel, *button_cc_Apply, *button_cc_Save;
	
	if (controller_config_window)
	{
		// Controller Configuration window is already created. Set focus.
		gtk_widget_grab_focus(controller_config_window);
		return NULL;
	}
	
	// Create the Controller Configuration window.
	CREATE_GTK_WINDOW(controller_config_window,
			  "controller_config_window",
			  "Controller Configuration",
			  controller_config_window_icon_pixbuf, "Gens2.ico");
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)controller_config_window, "delete_event",
			 G_CALLBACK(on_controller_config_window_close), NULL);
	g_signal_connect((gpointer)controller_config_window, "destroy_event",
			 G_CALLBACK(on_controller_config_window_close), NULL);
	
	// Main hbox.
	GtkWidget *hboxMain = gtk_hbox_new(FALSE, 0);
	gtk_widget_set_name(hboxMain, "hboxMain");
	gtk_widget_show(hboxMain);
	GLADE_HOOKUP_OBJECT(controller_config_window, hboxMain, "hboxMain");
	gtk_container_add(GTK_CONTAINER(controller_config_window), hboxMain);
	
	// Controller port vbox.
	GtkWidget *vboxControllerPorts = gtk_vbox_new(FALSE, 0);
	gtk_widget_set_name(vboxControllerPorts, "vboxControllerPorts");
	gtk_widget_show(vboxControllerPorts);
	GLADE_HOOKUP_OBJECT(controller_config_window, vboxControllerPorts, "vboxControllerPorts");
	gtk_box_pack_start(GTK_BOX(hboxMain), vboxControllerPorts, FALSE, FALSE, 0);
	
	// Frame for Port 1.
	frame_port_1 = gtk_frame_new(NULL);
	gtk_widget_set_name(frame_port_1, "frame_port_1");
	gtk_frame_set_shadow_type(GTK_FRAME(frame_port_1), GTK_SHADOW_ETCHED_IN);
	gtk_container_set_border_width(GTK_CONTAINER(frame_port_1), 4);
	gtk_widget_show(frame_port_1);
	GLADE_HOOKUP_OBJECT(controller_config_window, frame_port_1, "frame_port_1");
	gtk_box_pack_start(GTK_BOX(vboxControllerPorts), frame_port_1, TRUE, TRUE, 0);
	
	// Frame for Port 2.
	frame_port_2 = gtk_frame_new(NULL);
	gtk_widget_set_name(frame_port_2, "frame_port_2");
	gtk_frame_set_shadow_type(GTK_FRAME(frame_port_2), GTK_SHADOW_ETCHED_IN);
	gtk_container_set_border_width(GTK_CONTAINER(frame_port_2), 4);
	gtk_widget_show(frame_port_2);
	GLADE_HOOKUP_OBJECT(controller_config_window, frame_port_2, "frame_port_2");
	gtk_box_pack_start(GTK_BOX(vboxControllerPorts), frame_port_2, TRUE, TRUE, 0);
	
	// Add VBoxes for controller input.
	AddControllerVBox(frame_port_1, 1);
	AddControllerVBox(frame_port_2, 2);
	
	// Note VBox.
	GtkWidget *vboxNote = gtk_vbox_new(FALSE, 0);
	gtk_widget_set_name(vboxNote, "vboxNote");
	gtk_widget_show(vboxNote);
	GLADE_HOOKUP_OBJECT(controller_config_window, vboxNote, "vboxNote");
	gtk_box_pack_start(GTK_BOX(hboxMain), vboxNote, FALSE, FALSE, 0);
	
	// Frame for the note about teamplayer.
	frame_note = gtk_frame_new(NULL);
	gtk_widget_set_name(frame_note, "frame_note");
	gtk_frame_set_shadow_type(GTK_FRAME(frame_note), GTK_SHADOW_ETCHED_IN);
	gtk_widget_show(frame_note);
	gtk_container_set_border_width(GTK_CONTAINER(frame_note), 4);
	GLADE_HOOKUP_OBJECT(controller_config_window, frame_note, "frame_note");
	gtk_box_pack_start(GTK_BOX(vboxNote), frame_note, FALSE, FALSE, 0);
	
	// Label with teamplayer note. (Heading)
	label_note_heading = gtk_label_new("<b><i>Note</i></b>");
	gtk_widget_set_name(label_note_heading, "label_note_heading");
	gtk_label_set_use_markup(GTK_LABEL(label_note_heading), TRUE);
	gtk_widget_show(label_note_heading);
	GLADE_HOOKUP_OBJECT(controller_config_window, label_note_heading, "label_note_heading");
	gtk_frame_set_label_widget(GTK_FRAME(frame_note), label_note_heading);
	
	// Label with teamplayer note.
	label_note = gtk_label_new(
		"Players 1B, 1C, and 1D are enabled only if\n"
		"teamplayer is enabled on Port 1.\n\n"
		"Players 2B, 2C, and 2D are enabled only if\n"
		"teamplayer is enabled on Port 2.\n\n"
		"Only a few games support teamplayer (games which\n"
		"have 4 player support), so don't forget to use the\n"
		"\"load config\" and \"save config\" options. :)\n\n"
		"Controller configuration is applied when Save is clicked."
		);
	gtk_widget_set_name(label_note, "label_note");
	gtk_misc_set_padding(GTK_MISC(label_note), 8, 8);
	gtk_widget_show(label_note);
	GLADE_HOOKUP_OBJECT(controller_config_window, label_note, "label_note");
	gtk_container_add(GTK_CONTAINER(frame_note), label_note);
	
	// Setting Keys frame.
	frame_setting_keys = gtk_frame_new(NULL);
	gtk_widget_set_name(frame_setting_keys, "frame_setting_keys");
	gtk_container_set_border_width(GTK_CONTAINER(frame_setting_keys), 4);
	gtk_frame_set_shadow_type(GTK_FRAME(frame_setting_keys), GTK_SHADOW_ETCHED_IN);
	gtk_widget_show(frame_setting_keys);
	GLADE_HOOKUP_OBJECT(controller_config_window, frame_setting_keys, "frame_setting_keys");
	gtk_box_pack_start(GTK_BOX(vboxNote), frame_setting_keys, FALSE, FALSE, 0);
	
	// Label for the Setting Keys frame.
	label_setting_keys = gtk_label_new("<b><i>Setting Keys</i></b>");
	gtk_widget_set_name(label_setting_keys, "label_setting_keys");
	gtk_label_set_use_markup(GTK_LABEL(label_setting_keys), TRUE);
	gtk_widget_show(label_setting_keys);
	GLADE_HOOKUP_OBJECT(controller_config_window, label_setting_keys, "label_setting_keys");
	gtk_frame_set_label_widget(GTK_FRAME(frame_setting_keys), label_setting_keys);
	
	// Label indicating what key needs to be pressed.
	label_cc_echo = gtk_label_new("\n");
	gtk_widget_set_name(label_cc_echo, "label_echo");
	gtk_misc_set_padding(GTK_MISC(label_cc_echo), 8, 8);
	gtk_widget_show(label_cc_echo);
	GLADE_HOOKUP_OBJECT(controller_config_window, label_cc_echo, "label_echo");
	gtk_container_add(GTK_CONTAINER(frame_setting_keys), label_cc_echo);
	
	// HButton Box for the buttons.
	hbutton_box_savecancel = gtk_hbutton_box_new();
	gtk_widget_set_name(hbutton_box_savecancel, "hbutton_box_savecancel");
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbutton_box_savecancel), GTK_BUTTONBOX_END);
	gtk_widget_show(hbutton_box_savecancel);
	GLADE_HOOKUP_OBJECT(controller_config_window, hbutton_box_savecancel, "hbutton_box_savecancel");
	gtk_box_pack_end(GTK_BOX(vboxNote), hbutton_box_savecancel, FALSE, TRUE, 0);
	
	// Cancel button
	button_cc_Cancel = gtk_button_new_from_stock("gtk-cancel");
	gtk_widget_set_name(button_cc_Cancel, "button_cc_Cancel");
	gtk_widget_show(button_cc_Cancel);
	gtk_box_pack_start(GTK_BOX(hbutton_box_savecancel), button_cc_Cancel, FALSE, FALSE, 0);
	AddButtonCallback_Clicked(button_cc_Cancel, on_button_cc_Cancel_clicked);
	GLADE_HOOKUP_OBJECT(controller_config_window, button_cc_Cancel, "button_cc_Cancel");
	
	// Apply button
	button_cc_Apply = gtk_button_new_from_stock("gtk-apply");
	gtk_widget_set_name(button_cc_Apply, "button_cc_Apply");
	gtk_widget_show(button_cc_Apply);
	gtk_box_pack_start(GTK_BOX(hbutton_box_savecancel), button_cc_Apply, FALSE, FALSE, 0);
	AddButtonCallback_Clicked(button_cc_Apply, on_button_cc_Apply_clicked);
	GLADE_HOOKUP_OBJECT(controller_config_window, button_cc_Apply, "button_cc_Apply");
	
	// OK button
	button_cc_Save = gtk_button_new_from_stock("gtk-save");
	gtk_widget_set_name(button_cc_Save, "button_cc_Save");
	gtk_widget_show(button_cc_Save);
	AddButtonCallback_Clicked(button_cc_Save, on_button_cc_Save_clicked);
	gtk_box_pack_start(GTK_BOX(hbutton_box_savecancel), button_cc_Save, FALSE, FALSE, 0);
	GLADE_HOOKUP_OBJECT(controller_config_window, button_cc_Save, "button_cc_Save");
	
	return controller_config_window;
}


/**
 * AddControllerVBox(): Add a VBox containing controller configuration options.
 * @param frame Frame to add the VBox to.
 * @param port Port number (1 or 2).
 */
static void AddControllerVBox(GtkWidget *frame, int port)
{
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *label_port;
	GtkWidget *check_teamplayer;
	GtkWidget *table_players;
	GtkWidget *label_player;
	GtkWidget *combobox_padtype;
	GtkWidget *button_configure;
	char tmp[128];
	char player[4];
	int i, j, callbackPort;
	
	// TODO: Move this somewhere else?
	const char* PadTypes[2] =
	{
		"3 buttons",
		"6 buttons",
	};
	
	if (!frame || (port != 1 && port != 2))
		return;
	
	// VBox for this controller
	vbox = gtk_vbox_new(FALSE, 0);
	sprintf(tmp, "vbox_port_%d", port);
	gtk_widget_set_name(vbox, tmp);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	
	// Create the hbox.
	hbox = gtk_hbox_new(FALSE, 5);
	sprintf(tmp, "hbox_controller_%d", port);
	gtk_widget_set_name(hbox, tmp);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
	GLADE_HOOKUP_OBJECT(controller_config_window, hbox, tmp);
	
	// Port label
	sprintf(tmp, "<b><i>Port %d</i></b>", port);
	label_port = gtk_label_new(tmp);
	sprintf(tmp, "label_port_%d", port);
	gtk_widget_set_name(label_port, tmp);
	gtk_label_set_use_markup(GTK_LABEL(label_port), TRUE);
	gtk_widget_show(label_port);
	gtk_frame_set_label_widget(GTK_FRAME(frame), label_port);
	
	// Alignment object for the checkbox.
	GtkWidget *alignCheckTeamplayer = gtk_alignment_new(0.0f, 0.0f, 1.0f, 1.0f);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignCheckTeamplayer), 0, 8, 0, 0);
	gtk_widget_show(alignCheckTeamplayer);
	gtk_box_pack_start(GTK_BOX(hbox), alignCheckTeamplayer, FALSE, FALSE, 0);
	sprintf(tmp, "alignCheckTeamplayer_%d", port);
	GLADE_HOOKUP_OBJECT(controller_config_window, alignCheckTeamplayer, tmp);
	
	// Checkbox for enabling teamplayer.
	check_teamplayer = gtk_check_button_new_with_label("Use Teamplayer");
	gtk_widget_set_name(check_teamplayer, tmp);
	gtk_widget_show(check_teamplayer);
	gtk_container_add(GTK_CONTAINER(alignCheckTeamplayer), check_teamplayer);
	sprintf(tmp, "check_teamplayer_%d", port);
	GLADE_HOOKUP_OBJECT(controller_config_window, check_teamplayer, tmp);
	g_signal_connect((gpointer)check_teamplayer, "clicked",
			 G_CALLBACK(on_check_cc_Teamplayer_clicked), GINT_TO_POINTER(port));
	
	// Table for the player controls.
	table_players = gtk_table_new(4, 3, FALSE);
	sprintf(tmp, "table_players_%d", port);
	gtk_widget_set_name(table_players, tmp);
	gtk_container_set_border_width(GTK_CONTAINER(table_players), 0);
	gtk_table_set_col_spacings(GTK_TABLE(table_players), 12);
	gtk_widget_show(table_players);
	gtk_box_pack_start(GTK_BOX(vbox), table_players, TRUE, TRUE, 0);
	GLADE_HOOKUP_OBJECT(controller_config_window, table_players, tmp);
	
	// Player inputs
	for (i = 0; i < 4; i++)
	{
		if (i == 0)
			sprintf(player, "%d", port);
		else
			sprintf(player, "%d%c", port, 'A' + (char)i);
		
		// Player label
		sprintf(tmp, "Player %s", player);
		label_player = gtk_label_new(tmp);
		sprintf(tmp, "label_player_%s", player);
		gtk_widget_set_name(label_player, tmp);
		gtk_misc_set_alignment(GTK_MISC(label_player), 0, 0.5);
		gtk_widget_show(label_player);
		GLADE_HOOKUP_OBJECT(controller_config_window, label_player, tmp);
		gtk_table_attach(GTK_TABLE(table_players), label_player, 0, 1, i, i + 1,
				 (GtkAttachOptions)(GTK_FILL),
				 (GtkAttachOptions)(0), 0, 0);
		
		// Pad type
		combobox_padtype = gtk_combo_box_new_text();
		sprintf(tmp, "combobox_padtype_%s", player);
		gtk_widget_set_name(combobox_padtype, tmp);
		gtk_widget_show(combobox_padtype);
		GLADE_HOOKUP_OBJECT(controller_config_window, combobox_padtype, tmp);
		gtk_table_attach(GTK_TABLE(table_players), combobox_padtype, 1, 2, i, i + 1,
				 (GtkAttachOptions)(GTK_FILL),
				 (GtkAttachOptions)(0), 0, 0);
		
		// Pad type dropdown
		for (j = 0; j < 2; j++)
		{
			gtk_combo_box_append_text(GTK_COMBO_BOX(combobox_padtype), PadTypes[j]);
		}
		
		// "Reconfigure" button
		button_configure = gtk_button_new_with_label("Reconfigure");
		sprintf(tmp, "button_configure_%s", player);
		gtk_widget_set_name(button_configure, tmp);
		gtk_widget_show(button_configure);
		gtk_table_attach(GTK_TABLE(table_players), button_configure, 2, 3, i, i + 1,
				 (GtkAttachOptions)(GTK_FILL),
				 (GtkAttachOptions)(0), 0, 0);
		
		// Determine the port number to use for the callback.
		if (i == 0)
			callbackPort = port - 1;
		else
		{
			if (port == 1)
				callbackPort = i + 1;
			else // if (port == 2)
				callbackPort = i + 4;
		}
		g_signal_connect(GTK_OBJECT(button_configure), "clicked",
				 G_CALLBACK(on_button_cc_Reconfigure_clicked),
				 GINT_TO_POINTER(callbackPort));
		GLADE_HOOKUP_OBJECT(controller_config_window, button_configure, tmp);
	}
}

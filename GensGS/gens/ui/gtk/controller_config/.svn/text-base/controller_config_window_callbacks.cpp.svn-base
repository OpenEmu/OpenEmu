/***************************************************************************
 * Gens: (GTK+) Controller Configuration Window - Callback Functions.      *
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

#include <stdio.h>
#include <string.h>

#include "controller_config_window.hpp"
#include "controller_config_window_callbacks.hpp"
#include "controller_config_window_misc.hpp"

#include "gtk-misc.h"
#include "emulator/gens.hpp"


/**
 * Window is closed.
 */
gboolean on_controller_config_window_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(widget);
	GENS_UNUSED_PARAMETER(event);
	GENS_UNUSED_PARAMETER(user_data);
	
	gtk_widget_destroy(controller_config_window);
	controller_config_window = NULL;
	return FALSE;
}


/**
 * Teamplayer check box
 */
void on_check_cc_Teamplayer_clicked(GtkButton *button, gpointer user_data)
{
	// Enable/Disable the appropriate controller options depending on the port number.
	int port; char player; gboolean thisChecked;
	char tmpName[64];
	GtkWidget *label_player;
	GtkWidget *combobox_padtype;
	GtkWidget *button_configure;
	
	// Get the port number.
	port = GPOINTER_TO_INT(user_data);
	
	// Get whether this check button is checked or not.
	thisChecked = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
	
	// Loop for players xB through xD.
	for (player = 'B'; player <= 'D'; player++)
	{
		// Label
		sprintf(tmpName, "label_player_%d%c", port, player);
		label_player = lookup_widget(controller_config_window, tmpName);
		gtk_widget_set_sensitive(label_player, thisChecked);
		
		// Pad type
		sprintf(tmpName, "combobox_padtype_%d%c", port, player);
		combobox_padtype = lookup_widget(controller_config_window, tmpName);
		gtk_widget_set_sensitive(combobox_padtype, thisChecked);
		
		// Reconfigure button
		sprintf(tmpName, "button_configure_%d%c", port, player);
		button_configure = lookup_widget(controller_config_window, tmpName);
		gtk_widget_set_sensitive(button_configure, thisChecked);
	}
}


/**
 * "Reconfigure"
 */
void on_button_cc_Reconfigure_clicked(GtkButton *button, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(button);
	
	int player, padtype;
	char playerID[4];
	char objID[64];
	GtkWidget *combobox_padtype;
	
	player = GPOINTER_TO_INT(user_data);
	
	// Determine how many buttons are needed.
	// TODO: Make a separate player numeric-to-string function.
	switch (player)
	{
		case 0:
			strcpy(playerID, "1");
			break;
		case 2:
			strcpy(playerID, "1B");
			break;
		case 3:
			strcpy(playerID, "1C");
			break;
		case 4:
			strcpy(playerID, "1D");
			break;
		case 1:
			strcpy(playerID, "2");
			break;
		case 5:
			strcpy(playerID, "2B");
			break;
		case 6:
			strcpy(playerID, "2C");
			break;
		case 7:
			strcpy(playerID, "2D");
			break;
	}
	
	sprintf(objID, "combobox_padtype_%s", playerID);
	combobox_padtype = lookup_widget(controller_config_window, objID);
	
	// Get the pad type.
	padtype = gtk_combo_box_get_active(GTK_COMBO_BOX(combobox_padtype));
	if (padtype == -1)
		return;
	
	Reconfigure_Input(player, padtype);
}


/**
 * Cancel
 */
void on_button_cc_Cancel_clicked(GtkButton *button, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(button);
	GENS_UNUSED_PARAMETER(user_data);
	
	gtk_widget_destroy(controller_config_window);
	controller_config_window = NULL;
}


/**
 * Apply
 */
void on_button_cc_Apply_clicked(GtkButton *button, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(button);
	GENS_UNUSED_PARAMETER(user_data);
	
	Controller_Config_Save();
}


/**
 * Save
 */
void on_button_cc_Save_clicked(GtkButton *button, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(button);
	GENS_UNUSED_PARAMETER(user_data);
	
	Controller_Config_Save();
	gtk_widget_destroy(controller_config_window);
	controller_config_window = NULL;
}

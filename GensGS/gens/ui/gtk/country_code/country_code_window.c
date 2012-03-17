/***************************************************************************
 * Gens: (GTK+) Country Code Window.                                       *
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

#include "country_code_window.h"
#include "country_code_window_callbacks.h"
#include "gens/gens_window.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gtk/gtk.h>

// GENS GTK+ miscellaneous functions
#include "gtk-misc.h"

// Country codes
// TODO: Move this somewhere else?
const char* Country_Code_String[3] = {"USA", "Japan", "Europe"};

GtkWidget *country_code_window = NULL;

GtkAccelGroup *accel_group;


/**
 * create_country_code_window(): Create the Country Code Window.
 * @return Country Code Window.
 */
GtkWidget* create_country_code_window(void)
{
	GdkPixbuf *country_code_window_icon_pixbuf;
	GtkWidget *vbox_country;
	GtkWidget *frame_country, *label_frame_country;
	GtkWidget *hbox_list, *treeview_country_list;
	GtkWidget *vbox_updown, *button_up, *icon_up, *button_down, *icon_down;
	GtkWidget *hbutton_box_bottomRow;
	GtkWidget *button_Country_Cancel, *button_Country_Apply, *button_Country_Save;
	
	if (country_code_window)
	{
		// Country Code window is already created. Set focus.
		gtk_widget_grab_focus(country_code_window);
		return NULL;
	}
	
	accel_group = gtk_accel_group_new();
	
	// Create the Country Code window.
	CREATE_GTK_WINDOW(country_code_window,
			  "country_code_window",
			  "Country Code Order",
			  country_code_window_icon_pixbuf, "Gens2.ico");
	
	// Disable resizing.
	gtk_window_set_resizable(GTK_WINDOW(country_code_window), FALSE);
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)country_code_window, "delete_event",
			 G_CALLBACK(on_country_code_window_close), NULL);
	g_signal_connect((gpointer)country_code_window, "destroy_event",
			 G_CALLBACK(on_country_code_window_close), NULL);
	
	// Create the main VBox.
	vbox_country = gtk_vbox_new(FALSE, 5);
	gtk_widget_set_name(vbox_country, "vbox_country");
	gtk_widget_show(vbox_country);
	gtk_container_add(GTK_CONTAINER(country_code_window), vbox_country);
	GLADE_HOOKUP_OBJECT(country_code_window, vbox_country, "vbox_country");
	
	// Add a frame for country code selection.
	frame_country = gtk_frame_new(NULL);
	gtk_widget_set_name(frame_country, "frame_country");
	gtk_container_set_border_width(GTK_CONTAINER(frame_country), 5);
	gtk_frame_set_shadow_type(GTK_FRAME(frame_country), GTK_SHADOW_NONE);
	gtk_widget_show(frame_country);
	gtk_box_pack_start(GTK_BOX(vbox_country), frame_country, FALSE, FALSE, 0);
	GLADE_HOOKUP_OBJECT(country_code_window, frame_country, "frame_country");
	
	// Add a label to the country code selection frame.
	label_frame_country = gtk_label_new("<b><i>Country Code Order</i></b>");
	gtk_widget_set_name(label_frame_country, "label_frame_country");
	gtk_label_set_use_markup(GTK_LABEL(label_frame_country), TRUE);
	gtk_widget_show(label_frame_country);
	gtk_frame_set_label_widget(GTK_FRAME(frame_country), label_frame_country);
	GLADE_HOOKUP_OBJECT(country_code_window, label_frame_country, "label_frame_country");
	
	// HBox for the country listview and up/down buttons.
	hbox_list = gtk_hbox_new(FALSE, 0);
	gtk_widget_set_name(hbox_list, "hbox_list");
	gtk_widget_show(hbox_list);
	gtk_container_add(GTK_CONTAINER(frame_country), hbox_list);
	GLADE_HOOKUP_OBJECT(country_code_window, hbox_list, "hbox_list");
	
	// Tree view containing the country codes
	treeview_country_list = gtk_tree_view_new();
	gtk_widget_set_name(treeview_country_list, "treeview_country_list");
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(treeview_country_list), TRUE);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(treeview_country_list), FALSE);
	gtk_widget_show(treeview_country_list);
	gtk_box_pack_start(GTK_BOX(hbox_list), treeview_country_list, TRUE, TRUE, 0);
	GLADE_HOOKUP_OBJECT(country_code_window, treeview_country_list, "treeview_country_list");
	
	// VBox for the up/down buttons.
	vbox_updown = gtk_vbox_new(TRUE, 0);
	gtk_widget_set_name(vbox_updown, "vbox_updown");
	gtk_widget_show(vbox_updown);
	gtk_box_pack_start(GTK_BOX(hbox_list), vbox_updown, FALSE, FALSE, 0);
	GLADE_HOOKUP_OBJECT(country_code_window, vbox_updown, "vbox_updown");
	
	// Up button
	button_up = gtk_button_new();
	gtk_widget_set_name(button_up, "button_up");
	gtk_widget_show(button_up);
	gtk_box_pack_start(GTK_BOX(vbox_updown), button_up, TRUE, TRUE, 0);
	AddButtonCallback_Clicked(button_up, on_button_Country_up_clicked);
	GLADE_HOOKUP_OBJECT(country_code_window, button_up, "button_up");
	
	// Up icon
	icon_up = gtk_image_new_from_stock(GTK_STOCK_GO_UP, GTK_ICON_SIZE_BUTTON);
	gtk_widget_set_name(icon_up, "icon_up");
	gtk_widget_show(icon_up);
	gtk_button_set_image(GTK_BUTTON(button_up), icon_up);
	GLADE_HOOKUP_OBJECT(country_code_window, icon_up, "icon_up");
	
	// Down button
	button_down = gtk_button_new();
	gtk_widget_set_name(button_down, "button_down");
	gtk_widget_show(button_down);
	gtk_box_pack_start(GTK_BOX(vbox_updown), button_down, TRUE, TRUE, 0);
	AddButtonCallback_Clicked(button_down, on_button_Country_down_clicked);
	GLADE_HOOKUP_OBJECT(country_code_window, button_down, "button_down");
	
	// Down icon
	icon_down = gtk_image_new_from_stock(GTK_STOCK_GO_DOWN, GTK_ICON_SIZE_BUTTON);
	gtk_widget_set_name(icon_down, "icon_down");
	gtk_widget_show(icon_down);
	gtk_button_set_image(GTK_BUTTON(button_down), icon_down);
	GLADE_HOOKUP_OBJECT(country_code_window, icon_down, "icon_down");
	
	// Create an HButton Box for the buttons on the bottom.
	hbutton_box_bottomRow = gtk_hbutton_box_new();
	gtk_widget_set_name(hbutton_box_bottomRow, "hbutton_box_bottomRow");
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbutton_box_bottomRow), GTK_BUTTONBOX_END);
	gtk_widget_show(hbutton_box_bottomRow);
	gtk_box_pack_start(GTK_BOX(vbox_country), hbutton_box_bottomRow, FALSE, FALSE, 0);
	GLADE_HOOKUP_OBJECT(country_code_window, hbutton_box_bottomRow, "hbutton_box_bottomRow");
	
	// Cancel
	button_Country_Cancel = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_widget_set_name(button_Country_Cancel, "button_Country_Cancel");
	gtk_widget_show(button_Country_Cancel);
	gtk_box_pack_start(GTK_BOX(hbutton_box_bottomRow), button_Country_Cancel, FALSE, FALSE, 0);
	gtk_widget_add_accelerator(button_Country_Cancel, "activate", accel_group,
				   GDK_Escape, (GdkModifierType)(0), (GtkAccelFlags)(0));
	AddButtonCallback_Clicked(button_Country_Cancel, on_button_Country_Cancel_clicked);
	GLADE_HOOKUP_OBJECT(country_code_window, button_Country_Cancel, "button_Country_Cancel");
	
	// Apply
	button_Country_Apply = gtk_button_new_from_stock(GTK_STOCK_APPLY);
	gtk_widget_set_name(button_Country_Apply, "button_Country_Apply");
	gtk_widget_show(button_Country_Apply);
	gtk_box_pack_start(GTK_BOX(hbutton_box_bottomRow), button_Country_Apply, FALSE, FALSE, 0);
	AddButtonCallback_Clicked(button_Country_Apply, on_button_Country_Apply_clicked);
	GLADE_HOOKUP_OBJECT(country_code_window, button_Country_Apply, "button_Country_Apply");
	
	// Save
	button_Country_Save = gtk_button_new_from_stock(GTK_STOCK_SAVE);
	gtk_widget_set_name(button_Country_Save, "button_Country_Save");
	gtk_widget_show(button_Country_Save);
	gtk_box_pack_start(GTK_BOX(hbutton_box_bottomRow), button_Country_Save, FALSE, FALSE, 0);
	gtk_widget_add_accelerator(button_Country_Save, "activate", accel_group,
				   GDK_Return, (GdkModifierType)(0), (GtkAccelFlags)(0));
	gtk_widget_add_accelerator(button_Country_Save, "activate", accel_group,
				   GDK_KP_Enter, (GdkModifierType)(0), (GtkAccelFlags)(0));
	AddButtonCallback_Clicked(button_Country_Save, on_button_Country_Save_clicked);
	GLADE_HOOKUP_OBJECT(country_code_window, button_Country_Save, "button_Country_Save");
	
	// Add the accel group.
	gtk_window_add_accel_group(GTK_WINDOW(country_code_window), accel_group);
	
	return country_code_window;
}

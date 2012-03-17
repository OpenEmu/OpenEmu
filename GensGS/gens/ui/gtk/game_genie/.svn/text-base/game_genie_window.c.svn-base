/***************************************************************************
 * Gens: (GTK+) Game Genie Window.                                         *
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

#include "game_genie_window.h"
#include "game_genie_window_callbacks.h"
#include "gens/gens_window.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gtk/gtk.h>

// GENS GTK+ miscellaneous functions
#include "gtk-misc.h"

#include "emulator/gens.hpp"


// Macro to add a stock icon to a GtkButton.
#define Button_AddStockIcon(ButtonWidget, ButtonWidgetName, IconWidget, IconName)		\
{												\
	IconWidget = gtk_image_new_from_stock(IconName, GTK_ICON_SIZE_BUTTON);			\
	gtk_widget_set_name(IconWidget, ButtonWidgetName "_Icon");				\
	gtk_widget_show(IconWidget);								\
	gtk_button_set_image(GTK_BUTTON(ButtonWidget), IconWidget);				\
	GLADE_HOOKUP_OBJECT(game_genie_window, IconWidget, ButtonWidgetName "_Icon");		\
}


GtkWidget *game_genie_window = NULL;

GtkAccelGroup *accel_group;


/**
 * create_game_genie_window(): Create the Game Genie Window.
 * @return Game Genie Window.
 */
GtkWidget* create_game_genie_window(void)
{
	GdkPixbuf *game_genie_window_icon_pixbuf;
	GtkWidget *vbox_gg;
	GtkWidget *frame_gg;
	GtkWidget *label_gg_header;
	GtkWidget *label_gg_description;
	
	// Table layout
	GtkWidget *vbox2_gg;
	GtkWidget *table_gg;
	
	// Code / Name
	GtkWidget *label_gg_code;	GtkWidget *entry_gg_code;
	GtkWidget *label_gg_name;	GtkWidget *entry_gg_name;
	
	// Buttons
	GtkWidget *button_gg_addCode;	GtkWidget *button_gg_addCode_Icon;
	GtkWidget *hbutton_box_gg_buttonRow;
	GtkWidget *button_gg_delCode;
	GtkWidget *button_gg_deactAllCodes;	GtkWidget *button_gg_deactAllCodes_Icon;
	GtkWidget *button_gg_Cancel;
	GtkWidget *button_gg_Apply;
	GtkWidget *button_gg_Save;
	
	// List of codes
	GtkWidget *hbox_gg_list;
	GtkWidget *scroll_gg_list;
	GtkWidget *treeview_gg_list;
	
	// Strings
	const char* strInfoTitle = "<b><i>Information about Game Genie / Patch codes</i></b>";
	const char* strInfo =
			"Both Game Genie codes and Patch codes are supported.\n"
			"Check the box next to the code to activate it.\n"
			"Syntax for Game Genie codes: XXXX-YYYY\n"
			"Syntax for Patch codes: AAAAAA-DDDD (address-data)";
	
	if (game_genie_window)
	{
		// Game Genie window is already created. Set focus.
		gtk_widget_grab_focus(game_genie_window);
		return NULL;
	}
	
	accel_group = gtk_accel_group_new();
	
	// Create the Game Genie window.
	CREATE_GTK_WINDOW(game_genie_window,
			  "game_genie_window",
			  "Game Genie",
			  game_genie_window_icon_pixbuf, "Gens2.ico");
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)game_genie_window, "delete_event",
			 G_CALLBACK(on_game_genie_window_close), NULL);
	g_signal_connect((gpointer)game_genie_window, "destroy_event",
			 G_CALLBACK(on_game_genie_window_close), NULL);
	
	// Layout objects.
	vbox_gg = gtk_vbox_new(FALSE, 5);
	gtk_widget_set_name(vbox_gg, "vbox_gg");
	gtk_widget_show(vbox_gg);
	gtk_container_add(GTK_CONTAINER(game_genie_window), vbox_gg);
	GLADE_HOOKUP_OBJECT(game_genie_window, vbox_gg, "vbox_gg");
	
	frame_gg = gtk_frame_new(NULL);
	gtk_widget_set_name(frame_gg, "frame_gg");
	gtk_widget_show(frame_gg);
	gtk_box_pack_start(GTK_BOX(vbox_gg), frame_gg, FALSE, TRUE, 0);
	gtk_frame_set_shadow_type(GTK_FRAME(frame_gg), GTK_SHADOW_NONE);
	GLADE_HOOKUP_OBJECT(game_genie_window, frame_gg, "frame_gg");
	
	// Header label
	label_gg_header = gtk_label_new(strInfoTitle);
	gtk_widget_set_name(label_gg_header, "label_gg_header");
	gtk_label_set_use_markup(GTK_LABEL(label_gg_header), TRUE);
	gtk_widget_show(label_gg_header);
	gtk_frame_set_label_widget(GTK_FRAME(frame_gg), label_gg_header);
	GLADE_HOOKUP_OBJECT(game_genie_window, label_gg_header, "label_gg_header");
	
	// Description label
	label_gg_description = gtk_label_new(strInfo);
	gtk_widget_set_name(label_gg_description, "label_gg_description");
	gtk_label_set_use_markup(GTK_LABEL(label_gg_description), TRUE);
	gtk_widget_show(label_gg_description);
	gtk_container_add(GTK_CONTAINER(frame_gg), label_gg_description);
	gtk_misc_set_alignment(GTK_MISC(label_gg_description), 0.02, 0);
	GLADE_HOOKUP_OBJECT(game_genie_window, label_gg_description, "label_gg_description");
	
	// vbox for table layout
	vbox2_gg = gtk_vbox_new(FALSE, 0);
	gtk_widget_set_name(vbox2_gg, "vbox2_gg");
	gtk_widget_show(vbox2_gg);
	gtk_box_pack_start(GTK_BOX(vbox_gg), vbox2_gg, FALSE, TRUE, 0);
	GLADE_HOOKUP_OBJECT(game_genie_window, vbox2_gg, "vbox2_gg");
	
	// Table layout
	table_gg = gtk_table_new(2, 3, FALSE);
	gtk_widget_set_name(table_gg, "table_gg");
	gtk_widget_show(table_gg);
	gtk_table_set_col_spacings(GTK_TABLE(table_gg), 10);
	gtk_box_pack_start(GTK_BOX(vbox2_gg), table_gg, FALSE, TRUE, 0);
	GLADE_HOOKUP_OBJECT(game_genie_window, table_gg, "table_gg");
	
	// Code label and entry widgets
	label_gg_code = gtk_label_new("Code");
	gtk_widget_set_name(label_gg_code, "label_gg_code");
	gtk_misc_set_alignment(GTK_MISC(label_gg_code), 0, 0.5);
	gtk_widget_show(label_gg_code);
	gtk_table_attach(GTK_TABLE(table_gg), label_gg_code, 0, 1, 0, 1,
			 (GtkAttachOptions)(GTK_FILL),
			 (GtkAttachOptions)(0), 0, 0);
	GLADE_HOOKUP_OBJECT(game_genie_window, label_gg_code, "label_gg_code");
	
	entry_gg_code = gtk_entry_new();
	gtk_widget_set_name(entry_gg_code, "entry_gg_code");
	gtk_entry_set_max_length(GTK_ENTRY(entry_gg_code), 11);
	gtk_widget_show(entry_gg_code);
	gtk_table_attach(GTK_TABLE(table_gg), entry_gg_code, 1, 2, 0, 1,
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			 (GtkAttachOptions)(0), 0, 0);
	GLADE_HOOKUP_OBJECT(game_genie_window, entry_gg_code, "entry_gg_code");
	g_signal_connect((gpointer)entry_gg_code, "key-press-event",
			 G_CALLBACK(on_entry_gg_keypress), NULL);
	
	// Name label and entry widgets
	label_gg_name = gtk_label_new("Name");
	gtk_widget_set_name(label_gg_name, "label_gg_name");
	gtk_misc_set_alignment(GTK_MISC(label_gg_name), 0, 0.5);
	gtk_widget_show(label_gg_name);
	gtk_table_attach(GTK_TABLE(table_gg), label_gg_name, 0, 1, 1, 2,
			 (GtkAttachOptions)(GTK_FILL),
			 (GtkAttachOptions)(0), 0, 0);
	GLADE_HOOKUP_OBJECT(game_genie_window, label_gg_name, "label_gg_name");
	
	entry_gg_name = gtk_entry_new();
	gtk_widget_set_name(entry_gg_name, "entry_gg_name");
	gtk_entry_set_max_length(GTK_ENTRY(entry_gg_name), 239);
	gtk_widget_show(entry_gg_name);
	gtk_table_attach(GTK_TABLE(table_gg), entry_gg_name, 1, 2, 1, 2,
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			 (GtkAttachOptions)(0), 0, 0);
	GLADE_HOOKUP_OBJECT(game_genie_window, entry_gg_name, "entry_gg_name");
	g_signal_connect((gpointer)entry_gg_name, "key-press-event",
			 G_CALLBACK(on_entry_gg_keypress), NULL);
	
	// Add Code button
	button_gg_addCode = gtk_button_new_with_mnemonic("_Add Code");
	gtk_widget_set_name(button_gg_addCode, "button_gg_addCode");
	gtk_widget_show(button_gg_addCode);
	gtk_table_attach(GTK_TABLE(table_gg), button_gg_addCode, 2, 3, 0, 1,
			 (GtkAttachOptions)(GTK_FILL),
			 (GtkAttachOptions)(0), 0, 0);
	GLADE_HOOKUP_OBJECT(game_genie_window, button_gg_addCode, "button_gg_addCode");
	Button_AddStockIcon(button_gg_addCode, "button_gg_addCode", button_gg_addCode_Icon, "gtk-add");
	AddButtonCallback_Clicked(button_gg_addCode, on_button_gg_addCode_clicked);
	
	// Set the focus chain for the entry boxes.
	GList *lFocusChain = NULL;
	lFocusChain = g_list_append(lFocusChain, entry_gg_code);
	lFocusChain = g_list_append(lFocusChain, entry_gg_name);
	lFocusChain = g_list_append(lFocusChain, button_gg_addCode);
	lFocusChain = g_list_first(lFocusChain);
	gtk_container_set_focus_chain(GTK_CONTAINER(table_gg), lFocusChain);
	g_list_free(lFocusChain);
	
	// HBox for the code list
	hbox_gg_list = gtk_hbox_new(FALSE, 0);
	gtk_widget_set_name(hbox_gg_list, "hbox_gg_list");
	gtk_widget_show(hbox_gg_list);
	gtk_box_pack_start(GTK_BOX(vbox_gg), hbox_gg_list, TRUE, TRUE, 0);
	GLADE_HOOKUP_OBJECT(game_genie_window, hbox_gg_list, "hbox_gg_list");
	
	// Scrolled Window for the code list
	scroll_gg_list = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_set_name(scroll_gg_list, "scroll_gg_list");
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll_gg_list), GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_gg_list),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_widget_show(scroll_gg_list);
	gtk_box_pack_start(GTK_BOX(hbox_gg_list), scroll_gg_list, TRUE, TRUE, 0);
	GLADE_HOOKUP_OBJECT(game_genie_window, scroll_gg_list, "scroll_gg_list");
	
	// Tree view containing the Game Genie codes
	treeview_gg_list = gtk_tree_view_new();
	gtk_widget_set_name(treeview_gg_list, "treeview_gg_list");
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(treeview_gg_list), TRUE);
	gtk_widget_set_size_request(treeview_gg_list, -1, 160);
	gtk_widget_show(treeview_gg_list);
	gtk_container_add(GTK_CONTAINER(scroll_gg_list), treeview_gg_list);
	GLADE_HOOKUP_OBJECT(game_genie_window, treeview_gg_list, "treeview_gg_list");
	
	// HButton Box for the row of buttons on the bottom of the window
	hbutton_box_gg_buttonRow = gtk_hbutton_box_new();
	gtk_widget_set_name(hbutton_box_gg_buttonRow, "hbutton_box_gg_buttonRow");
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbutton_box_gg_buttonRow), GTK_BUTTONBOX_END);
	gtk_widget_show(hbutton_box_gg_buttonRow);
	gtk_box_pack_start(GTK_BOX(vbox_gg), hbutton_box_gg_buttonRow, FALSE, FALSE, 0);
	GLADE_HOOKUP_OBJECT(game_genie_window, hbutton_box_gg_buttonRow, "hbutton_box_gg_buttonRow");
	
	// Buttons on the bottom of the window.
	
	// Delete
	button_gg_delCode = gtk_button_new_from_stock("gtk-delete");
	gtk_widget_set_name(button_gg_delCode, "button_gg_delCode");
	gtk_widget_show(button_gg_delCode);
	gtk_box_pack_start(GTK_BOX(hbutton_box_gg_buttonRow), button_gg_delCode, FALSE, FALSE, 0);
	AddButtonCallback_Clicked(button_gg_delCode, on_button_gg_delCode_clicked);
	GLADE_HOOKUP_OBJECT(game_genie_window, button_gg_delCode, "button_gg_delCode");
	
	// Deactivate All Codes
	button_gg_deactAllCodes = gtk_button_new_with_mnemonic("Deac_tivate All");
	gtk_widget_set_name(button_gg_deactAllCodes, "button_gg_deactAllCodes");
	gtk_widget_show(button_gg_deactAllCodes);
	gtk_box_pack_start(GTK_BOX(hbutton_box_gg_buttonRow), button_gg_deactAllCodes, FALSE, FALSE, 0);
	Button_AddStockIcon(button_gg_deactAllCodes, "button_gg_deactAllCodes", button_gg_deactAllCodes_Icon, "gtk-remove");
	AddButtonCallback_Clicked(button_gg_deactAllCodes, on_button_gg_deactAllCodes_clicked);
	GLADE_HOOKUP_OBJECT(game_genie_window, button_gg_deactAllCodes, "button_gg_deactAllCodes");
	
	// Cancel
	button_gg_Cancel = gtk_button_new_from_stock("gtk-cancel");
	gtk_widget_set_name(button_gg_Cancel, "button_gg_Cancel");
	gtk_widget_show(button_gg_Cancel);
	gtk_box_pack_start(GTK_BOX(hbutton_box_gg_buttonRow), button_gg_Cancel, FALSE, FALSE, 0);
	gtk_widget_add_accelerator(button_gg_Cancel, "activate", accel_group,
				   GDK_Escape, (GdkModifierType)(0), (GtkAccelFlags)(0));
	AddButtonCallback_Clicked(button_gg_Cancel, on_button_gg_Cancel_clicked);
	GLADE_HOOKUP_OBJECT(game_genie_window, button_gg_Cancel, "button_gg_Cancel");
	
	// Apply
	button_gg_Apply = gtk_button_new_from_stock("gtk-apply");
	gtk_widget_set_name(button_gg_Apply, "button_gg_Apply");
	gtk_widget_show(button_gg_Apply);
	gtk_box_pack_start(GTK_BOX(hbutton_box_gg_buttonRow), button_gg_Apply, FALSE, FALSE, 0);
	AddButtonCallback_Clicked(button_gg_Apply, on_button_gg_Apply_clicked);
	GLADE_HOOKUP_OBJECT(game_genie_window, button_gg_Apply, "button_gg_Apply");
	
	// Save
	button_gg_Save = gtk_button_new_from_stock("gtk-save");
	gtk_widget_set_name(button_gg_Save, "button_gg_Save");
	gtk_widget_show(button_gg_Save);
	gtk_box_pack_start(GTK_BOX(hbutton_box_gg_buttonRow), button_gg_Save, FALSE, FALSE, 0);
	AddButtonCallback_Clicked(button_gg_Save, on_button_gg_Save_clicked);
	GLADE_HOOKUP_OBJECT(game_genie_window, button_gg_Save, "button_gg_Save");
	
	// Add the accel group.
	gtk_window_add_accel_group(GTK_WINDOW(game_genie_window), accel_group);
	
	return game_genie_window;
}

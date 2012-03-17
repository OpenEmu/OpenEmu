/***************************************************************************
 * Gens: (GTK+) Game Genie Window - Callback Functions.                    *
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

#include "game_genie_window.h"
#include "game_genie_window_callbacks.h"
#include "game_genie_window_misc.h"

#include "gtk-misc.h"
#include "emulator/gens.hpp"


/**
 * Window is closed.
 */
gboolean on_game_genie_window_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(widget);
	GENS_UNUSED_PARAMETER(event);
	GENS_UNUSED_PARAMETER(user_data);
	
	gtk_widget_destroy(game_genie_window);
	game_genie_window = NULL;
	return FALSE;
}


/**
 * Generic AddCode signal handler.
 */
void AddCode_Signal(GtkWidget *widget, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(user_data);
	
	GtkWidget *entry_gg_code, *entry_gg_name;
	gchar *code, *name;
	int length;
	
	// Look up the entry widgets.
	entry_gg_code = lookup_widget(widget, "entry_gg_code");
	entry_gg_name = lookup_widget(widget, "entry_gg_name");
	
	// Get the text.
	code = strdup(gtk_entry_get_text(GTK_ENTRY(entry_gg_code)));
	name = strdup(gtk_entry_get_text(GTK_ENTRY(entry_gg_name)));
	
	// Get the length of the code.
	// Game Genie codes are 9 characters long. ("XXXX-YYYY")
	// Patch codes are 11 characters long. ("AAAAAA-DDDD")
	// TODO: Better verification of valid codes.
	// TODO: Automatically add a hyphen for 8-char Game Genie codes and a colon for 10-char Patch codes.
	length = strlen(code);
	if (length == 9 || length == 11)
		GG_AddCode(NULL, name, code, 0);
	
	g_free(code);
	g_free(name);
	
	// Clear the contents of the entry widgets.
	gtk_entry_set_text(GTK_ENTRY(entry_gg_code), "");
	gtk_entry_set_text(GTK_ENTRY(entry_gg_name), "");
	
	// Set the focus to the code entry widget.
	gtk_widget_grab_focus(entry_gg_code);
}


/**
 * Add Code button
 */
void on_button_gg_addCode_clicked(GtkButton *button, gpointer user_data)
{
	AddCode_Signal(GTK_WIDGET(button), user_data);
}


/**
 * Key press event in either the code entry widget or the name entry widget.
 */
gboolean on_entry_gg_keypress(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	if (event->keyval == GDK_Return || event->keyval == GDK_KP_Enter)
	{
		AddCode_Signal(widget, user_data);
		return TRUE;
	}
	return FALSE;
}


/**
 * Delete Code
 */
void on_button_gg_delCode_clicked(GtkButton *button, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(button);
	GENS_UNUSED_PARAMETER(user_data);
	
	GG_DelSelectedCode();
}


/**
 * Deactivate All Codes
 */
void on_button_gg_deactAllCodes_clicked(GtkButton *button, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(button);
	GENS_UNUSED_PARAMETER(user_data);
	
	GG_DeactivateAllCodes();
}


/**
 * Code was toggled.
 */
void on_treeview_gg_list_item_toggled(GtkCellRendererToggle *cell_renderer,
				      gchar *path, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(cell_renderer);
	
	// Toggle the state of this item.
	GtkTreeIter iter;
	gboolean curState;
	
	// Get the iterator pointing to this list item.
	gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(user_data), &iter, path);
	
	// Get the current value.
	gtk_tree_model_get(GTK_TREE_MODEL(user_data), &iter, 0, &curState, -1);
	
	// Set the inverted value.
	gtk_list_store_set(GTK_LIST_STORE(user_data), &iter, 0, !curState, -1);
}


/**
 * Cancel
 */
void on_button_gg_Cancel_clicked(GtkButton *button, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(button);
	GENS_UNUSED_PARAMETER(user_data);
	
	gtk_widget_destroy(game_genie_window);
	game_genie_window = NULL;
}


/**
 * Apply
 */
void on_button_gg_Apply_clicked(GtkButton *button, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(button);
	GENS_UNUSED_PARAMETER(user_data);
	
	GG_SaveCodes();
}


/**
 * Save
 */
void on_button_gg_Save_clicked(GtkButton *button, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(button);
	GENS_UNUSED_PARAMETER(user_data);
	
	GG_SaveCodes();
	gtk_widget_destroy(game_genie_window);
	game_genie_window = NULL;
}

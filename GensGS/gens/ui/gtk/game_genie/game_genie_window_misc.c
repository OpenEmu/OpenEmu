/***************************************************************************
 * Gens: (GTK+) Game Genie Window - Miscellaneous Functions.               *
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
#include "gens/gens_window.hpp"
#include "util/file/ggenie.h"
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/vdp/vdp_io.h"

#include "emulator/g_main.hpp"

#include <gtk/gtk.h>
#include "gtk-misc.h"

// Stores the entries in the TreeView.
GtkListStore *listmodel_gg = NULL;


/**
 * Open_Game_Genie(): Opens the Game Genie window.
 */
void Open_Game_Genie(void)
{
	GtkWidget *gg, *treeview;
	GtkCellRenderer *toggle_renderer, *text_renderer;
	GtkTreeViewColumn *col_enabled, *col_code, *col_name;
	GtkTreeSelection *select;
	int i;
	
	ice = 0;
	
	gg = create_game_genie_window();
	if (!gg)
	{
		// Either an error occurred while creating the Game Genie window,
		// or the Game Genie window is already created.
		return;
	}
	gtk_window_set_transient_for(GTK_WINDOW(gg), GTK_WINDOW(gens_window));
	
	// Populate the TreeView.
	treeview = lookup_widget(gg, "treeview_gg_list");
	select = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
	gtk_tree_selection_set_mode(select, GTK_SELECTION_MULTIPLE);
	
	// Check if the listmodel_gg is already created.
	// If it is, clear it; if not, create a new one.
	if (listmodel_gg)
		gtk_list_store_clear(listmodel_gg);
	else
		listmodel_gg = gtk_list_store_new(3, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING);
	
	// Set the view model of the treeview.
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(listmodel_gg));
	
	// Create the renderer and columns.
	toggle_renderer = gtk_cell_renderer_toggle_new();
	col_enabled = gtk_tree_view_column_new_with_attributes("Enabled", toggle_renderer, "active", 0, NULL);
	text_renderer = gtk_cell_renderer_text_new();
	col_code = gtk_tree_view_column_new_with_attributes("Code", text_renderer, "text", 1, NULL);
	col_name = gtk_tree_view_column_new_with_attributes("Name", text_renderer, "text", 2, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col_enabled);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col_code);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col_name);
	
	// Connect the toggle renderer to the callback.
	g_signal_connect((gpointer)toggle_renderer, "toggled",
			 G_CALLBACK(on_treeview_gg_list_item_toggled), (gpointer)listmodel_gg);
	
	// Go through the list of codes and add them to the treeview.
	for (i = 0; i < 256; i++)
	{
		if (Game_Genie_Codes[i].code[0] == 0)
			continue;
		
		GG_AddCode(treeview, Game_Genie_Codes[i].name, Game_Genie_Codes[i].code, Game_Genie_Codes[i].active);
		
		// If the ROM is loaded, and this code applies to ROM data, apply the code.
		// Or something.
		// TODO: Figure out what this actually does.
		if ((Game_Genie_Codes[i].restore != 0xFFFFFFFF) &&
		    (Game_Genie_Codes[i].addr < Rom_Size) &&
		    (Genesis_Started))
		{
			Rom_Data[Game_Genie_Codes[i].addr] = (unsigned char)(Game_Genie_Codes[i].restore & 0xFF);
			Rom_Data[Game_Genie_Codes[i + 1].addr] = (unsigned char)((Game_Genie_Codes[i].restore & 0xFF00) >> 8);
		}
	}
	
	// Show the Game Genie window.
	gtk_widget_show_all(gg);
}


/**
 * GG_AddCode(): Add a code to the Game Genie treeview.
 * @param treeview GtkTreeView.
 * @param name Name of the code.
 * @param code Actual code.
 * @param enabled If non-zero, the code is enabled.
 */
void GG_AddCode(GtkWidget *treeview, const char *name, const char *code, int enabled)
{
	GtkTreeIter iter;
	char upperCode[16];
	int i;
	
	// Convert the code to uppercase.
	for (i = 0; i < 16; i++)
	{
		if (code[i] >= 'a' && code[i] <= 'z')
			upperCode[i] = code[i] - ('a' - 'A');
		else
			upperCode[i] = code[i];
		
		if (upperCode[i] == 0x00)
			break;
	}
	
	gtk_list_store_append(listmodel_gg, &iter);
	gtk_list_store_set(GTK_LIST_STORE(listmodel_gg), &iter, 0, enabled, 1, upperCode, 2, name, -1);
	
	// TODO: What is this for?
	if (enabled && treeview)
	{
		GtkTreeSelection *select;
		select = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
		gtk_tree_selection_select_iter(select, &iter);
	}
}


/**
 * GG_DelSelectedCode(): Delete the selected code.
 */
void GG_DelSelectedCode(void)
{
	GtkWidget *treeview;
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	gboolean need_check, row_erased, valid;
	
	treeview = lookup_widget(game_genie_window, "treeview_gg_list");
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
	
	// Delete all selected codes.
	need_check = TRUE;
	while (need_check)
	{
		row_erased = FALSE;
		valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(listmodel_gg), &iter);
		while (valid && !row_erased)
		{
			if (gtk_tree_selection_iter_is_selected(selection, &iter))
			{
				gtk_list_store_remove(listmodel_gg, &iter);
				row_erased = TRUE;
			}
			else
				valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(listmodel_gg), &iter);
		}
		if (!valid && !row_erased)
			need_check = FALSE;
	}
}


/**
 * GG_DeactivateAllCodes(): Deactivate all codes.
 */
void GG_DeactivateAllCodes(void)
{
	GtkWidget *treeview;
	GtkTreeIter iter;
	gboolean valid;
	
	treeview = lookup_widget(game_genie_window, "treeview_gg_list");
	
	// Deactivate all codes.
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(listmodel_gg), &iter);
	while (valid)
	{
		gtk_list_store_set(GTK_LIST_STORE(listmodel_gg), &iter, 0, 0, -1);
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(listmodel_gg), &iter);
	}
}


/**
 * GG_SaveCodes(): Save the codes from the GtkTreeView to Game_Genie_Codes[].
 */
void GG_SaveCodes(void)
{
	gboolean valid, enabled;
	GtkTreeIter iter;
	gchar *code, *name;
	int i;
	
	// Reinitialize the Game Genie array.
	Init_GameGenie();
	
	// Copy each item in the listview to the array.
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(listmodel_gg), &iter);
	i = 0;
	while (valid && i < 256)
	{
		gtk_tree_model_get(GTK_TREE_MODEL(listmodel_gg), &iter, 0, &enabled, 1, &code, 2, &name, -1);
		strcpy(Game_Genie_Codes[i].name, name);
		strcpy(Game_Genie_Codes[i].code, code);
		Game_Genie_Codes[i].active = (enabled ? 1 : 0);
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(listmodel_gg), &iter);
		i++;
	}
	
	// Decode and apply Game Genie codes.
	// TODO: Move this somewhere else?
	for (i = 0; i < 256; i++)
	{
		if ((Game_Genie_Codes[i].code[0] != 0) &&
		    (Game_Genie_Codes[i].addr == 0xFFFFFFFF) &&
		    (Game_Genie_Codes[i].data == 0))
		{
			// Decode this entry.
			decode(Game_Genie_Codes[i].code, (struct patch*)(&(Game_Genie_Codes[i].addr)));
			
			if ((Game_Genie_Codes[i].restore = 0xFFFFFFFF) &&
			    (Game_Genie_Codes[i].addr < Rom_Size) &&
			    (Genesis_Started))
			{
				Game_Genie_Codes[i].restore = (unsigned int)(Rom_Data[Game_Genie_Codes[i].addr] & 0xFF) +
						      (unsigned int)((Rom_Data[Game_Genie_Codes[i].addr + 1] & 0xFF) << 8);
			}
		}
	}
}

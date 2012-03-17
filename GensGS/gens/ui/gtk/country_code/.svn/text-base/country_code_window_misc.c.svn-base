/***************************************************************************
 * Gens: (GTK+) Country Code Window - Miscellaneous Functions.             *
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

#include "country_code_window.h"
#include "country_code_window_callbacks.h"
#include "country_code_window_misc.h"
#include "gens/gens_window.hpp"

#include <gtk/gtk.h>
#include "gtk-misc.h"

#include "emulator/g_main.hpp"

// Stores the entries in the TreeView.
GtkListStore *listmodel_country = NULL;


/**
 * Open_Country_Code(): Opens the Country Code window.
 */
void Open_Country_Code(void)
{
	GtkWidget *Country, *treeview;
	GtkCellRenderer *text_renderer;
	GtkTreeViewColumn *col_text;
	GtkTreeIter iter;
	int i;
	
	Country = create_country_code_window();
	if (!Country)
	{
		// Either an error occurred while creating the Country Code window,
		// or the Country Code window is already created.
		return;
	}
	gtk_window_set_transient_for(GTK_WINDOW(Country), GTK_WINDOW(gens_window));
	
	// Set up the country order treeview.
	// Elements in Country_Order[3] can have one of three values:
	// - 0 [USA]
	// - 1 [JAP]
	// - 2 [EUR]
	
	// Make sure the country code order is valid.
	Check_Country_Order();
	
	// Populate the TreeView.
	treeview = lookup_widget(Country, "treeview_country_list");
	
	// Check if the listmodel_country is already created.
	// If it is, clear it; if not, create a new one.
	if (listmodel_country)
		gtk_list_store_clear(listmodel_country);
	else
		listmodel_country = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);	
	
	// Set the view model of the treeview.
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(listmodel_country));
	
	// Create the renderer and the columns.
	text_renderer = gtk_cell_renderer_text_new();
	col_text = gtk_tree_view_column_new_with_attributes("Country", text_renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col_text);
	
	// Add the country codes to the treeview in the appropriate order.
	for (i = 0; i < 3; i++)
	{
		gtk_list_store_append(listmodel_country, &iter);
		gtk_list_store_set(GTK_LIST_STORE(listmodel_country), &iter,
						  0, Country_Code_String[Country_Order[i]],
						  1, Country_Order[i], -1);
	}
	
	// Show the Country Code window.
	gtk_widget_show_all(Country);
}


/**
 * Country_Save(): Save the settings.
 */
void Country_Save(void)
{
	// Save settings.
	gboolean valid;
	GtkTreeIter iter;
	int i, countryID;
	
	// Copy each item in the listview to the array.
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(listmodel_country), &iter);
	i = 0;
	while (valid && i < 3)
	{
		gtk_tree_model_get(GTK_TREE_MODEL(listmodel_country), &iter, 1, &countryID, -1);
		Country_Order[i] = countryID;
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(listmodel_country), &iter);
		i++;
	}
	
	// Validate the country code order.
	Check_Country_Order();
}


/**
 * Country_MoveUp(): Move the selected country up.
 */
void Country_MoveUp(void)
{
	GtkWidget *treeview;
	GtkTreeSelection *selection;
	GtkTreeIter iter, prevIter;
	gboolean notFirst, valid;
	treeview = lookup_widget(country_code_window, "treeview_country_list");
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
	
	// Find the selection and swap it with the item immediately before it.
	notFirst = FALSE;
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(listmodel_country), &iter);
	while (valid)
	{
		if (gtk_tree_selection_iter_is_selected(selection, &iter))
		{
			// Found the selection.
			if (notFirst)
			{
				// Not the first item. Swap it with the previous item.
				gtk_list_store_swap(listmodel_country, &iter, &prevIter);
			}
			break;
		}
		else
		{
			// Not selected. Store this iter as prevIter.
			prevIter = iter;
			// Since this isn't the first item, set notFirst.
			notFirst = TRUE;
			valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(listmodel_country), &iter);
		}
	}
}


/**
 * Country_MoveDown(): Move the selected country down.
 */
void Country_MoveDown(void)
{
	GtkWidget *treeview;
	GtkTreeSelection *selection;
	GtkTreeIter iter, nextIter;
	gboolean valid;
	treeview = lookup_widget(country_code_window, "treeview_country_list");
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
	
	// Find the selection and swap it with the item immediately after it.
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(listmodel_country), &iter);
	while (valid)
	{
		if (gtk_tree_selection_iter_is_selected(selection, &iter))
		{
			// Found the selection. Check if there's another item after it.
			nextIter = iter;
			valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(listmodel_country), &nextIter);
			if (valid)
			{
				// Not the last item. Swap it with the next item.
				gtk_list_store_swap(listmodel_country, &iter, &nextIter);
			}
			break;
		}
		else
		{
			// Not selected.
			valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(listmodel_country), &iter);
		}
	}
}

/***************************************************************************
 * Gens: (GTK+) Zip File Selection Dialog.                                 *
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

#include "zip_select_dialog.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <list>
using std::list;

#include "emulator/gens.hpp"

// Gens GTK+ miscellaneous functions
#include "gtk-misc.h"

#if 0
static GtkWidget *zip_select_dialog;
#endif

static gboolean on_m_lstFiles_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data);

// TODO: Improve this dialog.


/**
 * ZipSelectDialog(): Create the Zip File Selection Dialog.
 * @return Zip File Selection Dialog.
 */
ZipSelectDialog::ZipSelectDialog(GtkWindow *parent)
{
	// Create the Zip File Selection window.
	m_Window = gtk_dialog_new();
	gtk_widget_set_name(m_Window, "zip_select_dialog");
	gtk_container_set_border_width(GTK_CONTAINER(m_Window), 5);
	gtk_window_set_title(GTK_WINDOW(m_Window), "Archive File Selection");
	gtk_window_set_position(GTK_WINDOW(m_Window), GTK_WIN_POS_CENTER);
	gtk_window_set_modal(GTK_WINDOW(m_Window), TRUE);
	gtk_window_set_resizable(GTK_WINDOW(m_Window), FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(m_Window), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator(GTK_DIALOG(m_Window), FALSE);
	gtk_window_set_transient_for(GTK_WINDOW(m_Window), parent);
	
	// Set the window data.
	g_object_set_data(G_OBJECT(m_Window), "ZipSelectDialog", m_Window);
	
	// Make the window a decent size.
	gtk_widget_set_size_request(m_Window, 480, 280);
	
	// Load the Gens icon.
	GdkPixbuf *icon = create_pixbuf("Gens2.ico");
	if (icon)
	{
		gtk_window_set_icon(GTK_WINDOW(m_Window), icon);
		gdk_pixbuf_unref(icon);
	}
	
	// Add a frame for zip file selection.
	GtkWidget *fraZip = gtk_frame_new(NULL);
	gtk_widget_set_name(fraZip, "fraZip");
	gtk_container_set_border_width(GTK_CONTAINER(fraZip), 5);
	gtk_frame_set_shadow_type(GTK_FRAME(fraZip), GTK_SHADOW_NONE);
	gtk_widget_show(fraZip);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(m_Window)->vbox), fraZip);
	g_object_set_data_full(G_OBJECT(m_Window), "fraZip",
			       g_object_ref(fraZip), (GDestroyNotify)g_object_unref);
	
	// Add a label to the zip file selection frame.
	GtkWidget *lblZip = gtk_label_new("This archive contains multiple files.\nSelect which file you want to load.");
	gtk_widget_set_name(lblZip, "lblZip");
	gtk_label_set_justify(GTK_LABEL(lblZip), GTK_JUSTIFY_CENTER);
	gtk_widget_show(lblZip);
	gtk_frame_set_label_widget(GTK_FRAME(fraZip), lblZip);
	gtk_frame_set_label_align(GTK_FRAME(fraZip), 0.5, 0);
	gtk_frame_set_shadow_type(GTK_FRAME(fraZip), GTK_SHADOW_NONE);
	g_object_set_data_full(G_OBJECT(m_Window), "lblZip",
			       g_object_ref(lblZip), (GDestroyNotify)g_object_unref);
	
	// Scrolled Window for the file list
	GtkWidget *scrlFileList = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_set_name(scrlFileList, "scrlFileList");
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrlFileList),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_widget_show(scrlFileList);
	gtk_container_add(GTK_CONTAINER(fraZip), scrlFileList);
	g_object_set_data_full(G_OBJECT(m_Window), "scrlFileList",
			       g_object_ref(scrlFileList), (GDestroyNotify)g_object_unref);
	
	// Tree view containing the files in the archive.
	m_lstFiles = gtk_tree_view_new();
	gtk_widget_set_name(m_lstFiles, "m_lstFiles");
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(m_lstFiles), TRUE);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(m_lstFiles), FALSE);
	gtk_widget_show(m_lstFiles);
	gtk_container_add(GTK_CONTAINER(scrlFileList), m_lstFiles);
	g_signal_connect((gpointer)m_lstFiles, "button_press_event",
			  G_CALLBACK(on_m_lstFiles_button_press), m_Window);
	g_object_set_data_full(G_OBJECT(m_Window), "m_lstFiles",
			       g_object_ref(m_lstFiles), (GDestroyNotify)g_object_unref);
	
	// Create an accelerator group.
	m_AccelTable = gtk_accel_group_new();
	
	// Add the dialog buttons.
	addDialogButtons(m_Window, WndBase::BAlign_Default,
			 WndBase::BUTTON_CANCEL | WndBase::BUTTON_OK, 0,
			 WndBase::BUTTON_ALL);
	
	// Add the accel group.
	gtk_window_add_accel_group(GTK_WINDOW(m_Window), m_AccelTable);
}


ZipSelectDialog::~ZipSelectDialog()
{
	if (m_Window)
		gtk_widget_destroy(GTK_WIDGET(m_Window));
}


static gboolean on_m_lstFiles_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(widget);
	
	if (GTK_IS_DIALOG(user_data) && event->type == GDK_2BUTTON_PRESS)
	{
		// Item was double-clicked in the treeview.
		// Select the current item.
		gtk_dialog_response(GTK_DIALOG(user_data), GTK_RESPONSE_OK);
		return TRUE;
	}
	return FALSE;
}


/**
 * getFile(): Get a file using the Zip Select dialog.
 */
CompressedFile* ZipSelectDialog::getFile(list<CompressedFile>* lst)
{
	if (!lst)
	{
		// NULL list pointer passed. Don't do anything.
		return NULL;
	}
	
	gint dialogResponse;
	GtkTreeSelection *selection;
	gboolean valid;
	list<CompressedFile>::iterator lstIter;
	GtkTreeIter iter;
	CompressedFile *selFile;
	
	// Stores the entries in the TreeView.
	GtkListStore *lstdataFiles = NULL;
	
	// Create a list model.
	lstdataFiles = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);
	
	// Set the view model of the treeview.
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_lstFiles), GTK_TREE_MODEL(lstdataFiles));
	
	// Create the renderer and the columns.
	GtkCellRenderer *rendText = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *colText = gtk_tree_view_column_new_with_attributes("Zip", rendText, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(m_lstFiles), colText);
	
	// Add all files from the CompressedFile list.
	for (lstIter = lst->begin(); lstIter != lst->end(); lstIter++)
	{
		gtk_list_store_append(lstdataFiles, &iter);
		gtk_list_store_set(GTK_LIST_STORE(lstdataFiles), &iter,
				   0, (*lstIter).filename.c_str(), 1, &(*lstIter), -1);
	}
	
	// Select the first item by default.
	GtkTreePath *path;
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_lstFiles));
	path = gtk_tree_path_new_from_string("0");
	gtk_tree_selection_select_path(selection, path);
	gtk_tree_path_free(path);
	
	// Run the dialog.
	dialogResponse = gtk_dialog_run(GTK_DIALOG(m_Window));
	if (dialogResponse != GTK_RESPONSE_OK)
	{
		// No file was selected.
		return NULL;
	}
	
	// Get the selected file.
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_lstFiles));
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(lstdataFiles), &iter);
	while (valid)
	{
		if (gtk_tree_selection_iter_is_selected(selection, &iter))
		{
			// Found the selected file.
			gtk_tree_model_get(GTK_TREE_MODEL(lstdataFiles), &iter, 1, &selFile, -1);
			break;
		}
		else
		{
			// Didn't find the selected file yet.
			valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(lstdataFiles), &iter);
		}
	}
	
	// Return the selected CompressedFile*.
	return selFile;
}

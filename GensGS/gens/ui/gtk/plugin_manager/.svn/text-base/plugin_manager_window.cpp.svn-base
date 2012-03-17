/***************************************************************************
 * Gens: (GTK+) Plugin Manager Window.                                     *
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

#include "plugin_manager_window.hpp"
#include "gens/gens_window.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include <gtk/gtk.h>

// TODO: Get rid of gtk-misc.h
#include "gtk-misc.h"

// Plugin Manager
#include "plugins/pluginmgr.hpp"

// C++ includes
#include <string>
#include <sstream>
#include <vector>
using std::endl;
using std::string;
using std::stringstream;
using std::vector;


PluginManagerWindow* PluginManagerWindow::m_Instance = NULL;
PluginManagerWindow* PluginManagerWindow::Instance(GtkWindow *parent)
{
	if (m_Instance == NULL)
	{
		// Instance is deleted. Initialize the General Options window.
		m_Instance = new PluginManagerWindow();
	}
	else
	{
		// Instance already exists. Set focus.
		m_Instance->setFocus();
	}
	
	// Set modality of the window.
	if (!parent)
		parent = GTK_WINDOW(gens_window);
	m_Instance->setModal(parent);
	
	return m_Instance;
}


PluginManagerWindow::~PluginManagerWindow()
{
	if (m_Window)
		gtk_widget_destroy(GTK_WIDGET(m_Window));
	
	m_Instance = NULL;
}


gboolean PluginManagerWindow::GTK_Close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	return reinterpret_cast<PluginManagerWindow*>(user_data)->close();
}


void PluginManagerWindow::dlgButtonPress(uint32_t button)
{
	switch (button)
	{
		case WndBase::BUTTON_OK:
			close();
			break;
	}
}


/**
 * Window is closed.
 */
gboolean PluginManagerWindow::close(void)
{
	delete this;
	return FALSE;
}


/**
 * PluginManagerWindow: Create the General Options Window.
 */
PluginManagerWindow::PluginManagerWindow()
{
	m_Window = gtk_dialog_new();
	gtk_widget_set_name(GTK_WIDGET(m_Window), "PluginManagerWindow");
	gtk_container_set_border_width(GTK_CONTAINER(m_Window), 0);
	gtk_window_set_title(GTK_WINDOW(m_Window), "Plugin Manager");
	gtk_window_set_position(GTK_WINDOW(m_Window), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(m_Window), FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(m_Window), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator(GTK_DIALOG(m_Window), FALSE);
	
	// Set the window data.
	g_object_set_data(G_OBJECT(m_Window), "PluginManagerWindow", m_Window);
	
	// Load the Gens icon.
	GdkPixbuf *icon = create_pixbuf("Gens2.ico");
	if (icon)
	{
		gtk_window_set_icon(GTK_WINDOW(m_Window), icon);
		gdk_pixbuf_unref(icon);
	}
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)m_Window, "delete_event",
			  G_CALLBACK(PluginManagerWindow::GTK_Close), (gpointer)this);
	g_signal_connect((gpointer)m_Window, "destroy_event",
			  G_CALLBACK(PluginManagerWindow::GTK_Close), (gpointer)this);
	
	// Get the dialog VBox.
	GtkWidget *vboxDialog = GTK_DIALOG(m_Window)->vbox;
	gtk_widget_set_name(vboxDialog, "vboxDialog");
	gtk_widget_show(vboxDialog);
	g_object_set_data_full(G_OBJECT(m_Window), "vboxDialog",
			       g_object_ref(vboxDialog), (GDestroyNotify)g_object_unref);
	
	// Create the plugin list frame.
	createPluginListFrame(GTK_BOX(vboxDialog));
	
	// Create the plugin information frame.
	createPluginInfoFrame(GTK_BOX(vboxDialog));
	
	// Create an accelerator group.
	m_AccelTable = gtk_accel_group_new();
	
	// Add the OK button.
	addDialogButtons(m_Window, WndBase::BAlign_Default,
			 WndBase::BUTTON_OK, 0,
			 WndBase::BUTTON_ALL);
	
	// Add the accel group to the window.
	gtk_window_add_accel_group(GTK_WINDOW(m_Window), GTK_ACCEL_GROUP(m_AccelTable));
	
	// Populate the plugin list.
	lmPluginList = NULL;
	populatePluginList();
	
	// Show the window.
	setVisible(true);
	
	// Make sure nothing is selected initially.
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(lstPluginList));
	gtk_tree_selection_unselect_all(selection);
	lstPluginList_cursor_changed(NULL);
}


void PluginManagerWindow::createPluginListFrame(GtkBox *container)
{
	// Create the plugin list frame.
	GtkWidget *fraPluginList = gtk_frame_new(NULL);
	gtk_widget_set_name(fraPluginList, "fraPluginList");
	gtk_frame_set_shadow_type(GTK_FRAME(fraPluginList), GTK_SHADOW_ETCHED_IN);
	gtk_container_set_border_width(GTK_CONTAINER(fraPluginList), 4);
	gtk_widget_show(fraPluginList);
	g_object_set_data_full(G_OBJECT(m_Window), "fraPluginList",
			       g_object_ref(fraPluginList), (GDestroyNotify)g_object_unref);
	gtk_box_pack_start(container, fraPluginList, TRUE, TRUE, 0);
	
	// Label for the plugin list frame.
	GtkWidget *lblPluginList = gtk_label_new("<b><i>Internal Plugins</i></b>");
	gtk_widget_set_name(lblPluginList, "lblPluginList");
	gtk_label_set_use_markup(GTK_LABEL(lblPluginList), TRUE);
	gtk_widget_show(lblPluginList);
	g_object_set_data_full(G_OBJECT(m_Window), "lblPluginList",
			       g_object_ref(lblPluginList), (GDestroyNotify)g_object_unref);
	gtk_frame_set_label_widget(GTK_FRAME(fraPluginList), lblPluginList);
	
	// VBox for the plugin list.
	GtkWidget *vboxPluginList = gtk_vbox_new(FALSE, 0);
	gtk_widget_set_name(vboxPluginList, "vboxPluginList");
	gtk_container_set_border_width(GTK_CONTAINER(vboxPluginList), 8);
	gtk_widget_show(vboxPluginList);
	g_object_set_data_full(G_OBJECT(m_Window), "vboxPluginList",
			       g_object_ref(vboxPluginList), (GDestroyNotify)g_object_unref);
	gtk_container_add(GTK_CONTAINER(fraPluginList), vboxPluginList);
	
	// Scrolled Window for the plugin list.
	GtkWidget *scrlPluginList = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_set_name(scrlPluginList, "scrlPluginList");
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrlPluginList), GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrlPluginList),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_widget_show(scrlPluginList);
	gtk_box_pack_start(GTK_BOX(vboxPluginList), scrlPluginList, TRUE, TRUE, 0);
	g_object_set_data_full(G_OBJECT(m_Window), "scrlPluginList",
			       g_object_ref(scrlPluginList), (GDestroyNotify)g_object_unref);
	
	// Tree view containing the plugins.
	lstPluginList = gtk_tree_view_new();
	gtk_widget_set_name(lstPluginList, "lstPluginList");
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(lstPluginList), FALSE);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(lstPluginList), FALSE);
	gtk_widget_set_size_request(lstPluginList, 480, 160);
	gtk_widget_show(lstPluginList);
	gtk_container_add(GTK_CONTAINER(scrlPluginList), lstPluginList);
	g_object_set_data_full(G_OBJECT(m_Window), "lstPluginList",
			       g_object_ref(lstPluginList), (GDestroyNotify)g_object_unref);
	
	// Connect the treeview's "cursor-changed" signal.
	g_signal_connect((gpointer)lstPluginList, "cursor-changed",
			 G_CALLBACK(lstPluginList_cursor_changed_STATIC), this);
}


void PluginManagerWindow::createPluginInfoFrame(GtkBox *container)
{
	// Create the plugin information frame.
	GtkWidget *fraPluginInfo = gtk_frame_new(NULL);
	gtk_widget_set_name(fraPluginInfo, "fraPluginInfo");
	gtk_frame_set_shadow_type(GTK_FRAME(fraPluginInfo), GTK_SHADOW_ETCHED_IN);
	gtk_container_set_border_width(GTK_CONTAINER(fraPluginInfo), 4);
	gtk_widget_show(fraPluginInfo);
	g_object_set_data_full(G_OBJECT(m_Window), "fraPluginInfo",
			       g_object_ref(fraPluginInfo), (GDestroyNotify)g_object_unref);
	gtk_box_pack_start(container, fraPluginInfo, TRUE, TRUE, 0);
	
	// Label for the plugin information frame.
	GtkWidget *lblPluginInfo = gtk_label_new("<b><i>Plugin Information</i></b>");
	gtk_widget_set_name(lblPluginInfo, "lblPluginInfo");
	gtk_label_set_use_markup(GTK_LABEL(lblPluginInfo), TRUE);
	gtk_widget_show(lblPluginInfo);
	g_object_set_data_full(G_OBJECT(m_Window), "lblPluginInfo",
			       g_object_ref(lblPluginInfo), (GDestroyNotify)g_object_unref);
	gtk_frame_set_label_widget(GTK_FRAME(fraPluginInfo), lblPluginInfo);
	
	// VBox for the plugin information frame.
	GtkWidget *vboxPluginInfo = gtk_vbox_new(FALSE, 8);
	gtk_container_set_border_width(GTK_CONTAINER(vboxPluginInfo), 8);
	gtk_widget_set_name(vboxPluginInfo, "vboxPluginInfo");
	gtk_widget_show(vboxPluginInfo);
	g_object_set_data_full(G_OBJECT(m_Window), "vboxPluginInfo",
			       g_object_ref(vboxPluginInfo), (GDestroyNotify)g_object_unref);
	gtk_container_add(GTK_CONTAINER(fraPluginInfo), vboxPluginInfo);
	
	// HBox for the main plugin info.
	GtkWidget *hboxPluginMainInfo = gtk_hbox_new(FALSE, 8);
	gtk_widget_set_name(hboxPluginMainInfo, "hboxPluginMainInfo");
	gtk_widget_show(hboxPluginMainInfo);
	g_object_set_data_full(G_OBJECT(m_Window), "hboxPluginMainInfo",
			       g_object_ref(hboxPluginMainInfo), (GDestroyNotify)g_object_unref);
	gtk_box_pack_start(GTK_BOX(vboxPluginInfo), hboxPluginMainInfo, TRUE, FALSE, 0);
	
#ifdef GENS_PNG
	// Create the plugin icon widget.
	createPluginIconWidget(GTK_BOX(hboxPluginMainInfo));
#endif /* GENS_PNG */
	
	// VBox for the main plugin info.
	GtkWidget *vboxPluginMainInfo = gtk_vbox_new(FALSE, 4);
	gtk_widget_set_name(vboxPluginMainInfo, "vboxPluginMainInfo");
	gtk_widget_show(vboxPluginMainInfo);
	g_object_set_data_full(G_OBJECT(m_Window), "vboxPluginMainInfo",
			       g_object_ref(vboxPluginMainInfo), (GDestroyNotify)g_object_unref);
	gtk_box_pack_start(GTK_BOX(hboxPluginMainInfo), vboxPluginMainInfo, TRUE, TRUE, 0);
	
	// Label for the main plugin info.
	lblPluginMainInfo = gtk_label_new("\n\n\n\n\n");
	gtk_widget_set_name(lblPluginMainInfo, "lblPluginMainInfo");
	gtk_label_set_selectable(GTK_LABEL(lblPluginMainInfo), TRUE);
	gtk_misc_set_alignment(GTK_MISC(lblPluginMainInfo), 0.0f, 0.0f);
	gtk_widget_show(lblPluginMainInfo);
	g_object_set_data_full(G_OBJECT(m_Window), "lblPluginMainInfo",
			       g_object_ref(lblPluginMainInfo), (GDestroyNotify)g_object_unref);
	gtk_box_pack_start(GTK_BOX(vboxPluginMainInfo), lblPluginMainInfo, TRUE, FALSE, 0);
	
	// Label for secondary plugin info.
	m_lblPluginSecInfo = gtk_label_new("\n");
	gtk_widget_set_name(m_lblPluginSecInfo, "m_lblPluginSecInfo");
	gtk_label_set_selectable(GTK_LABEL(m_lblPluginSecInfo), TRUE);
	gtk_misc_set_alignment(GTK_MISC(m_lblPluginSecInfo), 0.0f, 0.0f);
	gtk_widget_show(m_lblPluginSecInfo);
	g_object_set_data_full(G_OBJECT(m_Window), "m_lblPluginSecInfo",
			       g_object_ref(m_lblPluginSecInfo), (GDestroyNotify)g_object_unref);
	gtk_container_add(GTK_CONTAINER(vboxPluginInfo), m_lblPluginSecInfo);
	
	// Frame for the plugin description.
	GtkWidget *fraPluginDesc = gtk_frame_new(NULL);
	gtk_widget_set_name(fraPluginDesc, "fraPluginDesc");
	gtk_frame_set_shadow_type(GTK_FRAME(fraPluginDesc), GTK_SHADOW_NONE);
	gtk_container_set_border_width(GTK_CONTAINER(fraPluginDesc), 4);
	gtk_widget_show(fraPluginDesc);
	g_object_set_data_full(G_OBJECT(m_Window), "fraPluginDesc",
			       g_object_ref(fraPluginDesc), (GDestroyNotify)g_object_unref);
	gtk_box_pack_start(GTK_BOX(vboxPluginInfo), fraPluginDesc, TRUE, TRUE, 0);
	
	// Label for the plugin description frame.
	lblPluginDescTitle = gtk_label_new(" ");
	gtk_widget_set_name(lblPluginDescTitle, "lblPluginDescTitle");
	g_object_set_data_full(G_OBJECT(m_Window), "lblPluginDescTitle",
			       g_object_ref(lblPluginDescTitle), (GDestroyNotify)g_object_unref);
	gtk_frame_set_label_widget(GTK_FRAME(fraPluginDesc), lblPluginDescTitle);
	
	// Label for the plugin description.
	lblPluginDesc = gtk_label_new(NULL);
	gtk_widget_set_name(lblPluginDesc, "lblPluginDesc");
	gtk_label_set_selectable(GTK_LABEL(lblPluginDesc), TRUE);
	gtk_misc_set_alignment(GTK_MISC(lblPluginDesc), 0.0f, 0.0f);
	gtk_widget_set_size_request(lblPluginDesc, -1, 64);
	gtk_widget_show(lblPluginDesc);
	g_object_set_data_full(G_OBJECT(m_Window), "lblPluginDesc",
			       g_object_ref(lblPluginDesc), (GDestroyNotify)g_object_unref);
	gtk_container_add(GTK_CONTAINER(fraPluginDesc), lblPluginDesc);
}


void PluginManagerWindow::populatePluginList(void)
{
	// Check if the list model is already created.
	// If it is, clear it; if not, create a new one.
	if (lmPluginList)
		gtk_list_store_clear(lmPluginList);
	else
		lmPluginList = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);
	
	// Set the view model of the treeview.
	gtk_tree_view_set_model(GTK_TREE_VIEW(lstPluginList), GTK_TREE_MODEL(lmPluginList));
	
	GtkTreeViewColumn *colPlugin;
	
	// Delete any existing columns.
	do
	{
		colPlugin = gtk_tree_view_get_column(GTK_TREE_VIEW(lstPluginList), 0);
		if (colPlugin)
			gtk_tree_view_remove_column(GTK_TREE_VIEW(lstPluginList), colPlugin);
	} while (colPlugin != NULL);
	
	// Create the renderer and columns.
	GtkCellRenderer *textRenderer = gtk_cell_renderer_text_new();
	colPlugin = gtk_tree_view_column_new_with_attributes("Plugin", textRenderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(lstPluginList), colPlugin);
	
	// Add all plugins to the treeview.
	char tmp[64];
	vector<MDP_t*>::iterator curPlugin;
	for (curPlugin = PluginMgr::vRenderPlugins.begin();
	     curPlugin != PluginMgr::vRenderPlugins.end(); curPlugin++)
	{
		GtkTreeIter iter;
		gtk_list_store_append(lmPluginList, &iter);
		
		MDP_t *plugin = (*curPlugin);
		const char *pluginName;
		if (plugin->desc && plugin->desc->name)
		{
			pluginName = plugin->desc->name;
		}
		else
		{
			// No description or name.
			// TODO: For external plugins, indicate the external file.
			char tmp[64];
			sprintf(tmp, "[No name: 0x%08X]", (unsigned int)plugin);
			pluginName = tmp;
		}
		
		gtk_list_store_set(GTK_LIST_STORE(lmPluginList), &iter, 0, pluginName, 1, plugin, -1);
	}
}


void PluginManagerWindow::lstPluginList_cursor_changed_STATIC(GtkTreeView *tree_view, gpointer user_data)
{
	reinterpret_cast<PluginManagerWindow*>(user_data)->lstPluginList_cursor_changed(tree_view);
}


void PluginManagerWindow::lstPluginList_cursor_changed(GtkTreeView *tree_view)
{
	// Check which plugin is clicked.
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(lstPluginList));
	
	GtkTreeIter iter;
	if (!gtk_tree_selection_get_selected(selection, (GtkTreeModel**)(&lmPluginList), &iter))
	{
		// No plugin selected.
		gtk_label_set_text(GTK_LABEL(lblPluginMainInfo), "No plugin selected.\n\n\n\n\n");
		gtk_label_set_text(GTK_LABEL(m_lblPluginSecInfo), "\n");
		gtk_label_set_text(GTK_LABEL(lblPluginDescTitle), " ");
		gtk_label_set_text(GTK_LABEL(lblPluginDesc), NULL);
#ifdef GENS_PNG
		clearIcon();
#endif /* GENS_PNG */
		return;
	}
	
	// Found a selected plugin.
	GValue gVal = { 0 };
	MDP_t *plugin;
	
	gtk_tree_model_get_value(GTK_TREE_MODEL(lmPluginList), &iter, 1, &gVal);
	plugin = (MDP_t*)g_value_peek_pointer(&gVal);
	g_value_unset(&gVal);
	
	// Get the plugin information.
	if (!plugin)
	{
		// Invalid plugin.
		gtk_label_set_text(GTK_LABEL(lblPluginMainInfo), "Invalid plugin selected.\n\n\n\n\n");
		gtk_label_set_text(GTK_LABEL(m_lblPluginSecInfo), "\n");
		gtk_label_set_text(GTK_LABEL(lblPluginDescTitle), " ");
		gtk_label_set_text(GTK_LABEL(lblPluginDesc), NULL);
#ifdef GENS_PNG
		clearIcon();
#endif /* GENS_PNG */
		return;
	}
	
	if (!plugin->desc)
	{
		gtk_label_set_text(GTK_LABEL(lblPluginMainInfo), "This plugin does not have a valid description field.\n\n\n\n\n");
		gtk_label_set_text(GTK_LABEL(m_lblPluginSecInfo), "\n");
		gtk_label_set_text(GTK_LABEL(lblPluginDescTitle), " ");
		gtk_label_set_text(GTK_LABEL(lblPluginDesc), NULL);
#ifdef GENS_PNG
		clearIcon();
#endif /* GENS_PNG */
		return;
	}
	
	// Fill in the descriptions.
	MDP_Desc_t *desc = plugin->desc;
	stringstream ssMainInfo;
	int lines = 4;			// Name, MDP Author, Version, and License are always printed.
	const int linesReserved = 6;	// Number of lines reserved.
	
	// Plugin name.
	ssMainInfo << "Name: " << (desc->name ? string(desc->name) : "(none)") << endl;
	
	// Plugin version.
	ssMainInfo << "Version: " << MDP_VERSION_MAJOR(plugin->pluginVersion)
				  << "." << MDP_VERSION_MINOR(plugin->pluginVersion)
				  << "." << MDP_VERSION_REVISION(plugin->pluginVersion) << endl;
	
	// Plugin author.
	ssMainInfo << "MDP Author: " + (desc->author_mdp ? string(desc->author_mdp) : "(none)") << endl;
	
	// Original code author.
	if (desc->author_orig)
	{
		ssMainInfo << "Original Author: " << string(desc->author_orig) << endl;
		lines++;
	}
	
	// Website.
	if (desc->website)
	{
		ssMainInfo << "Website: " << string(desc->website) << endl;
		lines++;
	}
	
	// License.
	ssMainInfo << "License: " + (desc->license ? string(desc->license) : "(none)");
	
	// Linebreaks needed.
	const int linesNeeded = linesReserved - lines;
	for (int i = 0; i < linesNeeded; i++)
	{
		ssMainInfo << endl;
	}
	
	// Set the main plugin information.
	gtk_label_set_text(GTK_LABEL(lblPluginMainInfo), ssMainInfo.str().c_str());
	
	// UUID.
	string sUUID = UUIDtoString(plugin->uuid);
	
	// Secondary plugin information.
	// Includes UUID and CPU flags.
	stringstream ssSecInfo;
	ssSecInfo << "UUID: " << sUUID << endl
		  << GetCPUFlags(plugin->cpuFlagsRequired, plugin->cpuFlagsSupported, true);
	
	// Set the secondary information label.
	gtk_label_set_text(GTK_LABEL(m_lblPluginSecInfo), ssSecInfo.str().c_str());
	gtk_label_set_use_markup(GTK_LABEL(m_lblPluginSecInfo), TRUE);
	
	// Plugin description.
	gtk_label_set_text(GTK_LABEL(lblPluginDesc), desc->description);
	if (desc->description)
	{
		gtk_label_set_text(GTK_LABEL(lblPluginDescTitle), "<b><i>Description:</i></b>");
		gtk_label_set_use_markup(GTK_LABEL(lblPluginDescTitle), TRUE);
	}
	else
	{
		gtk_label_set_text(GTK_LABEL(lblPluginDescTitle), " ");
	}
	
#ifdef GENS_PNG
	// Plugin icon.
	if (!displayIcon(desc->icon, desc->iconLength))
	{
		// No plugin icon found. Clear the pixbuf.
		clearIcon();
	}
#endif /* GENS_PNG */
}


#ifdef GENS_PNG
/**
 * createPluginIconWidget(): Create the GTK+ plugin icon widget and pixbuf.
 * @param container Container for the plugin icon widget.
 */
inline void PluginManagerWindow::createPluginIconWidget(GtkBox *container)
{
	// Plugin icon pixbuf.
	m_pbufPluginIcon = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, 32, 32);
	g_object_set_data_full(G_OBJECT(m_Window), "m_pbufPluginIcon",
			       g_object_ref(m_pbufPluginIcon), (GDestroyNotify)g_object_unref);
	
	// Plugin icon widget.
	m_imgPluginIcon = gtk_image_new();
	gtk_widget_set_name(m_imgPluginIcon, "m_imgPluginIcon");
	gtk_misc_set_alignment(GTK_MISC(m_imgPluginIcon), 0.0f, 0.0f);
	gtk_widget_show(m_imgPluginIcon);
	g_object_set_data_full(G_OBJECT(m_Window), "m_imgPluginIcon",
			       g_object_ref(m_imgPluginIcon), (GDestroyNotify)g_object_unref);
	gtk_box_pack_start(GTK_BOX(container), m_imgPluginIcon, FALSE, FALSE, 0);
	
	// Clear the icon.
	clearIcon();
}


/**
 * displayIcon(): Displays the plugin icon.
 * @param icon Icon data. (PNG format)
 * @param iconLength Length of the icon data.
 * @return True if the icon was displayed; false otherwise.
 */
bool PluginManagerWindow::displayIcon(const unsigned char* icon, const unsigned int iconLength)
{
	static const unsigned char pngMagicNumber[8] = {0x89, 'P', 'N', 'G',0x0D, 0x0A, 0x1A, 0x0A};
	
	if (!icon || iconLength < sizeof(pngMagicNumber))
		return false;
	
	// Check that the icon is in PNG format.
	if (memcmp(icon, pngMagicNumber, sizeof(pngMagicNumber)))
	{
		// Not in PNG format.
		return false;
	}

	// Initialize libpng.
	
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
		return false;
	
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return false;
	}
	
	png_infop end_info = png_create_info_struct(png_ptr);
	if (!end_info)
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return false;
	}
	
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		// TODO: Is setjmp() really necessary?
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		return false;
	}
	
	// Set the custom read function.
	png_dataptr = icon;
	png_datalen = iconLength;
	png_datapos = 0;
	
	void *read_io_ptr = png_get_io_ptr(png_ptr);
	png_set_read_fn(png_ptr, read_io_ptr, &png_user_read_data);
	
	// Get the PNG information.
	png_read_info(png_ptr, info_ptr);
	
	// Get the PNG information.
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type, compression_type, filter_method;
	bool has_alpha = false;
	
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
		     &interlace_type, &compression_type, &filter_method);
	
	
	if (width != 32 || height != 32)
	{
		// Not 32x32.
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return false;
	}
	
	// Make sure RGB color is used.
	if (color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png_ptr);
	else if (color_type == PNG_COLOR_TYPE_GRAY)
		png_set_gray_to_rgb(png_ptr);
	else if (color_type == PNG_COLOR_TYPE_RGB_ALPHA)
		has_alpha = true;
	
	// GTK+ expects RGBA format.
	// TODO: Check if this is the same on big-endian machines.
	
	// Convert tRNS to alpha channel.
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
	{
		png_set_tRNS_to_alpha(png_ptr);
		has_alpha = true;
	}
	
	// Convert 16-bit per channel PNG to 8-bit.
	if (bit_depth == 16)
		png_set_strip_16(png_ptr);
	
	// Get the new PNG information.
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
		     &interlace_type, &compression_type, &filter_method);
	
	// Check if the PNG image has an alpha channel.
	if (!has_alpha)
	{
		// No alpha channel specified.
		// Use filler instead.
		png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
	}
	
	// Update the PNG info.
	png_read_update_info(png_ptr, info_ptr);
	
	// Create the row pointers.
	int rowstride = gdk_pixbuf_get_rowstride(m_pbufPluginIcon);
	guchar *pixels = gdk_pixbuf_get_pixels(m_pbufPluginIcon);
	png_bytep row_pointers[32];
	for (unsigned int i = 0; i < 32; i++)
	{
		row_pointers[i] = pixels;
		pixels += rowstride;
	}
	
	// Read the image data.
	png_read_image(png_ptr, row_pointers);
	
	// Close the PNG image.
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	
	// Set the GTK+ image to the new icon.
	gtk_image_set_from_pixbuf(GTK_IMAGE(m_imgPluginIcon), m_pbufPluginIcon);
	
	return true;
}


/**
 * clearIcon(): Clear the plugin icon.
 */
void PluginManagerWindow::clearIcon(void)
{
	if (!m_pbufPluginIcon)
		return;
	
	guchar *pixels = gdk_pixbuf_get_pixels(m_pbufPluginIcon);
	int rowstride = gdk_pixbuf_get_rowstride(m_pbufPluginIcon);
	int height = gdk_pixbuf_get_height(m_pbufPluginIcon);
	int width = gdk_pixbuf_get_width(m_pbufPluginIcon);
	int bits_per_sample = gdk_pixbuf_get_bits_per_sample(m_pbufPluginIcon);
	int n_channels = gdk_pixbuf_get_n_channels(m_pbufPluginIcon);
	
	// The last row of the pixbuf data may not be fully allocated.
	// See http://library.gnome.org/devel/gdk-pixbuf/stable/gdk-pixbuf-gdk-pixbuf.html
	int size = (rowstride * (height - 1)) + (width * ((n_channels * bits_per_sample + 7) / 8));
	memset(pixels, 0x00, size);
	
	gtk_image_set_from_pixbuf(GTK_IMAGE(m_imgPluginIcon), m_pbufPluginIcon);
}
#endif /* GENS_PNG */

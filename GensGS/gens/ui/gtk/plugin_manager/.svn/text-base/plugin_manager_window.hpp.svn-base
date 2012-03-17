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

#ifndef GENS_GTK_PLUGIN_MANAGER_WINDOW_HPP
#define GENS_GTK_PLUGIN_MANAGER_WINDOW_HPP

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef __cplusplus

#include "wndbase.hpp"

// C++ includes
#include <string>

#ifdef GENS_PNG
#include <png.h>
#endif /* GENS_PNG */

class PluginManagerWindow : public WndBase
{
	public:
		static PluginManagerWindow* Instance(GtkWindow *parent = NULL);
		
		gboolean close(void);
		
		void lstPluginList_cursor_changed(GtkTreeView *tree_view);
		
	protected:
		PluginManagerWindow();
		~PluginManagerWindow();
		
		static PluginManagerWindow* m_Instance;
		
		void dlgButtonPress(uint32_t button);
		
		// Static functions required for GTK+ callbacks.
		static gboolean GTK_Close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
		static void lstPluginList_cursor_changed_STATIC(GtkTreeView *tree_view, gpointer user_data);
		
		// GTK+ objects
		GtkWidget *lstPluginList;
		GtkListStore *lmPluginList;
		GtkWidget *lblPluginMainInfo;
		GtkWidget *m_lblPluginSecInfo;
		GtkWidget *lblPluginDesc;
		GtkWidget *lblPluginDescTitle;
		
		// Initialization functions.
		void createPluginListFrame(GtkBox *container);
		void createPluginInfoFrame(GtkBox *container);
		void populatePluginList(void);
		
		// CPU flags.
		static std::string GetCPUFlags(uint32_t cpuFlagsRequired,
					       uint32_t cpuFlagsSupported,
					       bool formatting);
		
		// Convert UUID to string.
		static std::string UUIDtoString(const unsigned char *uuid);
		
#ifdef GENS_PNG
		// Plugin icon.
		GdkPixbuf *m_pbufPluginIcon;
		GtkWidget *m_imgPluginIcon;
		
		inline void createPluginIconWidget(GtkBox *container);
		bool displayIcon(const unsigned char* icon, const unsigned int iconLength);
		void clearIcon(void);
		
		// PNG read variables.
		// WARNING: Not multi-object/multi-thread safe!
		static const unsigned char *png_dataptr;
		static unsigned int png_datalen;
		static unsigned int png_datapos;
		static void png_user_read_data(png_structp png_ptr, png_bytep data, png_size_t length);
#endif /* GENS_PNG */
};

#endif /* __cplusplus */

#endif /* GENS_GTK_PLUGIN_MANAGER_WINDOW_HPP */

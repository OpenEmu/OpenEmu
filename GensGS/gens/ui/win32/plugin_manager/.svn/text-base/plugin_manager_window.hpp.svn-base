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
		static PluginManagerWindow* Instance(HWND parent = NULL);
		static bool isOpen(void) { return (m_Instance != NULL); }
		
		bool isDialogMessage(MSG *msg) { return IsDialogMessage((HWND)m_Window, msg); }
		
	protected:
		PluginManagerWindow();
		~PluginManagerWindow();
		
		static PluginManagerWindow* m_Instance;
		static WNDCLASS m_WndClass;
		
		// Window size.
		static const int m_WndWidth;
		static const int m_WndHeight;
		static const int m_fraPluginList_Width;
		static const int m_fraPluginList_Height;
		static const int m_fraPluginInfo_Width;
		static const int m_fraPluginInfo_Height;
		
		static LRESULT CALLBACK WndProc_STATIC(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		
		bool m_childWindowsCreated;
		void createChildWindows(HWND hWnd);
		
		// Win32 objects
		HWND m_lstPluginList;
		HWND m_lblPluginMainInfo;
		HWND m_lblPluginSecInfo;
		HWND m_lblPluginDesc;
		
		// Initialization functions.
		void createPluginListFrame(HWND hWnd);
		void createPluginInfoFrame(HWND hWnd);
		void populatePluginList(void);
		
		// lstPluginList cursor changed.
		void lstPluginList_cursor_changed(void);
		
		// CPU flags.
		static std::string GetCPUFlags(uint32_t cpuFlagsRequired,
					       uint32_t cpuFlagsSupported,
					       bool formatting);
		
		// Convert UUID to string.
		static std::string UUIDtoString(const unsigned char *uuid);
		
#ifdef GENS_PNG
		// Plugin icon.
		HBITMAP m_hbmpPluginIcon;
		void *m_bmpPluginIconData;
		HWND m_imgPluginIcon;
		
		inline void createPluginIconWidget(HWND hWnd);
		inline unsigned int getBGColor(void);
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

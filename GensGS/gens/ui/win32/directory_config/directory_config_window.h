/***************************************************************************
 * Gens: (Win32) Directory Configuration Window.                           *
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

#ifndef GENS_WIN32_DIRECTORY_CONFIG_WINDOW_H
#define GENS_WIN32_DIRECTORY_CONFIG_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

// Directory entries
struct DirEntry_t
{
	const char* title;
	const char* tag;
	char* entry;
};

// Contains all the Directory entries.
extern const unsigned short DirEntries_Count;
extern const struct DirEntry_t DirEntries[];
extern HWND dc_txtEntry[/*DirEntries_Count*/];

HWND create_directory_config_window(void); 
extern HWND directory_config_window;
void Directory_Config_Window_CreateChildWindows(HWND hWnd);

#ifdef __cplusplus
}
#endif

#endif /* GENS_WIN32_DIRECTORY_CONFIG_WINDOW_H */

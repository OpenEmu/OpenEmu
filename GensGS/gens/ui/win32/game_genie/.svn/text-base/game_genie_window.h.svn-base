/***************************************************************************
 * Gens: (Win32) Game Genie Window.                                        *
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
 
#ifndef GENS_WIN32_GAME_GENIE_WINDOW_H
#define GENS_WIN32_GAME_GENIE_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

HWND create_game_genie_window(void); 
extern HWND game_genie_window;
void Game_Genie_Window_CreateChildWindows(HWND hWnd);

// Controls
extern HWND gg_txtCode;
extern HWND gg_txtName;
extern HWND gg_lstvCodes;

// Old window procedures for gg_txtCode and gg_txtName.
extern WNDPROC gg_txtCode_oldProc;
extern WNDPROC gg_txtName_oldProc;

#ifdef __cplusplus
}
#endif

#endif /* GENS_WIN32_GAME_GENIE_WINDOW_H */

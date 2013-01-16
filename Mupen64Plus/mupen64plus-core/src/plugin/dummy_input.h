/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - dummy_input.h                                           *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2009 Richard Goedeken                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#if !defined(DUMMY_INPUT_H)
#define DUMMY_INPUT_H

#include "api/m64p_plugin.h"

extern m64p_error dummyinput_PluginGetVersion(m64p_plugin_type *PluginType, int *PluginVersion,
                                              int *APIVersion, const char **PluginNamePtr, int *Capabilities);
extern void dummyinput_InitiateControllers (CONTROL_INFO ControlInfo);
extern void dummyinput_GetKeys(int Control, BUTTONS * Keys );
extern void dummyinput_ControllerCommand(int Control, unsigned char *Command);
extern void dummyinput_GetKeys(int Control, BUTTONS * Keys);
extern void dummyinput_InitiateControllers(CONTROL_INFO ControlInfo);
extern void dummyinput_ReadController(int Control, unsigned char *Command);
extern int  dummyinput_RomOpen(void);
extern void dummyinput_RomClosed(void);
extern void dummyinput_SDL_KeyDown(int keymod, int keysym);
extern void dummyinput_SDL_KeyUp(int keymod, int keysym);

#endif /* DUMMY_INPUT_H */



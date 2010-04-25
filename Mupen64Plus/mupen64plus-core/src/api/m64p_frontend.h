/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-core - m64p_frontend.h                                    *
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

/* This header file defines typedefs for function pointers to Core functions
 * designed for use by the front-end user interface.
 */

#if !defined(M64P_FRONTEND_H)
#define M64P_FRONTEND_H

#include "m64p_types.h"

/* CoreStartup()
 *
 * This function initializes libmupen64plus for use by allocating memory,
 * creating data structures, and loading the configuration file.
 */
typedef m64p_error (*ptr_CoreStartup)(int, const char *, const char *, void *, void (*)(void *, int, const char *), void *, void (*)(void *, m64p_core_param, int));

/* CoreShutdown()
 *
 * This function saves the configuration file, then destroys data structures
 * and releases memory allocated by the core library.
 */
typedef m64p_error (*ptr_CoreShutdown)(void);

/* CoreAttachPlugin()
 *
 * This function attaches the given plugin to the emulator core. There can only
 * be one plugin of each type attached to the core at any given time. 
 */
typedef m64p_error (*ptr_CoreAttachPlugin)(m64p_plugin_type, m64p_dynlib_handle);

/* CoreDetachPlugin()
 *
 * This function detaches the given plugin from the emulator core, and re-attaches
 * the 'dummy' plugin functions. 
 */
typedef m64p_error (*ptr_CoreDetachPlugin)(m64p_plugin_type);

/* CoreDoCommand()
 *
 * This function sends a command to the emulator core.
 */
typedef m64p_error (*ptr_CoreDoCommand)(m64p_command, int, void *);

/* CoreOverrideVidExt()
 *
 * This function overrides the core's internal SDL-based OpenGL functions. This
 * override functionality allows a front-end to define its own video extension
 * functions to be used instead of the SDL functions. If any of the function
 * pointers in the structure are NULL, the override function will be disabled
 * and the core's internal SDL functions will be used.
 */
typedef m64p_error (*ptr_CoreOverrideVidExt)(m64p_video_extension_functions *);

/* CoreAddCheat()
 *
 * This function will add a Cheat Function to a list of currently active cheats
 * which are applied to the open ROM.
 */
typedef m64p_error (*ptr_CoreAddCheat)(const char *, m64p_cheat_code *, int);

/* CoreCheatEnabled()
 *
 * This function will enable or disable a Cheat Function which is in the list of
 * currently active cheats.
 */
typedef m64p_error (*ptr_CoreCheatEnabled)(const char *, int);

#endif /* #define M64P_FRONTEND_H */


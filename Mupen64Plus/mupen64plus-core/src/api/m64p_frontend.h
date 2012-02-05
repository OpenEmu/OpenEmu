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

#ifdef __cplusplus
extern "C" {
#endif


/* pointer types to the callback functions in the front-end application */
typedef void (*ptr_DebugCallback)(void *Context, int level, const char *message);
typedef void (*ptr_StateCallback)(void *Context, m64p_core_param param_type, int new_value);
#if defined(M64P_CORE_PROTOTYPES)
EXPORT void CALL DebugCallback(void *Context, int level, const char *message);
EXPORT void CALL StateCallback(void *Context, m64p_core_param param_type, int new_value);
#endif

/* CoreStartup()
 *
 * This function initializes libmupen64plus for use by allocating memory,
 * creating data structures, and loading the configuration file.
 */
typedef m64p_error (*ptr_CoreStartup)(int, const char *, const char *, void *, ptr_DebugCallback, void *, ptr_StateCallback);
#if defined(M64P_CORE_PROTOTYPES)
EXPORT m64p_error CALL CoreStartup(int, const char *, const char *, void *, ptr_DebugCallback, void *, ptr_StateCallback);
#endif

/* CoreShutdown()
 *
 * This function saves the configuration file, then destroys data structures
 * and releases memory allocated by the core library.
 */
typedef m64p_error (*ptr_CoreShutdown)(void);
#if defined(M64P_CORE_PROTOTYPES)
EXPORT m64p_error CALL CoreShutdown(void);
#endif

/* CoreAttachPlugin()
 *
 * This function attaches the given plugin to the emulator core. There can only
 * be one plugin of each type attached to the core at any given time. 
 */
typedef m64p_error (*ptr_CoreAttachPlugin)(m64p_plugin_type, m64p_dynlib_handle);
#if defined(M64P_CORE_PROTOTYPES)
EXPORT m64p_error CALL CoreAttachPlugin(m64p_plugin_type, m64p_dynlib_handle);
#endif

/* CoreDetachPlugin()
 *
 * This function detaches the given plugin from the emulator core, and re-attaches
 * the 'dummy' plugin functions. 
 */
typedef m64p_error (*ptr_CoreDetachPlugin)(m64p_plugin_type);
#if defined(M64P_CORE_PROTOTYPES)
EXPORT m64p_error CALL CoreDetachPlugin(m64p_plugin_type);
#endif

/* CoreDoCommand()
 *
 * This function sends a command to the emulator core.
 */
typedef m64p_error (*ptr_CoreDoCommand)(m64p_command, int, void *);
#if defined(M64P_CORE_PROTOTYPES)
EXPORT m64p_error CALL CoreDoCommand(m64p_command, int, void *);
#endif

/* CoreOverrideVidExt()
 *
 * This function overrides the core's internal SDL-based OpenGL functions. This
 * override functionality allows a front-end to define its own video extension
 * functions to be used instead of the SDL functions. If any of the function
 * pointers in the structure are NULL, the override function will be disabled
 * and the core's internal SDL functions will be used.
 */
typedef m64p_error (*ptr_CoreOverrideVidExt)(m64p_video_extension_functions *);
#if defined(M64P_CORE_PROTOTYPES)
EXPORT m64p_error CALL CoreOverrideVidExt(m64p_video_extension_functions *);
#endif

/* CoreAddCheat()
 *
 * This function will add a Cheat Function to a list of currently active cheats
 * which are applied to the open ROM.
 */
typedef m64p_error (*ptr_CoreAddCheat)(const char *, m64p_cheat_code *, int);
#if defined(M64P_CORE_PROTOTYPES)
EXPORT m64p_error CALL CoreAddCheat(const char *, m64p_cheat_code *, int);
#endif

/* CoreCheatEnabled()
 *
 * This function will enable or disable a Cheat Function which is in the list of
 * currently active cheats.
 */
typedef m64p_error (*ptr_CoreCheatEnabled)(const char *, int);
#if defined(M64P_CORE_PROTOTYPES)
EXPORT m64p_error CALL CoreCheatEnabled(const char *, int);
#endif

/* CoreGetRomSettings()
 *
 * This function will retrieve the ROM settings from the mupen64plus INI file for
 * the ROM image corresponding to the given CRC values.
 */
typedef m64p_error (*ptr_CoreGetRomSettings)(m64p_rom_settings *, int, int, int);
#if defined(M64P_CORE_PROTOTYPES)
EXPORT m64p_error CALL CoreGetRomSettings(m64p_rom_settings *, int, int, int);
#endif

#ifdef __cplusplus
}
#endif

#endif /* #define M64P_FRONTEND_H */


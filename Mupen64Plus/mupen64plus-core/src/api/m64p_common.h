/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-core - m64p_common.h                                      *
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

/* This header file defines typedefs for function pointers to common Core
 * and plugin functions, for use by the front-end and plugin modules to attach
 * to the dynamic libraries.
 */

#if !defined(M64P_COMMON_H)
#define M64P_COMMON_H

#include "m64p_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* PluginGetVersion()
 *
 * This function retrieves version information from a library. This
 * function is the same for the core library and the plugins.
 */
typedef m64p_error (*ptr_PluginGetVersion)(m64p_plugin_type *, int *, int *, const char **, int *);
#if defined(M64P_PLUGIN_PROTOTYPES) || defined(M64P_CORE_PROTOTYPES)
EXPORT m64p_error CALL PluginGetVersion(m64p_plugin_type *, int *, int *, const char **, int *);
#endif

/* CoreGetAPIVersions()
 *
 * This function retrieves API version information from the core.
 */
typedef m64p_error (*ptr_CoreGetAPIVersions)(int *, int *, int *, int *);
#if defined(M64P_CORE_PROTOTYPES)
EXPORT m64p_error CALL CoreGetAPIVersions(int *, int *, int *, int *);
#endif

/* CoreErrorMessage()
 *
 * This function returns a pointer to a NULL-terminated string giving a
 * human-readable description of the error.
*/
typedef const char * (*ptr_CoreErrorMessage)(m64p_error);
#if defined(M64P_CORE_PROTOTYPES)
EXPORT const char * CALL CoreErrorMessage(m64p_error);
#endif

/* PluginStartup()
 *
 * This function initializes a plugin for use by allocating memory, creating
 * data structures, and loading the configuration data.
*/
typedef m64p_error (*ptr_PluginStartup)(m64p_dynlib_handle, void *, void (*)(void *, int, const char *));
#if defined(M64P_PLUGIN_PROTOTYPES) || defined(M64P_CORE_PROTOTYPES)
EXPORT m64p_error CALL PluginStartup(m64p_dynlib_handle, void *, void (*)(void *, int, const char *));
#endif

/* PluginShutdown()
 *
 * This function destroys data structures and releases memory allocated by
 * the plugin library.
*/
typedef m64p_error (*ptr_PluginShutdown)(void);
#if defined(M64P_PLUGIN_PROTOTYPES) || defined(M64P_CORE_PROTOTYPES)
EXPORT m64p_error CALL PluginShutdown(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* #define M64P_COMMON_H */


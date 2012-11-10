/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-core - m64p_config.h                                      *
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

/* This header file defines typedefs for function pointers to the Core's
 * configuration handling functions.
 */

#if !defined(M64P_CONFIG_H)
#define M64P_CONFIG_H

#include "m64p_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ConfigListSections()
 *
 * This function is called to enumerate the list of Sections in the Mupen64Plus
 * configuration file. It is expected that there will be a section named "Core"
 * for core-specific configuration data, "Graphics" for common graphics options,
 * and one or more sections for each plugin library. 
 */
typedef m64p_error (*ptr_ConfigListSections)(void *, void (*)(void *, const char *));
#if defined(M64P_CORE_PROTOTYPES)
EXPORT m64p_error CALL ConfigListSections(void *, void (*)(void *, const char *));
#endif

/* ConfigOpenSection()
 *
 * This function is used to give a configuration section handle to the front-end
 * which may be used to read or write configuration parameter values in a given
 * section of the configuration file.
 */
typedef m64p_error (*ptr_ConfigOpenSection)(const char *, m64p_handle *);
#if defined(M64P_CORE_PROTOTYPES)
EXPORT m64p_error CALL ConfigOpenSection(const char *, m64p_handle *);
#endif

/* ConfigListParameters()
 *
 * This function is called to enumerate the list of Parameters in a given
 * Section of the Mupen64Plus configuration file. 
 */
typedef m64p_error (*ptr_ConfigListParameters)(m64p_handle, void *, void (*)(void *, const char *, m64p_type));
#if defined(M64P_CORE_PROTOTYPES)
EXPORT m64p_error CALL ConfigListParameters(m64p_handle, void *, void (*)(void *, const char *, m64p_type));
#endif

/* ConfigSaveFile()
 *
 * This function saves the entire current Mupen64Plus configuration to disk.
 */
typedef m64p_error (*ptr_ConfigSaveFile)(void);
#if defined(M64P_CORE_PROTOTYPES)
EXPORT m64p_error CALL ConfigSaveFile(void);
#endif

/* ConfigSaveSection()
 *
 * This function saves one section of the current Mupen64Plus configuration to disk.
 */
typedef m64p_error (*ptr_ConfigSaveSection)(const char *);
#if defined(M64P_CORE_PROTOTYPES)
EXPORT m64p_error CALL ConfigSaveSection(const char *);
#endif

/* ConfigHasUnsavedChanges()
 *
 * This function determines if a given Section (or all sections) of the Mupen64Plus Core configuration file has been modified since it was last saved or loaded.
 */
typedef int (*ptr_ConfigHasUnsavedChanges)(const char *);
#if defined(M64P_CORE_PROTOTYPES)
EXPORT int CALL ConfigHasUnsavedChanges(const char *);
#endif

/* ConfigDeleteSection()
 *
 * This function deletes a section from the Mupen64Plus configuration data.
 */
typedef m64p_error (*ptr_ConfigDeleteSection)(const char *SectionName);
#if defined(M64P_CORE_PROTOTYPES)
EXPORT m64p_error CALL ConfigDeleteSection(const char *SectionName);
#endif

/* ConfigRevertChanges()
 *
 * This function reverts changes previously made to one section of the configuration file, so that it will match with the configuration at the last time that it was loaded from or saved to disk.
 */
typedef m64p_error (*ptr_ConfigRevertChanges)(const char *SectionName);
#if defined(M64P_CORE_PROTOTYPES)
EXPORT m64p_error CALL ConfigRevertChanges(const char *SectionName);
#endif

/* ConfigSetParameter()
 *
 * This function sets the value of one of the emulator's configuration
 * parameters.
 */
typedef m64p_error (*ptr_ConfigSetParameter)(m64p_handle, const char *, m64p_type, const void *);
#if defined(M64P_CORE_PROTOTYPES)
EXPORT m64p_error CALL ConfigSetParameter(m64p_handle, const char *, m64p_type, const void *);
#endif

/* ConfigGetParameter()
 *
 * This function retrieves the value of one of the emulator's parameters. 
 */
typedef m64p_error (*ptr_ConfigGetParameter)(m64p_handle, const char *, m64p_type, void *, int);
#if defined(M64P_CORE_PROTOTYPES)
EXPORT m64p_error CALL ConfigGetParameter(m64p_handle, const char *, m64p_type, void *, int);
#endif

/* ConfigGetParameterType()
 *
 * This function retrieves the type of one of the emulator's parameters. 
 */
typedef m64p_error (*ptr_ConfigGetParameterType)(m64p_handle, const char *, m64p_type *);
#if defined(M64P_CORE_PROTOTYPES)
EXPORT m64p_error CALL ConfigGetParameterType(m64p_handle, const char *, m64p_type *);
#endif

/* ConfigGetParameterHelp()
 *
 * This function retrieves the help information about one of the emulator's
 * parameters.
 */
typedef const char * (*ptr_ConfigGetParameterHelp)(m64p_handle, const char *);
#if defined(M64P_CORE_PROTOTYPES)
EXPORT const char * CALL ConfigGetParameterHelp(m64p_handle, const char *);
#endif

/* ConfigSetDefault***()
 *
 * These functions are used to set the value of a configuration parameter if it
 * is not already present in the configuration file. This may happen if a new
 * user runs the emulator, or an upgraded module uses a new parameter, or the
 * user deletes his or her configuration file. If the parameter is already
 * present in the given section of the configuration file, then no action will
 * be taken and this function will return successfully.
 */
typedef m64p_error (*ptr_ConfigSetDefaultInt)(m64p_handle, const char *, int, const char *);
typedef m64p_error (*ptr_ConfigSetDefaultFloat)(m64p_handle, const char *, float, const char *);
typedef m64p_error (*ptr_ConfigSetDefaultBool)(m64p_handle, const char *, int, const char *);
typedef m64p_error (*ptr_ConfigSetDefaultString)(m64p_handle, const char *, const char *, const char *);
#if defined(M64P_CORE_PROTOTYPES)
EXPORT m64p_error CALL ConfigSetDefaultInt(m64p_handle, const char *, int, const char *);
EXPORT m64p_error CALL ConfigSetDefaultFloat(m64p_handle, const char *, float, const char *);
EXPORT m64p_error CALL ConfigSetDefaultBool(m64p_handle, const char *, int, const char *);
EXPORT m64p_error CALL ConfigSetDefaultString(m64p_handle, const char *, const char *, const char *);
#endif

/* ConfigGetParam***()
 *
 * These functions retrieve the value of one of the emulator's parameters in
 * the given section, and return the value directly to the calling function. If
 * an errors occurs (such as an invalid Section handle, or invalid
 * configuration parameter name), then an error will be sent to the front-end
 * via the DebugCallback() function, and either a 0 (zero) or an empty string
 * will be returned.
 */
typedef int          (*ptr_ConfigGetParamInt)(m64p_handle, const char *);
typedef float        (*ptr_ConfigGetParamFloat)(m64p_handle, const char *);
typedef int          (*ptr_ConfigGetParamBool)(m64p_handle, const char *);
typedef const char * (*ptr_ConfigGetParamString)(m64p_handle, const char *);
#if defined(M64P_CORE_PROTOTYPES)
EXPORT int          CALL ConfigGetParamInt(m64p_handle, const char *);
EXPORT float        CALL ConfigGetParamFloat(m64p_handle, const char *);
EXPORT int          CALL ConfigGetParamBool(m64p_handle, const char *);
EXPORT const char * CALL ConfigGetParamString(m64p_handle, const char *);
#endif

/* ConfigGetSharedDataFilepath()
 *
 * This function is provided to allow a plugin to retrieve a full pathname to a
 * given shared data file. This type of file is intended to be shared among
 * multiple users on a system, so it is likely to be read-only.
 */
typedef const char * (*ptr_ConfigGetSharedDataFilepath)(const char *);
#if defined(M64P_CORE_PROTOTYPES)
EXPORT const char * CALL ConfigGetSharedDataFilepath(const char *);
#endif

/* ConfigGetUserConfigPath()
 *
 * This function may be used by the plugins or front-end to get a path to the
 * directory for storing user-specific configuration files. This will be the
 * directory where "mupen64plus.cfg" is located.
 */
typedef const char * (*ptr_ConfigGetUserConfigPath)(void);
#if defined(M64P_CORE_PROTOTYPES)
EXPORT const char * CALL ConfigGetUserConfigPath(void);
#endif

/* ConfigGetUserDataPath()
 *
 * This function may be used by the plugins or front-end to get a path to the
 * directory for storing user-specific data files. This may be used to store
 * files such as screenshots, saved game states, or hi-res textures.
 */
typedef const char * (*ptr_ConfigGetUserDataPath)(void);
#if defined(M64P_CORE_PROTOTYPES)
EXPORT const char * CALL ConfigGetUserDataPath(void);
#endif

/* ConfigGetUserCachePath()
 *
 * This function may be used by the plugins or front-end to get a path to the
 * directory for storing cached user-specific data files. Files in this
 * directory may be deleted by the user to save space, so critical information
 * should not be stored here.  This directory may be used to store files such
 * as the ROM browser cache.
 */
typedef const char * (*ptr_ConfigGetUserCachePath)(void);
#if defined(M64P_CORE_PROTOTYPES)
EXPORT const char * CALL ConfigGetUserCachePath(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* #define M64P_CONFIG_H */


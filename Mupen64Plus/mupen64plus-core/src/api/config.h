/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-core - api/config.h                                       *
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
                       
/* This file contains the Core configuration functions
 */

#include "m64p_types.h"

/* these functions are only to be used within the Core library */

m64p_error ConfigInit(const char *ConfigDirOverride, const char *DataDirOverride);
m64p_error ConfigShutdown(void);

/* these functions are exported outside of the Core as well as used within it */

EXPORT m64p_error CALL ConfigListSections(void *context, void (*SectionListCallback)(void * context, const char * SectionName));
EXPORT m64p_error CALL ConfigOpenSection(const char *SectionName, m64p_handle *ConfigSectionHandle);
EXPORT m64p_error CALL ConfigListParameters(m64p_handle ConfigSectionHandle, void *context, void (*ParameterListCallback)(void * context, const char *ParamName, m64p_type ParamType));
EXPORT m64p_error CALL ConfigSaveFile(void);

EXPORT m64p_error CALL ConfigSetParameter(m64p_handle ConfigSectionHandle, const char *ParamName, m64p_type ParamType, const void *ParamValue);
EXPORT m64p_error CALL ConfigGetParameter(m64p_handle ConfigSectionHandle, const char *ParamName, m64p_type ParamType, void *ParamValue, int MaxSize);
EXPORT const char * CALL ConfigGetParameterHelp(m64p_handle ConfigSectionHandle, const char *ParamName);

EXPORT m64p_error CALL ConfigSetDefaultInt(m64p_handle ConfigSectionHandle, const char *ParamName, int ParamValue, const char *ParamHelp);
EXPORT m64p_error CALL ConfigSetDefaultFloat(m64p_handle ConfigSectionHandle, const char *ParamName, float ParamValue, const char *ParamHelp);
EXPORT m64p_error CALL ConfigSetDefaultBool(m64p_handle ConfigSectionHandle, const char *ParamName, int ParamValue, const char *ParamHelp);
EXPORT m64p_error CALL ConfigSetDefaultString(m64p_handle ConfigSectionHandle, const char *ParamName, const char * ParamValue, const char *ParamHelp);

EXPORT int CALL ConfigGetParamInt(m64p_handle ConfigSectionHandle, const char *ParamName);
EXPORT float CALL ConfigGetParamFloat(m64p_handle ConfigSectionHandle, const char *ParamName);
EXPORT int CALL ConfigGetParamBool(m64p_handle ConfigSectionHandle, const char *ParamName);
EXPORT const char * CALL ConfigGetParamString(m64p_handle ConfigSectionHandle, const char *ParamName);

EXPORT const char * CALL ConfigGetSharedDataFilepath(const char *filename);
EXPORT const char * CALL ConfigGetUserConfigPath(void);
EXPORT const char * CALL ConfigGetUserDataPath(void);
EXPORT const char * CALL ConfigGetUserCachePath(void);


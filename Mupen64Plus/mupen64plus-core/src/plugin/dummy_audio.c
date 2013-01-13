/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - dummy_audio.c                                           *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2008-2009 Richard Goedeken                              *
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

#include <stdlib.h>

#include "api/m64p_types.h"
#include "plugin.h"
#include "dummy_audio.h"

m64p_error dummyaudio_PluginGetVersion(m64p_plugin_type *PluginType, int *PluginVersion,
                                       int *APIVersion, const char **PluginNamePtr, int *Capabilities)
{
    if (PluginType != NULL)
        *PluginType = M64PLUGIN_AUDIO;

    if (PluginVersion != NULL)
        *PluginVersion = 0x00010000;

    if (APIVersion != NULL)
        *APIVersion = AUDIO_API_VERSION;

    if (PluginNamePtr != NULL)
        *PluginNamePtr = "Mupen64Plus-NoAudio";

    if (Capabilities != NULL)
        *Capabilities = 0;

    return M64ERR_SUCCESS;
}

void dummyaudio_AiDacrateChanged(int SystemType)
{
    return;
}

void dummyaudio_AiLenChanged(void)
{
    return;
}

int dummyaudio_InitiateAudio(AUDIO_INFO Audio_Info)
{
    return 1;
}

int dummyaudio_RomOpen(void)
{
    return 1;
}

void dummyaudio_RomClosed(void)
{
    return;
}

void dummyaudio_ProcessAList(void)
{
    return;
}

void dummyaudio_SetSpeedFactor(int percent)
{
    return;
}

void dummyaudio_VolumeUp(void)
{
    return;
}

void dummyaudio_VolumeDown(void)
{
    return;
}

int dummyaudio_VolumeGetLevel(void)
{
    return 0;
}

void dummyaudio_VolumeSetLevel(int level)
{
    return;
}

void dummyaudio_VolumeMute(void)
{
    return;
}

const char *dummyaudio_VolumeGetString(void)
{
    return "disabled";
}

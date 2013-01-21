/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - dummy_video.c                                           *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2008 John Chadwick (NMN)                                *
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

#include <stdlib.h>

#include "api/m64p_types.h"
#include "plugin.h"
#include "dummy_video.h"

m64p_error dummyvideo_PluginGetVersion(m64p_plugin_type *PluginType, int *PluginVersion,
                                       int *APIVersion, const char **PluginNamePtr, int *Capabilities)
{
    if (PluginType != NULL)
        *PluginType = M64PLUGIN_GFX;

    if (PluginVersion != NULL)
        *PluginVersion = 0x00010000;

    if (APIVersion != NULL)
        *APIVersion = GFX_API_VERSION;

    if (PluginNamePtr != NULL)
        *PluginNamePtr = "Mupen64Plus-NoVideo";

    if (Capabilities != NULL)
        *Capabilities = 0;

    return M64ERR_SUCCESS;
}

void dummyvideo_ChangeWindow (void)
{

}

int dummyvideo_InitiateGFX (GFX_INFO Gfx_Info)
{
    return 1;
}

void dummyvideo_MoveScreen (int xpos, int ypos)
{
}

void dummyvideo_ProcessDList(void)
{

}

void dummyvideo_ProcessRDPList(void)
{

}

void dummyvideo_RomClosed (void)
{

}

int dummyvideo_RomOpen (void)
{
    return 1;
}

void dummyvideo_ShowCFB (void)
{

}

void dummyvideo_UpdateScreen (void)
{

}

void dummyvideo_ViStatusChanged (void)
{

}

void dummyvideo_ViWidthChanged (void)
{

}

void dummyvideo_ReadScreen2 (void *dest, int *width, int *height, int front)
{

}

void dummyvideo_SetRenderingCallback(void (*callback)(int))
{

}

void dummyvideo_FBRead(unsigned int addr)
{
}

void dummyvideo_FBWrite(unsigned int addr, unsigned int size)
{
}

void dummyvideo_FBGetFrameBufferInfo(void *p)
{
}


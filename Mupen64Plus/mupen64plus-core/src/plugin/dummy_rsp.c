/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - dummy_rsp.c                                             *
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

#include <stdlib.h>

#include "api/m64p_types.h"
#include "plugin.h"
#include "dummy_rsp.h"

m64p_error dummyrsp_PluginGetVersion(m64p_plugin_type *PluginType, int *PluginVersion,
                                     int *APIVersion, const char **PluginNamePtr, int *Capabilities)
{
    if (PluginType != NULL)
        *PluginType = M64PLUGIN_RSP;

    if (PluginVersion != NULL)
        *PluginVersion = 0x00010000;

    if (APIVersion != NULL)
        *APIVersion = RSP_API_VERSION;

    if (PluginNamePtr != NULL)
        *PluginNamePtr = "Mupen64Plus-NoRSP";

    if (Capabilities != NULL)
        *Capabilities = 0;

    return M64ERR_SUCCESS;
}

unsigned int dummyrsp_DoRspCycles(unsigned int Cycles)
{
    return Cycles;
}

void dummyrsp_InitiateRSP(RSP_INFO Rsp_Info, unsigned int * CycleCount)
{
}

void dummyrsp_RomClosed(void)
{
}


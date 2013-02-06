/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - dummy_rsp.h                                             *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2009 Richard42                                          *
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

#if !defined(DUMMY_RSP_H)
#define DUMMY_RSP_H

#include "api/m64p_plugin.h"

extern m64p_error dummyrsp_PluginGetVersion(m64p_plugin_type *PluginType, int *PluginVersion,
                                            int *APIVersion, const char **PluginNamePtr, int *Capabilities);
extern unsigned int dummyrsp_DoRspCycles(unsigned int Cycles);
extern void dummyrsp_InitiateRSP(RSP_INFO Rsp_Info, unsigned int *CycleCount);
extern void dummyrsp_RomClosed(void);

#endif /* DUMMY_RSP_H */



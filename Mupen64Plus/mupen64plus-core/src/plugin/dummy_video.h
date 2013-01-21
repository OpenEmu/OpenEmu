/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - dummy_video.h                                           *
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

#if !defined(DUMMY_VIDEO_H)
#define DUMMY_VIDEO_H

#include "api/m64p_plugin.h"

extern m64p_error dummyvideo_PluginGetVersion(m64p_plugin_type *PluginType, int *PluginVersion,
                                              int *APIVersion, const char **PluginNamePtr, int *Capabilities);
extern void dummyvideo_ChangeWindow(void);
extern int dummyvideo_InitiateGFX(GFX_INFO Gfx_Info);
extern void dummyvideo_MoveScreen(int xpos, int ypos);
extern void dummyvideo_ProcessDList(void);
extern void dummyvideo_ProcessRDPList(void);
extern void dummyvideo_RomClosed(void);
extern int  dummyvideo_RomOpen(void);
extern void dummyvideo_ShowCFB(void);
extern void dummyvideo_UpdateScreen(void);
extern void dummyvideo_ViStatusChanged(void);
extern void dummyvideo_ViWidthChanged(void);
extern void dummyvideo_ReadScreen2(void *dest, int *width, int *height, int front);
extern void dummyvideo_SetRenderingCallback(void (*callback)(int));

extern void dummyvideo_FBRead(unsigned int addr);
extern void dummyvideo_FBWrite(unsigned int addr, unsigned int size);
extern void dummyvideo_FBGetFrameBufferInfo(void *p);

#endif /* DUMMY_VIDEO_H */



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - plugin.h                                                *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2002 Hacktarux                                          *
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

#ifndef PLUGIN_H
#define PLUGIN_H

#include "api/m64p_common.h"
#include "api/m64p_plugin.h"

extern m64p_error plugin_connect(m64p_plugin_type, m64p_dynlib_handle plugin_handle);
extern m64p_error plugin_start(m64p_plugin_type);
extern m64p_error plugin_check(void);

extern CONTROL Controls[4];

/*** Version requirement information ***/
#define RSP_API_VERSION   0x20000
#define GFX_API_VERSION   0x20100
#define AUDIO_API_VERSION 0x20000
#define INPUT_API_VERSION 0x20000

/* video plugin function pointers */
typedef struct _gfx_plugin_functions
{
	ptr_PluginGetVersion getVersion;
	ptr_ChangeWindow     changeWindow;
	ptr_InitiateGFX      initiateGFX;
	ptr_MoveScreen       moveScreen;
	ptr_ProcessDList     processDList;
	ptr_ProcessRDPList   processRDPList;
	ptr_RomClosed        romClosed;
	ptr_RomOpen          romOpen;
	ptr_ShowCFB          showCFB;
	ptr_UpdateScreen     updateScreen;
	ptr_ViStatusChanged  viStatusChanged;
	ptr_ViWidthChanged   viWidthChanged;
	ptr_ReadScreen2      readScreen;
	ptr_SetRenderingCallback setRenderingCallback;

	/* frame buffer plugin spec extension */
	ptr_FBRead          fBRead;
	ptr_FBWrite         fBWrite;
	ptr_FBGetFrameBufferInfo fBGetFrameBufferInfo;
} gfx_plugin_functions;

extern gfx_plugin_functions gfx;

/* audio plugin function pointers */
typedef struct _audio_plugin_functions
{
	ptr_PluginGetVersion  getVersion;
	ptr_AiDacrateChanged  aiDacrateChanged;
	ptr_AiLenChanged      aiLenChanged;
	ptr_InitiateAudio     initiateAudio;
	ptr_ProcessAList      processAList;
	ptr_RomClosed         romClosed;
	ptr_RomOpen           romOpen;
	ptr_SetSpeedFactor    setSpeedFactor;
	ptr_VolumeUp          volumeUp;
	ptr_VolumeDown        volumeDown;
	ptr_VolumeGetLevel    volumeGetLevel;
	ptr_VolumeSetLevel    volumeSetLevel;
	ptr_VolumeMute        volumeMute;
	ptr_VolumeGetString   volumeGetString;
} audio_plugin_functions;

extern audio_plugin_functions audio;

/* input plugin function pointers */
typedef struct _input_plugin_functions
{
	ptr_PluginGetVersion    getVersion;
	ptr_ControllerCommand   controllerCommand;
	ptr_GetKeys             getKeys;
	ptr_InitiateControllers initiateControllers;
	ptr_ReadController      readController;
	ptr_RomClosed           romClosed;
	ptr_RomOpen             romOpen;
	ptr_SDL_KeyDown         keyDown;
	ptr_SDL_KeyUp           keyUp;
} input_plugin_functions;

extern input_plugin_functions input;

/* RSP plugin function pointers */
typedef struct _rsp_plugin_functions
{
	ptr_PluginGetVersion    getVersion;
	ptr_DoRspCycles         doRspCycles;
	ptr_InitiateRSP         initiateRSP;
	ptr_RomClosed           romClosed;
} rsp_plugin_functions;

extern rsp_plugin_functions rsp;

#endif


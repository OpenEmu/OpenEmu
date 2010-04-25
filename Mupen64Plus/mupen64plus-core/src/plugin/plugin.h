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

#include "api/m64p_types.h"
#include "api/m64p_plugin.h"

extern m64p_error plugin_connect(m64p_plugin_type, m64p_dynlib_handle plugin_handle);
extern m64p_error plugin_start(m64p_plugin_type);
extern m64p_error plugin_check(void);

extern CONTROL Controls[4];

/*** Version requirement information ***/
#define MINIMUM_RSP_API_VERSION   0x10000
#define MINIMUM_GFX_API_VERSION   0x10000
#define MINIMUM_AUDIO_API_VERSION 0x10000
#define MINIMUM_INPUT_API_VERSION 0x10000

/* video plugin function pointers */
extern ptr_ChangeWindow    changeWindow;
extern ptr_InitiateGFX     initiateGFX;
extern ptr_MoveScreen      moveScreen;
extern ptr_ProcessDList    processDList;
extern ptr_ProcessRDPList  processRDPList;
extern ptr_RomClosed       romClosed_gfx;
extern ptr_RomOpen         romOpen_gfx;
extern ptr_ShowCFB         showCFB;
extern ptr_UpdateScreen    updateScreen;
extern ptr_ViStatusChanged viStatusChanged;
extern ptr_ViWidthChanged  viWidthChanged;
extern ptr_ReadScreen2     readScreen;
extern ptr_SetRenderingCallback setRenderingCallback;

/* frame buffer plugin spec extension */
extern ptr_FBRead          fBRead;
extern ptr_FBWrite         fBWrite;
extern ptr_FBGetFrameBufferInfo fBGetFrameBufferInfo;

/* audio plugin function pointers */
extern ptr_AiDacrateChanged  aiDacrateChanged;
extern ptr_AiLenChanged      aiLenChanged;
extern ptr_InitiateAudio     initiateAudio;
extern ptr_ProcessAList      processAList;
extern ptr_RomClosed         romClosed_audio;
extern ptr_RomOpen           romOpen_audio;
extern ptr_SetSpeedFactor    setSpeedFactor;
extern ptr_VolumeUp          volumeUp;
extern ptr_VolumeDown        volumeDown;
extern ptr_VolumeGetLevel    volumeGetLevel;
extern ptr_VolumeSetLevel    volumeSetLevel;
extern ptr_VolumeMute        volumeMute;
extern ptr_VolumeGetString   volumeGetString;

/* input plugin function pointers */
extern ptr_ControllerCommand   controllerCommand;
extern ptr_GetKeys             getKeys;
extern ptr_InitiateControllers initiateControllers;
extern ptr_ReadController      readController;
extern ptr_RomClosed           romClosed_input;
extern ptr_RomOpen             romOpen_input;
extern ptr_SDL_KeyDown         keyDown;
extern ptr_SDL_KeyUp           keyUp;

/* RSP plugin function pointers */
extern ptr_DoRspCycles         doRspCycles;
extern ptr_InitiateRSP         initiateRSP;
extern ptr_RomClosed           romClosed_RSP;

#endif


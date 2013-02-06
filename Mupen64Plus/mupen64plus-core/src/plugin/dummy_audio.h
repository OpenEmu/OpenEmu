/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - dummy_audio.h                                           *
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

#if !defined(DUMMY_AUDIO_H)
#define DUMMY_AUDIO_H

#include "api/m64p_plugin.h"

extern m64p_error dummyaudio_PluginGetVersion(m64p_plugin_type *PluginType, int *PluginVersion,
                                              int *APIVersion, const char **PluginNamePtr, int *Capabilities);
extern void dummyaudio_AiDacrateChanged(int SystemType);
extern void dummyaudio_AiLenChanged(void);
extern int  dummyaudio_InitiateAudio(AUDIO_INFO Audio_Info);
extern void dummyaudio_ProcessAList(void);
extern int  dummyaudio_RomOpen(void);
extern void dummyaudio_RomClosed(void);
extern void dummyaudio_SetSpeedFactor(int percent);
extern void dummyaudio_VolumeUp(void);
extern void dummyaudio_VolumeDown(void);
extern int dummyaudio_VolumeGetLevel(void);
extern void dummyaudio_VolumeSetLevel(int level);
extern void dummyaudio_VolumeMute(void);
extern const char * dummyaudio_VolumeGetString(void);

#endif /* DUMMY_AUDIO_H */



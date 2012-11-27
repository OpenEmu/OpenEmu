/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BACKENDS_MIXER_WINCE_SDL_H
#define BACKENDS_MIXER_WINCE_SDL_H

#include "backends/mixer/sdl/sdl-mixer.h"

/**
 * SDL mixer manager for WinCE
 */
class WINCESdlMixerManager : public SdlMixerManager {
public:
	WINCESdlMixerManager();
	virtual ~WINCESdlMixerManager();

	virtual void init();

private:

#ifdef USE_VORBIS
	bool checkOggHighSampleRate();
#endif

	static void private_sound_proc(void *param, byte *buf, int len);
	uint32 compute_sample_rate();

};

#endif

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

#ifdef __SYMBIAN32__

#include "backends/mixer/symbiansdl/symbiansdl-mixer.h"
#include "common/system.h"

#ifdef SAMPLES_PER_SEC_8000 // the GreanSymbianMMP format cannot handle values for defines :(
  #define SAMPLES_PER_SEC 8000
#else
  #define SAMPLES_PER_SEC 16000
#endif

SymbianSdlMixerManager::SymbianSdlMixerManager()
	:
	_stereoMixBuffer(0) {

}

SymbianSdlMixerManager::~SymbianSdlMixerManager() {
	delete[] _stereoMixBuffer;
}

void SymbianSdlMixerManager::startAudio() {
	// Need to create mixbuffer for stereo mix to downmix
	if (_obtained.channels != 2) {
		_stereoMixBuffer = new byte [_obtained.size * 2]; // * 2 for stereo values
	}

	SdlMixerManager::startAudio();
}

void SymbianSdlMixerManager::callbackHandler(byte *samples, int len) {
	assert(_mixer);
#if defined(S60) && !defined(S60V3)
	// If not stereo then we need to downmix
	if (_obtained.channels != 2) {
		_mixer->mixCallback(_stereoMixBuffer, len * 2);

		int16 *bitmixDst = (int16 *)samples;
		int16 *bitmixSrc = (int16 *)_stereoMixBuffer;

		for (int loop = len / 2; loop >= 0; loop --) {
			*bitmixDst = (*bitmixSrc + *(bitmixSrc + 1)) >> 1;
			bitmixDst++;
			bitmixSrc += 2;
		}
	} else
#else
	_mixer->mixCallback(samples, len);
#endif
}

#endif

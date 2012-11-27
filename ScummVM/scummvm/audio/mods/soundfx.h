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

/**
 * @file
 * Sound decoder used in engines:
 *  - cine
 */

#ifndef AUDIO_MODS_SOUNDFX_H
#define AUDIO_MODS_SOUNDFX_H

namespace Common {
class SeekableReadStream;
}

namespace Audio {

class AudioStream;

typedef byte *(*LoadSoundFxInstrumentCallback)(const char *name, uint32 *size);

/*
 * Factory function for SoundFX modules. Reads all data from the
 * given data stream and creates an AudioStream from this (no references to the
 * stream object is kept). If loadCb is non 0, then instruments are loaded using
 * it, buffers returned are free'd at the end of playback.
 */
AudioStream *makeSoundFxStream(Common::SeekableReadStream *data, LoadSoundFxInstrumentCallback loadCb, int rate = 44100, bool stereo = true);

} // End of namespace Audio

#endif

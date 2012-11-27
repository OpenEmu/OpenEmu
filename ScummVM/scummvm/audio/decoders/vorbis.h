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
 *  - agos
 *  - draci
 *  - kyra
 *  - queen
 *  - saga
 *  - sci
 *  - scumm
 *  - sword1
 *  - sword2
 *  - sword25
 *  - touche
 *  - tucker
 */

#ifndef AUDIO_VORBIS_H
#define AUDIO_VORBIS_H

#include "common/scummsys.h"
#include "common/types.h"

#ifdef USE_VORBIS

namespace Common {
class SeekableReadStream;
}

namespace Audio {

class SeekableAudioStream;

/**
 * Create a new SeekableAudioStream from the Ogg Vorbis data in the given stream.
 * Allows for seeking (which is why we require a SeekableReadStream).
 *
 * @param stream			the SeekableReadStream from which to read the Ogg Vorbis data
 * @param disposeAfterUse	whether to delete the stream after use
 * @return	a new SeekableAudioStream, or NULL, if an error occurred
 */
SeekableAudioStream *makeVorbisStream(
	Common::SeekableReadStream *stream,
	DisposeAfterUse::Flag disposeAfterUse);

} // End of namespace Audio

#endif // #ifdef USE_VORBIS
#endif // #ifndef AUDIO_VORBIS_H

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
 *  - touche
 *  - tucker
 */

#ifndef AUDIO_FLAC_H
#define AUDIO_FLAC_H

#include "common/scummsys.h"
#include "common/types.h"

#ifdef USE_FLAC

namespace Common {
class SeekableReadStream;
}

namespace Audio {

class SeekableAudioStream;

/**
 * Create a new SeekableAudioStream from the FLAC data in the given stream.
 * Allows for seeking (which is why we require a SeekableReadStream).
 *
 * @param stream            the SeekableReadStream from which to read the FLAC data
 * @param disposeAfterUse   whether to delete the stream after use
 * @return   a new SeekableAudioStream, or NULL, if an error occurred
 */
SeekableAudioStream *makeFLACStream(
	Common::SeekableReadStream *stream,
	DisposeAfterUse::Flag disposeAfterUse);

} // End of namespace Audio

#endif // #ifdef USE_FLAC
#endif // #ifndef AUDIO_FLAC_H

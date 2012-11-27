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
 *
 */

#ifndef TOLTECS_MOVIE_H
#define TOLTECS_MOVIE_H

#include "audio/audiostream.h"
#include "audio/mixer.h"	// for Audio::SoundHandle

namespace Toltecs {

class MoviePlayer {

public:
	MoviePlayer(ToltecsEngine *vm);
	~MoviePlayer();

	void playMovie(uint resIndex);

protected:
	ToltecsEngine *_vm;
	Audio::QueuingAudioStream *_audioStream;
	Audio::SoundHandle _audioStreamHandle;

	uint32 _chunkCount, _frameCount, _lastPrefetchOfs;
	uint32 _soundChunkFramesLeft, _framesPerSoundChunk;

	void unpackPalette(byte *source, byte *dest, int elemCount, int elemSize);
	void unpackRle(byte *source, byte *dest);

	void fetchAudioChunks();

	bool handleInput();

};

} // End of namespace Toltecs

#endif /* TOLTECS_MOVIE_H */

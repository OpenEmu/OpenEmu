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

#ifndef LASTEXPRESS_SND_H
#define LASTEXPRESS_SND_H

/*
	Sound format (.SND / .LNK)

	uint32 {4}   - data size
	uint16 {2}   - number of blocks

	// for each block
	    int16 {2}    - initial sample
	    byte {1}     - initial index
	    byte {1}     - unused (00)
	    byte {x}     - IMA ADPCM sample codes
*/

#include "audio/mixer.h"

namespace Audio {
class AudioStream;
class QueuingAudioStream;
}

namespace Common {
class SeekableReadStream;
}

namespace LastExpress {

class SimpleSound {
public:
	SimpleSound();
	virtual ~SimpleSound();

	void stop() const;
	virtual bool isFinished() = 0;

protected:
	void loadHeader(Common::SeekableReadStream *in);
	Audio::AudioStream *makeDecoder(Common::SeekableReadStream *in, uint32 size, int32 filterId = -1) const;
	void play(Audio::AudioStream *as);

	uint32 _size;   ///< data size
	                ///<  - NIS: size of all blocks, including those located in the matching LNK file
	                ///<  - LNK: size of the LNK file itself, including the header
	                ///<  - SND: size of all blocks
	uint16 _blocks; ///< number of blocks
	uint32 _blockSize;
	Audio::SoundHandle _handle;
};

class StreamedSound : public SimpleSound {
public:
	StreamedSound();
	~StreamedSound();

	bool load(Common::SeekableReadStream *stream, int32 filterId = -1);
	virtual bool isFinished();

	void setFilterId(int32 filterId);

private:
	Audio::AudioStream *_as;
	bool _loaded;
};

class AppendableSound : public SimpleSound {
public:
	AppendableSound();
	~AppendableSound();

	void queueBuffer(const byte *data, uint32 size);
	void queueBuffer(Common::SeekableReadStream *bufferIn);
	void finish();

	virtual bool isFinished();

private:
	Audio::QueuingAudioStream *_as;
	bool _finished;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_SND_H

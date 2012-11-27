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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_BASE_SOUNDBUFFER_H
#define WINTERMUTE_BASE_SOUNDBUFFER_H


#include "engines/wintermute/base/base.h"
#include "audio/mixer.h"
#include "common/stream.h"

namespace Audio {
class SeekableAudioStream;
class SoundHandle;
}

namespace Wintermute {

class BaseFile;
class BaseSoundBuffer : public BaseClass {
public:

	BaseSoundBuffer(BaseGame *inGame);
	virtual ~BaseSoundBuffer();

	bool pause();
	bool play(bool looping = false, uint32 startSample = 0);
	bool resume();
	bool stop();
	bool isPlaying();

	void setLooping(bool looping);

	uint32 getPosition();
	bool setPosition(uint32 pos);
	uint32 getLength();

	bool setLoopStart(uint32 pos);
	uint32 getLoopStart() const {
		return _loopStart;
	}

	bool setPan(float pan);
	bool setPrivateVolume(int colume);
	bool setVolume(int colume);
	void updateVolume();

	void setType(Audio::Mixer::SoundType Type);

	bool loadFromFile(const Common::String &filename, bool forceReload = false);
	void setStreaming(bool streamed, uint32 numBlocks = 0, uint32 blockSize = 0);
	bool applyFX(TSFXType type, float param1, float param2, float param3, float param4);

	//HSTREAM _stream;
	//HSYNC _sync;
	Audio::SeekableAudioStream *_stream;
	Audio::SoundHandle *_handle;

	bool _freezePaused;
	uint32 _loopStart;
	Audio::Mixer::SoundType _type;
	bool _looping;

	int _privateVolume;
private:
	uint32 _startPos;
	Common::String _filename;
	bool _streamed;
	Common::SeekableReadStream *_file;
	int _volume;
};

} // end of namespace Wintermute

#endif

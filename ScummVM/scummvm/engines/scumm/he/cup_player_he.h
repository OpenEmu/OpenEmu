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


#if !defined(SCUMM_HE_CUP_PLAYER_HE_H) && defined(ENABLE_HE)
#define SCUMM_HE_CUP_PLAYER_HE_H

#include "common/stream.h"

namespace Scumm {

struct CUP_Sfx {
	int16 num;
	uint32 flags;
};

struct CUP_SfxChannel {
	Audio::SoundHandle handle;
	int16 sfxNum;
	uint32 flags;
};

class CUP_Player {
public:

	enum {
		kSfxFlagLoop    = 1 << 0,
		kSfxFlagRestart = 1 << 1
	};

	enum {
		kSfxChannels = 8,
		kSfxQueueSize = 16,
		kDefaultPlaybackRate = 66,
		kDefaultVideoWidth = 640,
		kDefaultVideoHeight = 480
	};

	CUP_Player(OSystem *sys, ScummEngine_vCUPhe *vm, Audio::Mixer *mixer);

	bool open(const char *filename);
	void close();
	void play();
	void copyRectToScreen(const Common::Rect &r);
	void updateScreen();
	void updateSfx();
	void waitForSfxChannel(int channel);
	bool parseNextHeaderTag(Common::SeekableReadStream &dataStream);
	bool parseNextBlockTag(Common::SeekableReadStream &dataStream);
	void handleHEAD(Common::SeekableReadStream &dataStream, uint32 dataSize);
	void handleSFXB(Common::SeekableReadStream &dataStream, uint32 dataSize);
	void handleRGBS(Common::SeekableReadStream &dataStream, uint32 dataSize);
	void handleFRAM(Common::SeekableReadStream &dataStream, uint32 dataSize);
	void handleSRLE(Common::SeekableReadStream &dataStream, uint32 dataSize);
	bool handleLZSS(Common::SeekableReadStream &dataStream, uint32 dataSize);
	void handleRATE(Common::SeekableReadStream &dataStream, uint32 dataSize);
	void handleSNDE(Common::SeekableReadStream &dataStream, uint32 dataSize);
	void handleTOIL(Common::SeekableReadStream &dataStream, uint32 dataSize);

protected:

	ScummEngine_vCUPhe *_vm;
	Audio::Mixer *_mixer;
	OSystem *_system;

	Common::File _fileStream;

	int _playbackRate;
	int _width, _height;

	uint8 _paletteData[256 * 3];
	bool _paletteChanged;
	uint8 *_offscreenBuffer;

	uint8 *_inLzssBufData;
	uint32 _inLzssBufSize;
	uint8 *_outLzssBufData;
	uint32 _outLzssBufSize;

	uint32 _dataSize;

	int _sfxCount;
	uint8 *_sfxBuffer;
	CUP_SfxChannel _sfxChannels[kSfxChannels];
	CUP_Sfx _sfxQueue[kSfxQueueSize];
	int _sfxQueuePos;
	int _lastSfxChannel;
};

} // End of namespace Scumm

#endif

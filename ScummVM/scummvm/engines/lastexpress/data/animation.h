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

#ifndef LASTEXPRESS_ANIMATION_H
#define LASTEXPRESS_ANIMATION_H

/*
	Animation format (.NIS)

	uint32 {4}    - Number of chunks

	// for each chunk
	    uint16 {2}    - Type
	    uint16 {2}    - Tag
	    uint32 {4}    - Size of chunk
	    byte {x}      - Data (for "data" chunks: backgrounds, overlay & audio data)
*/

#include "lastexpress/drawable.h"

#include "common/array.h"

namespace Common {
class SeekableReadStream;
}

namespace LastExpress {

class AnimFrame;
class AppendableSound;

class Animation : public Drawable {
public:
	enum FlagType {
		kFlagDefault = 16384,
		kFlagProcess = 49152
	};

	Animation();
	~Animation();

	bool load(Common::SeekableReadStream *stream, int flag = kFlagDefault);
	bool process();
	bool hasEnded();
	Common::Rect draw(Graphics::Surface *surface);
	void play();

private:
	static const uint32 _soundBlockSize = 739;

	// despite their size field, info chunks don't have a payload
	enum ChunkType {
		kChunkTypeNone              = 0,
		kChunkTypeUnknown1          = 1,
		kChunkTypeUnknown2          = 2,
		kChunkTypeAudioInfo         = 3,
		kChunkTypeUnknown4          = 4,
		kChunkTypeUnknown5          = 5,
		kChunkTypeBackground1       = 10,
		kChunkTypeSelectBackground1 = 11,
		kChunkTypeBackground2       = 12,
		kChunkTypeSelectBackground2 = 13,
		kChunkTypeOverlay           = 20,
		kChunkTypeUpdate            = 21,
		kChunkTypeUpdateTransition  = 22,
		kChunkTypeSound1            = 30,
		kChunkTypeSound2            = 31,
		kChunkTypeAudioData         = 32,
		kChunkTypeAudioEnd          = 99
	};

	struct Chunk {
		ChunkType type;
		uint16 frame;
		uint32 size;

		Chunk() {
			type = kChunkTypeNone;
			frame = 0;
			size = 0;
		}
	};

	void reset();
	AnimFrame *processChunkFrame(Common::SeekableReadStream *in, const Chunk &c) const;
	void processChunkAudio(Common::SeekableReadStream *in, const Chunk &c);

	Common::SeekableReadStream *_stream;
	Common::Array<Chunk> _chunks;
	Common::Array<Chunk>::iterator _currentChunk;
	AnimFrame *_overlay, *_background1, *_background2;
	byte _backgroundCurrent;
	AppendableSound *_audio;

	uint32 _startTime;
	bool _changed;
	int _flag;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_ANIMATION_H

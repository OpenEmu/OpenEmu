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

#ifndef KYRA_VQA_H
#define KYRA_VQA_H

#include "common/scummsys.h"

#include "audio/mixer.h"

class OSystem;

namespace Audio {
class QueuingAudioStream;
} // End of namespace Audio

namespace Common {
class SeekableReadStream;
} // End of namespace Common

namespace Kyra {

class KyraEngine_v1;
class Screen;

class VQAMovie {
public:
	VQAMovie(KyraEngine_v1 *vm, OSystem *system);
	~VQAMovie();

	bool opened() { return _opened; }
	int frames() { return _opened ? _header.numFrames : -1; }

	// It's unlikely that we ever want to change the movie position from
	// its default.

	void setDrawPage(int page) { _drawPage = page; }

	bool open(const char *filename);
	void close();
	void play();

protected:
	OSystem *_system;
	KyraEngine_v1 *_vm;
	Screen *_screen;

	bool _opened;
	int _x, _y;
	int _drawPage;

	struct VQAHeader {
		uint16 version;
		uint16 flags;
		uint16 numFrames;
		uint16 width;
		uint16 height;
		uint8 blockW;
		uint8 blockH;
		uint8 frameRate;
		uint8 cbParts;
		uint16 colors;
		uint16 maxBlocks;
		uint32 unk1;
		uint16 unk2;
		uint16 freq;
		uint8 channels;
		uint8 bits;
		uint32 unk3;
		uint16 unk4;
		uint32 maxCBFZSize;
		uint32 unk5;
	};

	struct Buffer {
		uint8 *data;
		uint32 size;
	};

	Buffer _buffers[2];

	void initBuffers();
	void *allocBuffer(int num, uint32 size);
	void freeBuffers();

	void decodeSND1(byte *inbuf, uint32 insize, byte *outbuf, uint32 outsize);

	void displayFrame(uint frameNum);

	Common::SeekableReadStream *_file;

	VQAHeader _header;
	uint32 *_frameInfo;
	uint32 _codeBookSize;
	byte *_codeBook;
	byte *_partialCodeBook;
	bool _compressedCodeBook;
	int _partialCodeBookSize;
	int _numPartialCodeBooks;
	uint32 _numVectorPointers;
	uint16 *_vectorPointers;

	byte *_frame;

	Audio::QueuingAudioStream *_stream;
	Audio::SoundHandle _sound;

	uint32 readTag();
};

} // End of namespace Kyra

#endif

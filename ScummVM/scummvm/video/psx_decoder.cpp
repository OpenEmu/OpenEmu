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

// PlayStation Stream demuxer and XA audio decoder based on FFmpeg/libav
// MDEC video emulation based on http://kenai.com/downloads/jpsxdec/Old/PlayStation1_STR_format1-00.txt

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"
#include "common/bitstream.h"
#include "common/huffman.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/yuv_to_rgb.h"

#include "video/psx_decoder.h"

namespace Video {

// Here are the codes/lengths/symbols that are used for decoding
// DC coefficients (version 3 frames only)

#define DC_CODE_COUNT 9
#define DC_HUFF_VAL(b, n, p) (((b) << 16) | ((n) << 8) | (p))
#define GET_DC_BITS(x) ((x) >> 16)
#define GET_DC_NEG(x) ((int)(((x) >> 8) & 0xff))
#define GET_DC_POS(x) ((int)((x) & 0xff))

static const uint32 s_huffmanDCChromaCodes[DC_CODE_COUNT] = {
	254, 126, 62, 30, 14, 6, 2, 1, 0
};

static const byte s_huffmanDCChromaLengths[DC_CODE_COUNT] = {
	8, 7, 6, 5, 4, 3, 2, 2, 2
};

static const uint32 s_huffmanDCLumaCodes[DC_CODE_COUNT] = {
	126, 62, 30, 14, 6, 5, 1, 0, 4
};

static const byte s_huffmanDCLumaLengths[DC_CODE_COUNT] = {
	7, 6, 5, 4, 3, 3, 2, 2, 3
};

static const uint32 s_huffmanDCSymbols[DC_CODE_COUNT] = {
	DC_HUFF_VAL(8, 255, 128), DC_HUFF_VAL(7, 127, 64), DC_HUFF_VAL(6, 63, 32),
	DC_HUFF_VAL(5, 31, 16), DC_HUFF_VAL(4, 15, 8), DC_HUFF_VAL(3, 7, 4),
	DC_HUFF_VAL(2, 3, 2), DC_HUFF_VAL(1, 1, 1), DC_HUFF_VAL(0, 0, 0)
};

// Here are the codes/lengths/symbols that are used for decoding
// DC coefficients (version 2 and 3 frames)

#define AC_CODE_COUNT 113
#define AC_HUFF_VAL(z, a) ((z << 8) | a)
#define ESCAPE_CODE  ((uint32)-1) // arbitrary, just so we can tell what code it is
#define END_OF_BLOCK ((uint32)-2) // arbitrary, just so we can tell what code it is
#define GET_AC_ZERO_RUN(code) (code >> 8)
#define GET_AC_COEFFICIENT(code) ((int)(code & 0xff))

static const uint32 s_huffmanACCodes[AC_CODE_COUNT] = {
	// Regular codes
	3, 3, 4, 5, 5, 6, 7, 4, 5, 6, 7, 4, 5, 6, 7,
	32, 33, 34, 35, 36, 37, 38, 39, 8, 9, 10, 11,
	12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
	23, 24, 25, 26, 27, 28, 29, 30, 31, 16, 17,
	18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
	29, 30, 31, 16, 17, 18, 19, 20, 21, 22, 23,
	24, 25, 26, 27, 28, 29, 30, 31, 16, 17, 18,
	19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
	30, 31, 16, 17, 18, 19, 20, 21, 22, 23, 24,
	25, 26, 27, 28, 29, 30, 31,

	// Escape code
	1,
	// End of block code
	2
};

static const byte s_huffmanACLengths[AC_CODE_COUNT] = {
	// Regular codes
	2, 3, 4, 4, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7,
	8, 8, 8, 8, 8, 8, 8, 8, 10, 10, 10, 10, 10,
	10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13,
	13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
	13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16,

	// Escape code
	6,
	// End of block code
	2
};

static const uint32 s_huffmanACSymbols[AC_CODE_COUNT] = {
	// Regular codes
	AC_HUFF_VAL(0, 1), AC_HUFF_VAL(1, 1), AC_HUFF_VAL(0, 2), AC_HUFF_VAL(2, 1), AC_HUFF_VAL(0, 3),
	AC_HUFF_VAL(4, 1), AC_HUFF_VAL(3, 1), AC_HUFF_VAL(7, 1), AC_HUFF_VAL(6, 1), AC_HUFF_VAL(1, 2),
	AC_HUFF_VAL(5, 1), AC_HUFF_VAL(2, 2), AC_HUFF_VAL(9, 1), AC_HUFF_VAL(0, 4), AC_HUFF_VAL(8, 1),
	AC_HUFF_VAL(13, 1), AC_HUFF_VAL(0, 6), AC_HUFF_VAL(12, 1), AC_HUFF_VAL(11, 1), AC_HUFF_VAL(3, 2),
	AC_HUFF_VAL(1, 3), AC_HUFF_VAL(0, 5), AC_HUFF_VAL(10, 1), AC_HUFF_VAL(16, 1), AC_HUFF_VAL(5, 2),
	AC_HUFF_VAL(0, 7), AC_HUFF_VAL(2, 3), AC_HUFF_VAL(1, 4), AC_HUFF_VAL(15, 1), AC_HUFF_VAL(14, 1),
	AC_HUFF_VAL(4, 2), AC_HUFF_VAL(0, 11), AC_HUFF_VAL(8, 2), AC_HUFF_VAL(4, 3), AC_HUFF_VAL(0, 10),
	AC_HUFF_VAL(2, 4), AC_HUFF_VAL(7, 2), AC_HUFF_VAL(21, 1), AC_HUFF_VAL(20, 1), AC_HUFF_VAL(0, 9),
	AC_HUFF_VAL(19, 1), AC_HUFF_VAL(18, 1), AC_HUFF_VAL(1, 5), AC_HUFF_VAL(3, 3), AC_HUFF_VAL(0, 8),
	AC_HUFF_VAL(6, 2), AC_HUFF_VAL(17, 1), AC_HUFF_VAL(10, 2), AC_HUFF_VAL(9, 2), AC_HUFF_VAL(5, 3),
	AC_HUFF_VAL(3, 4), AC_HUFF_VAL(2, 5), AC_HUFF_VAL(1, 7), AC_HUFF_VAL(1, 6), AC_HUFF_VAL(0, 15),
	AC_HUFF_VAL(0, 14), AC_HUFF_VAL(0, 13), AC_HUFF_VAL(0, 12), AC_HUFF_VAL(26, 1), AC_HUFF_VAL(25, 1),
	AC_HUFF_VAL(24, 1), AC_HUFF_VAL(23, 1), AC_HUFF_VAL(22, 1), AC_HUFF_VAL(0, 31), AC_HUFF_VAL(0, 30),
	AC_HUFF_VAL(0, 29), AC_HUFF_VAL(0, 28), AC_HUFF_VAL(0, 27), AC_HUFF_VAL(0, 26), AC_HUFF_VAL(0, 25),
	AC_HUFF_VAL(0, 24), AC_HUFF_VAL(0, 23), AC_HUFF_VAL(0, 22), AC_HUFF_VAL(0, 21), AC_HUFF_VAL(0, 20),
	AC_HUFF_VAL(0, 19), AC_HUFF_VAL(0, 18), AC_HUFF_VAL(0, 17), AC_HUFF_VAL(0, 16), AC_HUFF_VAL(0, 40),
	AC_HUFF_VAL(0, 39), AC_HUFF_VAL(0, 38), AC_HUFF_VAL(0, 37), AC_HUFF_VAL(0, 36), AC_HUFF_VAL(0, 35),
	AC_HUFF_VAL(0, 34), AC_HUFF_VAL(0, 33), AC_HUFF_VAL(0, 32), AC_HUFF_VAL(1, 14), AC_HUFF_VAL(1, 13),
	AC_HUFF_VAL(1, 12), AC_HUFF_VAL(1, 11), AC_HUFF_VAL(1, 10), AC_HUFF_VAL(1, 9), AC_HUFF_VAL(1, 8),
	AC_HUFF_VAL(1, 18), AC_HUFF_VAL(1, 17), AC_HUFF_VAL(1, 16), AC_HUFF_VAL(1, 15), AC_HUFF_VAL(6, 3),
	AC_HUFF_VAL(16, 2), AC_HUFF_VAL(15, 2), AC_HUFF_VAL(14, 2), AC_HUFF_VAL(13, 2), AC_HUFF_VAL(12, 2),
	AC_HUFF_VAL(11, 2), AC_HUFF_VAL(31, 1), AC_HUFF_VAL(30, 1), AC_HUFF_VAL(29, 1), AC_HUFF_VAL(28, 1),
	AC_HUFF_VAL(27, 1),

	// Escape code
	ESCAPE_CODE,
	// End of block code
	END_OF_BLOCK
};

PSXStreamDecoder::PSXStreamDecoder(CDSpeed speed, uint32 frameCount) : _speed(speed), _frameCount(frameCount) {
	_stream = 0;
}

PSXStreamDecoder::~PSXStreamDecoder() {
	close();
}

#define RAW_CD_SECTOR_SIZE 2352

#define CDXA_TYPE_MASK     0x0E
#define CDXA_TYPE_DATA     0x08
#define CDXA_TYPE_AUDIO    0x04
#define CDXA_TYPE_VIDEO    0x02

bool PSXStreamDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	_stream = stream;
	readNextPacket();

	return true;
}

void PSXStreamDecoder::close() {
	VideoDecoder::close();
	_audioTrack = 0;
	_videoTrack = 0;
	_frameCount = 0;

	delete _stream;
	_stream = 0;
}

#define VIDEO_DATA_CHUNK_SIZE   2016
#define VIDEO_DATA_HEADER_SIZE  56

void PSXStreamDecoder::readNextPacket() {
	Common::SeekableReadStream *sector = 0;
	byte *partialFrame = 0;
	int sectorsRead = 0;

	while (_stream->pos() < _stream->size()) {
		sector = readSector();
		sectorsRead++;

		if (!sector)
			error("Corrupt PSX stream sector");

		sector->seek(0x11);
		byte track = sector->readByte();
		if (track >= 32)
			error("Bad PSX stream track");

		byte sectorType = sector->readByte() & CDXA_TYPE_MASK;

		switch (sectorType) {
		case CDXA_TYPE_DATA:
		case CDXA_TYPE_VIDEO:
			if (track == 1) {
				if (!_videoTrack) {
					_videoTrack = new PSXVideoTrack(sector, _speed, _frameCount);
					addTrack(_videoTrack);
				}

				sector->seek(28);
				uint16 curSector = sector->readUint16LE();
				uint16 sectorCount = sector->readUint16LE();
				sector->readUint32LE();
				uint16 frameSize = sector->readUint32LE();

				if (curSector >= sectorCount)
					error("Bad sector");

				if (!partialFrame)
					partialFrame = (byte *)malloc(sectorCount * VIDEO_DATA_CHUNK_SIZE);

				sector->seek(VIDEO_DATA_HEADER_SIZE);
				sector->read(partialFrame + curSector * VIDEO_DATA_CHUNK_SIZE, VIDEO_DATA_CHUNK_SIZE);

				if (curSector == sectorCount - 1) {
					// Done assembling the frame
					Common::SeekableReadStream *frame = new Common::MemoryReadStream(partialFrame, frameSize, DisposeAfterUse::YES);

					_videoTrack->decodeFrame(frame, sectorsRead);

					delete frame;
					delete sector;
					return;
				}
			} else
				error("Unhandled multi-track video");
			break;
		case CDXA_TYPE_AUDIO:
			// We only handle one audio channel so far
			if (track == 1) {
				if (!_audioTrack) {
					_audioTrack = new PSXAudioTrack(sector);
					addTrack(_audioTrack);
				}

				_audioTrack->queueAudioFromSector(sector);
			} else {
				warning("Unhandled multi-track audio");
			}
			break;
		default:
			// This shows up way too often, but the other sectors
			// are safe to ignore
			//warning("Unknown PSX sector type 0x%x", sectorType);
			break;
		}

		delete sector;
	}

	if (_stream->pos() >= _stream->size()) {
		if (_videoTrack)
			_videoTrack->setEndOfTrack();

		if (_audioTrack)
			_audioTrack->setEndOfTrack();
	}
}

bool PSXStreamDecoder::useAudioSync() const {
	// Audio sync is disabled since most audio data comes after video
	// data.
	return false;
}

static const byte s_syncHeader[12] = { 0x00, 0xff ,0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00 };

Common::SeekableReadStream *PSXStreamDecoder::readSector() {
	assert(_stream);

	Common::SeekableReadStream *stream = _stream->readStream(RAW_CD_SECTOR_SIZE);

	byte syncHeader[12];
	stream->read(syncHeader, 12);
	if (!memcmp(s_syncHeader, syncHeader, 12))
		return stream;

	return 0;
}

// Ha! It's palindromic!
#define AUDIO_DATA_CHUNK_SIZE   2304
#define AUDIO_DATA_SAMPLE_COUNT 4032

static const int s_xaTable[5][2] = {
   {   0,   0 },
   {  60,   0 },
   { 115, -52 },
   {  98, -55 },
   { 122, -60 }
};

PSXStreamDecoder::PSXAudioTrack::PSXAudioTrack(Common::SeekableReadStream *sector) {
	assert(sector);
	_endOfTrack = false;

	sector->seek(19);
	byte format = sector->readByte();
	bool stereo = (format & (1 << 0)) != 0;
	uint rate = (format & (1 << 2)) ? 18900 : 37800;
	_audStream = Audio::makeQueuingAudioStream(rate, stereo);

	memset(&_adpcmStatus, 0, sizeof(_adpcmStatus));
}

PSXStreamDecoder::PSXAudioTrack::~PSXAudioTrack() {
	delete _audStream;
}

bool PSXStreamDecoder::PSXAudioTrack::endOfTrack() const {
	return AudioTrack::endOfTrack() && _endOfTrack;
}

void PSXStreamDecoder::PSXAudioTrack::queueAudioFromSector(Common::SeekableReadStream *sector) {
	assert(sector);

	sector->seek(24);

	// This XA audio is different (yet similar) from normal XA audio! Watch out!
	// TODO: It's probably similar enough to normal XA that we can merge it somehow...
	// TODO: RTZ PSX needs the same audio code in a regular AudioStream class. Probably
	// will do something similar to QuickTime and creating a base class 'ISOMode2Parser'
	// or something similar.
	byte *buf = new byte[AUDIO_DATA_CHUNK_SIZE];
	sector->read(buf, AUDIO_DATA_CHUNK_SIZE);

	int channels = _audStream->isStereo() ? 2 : 1;
	int16 *dst = new int16[AUDIO_DATA_SAMPLE_COUNT];
	int16 *leftChannel = dst;
	int16 *rightChannel = dst + 1;

	for (byte *src = buf; src < buf + AUDIO_DATA_CHUNK_SIZE; src += 128) {
		for (int i = 0; i < 4; i++) {
			int shift = 12 - (src[4 + i * 2] & 0xf);
			int filter = src[4 + i * 2] >> 4;
			int f0 = s_xaTable[filter][0];
			int f1 = s_xaTable[filter][1];
			int16 s_1 = _adpcmStatus[0].sample[0];
			int16 s_2 = _adpcmStatus[0].sample[1];

			for (int j = 0; j < 28; j++) {
				byte d = src[16 + i + j * 4];
				int t = (int8)(d << 4) >> 4;
				int s = (t << shift) + ((s_1 * f0 + s_2 * f1 + 32) >> 6);
				s_2 = s_1;
				s_1 = CLIP<int>(s, -32768, 32767);
				*leftChannel = s_1;
				leftChannel += channels;
			}

			if (channels == 2) {
				_adpcmStatus[0].sample[0] = s_1;
				_adpcmStatus[0].sample[1] = s_2;
				s_1 = _adpcmStatus[1].sample[0];
				s_2 = _adpcmStatus[1].sample[1];
			}

			shift = 12 - (src[5 + i * 2] & 0xf);
			filter = src[5 + i * 2] >> 4;
			f0 = s_xaTable[filter][0];
			f1 = s_xaTable[filter][1];

			for (int j = 0; j < 28; j++) {
				byte d = src[16 + i + j * 4];
				int t = (int8)d >> 4;
				int s = (t << shift) + ((s_1 * f0 + s_2 * f1 + 32) >> 6);
				s_2 = s_1;
				s_1 = CLIP<int>(s, -32768, 32767);

				if (channels == 2) {
					*rightChannel = s_1;
					rightChannel += 2;
				} else {
					*leftChannel++ = s_1;
				}
			}

			if (channels == 2) {
				_adpcmStatus[1].sample[0] = s_1;
				_adpcmStatus[1].sample[1] = s_2;
			} else {
				_adpcmStatus[0].sample[0] = s_1;
				_adpcmStatus[0].sample[1] = s_2;
			}
		}
	}

	int flags = Audio::FLAG_16BITS;

	if (_audStream->isStereo())
		flags |= Audio::FLAG_STEREO;

#ifdef SCUMM_LITTLE_ENDIAN
	flags |= Audio::FLAG_LITTLE_ENDIAN;
#endif

	_audStream->queueBuffer((byte *)dst, AUDIO_DATA_SAMPLE_COUNT * 2, DisposeAfterUse::YES, flags);
	delete[] buf;
}

Audio::AudioStream *PSXStreamDecoder::PSXAudioTrack::getAudioStream() const {
	return _audStream;
}


PSXStreamDecoder::PSXVideoTrack::PSXVideoTrack(Common::SeekableReadStream *firstSector, CDSpeed speed, int frameCount) : _nextFrameStartTime(0, speed), _frameCount(frameCount) {
	assert(firstSector);

	firstSector->seek(40);
	uint16 width = firstSector->readUint16LE();
	uint16 height = firstSector->readUint16LE();
	_surface = new Graphics::Surface();
	_surface->create(width, height, g_system->getScreenFormat());

	_macroBlocksW = (width + 15) / 16;
	_macroBlocksH = (height + 15) / 16;
	_yBuffer = new byte[_macroBlocksW * _macroBlocksH * 16 * 16];
	_cbBuffer = new byte[_macroBlocksW * _macroBlocksH * 8 * 8];
	_crBuffer = new byte[_macroBlocksW * _macroBlocksH * 8 * 8];

	_endOfTrack = false;
	_curFrame = -1;
	_acHuffman = new Common::Huffman(0, AC_CODE_COUNT, s_huffmanACCodes, s_huffmanACLengths, s_huffmanACSymbols);
	_dcHuffmanChroma = new Common::Huffman(0, DC_CODE_COUNT, s_huffmanDCChromaCodes, s_huffmanDCChromaLengths, s_huffmanDCSymbols);
	_dcHuffmanLuma = new Common::Huffman(0, DC_CODE_COUNT, s_huffmanDCLumaCodes, s_huffmanDCLumaLengths, s_huffmanDCSymbols);
}

PSXStreamDecoder::PSXVideoTrack::~PSXVideoTrack() {
	_surface->free();
	delete _surface;

	delete[] _yBuffer;
	delete[] _cbBuffer;
	delete[] _crBuffer;
	delete _acHuffman;
	delete _dcHuffmanChroma;
	delete _dcHuffmanLuma;
}

uint32 PSXStreamDecoder::PSXVideoTrack::getNextFrameStartTime() const {
	return _nextFrameStartTime.msecs();
}

const Graphics::Surface *PSXStreamDecoder::PSXVideoTrack::decodeNextFrame() {
	return _surface;
}

void PSXStreamDecoder::PSXVideoTrack::decodeFrame(Common::SeekableReadStream *frame, uint sectorCount) {
	// A frame is essentially an MPEG-1 intra frame

	Common::BitStream16LEMSB bits(frame);

	bits.skip(16); // unknown
	bits.skip(16); // 0x3800
	uint16 scale = bits.getBits(16);
	uint16 version = bits.getBits(16);

	if (version != 2 && version != 3)
		error("Unknown PSX stream frame version");

	// Initalize default v3 DC here
	_lastDC[0] = _lastDC[1] = _lastDC[2] = 0;

	for (int mbX = 0; mbX < _macroBlocksW; mbX++)
		for (int mbY = 0; mbY < _macroBlocksH; mbY++)
			decodeMacroBlock(&bits, mbX, mbY, scale, version);

	// Output data onto the frame
	YUVToRGBMan.convert420(_surface, Graphics::YUVToRGBManager::kScaleFull, _yBuffer, _cbBuffer, _crBuffer, _surface->w, _surface->h, _macroBlocksW * 16, _macroBlocksW * 8);

	_curFrame++;

	// Increase the time by the amount of sectors we read
	// One may notice that this is still not the most precise
	// method since a frame takes up the time its sectors took
	// up instead of the amount of time it takes the next frame
	// to be read from the sectors. The actual frame rate should
	// be constant instead of variable, so the slight difference
	// in a frame's showing time is negligible (1/150 of a second).
	_nextFrameStartTime = _nextFrameStartTime.addFrames(sectorCount);
}

void PSXStreamDecoder::PSXVideoTrack::decodeMacroBlock(Common::BitStream *bits, int mbX, int mbY, uint16 scale, uint16 version) {
	int pitchY = _macroBlocksW * 16;
	int pitchC = _macroBlocksW * 8;

	// Note the strange order of red before blue
	decodeBlock(bits, _crBuffer + (mbY * pitchC + mbX) * 8, pitchC, scale, version, kPlaneV);
	decodeBlock(bits, _cbBuffer + (mbY * pitchC + mbX) * 8, pitchC, scale, version, kPlaneU);
	decodeBlock(bits, _yBuffer + (mbY * pitchY + mbX) * 16, pitchY, scale, version, kPlaneY);
	decodeBlock(bits, _yBuffer + (mbY * pitchY + mbX) * 16 + 8, pitchY, scale, version, kPlaneY);
	decodeBlock(bits, _yBuffer + (mbY * pitchY + mbX) * 16 + 8 * pitchY, pitchY, scale, version, kPlaneY);
	decodeBlock(bits, _yBuffer + (mbY * pitchY + mbX) * 16 + 8 * pitchY + 8, pitchY, scale, version, kPlaneY);
}

// Standard JPEG/MPEG zig zag table
static const byte s_zigZagTable[8 * 8] = {
	 0,  1,  5,  6, 14, 15, 27, 28,
	 2,  4,  7, 13, 16, 26, 29, 42,
	 3,  8, 12, 17, 25, 30, 41, 43,
	 9, 11, 18, 24, 31, 40, 44, 53,
	10, 19, 23, 32, 39, 45, 52, 54,
	20, 22, 33, 38, 46, 51, 55, 60,
	21, 34, 37, 47, 50, 56, 59, 61,
	35, 36, 48, 49, 57, 58, 62, 63
};

// One byte different from the standard MPEG-1 table
static const byte s_quantizationTable[8 * 8] = {
	 2, 16, 19, 22, 26, 27, 29, 34,
	16, 16, 22, 24, 27, 29, 34, 37,
	19, 22, 26, 27, 29, 34, 34, 38,
	22, 22, 26, 27, 29, 34, 37, 40,
	22, 26, 27, 29, 32, 35, 40, 48,
	26, 27, 29, 32, 35, 40, 48, 58,
	26, 27, 29, 34, 38, 46, 56, 69,
	27, 29, 35, 38, 46, 56, 69, 83
};

void PSXStreamDecoder::PSXVideoTrack::dequantizeBlock(int *coefficients, float *block, uint16 scale) {
	// Dequantize the data, un-zig-zagging as we go along
	for (int i = 0; i < 8 * 8; i++) {
		if (i == 0) // Special case for the DC coefficient
			block[i] = coefficients[i] * s_quantizationTable[i];
		else
			block[i] = (float)coefficients[s_zigZagTable[i]] * s_quantizationTable[i] * scale / 8;
	}
}

int PSXStreamDecoder::PSXVideoTrack::readDC(Common::BitStream *bits, uint16 version, PlaneType plane) {
	// Version 2 just has its coefficient as 10-bits
	if (version == 2)
		return readSignedCoefficient(bits);

	// Version 3 has it stored as huffman codes as a difference from the previous DC value

	Common::Huffman *huffman = (plane == kPlaneY) ? _dcHuffmanLuma : _dcHuffmanChroma;

	uint32 symbol = huffman->getSymbol(*bits);
	int dc = 0;

	if (GET_DC_BITS(symbol) != 0) {
		bool negative = (bits->getBit() == 0);
		dc = bits->getBits(GET_DC_BITS(symbol) - 1);

		if (negative)
			dc -= GET_DC_NEG(symbol);
		else
			dc += GET_DC_POS(symbol);
	}

	_lastDC[plane] += dc * 4; // convert from 8-bit to 10-bit
	return _lastDC[plane];
}

#define BLOCK_OVERFLOW_CHECK() \
	if (count > 63) \
		error("PSXStreamDecoder::readAC(): Too many coefficients")

void PSXStreamDecoder::PSXVideoTrack::readAC(Common::BitStream *bits, int *block) {
	// Clear the block first
	for (int i = 0; i < 63; i++)
		block[i] = 0;

	int count = 0;

	while (!bits->eos()) {
		uint32 symbol = _acHuffman->getSymbol(*bits);

		if (symbol == ESCAPE_CODE) {
			// The escape code!
			int zeroes = bits->getBits(6);
			count += zeroes + 1;
			BLOCK_OVERFLOW_CHECK();
			block += zeroes;
			*block++ = readSignedCoefficient(bits);
		} else if (symbol == END_OF_BLOCK) {
			// We're done
			break;
		} else {
			// Normal huffman code
			int zeroes = GET_AC_ZERO_RUN(symbol);
			count += zeroes + 1;
			BLOCK_OVERFLOW_CHECK();
			block += zeroes;

			if (bits->getBit())
				*block++ = -GET_AC_COEFFICIENT(symbol);
			else
				*block++ = GET_AC_COEFFICIENT(symbol);
		}
	}
}

int PSXStreamDecoder::PSXVideoTrack::readSignedCoefficient(Common::BitStream *bits) {
	uint val = bits->getBits(10);

	// extend the sign
	uint shift = 8 * sizeof(int) - 10;
	return (int)(val << shift) >> shift;
}

// IDCT table built with :
// _idct8x8[x][y] = cos(((2 * x + 1) * y) * (M_PI / 16.0)) * 0.5;
// _idct8x8[x][y] /= sqrt(2.0) if y == 0
static const double s_idct8x8[8][8] = {
	{ 0.353553390593274,  0.490392640201615,  0.461939766255643,  0.415734806151273,  0.353553390593274,  0.277785116509801,  0.191341716182545,  0.097545161008064 },
	{ 0.353553390593274,  0.415734806151273,  0.191341716182545, -0.097545161008064, -0.353553390593274, -0.490392640201615, -0.461939766255643, -0.277785116509801 },
	{ 0.353553390593274,  0.277785116509801, -0.191341716182545, -0.490392640201615, -0.353553390593274,  0.097545161008064,  0.461939766255643,  0.415734806151273 },
	{ 0.353553390593274,  0.097545161008064, -0.461939766255643, -0.277785116509801,  0.353553390593274,  0.415734806151273, -0.191341716182545, -0.490392640201615 },
	{ 0.353553390593274, -0.097545161008064, -0.461939766255643,  0.277785116509801,  0.353553390593274, -0.415734806151273, -0.191341716182545,  0.490392640201615 },
	{ 0.353553390593274, -0.277785116509801, -0.191341716182545,  0.490392640201615, -0.353553390593273, -0.097545161008064,  0.461939766255643, -0.415734806151273 },
	{ 0.353553390593274, -0.415734806151273,  0.191341716182545,  0.097545161008064, -0.353553390593274,  0.490392640201615, -0.461939766255643,  0.277785116509801 },
	{ 0.353553390593274, -0.490392640201615,  0.461939766255643, -0.415734806151273,  0.353553390593273, -0.277785116509801,  0.191341716182545, -0.097545161008064 }
};

void PSXStreamDecoder::PSXVideoTrack::idct(float *dequantData, float *result) {
	// IDCT code based on JPEG's IDCT code
	// TODO: Switch to the integer-based one mentioned in the docs
	// This is by far the costliest operation here

	float tmp[8 * 8];

	// Apply 1D IDCT to rows
	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 8; x++) {
			tmp[y + x * 8] = dequantData[0] * s_idct8x8[x][0]
							+ dequantData[1] * s_idct8x8[x][1]
							+ dequantData[2] * s_idct8x8[x][2]
							+ dequantData[3] * s_idct8x8[x][3]
							+ dequantData[4] * s_idct8x8[x][4]
							+ dequantData[5] * s_idct8x8[x][5]
							+ dequantData[6] * s_idct8x8[x][6]
							+ dequantData[7] * s_idct8x8[x][7];
		}

		dequantData += 8;
	}

	// Apply 1D IDCT to columns
	for (int x = 0; x < 8; x++) {
		const float *u = tmp + x * 8;
		for (int y = 0; y < 8; y++) {
			result[y * 8 + x] = u[0] * s_idct8x8[y][0]
								+ u[1] * s_idct8x8[y][1]
								+ u[2] * s_idct8x8[y][2]
								+ u[3] * s_idct8x8[y][3]
								+ u[4] * s_idct8x8[y][4]
								+ u[5] * s_idct8x8[y][5]
								+ u[6] * s_idct8x8[y][6]
								+ u[7] * s_idct8x8[y][7];
		}
	}
}

void PSXStreamDecoder::PSXVideoTrack::decodeBlock(Common::BitStream *bits, byte *block, int pitch, uint16 scale, uint16 version, PlaneType plane) {
	// Version 2 just has signed 10 bits for DC
	// Version 3 has them huffman coded
	int coefficients[8 * 8];
	coefficients[0] = readDC(bits, version, plane);
	readAC(bits, &coefficients[1]); // Read in the AC

	// Dequantize
	float dequantData[8 * 8];
	dequantizeBlock(coefficients, dequantData, scale);

	// Perform IDCT
	float idctData[8 * 8];
	idct(dequantData, idctData);

	// Now output the data
	for (int y = 0; y < 8; y++) {
		byte *dst = block + pitch * y;

		// Convert the result to be in the range [0, 255]
		for (int x = 0; x < 8; x++)
			*dst++ = (int)CLIP<float>(idctData[y * 8 + x], -128.0f, 127.0f) + 128;
	}
}


} // End of namespace Video

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

#include "common/debug.h"
#include "common/endian.h"
#include "common/system.h"
#include "common/stream.h"
#include "common/textconsole.h"

#include "graphics/surface.h"

#include "video/dxa_decoder.h"

#ifdef USE_ZLIB
  #include "common/zlib.h"
#endif

namespace Video {

DXADecoder::DXADecoder() {
}

DXADecoder::~DXADecoder() {
	close();
}

bool DXADecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	uint32 tag = stream->readUint32BE();

	if (tag != MKTAG('D','E','X','A')) {
		close();
		return false;
	}

	DXAVideoTrack *track = new DXAVideoTrack(stream);
	addTrack(track);

	readSoundData(stream);

	track->setFrameStartPos();
	return true;
}

void DXADecoder::readSoundData(Common::SeekableReadStream *stream) {
	// Skip over the tag by default
	stream->readUint32BE();
}

DXADecoder::DXAVideoTrack::DXAVideoTrack(Common::SeekableReadStream *stream) {
	_fileStream = stream;
	_curFrame = -1;
	_frameStartOffset = 0;
	_decompBuffer = 0;
	_inBuffer = 0;
	memset(_palette, 0, 256 * 3);

	uint8 flags = _fileStream->readByte();
	_frameCount = _fileStream->readUint16BE();
	int32 frameRate = _fileStream->readSint32BE();

	if (frameRate > 0)
		_frameRate = 1000 / frameRate;
	else if (frameRate < 0)
		_frameRate = 100000 / (-frameRate);
	else
		_frameRate = 10;

	_width = _fileStream->readUint16BE();
	_height = _fileStream->readUint16BE();

	if (flags & 0x80) {
		_scaleMode = S_INTERLACED;
		_curHeight = _height / 2;
	} else if (flags & 0x40) {
		_scaleMode = S_DOUBLE;
		_curHeight = _height / 2;
	} else {
		_scaleMode = S_NONE;
		_curHeight = _height;
	}

	_surface = new Graphics::Surface();
	_surface->format = Graphics::PixelFormat::createFormatCLUT8();

	debug(2, "flags 0x0%x framesCount %d width %d height %d rate %d", flags, getFrameCount(), getWidth(), getHeight(), getFrameRate().toInt());

	_frameSize = _width * _height;
	_decompBufferSize = _frameSize;
	_frameBuffer1 = new byte[_frameSize];
	memset(_frameBuffer1, 0, _frameSize);
	_frameBuffer2 = new byte[_frameSize];
	memset(_frameBuffer2, 0, _frameSize);

	_scaledBuffer = 0;
	if (_scaleMode != S_NONE) {
		_scaledBuffer = new byte[_frameSize];
		memset(_scaledBuffer, 0, _frameSize);
	}

#ifdef DXA_EXPERIMENT_MAXD
	// Check for an extended header
	if (flags & 1) {
		uint32 size;

		do {
			tag = _fileStream->readUint32BE();

			if (tag != 0)
				size = _fileStream->readUint32BE();

			switch (tag) {
				case 0: // No more tags
					break;
				case MKTAG('M','A','X','D'):
					assert(size == 4);
					_decompBufferSize = _fileStream->readUint32BE();
					break;
				default: // Unknown tag - skip it.
					while (size > 0) {
						byte dummy = _fileStream->readByte();
						size--;
					}
					break;
			}
		} while (tag != 0);
	}
#endif
}

DXADecoder::DXAVideoTrack::~DXAVideoTrack() {
	delete _fileStream;
	delete _surface;
	delete[] _frameBuffer1;
	delete[] _frameBuffer2;
	delete[] _scaledBuffer;
	delete[] _inBuffer;
	delete[] _decompBuffer;
}

bool DXADecoder::DXAVideoTrack::rewind() {
	_curFrame = -1;
	_fileStream->seek(_frameStartOffset);
	return true;
}

Graphics::PixelFormat DXADecoder::DXAVideoTrack::getPixelFormat() const {
	return _surface->format;
}

void DXADecoder::DXAVideoTrack::setFrameStartPos() {
	_frameStartOffset = _fileStream->pos();
}

void DXADecoder::DXAVideoTrack::decodeZlib(byte *data, int size, int totalSize) {
#ifdef USE_ZLIB
	unsigned long dstLen = totalSize;
	Common::uncompress(data, &dstLen, _inBuffer, size);
#endif
}

#define BLOCKW 4
#define BLOCKH 4

void DXADecoder::DXAVideoTrack::decode12(int size) {
#ifdef USE_ZLIB
	if (!_decompBuffer) {
		_decompBuffer = new byte[_decompBufferSize];
		memset(_decompBuffer, 0, _decompBufferSize);
	}

	/* decompress the input data */
	decodeZlib(_decompBuffer, size, _decompBufferSize);

	byte *dat = _decompBuffer;

	memcpy(_frameBuffer2, _frameBuffer1, _frameSize);

	for (uint32 by = 0; by < _height; by += BLOCKH) {
		for (uint32 bx = 0; bx < _width; bx += BLOCKW) {
			byte type = *dat++;
			byte *b2 = _frameBuffer1 + bx + by * _width;

			switch (type) {
			case 0:
				break;
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
			case 1:	{
				unsigned short diffMap;
				if (type >= 10 && type <= 15) {
					static const struct { uint8 sh1, sh2; } shiftTbl[6] = {
						{0, 0},	{8, 0},	{8, 8},	{8, 4},	{4, 0},	{4, 4}
					};
					diffMap = ((*dat & 0xF0) << shiftTbl[type-10].sh1) |
						  ((*dat & 0x0F) << shiftTbl[type-10].sh2);
					dat++;
				} else {
					diffMap = *(unsigned short*)dat;
					dat += 2;
				}

				for (int yc = 0; yc < BLOCKH; yc++) {
					for (int xc = 0; xc < BLOCKW; xc++) {
						if (diffMap & 0x8000) {
							b2[xc] = *dat++;
						}
						diffMap <<= 1;
					}
					b2 += _width;
				}
				break;
			}
			case 2:	{
				byte color = *dat++;

				for (int yc = 0; yc < BLOCKH; yc++) {
					for (int xc = 0; xc < BLOCKW; xc++) {
						b2[xc] = color;
					}
					b2 += _width;
				}
				break;
			}
			case 3:	{
				for (int yc = 0; yc < BLOCKH; yc++) {
					for (int xc = 0; xc < BLOCKW; xc++) {
						b2[xc] = *dat++;
					}
					b2 += _width;
				}
				break;
			}
			case 4:	{
				byte mbyte = *dat++;
				int mx = (mbyte >> 4) & 0x07;
				if (mbyte & 0x80)
					mx = -mx;
				int my = mbyte & 0x07;
				if (mbyte & 0x08)
					my = -my;
				byte *b1 = _frameBuffer2 + (bx+mx) + (by+my) * _width;
				for (int yc = 0; yc < BLOCKH; yc++) {
					memcpy(b2, b1, BLOCKW);
					b1 += _width;
					b2 += _width;
				}
				break;
			}
			case 5:
				break;
			default:
				error("decode12: Unknown type %d", type);
			}
		}
	}
#endif
}

void DXADecoder::DXAVideoTrack::decode13(int size) {
#ifdef USE_ZLIB
	uint8 *codeBuf, *dataBuf, *motBuf, *maskBuf;

	if (!_decompBuffer) {
		_decompBuffer = new byte[_decompBufferSize];
		memset(_decompBuffer, 0, _decompBufferSize);
	}

	/* decompress the input data */
	decodeZlib(_decompBuffer, size, _decompBufferSize);

	memcpy(_frameBuffer2, _frameBuffer1, _frameSize);

	int codeSize = _width * _curHeight / 16;
	int dataSize, motSize;

	dataSize = READ_BE_UINT32(&_decompBuffer[0]);
	motSize  = READ_BE_UINT32(&_decompBuffer[4]);
	//maskSize = READ_BE_UINT32(&_decompBuffer[8]);

	codeBuf = &_decompBuffer[12];
	dataBuf = &codeBuf[codeSize];
	motBuf = &dataBuf[dataSize];
	maskBuf = &motBuf[motSize];

	for (uint32 by = 0; by < _curHeight; by += BLOCKH) {
		for (uint32 bx = 0; bx < _width; bx += BLOCKW) {
			uint8 type = *codeBuf++;
			uint8 *b2 = (uint8 *)_frameBuffer1 + bx + by * _width;

			switch (type) {
			case 0:
				break;

			case 1: {
				uint16 diffMap = READ_BE_UINT16(maskBuf);
				maskBuf += 2;

				for (int yc = 0; yc < BLOCKH; yc++) {
					for (int xc = 0; xc < BLOCKW; xc++) {
						if (diffMap & 0x8000) {
							b2[xc] = *dataBuf++;
						}
						diffMap <<= 1;
					}
					b2 += _width;
				}
				break;
			}
			case 2: {
				uint8 color = *dataBuf++;

				for (int yc = 0; yc < BLOCKH; yc++) {
					for (int xc = 0; xc < BLOCKW; xc++) {
						b2[xc] = color;
					}
					b2 += _width;
				}
				break;
			}
			case 3: {
				for (int yc = 0; yc < BLOCKH; yc++) {
					for (int xc = 0; xc < BLOCKW; xc++) {
						b2[xc] = *dataBuf++;
					}
					b2 += _width;
				}
				break;
			}
			case 4: {
				uint8 mbyte = *motBuf++;

				int mx = (mbyte >> 4) & 0x07;
				if (mbyte & 0x80)
					mx = -mx;
				int my = mbyte & 0x07;
				if (mbyte & 0x08)
					my = -my;

				uint8 *b1 = (uint8 *)_frameBuffer2 + (bx+mx) + (by+my) * _width;
				for (int yc = 0; yc < BLOCKH; yc++) {
					memcpy(b2, b1, BLOCKW);
					b1 += _width;
					b2 += _width;
				}
				break;
			}
			case 8: {
				static const int subX[4] = {0, 2, 0, 2};
				static const int subY[4] = {0, 0, 2, 2};

				uint8 subMask = *maskBuf++;

				for (int subBlock = 0; subBlock < 4; subBlock++) {
					int sx = bx + subX[subBlock], sy = by + subY[subBlock];
					b2 = (uint8 *)_frameBuffer1 + sx + sy * _width;
					switch (subMask & 0xC0) {
					// 00: skip
					case 0x00:
						break;
					// 01: solid color
					case 0x40: {
						uint8 subColor = *dataBuf++;
						for (int yc = 0; yc < BLOCKH / 2; yc++) {
							for (int xc = 0; xc < BLOCKW / 2; xc++) {
								b2[xc] = subColor;
							}
							b2 += _width;
						}
						break;
					}
					// 02: motion vector
					case 0x80: {
						uint8 mbyte = *motBuf++;

						int mx = (mbyte >> 4) & 0x07;
						if (mbyte & 0x80)
							mx = -mx;

						int my = mbyte & 0x07;
						if (mbyte & 0x08)
							my = -my;

						uint8 *b1 = (uint8 *)_frameBuffer2 + (sx+mx) + (sy+my) * _width;
						for (int yc = 0; yc < BLOCKH / 2; yc++) {
							memcpy(b2, b1, BLOCKW / 2);
							b1 += _width;
							b2 += _width;
						}
						break;
					}
					// 03: raw
					case 0xC0:
						for (int yc = 0; yc < BLOCKH / 2; yc++) {
							for (int xc = 0; xc < BLOCKW / 2; xc++) {
								b2[xc] = *dataBuf++;
							}
							b2 += _width;
						}
						break;
					}
					subMask <<= 2;
				}
				break;
			}
			case 32:
			case 33:
			case 34: {
				int count = type - 30;
				uint8 pixels[4];

				memcpy(pixels, dataBuf, count);
				dataBuf += count;

				if (count == 2) {
					uint16 code = READ_BE_UINT16(maskBuf);
					maskBuf += 2;
					for (int yc = 0; yc < BLOCKH; yc++) {
						for (int xc = 0; xc < BLOCKW; xc++) {
							b2[xc] = pixels[code & 1];
							code >>= 1;
						}
						b2 += _width;
					}
				} else {
					uint32 code = READ_BE_UINT32(maskBuf);
					maskBuf += 4;
					for (int yc = 0; yc < BLOCKH; yc++) {
						for (int xc = 0; xc < BLOCKW; xc++) {
							b2[xc] = pixels[code & 3];
							code >>= 2;
						}
						b2 += _width;
					}
				}
				break;
			}
			default:
				error("decode13: Unknown type %d", type);
			}
		}
	}
#endif
}

const Graphics::Surface *DXADecoder::DXAVideoTrack::decodeNextFrame() {
	uint32 tag = _fileStream->readUint32BE();
	if (tag == MKTAG('C','M','A','P')) {
		_fileStream->read(_palette, 256 * 3);
		_dirtyPalette = true;
	}

	tag = _fileStream->readUint32BE();
	if (tag == MKTAG('F','R','A','M')) {
		byte type = _fileStream->readByte();
		uint32 size = _fileStream->readUint32BE();

		if (!_inBuffer || _inBufferSize < size) {
			delete[] _inBuffer;
			_inBuffer = new byte[size];
			memset(_inBuffer, 0, size);
			_inBufferSize = size;
		}

		_fileStream->read(_inBuffer, size);

		switch (type) {
		case 2:
			decodeZlib(_frameBuffer1, size, _frameSize);
			break;
		case 3:
			decodeZlib(_frameBuffer2, size, _frameSize);
			break;
		case 12:
			decode12(size);
			break;
		case 13:
			decode13(size);
			break;
		default:
			error("decodeFrame: Unknown compression type %d", type);
		}

		if (type == 3) {
			for (uint32 j = 0; j < _curHeight; ++j) {
				for (uint32 i = 0; i < _width; ++i) {
					const int offs = j * _width + i;
					_frameBuffer1[offs] ^= _frameBuffer2[offs];
				}
			}
		}
	}

	switch (_scaleMode) {
	case S_INTERLACED:
		for (int cy = 0; cy < _curHeight; cy++) {
			memcpy(&_scaledBuffer[2 * cy * _width], &_frameBuffer1[cy * _width], _width);
			memset(&_scaledBuffer[((2 * cy) + 1) * _width], 0, _width);
		}
		_surface->pixels = _scaledBuffer;
		break;
	case S_DOUBLE:
		for (int cy = 0; cy < _curHeight; cy++) {
			memcpy(&_scaledBuffer[2 * cy * _width], &_frameBuffer1[cy * _width], _width);
			memcpy(&_scaledBuffer[((2 * cy) + 1) * _width], &_frameBuffer1[cy * _width], _width);
		}
		_surface->pixels = _scaledBuffer;
		break;
	case S_NONE:
		_surface->pixels = _frameBuffer1;
		break;
	}

	// Copy in the relevant info to the Surface
	_surface->w = getWidth();
	_surface->h = getHeight();
	_surface->pitch = getWidth();

	_curFrame++;

	return _surface;
}

} // End of namespace Video

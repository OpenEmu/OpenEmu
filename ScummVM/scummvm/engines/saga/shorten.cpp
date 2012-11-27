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

#include "common/scummsys.h"
#include "common/textconsole.h"

#include "saga/shorten.h"

#ifdef SOUND_SHORTEN_H

// Based on etree's Shorten tool, version 3.6.1
// http://etree.org/shnutils/shorten/

// FIXME: This doesn't work yet correctly

#include "common/util.h"

#include "audio/decoders/raw.h"

namespace Saga {

#define MASKTABSIZE 33
#define MAX_SUPPORTED_VERSION 3
#define DEFAULT_BLOCK_SIZE 256

enum kShortenTypes {
	kTypeAU1 = 0,		// lossless ulaw
	kTypeS8 = 1,		// signed 8 bit
	kTypeU8 = 2,		// unsigned 8 bit
	kTypeS16HL = 3,		// signed 16 bit shorts: high-low
	kTypeU16HL = 4,		// unsigned 16 bit shorts: high-low
	kTypeS16LH = 5,		// signed 16 bit shorts: low-high
	kTypeU16LH = 6,		// unsigned 16 bit shorts: low-high
	kTypeULaw = 7,		// lossy ulaw
	kTypeAU2 = 8,		// new ulaw with zero mapping
	kTypeAU3 = 9,		// lossless alaw
	kTypeALaw = 10,		// lossy alaw
	kTypeWAV = 11,		// WAV
	kTypeAIFF = 12,		// AIFF
	kTypeEOF = 13,
	kTypeGenericULaw = 128,
	kTypeGenericALaw = 129
};

enum kShortenCommands {
	kCmdDiff0 = 0,
	kCmdDiff1 = 1,
	kCmdDiff2 = 2,
	kCmdDiff3 = 3,
	kCmdQuit = 4,
	kCmdBlockSize = 5,
	kCmdBitShift = 6,
	kCmdQLPC = 7,
	kCmdZero = 8,
	kCmdVerbatim = 9
};

#ifndef M_LN2
#define M_LN2 0.69314718055994530942
#endif

// ---------------------------------------------------------------------------

class ShortenGolombReader {
public:
	ShortenGolombReader(Common::ReadStream *stream, int version);
	~ShortenGolombReader() {}
	uint32 getUint32(uint32 numBits);    // UINT_GET
	int32 getURice(uint32 numBits);      // uvar_get
	int32 getSRice(uint32 numBits);      // var_get
private:
	int _version;
	uint32 _nbitget;
	uint32 _buf;
	uint32 _masktab[MASKTABSIZE];
	Common::ReadStream *_stream;
};

ShortenGolombReader::ShortenGolombReader(Common::ReadStream *stream, int version) {
	_stream = stream;
	_version = version;
	uint32 val = 0;
	_masktab[0] = 0;
	_nbitget = 0;
	_buf = 0;

	for (int i = 1; i < MASKTABSIZE; i++) {
		val <<= 1;
		val |= 1;
		_masktab[i] = val;
	}
}

int32 ShortenGolombReader::getURice(uint32 numBits) {
	int32 result = 0;

	if (!_nbitget) {
		_buf = _stream->readUint32BE();
		_nbitget = 32;
	}

	for (result = 0; !(_buf & (1L << --_nbitget)); result++) {
		if (!_nbitget) {
			_buf = _stream->readUint32BE();
			_nbitget = 32;
		}
	}

	while (numBits != 0) {
		if (_nbitget >= numBits) {
			result = (result << numBits) | ((_buf >> (_nbitget - numBits)) & _masktab[numBits]);
			_nbitget -= numBits;
			numBits = 0;
		} else {
			result = (result << _nbitget) | (_buf & _masktab[_nbitget]);
			_buf = _stream->readUint32BE();
			numBits -= _nbitget;
			_nbitget = 32;
		}
	}

	return result;
}

int32 ShortenGolombReader::getSRice(uint32 numBits) {
	uint32 uvar = (uint32) getURice(numBits + 1);
	return (uvar & 1) ? (int32) ~(uvar >> 1) : (int32) (uvar >> 1);
}

uint32 ShortenGolombReader::getUint32(uint32 numBits) {
	return (_version == 0) ? (uint32)getURice(numBits) : (uint32)getURice(getURice(2));
}

// ---------------------------------------------------------------------------

byte *loadShortenFromStream(Common::ReadStream &stream, int &size, int &rate, byte &flags) {
	int32 *buffer[2], *offset[2];	// up to 2 channels
	byte *unpackedBuffer = 0;
	byte *pBuf = unpackedBuffer;
	int prevSize = 0;
	int32 *lpc = 0;

	ShortenGolombReader *gReader;
	uint32 i, j, version, mean, type, channels, blockSize;
	uint32 maxLPC = 0, lpcqOffset = 0;
	int32 bitShift = 0, wrap = 0;
	flags = 0;
	size = 0;

	// Read header
	byte magic[4];
	stream.read(magic, 4);
	if (memcmp(magic, "ajkg", 4) != 0) {
		warning("loadShortenFromStream: No 'ajkg' header");
		return NULL;
	}

	version = stream.readByte();

	if (version > MAX_SUPPORTED_VERSION) {
		warning("loadShortenFromStream: Can't decode version %d, maximum supported version is %d", version, MAX_SUPPORTED_VERSION);
		return NULL;
	}

	mean = (version < 2) ? 0 : 4;

	gReader = new ShortenGolombReader(&stream, version);

	// Get file type
	type = gReader->getUint32(4);

	switch (type) {
		case kTypeS8:
			break;
		case kTypeU8:
			flags |= Audio::FLAG_UNSIGNED;
			break;
		case kTypeS16LH:
			flags |= Audio::FLAG_LITTLE_ENDIAN;
		case kTypeS16HL:
			flags |= Audio::FLAG_16BITS;
			break;
		case kTypeU16LH:
			flags |= Audio::FLAG_LITTLE_ENDIAN;
		case kTypeU16HL:
			flags |= Audio::FLAG_16BITS;
			flags |= Audio::FLAG_UNSIGNED;
			break;
		case kTypeWAV:
			// TODO: Perhaps implement this if we find WAV Shorten encoded files
			warning("loadShortenFromStream: Type WAV is not supported");
			delete gReader;
			return NULL;
		case kTypeAIFF:
			// TODO: Perhaps implement this if we find AIFF Shorten encoded files
			warning("loadShortenFromStream: Type AIFF is not supported");
			delete gReader;
			return NULL;
		case kTypeAU1:
		case kTypeAU2:
		case kTypeAU3:
		case kTypeULaw:
		case kTypeALaw:
		case kTypeEOF:
		case kTypeGenericULaw:
		case kTypeGenericALaw:
		default:
			warning("loadShortenFromStream: Type %d is not supported", type);
			delete gReader;
			return NULL;
	}

	// Get channels
	channels = gReader->getUint32(0);
	if (channels != 1 && channels != 2) {
		warning("loadShortenFromStream: Only 1 or 2 channels are supported, stream contains %d channels", channels);
		delete gReader;
		return NULL;
	}

	// Get block size
	if (version > 0) {
		blockSize = gReader->getUint32((int) (log((double) DEFAULT_BLOCK_SIZE) / M_LN2));
		maxLPC = gReader->getUint32(2);
		mean = gReader->getUint32(0);
		uint32 skipBytes = gReader->getUint32(1);
		if (skipBytes > 0) {
			prevSize = size;
			size += skipBytes;
			unpackedBuffer = (byte *) realloc(unpackedBuffer, size);
			pBuf = unpackedBuffer + prevSize;
			for (i = 0; i < skipBytes; i++) {
				*pBuf++ = gReader->getUint32(7) & 0xFF;
			}
		}
	} else {
		blockSize = DEFAULT_BLOCK_SIZE;
	}

	wrap = MAX<uint32>(3, maxLPC);

	// Initialize buffers
	for (i = 0; i < channels; i++) {
		buffer[i] = (int32 *)malloc((blockSize + wrap) * 4);
		offset[i] = (int32 *)malloc((MAX<uint32>(1, mean)) * 4);
		memset(buffer[i], 0, (blockSize + wrap) * 4);
		memset(offset[i], 0, (MAX<uint32>(1, mean)) * 4);
	}

	if (maxLPC > 0)
		lpc = (int32 *) malloc(maxLPC * 4);

	if (version > 1)
		lpcqOffset = 1 << 5;

	// Init offset
	int32 offsetMean = 0;
	uint32 blocks = MAX<int>(1, mean);

	if (type == kTypeU8)
		offsetMean = 0x80;
	else if (type == kTypeU16HL || type == kTypeU16LH)
		offsetMean = 0x8000;

	for (uint32 channel = 0; channel < channels; channel++)
		for (uint32 block = 0; block < blocks; block++)
			offset[channel][block] = offsetMean;


	uint32 curChannel = 0, cmd = 0;

	// Parse Shorten commands
	while (true) {
		cmd = gReader->getURice(2);

		if (cmd == kCmdQuit)
			break;

		switch (cmd) {
			case kCmdZero:
			case kCmdDiff0:
			case kCmdDiff1:
			case kCmdDiff2:
			case kCmdDiff3:
			case kCmdQLPC:
				{
				int32 channelOffset = 0, energy = 0;
				uint32 lpcNum = 0;

				if (cmd != kCmdZero) {
					energy = gReader->getURice(3);
					// hack for version 0
					if (version == 0)
						energy--;
				}

				// Find mean offset (code duplicated below)
				if (mean == 0) {
					channelOffset = offset[curChannel][0];
				} else {
					int32 sum = (version < 2) ? 0 : mean / 2;
					for (i = 0; i < mean; i++)
						sum += offset[curChannel][i];

					channelOffset = sum / mean;

					if (version >= 2 && bitShift > 0)
						channelOffset = (channelOffset >> (bitShift - 1)) >> 1;
				}

				// FIXME: The original code in this bit tries to modify memory outside of the array (negative indices)
				// in cases kCmdDiff1, kCmdDiff2 and kCmdDiff3
				// I've removed those invalid writes, since they happen all the time (even when curChannel is 0)
				switch (cmd) {
					case kCmdZero:
						for (i = 0; i < blockSize; i++)
							buffer[curChannel][i] = 0;
						break;
					case kCmdDiff0:
						for (i = 0; i < blockSize; i++)
							buffer[curChannel][i] = gReader->getSRice(energy) + channelOffset;
						break;
					case kCmdDiff1:
						gReader->getSRice(energy);	// i = 0 (to fix invalid table/memory access)
						for (i = 1; i < blockSize; i++)
							buffer[curChannel][i] = gReader->getSRice(energy) + buffer[curChannel][i - 1];
						break;
					case kCmdDiff2:
						gReader->getSRice(energy);	// i = 0 (to fix invalid table/memory access)
						gReader->getSRice(energy);	// i = 1 (to fix invalid table/memory access)
						for (i = 2; i < blockSize; i++)
							buffer[curChannel][i] = gReader->getSRice(energy) + 2 * buffer[curChannel][i - 1] - buffer[curChannel][i - 2];
						break;
					case kCmdDiff3:
						gReader->getSRice(energy);	// i = 0 (to fix invalid table/memory access)
						gReader->getSRice(energy);	// i = 1 (to fix invalid table/memory access)
						gReader->getSRice(energy);	// i = 2 (to fix invalid table/memory access)
						for (i = 3; i < blockSize; i++)
							buffer[curChannel][i] = gReader->getSRice(energy) + 3 * (buffer[curChannel][i - 1] - buffer[curChannel][i - 2]) + buffer[curChannel][i - 3];
						break;
					case kCmdQLPC:
						lpcNum = gReader->getURice(2);

						// Safeguard: if maxLPC < lpcNum, realloc the lpc buffer
						if (maxLPC < lpcNum) {
							warning("Safeguard: maxLPC < lpcNum (should never happen)");
							maxLPC = lpcNum;
							int32 *tmp = (int32 *) realloc(lpc, maxLPC * 4);
							if ((tmp != NULL) || (maxLPC == 0)) {
								lpc = tmp;
							} else {
								error("loadShortenFromStream(): Error while reallocating memory");
							}
						}

						for (i = 0; i < lpcNum; i++)
							lpc[i] = gReader->getSRice(5);

						for (i = 0; i < lpcNum; i++)
							buffer[curChannel][i - lpcNum] -= channelOffset;

						for (i = 0; i < blockSize; i++) {
							int32 sum = lpcqOffset;
							for (j = 0; j < lpcNum; j++) {
								// FIXME: The original code did an invalid memory access here
								// (if i and j are 0, the array index requested is -1)
								// I've removed those invalid writes, since they happen all the time (even when curChannel is 0)
								if (i <= j)	// ignore invalid table/memory access
									continue;
								sum += lpc[j] * buffer[curChannel][i - j - 1];
							}
							buffer[curChannel][i] = gReader->getSRice(energy) + (sum >> 5);
						}

						if (channelOffset > 0)
							for (i = 0; i < blockSize; i++)
								buffer[curChannel][i] += channelOffset;

						break;
				}

				// Store mean value, if appropriate (duplicated code from above)
				if (mean > 0) {
					int32 sum = (version < 2) ? 0 : blockSize / 2;
					for (i = 0; i < blockSize; i++)
						sum += buffer[curChannel][i];

					for (i = 1; i < mean; i++)
						offset[curChannel][i - 1] = offset[curChannel][i];

					offset[curChannel][mean - 1] = sum / blockSize;

					if (version >= 2 && bitShift > 0)
						offset[curChannel][mean - 1] = offset[curChannel][mean - 1] << bitShift;
				}


				// Do the wrap
				// FIXME: removed for now, as this corrupts the heap, because it
				// accesses negative array indices
				//for (int32 k = -wrap; k < 0; k++)
				//	buffer[curChannel][k] = buffer[curChannel][k + blockSize];

				// Fix bitshift
				if (bitShift > 0) {
					for (i = 0; i < blockSize; i++)
						buffer[curChannel][i] <<= bitShift;
				}

				if (curChannel == channels - 1) {
					int dataSize = (flags & Audio::FLAG_16BITS) ? 2 : 1;
					int limit = (flags & Audio::FLAG_16BITS) ? 32767 : 127;
					limit = (flags & Audio::FLAG_UNSIGNED) ? limit * 2 + 1 : limit;

					prevSize = size;
					size += (blockSize * dataSize);
					byte *tmp = (byte *) realloc(unpackedBuffer, size);
					if ((tmp != NULL) || (size == 0)) {
						unpackedBuffer = tmp;
					} else {
						error("loadShortenFromStream(): Error while reallocating memory");
					}
					pBuf = unpackedBuffer + prevSize;

					if (flags & Audio::FLAG_16BITS) {
						for (i = 0; i < blockSize; i++) {
							for (j = 0; j < channels; j++) {
								int16 val = (int16)(MIN<int32>(buffer[j][i], limit) & 0xFFFF);
								// values are written in LE
								*pBuf++ = (byte) (val & 0xFF);
								*pBuf++ = (byte) ((val >> 8) & 0xFF);
							}
						}
					} else {
						for (i = 0; i < blockSize; i++)
							for (j = 0; j < channels; j++)
								*pBuf++ = (byte)(MIN<int32>(buffer[j][i], limit) & 0xFF);
					}
				}
				curChannel = (curChannel + 1) % channels;

				}
				break;
			case kCmdBlockSize:
				blockSize = gReader->getUint32((uint32)log((double) blockSize / M_LN2));
				break;
			case kCmdBitShift:
				bitShift = gReader->getURice(2);
				break;
			case kCmdVerbatim:
				{

				uint32 vLen = (uint32)gReader->getURice(5);
				prevSize = size;
				size += vLen;
				byte *tmp = (byte *) realloc(unpackedBuffer, size);
				if ((tmp != NULL) || (size == 0)) {
					unpackedBuffer = tmp;
				} else {
					error("loadShortenFromStream(): Error while reallocating memory");
				}
				pBuf = unpackedBuffer + prevSize;

				while (vLen--) {
					*pBuf++ = (byte)(gReader->getURice(8) & 0xFF);
				}

				}
				break;
			default:
				warning("loadShortenFromStream: Unknown command: %d", cmd);

				// Cleanup
				for (i = 0; i < channels; i++) {
					free(buffer[i]);
					free(offset[i]);
				}

				if (maxLPC > 0)
					free(lpc);

				if (size > 0)
					free(unpackedBuffer);

				delete gReader;
				return NULL;
				break;
		}
	}

	// Rate is always 44100Hz
	rate = 44100;

	// Cleanup
	for (i = 0; i < channels; i++) {
		free(buffer[i]);
		free(offset[i]);
	}

	if (maxLPC > 0)
		free(lpc);

	delete gReader;
	return unpackedBuffer;
}

Audio::AudioStream *makeShortenStream(Common::SeekableReadStream &stream) {
	int size, rate;
	byte *data, flags;
	data = loadShortenFromStream(stream, size, rate, flags);

	if (!data)
		return 0;

	// Since we allocated our own buffer for the data, we must specify DisposeAfterUse::YES.
	return Audio::makeRawStream(data, size, rate, flags);
}

} // End of namespace Audio

#endif // defined(SOUND_SHORTEN_H)

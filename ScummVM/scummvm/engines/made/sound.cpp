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

#include "made/sound.h"

#include "common/endian.h"

namespace Made {

void ManholeEgaSoundDecompressor::decompress(byte *source, byte *dest, uint32 size) {
	/* Some kind of ADPCM compression. I hope this works on BE machines. */
	int newmode;
	_source = source;
	_dest = dest;
	_size = size;
	_bitBuffer = 0;
	_bitsLeft = 0;
	_writeFlag = false;
	_eof = false;
	_sample1 = 0x80000;
	_sample2 = 0x800000;
	_sample3 = 0x800000;
	_sample4 = 0x800000;
	_mode = getBit();
	while (!_eof) {
		update1();
		update3();
		update0();
		newmode = getBit();
		if (_eof)
			break;
		if (newmode == _mode) {
			update1();
			update3();
			do {
				update0();
				newmode = getBit();
				if (_eof || newmode != _mode)
					break;
				update2();
				update3();
			} while (1);
		}
		_mode = newmode;
	}
}

int ManholeEgaSoundDecompressor::getBit() {
	if (_bitsLeft == 0) {
		if (_size == 0) {
			_eof = true;
			return 0;
		}
		_bitBuffer = READ_BE_UINT16(_source);
		_source += 2;
		_bitsLeft = 16;
		_size -= 2;
	}
	int temp = _bitBuffer & 0x8000;
	_bitBuffer <<= 1;
	_bitsLeft--;
	return temp;
}

void ManholeEgaSoundDecompressor::update0() {
	SWAP(_sample1, _sample3);
	if (_sample2 & 0x80000000) {
		_sample2 -= (_sample2 >> 8) | 0xFF000000;
	} else {
		_sample2 -= _sample2 >> 8;
	}
	_sample2 += 0x8000;
	if (_sample2 & 0x80000000) {
		_sample2 = 0;
	} else if ((_sample2 & 0xFFFF0000) > 0x00FF0000) {
		_sample2 = 0xFF0000;
	}
	_sample1 += _sample2;
	_sample1 >>= 1;
	_sample1 -= _sample4;
	_sample1 >>= 2;
	_sample4 += _sample1;
	if (_writeFlag) {
		*_dest++ = (_sample4 & 0xFF0000) >> 16;
	}
	_writeFlag = !_writeFlag;
	_sample1 = _sample2;
	SWAP(_sample1, _sample3);
}

void ManholeEgaSoundDecompressor::update1() {
	if (_sample1 & 0x80000000) {
		_sample1 -= (_sample1 >> 8) | 0xFF000000;
	} else {
		_sample1 -= _sample1 >> 8;
	}
	_sample1 += 500;
}

void ManholeEgaSoundDecompressor::update2() {
	uint32 temp = (_sample1 >> 6) | ((_sample1 & 0xFF) << 27) | ((_sample1 & 0xC0) >> 5);
	if (_sample1 & 0x80000000) {
		_sample1 += temp | 0xFC000000;
	} else {
		_sample1 += temp & 0x03FFFFFF;
	}
	_sample1 += 500;
}

void ManholeEgaSoundDecompressor::update3() {
	if (_mode)
		_sample2 -= _sample1;
	else
		_sample2 += _sample1;
}

void decompressSound(byte *source, byte *dest, uint16 chunkSize, uint16 chunkCount, SoundEnergyArray *soundEnergyArray) {

	int16 prevSample = 0, workSample = 0;
	byte soundBuffer[1025];
	byte deltaSoundBuffer[1024];
	int16 soundBuffer2[16];
	byte deltaType, type;
	uint16 workChunkSize, byteCount, bitCount;
	byte bitMask, bitShift;
	uint16 ofs = 0;
	uint16 i = 0, l = 0;
	byte val;

	SoundEnergyItem soundEnergyItem;

	const int modeValues[3][4] = {
		{ 2, 8, 0x01, 1},
		{ 4, 4, 0x03, 2},
		{16, 2, 0x0F, 4}
	};

	soundEnergyItem.position = 0;

	if (soundEnergyArray)
		soundEnergyArray->clear();

	while (chunkCount--) {
		deltaType = (*source) >> 6;
		workChunkSize = chunkSize;

		if (deltaType == 1)
			workChunkSize /= 2;
		else if (deltaType == 2)
			workChunkSize /= 4;

		type = (*source++) & 0x0F;

		workSample = prevSample;

		soundEnergyItem.position += chunkSize;

		switch (type) {

		case 0:
			memset(soundBuffer, 0x80, workChunkSize);
			workSample = 0;

			soundEnergyItem.energy = 0;
			if (soundEnergyArray)
				soundEnergyArray->push_back(soundEnergyItem);

			break;

		case 1:
			break;

		case 2:
		case 3:
		case 4:
			byteCount = modeValues[type - 2][0];
			bitCount = modeValues[type - 2][1];
			bitMask = modeValues[type - 2][2];
			bitShift = modeValues[type - 2][3];
			ofs = 0;

			for (i = 0; i < byteCount; i++)
				soundBuffer2[i] = (*source++) * 2 - 128;

			while (ofs < workChunkSize) {
				val = *source++;
				for (i = 0; i < bitCount; i++) {
					workSample = CLIP<int16>(workSample + soundBuffer2[val & bitMask], -127, 127);
					val >>= bitShift;
					soundBuffer[ofs++] = workSample + 128;
				}
			}

			soundEnergyItem.energy = type - 1;
			if (soundEnergyArray)
				soundEnergyArray->push_back(soundEnergyItem);

			break;

		case 5:
			for (i = 0; i < workChunkSize; i++)
				soundBuffer[i] = *source++;
			workSample = soundBuffer[workChunkSize - 1] - 128;

			soundEnergyItem.energy = 4;
			if (soundEnergyArray)
				soundEnergyArray->push_back(soundEnergyItem);

			break;

		default:
	 		return;

		}

		if (deltaType > 0) {
			if (deltaType == 1) {
				for (i = 0; i < chunkSize - 1; i += 2) {
					l = i / 2;
					deltaSoundBuffer[i] = soundBuffer[l];
					deltaSoundBuffer[i + 1] = (soundBuffer[l] + soundBuffer[l + 1]) / 2;
				}
			} else if (deltaType == 2) {
				for (i = 0; i < chunkSize - 1; i += 4) {
					l = i / 4;
					deltaSoundBuffer[i] = soundBuffer[l];
					deltaSoundBuffer[i + 2] = (soundBuffer[l] + soundBuffer[l + 1]) / 2;
					deltaSoundBuffer[i + 1] = (deltaSoundBuffer[i + 2] + soundBuffer[l]) / 2;
					deltaSoundBuffer[i + 3] = (deltaSoundBuffer[i + 2] + soundBuffer[l + 1]) / 2;
				}
			}
			for (i = 0; i < chunkSize; i++)
				soundBuffer[i] = deltaSoundBuffer[i];
		}

		prevSample = workSample;
		memcpy(dest, soundBuffer, chunkSize);
		dest += chunkSize;

	}

}

} // End of namespace Made

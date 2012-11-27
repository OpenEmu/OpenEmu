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

#include "common/util.h"
#include "common/memstream.h"
#include "common/textconsole.h"

#include "audio/mixer.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/wave.h"

#include "gob/sound/sounddesc.h"
#include "gob/resources.h"

namespace Gob {

SoundDesc::SoundDesc() {
	_resource = 0;

	_data = _dataPtr = 0;
	_size = 0;

	_type = SOUND_SND;

	_repCount = 0;
	_frequency = 0;
	_flag = 0;
	_id = 0;
	_mixerFlags = 0;
}

SoundDesc::~SoundDesc() {
	free();
}

void SoundDesc::swap(SoundDesc &desc) {
	SWAP(_repCount  , desc._repCount);
	SWAP(_frequency , desc._frequency);
	SWAP(_flag      , desc._flag);
	SWAP(_id        , desc._id);
	SWAP(_mixerFlags, desc._mixerFlags);
	SWAP(_resource  , desc._resource);
	SWAP(_data      , desc._data);
	SWAP(_dataPtr   , desc._dataPtr);
	SWAP(_size      , desc._size);
	SWAP(_type      , desc._type);
}

void SoundDesc::set(SoundType type, byte *data, uint32 dSize) {
	free();

	_type = type;
	_data = _dataPtr = data;
	_size = dSize;
}

void SoundDesc::set(SoundType type, Resource *resource) {
	byte *data = 0;
	uint32 dSize = 0;

	if (resource && (resource->getSize() > 0)) {
		data = resource->getData();
		dSize = resource->getSize();
	}

	set(type, data, dSize);

	_resource = resource;
}

bool SoundDesc::load(SoundType type, byte *data, uint32 dSize) {
	free();

	switch (type) {
	case SOUND_ADL:
		return loadADL(data, dSize);
	case SOUND_SND:
		return loadSND(data, dSize);
	case SOUND_WAV:
		return loadWAV(data, dSize);
	}

	return false;
}

bool SoundDesc::load(SoundType type, Resource *resource) {
	if (!resource || (resource->getSize() <= 0))
		return false;

	if (!load(type, resource->getData(), resource->getSize()))
		return false;

	_resource = resource;
	return true;
}

void SoundDesc::free() {
	if (_resource) {
		delete _resource;
		_data = 0;
	}

	delete[] _data;

	_resource = 0;
	_data = 0;
	_dataPtr = 0;
	_id = 0;
}

void SoundDesc::convToSigned() {
	if ((_type != SOUND_SND) && (_type != SOUND_WAV))
		return;
	if (!_data || !_dataPtr)
		return;

	if (_mixerFlags & Audio::FLAG_16BITS) {
		byte *data = _dataPtr;
		for (uint32 i = 0; i < _size; i++, data += 2)
			WRITE_LE_UINT16(data, READ_LE_UINT16(data) ^ 0x8000);
	} else
		for (uint32 i = 0; i < _size; i++)
			_dataPtr[i] ^= 0x80;

}

int16 SoundDesc::calcFadeOutLength(int16 frequency) {
	return (10 * (_size / 2)) / frequency;
}

uint32 SoundDesc::calcLength(int16 repCount, int16 frequency, bool fade) {
	uint32 fadeSize = fade ? _size / 2 : 0;
	return ((_size * repCount - fadeSize) * 1000) / frequency;
}

bool SoundDesc::loadSND(byte *data, uint32 dSize) {
	assert(dSize > 6);

	_type = SOUND_SND;
	_data = data;
	_dataPtr = data + 6;
	_frequency = MAX((int16) READ_BE_UINT16(data + 4), (int16) 4700);
	_flag = data[0] ? (data[0] & 0x7F) : 8;
	data[0] = 0;
	_size = MIN(READ_BE_UINT32(data), dSize - 6);

	return true;
}

bool SoundDesc::loadWAV(byte *data, uint32 dSize) {
	Common::MemoryReadStream stream(data, dSize);

	int wavSize, wavRate;
	byte wavFlags;
	uint16 wavtype;

	if (!Audio::loadWAVFromStream(stream, wavSize, wavRate, wavFlags, &wavtype, 0))
		return false;

	if (wavFlags & Audio::FLAG_16BITS) {
		_mixerFlags |= Audio::FLAG_16BITS;
		wavSize >>= 1;
	}

	if (wavFlags & Audio::FLAG_STEREO) {
		warning("TODO: SoundDesc::loadWAV() - stereo");
		return false;
	}

	_data = data;
	_dataPtr = data + stream.pos();
	_size = wavSize;
	_frequency = wavRate;

	if (wavFlags & Audio::FLAG_UNSIGNED)
		convToSigned();

	return true;
}

bool SoundDesc::loadADL(byte *data, uint32 dSize) {
	_type = SOUND_ADL;
	_data = _dataPtr = data;
	_size = dSize;

	return true;
}

} // End of namespace Gob

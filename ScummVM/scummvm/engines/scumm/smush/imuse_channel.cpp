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


#include "common/endian.h"

#include "scumm/scumm.h"	// For DEBUG_SMUSH
#include "scumm/util.h"
#include "scumm/smush/channel.h"
#include "scumm/imuse_digi/dimuse_codecs.h"	// for decode12BitsSample

namespace Scumm {

ImuseChannel::ImuseChannel(int32 track) : SmushChannel(track) {
}

bool ImuseChannel::isTerminated() const {
	return (_dataSize <= 0 && _sbuffer == 0);
}

bool ImuseChannel::setParameters(int32 nb, int32 size, int32 flags, int32 unk1, int32) {
	if ((flags == 1) || (flags == 2) || (flags == 3)) {
		_volume = 127;
	} else if ((flags >= 100) && (flags <= 163)) {
		_volume = flags * 2 - 200;
	} else if ((flags >= 200) && (flags <= 263)) {
		_volume = flags * 2 - 400;
	} else if ((flags >= 300) && (flags <= 363)) {
		_volume = flags * 2 - 600;
	} else {
		error("ImuseChannel::setParameters(): bad flags: %d", flags);
	}
	_pan = 0;
	return true;
}

bool ImuseChannel::checkParameters(int32 index, int32 nbframes, int32 size, int32 track_flags, int32 unk1) {
	return true;
}

bool ImuseChannel::appendData(Common::SeekableReadStream &b, int32 size) {
	if (_dataSize == -1) {
		assert(size > 8);
		uint32 imus_type = b.readUint32BE();
		/*uint32 imus_size =*/ b.readUint32BE();
		if (imus_type != MKTAG('i','M','U','S'))
			error("Invalid Chunk for imuse_channel");
		size -= 8;
		_tbufferSize = size;
		assert(_tbufferSize);
		_tbuffer = (byte *)malloc(_tbufferSize);
		if (!_tbuffer)
			error("imuse_channel failed to allocate memory");
		b.read(_tbuffer, size);
		_dataSize = -2;
	} else {
		if (_tbuffer) {
			byte *old = _tbuffer;
			int32 new_size = size + _tbufferSize;
			_tbuffer = (byte *)malloc(new_size);
			if (!_tbuffer)
				error("imuse_channel failed to allocate memory");
			memcpy(_tbuffer, old, _tbufferSize);
			free(old);
			b.read(_tbuffer + _tbufferSize, size);
			_tbufferSize += size;
		} else {
			_tbufferSize = size;
			_tbuffer = (byte *)malloc(_tbufferSize);
			if (!_tbuffer)
				error("imuse_channel failed to allocate memory");
			b.read(_tbuffer, size);
		}
	}

	processBuffer();

	_srbufferSize = _sbufferSize;
	if (_sbuffer && _bitsize == 12)
		decode();

	return true;
}

bool ImuseChannel::handleMap(byte *data) {
	// Read the chunk size & skip over the chunk header
	int32 size = READ_BE_UINT32(data + 4);
	data += 8;

	while (size > 0) {
		uint32 subType = READ_BE_UINT32(data);
		int32 subSize = READ_BE_UINT32(data + 4);
		data += 8;
		size -= 8;

		switch (subType) {
		case MKTAG('F','R','M','T'):
			if (subSize != 20)
				error("invalid size for FRMT Chunk");
			//uint32 imuse_start = READ_BE_UINT32(data);
			//uint32 unk = READ_BE_UINT32(data+4);
			_bitsize = READ_BE_UINT32(data+8);
			_rate = READ_BE_UINT32(data+12);
			_channels = READ_BE_UINT32(data+16);
			assert(_channels == 1 || _channels == 2);
			break;
		case MKTAG('T','E','X','T'):
			// Ignore this
			break;
		case MKTAG('R','E','G','N'):
			if (subSize != 8)
				error("invalid size for REGN Chunk");
			break;
		case MKTAG('S','T','O','P'):
			if (subSize != 4)
				error("invalid size for STOP Chunk");
			break;
		default:
			error("Unknown iMUS subChunk found : %s, %d", tag2str(subType), subSize);
		}

		data += subSize;
		size -= subSize;
	}
	return true;
}

void ImuseChannel::decode() {
	int remaining_size = _sbufferSize % 3;
	if (remaining_size) {
		_srbufferSize -= remaining_size;
		assert(_inData);
		if (_tbuffer == 0) {
			_tbuffer = (byte *)malloc(remaining_size);
			memcpy(_tbuffer, _sbuffer + _sbufferSize - remaining_size, remaining_size);
			_tbufferSize = remaining_size;
			_sbufferSize -= remaining_size;
		} else {
			debugC(DEBUG_SMUSH, "impossible ! : %p, %d, %d, %p(%d), %p(%d, %d)",
				(const void *)this, _dataSize, _inData, _tbuffer, _tbufferSize, _sbuffer, _sbufferSize, _srbufferSize);
			byte *old = _tbuffer;
			int new_size = remaining_size + _tbufferSize;
			_tbuffer = (byte *)malloc(new_size);
			if (!_tbuffer)
				error("imuse_channel failed to allocate memory");
			memcpy(_tbuffer, old, _tbufferSize);
			free(old);
			memcpy(_tbuffer + _tbufferSize, _sbuffer + _sbufferSize - remaining_size, remaining_size);
			_tbufferSize += remaining_size;
		}
	}

	byte *keep;
	_sbufferSize = BundleCodecs::decode12BitsSample(_sbuffer, &keep, _sbufferSize);
	free(_sbuffer);
	_sbuffer = (byte *)keep;
}

bool ImuseChannel::handleSubTags(int32 &offset) {
	if (_tbufferSize - offset >= 8) {
		uint32 type = READ_BE_UINT32(_tbuffer + offset);
		uint32 size = READ_BE_UINT32(_tbuffer + offset + 4);
		uint32 available_size = _tbufferSize - offset;
		switch (type) {
		case MKTAG('M','A','P',' '):
			_inData = false;
			if (available_size >= (size + 8)) {
				handleMap((byte *)_tbuffer + offset);
			}
			break;
		case MKTAG('D','A','T','A'):
			_inData = true;
			_dataSize = size;
			offset += 8;
			{
				int reqsize = 1;
				if (_channels == 2)
					reqsize *= 2;
				if (_bitsize == 16)
					reqsize *= 2;
				else if (_bitsize == 12) {
					if (reqsize > 1)
						reqsize = reqsize * 3 / 2;
					else reqsize = 3;
				}
				if ((size % reqsize) != 0) {
					debugC(DEBUG_SMUSH, "Invalid iMUS sound data size : (%d %% %d) != 0, correcting...", size, reqsize);
					size += 3 - (size % reqsize);
				}
			}
			return false;
		default:
			error("unknown Chunk in iMUS track : %s ", tag2str(type));
		}
		offset += size + 8;
		return true;
	}
	return false;
}

byte *ImuseChannel::getSoundData() {
	byte *tmp = _sbuffer;

	assert(_dataSize > 0);
	_dataSize -= _srbufferSize;

	_sbuffer = 0;
	_sbufferSize = 0;

	return tmp;
}

} // End of namespace Scumm

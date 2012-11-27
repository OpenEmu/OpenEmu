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


#include "common/textconsole.h"

#include "scumm/smush/channel.h"

namespace Scumm {

SmushChannel::SmushChannel(int32 track) :
	_track(track),
	_tbuffer(0),
	_tbufferSize(0),
	_sbuffer(0),
	_sbufferSize(0),
	_dataSize(-1),
	_inData(false),
	_volume(0),
	_pan(0) {
}

SmushChannel::~SmushChannel() {
	free(_tbuffer);
	free(_sbuffer);
}

void SmushChannel::processBuffer() {
	assert(_tbuffer != 0);
	assert(_tbufferSize != 0);
	assert(_sbuffer == 0);
	assert(_sbufferSize == 0);

	if (_inData) {
		if (_dataSize < _tbufferSize) {
			int32 offset = _dataSize;
			while (handleSubTags(offset))
				;
			_sbufferSize = _dataSize;
			_sbuffer = _tbuffer;
			if (offset < _tbufferSize) {
				int new_size = _tbufferSize - offset;
				_tbuffer = (byte *)malloc(new_size);
				// FIXME: _tbuffer might be 0 if new_size is 0.
				// NB: Also check other "if (_tbuffer)" locations in smush
				if (!_tbuffer)
					error("smush channel failed to allocate memory");
				memcpy(_tbuffer, _sbuffer + offset, new_size);
				_tbufferSize = new_size;
			} else {
				_tbuffer = 0;
				_tbufferSize = 0;
			}
			if (_sbufferSize == 0) {
				free(_sbuffer);
				_sbuffer = 0;
			}
		} else {
			_sbufferSize = _tbufferSize;
			_sbuffer = _tbuffer;
			_tbufferSize = 0;
			_tbuffer = 0;
		}
	} else {
		int32 offset = 0;
		while (handleSubTags(offset))
			;
		if (_inData) {
			_sbufferSize = _tbufferSize - offset;
			assert(_sbufferSize);
			_sbuffer = (byte *)malloc(_sbufferSize);
			if (!_sbuffer)
				error("smush channel failed to allocate memory");
			memcpy(_sbuffer, _tbuffer + offset, _sbufferSize);
			free(_tbuffer);
			_tbuffer = 0;
			_tbufferSize = 0;
		} else {
			if (offset) {
				byte *old = _tbuffer;
				int32 new_size = _tbufferSize - offset;
				_tbuffer = (byte *)malloc(new_size);
				// NB: Also check other "if (_tbuffer)" locations in smush
				if (!_tbuffer) {
					if (new_size)
						error("smush channel failed to allocate memory");
				} else {
					memcpy(_tbuffer, old + offset, new_size);
				}
				_tbufferSize = new_size;
				free(old);
			}
		}
	}
}


} // End of namespace Scumm

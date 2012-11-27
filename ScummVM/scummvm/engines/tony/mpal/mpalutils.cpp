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
 *
 */

#include "tony/mpal/mpalutils.h"
#include "tony/tony.h"
#include "common/memstream.h"

namespace Tony {

namespace MPAL {

/****************************************************************************\
*       RMRes methods
\****************************************************************************/

/**
 * Constructor
 * @param resId					MPAL resource to open
 */
RMRes::RMRes(uint32 resID) {
	_h = g_vm->_resUpdate.queryResource(resID);
	if (_h == NULL)
		_h = mpalQueryResource(resID);
	if (_h != NULL)
		_buf = (byte *)globalLock(_h);
}

/**
 * Destructor
 */
RMRes::~RMRes() {
	if (_h != NULL) {
		globalUnlock(_h);
		globalFree(_h);
	}
}

/**
 * Returns a pointer to the resource
 */
const byte *RMRes::dataPointer() {
	return _buf;
}

/**
 * Returns a pointer to the resource
 */
RMRes::operator const byte *() {
	return dataPointer();
}

/**
 * Returns the size of the resource
 */
unsigned int RMRes::size() {
	return globalSize(_h);
}

Common::SeekableReadStream *RMRes::getReadStream() {
	return new Common::MemoryReadStream(_buf, size());
}

bool RMRes::isValid() {
	return _h != NULL;
}

/****************************************************************************\
*       RMResRaw methods
\****************************************************************************/

RMResRaw::RMResRaw(uint32 resID) : RMRes(resID) {
}

RMResRaw::~RMResRaw() {
}

const byte *RMResRaw::dataPointer() {
	return _buf + 8;
}

RMResRaw::operator const byte *() {
	return dataPointer();
}

int RMResRaw::width() {
	return READ_LE_UINT16(_buf + 4);
}

int RMResRaw::height() {
	return READ_LE_UINT16(_buf + 6);
}

} // end of namespace MPAL

} // end of namespace Tony

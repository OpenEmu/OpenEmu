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

#include "gob/gob.h"
#include "gob/init.h"
#include "gob/global.h"
#include "gob/draw.h"
#include "gob/video.h"

namespace Gob {

Init_Geisha::Init_Geisha(GobEngine *vm) : Init_v1(vm) {
}

Init_Geisha::~Init_Geisha() {
}

void Init_Geisha::initVideo() {
	Init_v1::initVideo();

	_vm->_draw->_cursorWidth       = 16;
	_vm->_draw->_cursorHeight      = 23;
	_vm->_draw->_transparentCursor =  1;
}

} // End of namespace Gob

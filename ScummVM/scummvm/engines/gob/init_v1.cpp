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

namespace Gob {

Init_v1::Init_v1(GobEngine *vm) : Init(vm) {
}

Init_v1::~Init_v1() {
}

void Init_v1::initVideo() {
	if (_vm->_global->_videoMode)
		_vm->validateVideoMode(_vm->_global->_videoMode);

	_vm->_global->_mousePresent = 1;

	if ((_vm->_global->_videoMode == 0x13) && !_vm->isEGA())
		_vm->_global->_colorCount = 256;

	_vm->_global->_pPaletteDesc = &_vm->_global->_paletteStruct;
	_vm->_global->_pPaletteDesc->vgaPal = _vm->_draw->_vgaPalette;
	_vm->_global->_pPaletteDesc->unused1 = _vm->_global->_unusedPalette1;
	_vm->_global->_pPaletteDesc->unused2 = _vm->_global->_unusedPalette2;

	_vm->_video->initSurfDesc(320, 200, PRIMARY_SURFACE);

	_vm->_draw->_cursorWidth       = 16;
	_vm->_draw->_cursorHeight      = 16;
	_vm->_draw->_transparentCursor =  1;
}

} // End of namespace Gob

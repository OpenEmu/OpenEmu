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

/*
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janus B. Wisniewski and L.K. Avalon
 */

#include "cge/game.h"
#include "cge/events.h"

namespace CGE {

Fly::Fly(CGEEngine *vm, Bitmap **shpl)
	: Sprite(vm, shpl), _tx(0), _ty(0), _vm(vm) {
	step(_vm->newRandom(2));
	gotoxy(kFlyL + _vm->newRandom(kFlyR - kFlyL - _w), kFlyT + _vm->newRandom(kFlyB - kFlyT - _h));
}

void Fly::tick() {
	step();
	if (_flags._kept)
		return;
	if (_vm->newRandom(10) < 1) {
		_tx = _vm->newRandom(3) - 1;
		_ty = _vm->newRandom(3) - 1;
	}
	if (_x + _tx < kFlyL || _x + _tx + _w > kFlyR)
		_tx = -_tx;
	if (_y + _ty < kFlyT || _y + _ty + _h > kFlyB)
		_ty = -_ty;
	gotoxy(_x + _tx, _y + _ty);
}

} // End of namespace CGE

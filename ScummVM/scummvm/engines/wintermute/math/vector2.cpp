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
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/math/vector2.h"
#include <math.h>

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
Vector2::Vector2() {
	x = y = 0.0f;
}

//////////////////////////////////////////////////////////////////////////
Vector2::Vector2(float xVal, float yVal) {
	this->x = xVal;
	this->y = yVal;
}

//////////////////////////////////////////////////////////////////////////
Vector2::~Vector2() {
}


//////////////////////////////////////////////////////////////////////////
float Vector2::length() const {
	return (float)sqrt(x * x + y * y);
}

} // end of namespace Wintermute

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

#include "engines/wintermute/math/matrix4.h"
#include "engines/wintermute/math/vector2.h"
#include <math.h>

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
Matrix4::Matrix4() {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			m[i][j] = 0.0f;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
Matrix4::~Matrix4() {
}


//////////////////////////////////////////////////////////////////////////
void Matrix4::identity() {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			m[i][j] = 0.0f;
		}
	}
	m[0][0] = 1.0f;
	m[1][1] = 1.0f;
	m[2][2] = 1.0f;
	m[3][3] = 1.0f;

}

//////////////////////////////////////////////////////////////////////////
void Matrix4::rotationZ(float angle) {
	identity();

	m[0][0] = cos(angle);
	m[1][1] = cos(angle);
	m[0][1] = sin(angle);
	m[1][0] = -sin(angle);
}

//////////////////////////////////////////////////////////////////////////
void Matrix4::transformVector2(Vector2 &vec) {
	float norm;

	norm = m[0][3] * vec.x + m[1][3] * vec.y + m[3][3];

	float x = (m[0][0] * vec.x + m[1][0] * vec.y + m[3][0]) / norm;
	float y = (m[0][1] * vec.x + m[1][1] * vec.y + m[3][1]) / norm;

	vec.x = x;
	vec.y = y;
}

} // end of namespace Wintermute

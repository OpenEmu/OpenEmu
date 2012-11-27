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

#ifndef WINTERMUTE_VECTOR2_H
#define WINTERMUTE_VECTOR2_H

namespace Wintermute {

class Vector2 {
public:
	Vector2();
	Vector2(float x, float y);
	~Vector2();

	float length() const;

	inline Vector2 &operator= (const Vector2 &other) {
		x = other.x;
		y = other.y;

		return *this;
	}

	inline Vector2 operator+ (const Vector2 &other) const {
		return Vector2(x + other.x, y + other.y);
	}

	inline Vector2 operator- (const Vector2 &other) const {
		return Vector2(x - other.x, y - other.y);
	}

	inline Vector2 operator* (const float scalar) const {
		return Vector2(x * scalar, y * scalar);
	}

	inline Vector2 &operator+= (const Vector2 &other) {
		x += other.x;
		y += other.y;

		return *this;
	}


	float x;
	float y;
};

} // end of namespace Wintermute

#endif

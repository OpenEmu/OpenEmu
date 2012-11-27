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

#ifndef LASTEXPRESS_ENTITY39_H
#define LASTEXPRESS_ENTITY39_H

#include "lastexpress/entities/entity.h"

namespace LastExpress {

class LastExpressEngine;

class Entity39 : public Entity {
public:
	Entity39(LastExpressEngine *engine);
	~Entity39() {}

	/**
	 * Setup Chapter 1
	 */
	DECLARE_FUNCTION(chapter1)

	/**
	 * Setup Chapter 2
	 */
	DECLARE_FUNCTION(chapter2)

	/**
	 * Setup Chapter 3
	 */
	DECLARE_FUNCTION(chapter3)

	/**
	 * Setup Chapter 4
	 */
	DECLARE_FUNCTION(chapter4)

	/**
	 * Setup Chapter 5
	 */
	DECLARE_FUNCTION(chapter5)

	/**
	 * Process function
	 */
	DECLARE_FUNCTION(process)

private:
	char _sequence[12];
	int _counter;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_ENTITY39_H

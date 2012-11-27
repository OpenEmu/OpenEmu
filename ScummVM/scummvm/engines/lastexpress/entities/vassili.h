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

#ifndef LASTEXPRESS_VASSILI_H
#define LASTEXPRESS_VASSILI_H

#include "lastexpress/entities/entity.h"

namespace LastExpress {

class LastExpressEngine;

class Vassili : public Entity {
public:
	Vassili(LastExpressEngine *engine);
	~Vassili() {}

	/**
	 * Resets the entity
	 */
	DECLARE_FUNCTION(reset)

	/**
	 * Draws the entity
	 *
	 * @param sequence The sequence to draw
	 */
	DECLARE_FUNCTION_1(draw, const char *sequence)

	/**
	 * Saves the game
	 *
	 * @param savegameType The type of the savegame
	 * @param param        The param for the savegame (EventIndex or TimeValue)
	 */
	DECLARE_FUNCTION_2(savegame, SavegameType savegameType, uint32 param)

	/**
	 * Setup Chapter 1
	 */
	DECLARE_FUNCTION(chapter1)

	/**
	 * Handle Chapter 1 events
	 */
	DECLARE_FUNCTION(chapter1Handler)

	DECLARE_FUNCTION(function6)
	DECLARE_FUNCTION(function7)
	DECLARE_FUNCTION(function8)
	DECLARE_FUNCTION(function9)
	DECLARE_FUNCTION(seizure)
	DECLARE_FUNCTION(drawInBed)

	/**
	 * Setup Chapter 2
	 */
	DECLARE_FUNCTION(chapter2)

	DECLARE_FUNCTION(sleeping)

	/**
	 * Setup Chapter 3
	 */
	DECLARE_FUNCTION(chapter3)

	DECLARE_FUNCTION(stealEgg)

	/**
	 * Setup Chapter 4
	 */
	DECLARE_FUNCTION(chapter4)

	/**
	 * Handle Chapter 4 events
	 */
	DECLARE_FUNCTION(chapter4Handler)

	/**
	 * Setup Chapter 5
	 */
	DECLARE_FUNCTION(chapter5)
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_VASSILI_H

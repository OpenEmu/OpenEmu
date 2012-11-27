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

#ifndef LASTEXPRESS_MAX_H
#define LASTEXPRESS_MAX_H

#include "lastexpress/entities/entity.h"

namespace LastExpress {

class LastExpressEngine;

class Max : public Entity {
public:
	Max(LastExpressEngine *engine);
	~Max() {}

	/**
	 * Resets the entity
	 */
	DECLARE_FUNCTION(reset)

	/**
	 * Plays sound
	 *
	 * @param filename The sound filename
	 */
	DECLARE_FUNCTION_1(playSound, const char *filename)

	/**
	* Draws the entity
	*
	* @param savepoint The savepoint
	*                    - The sequence to draw
	*/
	DECLARE_FUNCTION_NOSETUP(draw)

	/**
	 * Handles entering/exiting a compartment.
	 *
	 * @param sequence    The sequence to draw
	 * @param compartment The compartment
	 */
	DECLARE_FUNCTION_2(enterExitCompartment, const char *sequence, ObjectIndex compartment)

	/**
	 * Saves the game
	 *
	 * @param savegameType The type of the savegame
	 * @param param        The param for the savegame (EventIndex or TimeValue)
	 */
	DECLARE_FUNCTION_2(savegame, SavegameType savegameType, uint32 param)

	/**
	 * Handle Chapter 1 & 2 events
	 */
	DECLARE_FUNCTION(chapter12_handler)

	DECLARE_FUNCTION(function7)

	/**
	 * Handle Chapter 4 events
	 */
	DECLARE_FUNCTION(chapter4Handler)

	DECLARE_FUNCTION(function9)

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
	 * Handle Chapter 3 events
	 */
	DECLARE_FUNCTION(chapter3Handler)

	DECLARE_FUNCTION(freeFromCage)
	DECLARE_FUNCTION(function15)

	/**
	 * Setup Chapter 4
	 */
	DECLARE_FUNCTION(chapter4)

	DECLARE_FUNCTION(function17)

	/**
	 * Setup Chapter 5
	 */
	DECLARE_FUNCTION(chapter5)
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_MAX_H

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

#ifndef LASTEXPRESS_MAHMUD_H
#define LASTEXPRESS_MAHMUD_H

#include "lastexpress/entities/entity.h"

namespace LastExpress {

class LastExpressEngine;

class Mahmud : public Entity {
public:
	Mahmud(LastExpressEngine *engine);
	~Mahmud() {}

	/**
	 * Resets the entity
	 */
	DECLARE_FUNCTION(reset)

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
	 * Handles entering/exiting a compartment.
	 *
	 * @param sequence    The sequence to draw
	 * @param compartment The compartment
	 * @param ticks       The time ticks
	 * @param object      The object for loading the scene
	 */
	DECLARE_FUNCTION_4(enterExitCompartment2, const char *sequence, ObjectIndex compartment, uint32 ticks, ObjectIndex object)

	/**
	 * Plays sound
	 *
	 * @param filename The sound filename
	 */
	DECLARE_FUNCTION_1(playSound, const char *filename)

	/**
	 * Plays sound
	 *
	 * @param filename The sound filename
	 */
	DECLARE_FUNCTION_1(playSoundMertens, const char *filename)

	/**
	 * Updates parameter 2 using time value
	 *
	 * @param savepoint The savepoint
	 *                    - Time to add
	 */
	DECLARE_FUNCTION_NOSETUP(updateFromTime)

	/**
	 * Saves the game
	 *
	 * @param savegameType The type of the savegame
	 * @param param        The param for the savegame (EventIndex or TimeValue)
	 */
	DECLARE_FUNCTION_2(savegame, SavegameType savegameType, uint32 param)

	/**
	 * Updates the entity
	 *
	 * @param car            The car
	 * @param entityPosition The entity position
	 */
	DECLARE_FUNCTION_2(updateEntity, CarIndex car, EntityPosition entityPosition)

	DECLARE_FUNCTION_2(function10, ObjectIndex, bool)
	DECLARE_FUNCTION(function11)
	DECLARE_FUNCTION(function12)
	DECLARE_FUNCTION(function13)

	/**
	 * Handle chapters events
	 */
	DECLARE_FUNCTION(chaptersHandler)

	/**
	 * Setup Chapter 1
	 */
	DECLARE_FUNCTION(chapter1)

	/**
	 * Reset chapter data
	 */
	DECLARE_FUNCTION(resetChapter)

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
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_MAHMUD_H

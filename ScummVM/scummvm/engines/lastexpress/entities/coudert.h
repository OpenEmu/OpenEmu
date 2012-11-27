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

#ifndef LASTEXPRESS_COUDERT_H
#define LASTEXPRESS_COUDERT_H

#include "lastexpress/entities/entity.h"
namespace LastExpress {

class LastExpressEngine;

class Coudert : public Entity {
public:
	Coudert(LastExpressEngine *engine);
	~Coudert() {}

	/**
	 * Resets the entity
	 */
	DECLARE_FUNCTION(reset)

	/**
	 * Handle meeting Coudert with the blooded jacket
	 *
	 * @param sequence The sequence to draw
	 */
	DECLARE_FUNCTION_1(bloodJacket, const char *sequence)

	/**
	 * Handles entering/exiting a compartment.
	 *
	 * @param sequence    The sequence to draw
	 * @param compartment The compartment
	 */
	DECLARE_FUNCTION_2(enterExitCompartment, const char *sequence, ObjectIndex compartment)

	/**
	 * Process callback action when the entity direction is not kDirectionRight
	 */
	DECLARE_FUNCTION(callbackActionOnDirection)

	/**
	* Handles entering/exiting a compartment.
	*
	* @param sequence        The sequence to draw
	* @param compartment     The compartment
	* @param entityPosition1 The entity position 1
	* @param entityPosition2 The entity position 2
	*/
	DECLARE_FUNCTION_4(enterExitCompartment2, const char *sequence, ObjectIndex compartment, EntityPosition entityPosition1, EntityPosition entityPosition2)

	/**
	 * Plays sound
	 *
	 * @param filename The sound filename
	 */
	DECLARE_FUNCTION_1(playSound, const char *filename)

	/**
	 * Plays sound
	 *
	 * @param savepoint The savepoint
	 *                    - the sound filename
	 */
	DECLARE_FUNCTION_NOSETUP(playSound16)

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

	/**
	 * Updates parameter 2 using time value
	 *
	 * @param time The time to add
	 */
	DECLARE_FUNCTION_1(updateFromTime, uint32 time)

	/**
	 * Updates parameter 2 using ticks value
	 *
	 * @param ticks The number of ticks to add
	 */
	DECLARE_FUNCTION_1(updateFromTicks, uint32 ticks)

	DECLARE_FUNCTION_1(excuseMe, EntityIndex entity)
	DECLARE_FUNCTION_2(function13, bool, EntityIndex entity)
	DECLARE_FUNCTION_1(function14, EntityIndex entity)
	DECLARE_FUNCTION_1(function15, bool)
	DECLARE_FUNCTION(function16)
	DECLARE_FUNCTION_1(function17, bool)
	DECLARE_FUNCTION(function18)
	DECLARE_FUNCTION_1(function19, bool)

	/**
	 * ???
	 *
	 * @param object1 The first object index
	 * @param object2 The second object index
	 */
	DECLARE_FUNCTION_2(function20, ObjectIndex object1, ObjectIndex object2)

	DECLARE_FUNCTION(function21)
	DECLARE_FUNCTION(function22)
	DECLARE_FUNCTION(function23)
	DECLARE_FUNCTION(visitCompartmentF)
	DECLARE_FUNCTION(function25)
	DECLARE_FUNCTION(function26)
	DECLARE_FUNCTION(function27)
	DECLARE_FUNCTION(visitCompartmentB)
	DECLARE_FUNCTION(visitCompartmentA)

	/**
	 * ???
	 *
	 * @param compartment The compartment
	 */
	DECLARE_FUNCTION_1(function30, ObjectIndex compartment)

	DECLARE_FUNCTION_1(function31, uint32)
	DECLARE_FUNCTION(function32)
	DECLARE_FUNCTION(function33)
	DECLARE_FUNCTION_1(function34, bool)
	DECLARE_FUNCTION_1(function35, bool)

	/**
	 * Setup Chapter 1
	 */
	DECLARE_FUNCTION(chapter1)
	DECLARE_FUNCTION(function37)
	DECLARE_FUNCTION(function38)
	DECLARE_FUNCTION(function39)

	/**
	 * Handle Chapter 1 events
	 */
	DECLARE_FUNCTION(chapter1Handler)

	DECLARE_FUNCTION(function41)

	/**
	 * Setup Chapter 2
	 */
	DECLARE_FUNCTION(chapter2)

	DECLARE_FUNCTION(function43)

	/**
	 * Setup Chapter 3
	 */
	DECLARE_FUNCTION(chapter3)

	DECLARE_FUNCTION(function45)
	DECLARE_FUNCTION(function46)
	DECLARE_FUNCTION_1(function47, bool)
	DECLARE_FUNCTION(function48)
	DECLARE_FUNCTION(function49)
	DECLARE_FUNCTION(function50)
	DECLARE_FUNCTION(function51)

	/**
	 * Setup Chapter 4
	 */
	DECLARE_FUNCTION(chapter4)

	DECLARE_FUNCTION(function53)
	DECLARE_FUNCTION(function54)
	DECLARE_FUNCTION(function55)
	DECLARE_FUNCTION(function56)

	/**
	 * Setup Chapter 5
	 */
	DECLARE_FUNCTION(chapter5)

	/**
	 * Handle Chapter 5 events
	 */
	DECLARE_FUNCTION(chapter5Handler)

	DECLARE_FUNCTION(function59)
	DECLARE_FUNCTION(function60)
	DECLARE_FUNCTION(function61)
	DECLARE_FUNCTION(function62)

	DECLARE_NULL_FUNCTION()

private:
	void visitCompartment(const SavePoint &savepoint, EntityPosition position, const char *seq1, ObjectIndex compartment, const char *seq2, const char *seq3, EntityPosition sittingPosition, ObjectIndex object, const char *seq4);
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_COUDERT_H

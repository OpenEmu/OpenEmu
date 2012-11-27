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

#ifndef LASTEXPRESS_MERTENS_H
#define LASTEXPRESS_MERTENS_H

#include "lastexpress/entities/entity.h"

namespace LastExpress {

class LastExpressEngine;

class Mertens : public Entity {
private:
	// The type of action when entering Tyler compartment
	enum MertensActionType {
		kMertensActionNone = 0,
		kMertensAction1 = 1,
		kMertensAction2 = 2,
		kMertensAction3 = 3
	};

public:
	Mertens(LastExpressEngine *engine);
	~Mertens() {}

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
	 * Handles entering/exiting a compartment and updates position/play animation
	 *
	 * @param sequence    The sequence to draw
	 * @param compartment The compartment
	 */
	DECLARE_FUNCTION_2(enterExitCompartment2, const char *sequence, ObjectIndex compartment)

	/**
	 * Handles entering/exiting a compartment.
	 *
	 * @param sequence        The sequence to draw
	 * @param compartment     The compartment
	 * @param entityPosition1 The entity position
	 * @param entityPosition1 The entity position to check
	 *
	 * @note We are not using the shared function due to too many differences
	 */
	DECLARE_FUNCTION_4(enterExitCompartment3, const char *sequence, ObjectIndex compartment, EntityPosition entityPosition1, EntityPosition entityPosition2)

	/**
	 * Process callback action when the entity direction is not kDirectionRight
	 */
	DECLARE_FUNCTION(callbackActionOnDirection)

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
	DECLARE_FUNCTION_1(playSound16, const char *filename)

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

	DECLARE_FUNCTION_1(function11, uint32 time)

	/**
	* Says "Bonsoir" to another character
	*
	* @param entity The entity
	*/
	DECLARE_FUNCTION_1(bonsoir, EntityIndex entity)
	DECLARE_FUNCTION_2(function13, bool, bool)
	DECLARE_FUNCTION_1(function14, EntityIndex entity)
	DECLARE_FUNCTION_1(function15, bool)
	DECLARE_FUNCTION_1(function16, bool)
	DECLARE_FUNCTION(function17)
	DECLARE_FUNCTION(function18)
	DECLARE_FUNCTION(function19)
	DECLARE_FUNCTION(function20)

	/**
	 * ???
	 *
	 * @param object1 First object index
	 * @param object2 Second object index
	 */
	DECLARE_FUNCTION_2(function21, ObjectIndex object1, ObjectIndex object2)
	DECLARE_FUNCTION(function22)
	DECLARE_FUNCTION(function23)
	DECLARE_FUNCTION(function24)
	DECLARE_FUNCTION(function25)
	DECLARE_FUNCTION_1(function26, bool)
	DECLARE_FUNCTION_1(tylerCompartment, MertensActionType action)
	DECLARE_FUNCTION_1(function28, const char *soundName)
	DECLARE_FUNCTION_2(function29, const char *soundName1, const char *soundName2)
	DECLARE_FUNCTION_1(function30, MertensActionType action)
	DECLARE_FUNCTION_1(function31, MertensActionType action)
	DECLARE_FUNCTION(function32)
	DECLARE_FUNCTION(function33)

	/**
	 * Setup Chapter 1
	 */
	DECLARE_FUNCTION(chapter1)
	DECLARE_FUNCTION(function35)
	DECLARE_FUNCTION(function36)
	DECLARE_FUNCTION(function37)
	DECLARE_FUNCTION(function38)
	DECLARE_FUNCTION(function39)
	DECLARE_FUNCTION(function40)

	/**
	 * Handle Chapter 1 events
	 */
	DECLARE_FUNCTION(chapter1Handler)

	DECLARE_FUNCTION(function42)

	/**
	 * Setup Chapter 2
	 */
	DECLARE_FUNCTION(chapter2)

	DECLARE_FUNCTION(function44)

	/**
	 * Setup Chapter 3
	 */
	DECLARE_FUNCTION(chapter3)

	DECLARE_FUNCTION(function46)

	/**
	 * Setup Chapter 4
	 */
	DECLARE_FUNCTION(chapter4)

	DECLARE_FUNCTION(function48)
	DECLARE_FUNCTION(function49)

	/**
	 * Setup Chapter 5
	 */
	DECLARE_FUNCTION(chapter5)

	/**
	 * Handle Chapter 5 events
	 */
	DECLARE_FUNCTION(chapter5Handler)

	DECLARE_FUNCTION(function52)
	DECLARE_FUNCTION(function53)

	DECLARE_NULL_FUNCTION()

private:
	void loadSceneFromPosition();
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_MERTENS_H

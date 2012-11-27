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

#ifndef LASTEXPRESS_AUGUST_H
#define LASTEXPRESS_AUGUST_H

#include "lastexpress/entities/entity.h"

namespace LastExpress {

class LastExpressEngine;

class August : public Entity {
public:
	August(LastExpressEngine *engine);
	~August() {}

	/**
	 * Resets the entity
	 */
	DECLARE_FUNCTION(reset)

	/**
	 * Updates parameter 2 using time value
	 *
	 * @param time The time to add
	 */
	DECLARE_FUNCTION_1(updateFromTime, uint32 time)

	/**
	 * Draws the entity
	 *
	 * @param sequence The sequence to draw
	 */
	DECLARE_FUNCTION_1(draw, const char *sequence)

	/**
	 * Updates the position
	 *
	 * @param sequence1      The sequence to draw
	 * @param car            The car
	 * @param position       The position
	 */
	DECLARE_FUNCTION_3(updatePosition, const char *sequence1, CarIndex car, Position position)

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
	 * @param sequence    The sequence to draw
	 * @param compartment The compartment
	 */
	DECLARE_FUNCTION_2(enterExitCompartment3, const char *sequence, ObjectIndex compartment)

	/**
	 * Process callback action when the entity direction is not kDirectionRight
	 */
	DECLARE_FUNCTION(callbackActionOnDirection)

	/**
	 * Call a savepoint (or draw sequence in default case)
	 *
	 * @param sequence1   The sequence to draw in the default case
	 * @param entity      The entity
	 * @param action      The action
	 * @param sequence2   The sequence name for the savepoint
	 */
	DECLARE_FUNCTION_4(callSavepoint, const char *sequence1, EntityIndex entity, ActionIndex action, const char *sequence2)

	/**
	 * Call a savepoint
	 *
	 * @param param1 The entity
	 * @param param2 The action
	 * @param seq    The sequence name for the savepoint
	 */
	DECLARE_FUNCTION_3(callSavepointNoDrawing, EntityIndex entity, ActionIndex action, const char *sequence)

	/**
	 * Draws the entity along with another one
	 *
	 * @param sequence1   The sequence to draw
	 * @param sequence2   The sequence to draw for the second entity
	 * @param entity      The EntityIndex of the second entity
	 */
	DECLARE_FUNCTION_3(draw2, const char *sequence1, const char *sequence2, EntityIndex entity)

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
	 * Process callback action when somebody is standing in the restaurant or salon.
	 */
	DECLARE_FUNCTION(callbackActionRestaurantOrSalon)

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

	DECLARE_FUNCTION_1(function17, TimeValue timeValue)

	/**
	 * Updates the entity
	 *
	 * @param param1 The car
	 * @param param2 The entity position
	 */
	DECLARE_FUNCTION_2(updateEntity2, CarIndex car, EntityPosition entityPosition)

	DECLARE_FUNCTION_2(function19, bool, bool)

	DECLARE_FUNCTION_1(function20, bool)
	DECLARE_FUNCTION_1(function21, TimeValue timeValue)

	/**
	 * Setup Chapter 1
	 */
	DECLARE_FUNCTION(chapter1)

	DECLARE_FUNCTION_1(function23, TimeValue timeValue)
	DECLARE_FUNCTION(dinner)

	/**
	 * Handle Chapter 1 events
	 */
	DECLARE_FUNCTION(chapter1Handler)

	DECLARE_FUNCTION(function26)
	DECLARE_FUNCTION(function27)
	DECLARE_FUNCTION(function28)
	DECLARE_FUNCTION(function29)
	DECLARE_FUNCTION(restaurant)
	DECLARE_FUNCTION(function31)
	DECLARE_FUNCTION(function32)
	DECLARE_FUNCTION(function33)
	DECLARE_FUNCTION(function34)

	/**
	 * Setup Chapter 2
	 */
	DECLARE_FUNCTION(chapter2)

	/**
	 * Handle Chapter 2 events
	 */
	DECLARE_FUNCTION(chapter2Handler)

	DECLARE_FUNCTION(function37)
	DECLARE_FUNCTION(function38)
	DECLARE_FUNCTION(function39)

	/**
	 * Setup Chapter 3
	 */
	DECLARE_FUNCTION(chapter3)

	DECLARE_FUNCTION_2(function41, CarIndex car, EntityPosition entityPosition)
	DECLARE_FUNCTION_3(function42, CarIndex car, EntityPosition entityPosition, bool)

	/**
	 * Handle Chapter 3 events
	 */
	DECLARE_FUNCTION(chapter3Handler)

	DECLARE_FUNCTION(function44)
	DECLARE_FUNCTION(function45)
	DECLARE_FUNCTION(function46)
	DECLARE_FUNCTION(function47)
	DECLARE_FUNCTION(function48)
	DECLARE_FUNCTION(function49)
	DECLARE_FUNCTION(function50)
	DECLARE_FUNCTION(function51)
	DECLARE_FUNCTION(function52)
	DECLARE_FUNCTION(function53)
	DECLARE_FUNCTION(function54)
	DECLARE_FUNCTION(function55)
	DECLARE_FUNCTION(function56)

	/**
	 * Setup Chapter 4
	 */
	DECLARE_FUNCTION(chapter4)
	/**
	 * Handle Chapter 4 events
	 */
	DECLARE_FUNCTION(chapter4Handler)

	DECLARE_FUNCTION(function59)
	DECLARE_FUNCTION(function60)
	DECLARE_FUNCTION(function61)
	DECLARE_FUNCTION(function62)
	DECLARE_FUNCTION(function63)
	DECLARE_FUNCTION(function64)
	DECLARE_FUNCTION(function65)

	/**
	 * Setup Chapter 5
	 */
	DECLARE_FUNCTION(chapter5)

	/**
	 * Handle Chapter 5 events
	 */
	DECLARE_FUNCTION(chapter5Handler)

	DECLARE_FUNCTION(function68)
	DECLARE_FUNCTION(unhookCars)

	DECLARE_NULL_FUNCTION()
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_AUGUST_H

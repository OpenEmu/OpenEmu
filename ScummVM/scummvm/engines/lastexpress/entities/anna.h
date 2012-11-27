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

#ifndef LASTEXPRESS_ANNA_H
#define LASTEXPRESS_ANNA_H

#include "lastexpress/entities/entity.h"

namespace LastExpress {

class LastExpressEngine;

class Anna : public Entity {
public:
	Anna(LastExpressEngine *engine);
	~Anna() {}

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
	 * Plays sound
	 *
	 * @param filename The sound filename
	 */
	DECLARE_FUNCTION_1(playSound, const char *filename)

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

	/**
	 * Updates parameter 2 using time value
	 *
	 * @param time The time to add
	 */
	DECLARE_FUNCTION_1(updateFromTime, uint32 time)

	DECLARE_FUNCTION(function12)

	/**
	 * Draws the entity along with another one
	 *
	 * @param sequence1   The sequence to draw
	 * @param sequence2   The sequence to draw for the second entity
	 * @param entity      The EntityIndex of the second entity
	 */
	DECLARE_FUNCTION_3(draw2, const char *sequence1, const char *sequence2, EntityIndex entity)

	/**
	 * Updates parameter 2 using ticks value
	 *
	 * @param ticks The number of ticks to add
	 */
	DECLARE_FUNCTION_1(updateFromTicks, uint32 ticks)

	DECLARE_FUNCTION_2(function15, TimeValue timeValue, const char *sequence)

	/**
	 * Setup Chapter 1
	 */
	DECLARE_FUNCTION(chapter1)

	DECLARE_FUNCTION_2(function17, uint32, uint32)

	DECLARE_FUNCTION_1(function18, TimeValue timeValue)

	/**
	 * Handle Chapter 1 events
	 */
	DECLARE_FUNCTION(chapter1Handler)
	DECLARE_FUNCTION(function20)
	DECLARE_FUNCTION(function21)
	DECLARE_FUNCTION(function22)
	DECLARE_FUNCTION(function23)
	DECLARE_FUNCTION(function24)
	DECLARE_FUNCTION(function25)
	DECLARE_FUNCTION(function26)
	DECLARE_FUNCTION(function27)
	DECLARE_FUNCTION(function28)
	DECLARE_FUNCTION(function29)
	DECLARE_FUNCTION(function30)
	DECLARE_FUNCTION(function31)
	DECLARE_FUNCTION(function32)
	DECLARE_FUNCTION(function33)
	DECLARE_FUNCTION(function34)
	DECLARE_FUNCTION(function35)
	DECLARE_FUNCTION(function36)
	DECLARE_FUNCTION(function37)
	DECLARE_FUNCTION(function38)
	DECLARE_FUNCTION_2(function39, CarIndex car, EntityPosition entityPosition)
	DECLARE_FUNCTION(function40)
	DECLARE_FUNCTION(function41)

	/**
	 * Setup Chapter 2
	 */
	DECLARE_FUNCTION(chapter2)

	/**
	 * Handle Chapter 2 events
	 */
	DECLARE_FUNCTION(chapter2Handler)

	/**
	 * Setup Chapter 3
	 */
	DECLARE_FUNCTION(chapter3)
	DECLARE_FUNCTION_1(function45, bool useAction1)

	/**
	 * Handle Chapter 3 events
	 */
	DECLARE_FUNCTION(chapter3Handler)
	DECLARE_FUNCTION(function47)
	DECLARE_FUNCTION(function48)
	DECLARE_FUNCTION(leaveTableWithAugust)
	DECLARE_FUNCTION(function50)
	DECLARE_FUNCTION(function51)
	DECLARE_FUNCTION(function52)
	DECLARE_FUNCTION(function53)
	DECLARE_FUNCTION(function54)
	DECLARE_FUNCTION(function55)
	DECLARE_FUNCTION(function56)
	DECLARE_FUNCTION(function57)
	DECLARE_FUNCTION(function58)
	DECLARE_FUNCTION(function59)
	DECLARE_FUNCTION(function60)
	DECLARE_FUNCTION(function61)
	DECLARE_FUNCTION(function62)
	DECLARE_FUNCTION(function63)
	DECLARE_FUNCTION(baggage)
	DECLARE_FUNCTION(function65)

	/**
	 * Setup Chapter 4
	 */
	DECLARE_FUNCTION(chapter4)

	/**
	 * Handle Chapter 4 events
	 */
	DECLARE_FUNCTION(chapter4Handler)

	DECLARE_FUNCTION(function68)
	DECLARE_FUNCTION(function69)
	DECLARE_FUNCTION(function70)
	DECLARE_FUNCTION(function71)
	DECLARE_FUNCTION_2(function72, CarIndex car, EntityPosition entityPosition)
	DECLARE_FUNCTION(function73)

	/**
	 * Setup Chapter 5
	 */
	DECLARE_FUNCTION(chapter5)

	/**
	 * Handle Chapter 5 events
	 */
	DECLARE_FUNCTION(chapter5Handler)
	DECLARE_FUNCTION(function76)
	DECLARE_FUNCTION(function77)
	DECLARE_FUNCTION(function78)
	DECLARE_FUNCTION(function79)
	DECLARE_FUNCTION(function80)
	DECLARE_FUNCTION(finalSequence)
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_ANNA_H

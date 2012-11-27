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

#ifndef LASTEXPRESS_BOUTAREL_H
#define LASTEXPRESS_BOUTAREL_H

#include "lastexpress/entities/entity.h"

namespace LastExpress {

class LastExpressEngine;

class Boutarel : public Entity {
public:
	Boutarel(LastExpressEngine *engine);
	~Boutarel() {}

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
	 * @param sequence The sequence to draw
	 */
	DECLARE_FUNCTION_1(draw, const char *sequence)

	/**
	 * Updates parameter 2 using time value
	 *
	 * @param time The time to add
	 */
	DECLARE_FUNCTION_1(updateFromTime, uint32 time)

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
	 * Process callback action when the entity direction is not kDirectionRight
	 */
	DECLARE_FUNCTION(callbackActionOnDirection)

	/**
	 * Process callback action when somebody is standing in the restaurant or salon.
	 */
	DECLARE_FUNCTION(callbackActionRestaurantOrSalon)

	/**
	 * Updates the entity
	 *
	 * @param car            The car
	 * @param entityPosition The entity position
	 */
	DECLARE_FUNCTION_2(updateEntity, CarIndex car, EntityPosition entityPosition)

	DECLARE_FUNCTION_1(function11, bool)
	DECLARE_FUNCTION(enterTableWithMmeBoutarel)
	DECLARE_FUNCTION(leaveTableWithMmeBoutarel)
	DECLARE_FUNCTION_1(function14, bool)
	DECLARE_FUNCTION_2(function15, bool, const char *sequence)
	DECLARE_FUNCTION_2(function16, bool, const char *sequence)
	DECLARE_FUNCTION_2(function17, TimeValue timeValue, const char *sequence)
	DECLARE_FUNCTION_1(function18, TimeValue timeValue)

	/**
	 * Setup Chapter 1
	 */
	DECLARE_FUNCTION(chapter1)
	DECLARE_FUNCTION(function20)

	/**
	 * Handle Chapter 1 events
	 */
	DECLARE_FUNCTION(chapter1Handler)

	DECLARE_FUNCTION(function22)

	/**
	 * Setup Chapter 2
	 */
	DECLARE_FUNCTION(chapter2)

	/**
	 * Handle Chapter 2 events
	 */
	DECLARE_FUNCTION(chapter2Handler)

	DECLARE_FUNCTION(function25)

	/**
	 * Setup Chapter 3
	 */
	DECLARE_FUNCTION(chapter3)

	/**
	 * Handle Chapter 3 events
	 */
	DECLARE_FUNCTION(chapter3Handler)

	DECLARE_FUNCTION(function28)
	DECLARE_FUNCTION(function29)
	DECLARE_FUNCTION(function30)

	/**
	 * Setup Chapter 4
	 */
	DECLARE_FUNCTION(chapter4)

	/**
	 * Handle Chapter 4 events
	 */
	DECLARE_FUNCTION(chapter4Handler)

	DECLARE_FUNCTION(function33)
	DECLARE_FUNCTION(function34)
	DECLARE_FUNCTION(function35)

	/**
	 * Setup Chapter 5
	 */
	DECLARE_FUNCTION(chapter5)

	/**
	 * Handle Chapter 5 events
	 */
	DECLARE_FUNCTION(chapter5Handler)

	DECLARE_FUNCTION(function38)
	DECLARE_NULL_FUNCTION()
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_BOUTAREL_H

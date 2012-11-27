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

#ifndef LASTEXPRESS_PASCALE_H
#define LASTEXPRESS_PASCALE_H

#include "lastexpress/entities/entity.h"

namespace LastExpress {

class LastExpressEngine;

class Pascale : public Entity {
public:
	Pascale(LastExpressEngine *engine);
	~Pascale() {}

	/**
	 * Draws the entity
	 *
	 * @param sequence The sequence to draw
	 */
	DECLARE_FUNCTION_1(draw, const char *sequence)

	/**
	 * Process callback action when somebody is standing in the restaurant or salon.
	 */
	DECLARE_FUNCTION(callbackActionRestaurantOrSalon)

	/**
	 * Process callback action when the entity direction is not kDirectionRight
	 */
	DECLARE_FUNCTION(callbackActionOnDirection)

	/**
	 * Updates parameter 2 using time value
	 *
	 * @param time The time to add
	 */
	DECLARE_FUNCTION_1(updateFromTime, uint32 time)

	/**
	 * Updates the position
	 *
	 * @param savepoint The savepoint
	 *                    - The sequence to draw
	 *                    - The car
	 *                    - The position
	 */
	DECLARE_FUNCTION_NOSETUP(updatePosition)

	/**
	 * Plays sound
	 *
	 * @param filename The sound filename
	 */
	DECLARE_FUNCTION_1(playSound, const char *filename)

	/**
	 * Draws the entity along with another one
	 *
	 * @param savepoint The savepoint
	 *                    - The sequence to draw
	 *                    - The sequence to draw for the second entity
	 *                    - The EntityIndex of the second entity
	 */
	DECLARE_FUNCTION_NOSETUP(draw2)

	DECLARE_FUNCTION(welcomeSophieAndRebecca)
	DECLARE_FUNCTION(sitSophieAndRebecca)
	DECLARE_FUNCTION(welcomeCath)
	DECLARE_FUNCTION(function11)

	/**
	 * Setup Chapter 1
	 */
	DECLARE_FUNCTION(chapter1)

	DECLARE_FUNCTION(getMessageFromAugustToTyler)
	DECLARE_FUNCTION(sitAnna)
	DECLARE_FUNCTION(welcomeAnna)
	DECLARE_FUNCTION(serveTatianaVassili)

	/**
	 * Handle Chapter 1 events
	 */
	DECLARE_FUNCTION(chapter1Handler)

	DECLARE_FUNCTION(function18)
	DECLARE_FUNCTION(function19)

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

	DECLARE_FUNCTION(function23)
	DECLARE_FUNCTION(welcomeAbbot)

	/**
	 * Setup Chapter 4
	 */
	DECLARE_FUNCTION(chapter4)

	/**
	 * Handle Chapter 4 events
	 */
	DECLARE_FUNCTION(chapter4Handler)

	DECLARE_FUNCTION(function27)
	DECLARE_FUNCTION(messageFromAnna)
	DECLARE_FUNCTION(function29)
	DECLARE_FUNCTION(function30)

	/**
	 * Setup Chapter 5
	 */
	DECLARE_FUNCTION(chapter5)

	/**
	 * Handle Chapter 5 events
	 */
	DECLARE_FUNCTION(chapter5Handler)

	DECLARE_FUNCTION(function33)

	DECLARE_NULL_FUNCTION()
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_PASCALE_H

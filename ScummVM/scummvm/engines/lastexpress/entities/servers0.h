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

#ifndef LASTEXPRESS_SERVERS0_H
#define LASTEXPRESS_SERVERS0_H

#include "lastexpress/entities/entity.h"

namespace LastExpress {

class LastExpressEngine;

class Servers0 : public Entity {
public:
	Servers0(LastExpressEngine *engine);
	~Servers0() {}

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
	 * Updates parameter 2 using time value
	 *
	 * @param savepoint The savepoint
	 *                    - Time to add
	 */
	DECLARE_FUNCTION_NOSETUP(updateFromTime)

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
	 * Process callback action when the entity direction is not kDirectionRight
	 */
	DECLARE_FUNCTION_NOSETUP(callbackActionOnDirection)

	/**
	 * Plays sound
	 *
	 * @param filename The sound filename
	 */
	DECLARE_FUNCTION_1(playSound, const char *filename)

	DECLARE_FUNCTION(function7)
	DECLARE_FUNCTION(function8)
	DECLARE_FUNCTION(function9)
	DECLARE_FUNCTION(function10)

	/**
	 * Setup Chapter 1
	 */
	DECLARE_FUNCTION(chapter1)
	DECLARE_FUNCTION(function12)
	DECLARE_FUNCTION(function13)
	DECLARE_FUNCTION(function14)
	DECLARE_FUNCTION(function15)
	DECLARE_FUNCTION(function16)
	DECLARE_FUNCTION(function17)
	DECLARE_FUNCTION(function18)
	DECLARE_FUNCTION(function19)

	/**
	 * Handle Chapter 1 events
	 */
	DECLARE_FUNCTION(chapter1Handler)

	DECLARE_FUNCTION(function21)
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
	DECLARE_FUNCTION(function26)

	/**
	 * Setup Chapter 3
	 */
	DECLARE_FUNCTION(chapter3)

	/**
	 * Handle Chapter 3 events
	 */
	DECLARE_FUNCTION(chapter3Handler)

	DECLARE_FUNCTION(augustAnnaDateOrder)
	DECLARE_FUNCTION(function30)

	/**
	 * Setup Chapter 4
	 */
	DECLARE_FUNCTION(chapter4)

	/**
	 * Handle Chapter 4 events
	 */
	DECLARE_FUNCTION(chapter4Handler)

	DECLARE_FUNCTION(augustOrderSteak)
	DECLARE_FUNCTION(augustServeDuck)
	DECLARE_FUNCTION(function35)

	/**
	 * Setup Chapter 5
	 */
	DECLARE_FUNCTION(chapter5)

	/**
	 * Handle Chapter 5 events
	 */
	DECLARE_FUNCTION(chapter5Handler)

	DECLARE_NULL_FUNCTION()

private:
	void handleServer(const SavePoint &savepoint, const char *name, EntityIndex entity, ActionIndex action, uint *parameter, const char *name2 = "");
	void serveTable(const SavePoint &savepoint, const char *seq1, EntityIndex entity, const char *seq2, const char *seq3, const char *seq4, uint *parameter, bool shouldUpdatePosition = true, bool pushSavepoint = false, Position position = 0);
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_SERVERS0_H

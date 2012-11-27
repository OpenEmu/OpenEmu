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

#ifndef LASTEXPRESS_TRAIN_H
#define LASTEXPRESS_TRAIN_H

#include "lastexpress/entities/entity.h"

namespace LastExpress {

class LastExpressEngine;

class Train : public Entity {
public:
	Train(LastExpressEngine *engine);
	~Train() {}

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

	/**
	 * Handle Harem events
	 *
	 * @param compartment The compartment to handle
	 * @param counter     ??? (checked to decide which sound to make when knocking)
	 */
	DECLARE_FUNCTION_2(harem, ObjectIndex compartment, uint32 counter)

	/**
	 * Handles Train events
	 */
	DECLARE_FUNCTION(process)

private:
	// Helper methods
	void resetParam8();
	void handleCompartmentAction();
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_TRAIN_H

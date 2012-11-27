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

#ifndef LASTEXPRESS_GENDARMES_H
#define LASTEXPRESS_GENDARMES_H

#include "lastexpress/entities/entity.h"

#include "lastexpress/sound/sound.h"

namespace LastExpress {

class LastExpressEngine;

class Gendarmes : public Entity {
public:
	Gendarmes(LastExpressEngine *engine);
	~Gendarmes() {}

	/**
	 * Resets the entity
	 */
	DECLARE_FUNCTION(reset)

	/**
	 * Setup Chapter 1
	 */
	DECLARE_FUNCTION(chapter1)

	DECLARE_FUNCTION_1(arrestDraw, const char *sequence)
	DECLARE_FUNCTION_1(arrestPlaysound, const char *soundName)
	DECLARE_FUNCTION_1(arrestPlaysound16, const char *soundName)
	DECLARE_FUNCTION_1(arrestCallback, uint32 timeValue)

	/**
	 * Saves the game
	 *
	 * @param savegameType The type of the savegame
	 * @param param        The param for the savegame (EventIndex or TimeValue)
	 */
	DECLARE_FUNCTION_2(savegame, SavegameType savegameType, uint32 param)

	DECLARE_FUNCTION_2(arrestUpdateEntity, CarIndex car, EntityPosition entityPosition)
	DECLARE_FUNCTION_4(function9, CarIndex car, EntityPosition entityPosition, const char *sequence1, const char *sequence2)
	DECLARE_FUNCTION_3(function10, CarIndex car, EntityPosition entityPosition, ObjectIndex object)
	DECLARE_FUNCTION(chapter1Handler)
	DECLARE_FUNCTION(function12)
	DECLARE_FUNCTION(function13)

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

private:
	void arrest(const SavePoint &savepoint, bool playSound = false, SoundFlag flag = kFlagInvalid, bool checkCallback = false, bool shouldUpdateEntity = false);
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_GENDARMES_H

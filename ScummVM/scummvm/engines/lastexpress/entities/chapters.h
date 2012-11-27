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

#ifndef LASTEXPRESS_CHAPTERS_H
#define LASTEXPRESS_CHAPTERS_H

#include "lastexpress/entities/entity.h"

namespace LastExpress {

class LastExpressEngine;

class Chapters : public Entity {
public:
	Chapters(LastExpressEngine *engine);
	~Chapters() {}

	/**
	 * Saves the game
	 *
	 * @param savegameType The type of the savegame
	 * @param param        The param for the savegame (EventIndex or TimeValue)
	 */
	DECLARE_FUNCTION_2(savegame, SavegameType savegameType, uint32 param)

	/**
	 * Exit a train station
	 *
	 * @param stationName The name of the train station
	 * @param index       The index of the train station
	 */
	DECLARE_FUNCTION_2(enterStation, const char *stationName, CityIndex index)

	/**
	 * Exit a train station
	 *
	 * @param stationName The name of the train station
	 */
	DECLARE_FUNCTION_1(exitStation, const char *stationName)

	/**
	 * Setup Chapter 1
	 */
	DECLARE_FUNCTION(chapter1)

	/**
	 * Reset main entities
	 */
	DECLARE_FUNCTION(resetMainEntities)

	/**
	 * Handle end of Chapter 1 events
	 */
	DECLARE_FUNCTION(chapter1End)

	/**
	 * Init Chapter 1 data
	 */
	DECLARE_FUNCTION(chapter1Init)

	/**
	 * Handle Chapter 1 events
	 */
	DECLARE_FUNCTION(chapter1Handler)

	/**
	 * Handle switching to Chapter 2 after the end of Chapter 1
	 */
	DECLARE_FUNCTION(chapter1Next)

	/**
	 * Setup Chapter 2
	 */
	DECLARE_FUNCTION(chapter2)

	/**
	 * Init Chapter 2 data
	 */
	DECLARE_FUNCTION(chapter2Init)

	/**
	 * Handle Chapter 2 events
	 */
	DECLARE_FUNCTION(chapter2Handler)

	/**
	 * Setup Chapter 3
	 */
	DECLARE_FUNCTION(chapter3)

	/**
	 * Init Chapter 3 data
	 */
	DECLARE_FUNCTION(chapter3Init)

	/**
	 * Handle Chapter 3 events
	 */
	DECLARE_FUNCTION(chapter3Handler)

	/**
	 * Handle Vienna events
	 */
	DECLARE_FUNCTION(viennaEvents)

	/**
	 * Setup Chapter 4
	 */
	DECLARE_FUNCTION(chapter4)

	/**
	 * Init Chapter 4 data
	 */
	DECLARE_FUNCTION(chapter4Init)

	/**
	 * Handle Chapter 4 events
	 */
	DECLARE_FUNCTION(chapter4Handler)

	/**
	 * Setup Chapter 5
	 */
	DECLARE_FUNCTION(chapter5)

	/**
	 * Init Chapter 5 data
	 */
	DECLARE_FUNCTION(chapter5Init)

	/**
	 * Handle Chapter 5 events
	 */
	DECLARE_FUNCTION(chapter5Handler)

private:
	bool timeCheckEnterStation(TimeValue timeValue, uint &parameter, byte callback, const char *sequence, CityIndex cityIndex);
	bool timeCheckExitStation(TimeValue timeValue, uint &parameter, byte callback, const char *sequence);
	void enterExitStation(const SavePoint &savepoint, bool isEnteringStation);
	void enterExitHelper(bool isEnteringStation);
	void playSteam() const;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_CHAPTERS_H

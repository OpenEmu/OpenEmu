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

#ifndef LASTEXPRESS_VERGES_H
#define LASTEXPRESS_VERGES_H

#include "lastexpress/entities/entity.h"

namespace LastExpress {

class LastExpressEngine;

class Verges : public Entity {
public:
	Verges(LastExpressEngine *engine);
	~Verges() {}

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
	 * @param savepoint The savepoint
	 *                    - the sound filename
	 */
	DECLARE_FUNCTION_NOSETUP(playSound16)

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

	DECLARE_FUNCTION_1(walkBetweenCars, const char *soundName)
	DECLARE_FUNCTION_3(makeAnnouncement, CarIndex car, EntityPosition entityPosition, const char *soundName)
	DECLARE_FUNCTION(function11)
	DECLARE_FUNCTION(function12)
	DECLARE_FUNCTION_1(baggageCar, bool)

	/**
	 * Updates parameter 2 using time value
	 *
	 * @param time The time to add
	 */
	DECLARE_FUNCTION_1(updateFromTime, uint32 time)

	DECLARE_FUNCTION_2(dialog, EntityIndex entity, const char *soundName)
	DECLARE_FUNCTION_3(dialog2, EntityIndex entityIndex, const char *soundName1, const char *soundName2)
	DECLARE_FUNCTION(talkAboutPassengerList)

	/**
	 * Setup Chapter 1
	 */
	DECLARE_FUNCTION(chapter1)

	DECLARE_FUNCTION_NOSETUP(talkHarem)
	DECLARE_FUNCTION(talkPassengerList)
	DECLARE_FUNCTION(talkGendarmes)
	DECLARE_FUNCTION(askMertensToRelayAugustInvitation)
	DECLARE_FUNCTION(function23)
	DECLARE_FUNCTION(policeGettingOffTrain)
	DECLARE_FUNCTION(policeSearch)

	/**
	 * Handle Chapter 1 events
	 */
	DECLARE_FUNCTION(chapter1Handler)

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

	DECLARE_FUNCTION_1(function30, const char *soundName)
	DECLARE_FUNCTION(talkAboutMax)
	DECLARE_FUNCTION(function32)
	DECLARE_FUNCTION(function33)
	DECLARE_FUNCTION(function34)
	DECLARE_FUNCTION(organizeConcertInvitations)

	/**
	 * Setup Chapter 4
	 */
	DECLARE_FUNCTION(chapter4)

	/**
	 * Handle Chapter 4 events
	 */
	DECLARE_FUNCTION(chapter4Handler)

	DECLARE_FUNCTION(resetState)

	/**
	 * Setup Chapter 5
	 */
	DECLARE_FUNCTION(chapter5)

	/**
	 * Handle Chapter 5 events
	 */
	DECLARE_FUNCTION(chapter5Handler)

	DECLARE_FUNCTION(askPassengersToStayInCompartments)
	DECLARE_FUNCTION(end)

private:
	void talk(const SavePoint &savepoint, const char *sound1, const char *sound2);
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_VERGES_H

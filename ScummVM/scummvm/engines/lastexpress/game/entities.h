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

#ifndef LASTEXPRESS_ENTITIES_H
#define LASTEXPRESS_ENTITIES_H

/*
	Entities
	--------

	The entities structure contains 40 Entity_t structures for each entity

*/

#include "lastexpress/entities/entity.h"

#include "lastexpress/shared.h"

#include "common/rect.h"
#include "common/serializer.h"

namespace LastExpress {

class LastExpressEngine;
class Sequence;

class Entities : Common::Serializable {
public:
	Entities(LastExpressEngine *engine);
	~Entities();

	// Serializable
	void saveLoadWithSerializer(Common::Serializer &ser);
	void savePositions(Common::Serializer &ser);
	void saveCompartments(Common::Serializer &ser);

	void setup(bool isFirstChapter, EntityIndex entity);
	void setupChapter(ChapterIndex chapter);
	void reset();

	// Update & drawing

	/**
	 * Reset an entity state
	 *
	 * @param entity entity index
	 * @note remember to call the function pointer (we do not pass it our implementation)
	 */
	void resetState(EntityIndex entity);
	void updateFields() const;
	void updateSequences() const;
	void updateCallbacks();

	EntityIndex canInteractWith(const Common::Point &point) const;
	bool compare(EntityIndex entity1, EntityIndex entity2) const;

	/**
	 * Update an entity current sequence frame (and related fields)
	 *
	 * @param entity entity index
	 */
	void updateFrame(EntityIndex entity) const;
	void updatePositionEnter(EntityIndex entity, CarIndex car, Position position);
	void updatePositionExit(EntityIndex entity, CarIndex car, Position position);
	void enterCompartment(EntityIndex entity, ObjectIndex compartment, bool useCompartment1 = false);
	void exitCompartment(EntityIndex entity, ObjectIndex compartment, bool useCompartment1 = false);

	// Sequences
	void drawSequenceLeft(EntityIndex index, const char *sequence) const;
	void drawSequenceRight(EntityIndex index, const char *sequence) const;
	void clearSequences(EntityIndex index) const;

	bool updateEntity(EntityIndex entity, CarIndex car, EntityPosition position) const;
	bool hasValidFrame(EntityIndex entity) const;

	// Accessors
	Entity *get(EntityIndex entity);
	EntityData::EntityCallData *getData(EntityIndex entity) const;
	int getPosition(CarIndex car, Position position) const;
	int getCompartments(int index) const;
	int getCompartments1(int index) const;

	// Scene
	void loadSceneFromEntityPosition(CarIndex car, EntityPosition position, bool alternate = false) const;

	//////////////////////////////////////////////////////////////////////////
	// Checks
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Query if 'entity' is inside a compartment
	 *
	 * @param entity   The entity.
	 * @param car 	   The car.
	 * @param position The position.
	 *
	 * @return true if inside the compartment, false if not.
	 */
	bool isInsideCompartment(EntityIndex entity, CarIndex car, EntityPosition position) const;

	bool checkFields2(ObjectIndex object) const;

	/**
	 * Query if 'entity' is in compartment cars.
	 *
	 * @param entity The entity.
	 *
	 * @return true if in compartment cars, false if not.
	 */
	bool isInsideCompartments(EntityIndex entity) const;

	/**
	 * Query if the player is in the specified position
	 *
	 * @param car       The car.
	 * @param position  The position.
	 * @return true if player is in that position, false if not.
	 */
	bool isPlayerPosition(CarIndex car, Position position) const;

	/**
	 * Query if 'entity' is inside a train car
	 *
	 * @param entity The entity.
	 * @param car    The car.
	 *
	 * @return true if inside a train car, false if not.
	 */
	bool isInsideTrainCar(EntityIndex entity, CarIndex car) const;

	/**
	 * Query if 'entity' is in green car entrance.
	 *
	 * @param entity The entity.
	 *
	 * @return true if in the green car entrance, false if not.
	 */
	bool isInGreenCarEntrance(EntityIndex entity) const;

	/**
	 * Query if the player is in a specific car
	 *
	 * @param car The car.
	 *
	 * @return true if player is in the car, false if not.
	 */
	bool isPlayerInCar(CarIndex car) const;

	/**
	 * Query if 'entity' is going in the up or down direction.
	 *
	 * @param entity The entity.
	 *
	 * @return true if direction is up or down, false if not.
	 */
	bool isDirectionUpOrDown(EntityIndex entity) const;

	/**
	 * Query if the distance between the two entities is less 'distance'
	 *
	 * @param entity1  The first entity.
	 * @param entity2  The second entity.
	 * @param distance The distance.
	 *
	 * @return true if the distance between entities is less than 'distance', false if not.
	 */
	bool isDistanceBetweenEntities(EntityIndex entity1, EntityIndex entity2, uint distance) const;

	bool checkFields10(EntityIndex entity) const;

	/**
	 * Query if there is somebody in the restaurant or salon.
	 *
	 * @return true if somebody is in the restaurant or salon, false if not.
	 */
	bool isSomebodyInsideRestaurantOrSalon() const;

	/**
	 * Query if 'entity' is in the salon.
	 *
	 * @param entity The entity.
	 *
	 * @return true if in the salon, false if not.
	 */
	bool isInSalon(EntityIndex entity) const;

	/**
	 * Query if 'entity' is in the restaurant.
	 *
	 * @param entity The entity.
	 *
	 * @return true if in the restaurant, false if not.
	 */
	bool isInRestaurant(EntityIndex entity) const;

	/**
	 * Query if 'entity' is in Kronos salon.
	 *
	 * @param entity The entity.
	 *
	 * @return true if in Kronos salon, false if not.
	 */
	bool isInKronosSalon(EntityIndex entity) const;

	/**
	 * Query if the player is outside Alexei window.
	 *
	 * @return true if outside alexei window, false if not.
	 */
	bool isOutsideAlexeiWindow() const;

	/**
	 * Query if the player is outside Anna window.
	 *
	 * @return true if outside anna window, false if not.
	 */
	bool isOutsideAnnaWindow() const;

	/**
	 * Query if 'entity' is in the kitchen.
	 *
	 * @param entity The entity.
	 *
	 * @return true if in the kitchen, false if not.
	 */
	bool isInKitchen(EntityIndex entity) const;

	/**
	 * Query if nobody is in a compartment at that position.
	 *
	 * @param car 	   The car.
	 * @param position The position.
	 *
	 * @return true if nobody is in a compartment, false if not.
	 */
	bool isNobodyInCompartment(CarIndex car, EntityPosition position) const;

	bool checkFields19(EntityIndex entity, CarIndex car, EntityPosition position) const;

	/**
	 * Query if 'entity' is in the baggage car entrance.
	 *
	 * @param entity The entity.
	 *
	 * @return true if in the baggage car entrance, false if not.
	 */
	bool isInBaggageCarEntrance(EntityIndex entity) const;

	/**
	 * Query if 'entity' is in the baggage car.
	 *
	 * @param entity The entity.
	 *
	 * @return true if in the baggage car, false if not.
	 */
	bool isInBaggageCar(EntityIndex entity) const;

	/**
	 * Query if 'entity' is in Kronos sanctum.
	 *
	 * @param entity The entity.
	 *
	 * @return true if in Kronos sanctum, false if not.
	 */
	bool isInKronosSanctum(EntityIndex entity) const;

	/**
	 * Query if 'entity' is in Kronos car entrance.
	 *
	 * @param entity The entity.
	 *
	 * @return true if in Kronos car entrance, false if not.
	 */
	bool isInKronosCarEntrance(EntityIndex entity) const;

	/**
	 * Check distance from position.
	 *
	 * @param entity   The entity.
	 * @param position The position.
	 * @param distance The distance.
	 *
	 * @return true if distance is bigger, false otherwise.
	 */
	bool checkDistanceFromPosition(EntityIndex entity, EntityPosition position, int distance) const;

	/**
	 * Query if 'entity' is walking opposite to player.
	 *
	 * @param entity The entity.
	 *
	 * @return true if walking opposite to player, false if not.
	 */
	bool isWalkingOppositeToPlayer(EntityIndex entity) const;

	/**
	 * Query if 'entity' is female.
	 *
	 * @param entity The entity.
	 *
	 * @return true if female, false if not.
	 */
	static bool isFemale(EntityIndex entity);

	/**
	 * Query if 'entity' is married.
	 *
	 * @param entity The entity.
	 *
	 * @return true if married, false if not.
	 */
	static bool isMarried(EntityIndex entity);

private:
	static const int _compartmentsCount = 16;
	static const int _positionsCount = 100 * 10;  // 100 positions per train car

	LastExpressEngine	    *_engine;
	EntityData 			    *_header;
	Common::Array<Entity *>  _entities;

	// Compartments & positions
	uint _compartments[_compartmentsCount];
	uint _compartments1[_compartmentsCount];
	uint _positions[_positionsCount];

	void executeCallbacks();
	void incrementDirectionCounter(EntityData::EntityCallData *data) const;
	void processEntity(EntityIndex entity);

	void drawSequence(EntityIndex entity, const char *sequence, EntityDirection direction) const;
	void drawSequences(EntityIndex entity, EntityDirection direction, bool loadSequence) const;
	void loadSequence2(EntityIndex entity, Common::String sequenceName, Common::String sequenceName2, byte field30, bool loadSequence) const;

	void clearEntitySequenceData(EntityData::EntityCallData *data, EntityDirection direction) const;
	void computeCurrentFrame(EntityIndex entity) const;
	int16 getCurrentFrame(EntityIndex entity, Sequence *sequence, EntityPosition position, bool doProcessing) const;
	void processFrame(EntityIndex entity, bool keepPreviousFrame, bool dontPlaySound);
	void drawNextSequence(EntityIndex entity) const;
	void updateEntityPosition(EntityIndex entity) const;
	void copySequenceData(EntityIndex entity) const;

	bool changeCar(EntityData::EntityCallData *data, EntityIndex entity, CarIndex car, EntityPosition position, bool increment, EntityPosition newPosition, CarIndex newCar) const;

	void getSequenceName(EntityIndex entity, EntityDirection direction, Common::String &sequence1, Common::String &sequence2) const;

	void updatePositionsEnter(EntityIndex entity, CarIndex car, Position position1, Position position2, Position position3, Position position4);
	void updatePositionsExit(EntityIndex entity, CarIndex car, Position position1, Position position2);

	void resetSequences(EntityIndex entity) const;

	bool checkPosition(EntityPosition position) const;
	bool checkSequenceFromPosition(EntityIndex entity) const;
	EntityPosition getEntityPositionFromCurrentPosition() const;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_ENTITIES_H

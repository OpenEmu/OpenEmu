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

#include "lastexpress/game/entities.h"

// Data
#include "lastexpress/data/scene.h"
#include "lastexpress/data/sequence.h"

// Entities
#include "lastexpress/entities/abbot.h"
#include "lastexpress/entities/alexei.h"
#include "lastexpress/entities/alouan.h"
#include "lastexpress/entities/anna.h"
#include "lastexpress/entities/august.h"
#include "lastexpress/entities/boutarel.h"
#include "lastexpress/entities/chapters.h"
#include "lastexpress/entities/cooks.h"
#include "lastexpress/entities/coudert.h"
#include "lastexpress/entities/entity39.h"
#include "lastexpress/entities/francois.h"
#include "lastexpress/entities/gendarmes.h"
#include "lastexpress/entities/hadija.h"
#include "lastexpress/entities/ivo.h"
#include "lastexpress/entities/kahina.h"
#include "lastexpress/entities/kronos.h"
#include "lastexpress/entities/mahmud.h"
#include "lastexpress/entities/max.h"
#include "lastexpress/entities/mertens.h"
#include "lastexpress/entities/milos.h"
#include "lastexpress/entities/mmeboutarel.h"
#include "lastexpress/entities/pascale.h"
#include "lastexpress/entities/rebecca.h"
#include "lastexpress/entities/salko.h"
#include "lastexpress/entities/servers0.h"
#include "lastexpress/entities/servers1.h"
#include "lastexpress/entities/sophie.h"
#include "lastexpress/entities/tables.h"
#include "lastexpress/entities/tatiana.h"
#include "lastexpress/entities/train.h"
#include "lastexpress/entities/vassili.h"
#include "lastexpress/entities/verges.h"
#include "lastexpress/entities/vesna.h"
#include "lastexpress/entities/yasmin.h"

// Game
#include "lastexpress/game/logic.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/scenes.h"
#include "lastexpress/game/state.h"

#include "lastexpress/sound/queue.h"

#include "lastexpress/graphics.h"
#include "lastexpress/lastexpress.h"
#include "lastexpress/resource.h"

namespace LastExpress {

#define STORE_VALUE(data) ((uint)1 << (uint)data)

static const EntityPosition objectsPosition[8] = {kPosition_8200, kPosition_7500,
	                                              kPosition_6470, kPosition_5790,
	                                              kPosition_4840, kPosition_4070,
	                                              kPosition_3050, kPosition_2740};

static const EntityPosition entityPositions[41] = {kPositionNone,  kPosition_851,  kPosition_1430, kPosition_2110, kPositionNone,
	                                               kPosition_2410, kPosition_2980, kPosition_3450, kPosition_3760, kPosition_4100,
	                                               kPosition_4680, kPosition_5140, kPosition_5440, kPosition_5810, kPosition_6410,
	                                               kPosition_6850, kPosition_7160, kPosition_7510, kPosition_8514, kPositionNone,
	                                               kPositionNone,  kPositionNone,  kPosition_2086, kPosition_2690, kPositionNone,
	                                               kPosition_3110, kPosition_3390, kPosition_3890, kPosition_4460, kPosition_4770,
	                                               kPosition_5090, kPosition_5610, kPosition_6160, kPosition_6460, kPosition_6800,
	                                               kPosition_7320, kPosition_7870, kPosition_8160, kPosition_8500, kPosition_9020,
	                                               kPosition_9269};

#define ADD_ENTITY(class) \
	_entities.push_back(new class(engine));

#define COMPUTE_SEQUENCE_NAME(sequenceTo, sequenceFrom) { \
	sequenceTo = sequenceFrom; \
	for (int seqIdx = 0; seqIdx < 7; seqIdx++) \
		sequenceTo.deleteLastChar(); \
	if (isInsideTrainCar(entityIndex, kCarGreenSleeping) || isInsideTrainCar(entityIndex, kCarRedSleeping)) { \
		if (data->car < getData(kEntityPlayer)->car || (data->car == getData(kEntityPlayer)->car && data->entityPosition < getData(kEntityPlayer)->entityPosition)) \
			sequenceTo += "R.SEQ"; \
		else \
			sequenceTo += "F.SEQ"; \
	} else { \
		sequenceTo += ".SEQ"; \
	} \
}

#define TRY_LOAD_SEQUENCE(sequence, name, name1, name2) { \
	if (data->car == getData(kEntityPlayer)->car) \
		sequence = loadSequence1(name1, field30); \
	if (sequence) { \
		name = name1; \
	} else { \
		if (name2 != "") \
			sequence = loadSequence1(name2, field30); \
		name = (sequence ? name2 : ""); \
	} \
}

//////////////////////////////////////////////////////////////////////////
// Entities
//////////////////////////////////////////////////////////////////////////
Entities::Entities(LastExpressEngine *engine) : _engine(engine) {
	_header = new EntityData();

	_entities.push_back(NULL);      // Header
	ADD_ENTITY(Anna);
	ADD_ENTITY(August);
	ADD_ENTITY(Mertens);
	ADD_ENTITY(Coudert);
	ADD_ENTITY(Pascale);
	ADD_ENTITY(Servers0);
	ADD_ENTITY(Servers1);
	ADD_ENTITY(Cooks);
	ADD_ENTITY(Verges);
	ADD_ENTITY(Tatiana);
	ADD_ENTITY(Vassili);
	ADD_ENTITY(Alexei);
	ADD_ENTITY(Abbot);
	ADD_ENTITY(Milos);
	ADD_ENTITY(Vesna);
	ADD_ENTITY(Ivo);
	ADD_ENTITY(Salko);
	ADD_ENTITY(Kronos);
	ADD_ENTITY(Kahina);
	ADD_ENTITY(Francois);
	ADD_ENTITY(MmeBoutarel);
	ADD_ENTITY(Boutarel);
	ADD_ENTITY(Rebecca);
	ADD_ENTITY(Sophie);
	ADD_ENTITY(Mahmud);
	ADD_ENTITY(Yasmin);
	ADD_ENTITY(Hadija);
	ADD_ENTITY(Alouan);
	ADD_ENTITY(Gendarmes);
	ADD_ENTITY(Max);
	ADD_ENTITY(Chapters);
	ADD_ENTITY(Train);

	// Special case for tables
	_entities.push_back(new Tables(engine, kEntityTables0));
	_entities.push_back(new Tables(engine, kEntityTables1));
	_entities.push_back(new Tables(engine, kEntityTables2));
	_entities.push_back(new Tables(engine, kEntityTables3));
	_entities.push_back(new Tables(engine, kEntityTables4));
	_entities.push_back(new Tables(engine, kEntityTables5));

	ADD_ENTITY(Entity39);

	// Init compartments & positions
	memset(&_compartments, 0, sizeof(_compartments));
	memset(&_compartments1, 0, sizeof(_compartments1));
	memset(&_positions, 0, sizeof(_positions));
}

Entities::~Entities() {
	SAFE_DELETE(_header);

	for (uint i = 0; i < _entities.size(); i++)
		SAFE_DELETE(_entities[i]);

	_entities.clear();

	// Zero passed pointers
	_engine = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Accessors
//////////////////////////////////////////////////////////////////////////
Entity *Entities::get(EntityIndex entity) {
	assert((uint)entity < _entities.size());

	if (entity == kEntityPlayer)
		error("[Entities::get] Cannot get entity for kEntityPlayer");

	return _entities[entity];
}

EntityData::EntityCallData *Entities::getData(EntityIndex entity) const {
	assert((uint)entity < _entities.size());

	if (entity == kEntityPlayer)
		return _header->getCallData();

	return _entities[entity]->getData();
}

int Entities::getPosition(CarIndex car, Position position) const {
	int index = 100 * car + position;

	if (car > 10)
		error("[Entities::getPosition] Trying to access an invalid car (was: %d, valid:0-9)", car);

	if (position > 100)
		error("[Entities::getPosition] Trying to access an invalid position (was: %d, valid:0-100)", position);

	return _positions[index];
}

int Entities::getCompartments(int index) const {
	if (index >= _compartmentsCount)
		error("[Entities::getCompartments] Trying to access an invalid compartment (was: %d, valid:0-15)", index);

	return _compartments[index];
}

int Entities::getCompartments1(int index) const {
	if (index >= _compartmentsCount)
		error("[Entities::getCompartments] Trying to access an invalid compartment (was: %d, valid:0-15)", index);

	return _compartments1[index];
}

//////////////////////////////////////////////////////////////////////////
// Savegame
//////////////////////////////////////////////////////////////////////////
void Entities::saveLoadWithSerializer(Common::Serializer &s) {
	_header->saveLoadWithSerializer(s);
	for (uint i = 1; i < _entities.size(); i++)
		_entities[i]->saveLoadWithSerializer(s);
}

void Entities::savePositions(Common::Serializer &s) {
	for (uint i = 0; i < (uint)_positionsCount; i++)
		s.syncAsUint32LE(_positions[i]);
}

void Entities::saveCompartments(Common::Serializer &s) {
	for (uint i = 0; i < (uint)_compartmentsCount; i++)
		s.syncAsUint32LE(_compartments[i]);

	for (uint i = 0; i < (uint)_compartmentsCount; i++)
		s.syncAsUint32LE(_compartments1[i]);
}

//////////////////////////////////////////////////////////////////////////
// Setup
//////////////////////////////////////////////////////////////////////////
void Entities::setup(bool isFirstChapter, EntityIndex entityIndex) {
	setupChapter(isFirstChapter ? kChapter1 : kChapterAll);

	bool flag_4 = false;

	if (!isFirstChapter) {
		getFlags()->flag_4 = false;

		if (entityIndex) {
			getSavePoints()->call(kEntityPlayer, entityIndex, kActionNone);
			flag_4 = getFlags()->flag_4;
		}
	}

	getFlags()->flag_4 = flag_4;
	if (!getFlags()->flag_4)
		getScenes()->loadScene(getState()->scene);
}

void Entities::setupChapter(ChapterIndex chapter) {
	if (chapter) {
		// Reset current call, inventory item & draw sequences
		for (uint i = 1; i < _entities.size(); i++) {
			getData((EntityIndex)i)->currentCall = 0;
			getData((EntityIndex)i)->inventoryItem = kItemNone;

			clearSequences((EntityIndex)i);
		}

		// Init compartments & positions
		memset(&_compartments, 0, sizeof(_compartments));
		memset(&_compartments1, 0, sizeof(_compartments1));
		memset(&_positions, 0, sizeof(_positions));

		getSoundQueue()->resetQueue(kSoundType13);
	}

	// we skip the header when doing entity setup
	for (uint i = 1; i < _entities.size(); i++) {
		// Special case of chapters (prevents infinite loop as we will be called from Chapters functions when changing chapters)
		if (i == kEntityChapters && chapter >= 2)
			continue;

		_entities[i]->setup(chapter);
	}
}

void Entities::reset() {
	// Reset header
	delete _header;
	_header = new EntityData();

	for (uint i = 1; i < _entities.size(); i++)
		resetSequences((EntityIndex)i);

	getScenes()->resetDoorsAndClock();
}

//////////////////////////////////////////////////////////////////////////
// State & Sequences
//////////////////////////////////////////////////////////////////////////

EntityIndex Entities::canInteractWith(const Common::Point &point) const {
	if (!getFlags()->isGameRunning)
		return kEntityPlayer;

	EntityIndex index = kEntityPlayer;
	int location = 10000;

	// Check if there is an entity we can interact with
	for (uint i = 0; i < _entities.size(); i++) {

		// Skip entities with no current frame
		if (!getData((EntityIndex)i)->frame)
			continue;

		FrameInfo *info =  getData((EntityIndex)i)->frame->getInfo();

		// Check the hotspot
		if (info->hotspot.contains(point)) {

			// If closer to us, update with its values
			if (location > info->location) {
				location = info->location;
				index = (EntityIndex)i;
			}
		}
	}

	// Check if we found an entity
	if (!index)
		return kEntityPlayer;

	// Check that there is an item to interact with
	if (!getData(index)->inventoryItem)
		return kEntityPlayer;

	return index;
}

void Entities::resetState(EntityIndex entityIndex) {
	getData(entityIndex)->currentCall = 0;
	getData(entityIndex)->inventoryItem = kItemNone;

	if (getSoundQueue()->isBuffered(entityIndex))
		getSoundQueue()->removeFromQueue(entityIndex);

	clearSequences(entityIndex);

	if (entityIndex == kEntity39)
		entityIndex = kEntityPlayer;

	if (entityIndex > kEntityChapters)
		return;

	// reset compartments and positions for this entity
	for (int i = 0; i < _positionsCount; i++)
		_positions[i] &= ~STORE_VALUE(entityIndex);

	for (int i = 0; i < _compartmentsCount; i++) {
		_compartments[i] &= ~STORE_VALUE(entityIndex);
		_compartments1[i] &= ~STORE_VALUE(entityIndex);
	}

	getLogic()->updateCursor();
}


void Entities::updateFields() const {
	if (!getFlags()->isGameRunning)
		return;

	for (int i = 0; i < (int)_entities.size(); i++) {

		if (!getSavePoints()->getCallback((EntityIndex)i))
			continue;

		EntityData::EntityCallData *data = getData((EntityIndex)i);
		int positionDelta = data->field_4A3 * 10;
		switch (data->direction) {
		default:
			break;

		case kDirectionUp:
			if (data->entityPosition >= 10000 - positionDelta)
				data->entityPosition = (EntityPosition)(data->entityPosition + positionDelta);
			break;

		case kDirectionDown:
			if (data->entityPosition > positionDelta)
				data->entityPosition = (EntityPosition)(data->entityPosition - positionDelta);
			break;

		case kDirectionLeft:
			data->currentFrame++;
			break;

		case kDirectionRight:
			data->field_4A1 += 9;
			break;

		case kDirectionSwitch:
			if (data->directionSwitch == kDirectionRight)
				data->field_4A1 += 9;
			break;

		}
	}
}

void Entities::updateFrame(EntityIndex entityIndex) const {
	Sequence *sequence = NULL;
	int16 *currentFrame = NULL;
	bool found = false;

	if (getData(entityIndex)->direction == kDirectionSwitch) {
		sequence = getData(entityIndex)->sequence2;
		currentFrame = &getData(entityIndex)->currentFrame2;
	} else {
		sequence = getData(entityIndex)->sequence;
		currentFrame = &getData(entityIndex)->currentFrame;
	}

	if (!sequence)
		return;

	// Save current values
	int16 oldFrame = *currentFrame;
	int16 field_4A1 = getData(entityIndex)->field_4A1;

	do {
		// Check we do not get past the end
		if (*currentFrame >= (int)sequence->count() - 1)
			break;

		// Get the proper frame
		FrameInfo *info = sequence->getFrameInfo((uint16)*currentFrame);

		if (info->field_33 & 8) {
			found = true;
		} else {
			if (info->soundAction == 35)
				found = true;

			getData(entityIndex)->field_4A1 += info->field_30;

			// Progress to the next frame
			++*currentFrame;
		}
	} while (!found);

	// Restore old values
	if (!found) {
		*currentFrame = oldFrame;
		getData(entityIndex)->field_4A1 = field_4A1;
	}
}

void Entities::updateSequences() const {
	if (!getFlags()->isGameRunning)
		return;

	// Update the train clock & doors
	getScenes()->updateDoorsAndClock();

	//////////////////////////////////////////////////////////////////////////
	// First pass: Drawing
	//////////////////////////////////////////////////////////////////////////
	for (uint i = 1; i < _entities.size(); i++) {
		EntityIndex entityIndex = (EntityIndex)i;

		if (!getSavePoints()->getCallback(entityIndex))
			continue;

		EntityData::EntityCallData *data = getData(entityIndex);

		if (data->frame) {
			getScenes()->removeFromQueue(data->frame);
			SAFE_DELETE(data->frame);
		}

		if (data->frame1) {
			getScenes()->removeFromQueue(data->frame1);
			SAFE_DELETE(data->frame1);
		}

		if (data->direction == kDirectionSwitch) {

			// Clear sequence 2
			if (data->sequence)
				SAFE_DELETE(data->sequence);

			// Replace by sequence 3 if available
			if (data->sequence2) {
				data->sequence = data->sequence2;
				data->sequenceName = data->sequenceName2;

				data->sequence2 = NULL;
				data->sequenceName2 = "";
			}

			data->direction = data->directionSwitch;
			data->currentFrame = -1;
			data->field_49B = 0;
		}

		// Draw sequences
		drawSequences(entityIndex, data->direction, false);
	}

	//////////////////////////////////////////////////////////////////////////
	// Second pass: Load sequences for next pass
	//////////////////////////////////////////////////////////////////////////
	for (uint i = 1; i < _entities.size(); i++) {
		EntityIndex entityIndex = (EntityIndex)i;

		if (!getSavePoints()->getCallback(entityIndex))
			continue;

		EntityData::EntityCallData *data = getData(entityIndex);
		byte field30 = (data->direction == kDirectionLeft ? entityIndex + 35 : 15);

		if (data->sequenceName != "" && !data->sequence) {
			data->sequence = loadSequence1(data->sequenceName, field30);

			// If sequence 2 was loaded correctly, remove the copied name
			// otherwise, compute new name
			if (data->sequence) {
				data->sequenceNameCopy = "";
			} else {
				Common::String sequenceName;

				// Left and down directions
				if (data->direction == kDirectionLeft || data->direction == kDirectionRight) {
					COMPUTE_SEQUENCE_NAME(sequenceName, data->sequenceName);

					// Try loading the sequence
					data->sequence = loadSequence1(sequenceName, field30);
				}

				// Update sequence names
				data->sequenceNameCopy = (data->sequence ? "" : data->sequenceName);
				data->sequenceName = (data->sequence ? sequenceName : "");
			}
		}

		// Update sequence 3
		if (data->sequenceName2 != "" && !data->sequence2) {

			if (data->car == getData(kEntityPlayer)->car)
				data->sequence2 = loadSequence1(data->sequenceName2, field30);

			if (!data->sequence2) {
				Common::String sequenceName;

				// Left and down directions
				if (data->directionSwitch == kDirectionLeft || data->directionSwitch == kDirectionRight) {
					COMPUTE_SEQUENCE_NAME(sequenceName, data->sequenceName2);

					// Try loading the sequence
					data->sequence2 = loadSequence1(sequenceName, field30);
				}

				// Update sequence names
				data->sequenceName2 = (data->sequence2 ? sequenceName : "");
			}
		}
	}
}

void Entities::resetSequences(EntityIndex entityIndex) const {

	// Reset direction
	if (getData(entityIndex)->direction == kDirectionSwitch) {
		getData(entityIndex)->direction = getData(entityIndex)->directionSwitch;
		getData(entityIndex)->field_49B = 0;
		getData(entityIndex)->currentFrame = -1;
	}

	SAFE_DELETE(getData(entityIndex)->frame);
	SAFE_DELETE(getData(entityIndex)->frame1);

	SAFE_DELETE(getData(entityIndex)->sequence);
	SAFE_DELETE(getData(entityIndex)->sequence2);
	SAFE_DELETE(getData(entityIndex)->sequence3);

	getData(entityIndex)->field_4A9 = false;
	getData(entityIndex)->field_4AA = false;

	strcpy((char *)&getData(entityIndex)->sequenceNameCopy, "");
	strcpy((char *)&getData(entityIndex)->sequenceName, "");
	strcpy((char *)&getData(entityIndex)->sequenceName2, "");

	getScenes()->resetQueue();
}

//////////////////////////////////////////////////////////////////////////
// Callbacks
//////////////////////////////////////////////////////////////////////////
void Entities::updateCallbacks() {
	if (!getFlags()->isGameRunning)
		return;

	getFlags()->flag_entities_0 = false;

	if (getFlags()->flag_entities_1) {
		executeCallbacks();
		getFlags()->flag_entities_0 = true;
	} else {
		getFlags()->flag_entities_1 = true;
		executeCallbacks();
		getFlags()->flag_entities_1 = false;
	}
}

void Entities::executeCallbacks() {
	for (uint i = 1; i < _entities.size(); i++) {
		if (getFlags()->flag_entities_0)
			break;

		if (getSavePoints()->getCallback((EntityIndex)i))
			processEntity((EntityIndex)i);
	}

	if (getFlags()->flag_entities_0)
		return;

	bool processed = true;
	do {
		processed = true;
		for (int i = 1; i < (int)_entities.size(); i++) {
			if (getFlags()->flag_entities_0)
				break;

			if (getSavePoints()->getCallback((EntityIndex)i)) {
				if (getData((EntityIndex)i)->doProcessEntity) {
					processed = false;
					processEntity((EntityIndex)i);
				}
			}
		}
	} while (!processed);
}

//////////////////////////////////////////////////////////////////////////
// Processing
//////////////////////////////////////////////////////////////////////////
void Entities::incrementDirectionCounter(EntityData::EntityCallData *data) const {
	data->doProcessEntity = false;

	if (data->direction == kDirectionRight || (data->direction == kDirectionSwitch && data->directionSwitch == kDirectionRight))
		++data->field_4A1;
}

void Entities::processEntity(EntityIndex entityIndex) {
	EntityData::EntityCallData *data = getData(entityIndex);
	bool keepPreviousFrame = false;

	data->doProcessEntity = false;

	if (getData(kEntityPlayer)->car != data->car && data->direction != kDirectionRight && data->direction != kDirectionSwitch) {

		if (data->position) {
			updatePositionExit(entityIndex, data->car2, data->position);
			data->car2 = kCarNone;
			data->position = 0;
		}

		getScenes()->removeAndRedraw(&data->frame, false);
		getScenes()->removeAndRedraw(&data->frame1, false);

		incrementDirectionCounter(data);
		return;
	}

	if (data->frame1) {
		getScenes()->removeAndRedraw(&data->frame1, false);

		if (data->frame && data->frame->getInfo()->subType != kFrameType3) {
			data->frame->getInfo()->subType = kFrameTypeNone;
			getScenes()->setFlagDrawSequences();
		}
	}

	SAFE_DELETE(data->sequence3);

	if (!data->frame || !data->direction) {
		if (!data->sequence)
label_nosequence:
			drawSequences(entityIndex, data->direction, true);

		data->doProcessEntity = false;
		computeCurrentFrame(entityIndex);

		if (getFlags()->flag_entities_0 || data->doProcessEntity)
			return;

		if (data->sequence && data->currentFrame != -1 && data->currentFrame <= (int16)(data->sequence->count() - 1)) {
			processFrame(entityIndex, false, true);

			if (!getFlags()->flag_entities_0 && !data->doProcessEntity) {
				incrementDirectionCounter(data);
				return;
			}
		} else {
			if (data->direction == kDirectionRight && data->field_4A1 > 100) {
				getSavePoints()->push(kEntityPlayer, entityIndex, kActionExitCompartment);
				getSavePoints()->process();

				if (getFlags()->flag_entities_0 || data->doProcessEntity)
					return;
			}

			if (data->position) {
				updatePositionExit(entityIndex, data->car2, data->position);
				data->car2 = kCarNone;
				data->position = 0;
			}

			incrementDirectionCounter(data);
		}
		return;
	}

	if (!data->sequence)
		goto label_nosequence;

	if (data->frame->getInfo()->field_30 > (data->field_49B + 1) || (data->direction == kDirectionLeft && data->sequence->count() == 1)) {
		++data->field_49B;
	} else if (data->frame->getInfo()->field_30 <= data->field_49B || data->frame->getInfo()->keepPreviousFrame) {
		if (data->frame->getInfo()->keepPreviousFrame == 1)
			keepPreviousFrame = true;

		// Increment current frame
		++data->currentFrame;

		if (data->currentFrame > (int16)(data->sequence->count() - 1) || (data->field_4A9 && checkSequenceFromPosition(entityIndex))) {

			if (data->direction == kDirectionLeft) {
				data->currentFrame = 0;
			} else {
				keepPreviousFrame = true;
				drawNextSequence(entityIndex);

				if (getFlags()->flag_entities_0 || data->doProcessEntity)
					return;

				if (!data->sequence2) {
					updateEntityPosition(entityIndex);
					data->doProcessEntity = false;
					return;
				}

				copySequenceData(entityIndex);
			}

		}

		processFrame(entityIndex, keepPreviousFrame, false);

		if (getFlags()->flag_entities_0 || data->doProcessEntity)
			return;
	} else {
		++data->field_49B;
	}

	incrementDirectionCounter(data);
}

void Entities::computeCurrentFrame(EntityIndex entityIndex) const {
	EntityData::EntityCallData *data = getData(entityIndex);
	int16 originalCurrentFrame = data->currentFrame;

	if (!data->sequence) {
		data->currentFrame = -1;
		return;
	}

	switch (data->direction) {
	default:
		break;

	case kDirectionNone:
	case kDirectionSwitch:
		data->currentFrame = -1;
		break;

	case kDirectionUp:
	case kDirectionDown: {
		Scene *scene = getScenes()->get(getState()->scene);

		if (scene->position > 40)
			break;

		switch (scene->position) {
		default:
		case 4:
		case 19:
		case 20:
		case 21:
		case 24:
			break;

		case 1:
		case 18:
		case 22:
		case 40:
			data->currentFrame = getCurrentFrame(entityIndex, data->sequence, kPositionNone, false);
			break;

		case 2:
		case 3:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
			if (data->field_4A9) {
				if (getData(kEntityPlayer)->entityPosition >= data->entityPosition) {
					data->currentFrame = -1;
				} else {
					data->currentFrame = getCurrentFrame(entityIndex, data->sequence, getEntityPositionFromCurrentPosition(), true);

					if (data->currentFrame != -1 && originalCurrentFrame == data->currentFrame)
						if (data->currentFrame < (int)(data->sequence->count() - 2))
							data->currentFrame += 2;
				}
			} else {
				data->currentFrame = getCurrentFrame(entityIndex, data->sequence, kPositionNone, false);
			}
			break;

		case 23:
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		case 30:
		case 31:
		case 32:
		case 33:
		case 34:
		case 35:
		case 36:
		case 37:
		case 38:
		case 39:
			if (data->field_4A9) {
				if (getData(kEntityPlayer)->entityPosition <= data->entityPosition) {
					data->currentFrame = -1;
				} else {
					data->currentFrame = getCurrentFrame(entityIndex, data->sequence, getEntityPositionFromCurrentPosition(), true);

					if (data->currentFrame != -1 && originalCurrentFrame == data->currentFrame)
						if (data->currentFrame < (int)(data->sequence->count() - 2))
							data->currentFrame += 2;
				}
			} else {
				data->currentFrame = getCurrentFrame(entityIndex, data->sequence, kPositionNone, false);
			}
			break;
		}

		}
		break;


	case kDirectionLeft:
		if (data->currentFrame == -1 || data->currentFrame >= (int32)data->sequence->count()) {
			data->currentFrame = 0;
			data->field_49B = 0;
		}
		break;

	case kDirectionRight:
		bool found = false;
		bool flag = false;
		uint16 frameIndex = 0;
		byte field30 = 0;

		int16 currentFrameCopy = (!data->currentFrame && !data->field_4A1) ? -1 : data->currentFrame;

		// Process frames
		do {
			if (frameIndex >= data->sequence->count())
				break;

			FrameInfo *info = data->sequence->getFrameInfo(frameIndex);

			if (field30 + info->field_30 >= data->field_4A1) {
				found = true;
				break;
			}

			if (field30 > data->field_4A1 - 10) {
				if (info->soundAction)
					getSound()->playSoundEvent(entityIndex, info->soundAction, (field30 <= data->field_4A1 - info->field_31) ? 0 : (byte)(field30 + info->field_31 - data->field_4A1));
			}

			field30 += info->field_30;

			if (info->field_33 & 4)
				flag = true;

			if (info->field_33 & 2) {
				flag = false;

				getSavePoints()->push(kEntityPlayer, entityIndex, kAction10);
				getSavePoints()->process();

				if (getFlags()->flag_entities_0 || data->doProcessEntity)
					return;
			}

			if (info->field_33 & 16) {
				getSavePoints()->push(kEntityPlayer, entityIndex, kAction4);
				getSavePoints()->process();

				if (getFlags()->flag_entities_0 || data->doProcessEntity)
					return;
			}

			frameIndex++;

		} while (!found);

		if (found) {

			if (flag) {
				bool found2 = false;

				do {
					if (frameIndex >= data->sequence->count())
						break;

					FrameInfo *info = data->sequence->getFrameInfo(frameIndex);
					if (info->field_33 & 2) {
						found2 = true;

						getSavePoints()->push(kEntityPlayer, entityIndex, kAction10);
						getSavePoints()->process();

						if (getFlags()->flag_entities_0 || data->doProcessEntity)
							return;

					} else {
						data->field_4A1 += info->field_30;

						byte soundAction = data->sequence->getFrameInfo((uint16)data->currentFrame)->soundAction;
						if (soundAction)
							getSound()->playSoundEvent(entityIndex, soundAction);

						++frameIndex;
					}

				} while (!found2);

				if (found2) {
					data->currentFrame = frameIndex;
					data->field_49B = 0;

					byte soundAction = data->sequence->getFrameInfo((uint16)data->currentFrame)->soundAction;
					byte field31 = data->sequence->getFrameInfo((uint16)data->currentFrame)->field_31;
					if (soundAction && data->currentFrame != currentFrameCopy)
						getSound()->playSoundEvent(entityIndex, soundAction, field31);

				} else {
					data->currentFrame = (int16)(data->sequence->count() - 1);
					data->field_49B = data->sequence->getFrameInfo((uint16)data->currentFrame)->field_30;
				}

			} else {

				data->currentFrame = frameIndex;
				data->field_49B = data->field_4A1 - field30;

				byte soundAction = data->sequence->getFrameInfo((uint16)data->currentFrame)->soundAction;
				byte field31 = data->sequence->getFrameInfo((uint16)data->currentFrame)->field_31;
				if (soundAction && data->currentFrame != currentFrameCopy)
					getSound()->playSoundEvent(entityIndex, soundAction, field31 <= data->field_49B ? 0 : (byte)(field31 - data->field_49B));
			}
		} else {
			data->currentFrame = (int16)(data->sequence->count() - 1);
			data->field_49B = data->sequence->getFrameInfo((uint16)data->currentFrame)->field_30;

			getSavePoints()->push(kEntityPlayer, entityIndex, kActionExitCompartment);
			getSavePoints()->process();
		}
		break;
	}
}

int16 Entities::getCurrentFrame(EntityIndex entity, Sequence *sequence, EntityPosition position, bool doProcessing) const {
	EntityData::EntityCallData *data = getData(entity);

	EntityPosition firstFramePosition = sequence->getFrameInfo(0)->entityPosition;
	EntityPosition lastFramePosition = sequence->getFrameInfo(sequence->count() - 1)->entityPosition;

	bool isGoingForward = (firstFramePosition < lastFramePosition);

	if (!doProcessing) {
		if (!isGoingForward) {
			if (data->field_4A3 + firstFramePosition < data->entityPosition || lastFramePosition - data->field_4A3 > data->entityPosition)
				return -1;
		} else {
			if (firstFramePosition - data->field_4A3 > data->entityPosition || lastFramePosition + data->field_4A3 < data->entityPosition)
				return -1;
		}
	}

	if (sequence->count() == 0)
		return 0;

	// Search for the correct frame
	// TODO: looks slightly like some sort of binary search
	uint16 frame = 0;
	uint16 numFrames = sequence->count() - 1;

	for (;;) {
		uint16 currentFrame = (frame + numFrames) / 2;

		if (position + sequence->getFrameInfo(currentFrame)->entityPosition <= data->entityPosition) {
			if (!isGoingForward)
				numFrames = (frame + numFrames) / 2;
			else
				frame = (frame + numFrames) / 2;
		} else {
			if (isGoingForward)
				numFrames = (frame + numFrames) / 2;
			else
				frame = (frame + numFrames) / 2;
		}

		if (numFrames - frame == 1) {
			uint16 lastFramePos = ABS(position - (sequence->getFrameInfo(numFrames)->entityPosition + data->entityPosition));
			uint16 framePosition = ABS(position - (sequence->getFrameInfo(frame)->entityPosition + data->entityPosition));

			return (framePosition > lastFramePos) ? numFrames : frame;
		}

		if (numFrames <= frame)
			return currentFrame;
	}
}

void Entities::processFrame(EntityIndex entityIndex, bool keepPreviousFrame, bool dontPlaySound) {
	EntityData::EntityCallData *data = getData(entityIndex);

	// Set frame to be drawn again
	if (data->frame && keepPreviousFrame) {
		if (data->frame->getInfo()->subType != kFrameType3)
			data->frame->getInfo()->subType = kFrameType2;

		getScenes()->setFlagDrawSequences();
	}

	// Remove old frame from queue
	if (data->frame && !keepPreviousFrame)
		getScenes()->removeFromQueue(data->frame);

	// Stop if nothing else to draw
	if (data->currentFrame < 0)
		return;

	if (data->currentFrame > (int)data->sequence->count())
		return;

	// Get new frame info
	FrameInfo *info = data->sequence->getFrameInfo((uint16)data->currentFrame);

	if (data->frame && data->frame->getInfo()->subType != kFrameType3 && (!info->field_2E || keepPreviousFrame))
		getScenes()->setCoordinates(data->frame);

	// Update position
	if (info->entityPosition) {
		data->entityPosition = info->entityPosition;
		if (data->field_4A9)
			data->entityPosition = (EntityPosition)(data->entityPosition + getEntityPositionFromCurrentPosition());
	}

	info->location = entityIndex + ABS(getData(entityIndex)->entityPosition - getData(kEntityPlayer)->entityPosition);

	if (info->subType != kFrameType3) {
		info->subType = kFrameType1;

		if (!keepPreviousFrame)
			info->subType = kFrameTypeNone;
	}

	if (info->field_33 & 1)
		getSavePoints()->push(kEntityPlayer, entityIndex, kActionExcuseMeCath);

	if (info->field_33 & 2) {
		getSavePoints()->push(kEntityPlayer, entityIndex, kAction10);
		getSavePoints()->process();

		if (getFlags()->flag_entities_0 || data->doProcessEntity)
			return;
	}

	if (info->field_33 & 16) {
		getSavePoints()->push(kEntityPlayer, entityIndex, kAction4);
		getSavePoints()->process();

		if (getFlags()->flag_entities_0 || data->doProcessEntity)
			return;
	}

	if (data->position) {
		updatePositionExit(entityIndex, data->car2, data->position);
		data->car2 = kCarNone;
		data->position = 0;
	}

	if (info->position) {
		data->car2 = data->car;
		data->position = info->position;
		updatePositionEnter(entityIndex, data->car2, data->position);

		if (getFlags()->flag_entities_0 || data->doProcessEntity)
			return;
	}

	if (info->soundAction && !dontPlaySound)
		getSound()->playSoundEvent(entityIndex, info->soundAction, info->field_31);

	// Add the new frame to the queue
	SequenceFrame *frame = new SequenceFrame(data->sequence, (uint16)data->currentFrame);
	getScenes()->addToQueue(frame);

	// Keep previous frame if needed and store the new frame
	if (keepPreviousFrame) {
		SAFE_DELETE(data->frame1);
		data->frame1 = data->frame;
	} else {
		SAFE_DELETE(data->frame);
	}

	data->frame = frame;

	if (!dontPlaySound)
		data->field_49B = keepPreviousFrame ? 0 : 1;
}

void Entities::drawNextSequence(EntityIndex entityIndex) const {
	EntityData::EntityCallData *data = getData(entityIndex);

	if (data->direction == kDirectionRight) {
		getSavePoints()->push(kEntityPlayer, entityIndex, kActionExitCompartment);
		getSavePoints()->process();

		if (getFlags()->flag_entities_0 || data->doProcessEntity)
			return;
	}

	if (!isDirectionUpOrDown(entityIndex))
		return;

	if (data->sequence2)
		return;

	if (!getScenes()->checkPosition(kSceneNone, SceneManager::kCheckPositionLookingAtDoors))
		return;

	if (getData(kEntityPlayer)->car != data->car)
		return;

	if (!data->field_4A9 || isWalkingOppositeToPlayer(entityIndex)) {
		if (!data->field_4A9 && isWalkingOppositeToPlayer(entityIndex)) {
			data->entityPosition = kPosition_2088;

			if (data->direction != kDirectionUp)
				data->entityPosition = kPosition_8512;

			drawSequences(entityIndex, data->direction, true);
		}
	} else {
		data->entityPosition = kPosition_8514;

		if (data->direction != kDirectionUp)
			data->entityPosition = kPosition_2086;

		drawSequences(entityIndex, data->direction, true);
	}
}

void Entities::updateEntityPosition(EntityIndex entityIndex) const {
	EntityData::EntityCallData *data = getData(entityIndex);

	getScenes()->removeAndRedraw(&data->frame, false);

	SAFE_DELETE(data->frame1);
	data->field_49B = 0;

	if (isDirectionUpOrDown(entityIndex)
	 && (getScenes()->checkPosition(kSceneNone, SceneManager::kCheckPositionLookingUp) || getScenes()->checkPosition(kSceneNone, SceneManager::kCheckPositionLookingDown))
	 && data->car == getData(kEntityPlayer)->car) {

		if (isWalkingOppositeToPlayer(entityIndex)) {
			data->entityPosition = getData(kEntityPlayer)->entityPosition;
		} else if (data->field_4A9) {
			data->entityPosition = (data->direction == kDirectionUp) ? kPosition_8514 : kPosition_2086;
		} else {
			if (isPlayerPosition(kCarGreenSleeping, 1) || isPlayerPosition(kCarGreenSleeping, 40)
			 || isPlayerPosition(kCarRedSleeping, 1) || isPlayerPosition(kCarRedSleeping, 40)) {
				 data->entityPosition = (data->direction == kDirectionUp) ? kPosition_2588 : kPosition_8012;
			} else {
				data->entityPosition = (data->direction == kDirectionUp) ? kPosition_9271 : kPosition_849;
			}
		}
	}

	SAFE_DELETE(data->sequence);
	data->sequenceName = "";
	data->field_4A9 = false;

	if (data->directionSwitch)
		data->direction = data->directionSwitch;
}

void Entities::copySequenceData(EntityIndex entityIndex) const {
	EntityData::EntityCallData *data = getData(entityIndex);

	if (data->sequence)
		data->sequence3 = data->sequence;

	data->sequence = data->sequence2;
	data->sequenceName = data->sequenceName2;
	data->field_4A9 = data->field_4AA;

	if (data->directionSwitch)
		data->direction = data->directionSwitch;

	// Clear sequence 3
	data->sequence2 = NULL;
	data->sequenceName2 = "";
	data->field_4AA = false;
	data->directionSwitch = kDirectionNone;

	if (data->field_4A9) {
		computeCurrentFrame(entityIndex);

		if (data->currentFrame == -1)
			data->currentFrame = 0;
	} else {
		data->currentFrame = data->currentFrame2;
		data->currentFrame2 = 0;

		if (data->currentFrame == -1)
			data->currentFrame = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
// Drawing
//////////////////////////////////////////////////////////////////////////
void Entities::drawSequenceLeft(EntityIndex index, const char *sequence) const {
	drawSequence(index, sequence, kDirectionLeft);
}

void Entities::drawSequenceRight(EntityIndex index, const char *sequence) const {
	drawSequence(index, sequence, kDirectionRight);
}

void Entities::clearSequences(EntityIndex entityIndex) const {
	debugC(8, kLastExpressDebugLogic, "Clear sequences for entity %s", ENTITY_NAME(entityIndex));

	EntityData::EntityCallData *data = getData(entityIndex);

	getScenes()->removeAndRedraw(&data->frame, false);
	getScenes()->removeAndRedraw(&data->frame1, false);

	if (data->sequence2) {
		SAFE_DELETE(data->sequence2);
		data->sequenceName2 = "";
		data->field_4AA = false;
		data->directionSwitch = kDirectionNone;
	}

	if (data->sequence) {
		SAFE_DELETE(data->sequence);
		data->sequenceName = "";
		data->field_4A9 = false;
		data->currentFrame = -1;
	}

	data->sequenceNamePrefix = "";
	data->direction = kDirectionNone;
	data->doProcessEntity = true;
}

void Entities::drawSequence(EntityIndex index, const char *sequence, EntityDirection direction) const {
	debugC(8, kLastExpressDebugLogic, "Drawing sequence %s for entity %s with direction %s", sequence, ENTITY_NAME(index), DIRECTION_NAME(direction));

	// Copy sequence name
	getData(index)->sequenceNamePrefix = sequence;
	getData(index)->sequenceNamePrefix.toUppercase();
	getData(index)->sequenceNamePrefix += "-";

	// Reset fields
	getData(index)->field_49B = 0;
	getData(index)->currentFrame = 0;
	getData(index)->field_4A1 = 0;

	drawSequences(index, direction, true);
}

void Entities::drawSequences(EntityIndex entityIndex, EntityDirection direction, bool loadSequence) const {
	EntityData::EntityCallData *data = getData(entityIndex);

	// Compute value for loading sequence depending on direction
	byte field30 = (direction == kDirectionLeft ? entityIndex + 35 : 15);

	data->doProcessEntity = true;
	bool field4A9 = data->field_4A9;

	// First case: different car and not going right: cleanup and return
	if (data->car != getData(kEntityPlayer)->car && direction != kDirectionRight) {
		clearEntitySequenceData(data, direction);
		return;
	}

	data->directionSwitch = kDirectionNone;

	// Process sequence names
	Common::String sequenceName;
	Common::String sequenceName1;
	Common::String sequenceName2;
	Common::String sequenceName3;

	getSequenceName(entityIndex, direction, sequenceName1, sequenceName2);

	// No sequence 1: cleanup and return
	if (sequenceName1 == "") {
		clearEntitySequenceData(data, direction);
		return;
	}

	if (sequenceName1 == data->sequenceNameCopy) {
		data->direction = direction;
		return;
	}

	if (direction == kDirectionLeft || direction == kDirectionRight) {
		COMPUTE_SEQUENCE_NAME(sequenceName, sequenceName1);

		if (sequenceName3 != "")
			COMPUTE_SEQUENCE_NAME(sequenceName3, sequenceName2);
	}

	if (!data->frame) {
		data->direction = direction;

		if (sequenceName1 == data->sequenceName) {
			if (sequenceName2 == "")
				return;

			loadSequence2(entityIndex, sequenceName2, sequenceName3, field30, loadSequence);
			return;
		}

		SAFE_DELETE(data->sequence);

		if (sequenceName1 != data->sequenceName2) {

			if (loadSequence) {

				if (data->car == getData(kEntityPlayer)->car)
					data->sequence = loadSequence1(sequenceName1, field30);

				if (data->sequence) {
					data->sequenceName = sequenceName1;
					data->sequenceNameCopy = "";
				} else {
					if (sequenceName != "")
						data->sequence = loadSequence1(sequenceName, field30);

					data->sequenceName = (data->sequence ? sequenceName : "");
					data->sequenceNameCopy = (data->sequence ? "" : sequenceName1);
				}
			} else {
				data->sequenceName = sequenceName1;
			}

			if (sequenceName2 != "") {
				loadSequence2(entityIndex, sequenceName2, sequenceName3, field30, loadSequence);
				return;
			}

			if (!data->sequence2) {
				if (sequenceName2 == "")
					return;

				loadSequence2(entityIndex, sequenceName2, sequenceName3, field30, loadSequence);
				return;
			}

			SAFE_DELETE(data->sequence2);
		} else {
			data->sequence = data->sequence2;
			data->sequenceName = data->sequenceName2;
			data->sequence2 = NULL;
		}

		data->sequenceName2 = "";

		if (sequenceName2 == "")
			return;

		loadSequence2(entityIndex, sequenceName2, sequenceName3, field30, loadSequence);
		return;
	}

	if (data->sequenceName != sequenceName1) {

		if (data->sequenceName2 != sequenceName1) {
			SAFE_DELETE(data->sequence2);
			TRY_LOAD_SEQUENCE(data->sequence2, data->sequenceName2, sequenceName1, sequenceName);
		}

		data->field_4AA = data->field_4A9;
		if ((direction != kDirectionUp && direction != kDirectionDown) || data->field_4AA || !data->sequence2) {
			data->currentFrame2 = 0;
		} else {
			data->currentFrame2 = getCurrentFrame(entityIndex, data->sequence2, kPositionNone, false);

			if (data->currentFrame2 == -1) {
				clearSequences(entityIndex);
				return;
			}
		}

		data->field_4A9 = field4A9;
		data->field_49B = data->frame->getInfo()->field_30;
		data->currentFrame = (int16)(data->sequence->count() - 1);
		data->direction = kDirectionSwitch;
		data->directionSwitch = direction;
	} else {
		SAFE_DELETE(data->sequence2);

		data->sequence2 = loadSequence1(data->sequence->getName(), data->sequence->getField30());

		data->sequenceName2 = data->sequenceName;
		data->field_4AA = data->field_4A9;
		data->field_49B = data->frame->getInfo()->field_30;
		data->currentFrame = (int16)(data->sequence->count() - 1);
		data->direction = kDirectionSwitch;
		data->directionSwitch = direction;

		if ((direction != kDirectionUp && direction != kDirectionDown) || data->field_4AA || !data->sequence2) {
			data->currentFrame2 = 0;
		} else {
			data->currentFrame2 = getCurrentFrame(entityIndex, data->sequence2, kPositionNone, false);

			if (data->currentFrame2 == -1)
				clearSequences(entityIndex);
		}
	}
}

void Entities::loadSequence2(EntityIndex entityIndex, Common::String sequenceName, Common::String sequenceName2, byte field30, bool reloadSequence) const {
	EntityData::EntityCallData *data = getData(entityIndex);

	if (data->sequenceName2 == sequenceName)
		return;

	if (data->sequence2)
		SAFE_DELETE(data->sequence2);

	if (reloadSequence) {
		TRY_LOAD_SEQUENCE(data->sequence2, data->sequenceName2, sequenceName, sequenceName2);
	} else {
		data->sequenceName2 = sequenceName;
	}
}

void Entities::getSequenceName(EntityIndex index, EntityDirection direction, Common::String &sequence1, Common::String &sequence2) const {
	EntityData::EntityCallData *data = getData(index);
	Position position = getScenes()->get(getState()->scene)->position;

	// reset fields
	data->field_4A9 = false;
	data->field_4AA = false;

	switch (direction) {
	default:
		break;

	case kDirectionUp:
		switch (position) {
		default:
			break;

		case 1:
			if (data->entityPosition < kPosition_2587)
				sequence1 = Common::String::format("%02d%01d-01u.seq", index, data->clothes);
			break;

		case 2:
		case 3:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
			if (data->entityPosition >= kPosition_9270)
				break;

			if (data->entityPosition >= kPosition_8513) {
				sequence1 = Common::String::format("%02d%01d-%02deu.seq", index, data->clothes, position);
			} else {
				sequence1 = Common::String::format("%02d%01d-03u.seq", index, data->clothes);
				sequence2 = Common::String::format("%02d%01d-%02deu.seq", index, data->clothes, position);
				data->field_4A9 = true;
			}
			break;

		case 18:
			if (data->entityPosition < kPosition_9270)
				sequence1 = Common::String::format("%02d%01d-18u.seq", index, data->clothes);
			break;

		case 22:
			if (getData(kEntityPlayer)->entityPosition > data->entityPosition)
				sequence1 = Common::String::format("%02d%01d-22u.seq", index, data->clothes);
			break;

		case 23:
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		case 30:
		case 31:
		case 32:
		case 33:
		case 34:
		case 35:
		case 36:
		case 37:
		case 38:
		case 39:
			if (getData(kEntityPlayer)->entityPosition <= data->entityPosition)
				break;

			if (data->entityPosition >= kPosition_2087) {
				sequence1 = Common::String::format("%02d%01d-38u.seq", index, data->clothes);
				data->field_4A9 = true;
			} else {
				sequence1 = Common::String::format("%02d%01d-%02deu.seq", index, data->clothes, position);
				sequence2 = Common::String::format("%02d%01d-38u.seq", index, data->clothes);
				data->field_4AA = true;
			}
			break;

		case 40:
			if (getData(kEntityPlayer)->entityPosition > data->entityPosition)
				sequence1 = Common::String::format("%02d%01d-40u.seq", index, data->clothes);
			break;
		}
		break;

	case kDirectionDown:
		switch (position) {
		default:
			break;

		case 1:
			if (getData(kEntityPlayer)->entityPosition < data->entityPosition)
				sequence1 = Common::String::format("%02d%01d-01d.seq", index, data->clothes);
			break;

		case 2:
		case 3:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
			if (getData(kEntityPlayer)->entityPosition >= data->entityPosition)
				break;

			if (data->entityPosition <= kPosition_8513) {
				sequence1 = Common::String::format("%02d%01d-03d.seq", index, data->clothes);
				data->field_4A9 = true;
			} else {
				sequence1 = Common::String::format("%02d%01d-%02ded.seq", index, data->clothes, position);
				sequence2 = Common::String::format("%02d%01d-03d.seq", index, data->clothes);
				data->field_4AA = true;
			}
			break;

		case 18:
			if (getData(kEntityPlayer)->entityPosition < data->entityPosition)
				sequence1 = Common::String::format("%02d%01d-18d.seq", index, data->clothes);
			break;

		case 22:
			if (data->entityPosition > kPosition_850)
				sequence1 = Common::String::format("%02d%01d-22d.seq", index, data->clothes);
			break;

		case 23:
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		case 30:
		case 31:
		case 32:
		case 33:
		case 34:
		case 35:
		case 36:
		case 37:
		case 38:
		case 39:
			if (data->entityPosition <= kPosition_850)
				break;

			if (data->entityPosition <= kPosition_2087) {
				sequence1 = Common::String::format("%02d%01d-%02ded.seq", index, data->clothes, position);
			} else {
				sequence1 = Common::String::format("%02d%01d-38d.seq", index, data->clothes);
				sequence2 = Common::String::format("%02d%01d-%02ded.seq", index, data->clothes, position);
				data->field_4A9 = true;
			}
			break;

		case 40:
			if (getData(kEntityPlayer)->entityPosition > kPosition_8013)
				sequence1 = Common::String::format("%02d%01d-40d.seq", index, data->clothes);
			break;
		}
		break;

	// First part of sequence is already set
	case kDirectionLeft:
	case kDirectionRight:
		sequence1 = Common::String::format("%s%02d.seq", data->sequenceNamePrefix.c_str(), position);
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
/// Compartments
//////////////////////////////////////////////////////////////////////////
void Entities::enterCompartment(EntityIndex entity, ObjectIndex compartment, bool useCompartment1) {
	if (entity > kEntityChapters)
		return;

	switch (compartment) {
	default:
		// Return here so we do not update the compartments
		return;

	case kObjectCompartment1:
		updatePositionsEnter(entity, kCarGreenSleeping, 41, 51, 17, 38);
		break;

	case kObjectCompartment2:
		updatePositionsEnter(entity, kCarGreenSleeping, 42, 52, 15, 36);
		break;

	case kObjectCompartment3:
		updatePositionsEnter(entity, kCarGreenSleeping, 43, 53, 13, 34);
		break;

	case kObjectCompartment4:
		updatePositionsEnter(entity, kCarGreenSleeping, 44, 54, 11, 32);
		break;

	case kObjectCompartment5:
		updatePositionsEnter(entity, kCarGreenSleeping, 45, 55, 9, 30);
		break;

	case kObjectCompartment6:
		updatePositionsEnter(entity, kCarGreenSleeping, 46, 56, 7, 28);
		break;

	case kObjectCompartment7:
		updatePositionsEnter(entity, kCarGreenSleeping, 47, 57, 5, 26);
		break;

	case kObjectCompartment8:
		updatePositionsEnter(entity, kCarGreenSleeping, 48, 58, 3, 25);
		break;

	case kObjectCompartmentA:
		updatePositionsEnter(entity, kCarRedSleeping, 41, 51, 17, 38);
		break;

	case kObjectCompartmentB:
		updatePositionsEnter(entity, kCarRedSleeping, 42, 52, 15, 36);
		break;

	case kObjectCompartmentC:
		updatePositionsEnter(entity, kCarRedSleeping, 43, 53, 13, 34);
		break;

	case kObjectCompartmentD:
		updatePositionsEnter(entity, kCarRedSleeping, 44, 54, 11, 32);
		break;

	case kObjectCompartmentE:
		updatePositionsEnter(entity, kCarRedSleeping, 45, 55, 9, 30);
		break;

	case kObjectCompartmentF:
		updatePositionsEnter(entity, kCarRedSleeping, 46, 56, 7, 28);
		break;

	case kObjectCompartmentG:
		updatePositionsEnter(entity, kCarRedSleeping, 47, 57, 5, 26);
		break;

	case kObjectCompartmentH:
		updatePositionsEnter(entity, kCarRedSleeping, 48, 58, 3, 25);
		break;
	}

	// Update compartments
	int index = (compartment < 32 ? compartment - 1 : compartment - 24);
	if (index >= 16)
		error("[Entities::enterCompartment] Invalid compartment index");

	if (useCompartment1)
		_compartments1[index] |= STORE_VALUE(entity);
	else
		_compartments[index] |= STORE_VALUE(entity);
}

void Entities::exitCompartment(EntityIndex entity, ObjectIndex compartment, bool useCompartment1) {
	if (entity > kEntityChapters)
		return;

	// TODO factorize in one line
	switch (compartment) {
	default:
		// Return here so we do not update the compartments
		return;

	case kObjectCompartment1:
		updatePositionsExit(entity, kCarGreenSleeping, 41, 51);
		break;

	case kObjectCompartment2:
		updatePositionsExit(entity, kCarGreenSleeping, 42, 52);
		break;

	case kObjectCompartment3:
		updatePositionsExit(entity, kCarGreenSleeping, 43, 53);
		break;

	case kObjectCompartment4:
		updatePositionsExit(entity, kCarGreenSleeping, 44, 54);
		break;

	case kObjectCompartment5:
		updatePositionsExit(entity, kCarGreenSleeping, 45, 55);
		break;

	case kObjectCompartment6:
		updatePositionsExit(entity, kCarGreenSleeping, 46, 56);
		break;

	case kObjectCompartment7:
		updatePositionsExit(entity, kCarGreenSleeping, 47, 57);
		break;

	case kObjectCompartment8:
		updatePositionsExit(entity, kCarGreenSleeping, 48, 58);
		break;

	case kObjectCompartmentA:
		updatePositionsExit(entity, kCarRedSleeping, 41, 51);
		break;

	case kObjectCompartmentB:
		updatePositionsExit(entity, kCarRedSleeping, 42, 52);
		break;

	case kObjectCompartmentC:
		updatePositionsExit(entity, kCarRedSleeping, 43, 53);
		break;

	case kObjectCompartmentD:
		updatePositionsExit(entity, kCarRedSleeping, 44, 54);
		break;

	case kObjectCompartmentE:
		updatePositionsExit(entity, kCarRedSleeping, 45, 55);
		break;

	case kObjectCompartmentF:
		updatePositionsExit(entity, kCarRedSleeping, 46, 56);
		break;

	case kObjectCompartmentG:
		updatePositionsExit(entity, kCarRedSleeping, 47, 57);
		break;

	case kObjectCompartmentH:
		updatePositionsExit(entity, kCarRedSleeping, 48, 58);
		break;
	}

	// Update compartments
	int index = (compartment < 32 ? compartment - 1 : compartment - 24);
	if (index >= 16)
		error("[Entities::exitCompartment] Invalid compartment index");

	if (useCompartment1)
		_compartments1[index] &= ~STORE_VALUE(entity);
	else
		_compartments[index] &= ~STORE_VALUE(entity);
}

void Entities::updatePositionEnter(EntityIndex entity, CarIndex car, Position position) {
	if (entity == kEntity39)
		entity = kEntityPlayer;

	if (entity > kEntityChapters)
		return;

	_positions[100 * car + position] |= STORE_VALUE(entity);

	if (isPlayerPosition(car, position) || (car == kCarRestaurant && position == 57 && isPlayerPosition(kCarRestaurant, 50))) {
		getSound()->excuseMe(entity);
		getScenes()->loadScene(getScenes()->processIndex(getState()->scene));
		getSound()->playSound(kEntityPlayer, "CAT1127A");
	} else {
		getLogic()->updateCursor();
	}
}

void Entities::updatePositionExit(EntityIndex entity, CarIndex car, Position position) {
	if (entity == kEntity39)
		entity = kEntityPlayer;

	if (entity > kEntityChapters)
		return;

	_positions[100 * car + position] &= ~STORE_VALUE(entity);

	getLogic()->updateCursor();
}

void Entities::updatePositionsEnter(EntityIndex entity, CarIndex car, Position position1, Position position2, Position position3, Position position4) {
	if (entity == kEntity39)
		entity = kEntityPlayer;

	if (entity > kEntityChapters)
		return;

	_positions[100 * car + position1] |= STORE_VALUE(entity);
	_positions[100 * car + position2] |= STORE_VALUE(entity);

	// FIXME: also checking two DWORD values that do not seem to updated anywhere...
	if (isPlayerPosition(car, position1) || isPlayerPosition(car, position2) || isPlayerPosition(car, position3) || isPlayerPosition(car, position4)) {
		getSound()->excuseMe(entity);
		getScenes()->loadScene(getScenes()->processIndex(getState()->scene));
		getSound()->playSound(kEntityPlayer, "CAT1127A");
	} else {
		getLogic()->updateCursor();
	}
}

void Entities::updatePositionsExit(EntityIndex entity, CarIndex car, Position position1, Position position2) {
	if (entity == kEntity39)
		entity = kEntityPlayer;

	if (entity > kEntityChapters)
		return;

	_positions[100 * car + position1] &= ~STORE_VALUE(entity);
	_positions[100 * car + position2] &= ~STORE_VALUE(entity);

	getLogic()->updateCursor();
}

void Entities::loadSceneFromEntityPosition(CarIndex car, EntityPosition entityPosition, bool alternate) const {

	// Determine position
	Position position = (alternate ? 1 : 40);
	do {
		if (entityPosition > entityPositions[position]) {
			if (alternate)
				break;

			// For default value, we ignore position 24
			if (position != 24)
				break;
		}

		alternate ? ++position : --position;

	} while (alternate ? position <= 18 : position >= 22);

	// For position outside bounds, use minimal value
	if ((alternate && position > 18) || (alternate && position < 22)) {
		getScenes()->loadSceneFromPosition(car, alternate ? 18 : 22);
		return;
	}

	// Load scene from position
	switch (position) {
	default:
		getScenes()->loadSceneFromPosition(car, (Position)(position + (alternate ? - 1 : 1)));
		break;

	// Alternate
	case 1:
		if (alternate) getScenes()->loadSceneFromPosition(car, 1);
		break;

	case 5:
		if (alternate) getScenes()->loadSceneFromPosition(car, 3);
		break;

	// Default
	case 23:
		if (!alternate) getScenes()->loadSceneFromPosition(car, 25);
		break;

	case 40:
		if (!alternate) getScenes()->loadSceneFromPosition(car, 40);
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
//	Checks
//////////////////////////////////////////////////////////////////////////
bool Entities::hasValidFrame(EntityIndex entity) const {
	return (getData(entity)->frame && (getData(entity)->frame->getInfo()->subType != kFrameType3));
}

bool Entities::compare(EntityIndex entity1, EntityIndex entity2) const {
	EntityData::EntityCallData *data1 = getData(entity1);
	EntityData::EntityCallData *data2 = getData(entity2);

	if (data2->car != data1->car
	 || data1->car < kCarGreenSleeping
	 || data1->car > kCarRedSleeping)
		return false;

	EntityPosition position1 = (data1->entityPosition >= data2->entityPosition) ? data1->entityPosition : data2->entityPosition;
	EntityPosition position2 = (data1->entityPosition >= data2->entityPosition) ? data2->entityPosition : data1->entityPosition;

	// Compute position
	int index1 = 7;
	do {
		if (objectsPosition[index1] >= position2)
			break;

		--index1;
	} while (index1 > -1);

	int index2 = 0;
	do {
		if (objectsPosition[index2] <= position2)
			break;

		++index2;
	} while (index2 < 8);

	if (index1 > -1 && index2 < 8 && index2 <= index1) {
		while (index2 <= index1) {
			if (getCompartments(index2 + (data1->car == kCarGreenSleeping ? 0 : 8)))
				return true;

			if (getCompartments1(index2 + (data1->car == kCarGreenSleeping ? 0 : 8)))
				return true;

			++index2;
		}
	}

	for (EntityIndex entity = kEntityAnna; entity <= kEntity39; entity = (EntityIndex)(entity + 1)) {

		if (entity1 == entity || entity2 == entity)
			continue;

		if (!isDirectionUpOrDown(entity))
			continue;

		if (data1->car == getEntityData(entity)->car
		 && getEntityData(entity)->entityPosition > position2
		 && getEntityData(entity)->entityPosition < position1)
			return true;
	}

	return false;
}

bool Entities::updateEntity(EntityIndex entity, CarIndex car, EntityPosition position) const {
	EntityData::EntityCallData *data = getData(entity);
	EntityDirection direction = kDirectionNone;
	int delta = 0;
	bool flag1 = false;
	bool flag2 = false;
	bool flag3 = false;

	if (position == kPosition_2000
	 && getScenes()->checkPosition(kSceneNone, SceneManager::kCheckPositionLookingUp)
	 && !isPlayerPosition(kCarGreenSleeping, 1)
	 && !isPlayerPosition(kCarRedSleeping, 2))
		 position = kPosition_1500;

	if (data->direction != kDirectionUp && data->direction != kDirectionDown)
		data->field_497 = 0;

	if (data->field_497) {
		data->field_497--;

		if (data->field_497 == 128)
			data->field_497 = 0;

		if ((data->field_497 & 127) != 8) {
			data->field_49B = 0;
			return false;
		}

		flag1 = true;

		if (data->field_497 & 128)
			flag2 = true;
	}

	if (data->car != car)
		goto label_process_entity;

	// Calculate delta
	delta = ABS(data->entityPosition - position);
	if (delta < 100 || (position > kPosition_850 && position < kPosition_9270 && delta < 300))
		flag3 = true;

	if (!flag3) {
		if ((getScenes()->checkPosition(kSceneNone, SceneManager::kCheckPositionLookingUp) && data->direction == kDirectionUp)
		 || (getScenes()->checkPosition(kSceneNone, SceneManager::kCheckPositionLookingDown) && data->direction == kDirectionDown)) {
			 if (!checkPosition(position) && isDistanceBetweenEntities(entity, kEntityPlayer, 250))
				 flag3 = true;
		}

		if (!flag3)
			goto label_process_entity;
	}

	if (getEntities()->hasValidFrame(entity)
	 && getEntities()->isWalkingOppositeToPlayer(entity)
	 && !getEntities()->checkPosition(position)) {
		flag3 = false;
		position = (EntityPosition)(getData(kEntityPlayer)->entityPosition + 250 * (data->direction == kDirectionUp ? 1 : -1));
	}

	if (!flag3) {
label_process_entity:

		// Calculate direction
		if (data->car < car)
			direction = kDirectionUp;
		else if (data->car > car)
			direction = kDirectionDown;
		else // same car
			direction = (data->entityPosition < position) ? kDirectionUp : kDirectionDown;

		if (data->direction == direction) {
			if (!flag1) {

				if (checkDistanceFromPosition(entity, kPosition_1500, 750) && entity != kEntityFrancois) {

					if (data->entity != kEntityPlayer) {
						if (data->direction != kDirectionUp || (position <= kPosition_2000 && data->car == car)) {
							if (data->direction == kDirectionDown && (position < kPosition_1500 || data->car != car)) {
								if (data->entityPosition > kPosition_1500 && (data->car == kCarGreenSleeping || data->car == kCarRedSleeping)) {
									data->entity = (data->car == kCarGreenSleeping) ? kEntityMertens : kEntityCoudert;
									getSavePoints()->push(entity, data->entity, kAction11);
								}
							}
						} else {
							if (data->entityPosition < kPosition_1500 && (data->car == kCarGreenSleeping || data->car == kCarRedSleeping)) {
								data->entity = (data->car == kCarGreenSleeping) ? kEntityMertens : kEntityCoudert;
								getSavePoints()->push(entity, data->entity, kAction11, 1);
							}
						}
					}

				} else if (data->entity) {
					getSavePoints()->push(entity, data->entity, kAction16);
					data->entity = kEntityPlayer;
				}

				if (hasValidFrame(entity)) {

					if (!data->field_4A9)
						return false;

					int compartmentIndex = 0;
					if (data->car == kCarGreenSleeping)
						compartmentIndex = 0;
					else if (data->car == kCarRedSleeping)
						compartmentIndex = 8;

					for (int i = 0; i < 8; i++) {
						if (getCompartments(compartmentIndex) || getCompartments1(compartmentIndex)) {
							if (checkDistanceFromPosition(entity, objectsPosition[i], 750)) {
								if (checkPosition(objectsPosition[i])) {

									if ((data->direction == kDirectionUp   && data->entityPosition < objectsPosition[i] && (data->car != car || position > objectsPosition[i]))
									 || (data->direction == kDirectionDown && data->entityPosition > objectsPosition[i] && (data->car != car || position < objectsPosition[i]))) {

										 getSound()->excuseMe(entity, (EntityIndex)(State::getPowerOfTwo((uint32)(getCompartments(compartmentIndex) ? getCompartments(compartmentIndex) : getCompartments1(compartmentIndex)))));

										 data->field_497 = 144;

										 break;
									}
								}
							}
						}

						compartmentIndex++;
					}

					for (EntityIndex entityIndex = kEntityAnna; entityIndex <= kEntity39; entityIndex = (EntityIndex)(entityIndex + 1)) {
						if (getSavePoints()->getCallback(entityIndex)
						 && hasValidFrame(entityIndex)
						 && entityIndex != entity
						 && isDistanceBetweenEntities(entity, entityIndex, 750)
						 && isDirectionUpOrDown(entityIndex)
						 && (entity != kEntityRebecca || entityIndex != kEntitySophie)
						 && (entity != kEntitySophie || entityIndex != kEntityRebecca)
						 && (entity != kEntityIvo || entityIndex != kEntitySalko)
						 && (entity != kEntitySalko || entityIndex != kEntityIvo)
						 && (entity != kEntityMilos || entityIndex != kEntityVesna)
						 && (entity != kEntityVesna || entityIndex != kEntityMilos)) {

							 EntityData::EntityCallData *data2 = getData(entityIndex);

							 if (data->direction != data2->direction) {

								 if ((data->direction != kDirectionUp || data2->entityPosition <= data->entityPosition)
								  && (data->direction != kDirectionDown || data2->entityPosition >= data->entityPosition))
									continue;

								 data->field_49B = 0;
								 data2->field_49B = 0;

								 data->field_497 = 16;
								 data2->field_497 = 16;

								 getSound()->excuseMe(entity, entityIndex);
								 getSound()->excuseMe(entityIndex, entity);

								 if (entityIndex > entity)
									 ++data2->field_497;

								 break;
							 }

							 if (ABS(data2->entityPosition - getData(kEntityPlayer)->entityPosition) < ABS(data->entityPosition - getData(kEntityPlayer)->entityPosition)) {

								 if (!isWalkingOppositeToPlayer(entity)) {

									 if (direction == kDirectionUp) {
										 if (data->entityPosition < kPosition_9500)
											 data->entityPosition = (EntityPosition)(data->entityPosition + 500);
									 } else {
										 if (data->entityPosition > kPosition_500)
											 data->entityPosition = (EntityPosition)(data->entityPosition - 500);
									 }

									 drawSequences(entity, direction, true);

									 return false;
								 }
								 data->field_49B = 0;

								 break;
							 }
						}
					}

					return false;
				}

				if (data->direction == kDirectionUp) {
					if (data->entityPosition + data->field_4A3 < 10000)
						data->entityPosition = (EntityPosition)(data->entityPosition + data->field_4A3);
				} else {
					if (data->entityPosition > data->field_4A3)
						data->entityPosition = (EntityPosition)(data->entityPosition - data->field_4A3);
				}

				if (data->entityPosition <= kPosition_9270 || data->direction != kDirectionUp) {
					if (data->entityPosition < kPosition_850 && data->direction == kDirectionDown) {
						if (changeCar(data, entity, car, position, false, kPosition_9269, kCarKronos))
							return true;
					}
				} else {
					if (changeCar(data, entity, car, position, true, kPosition_851, kCarGreenSleeping))
						return true;
				}

				if (getData(kEntityPlayer)->car == data->car && data->location == kLocationOutsideCompartment) {
					if (data->direction == kDirectionUp) {

						if (getData(kEntityPlayer)->entityPosition > data->entityPosition
						 && getData(kEntityPlayer)->entityPosition - data->entityPosition >= 500
						 && data->field_4A3 + 500 > getData(kEntityPlayer)->entityPosition - data->entityPosition) {

							 if (getScenes()->checkPosition(kSceneNone, SceneManager::kCheckPositionLookingUp) || getScenes()->checkCurrentPosition(false)) {
								 getSavePoints()->push(kEntityPlayer, entity, kActionExcuseMe);

								 if (getScenes()->checkCurrentPosition(false))
									 getScenes()->loadSceneFromObject((ObjectIndex)getScenes()->get(getState()->scene)->param1, true);

							 } else if (getScenes()->checkPosition(kSceneNone, SceneManager::kCheckPositionLookingDown)) {
								 getSavePoints()->push(kEntityPlayer, entity, kActionExcuseMeCath);
							 }
						}
					} else {
						if (getData(kEntityPlayer)->entityPosition < data->entityPosition
						 && data->entityPosition - getData(kEntityPlayer)->entityPosition >= 500
						 && data->field_4A3 + 500 > data->entityPosition - getData(kEntityPlayer)->entityPosition) {

							if (getScenes()->checkPosition(kSceneNone, SceneManager::kCheckPositionLookingUp)) {
								 getSavePoints()->push(kEntityPlayer, entity, kActionExcuseMeCath);
							} else if (getScenes()->checkPosition(kSceneNone, SceneManager::kCheckPositionLookingDown) || getScenes()->checkCurrentPosition(false)) {
								 getSavePoints()->push(kEntityPlayer, entity, kActionExcuseMe);

								 if (getScenes()->checkCurrentPosition(false))
									 getScenes()->loadSceneFromObject((ObjectIndex)getScenes()->get(getState()->scene)->param1);
							}
						}
					}
					return false;
				}
			}
		} else if (!flag1) {
			drawSequences(entity, direction, true);
			return false;
		}

		//////////////////////////////////////////////////////////////////////////
		// Adjust positions

		// Direction Up
		if (direction == kDirectionUp) {
			if (data->entityPosition < (flag2 ? kPosition_8800 : kPosition_9250))
				data->entityPosition = (EntityPosition)(data->entityPosition + (flag2 ? kPosition_1200 : kPosition_750));

			if (data->car == car && data->entityPosition >= position) {
				data->entityPosition = position;
				data->direction = kDirectionNone;
				data->entity = kEntityPlayer;
				return true;
			}

			drawSequences(entity, direction, true);
			return false;
		}

		// Direction Down
		if (data->entityPosition > (flag2 ? kPosition_1200 : kPosition_750))
			data->entityPosition = (EntityPosition)(data->entityPosition - (flag2 ? kPosition_1200 : kPosition_750));

		if (data->car == car && data->entityPosition <= position) {
			data->entityPosition = position;
			data->direction = kDirectionNone;
			data->entity = kEntityPlayer;
			return true;
		}

		drawSequences(entity, direction, true);
		return false;
	}

	data->entityPosition = position;
	if (data->direction == kDirectionUp || data->direction == kDirectionDown)
		data->direction = kDirectionNone;
	data->entity = kEntityPlayer;

	return true;
}

bool Entities::changeCar(EntityData::EntityCallData *data, EntityIndex entity, CarIndex car, EntityPosition position, bool increment, EntityPosition newPosition, CarIndex newCar) const {
	if (getData(kEntityPlayer)->car == data->car) {
		getSound()->playSoundEvent(entity, 36);
		getSound()->playSoundEvent(entity, 37, 30);
	}

	data->car = (CarIndex)(increment ? data->car + 1 : data->car - 1);
	data->entityPosition = newPosition;

	if (data->car == newCar) {
		if (isInGreenCarEntrance(kEntityPlayer)) {
			getSound()->playSoundEvent(kEntityPlayer, 14);
			getSound()->excuseMe(entity, kEntityPlayer, kFlagDefault);
			getScenes()->loadSceneFromPosition(kCarGreenSleeping, 1);
			getSound()->playSound(kEntityPlayer, "CAT1127A");
			getSound()->playSoundEvent(kEntityPlayer, 15);
		}
	}

	if ((increment ? data->car > car : data->car < car) || (data->car == car && (increment ? data->entityPosition >= position : data->entityPosition <= position))) {
		data->car = car;
		data->entityPosition = position;
		data->direction = kDirectionNone;
		data->entity = kEntityPlayer;

		return true;
	}

	if (data->car == newCar) {
		if (isInKronosCarEntrance(kEntityPlayer)) {
			getSound()->playSoundEvent(kEntityPlayer, 14);
			getSound()->excuseMe(entity, kEntityPlayer, kFlagDefault);
			getScenes()->loadSceneFromPosition(kCarGreenSleeping, 62);
			getSound()->playSound(kEntityPlayer, "CAT1127A");
			getSound()->playSoundEvent(kEntityPlayer, 15);
		}
	}

	if (data->car == getData(kEntityPlayer)->car) {
		getSound()->playSoundEvent(entity, 36);
		getSound()->playSoundEvent(entity, 37, 30);
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
// CHECKS
//////////////////////////////////////////////////////////////////////////
bool Entities::isInsideCompartment(EntityIndex entity, CarIndex car, EntityPosition position) const {
	return (getData(entity)->entityPosition == position
		 && getData(entity)->location == kLocationInsideCompartment
		 && getData(entity)->car == car);
}

bool Entities::checkFields2(ObjectIndex object) const {

	EntityPosition position = kPositionNone;
	CarIndex car = kCarNone;

	switch (object) {
	default:
		return false;

	case kObjectCompartment1:
	case kObjectCompartment2:
	case kObjectCompartment3:
	case kObjectCompartment4:
	case kObjectCompartment5:
	case kObjectCompartment6:
	case kObjectCompartment7:
	case kObjectCompartment8:
		position = objectsPosition[object - 1];
		car = kCarGreenSleeping;
		if (isInsideCompartment(kEntityPlayer, car, position))
			return false;
		break;

	case kObjectHandleBathroom:
	case kObjectHandleInsideBathroom:
	case kObjectKitchen:
	case kObject20:
	case kObject21:
	case kObject22:
		position = objectsPosition[object-17];
		car = kCarGreenSleeping;
		break;

	case kObjectCompartmentA:
	case kObjectCompartmentB:
	case kObjectCompartmentC:
	case kObjectCompartmentD:
	case kObjectCompartmentE:
	case kObjectCompartmentF:
	case kObjectCompartmentG:
	case kObjectCompartmentH:
		position = objectsPosition[object-32];
		car = kCarRedSleeping;
		if (isInsideCompartment(kEntityPlayer, car, position))
			return false;
		break;

	case kObject48:
	case kObject49:
	case kObject50:
	case kObject51:
	case kObject52:
	case kObject53:
		position = objectsPosition[object-48];
		car = kCarRedSleeping;
		break;

	}

	uint index = 1;
	while (!isInsideCompartment((EntityIndex)index, car, position) || index == kEntityVassili) {
		index++;
		if (index >= 40)
			return false;
	}

	return true;
}

bool Entities::isInsideCompartments(EntityIndex entity) const {
	return (getData(entity)->car == kCarGreenSleeping
		 || getData(entity)->car == kCarRedSleeping)
		 && getData(entity)->location == kLocationInsideCompartment;
}

bool Entities::isPlayerPosition(CarIndex car, Position position) const {
	return getData(kEntityPlayer)->car == car && getScenes()->get(getState()->scene)->position == position;
}

bool Entities::isInsideTrainCar(EntityIndex entity, CarIndex car) const {
	return getData(entity)->car == car && getData(entity)->location <= kLocationInsideCompartment;
}

bool Entities::isInGreenCarEntrance(EntityIndex entity) const {
	return isInsideTrainCar(entity, kCarGreenSleeping) && getData(entity)->entityPosition < kPosition_850;
}

bool Entities::isPlayerInCar(CarIndex car) const {
	return isInsideTrainCar(kEntityPlayer, car) && getData(kEntityPlayer)->location && !isInGreenCarEntrance(kEntityPlayer);
}

bool Entities::isDirectionUpOrDown(EntityIndex entity) const {
	return getData(entity)->direction == kDirectionUp || getData(entity)->direction == kDirectionDown;
}

bool Entities::isDistanceBetweenEntities(EntityIndex entity1, EntityIndex entity2, uint distance) const {
	return getData(entity1)->car == getData(entity2)->car
	    && (uint)ABS(getData(entity1)->entityPosition - getData(entity2)->entityPosition) <= distance
		&& (getData(entity1)->location != kLocationOutsideTrain || getData(entity2)->location != kLocationOutsideTrain);
}

bool Entities::checkFields10(EntityIndex entity) const {
	return getData(entity)->location <= kLocationOutsideTrain;
}

bool Entities::isSomebodyInsideRestaurantOrSalon() const {
	for (uint i = 1; i < _entities.size(); i++) {
		EntityIndex index = (EntityIndex)i;

		if (getData(index)->location == kLocationOutsideCompartment && (isInSalon(index) || isInRestaurant(index)))
			return false;
	}

	return true;
}

bool Entities::isInSalon(EntityIndex entity) const {
	return isInsideTrainCar(entity, kCarRestaurant)
		&& getData(entity)->entityPosition >= kPosition_1540
		&& getData(entity)->entityPosition <= kPosition_3650;
}

bool Entities::isInRestaurant(EntityIndex entity) const {
	return isInsideTrainCar(entity, kCarRestaurant)
		&& getData(entity)->entityPosition >= kPosition_3650
		&& getData(entity)->entityPosition <= kPosition_5800;
}

bool Entities::isInKronosSalon(EntityIndex entity) const {
	return isInsideTrainCar(entity, kCarKronos)
		&& getData(entity)->entityPosition >= kPosition_5500
		&& getData(entity)->entityPosition <= kPosition_7500;
}

bool Entities::isOutsideAlexeiWindow() const {
	return (getData(kEntityPlayer)->entityPosition == kPosition_7500 || getData(kEntityPlayer)->entityPosition == kPosition_8200)
		 && getData(kEntityPlayer)->location == kLocationOutsideTrain
		 && getData(kEntityPlayer)->car == kCarGreenSleeping;
}

bool Entities::isOutsideAnnaWindow() const {
	return (getData(kEntityPlayer)->entityPosition == kPosition_4070 || getData(kEntityPlayer)->entityPosition == kPosition_4840)
		 && getData(kEntityPlayer)->location == kLocationOutsideTrain
		 && getData(kEntityPlayer)->car == kCarRedSleeping;
}

bool Entities::isInKitchen(EntityIndex entity) const {
	return isInsideTrainCar(entity, kCarRestaurant) && getData(entity)->entityPosition > kPosition_5800;
}

bool Entities::isNobodyInCompartment(CarIndex car, EntityPosition position) const {
	for (uint i = 1; i < _entities.size(); i++) {
		if (isInsideCompartment((EntityIndex)i, car, position))
			return false;
	}
	return true;
}

bool Entities::checkFields19(EntityIndex entity, CarIndex car, EntityPosition position) const {

	if (getData(entity)->car != car ||  getData(entity)->location != kLocationInsideCompartment)
		return false;

	EntityPosition entityPosition = getData(entity)->entityPosition;

	// Test values
	if (position == kPosition_4455) {
		if (entityPosition == kPosition_4070 || entityPosition == kPosition_4455 || entityPosition == kPosition_4840)
			return true;

		return false;
	}

	if (position == kPosition_6130) {
		if (entityPosition == kPosition_5790 || entityPosition == kPosition_6130 || entityPosition == kPosition_6470)
			return true;

		return false;
	}

	if (position != kPosition_7850
	 || (entityPosition != kPosition_7500 && entityPosition != kPosition_7850 && entityPosition != kPosition_8200))
		return false;

	return true;
}

bool Entities::isInBaggageCarEntrance(EntityIndex entity) const {
	return isInsideTrainCar(entity, kCarBaggage)
		&& getData(entity)->entityPosition >= kPosition_4500
		&& getData(entity)->entityPosition <= kPosition_5500;
}

bool Entities::isInBaggageCar(EntityIndex entity) const {
	return isInsideTrainCar(entity, kCarBaggage) && getData(entity)->entityPosition < kPosition_4500;
}

bool Entities::isInKronosSanctum(EntityIndex entity) const {
	return isInsideTrainCar(entity, kCarKronos)
		&& getData(entity)->entityPosition >= kPosition_3500
		&& getData(entity)->entityPosition <= kPosition_5500;
}

bool Entities::isInKronosCarEntrance(EntityIndex entity) const {
	return isInsideTrainCar(entity, kCarKronos) && getData(entity)->entityPosition > kPosition_7900;
}

bool Entities::checkDistanceFromPosition(EntityIndex entity, EntityPosition position, int distance) const {
	return distance >= ABS(getData(entity)->entityPosition - position);
}

bool Entities::isWalkingOppositeToPlayer(EntityIndex entity) const {
	if (getData(entity)->direction == kDirectionUp && getScenes()->checkPosition(kSceneNone, SceneManager::kCheckPositionLookingDown))
		return true;

	return (getData(entity)->direction == kDirectionDown && getScenes()->checkPosition(kSceneNone, SceneManager::kCheckPositionLookingUp));
}

bool Entities::isFemale(EntityIndex entity) {
	return (entity == kEntityAnna
		 || entity == kEntityTatiana
		 || entity == kEntityVesna
		 || entity == kEntityKahina
		 || entity == kEntityMmeBoutarel
		 || entity == kEntityRebecca
		 || entity == kEntitySophie
		 || entity == kEntityYasmin
		 || entity == kEntityHadija
		 || entity == kEntityAlouan);
}

bool Entities::isMarried(EntityIndex entity) {
	return (entity != kEntityTatiana
		 && entity != kEntityRebecca
		 && entity != kEntitySophie);
}

bool Entities::checkPosition(EntityPosition position) const {
	Position position1 = 0;
	Position position2 = 0;

	switch (position) {
	default:
		return true;

	case kPosition_1500:
		position1 = 1;
		position2 = 23;
		break;

	case kPosition_2740:
		position1 = 3;
		position2 = 25;
		break;

	case kPosition_3050:
		position1 = 5;
		position2 = 26;
		break;

	case kPosition_4070:
		position1 = 7;
		position2 = 28;
		break;

	case kPosition_4840:
		position1 = 9;
		position2 = 30;
		break;

	case kPosition_5790:
		position1 = 11;
		position2 = 32;
		break;

	case kPosition_6470:
		position1 = 13;
		position2 = 34;
		break;

	case kPosition_7500:
		position1 = 15;
		position2 = 36;
		break;

	case kPosition_8200:
		position1 = 17;
		position2 = 38;
		break;
	}

	if (getScenes()->checkPosition(kSceneNone, SceneManager::kCheckPositionLookingUp) && entityPositions[position1] >= getEntityData(kEntityPlayer)->entityPosition)
		return true;
	else
		return (getScenes()->checkPosition(kSceneNone, SceneManager::kCheckPositionLookingDown) && entityPositions[position2] <= getEntityData(kEntityPlayer)->entityPosition);
}

bool Entities::checkSequenceFromPosition(EntityIndex entity) const {
	FrameInfo *info = getEntityData(entity)->sequence->getFrameInfo((uint16)getEntityData(entity)->currentFrame);

	if (getEntityData(entity)->direction == kDirectionUp)
		return (getScenes()->checkPosition(kSceneNone, SceneManager::kCheckPositionLookingUp)
			 && info->entityPosition + getEntityPositionFromCurrentPosition() > kPosition_8513);

	if (getEntityData(entity)->direction == kDirectionDown)
		return (getScenes()->checkPosition(kSceneNone, SceneManager::kCheckPositionLookingDown)
			 && info->entityPosition + getEntityPositionFromCurrentPosition() < kPosition_2087);

	return false;
}

EntityPosition Entities::getEntityPositionFromCurrentPosition() const {
	// Get the scene position first
	Position position = getScenes()->get(getState()->scene)->position;

	if (getScenes()->checkPosition(kSceneNone, SceneManager::kCheckPositionLookingUp))
		return (EntityPosition)(entityPositions[position] - kPosition_1430);

	if (getScenes()->checkPosition(kSceneNone, SceneManager::kCheckPositionLookingDown))
		return (EntityPosition)(entityPositions[position] - kPosition_9020);

	return kPositionNone;
}

void Entities::clearEntitySequenceData(EntityData::EntityCallData *data, EntityDirection direction) const {
	getScenes()->removeAndRedraw(&data->frame, false);
	getScenes()->removeAndRedraw(&data->frame1, false);

	SAFE_DELETE(data->sequence);
	SAFE_DELETE(data->sequence2);

	data->sequenceName = "";
	data->sequenceName2 = "";

	data->field_4A9 = false;
	data->field_4AA = false;
	data->directionSwitch = kDirectionNone;

	data->currentFrame = -1;
	data->currentFrame2 = 0;

	data->direction = direction;
}

} // End of namespace LastExpress

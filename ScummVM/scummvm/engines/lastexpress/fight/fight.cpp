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

#include "lastexpress/fight/fight.h"

#include "lastexpress/fight/fighter_anna.h"
#include "lastexpress/fight/fighter_ivo.h"
#include "lastexpress/fight/fighter_milos.h"
#include "lastexpress/fight/fighter_salko.h"
#include "lastexpress/fight/fighter_vesna.h"

#include "lastexpress/data/cursor.h"
#include "lastexpress/data/sequence.h"

#include "lastexpress/game/entities.h"
#include "lastexpress/game/inventory.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/scenes.h"
#include "lastexpress/game/state.h"

#include "lastexpress/sound/queue.h"

#include "lastexpress/graphics.h"
#include "lastexpress/lastexpress.h"
#include "lastexpress/resource.h"

namespace LastExpress {

Fight::FightData::FightData() {
	player = NULL;
	opponent = NULL;

	index = 0;

	isFightRunning = false;

	memset(&sequences, 0, sizeof(sequences));
}

Fight::FightData::~FightData() {
	SAFE_DELETE(player);
	SAFE_DELETE(opponent);
}

Fight::Fight(LastExpressEngine *engine) : _engine(engine), _data(NULL), _endType(kFightEndLost), _state(0), _handleTimer(false) {
}

Fight::~Fight() {
	clearData();
	_data = NULL;

	// Zero passed pointers
	_engine = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Events
//////////////////////////////////////////////////////////////////////////
void Fight::eventMouse(const Common::Event &ev) {
	if (!_data || _data->index)
		return;

	// TODO move all the egg handling to inventory functions

	getFlags()->mouseLeftClick = false;
	getFlags()->shouldRedraw = false;
	getFlags()->mouseRightClick = false;

	if (ev.mouse.x < 608 || ev.mouse.y < 448 || ev.mouse.x >= 640 || ev.mouse.x >= 480) {

		// Handle right button click
		if (ev.type == Common::EVENT_RBUTTONUP) {
			getSoundQueue()->removeFromQueue(kEntityTables0);
			setStopped();

			getGlobalTimer() ? _state = 0 : ++_state;

			getFlags()->mouseRightClick = true;
		}

		if (_handleTimer) {
			// Timer expired => show with full brightness
			if (!getGlobalTimer())
				getInventory()->drawBlinkingEgg();

			_handleTimer = false;
		}

		// Check hotspots
		Scene *scene = getScenes()->get(getState()->scene);
		SceneHotspot *hotspot = NULL;

		if (!scene->checkHotSpot(ev.mouse, &hotspot)) {
			_engine->getCursor()->setStyle(kCursorNormal);
		} else {
			_engine->getCursor()->setStyle((CursorStyle)hotspot->cursor);

			// Call player function
			if (_data->player->canInteract((Fighter::FightAction)hotspot->action)) {
				if (ev.type == Common::EVENT_LBUTTONUP)
					_data->player->handleAction((Fighter::FightAction)hotspot->action);
			} else {
				_engine->getCursor()->setStyle(kCursorNormal);
			}
		}
	} else {
		// Handle clicks on menu icon

		if (!_handleTimer) {
			// Timer expired => show with full brightness
			if (!getGlobalTimer())
				getInventory()->drawBlinkingEgg();

			_handleTimer = true;
		}

		// Stop fight if clicked
		if (ev.type == Common::EVENT_LBUTTONUP) {
			_handleTimer = false;
			getSoundQueue()->removeFromQueue(kEntityTables0);
			bailout(kFightEndExit);
		}

		// Reset timer on right click
		if (ev.type == Common::EVENT_RBUTTONUP) {
			if (getGlobalTimer()) {
				if (getSoundQueue()->isBuffered("TIMER"))
					getSoundQueue()->removeFromQueue("TIMER");

				setGlobalTimer(900);
			}
		}
	}

	getFlags()->shouldRedraw = true;
}

void Fight::eventTick(const Common::Event &ev) {
	handleTick(ev, true);
}

void Fight::handleTick(const Common::Event &ev, bool isProcessing) {
	// TODO move all the egg handling to inventory functions

	// Blink egg
	if (getGlobalTimer()) {
		warning("[Fight::handleTick] Egg blinking not implemented");
	}

	if (!_data || _data->index)
		return;

	SceneHotspot *hotspot = NULL;
	if (!getScenes()->get(getState()->scene)->checkHotSpot(ev.mouse, &hotspot) || !_data->player->canInteract((Fighter::FightAction)hotspot->action)) {
		_engine->getCursor()->setStyle(kCursorNormal);
	} else {
		_engine->getCursor()->setStyle((CursorStyle)hotspot->cursor);
	}

	_data->player->update();
	_data->opponent->update();

	// Draw sequences
	if (!_data->isFightRunning)
		return;

	if (isProcessing)
		getScenes()->drawFrames(true);

	if (_data->index) {
		// Set next sequence name index
		_data->index--;
		_data->sequences[_data->index] = loadSequence(_data->names[_data->index]);
	}
}

//////////////////////////////////////////////////////////////////////////
// Setup
//////////////////////////////////////////////////////////////////////////
Fight::FightEndType Fight::setup(FightType type) {
	if (_data)
		error("[Fight::setup] Calling fight setup again while a fight is already in progress");

	//////////////////////////////////////////////////////////////////////////
	// Prepare UI & state
	if (_state >= 5 && (type == kFightSalko || type == kFightVesna)) {
		_state = 0;
		return kFightEndWin;
	}

	getInventory()->showHourGlass();
	// TODO events function
	getFlags()->flag_0 = false;
	getFlags()->mouseRightClick = false;
	getEntities()->reset();

	// Compute scene to use
	SceneIndex sceneIndex;
	switch(type) {
	default:
		sceneIndex = kSceneFightDefault;
		break;

	case kFightMilos:
		sceneIndex = (getObjects()->get(kObjectCompartment1).location2 < kObjectLocation3) ? kSceneFightMilos : kSceneFightMilosBedOpened;
		break;

	case kFightAnna:
		sceneIndex = kSceneFightAnna;
		break;

	case kFightIvo:
		sceneIndex = kSceneFightIvo;
		break;

	case kFightSalko:
		sceneIndex = kSceneFightSalko;
		break;

	case kFightVesna:
		sceneIndex = kSceneFightVesna;
		break;
	}

	if (getFlags()->shouldRedraw) {
		getFlags()->shouldRedraw = false;
		askForRedraw();
		//redrawScreen();
	}

	// Load the scene object
	Scene *scene = getScenes()->get(sceneIndex);

	// Update game entities and state
	getEntityData(kEntityPlayer)->entityPosition = scene->entityPosition;
	getEntityData(kEntityPlayer)->location = scene->location;

	getState()->scene = sceneIndex;

	getFlags()->flag_3 = true;

	// Draw the scene
	_engine->getGraphicsManager()->draw(scene, GraphicsManager::kBackgroundC);
	// FIXME move to start of fight?
	askForRedraw();
	redrawScreen();

	//////////////////////////////////////////////////////////////////////////
	// Setup the fight
	_data = new FightData;
	loadData(type);

	// Show opponents & egg button
	Common::Event emptyEvent;
	handleTick(emptyEvent, false);
	getInventory()->drawEgg();

	// Start fight
	_endType = kFightEndLost;
	while (_data->isFightRunning) {
		if (_engine->handleEvents())
			continue;

		getSoundQueue()->updateQueue();
	}

	// Cleanup after fight is over
	clearData();

	return _endType;
}

//////////////////////////////////////////////////////////////////////////
// Status
//////////////////////////////////////////////////////////////////////////
void Fight::setStopped() {
	if (_data)
		_data->isFightRunning = false;
}

void Fight::bailout(FightEndType type) {
	_state = 0;
	_endType = type;
	setStopped();
}

//////////////////////////////////////////////////////////////////////////
// Cleanup
//////////////////////////////////////////////////////////////////////////
void Fight::clearData() {
	if (!_data)
		return;

	// Clear data
	SAFE_DELETE(_data);

	_engine->restoreEventHandlers();
}

//////////////////////////////////////////////////////////////////////////
// Loading
//////////////////////////////////////////////////////////////////////////
void Fight::loadData(FightType type) {
	if (!_data)
		error("[Fight::loadData] Data not initialized");

	switch (type) {
	default:
		break;

	case kFightMilos:
		_data->player   = new FighterPlayerMilos(_engine);
		_data->opponent = new FighterOpponentMilos(_engine);
		break;

	case kFightAnna:
		_data->player   = new FighterPlayerAnna(_engine);
		_data->opponent = new FighterOpponentAnna(_engine);
		break;

	case kFightIvo:
		_data->player   = new FighterPlayerIvo(_engine);
		_data->opponent = new FighterOpponentIvo(_engine);
		break;

	case kFightSalko:
		_data->player   = new FighterPlayerSalko(_engine);
		_data->opponent = new FighterOpponentSalko(_engine);
		break;

	case kFightVesna:
		_data->player   = new FighterPlayerVesna(_engine);
		_data->opponent = new FighterOpponentVesna(_engine);
		break;
	}

	if (!_data->player || !_data->opponent)
		error("[Fight::loadData] Error loading fight data (type=%d)", type);

	// Setup opponent pointers
	setOpponents();

	//////////////////////////////////////////////////////////////////////////
	// Start running the fight
	_data->isFightRunning = true;

	if (_state < 5) {
		_data->player->setSequenceAndDraw(0, Fighter::kFightSequenceType0);
		_data->opponent->setSequenceAndDraw(0, Fighter::kFightSequenceType0);
		goto end_load;
	}

	switch(type) {
	default:
		break;

	case kFightMilos:
		_data->opponent->setCountdown(1);
		_data->player->setSequenceAndDraw(4, Fighter::kFightSequenceType0);
		_data->opponent->setSequenceAndDraw(0, Fighter::kFightSequenceType0);
		break;

	case kFightIvo:
		_data->opponent->setCountdown(1);
		_data->player->setSequenceAndDraw(3, Fighter::kFightSequenceType0);
		_data->opponent->setSequenceAndDraw(6, Fighter::kFightSequenceType0);
		break;

	case kFightVesna:
		_data->opponent->setCountdown(1);
		_data->player->setSequenceAndDraw(0, Fighter::kFightSequenceType0);
		_data->player->setSequenceAndDraw(3, Fighter::kFightSequenceType2);
		_data->opponent->setSequenceAndDraw(5, Fighter::kFightSequenceType0);
		break;
	}

end_load:
	// Setup event handlers
	_engine->backupEventHandlers();
	SET_EVENT_HANDLERS(Fight, this);
}

void Fight::setOpponents() {
	if (!_data)
		error("[Fight::setOpponents] Data not initialized");

	_data->player->setOpponent(_data->opponent);
	_data->opponent->setOpponent(_data->player);

	_data->player->setFight(this);
	_data->opponent->setFight(this);
}

} // End of namespace LastExpress

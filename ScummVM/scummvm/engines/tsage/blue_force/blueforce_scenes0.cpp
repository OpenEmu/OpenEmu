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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "tsage/blue_force/blueforce_scenes0.h"
#include "tsage/blue_force/blueforce_dialogs.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace TsAGE {

namespace BlueForce {

/*--------------------------------------------------------------------------
 * Scene 20 - Tsunami Title Screen
 *
 *--------------------------------------------------------------------------*/

void Scene20::Action1::signal() {
	Scene20 *scene = (Scene20 *)BF_GLOBALS._sceneManager._scene;
	static byte black[3] = { 0, 0, 0 };

	switch (_actionIndex++) {
	case 0:
		setDelay(2);
		break;
	case 1:
		_sound.play(1);
		BF_GLOBALS._scenePalette.addRotation(64, 127, -1, 1, this);
		break;
	case 2:
		scene->_tsunamiWave.setVisage(22);
		scene->_tsunamiWave._strip = 1;
		scene->_tsunamiWave._frame = 1;
		scene->_tsunamiWave.changeZoom(100);

		scene->_letterT.setVisage(22);
		scene->_letterT._strip = 2;
		scene->_letterT._frame = 1;
		scene->_letterT.changeZoom(100);

		scene->_letterS.setVisage(22);
		scene->_letterS._strip = 3;
		scene->_letterS._frame = 1;
		scene->_letterS.changeZoom(100);

		scene->_letterU.setVisage(22);
		scene->_letterU._strip = 4;
		scene->_letterU._frame = 1;
		scene->_letterU.changeZoom(100);

		scene->_letterN.setVisage(22);
		scene->_letterN._strip = 5;
		scene->_letterN._frame = 1;
		scene->_letterN.changeZoom(100);

		scene->_letterA.setVisage(22);
		scene->_letterA._strip = 6;
		scene->_letterA._frame = 1;
		scene->_letterA.changeZoom(100);

		scene->_letterM.setVisage(22);
		scene->_letterM._strip = 7;
		scene->_letterM._frame = 1;
		scene->_letterM.changeZoom(100);

		scene->_letterI.setVisage(22);
		scene->_letterI._strip = 8;
		scene->_letterI._frame = 1;
		scene->_letterI.changeZoom(100);

		setDelay(1);
		break;
	case 3:
		BF_GLOBALS._scenePalette.addFader(scene->_scenePalette._palette, 256, 8, this);
		break;
	case 4:
		setDelay(60);
		break;
	case 5:
		scene->_letterT.animate(ANIM_MODE_5, NULL);
		scene->_letterS.animate(ANIM_MODE_5, NULL);
		scene->_letterU.animate(ANIM_MODE_5, NULL);
		scene->_letterN.animate(ANIM_MODE_5, NULL);
		scene->_letterA.animate(ANIM_MODE_5, NULL);
		scene->_letterM.animate(ANIM_MODE_5, this);
		break;
	case 6:
		setDelay(120);
		break;
	case 7:
		BF_GLOBALS._scenePalette.addFader(black, 1, 5, this);
		break;
	case 8:
		BF_GLOBALS._sceneManager.changeScene(100);
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene20::postInit(SceneObjectList *OwnerList) {
	loadScene(20);
	SceneExt::postInit();
	setZoomPercents(60, 85, 200, 100);
	BF_GLOBALS._interfaceY = SCREEN_HEIGHT;

	_scenePalette.loadPalette(1);
	_scenePalette.loadPalette(22);

	_tsunamiWave.postInit();
	_tsunamiWave.setVisage(21);
	_tsunamiWave._strip = 1;
	_tsunamiWave._frame = 1;
	_tsunamiWave.animate(ANIM_MODE_NONE, NULL);
	_tsunamiWave.setPosition(Common::Point(62, 85));
	_tsunamiWave.changeZoom(100);

	_letterT.postInit();
	_letterT.setVisage(21);
	_letterT._strip = 2;
	_letterT._frame = 1;
	_letterT.animate(ANIM_MODE_NONE, NULL);
	_letterT.setPosition(Common::Point(27, 94));
	_letterT.changeZoom(100);

	_letterS.postInit();
	_letterS.setVisage(21);
	_letterS._strip = 2;
	_letterS._frame = 2;
	_letterS.animate(ANIM_MODE_NONE, NULL);
	_letterS.setPosition(Common::Point(68, 94));
	_letterS.changeZoom(100);

	_letterU.postInit();
	_letterU.setVisage(21);
	_letterU._strip = 2;
	_letterU._frame = 3;
	_letterU.animate(ANIM_MODE_NONE, NULL);
	_letterU.setPosition(Common::Point(110, 94));
	_letterU.changeZoom(100);

	_letterN.postInit();
	_letterN.setVisage(21);
	_letterN._strip = 2;
	_letterN._frame = 4;
	_letterN.animate(ANIM_MODE_NONE, NULL);
	_letterN.setPosition(Common::Point(154, 94));
	_letterN.changeZoom(100);

	_letterA.postInit();
	_letterA.setVisage(21);
	_letterA._strip = 2;
	_letterA._frame = 5;
	_letterA.animate(ANIM_MODE_NONE, NULL);
	_letterA.setPosition(Common::Point(199, 94));
	_letterA.changeZoom(100);

	_letterM.postInit();
	_letterM.setVisage(21);
	_letterM._strip = 2;
	_letterM._frame = 6;
	_letterM.animate(ANIM_MODE_NONE, NULL);
	_letterM.setPosition(Common::Point(244, 94));
	_letterM.changeZoom(100);

	_letterI.postInit();
	_letterI.setVisage(21);
	_letterI._strip = 2;
	_letterI._frame = 7;
	_letterI.animate(ANIM_MODE_NONE, NULL);
	_letterI.setPosition(Common::Point(286, 94));
	_letterI.changeZoom(100);

	setAction(&_action1);
	BF_GLOBALS._dialogCenter.y = 165;
}

/*--------------------------------------------------------------------------
 * Scene 50 - Map Screen
 *
 *--------------------------------------------------------------------------*/

Scene50::Tooltip::Tooltip(): SavedObject() {
	_newSceneNumber = _locationId = 0;
}

void Scene50::Tooltip::synchronize(Serializer &s) {
	SavedObject::synchronize(s);
	_bounds.synchronize(s);
	s.syncString(_msg);

	if (s.getVersion() >= 10) {
		s.syncAsSint16LE(_newSceneNumber);
		s.syncAsSint16LE(_locationId);
	}
}

void Scene50::Tooltip2::signal() {
	switch (_actionIndex++) {
	case 0:
		setDelay(60);
		break;
	case 1: {
		Common::Point pt(410, 181);
		NpcMover *mover = new NpcMover();
		((SceneObject *)_owner)->addMover(mover, &pt, this);
		break;
	}
	case 2:
		_owner->remove();
		break;
	default:
		break;
	}
}

void Scene50::Tooltip2::dispatch() {
	Action::dispatch();
	SceneObject *owner = (SceneObject *)_owner;

	if ((_actionIndex == 2) && (owner->_percent < 100)) {
		owner->changeZoom(owner->_percent + 1);
	}
}

void Scene50::Tooltip::set(const Rect &bounds, int sceneNum, const Common::String &msg, int locationId) {
	_bounds = bounds;
	_newSceneNumber = sceneNum;
	_msg = msg;
	_locationId = locationId;
}

void Scene50::Tooltip::update() {
	// Set up the text object for the scene with the appropriate settings
	Scene50 *scene = (Scene50 *)BF_GLOBALS._sceneManager._scene;
	scene->_text._fontNumber = 10;
	scene->_text._color1 = BF_GLOBALS._scenePalette._colors.foreground;
	scene->_text._color2 = BF_GLOBALS._scenePalette._colors.background;
	scene->_text.setup(_msg);

	int yp = (scene->_text._textSurface.getBounds().height() == 0) ? _bounds.bottom : _bounds.top;
	scene->_text.setPosition(Common::Point(_bounds.left, yp));
}

void Scene50::Tooltip::highlight(bool btnDown) {
	Scene50 *scene = (Scene50 *)BF_GLOBALS._sceneManager._scene;

	// In the original, a variable was used, always set to 0. The check is simplified
	if (_newSceneNumber != 0)
		update();

	if (btnDown) {
		if ((BF_GLOBALS._bookmark == bCalledToDrunkStop) && BF_GLOBALS.getFlag(beenToJRDay2))
			scene->_sceneNumber = 600;
		else if (BF_GLOBALS._bookmark == bBookedGreen)
			scene->_sceneNumber = 410;
		else {
			BF_GLOBALS._driveToScene = _newSceneNumber;

			switch (BF_GLOBALS._driveFromScene) {
			case 330:
			case 340:
			case 342:
				BF_GLOBALS._player.disableControl();
				BF_GLOBALS._mapLocationId = _locationId;

				if (BF_GLOBALS._driveToScene != 330) {
					scene->_sceneNumber = 330;
				} else {
					scene->_sceneNumber = (BF_GLOBALS._dayNumber != 1) || (BF_GLOBALS._bookmark < bStartOfGame) ||
						(BF_GLOBALS._bookmark >= bCalledToDomesticViolence) ? 342 : 340;
				}
				break;

			case 410:
			case 551:
				if (BF_GLOBALS.getFlag((BF_GLOBALS._driveFromScene == 410) ? fSearchedTruck : didDrunk)) {
					BF_GLOBALS._mapLocationId = _locationId;
					BF_GLOBALS._player.disableControl();
					scene->_sceneNumber = _newSceneNumber;
				} else {
					BF_GLOBALS._deathReason = 4;
					BF_GLOBALS._sceneManager.changeScene(666);
					return;
				}
				break;

			case 300:
				if (_locationId == 1) {
					BF_GLOBALS._driveToScene = 300;
					_newSceneNumber = 300;
				}
				// Deliberate fall through to default
			default:
				BF_GLOBALS._mapLocationId = _locationId;
				BF_GLOBALS._player.disableControl();
				scene->_sceneNumber = _newSceneNumber;
				break;
			}
		}

		// Signal the scene to change to the new scene
		scene->_sceneMode = 1;
		scene->signal();
	}
}

/*--------------------------------------------------------------------------*/

Scene50::Scene50() {
	_sceneNumber = 0;
}


void Scene50::synchronize(Serializer &s) {
	if (s.getVersion() >= 10) {
		SceneExt::synchronize(s);
		s.syncAsSint16LE(_sceneNumber);
	}
}

void Scene50::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();

	BF_GLOBALS._interfaceY = 200;
	T2_GLOBALS._uiElements._active = false;
	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.setVisage(830);
	BF_GLOBALS._player.setStrip(3);
	BF_GLOBALS._player.setPosition(Common::Point(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2));
	BF_GLOBALS._player.hide();
	BF_GLOBALS._player.disableControl();

	BF_GLOBALS._scrollFollower = NULL;
	_text._color1 = 19;
	_text._color2 = 9;
	_text._color3 = 9;
	_text._width = 75;
	_text._fontNumber = 4;
	_text._textMode = ALIGN_LEFT;
	_text.fixPriority(256);

	// Set all the hotspots
	_location3.set(Rect(10, 92, 24, 105), 180, GRANDMA_FRANNIE, 4);
	_location2.set(Rect(443, 149, 508, 178), 330, MARINA, 2);
	_location1.set(Rect(573, 103, 604, 130), 190, POLICE_DEPARTMENT, 1);
	_location4.set(Rect(313, 21, 325, 33), 114, TONYS_BAR, 8);
	_location8.set(Rect(69, 79, 82, 88), 580, CHILD_PROTECTIVE_SERVICES, 256);
	_location6.set(Rect(242, 131, 264, 144), 440, ALLEY_CAT, 64);
	_location5.set(Rect(383, 57, 402, 70), 380, CITY_HALL_JAIL, 32);
	_location7.set(Rect(128, 32, 143, 42), 800, JAMISON_RYAN, 128);
	_location9.set(Rect(349, 125, 359, 132),
		(BF_GLOBALS._bookmark == bInspectionDone) || (BF_GLOBALS._bookmark == bCalledToDrunkStop) ? 551 : 550,
		BIKINI_HUT, 16);

	_item.setBounds(Rect(0, 0, SCREEN_WIDTH * 2, SCREEN_HEIGHT));
	BF_GLOBALS._sceneItems.push_back(&_item);

	// Find the location to start at
	int selectedTooltip = BF_GLOBALS._mapLocationId;
	Tooltip *tooltip = NULL;
	int xp = 0;

	switch (selectedTooltip) {
	case 1:
		tooltip = &_location1;
		xp = 588;
		break;
	case 2:
		tooltip = &_location2;
		xp = 475;
		break;
	case 4:
		tooltip = &_location3;
		xp = 17;
		break;
	case 8:
		tooltip = &_location4;
		xp = 319;
		break;
	case 32:
		tooltip = &_location5;
		xp = 392;
		break;
	case 64:
		tooltip = &_location6;
		xp = 253;
		break;
	case 128:
		tooltip = &_location7;
		xp = 135;
		break;
	case 16:
		tooltip = &_location9;
		xp = 354;
		break;
	case 256:
		tooltip = &_location8;
		xp = 75;
		break;
	}

	_timer.set(240, this);
	_sceneBounds.center(xp, SCREEN_HEIGHT / 2);
	loadScene(55);
	_sceneMode = 2;

	loadBackground(xp - 160, 0);
	tooltip->update();
}

void Scene50::remove() {
	// Blank out the screen
	clearScreen();
	BF_GLOBALS._scrollFollower = &BF_GLOBALS._player;

	SceneExt::remove();
	T2_GLOBALS._uiElements._active = true;
}

void Scene50::signal() {
	if (_sceneMode == 1) {
		// Destination selected
		if ((BF_GLOBALS._driveFromScene == 551) && (_sceneNumber != BF_GLOBALS._driveFromScene)) {
			BF_GLOBALS.clearFlag(f1015Drunk);
			BF_GLOBALS.clearFlag(f1027Drunk);
			BF_GLOBALS.clearFlag(f1035Drunk);
			BF_GLOBALS.clearFlag(f1097Drunk);
			BF_GLOBALS.clearFlag(f1098Drunk);
		}

		if ((BF_GLOBALS._driveFromScene == 410) && (_sceneNumber != BF_GLOBALS._driveFromScene)) {
			BF_GLOBALS.setFlag(f1097Frankie);
		}

		if ((BF_GLOBALS._driveFromScene == 340) && (_sceneNumber != BF_GLOBALS._driveFromScene)) {
			BF_GLOBALS.setFlag(f1097Marina);
		}

		if ((BF_GLOBALS._driveFromScene == 380) && (_sceneNumber != BF_GLOBALS._driveFromScene)) {
			if (BF_GLOBALS._bookmark >= bLauraToParamedics)
				BF_GLOBALS.setFlag(f1098Marina);
			if (BF_GLOBALS._bookmark >= bStoppedFrankie)
				BF_GLOBALS.setFlag(f1098Frankie);
			if (BF_GLOBALS._bookmark == bArrestedGreen) {
				BF_GLOBALS._deathReason = 19;
				_sceneNumber = 666;
			}
		}

		if ((_sceneNumber == 551) && BF_GLOBALS.getFlag(fHasDrivenFromDrunk))
			_sceneNumber = 550;

		BF_GLOBALS._sound1.fadeOut2(NULL);
		BF_GLOBALS._sceneManager.changeScene(_sceneNumber);

	} else if (_sceneMode == 2) {
		// Initial delay complete, time to switch to interactive mode
		_text.remove();
		BF_GLOBALS._player.enableControl();
		BF_GLOBALS._events.setCursor(CURSOR_WALK);
		_sceneMode = 0;
	}
}

void Scene50::process(Event &event) {
	SceneExt::process(event);
	Common::Point pt(event.mousePos.x + _sceneBounds.left, event.mousePos.y + _sceneBounds.top);
	bool mouseDown = false;

	if ((event.mousePos.x > 270 && (_sceneBounds.right < (SCREEN_WIDTH * 2))))
		loadBackground(4, 0);
	else if ((event.mousePos.x < 50) && (_sceneBounds.left > 0))
		loadBackground(-4, 0);
	else
		mouseDown = event.eventType == EVENT_BUTTON_DOWN;

	if (BF_GLOBALS._player._uiEnabled) {
		Tooltip *tooltipList[9] = { &_location1, &_location2, &_location3, &_location4,
			&_location5, &_location6, &_location7, &_location8, &_location9 };

		for (int idx = 0; idx < 9; ++idx) {
			if (tooltipList[idx]->_bounds.contains(pt)) {
				// Found a tooltip to highlight
				tooltipList[idx]->highlight(mouseDown);
				return;
			}
		}

		// No hotspot selected, so remove any current tooltip display
		_text.remove();
	}
}

/*--------------------------------------------------------------------------
 * Scene 60 - Motorcycle
 *
 *--------------------------------------------------------------------------*/

bool Scene60::Ignition::startAction(CursorType action, Event &event) {
	Scene60 *scene = (Scene60 *)BF_GLOBALS._sceneManager._scene;
	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(60, 15);
		break;
	default:
		switch (BF_GLOBALS._dayNumber) {
		case 1:
			if (BF_GLOBALS.getFlag(onDuty) && check1())
				return true;
			break;
		case 2:
			if (BF_GLOBALS.getFlag(onDuty) && check2())
				return true;
		}

		BF_GLOBALS._sound1.play(BF_GLOBALS.getFlag(fWithLyle) ? 80 : 31);
		BF_GLOBALS._sound1.holdAt(1);
		scene->fadeOut();
		BF_GLOBALS._sceneManager.changeScene(50);
		break;
	}

	return true;
}

bool Scene60::Ignition::check1() {
	if (BF_GLOBALS._bookmark >= bStoppedFrankie) {
		BF_GLOBALS._subFlagBitArr1 |= 1;
		return false;
	} else {
		if ((BF_GLOBALS._bookmark == bBookedGreen) && BF_GLOBALS.getFlag(fArrivedAtGangStop)) {
			BF_GLOBALS.set2Flags(f1035Frankie);
			BF_GLOBALS._sceneManager.changeScene(410);
		}

		if (BF_GLOBALS._bookmark >= bLauraToParamedics) {
			if (BF_GLOBALS.getFlag(fLeftTraceIn910)) {
				if (BF_GLOBALS._bookmark < bBookedGreen) {
					BF_GLOBALS._bookmark = bBookedGreen;
					BF_GLOBALS.clearFlag(fCalledBackup);
					BF_GLOBALS.set2Flags(f1035Frankie);
					return false;
				} else if (BF_GLOBALS._bookmark == bBookedGreen) {
					if (!BF_GLOBALS.getFlag(fCalledBackup))
						BF_GLOBALS.setFlag(f1035Frankie);

					BF_GLOBALS._sceneManager.changeScene(410);
					return true;
				}
			}

		} else if (BF_GLOBALS._bookmark < bStartOfGame) {
			// Should never reach here
		} else if (BF_GLOBALS._bookmark < bCalledToDomesticViolence) {
			if ((BF_GLOBALS._subFlagBitArr1 >> 1) & 1)
				BF_GLOBALS.setFlag(fLateToMarina);
			else
				BF_GLOBALS._subFlagBitArr1 |= 2;
		} else {
			int v = (((BF_GLOBALS._subFlagBitArr1 >> 2) & 15) + 1) & 15;
			BF_GLOBALS._subFlagBitArr1 = (BF_GLOBALS._subFlagBitArr1 & 0xC3) | (v << 2);

			if ((v != 1) && (v != 2)) {
				BF_GLOBALS._deathReason = 19;
				BF_GLOBALS._sceneManager.changeScene(666);
				return true;
			}
		}
	}

	BF_GLOBALS._subFlagBitArr1 |= 1;
	return false;
}

bool Scene60::Ignition::check2() {
	switch (BF_GLOBALS._bookmark) {
	case bInspectionDone:
		if (BF_GLOBALS._subFlagBitArr2 & 1) {
			BF_GLOBALS.setFlag(fLateToDrunkStop);
		} else {
			BF_GLOBALS._subFlagBitArr2 |= 1;
		}
		break;
	case bCalledToDrunkStop:
		BF_GLOBALS.setFlag(fHasDrivenFromDrunk);
		break;
	default:
		break;
	}

	BF_GLOBALS._subFlagBitArr1 |= 0x80;
	return false;
}

/*--------------------------------------------------------------------------*/

bool Scene60::Item3::startAction(CursorType action, Event &event) {
	Scene60 *scene = (Scene60 *)BF_GLOBALS._sceneManager._scene;
	scene->fadeOut();
	BF_GLOBALS._sceneManager.changeScene(scene->_newScene);
	return true;
}

bool Scene60::Radio::startAction(CursorType action, Event &event) {
	Scene60 *scene = (Scene60 *)BF_GLOBALS._sceneManager._scene;

	switch(action) {
	case CURSOR_LOOK:
		SceneItem::display2(60, 0);
		break;
	case CURSOR_USE:
	case CURSOR_TALK:
		scene->_sound.play(32);
		scene->setAction(&scene->_action1);
		break;
	default:
		SceneItem::display2(60, 1);
		break;
	}
	return true;
}

bool Scene60::Compartment::startAction(CursorType action, Event &event) {
	Scene60 *scene = (Scene60 *)BF_GLOBALS._sceneManager._scene;

	switch(action) {
	case CURSOR_LOOK:
		SceneItem::display2(60, 8);
		break;
	case CURSOR_USE:
		if ((BF_INVENTORY.getObjectScene(INV_TICKET_BOOK) == 1) &&
				(BF_INVENTORY.getObjectScene(INV_MIRANDA_CARD) == 1)) {
			SceneItem::display2(60, 9);
		}
		break;
	case CURSOR_TALK:
		SceneItem::display2(60, 10);
		break;
	case INV_TICKET_BOOK:
		SceneItem::display2(60, 11);
		scene->_ticketBook.show();
		BF_INVENTORY.setObjectScene(INV_TICKET_BOOK, 60);
		BF_GLOBALS._events.setCursor(CURSOR_USE);
		BF_GLOBALS._sceneItems.addBefore(&scene->_radio, &scene->_ticketBook);
		break;
	case INV_MIRANDA_CARD:
		SceneItem::display2(60, 12);
		scene->_mirandaCard.show();
		BF_INVENTORY.setObjectScene(INV_MIRANDA_CARD, 60);
		BF_GLOBALS._events.setCursor(CURSOR_USE);
		BF_GLOBALS._sceneItems.addAfter(&scene->_compartmentDoor, &scene->_mirandaCard);
		break;
	default:
		return NamedHotspot::startAction(action, event);
	}

	return true;
}

/*--------------------------------------------------------------------------*/

bool Scene60::MirandaCard::startAction(CursorType action, Event &event) {
	Scene60 *scene = (Scene60 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(60, 5);
		return true;
	case CURSOR_USE:
		if (BF_INVENTORY.getObjectScene(INV_MIRANDA_CARD) == 60) {
			SceneItem::display2(60, 6);
			BF_INVENTORY.setObjectScene(INV_MIRANDA_CARD, 1);
			if (!BF_GLOBALS.getFlag(fGotPointsForTktBook)) {
				T2_GLOBALS._uiElements.addScore(10);
				BF_GLOBALS.setFlag(fGotPointsForTktBook);
			}

			scene->_mirandaCard.hide();
			BF_GLOBALS._sceneItems.remove(&scene->_mirandaCard);
		}
		return true;
	case CURSOR_TALK:
		SceneItem::display2(60, 7);
		return true;
	default:
		return NamedObject::startAction(action, event);
		break;
	}
}

bool Scene60::TicketBook::startAction(CursorType action, Event &event) {
	Scene60 *scene = (Scene60 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(60, 2);
		return true;
	case CURSOR_USE:
		if (BF_INVENTORY.getObjectScene(INV_TICKET_BOOK) == 60) {
			scene->_ticketBook.hide();
			BF_GLOBALS._sceneItems.remove(&scene->_ticketBook);
			SceneItem::display2(60, 3);
			BF_INVENTORY.setObjectScene(INV_TICKET_BOOK, 1);
			if (!BF_GLOBALS.getFlag(fShotNicoIn910)) {
				T2_GLOBALS._uiElements.addScore(10);
				BF_GLOBALS.setFlag(fShotNicoIn910);
			}
		}
		return true;
	case CURSOR_TALK:
		SceneItem::display2(60, 4);
		return true;
	default:
		return NamedObject::startAction(action, event);
		break;
	}
}

bool Scene60::CompartmentDoor::startAction(CursorType action, Event &event) {
	Scene60 *scene = (Scene60 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(60, 13);
		return true;
	case CURSOR_USE:
		if (_flag) {
			_flag = false;
			BF_GLOBALS._player.disableControl();
			Common::Point pt(308, 165);
			NpcMover *mover = new NpcMover();
			addMover(mover, &pt, scene);
		} else {
			_flag = true;
			BF_GLOBALS._player.disableControl();
			Common::Point pt(288, 165);
			NpcMover *mover = new NpcMover();
			addMover(mover, &pt, scene);
		}
		return true;
	case CURSOR_TALK:
		SceneItem::display2(60, 14);
		return true;
	default:
		return NamedObject::startAction(action, event);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene60::Action1::signal() {
	Scene60 *scene = (Scene60 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(2);
		break;
	case 1:
		scene->_stripManager.start(634, this);
		break;
	case 2:
		_state = useRadio();
		setDelay(4);
		break;
	case 3:
		switch (_state) {
		case 1:
			if (BF_GLOBALS.removeFlag(fCan1004Marina)) {
				T2_GLOBALS._uiElements.addScore(10);
				_state = 606;
			} else if (BF_GLOBALS.removeFlag(fCan1004Drunk)) {
				T2_GLOBALS._uiElements.addScore(10);
				_state = 606;
			} else {
				_state = 611;
			}
			break;
		case 2:
			_state = 612;
			break;
		case 3:
			if (BF_GLOBALS.removeFlag(f1015Marina)) {
				T2_GLOBALS._uiElements.addScore(10);
				_state = 613;
			} else if (BF_GLOBALS.removeFlag(f1015Frankie)) {
				T2_GLOBALS._uiElements.addScore(10);
				_state = 614;
			} else if (BF_GLOBALS.removeFlag(f1015Drunk)) {
				T2_GLOBALS._uiElements.addScore(10);
				_state = 615;
			} else {
				_state = 616;
			}
			break;
		case 4:
			if (BF_GLOBALS.removeFlag(f1027Marina)) {
				T2_GLOBALS._uiElements.addScore(10);
				_actionIndex = 5;
				_state = 617;
			} else if (BF_GLOBALS.removeFlag(f1027Frankie)) {
				T2_GLOBALS._uiElements.addScore(10);
				_actionIndex = 5;
				_state = 618;
			} else if (BF_GLOBALS.removeFlag(f1015Drunk)) {
				T2_GLOBALS._uiElements.addScore(10);
				_actionIndex = 5;
				_state = 619;
			} else {
				_state = 620;
			}
			break;
		case 5:
			if (BF_GLOBALS.removeFlag(f1035Marina)) {
				BF_GLOBALS.setFlag(fCalledBackup);
				T2_GLOBALS._uiElements.addScore(50);
				_state = 621;
			} else if (BF_GLOBALS.removeFlag(f1035Frankie)) {
				BF_GLOBALS.setFlag(fCalledBackup);
				T2_GLOBALS._uiElements.addScore(50);
				_actionIndex = 5;
				_state = 622;
			} else if (BF_GLOBALS.removeFlag(f1035Drunk)) {
				T2_GLOBALS._uiElements.addScore(10);
				_state = 623;
			} else {
				_state = 624;
			}
			break;
		case 6:
			if (BF_GLOBALS.removeFlag(f1097Marina)) {
				T2_GLOBALS._uiElements.addScore(10);
				_state = 625;
			} else if (BF_GLOBALS.removeFlag(f1097Frankie)) {
				T2_GLOBALS._uiElements.addScore(10);
				_actionIndex = 5;
				_state = 626;
			} else if (BF_GLOBALS.removeFlag(f1097Drunk)) {
				T2_GLOBALS._uiElements.addScore(10);
				_state = 627;
			} else {
				_state = 628;
			}
			break;
		case 7:
			if (BF_GLOBALS.removeFlag(f1098Marina)) {
				T2_GLOBALS._uiElements.addScore(10);
				_state = 629;
			} else if (BF_GLOBALS.removeFlag(f1098Frankie)) {
				T2_GLOBALS._uiElements.addScore(10);
				_state = 630;
			} else if (BF_GLOBALS.removeFlag(f1098Drunk)) {
				T2_GLOBALS._uiElements.addScore(10);
				_state = 631;
			} else {
				_state = 632;
			}
			break;
		case 0:
		default:
			_state = 610;
			break;
		}

		scene->_stripManager.start(_state, this);
		break;
	case 4:
		remove();
	case 5:
		setDelay(120);
		break;
	case 6:
		_actionIndex = 4;
		scene->_stripManager.start(633, this);
		break;
	}
}

int Scene60::Action1::useRadio() {
	return RadioConvDialog::show();
}

void Scene60::Action2::signal() {
	Scene60 *scene = (Scene60 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.disableControl();
		scene->_sound.play(32);
		setDelay(2);
		break;
	case 1:
		BF_GLOBALS._bookmark = bStartOfGame;
		BF_GLOBALS.set2Flags(f1035Marina);
		scene->_stripManager.start(60, this);
		break;
	case 2:
		BF_GLOBALS._player.enableControl();
		remove();
		break;
	}
}

void Scene60::Action3::signal() {
	Scene60 *scene = (Scene60 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.disableControl();
		scene->_sound.play(32);
		setDelay(2);
		break;
	case 1:
		BF_GLOBALS._bookmark = bInspectionDone;
		BF_GLOBALS.set2Flags(f1035Drunk);
		BF_GLOBALS.setFlag(fCan1004Drunk);
		scene->_stripManager.start(71, this);
		break;
	case 2:
		scene->_field1222 = true;
		BF_GLOBALS._player.enableControl();
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene60::Scene60(): SceneExt() {
	_newScene = 0;
	_sceneNumber = 0;
	_visage = 0;
	_cursorId = CURSOR_NONE;
	_field1222 = false;
}

void Scene60::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_newScene);
	s.syncAsSint16LE(_sceneNumber);
	s.syncAsSint16LE(_visage);
	s.syncAsSint16LE(_cursorId);
	s.syncAsSint16LE(_field1222);
}

void Scene60::postInit(SceneObjectList *OwnerList) {
	_newScene = BF_GLOBALS._driveFromScene = BF_GLOBALS._sceneManager._previousScene;

	// Set up which scene background to use
	switch (_newScene) {
	case 300:
		_sceneNumber = 1301;
		break;
	case 380:
		_sceneNumber = 1380;
		break;
	case 410:
		_sceneNumber = 1410;
		break;
	case 551:
		_sceneNumber = 1550;
		break;
	case 550:
		_sceneNumber = 1555;
		break;
	case 580:
		_sceneNumber = 1580;
		break;
	case 800:
		_sceneNumber = 1810;
		break;
	default:
		_sceneNumber = 60;
		break;
	}

	if (_sceneNumber == 1550) {
		if (BF_GLOBALS.getFlag(fHasDrivenFromDrunk))
			_sceneNumber = 1555;
		else {
			_object1.postInit();
			_object1.setVisage(1550);
			_object1.animate(ANIM_MODE_2);
			_object1.setPosition(Common::Point(158, 18));
		}
	}

	loadScene(_sceneNumber);

	if ((_sceneNumber == 1810) && (BF_GLOBALS._dayNumber > 1) &&
			(BF_GLOBALS._dayNumber < 5) && !BF_GLOBALS.getFlag(fWithLyle) &&
			((BF_GLOBALS._dayNumber != 4) && (BF_GLOBALS._bookmark >= bEndDayThree))) {
		_car.setup(1810, 1, 1, 164, 131, 1);
	}

	if ((_sceneNumber == 1410) && (BF_GLOBALS._bookmark == bBookedGreen) &&
			!BF_GLOBALS.getFlag(fDriverOutOfTruck)) {
		_object1.postInit();
		_object1.setVisage(410);
		_object1.setStrip(6);
		_object1.setPosition(Common::Point(135, 47));
	}

	if (BF_GLOBALS.getFlag(fWithLyle)) {
		_visage = 62;
		_ignition._sceneRegionId = 22;
	} else if (BF_GLOBALS.getFlag(onDuty)) {
		_visage = 63;
		_ignition._sceneRegionId = 20;
	} else {
		_visage = 61;
		_ignition._sceneRegionId = 28;
	}
	_dashboard.setup(_visage, 1, 1, 160, 168, 100);
	_cursorId = CURSOR_USE;

	if (_visage == 63) {
		_compartmentDoor.postInit();
		_compartmentDoor.setVisage(60);
		_compartmentDoor.setStrip(1);
		_compartmentDoor.setFrame(1);
		_compartmentDoor.setPosition(Common::Point(288, 165));
		_compartmentDoor.setPriority(250);
		_compartmentDoor._flag = true;
		BF_GLOBALS._sceneItems.push_back(&_compartmentDoor);

		_mirandaCard.postInit();
		_mirandaCard.setVisage(60);
		_mirandaCard.setStrip(2);
		_mirandaCard.setFrame(2);
		_mirandaCard.setPosition(Common::Point(280, 160));

		if (BF_INVENTORY.getObjectScene(INV_MIRANDA_CARD) == 60) {
			_mirandaCard.show();
			BF_GLOBALS._sceneItems.push_back(&_mirandaCard);
		} else {
			_mirandaCard.hide();
		}

		_ticketBook.postInit();
		_ticketBook.setVisage(60);
		_ticketBook.setStrip(2);
		_ticketBook.setFrame(1);
		_ticketBook.setPosition(Common::Point(289, 161));

		if (BF_INVENTORY.getObjectScene(INV_TICKET_BOOK) == 60) {
			_ticketBook.show();
			BF_GLOBALS._sceneItems.push_back(&_ticketBook);
		} else {
			_ticketBook.hide();
		}
	}

	_item3._sceneRegionId = 7;
	_radio._sceneRegionId = 12;
	_compartment._sceneRegionId = 14;

	_stripManager.addSpeaker(&_gameTextSpeaker);
	_stripManager.addSpeaker(&_jakeRadioSpeaker);

	if (BF_GLOBALS.getFlag(onDuty) && !BF_GLOBALS.getFlag(fWithLyle)) {
		BF_GLOBALS._sceneItems.push_back(&_radio);
		BF_GLOBALS._sceneItems.push_back(&_compartment);
	}

	BF_GLOBALS._sceneItems.push_back(&_ignition);
	BF_GLOBALS._sceneItems.push_back(&_item3);
	BF_GLOBALS._player.enableControl();
	BF_GLOBALS._events.setCursor(CURSOR_USE);

	switch (BF_GLOBALS._dayNumber) {
	case 1:
		if (BF_GLOBALS.getFlag(onDuty) && (BF_GLOBALS._subFlagBitArr1 & 1) &&
				(BF_GLOBALS._bookmark < bStartOfGame) && (BF_GLOBALS._sceneManager._previousScene != 342)) {
			setAction(&_action2);
			if (BF_GLOBALS._sceneManager._previousScene == 342)
				_newScene = 340;
		}
		break;
	case 2:
		if (BF_GLOBALS.getFlag(onDuty) && ((BF_GLOBALS._subFlagBitArr1 >> 7) & 1) &&
				(BF_GLOBALS._sceneManager._previousScene != 550) &&
				(BF_GLOBALS._bookmark < bInspectionDone)) {
			setAction(&_action3);
		}
	}
}

void Scene60::remove() {
	BF_GLOBALS._player.enableControl();
	BF_GLOBALS._events.setCursor(_cursorId);

	if (_cursorId == CURSOR_EXIT)
		BF_GLOBALS._events.setCursor(CURSOR_USE);
}

void Scene60::signal() {
	++_sceneMode;
	BF_GLOBALS._player.enableControl();
}

void Scene60::dispatch() {
	SceneExt::dispatch();

	int idx = BF_GLOBALS._sceneRegions.indexOf(Common::Point(
		BF_GLOBALS._sceneManager._scene->_sceneBounds.left + BF_GLOBALS._events._mousePos.x,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + BF_GLOBALS._events._mousePos.y));

	if (idx == _item3._sceneRegionId) {
		if (BF_GLOBALS._events.getCursor() != CURSOR_EXIT) {
			_cursorId = BF_GLOBALS._events.getCursor();
			BF_GLOBALS._events.setCursor(CURSOR_EXIT);
		}
	} else {
		if (BF_GLOBALS._events.getCursor() == CURSOR_EXIT) {
			BF_GLOBALS._events.setCursor(_cursorId);
		}
	}
}

} // End of namespace BlueForce

} // End of namespace TsAGE

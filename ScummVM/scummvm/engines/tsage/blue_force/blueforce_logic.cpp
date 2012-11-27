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

#include "common/config-manager.h"
#include "tsage/blue_force/blueforce_logic.h"
#include "tsage/blue_force/blueforce_dialogs.h"
#include "tsage/blue_force/blueforce_scenes0.h"
#include "tsage/blue_force/blueforce_scenes1.h"
#include "tsage/blue_force/blueforce_scenes2.h"
#include "tsage/blue_force/blueforce_scenes3.h"
#include "tsage/blue_force/blueforce_scenes4.h"
#include "tsage/blue_force/blueforce_scenes5.h"
#include "tsage/blue_force/blueforce_scenes6.h"
#include "tsage/blue_force/blueforce_scenes7.h"
#include "tsage/blue_force/blueforce_scenes8.h"
#include "tsage/blue_force/blueforce_scenes9.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/graphics.h"
#include "tsage/staticres.h"

namespace TsAGE {

namespace BlueForce {

void BlueForceGame::start() {
	int slot = -1;

	// Check for a savegame to load straight from the launcher
	if (ConfMan.hasKey("save_slot")) {
		slot = ConfMan.getInt("save_slot");
		Common::String file = g_vm->generateSaveName(slot);
		Common::InSaveFile *in = g_vm->_system->getSavefileManager()->openForLoading(file);
		if (in)
			delete in;
		else
			slot = -1;
	}

	if (slot >= 0)
		// Set the savegame slot to load in the main loop
		g_globals->_sceneHandler->_loadGameSlot = slot;
	else
		// Switch to the title screen
		g_globals->_sceneManager.setNewScene(20);
}

Scene *BlueForceGame::createScene(int sceneNumber) {
	switch (sceneNumber) {
	/* Scene Group #0 */
	case 20:
		// Tsunami Title Screen
		return new Scene20();
	case 50:
		// Map screen
		return new Scene50();
	case 60:
		// Motorcycle
		return new Scene60();
	/* Scene Group #1 */
	case 100:
		// Tsnunami Title Screen #2
		return new Scene100();
	case 109:
		// Introduction - Bar Room
		return new Scene109();
	case 110:
		// Introduction - Outside the bar
		return new Scene110();
	case 114:
		// Outside Tony's Bar
		return new Scene114();
	case 115:
		// Inside Tony's Bar
		return new Scene115();
	case 125:
		// Intro - Chase in the city
		return new Scene125();
	case 140:
		// Intro - Burglar near the House
		return new Scene140();
	case 150:
		// Intro - Burglar inside the house
		return new Scene150();
	case 160:
		// Intro - Burial
		return new Scene160();
	case 180:
		// Front of Home
		return new Scene180();
	case 190:
		// Front of Police Station
		return new Scene190();
	case 200:
		// Credits - Motorcycle Training
		return new Scene200();
	case 210:
		// Credits - Car Training
		return new Scene210();
	case 220:
		// Credits - Martial Arts
		return new Scene220();
	case 225:
		// Credits - Gun Training
		return new Scene225();
	case 265:
		// Intro - Graduation Article
		return new Scene265();
	case 270:
		// Living Room & Kitchen
		return new Scene270();
	case 271:
		// Living Room & Kitchen #2
		return new Scene271();
	case 280:
		// Bedroom Flashback cut-scene
		return new Scene280();
	case 300:
		// Outside Police Station
		return new Scene300();
	case 315:
		// Inside Police Station
		return new Scene315();
	case 325:
		// Police Station Conference Room
		return new Scene325();
	case 330:
		// Approaching Marina
		return new Scene330();
	case 340:
		// Marina, Domestic Disturbance
		return new Scene340();
	case 342:
		// Marina, Normal
		return new Scene342();
	case 350:
		// Marina, Outside Boat
		return new Scene350();
	case 355:
		// Future Wave Exterior
		return new Scene355();
	case 360:
		// Future Wave Interior
		return new Scene360();
	case 370:
		// Future Wave Bedroom
		return new Scene370();
	case 380:
		// Outside City Hall & Jail
		return new Scene380();
	case 385:
		// City Hall
		return new Scene385();
	case 390:
		// City Jail
		return new Scene390();
	case 410:
		// Traffic Stop Gang Members
		return new Scene410();
	case 415:
		// Searching Truck
		return new Scene415();
	case 440:
		// Outside Alleycat Bowl
		return new Scene440();
	case 450:
		// Inside Alleycat Bowl
		return new Scene450();
	case 550:
		// Outside Bikini Hut
		return new Scene550();
	case 551:
		// Outside Bikini Hut (Drunk Stop)
		return new Scene551();
	case 560:
		// Study
		return new Scene560();
	case 570:
		// Computer
		return new Scene570();
	case 580:
		// Child Protective Services Parking Lot
		return new Scene580();
	case 590:
		// Child Protective Services
		return new Scene590();
	case 600:
		// Crash cut-scene
		return new Scene600();
	case 620:
		// Hospital cut-scene
		return new Scene620();
	case 666:
		// Death scene
		return new Scene666();
	case 690:
		// Decking
		return new Scene690();
	case 710:
		// Beach
		return new Scene710();
	case 800:
		// Jamison & Ryan
		return new Scene800();
	case 810:
		// Lyle's Office
		return new Scene810();
	case 820:
		// Microfiche Reader
		return new Scene820();
	case 830:
		// Outside Boat Rentals
		return new Scene830();
	case 840:
		// Boat Rentals
		return new Scene840();
	case 850:
		// Boat Leaving/Entering Marina
		return new Scene850();
	case 860:
		// Boat Entering Cove
		return new Scene860();
	case 870:
		// Cove Beach
		return new Scene870();
	case 880:
		// Beach Path
		return new Scene880();
	case 900:
		// Outside the Warehouse
		return new Scene900();
	case 910:
		// Inside the Warehouse
		return new Scene910();
	case 920:
		// Inside the Warehouse: Secret room
		return new Scene920();
	case 930:
		// Inside the caravan
		return new Scene930();
	case 935:
		// Hidden in the wardrobe
		return new Scene935();
	case 940:
		// Jail ending animation
		return new Scene940();
	default:
		error("Unknown scene number - %d", sceneNumber);
		break;
	}
}

/**
 * Returns true if it is currently okay to restore a game
 */
bool BlueForceGame::canLoadGameStateCurrently() {
	// Don't allow a game to be loaded if a dialog is active
	return g_globals->_gfxManagers.size() == 1;
}

/**
 * Returns true if it is currently okay to save the game
 */
bool BlueForceGame::canSaveGameStateCurrently() {
	// Don't allow a game to be saved if a dialog is active
	return g_globals->_gfxManagers.size() == 1;
}

void BlueForceGame::rightClick() {
	RightClickDialog *dlg = new RightClickDialog();
	dlg->execute();
	delete dlg;
}

void BlueForceGame::processEvent(Event &event) {
	if (event.eventType == EVENT_KEYPRESS) {
		switch (event.kbd.keycode) {
		case Common::KEYCODE_F1:
			// F1 - Help
			MessageDialog::show(HELP_MSG, OK_BTN_STRING);
			break;

		case Common::KEYCODE_F2:
			// F2 - Sound Options
			SoundDialog::execute();
			break;

		case Common::KEYCODE_F3:
			// F3 - Quit
			quitGame();
			event.handled = false;
			break;

		case Common::KEYCODE_F4:
			// F4 - Restart
			restartGame();
			g_globals->_events.setCursorFromFlag();
			break;

		case Common::KEYCODE_F7:
			// F7 - Restore
			restoreGame();
			g_globals->_events.setCursorFromFlag();
			break;

		case Common::KEYCODE_F10:
			// F10 - Pause
			GfxDialog::setPalette();
			MessageDialog::show(GAME_PAUSED_MSG, OK_BTN_STRING);
			g_globals->_events.setCursorFromFlag();
			break;

		default:
			break;
		}
	}
}

void BlueForceGame::restart() {
	g_globals->_scenePalette.clearListeners();
	g_globals->_soundHandler.stop();

	// Reset the globals
	g_globals->reset();

	// Clear save/load slots
	g_globals->_sceneHandler->_saveGameSlot = -1;
	g_globals->_sceneHandler->_loadGameSlot = -1;

	g_globals->_stripNum = 0;
	g_globals->_events.setCursor(CURSOR_WALK);

	// Change to the first game scene
	g_globals->_sceneManager.changeScene(190);
}

/*--------------------------------------------------------------------------*/

AObjectArray::AObjectArray(): EventHandler() {
	_inUse = false;
	clear();
}

void AObjectArray::clear() {
	for (int i = 0; i < OBJ_ARRAY_SIZE; ++i)
		_objList[i] = NULL;
}

void AObjectArray::synchronize(Serializer &s) {
	EventHandler::synchronize(s);
	for (int i = 0; i < OBJ_ARRAY_SIZE; ++i)
		SYNC_POINTER(_objList[i]);
}

void AObjectArray::process(Event &event) {
	if (_inUse)
		error("Array error");
	_inUse = true;

	for (int i = 0; i < OBJ_ARRAY_SIZE; ++i) {
		if (_objList[i])
			_objList[i]->process(event);
	}

	_inUse = false;
}

void AObjectArray::dispatch() {
	if (_inUse)
		error("Array error");
	_inUse = true;

	for (int i = 0; i < OBJ_ARRAY_SIZE; ++i) {
		if (_objList[i])
			_objList[i]->dispatch();
	}

	_inUse = false;
}

int AObjectArray::getNewIndex() {
	for (int i = 0; i < OBJ_ARRAY_SIZE; ++i) {
		if (!_objList[i])
			return i;
	}
	error("AObjectArray too full.");
}

void AObjectArray::add(EventHandler *obj) {
	int idx = getNewIndex();
	_objList[idx] = obj;
}

void AObjectArray::remove(EventHandler *obj) {
	for (int i = 0; i < OBJ_ARRAY_SIZE; ++i) {
		if (_objList[i] == obj) {
			_objList[i] = NULL;
			return;
		}
	}
}

/*--------------------------------------------------------------------------*/

Timer::Timer() {
	_endFrame = 0;
	_endHandler = NULL;
	_tickAction = NULL;
}

void Timer::remove() {
	_endFrame = 0;
	_endHandler = NULL;

	((Scene100 *)BF_GLOBALS._sceneManager._scene)->removeTimer(this);
}

void Timer::synchronize(Serializer &s) {
	EventHandler::synchronize(s);
	SYNC_POINTER(_tickAction);
	SYNC_POINTER(_endHandler);
	s.syncAsUint32LE(_endFrame);
}

void Timer::signal() {
	assert(_endHandler);
	EventHandler *item = _endHandler;
	remove();
	item->signal();
}

void Timer::dispatch() {
	if (_tickAction)
		_tickAction->dispatch();

	if (_endFrame) {
		uint32 frameNumber = BF_GLOBALS._events.getFrameNumber();
		if (frameNumber > _endFrame)
			// Timer has expired
			signal();
	}
}

void Timer::set(uint32 delay, EventHandler *endHandler) {
	assert(delay != 0);

	_endFrame = BF_GLOBALS._sceneHandler->getFrameDifference() + delay;
	_endHandler = endHandler;

	((SceneExt *)BF_GLOBALS._sceneManager._scene)->addTimer(this);
}

/*--------------------------------------------------------------------------*/

TimerExt::TimerExt(): Timer() {
	_action = NULL;
}

void TimerExt::set(uint32 delay, EventHandler *endHandler, Action *newAction) {
	_newAction = newAction;
	Timer::set(delay, endHandler);
}

void TimerExt::synchronize(Serializer &s) {
	EventHandler::synchronize(s);
	SYNC_POINTER(_action);
}

void TimerExt::remove() {
	_action = NULL;
	Timer::remove();
}

void TimerExt::signal() {
	EventHandler *endHandler = _endHandler;
	Action *newAction = _newAction;
	remove();

	// If the end action doesn't have an action anymore, set it to the specified new action
	assert(endHandler);
	if (!endHandler->_action)
		endHandler->setAction(newAction);
}

/*--------------------------------------------------------------------------*/

void SceneItemType2::startMove(SceneObject *sceneObj, va_list va) {
}

/*--------------------------------------------------------------------------*/

void NamedObject::postInit(SceneObjectList *OwnerList) {
	_lookLineNum = _talkLineNum = _useLineNum = -1;
	SceneObject::postInit();
}

void NamedObject::synchronize(Serializer &s) {
	SceneObject::synchronize(s);
	s.syncAsSint16LE(_resNum);
	s.syncAsSint16LE(_lookLineNum);
	s.syncAsSint16LE(_talkLineNum);
	s.syncAsSint16LE(_useLineNum);
}

bool NamedObject::startAction(CursorType action, Event &event) {
	bool handled = true;

	switch (action) {
	case CURSOR_LOOK:
		if (_lookLineNum == -1)
			handled = false;
		else
			SceneItem::display2(_resNum, _lookLineNum);
		break;
	case CURSOR_USE:
		if (_useLineNum == -1)
			handled = false;
		else
			SceneItem::display2(_resNum, _useLineNum);
		break;
	case CURSOR_TALK:
		if (_talkLineNum == -1)
			handled = false;
		else
			SceneItem::display2(_resNum, _talkLineNum);
		break;
	default:
		handled = false;
		break;
	}

	if (!handled)
		handled = ((SceneExt *)BF_GLOBALS._sceneManager._scene)->display(action);
	return handled;
}

/*--------------------------------------------------------------------------*/

CountdownObject::CountdownObject(): NamedObject() {
	_countDown = 0;
}

void CountdownObject::synchronize(Serializer &s) {
	SceneObject::synchronize(s);
	s.syncAsSint16LE(_countDown);
}

void CountdownObject::dispatch() {
	int frameNum = _frame;
	SceneObject::dispatch();

	if ((frameNum != _frame) && (_countDown > 0)) {
		if (--_countDown == 0) {
			animate(ANIM_MODE_NONE, 0);
			_frame = 1;
		}
	}
}

void CountdownObject::fixCountdown(int mode, ...) {
	if (mode == 8) {
		va_list va;
		va_start(va, mode);

		_countDown = va_arg(va, int);
		animate(ANIM_MODE_8, _countDown, NULL);
		va_end(va);
	}
}

/*--------------------------------------------------------------------------*/

FollowerObject::FollowerObject(): NamedObject() {
	_object = NULL;
}

void FollowerObject::synchronize(Serializer &s) {
	NamedObject::synchronize(s);
	SYNC_POINTER(_object);
}

void FollowerObject::remove() {
	NamedObject::remove();
	_object = NULL;
}

void FollowerObject::dispatch() {
	SceneObject::dispatch();
	assert(_object);

	if ((_object->_flags & OBJFLAG_HIDE) || ((_object->_visage != 307) &&
		((_object->_visage != 308) || (_object->_strip != 1)))) {
		hide();
	} else if ((_object->_visage != 308) || (_object->_strip != 1)) {
		show();
		setStrip(_object->_strip);
		setPosition(Common::Point(_object->_position.x + 1, _object->_position.y), _yDiff);
	}
}

void FollowerObject::reposition() {
	assert(_object);
	setStrip(_object->_strip);
	setPosition(_object->_position, _yDiff);
	NamedObject::reposition();
}

void FollowerObject::setup(SceneObject *object, int visage, int frameNum, int yDiff) {
	SceneObject::postInit();
	_object = object;
	_yDiff = yDiff;
	setVisage(visage);
	setFrame(frameNum);

	dispatch();
}

/*--------------------------------------------------------------------------*/

FocusObject::FocusObject(): NamedObject() {
	_img = surfaceFromRes(1, 5, 7);
}

void FocusObject::postInit(SceneObjectList *OwnerList) {
	NamedObject::postInit(OwnerList);
	_resNum = 560;
	_lookLineNum = 43;
	_talkLineNum = 44;
	_useLineNum = -1;
	_v90 = 0;
	_v92 = 1;

	SceneExt *scene = (SceneExt *)BF_GLOBALS._sceneManager._scene;
	scene->_focusObject = this;
	BF_GLOBALS._sceneItems.push_front(this);
}

void FocusObject::synchronize(Serializer &s) {
	NamedObject::synchronize(s);
	s.syncAsSint16LE(_v90);
	s.syncAsSint16LE(_v92);
}

void FocusObject::remove() {
	BF_GLOBALS._sceneItems.remove(this);

	SceneExt *scene = (SceneExt *)BF_GLOBALS._sceneManager._scene;
	if (scene->_focusObject == this)
		scene->_focusObject = NULL;

	BF_GLOBALS._events.setCursor(BF_GLOBALS._events.getCursor());
	NamedObject::remove();
}

void FocusObject::process(Event &event) {
	if (BF_GLOBALS._player._enabled) {
		if (_bounds.contains(event.mousePos)) {
			// Reset the cursor back to normal
			BF_GLOBALS._events.setCursor(BF_GLOBALS._events.getCursor());

			if ((event.eventType == EVENT_BUTTON_DOWN) && (BF_GLOBALS._events.getCursor() == CURSOR_WALK) &&
					(event.btnState == 3)) {
				BF_GLOBALS._events.setCursor(CURSOR_USE);
				event.handled = true;
			}
		} else if (event.mousePos.y < 168) {
			// Change the cursor to an 'Exit' image
			BF_GLOBALS._events.setCursor(_img);
			if (event.eventType == EVENT_BUTTON_DOWN) {
				// Remove the object from display
				event.handled = true;
				remove();
			}
		}
	}

	if (_action)
		_action->process(event);
}

/*--------------------------------------------------------------------------*/

SceneExt::SceneExt(): Scene() {
	_stripManager._onBegin = SceneExt::startStrip;
	_stripManager._onEnd = SceneExt::endStrip;

	_field372 = _field37A = 0;
	_savedPlayerEnabled = false;
	_savedUiEnabled = false;
	_savedCanWalk = false;
	_focusObject = NULL;
	_cursorVisage.setVisage(1, 8);
}

void SceneExt::postInit(SceneObjectList *OwnerList) {
	Scene::postInit(OwnerList);
	if (BF_GLOBALS._dayNumber) {
		// Blank out the bottom portion of the screen
		BF_GLOBALS._interfaceY = UI_INTERFACE_Y;

		Rect r(0, UI_INTERFACE_Y, SCREEN_WIDTH, SCREEN_HEIGHT);
		BF_GLOBALS.gfxManager().getSurface().fillRect(r, 0);
	}
}

void SceneExt::remove() {
	T2_GLOBALS._uiElements.hide();
	T2_GLOBALS._uiElements.resetClear();

	// Below code originally in Blue Force Scene::remove(). Placed here to avoid contaminating
	// core class with Blue Force specific code
	if (_action) {
		if (_action->_endHandler)
			_action->_endHandler = NULL;
		_action->remove();
	}

	_focusObject = NULL;
}

void SceneExt::process(Event &event) {
	_objArray2.process(event);
	if (!event.handled)
		Scene::process(event);
}

void SceneExt::dispatch() {
	_timerList.dispatch();

	if (_field37A) {
		if ((--_field37A == 0) && BF_GLOBALS._dayNumber) {
			if (T2_GLOBALS._uiElements._active && BF_GLOBALS._player._enabled) {
				T2_GLOBALS._uiElements.show();
			}

			_field37A = 0;
		}
	}

	Scene::dispatch();
}

void SceneExt::loadScene(int sceneNum) {
	Scene::loadScene(sceneNum);

	_v51C34.top = 0;
	_v51C34.bottom = 300;
	BF_GLOBALS._sceneHandler->_delayTicks = 1;
}

void SceneExt::checkGun() {
	// Remove a bullet from the currently loaded clip
	if (BF_GLOBALS.getFlag(fLoadedSpare) && (BF_GLOBALS._clip2Bullets > 0)) {
		if (--BF_GLOBALS._clip2Bullets == 0)
			BF_GLOBALS.clearFlag(fGunLoaded);
	} else {
		if (BF_GLOBALS._clip1Bullets > 0)
			--BF_GLOBALS._clip1Bullets;

		if (!BF_GLOBALS._clip1Bullets)
			BF_GLOBALS.clearFlag(fGunLoaded);
	}

	BF_GLOBALS._sound3.play(4);
}

bool SceneExt::display(CursorType action) {
	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(9000, BF_GLOBALS._randomSource.getRandomNumber(2));
		break;
	case CURSOR_USE:
		SceneItem::display2(9000, BF_GLOBALS._randomSource.getRandomNumber(2) + 6);
		break;
	case CURSOR_TALK:
		SceneItem::display2(9000, BF_GLOBALS._randomSource.getRandomNumber(2) + 3);
		break;
	case INV_COLT45:
		gunDisplay();
		break;
	default:
		if (action < BF_LAST_INVENT)
			SceneItem::display2(9002, (int)action);
		else
			return false;
		break;
	}

	return true;
}

void SceneExt::fadeOut() {
	uint32 black = 0;
	BF_GLOBALS._scenePalette.fade((const byte *)&black, false, 100);
}

void SceneExt::gunDisplay() {
	if (!BF_GLOBALS.getFlag(gunDrawn)) {
		// Gun not drawn
		SceneItem::display2(1, BF_GLOBALS.getFlag(fCanDrawGun) ? 0 : 4);
	} else if (!BF_GLOBALS.getFlag(fGunLoaded)) {
		// Gun not loaded
		SceneItem::display2(1, 1);
	} else if (!BF_GLOBALS.getHasBullets()) {
		// Out of ammunition
		SceneItem::display2(1, 2);
	} else {
		// Check scene for whether gun can fire
		checkGun();
	}
}

void SceneExt::startStrip() {
	SceneExt *scene = (SceneExt *)BF_GLOBALS._sceneManager._scene;
	scene->_field372 = 1;
	scene->_savedPlayerEnabled = BF_GLOBALS._player._enabled;

	if (scene->_savedPlayerEnabled) {
		scene->_savedUiEnabled = BF_GLOBALS._player._uiEnabled;
		scene->_savedCanWalk = BF_GLOBALS._player._canWalk;
		BF_GLOBALS._player.disableControl();

		if (!BF_GLOBALS._v50696 && T2_GLOBALS._uiElements._active)
			T2_GLOBALS._uiElements.hide();
	}
}

void SceneExt::endStrip() {
	SceneExt *scene = (SceneExt *)BF_GLOBALS._sceneManager._scene;
	scene->_field372 = 0;

	if (scene->_savedPlayerEnabled) {
		BF_GLOBALS._player.enableControl();
		BF_GLOBALS._player._uiEnabled = scene->_savedUiEnabled;
		BF_GLOBALS._player._canWalk = scene->_savedCanWalk;

		if (!BF_GLOBALS._v50696 && T2_GLOBALS._uiElements._active)
			T2_GLOBALS._uiElements.show();
	}
}

void SceneExt::clearScreen() {
	BF_GLOBALS._screenSurface.fillRect(BF_GLOBALS._screenSurface.getBounds(), 0);
}

/*--------------------------------------------------------------------------*/

PalettedScene::PalettedScene(): SceneExt() {
	_field794 = 0;
}

void PalettedScene::synchronize(Serializer &s) {
	SceneExt::synchronize(s);
	s.syncAsSint16LE(_field794);
}

void PalettedScene::postInit(SceneObjectList *OwnerList) {
	_field794 = 0;
	_palette._field412 = 1;
	SceneExt::postInit(OwnerList);
}

void PalettedScene::remove() {
	SceneExt::remove();
	if (_field794 == 1) {
		for (SynchronizedList<SceneObject *>::iterator i = BF_GLOBALS._sceneObjects->begin();
				i != BF_GLOBALS._sceneObjects->end(); ++i)
			(*i)->remove();

		BF_GLOBALS._sceneObjects->draw();
		BF_GLOBALS._scenePalette.loadPalette(2);
		BF_GLOBALS._v51C44 = 1;
		BF_GLOBALS._sceneManager._hasPalette = true;
	}

	BF_GLOBALS._scenePalette._field412 = 0;
}

PaletteFader *PalettedScene::addFader(const byte *arrBufferRGB, int step, Action *action) {
	_field794 = 1;
	return BF_GLOBALS._scenePalette.addFader(arrBufferRGB, 1, step, action);
}

void PalettedScene::add2Faders(const byte *arrBufferRGB, int step, int paletteNum, Action *action) {
	BF_GLOBALS._scenePalette.addFader(arrBufferRGB, 1, 100, NULL);
	_palette.loadPalette(paletteNum);
	_palette.loadPalette(2);
	BF_GLOBALS._scenePalette.addFader(_palette._palette, 256, step, action);
}

void PalettedScene::transition(const byte *arrBufferRGB, int percent, int paletteNum, Action *action, int fromColor1, int fromColor2, int toColor1, int toColor2, bool flag) {
	byte tmpPalette[768];

	_palette.loadPalette(paletteNum);
	_palette.loadPalette(2);
	if (flag) {
		for (int i = fromColor1; i <= fromColor2; i++) {
			tmpPalette[(3 * i)]     = BF_GLOBALS._scenePalette._palette[(3 * i)];
			tmpPalette[(3 * i) + 1] = BF_GLOBALS._scenePalette._palette[(3 * i) + 1];
			tmpPalette[(3 * i) + 2] = BF_GLOBALS._scenePalette._palette[(3 * i) + 2];
		}
	} else {
		for (int i = fromColor1; i <= fromColor2; i++) {
			tmpPalette[(3 * i)]     = _palette._palette[(3 * i)];
			tmpPalette[(3 * i) + 1] = _palette._palette[(3 * i) + 1];
			tmpPalette[(3 * i) + 2] = _palette._palette[(3 * i) + 2];
		}
	}

	for (int i = toColor1; i <= toColor2; i++) {
		tmpPalette[i * 3] = _palette._palette[i * 3] - ((_palette._palette[i * 3] - arrBufferRGB[i * 3]) * (100 - percent)) / 100;
		tmpPalette[i * 3 + 1] = _palette._palette[i * 3 + 1] - ((_palette._palette[i * 3 + 1] - arrBufferRGB[i * 3 + 1]) * (100 - percent)) / 100;
		tmpPalette[i * 3 + 2] = _palette._palette[i * 3 + 2] - ((_palette._palette[i * 3 + 2] - arrBufferRGB[i * 3 + 2]) * (100 - percent)) / 100;
	}

	BF_GLOBALS._scenePalette.addFader((const byte *)tmpPalette, 256, 100, action);
}


/*--------------------------------------------------------------------------*/

void SceneHandlerExt::postInit(SceneObjectList *OwnerList) {
	SceneHandler::postInit(OwnerList);

	// Load the low end palette data
	BF_GLOBALS._scenePalette.loadPalette(2);
	BF_GLOBALS._scenePalette.refresh();
}

void SceneHandlerExt::process(Event &event) {
	SceneExt *scene = (SceneExt *)BF_GLOBALS._sceneManager._scene;
	if (scene && scene->_focusObject)
		scene->_focusObject->process(event);

	if (T2_GLOBALS._uiElements._active) {
		T2_GLOBALS._uiElements.process(event);
		if (event.handled)
			return;
	}

	// If the strip proxy is currently being controlled by the strip manager,
	// then pass all events to it first
	if (BF_GLOBALS._stripProxy._action) {
		BF_GLOBALS._stripProxy._action->process(event);
		if (event.handled)
			return;
	}

	// If the user clicks the button whilst the introduction is active, prompt for playing the game
	if ((BF_GLOBALS._dayNumber == 0) && (event.eventType == EVENT_BUTTON_DOWN)) {
		// Prompt user for whether to start play or watch introduction
		BF_GLOBALS._player.enableControl();
		BF_GLOBALS._events.setCursor(CURSOR_WALK);

		if (MessageDialog::show2(WATCH_INTRO_MSG, START_PLAY_BTN_STRING, INTRODUCTION_BTN_STRING) == 0) {
			// Start the game
			BF_GLOBALS._dayNumber = 1;
			BF_GLOBALS._sceneManager.changeScene(190);
		} else {
			BF_GLOBALS._player.disableControl();
		}

		event.handled = true;
	}

	SceneHandler::process(event);
}

void SceneHandlerExt::playerAction(Event &event) {
	if (BF_GLOBALS._events.getCursor() == INV_DOG_WHISTLE) {
		SceneItem::display2(1, 6);
		event.handled = true;
	}
}

void SceneHandlerExt::processEnd(Event &event) {
	// Check for a fallback text display for the given cursor/item being used in the scene
	if (!event.handled && BF_GLOBALS._sceneManager._scene) {
		CursorType cursor = BF_GLOBALS._events.getCursor();
		if (((SceneExt *)BF_GLOBALS._sceneManager._scene)->display(cursor))
			event.handled = true;
	}
}

/*--------------------------------------------------------------------------*/

BlueForceInvObjectList::BlueForceInvObjectList():
		_none(9, 5, 1),
		_colt45(9, 1, 1),
		_ammoClip(9, 4, 2),
		_spareClip(9, 4, 3),
		_handcuffs(9, 1, 4),
		_greensGun(9, 1, 5),
		_ticketBook(9, 1, 6),
		_mirandaCard(9, 1, 7),
		_forestRap(9, 1, 8),
		_greenId(9, 1, 9),
		_baseballCard(9, 1, 10),
		_bookingGreen(9, 1, 11),
		_flare(9, 1, 12),
		_cobbRap(9, 1, 13),
		_bullet22(9, 1, 14),
		_autoRifle(9, 1, 15),
		_wig(9, 1, 16),
		_frankieId(9, 1, 17),
		_tyroneId(9, 1, 18),
		_snub22(9, 1, 19),
		_bug(1, 1, 1),
		_bookingFrankie(9, 2, 1),
		_bookingGang(9, 2, 2),
		_fbiTeletype(9, 2, 3),
		_daNote(9, 2, 4),
		_printOut(9, 2, 5),
		_warehouseKeys(9, 2, 6),
		_centerPunch(9, 2, 7),
		_tranqGun(9, 2, 8),
		_hook(9, 2, 9),
		_rags(9, 2, 10),
		_jar(9, 2, 11),
		_screwdriver(9, 2, 12),
		_dFloppy(9, 2, 13),
		_blankDisk(9, 2, 14),
		_stick(9, 2, 15),
		_crate1(9, 2, 16),
		_crate2(9, 2, 17),
		_shoebox(9, 2, 18),
		_badge(9, 2, 19),
		_bug2(1, 1, 1),
		_rentalCoupon(9, 3, 1),
		_nickel(9, 3, 2),
		_lyleCard(9, 3, 3),
		_carterNote(9, 3, 4),
		_mugshot(9, 3, 5),
		_clipping(9, 3, 6),
		_microfilm(9, 3, 7),
		_waveKeys(9, 3, 8),
		_rentalKeys(9, 3, 9),
		_napkin(9, 3, 10),
		_dmvPrintout(9, 3, 11),
		_fishingNet(9, 3, 12),
		_id(9, 3, 13),
		_bullets9mm(9, 3, 14),
		_schedule(9, 3, 15),
		_grenades(9, 3, 16),
		_yellowCord(9, 3, 17),
		_halfYellowCord(9, 3, 18),
		_blackCord(9, 3, 19),
		_bug3(1, 1, 1),
		_halfBlackCord(9, 4, 1),
		_warrant(9, 4, 2),
		_jacket(9, 4, 3),
		_greensKnife(9, 4, 4),
		_dogWhistle(9, 4, 5),
		_ammoBelt(9, 1, 2),
		_alleyCatKey(9, 4, 7) {

	// Add the items to the list
	_itemList.push_back(&_none);
	_itemList.push_back(&_colt45);
	_itemList.push_back(&_ammoClip);
	_itemList.push_back(&_spareClip);
	_itemList.push_back(&_handcuffs);
	_itemList.push_back(&_greensGun);
	_itemList.push_back(&_ticketBook);
	_itemList.push_back(&_mirandaCard);
	_itemList.push_back(&_forestRap);
	_itemList.push_back(&_greenId);
	_itemList.push_back(&_baseballCard);
	_itemList.push_back(&_bookingGreen);
	_itemList.push_back(&_flare);
	_itemList.push_back(&_cobbRap);
	_itemList.push_back(&_bullet22);
	_itemList.push_back(&_autoRifle);
	_itemList.push_back(&_wig);
	_itemList.push_back(&_frankieId);
	_itemList.push_back(&_tyroneId);
	_itemList.push_back(&_snub22);
	_itemList.push_back(&_bug);
	_itemList.push_back(&_bookingFrankie);
	_itemList.push_back(&_bookingGang);
	_itemList.push_back(&_fbiTeletype);
	_itemList.push_back(&_daNote);
	_itemList.push_back(&_printOut);
	_itemList.push_back(&_warehouseKeys);
	_itemList.push_back(&_centerPunch);
	_itemList.push_back(&_tranqGun);
	_itemList.push_back(&_hook);
	_itemList.push_back(&_rags);
	_itemList.push_back(&_jar);
	_itemList.push_back(&_screwdriver);
	_itemList.push_back(&_dFloppy);
	_itemList.push_back(&_blankDisk);
	_itemList.push_back(&_stick);
	_itemList.push_back(&_crate1);
	_itemList.push_back(&_crate2);
	_itemList.push_back(&_shoebox);
	_itemList.push_back(&_badge);
	_itemList.push_back(&_bug2);
	_itemList.push_back(&_rentalCoupon);
	_itemList.push_back(&_nickel);
	_itemList.push_back(&_lyleCard);
	_itemList.push_back(&_carterNote);
	_itemList.push_back(&_mugshot);
	_itemList.push_back(&_clipping);
	_itemList.push_back(&_microfilm);
	_itemList.push_back(&_waveKeys);
	_itemList.push_back(&_rentalKeys);
	_itemList.push_back(&_napkin);
	_itemList.push_back(&_dmvPrintout);
	_itemList.push_back(&_fishingNet);
	_itemList.push_back(&_id);
	_itemList.push_back(&_bullets9mm);
	_itemList.push_back(&_schedule);
	_itemList.push_back(&_grenades);
	_itemList.push_back(&_yellowCord);
	_itemList.push_back(&_halfYellowCord);
	_itemList.push_back(&_blackCord);
	_itemList.push_back(&_bug3);
	_itemList.push_back(&_halfBlackCord);
	_itemList.push_back(&_warrant);
	_itemList.push_back(&_jacket);
	_itemList.push_back(&_greensKnife);
	_itemList.push_back(&_dogWhistle);
	_itemList.push_back(&_ammoBelt);
	_itemList.push_back(&_alleyCatKey);
}

void BlueForceInvObjectList::reset() {
	// Reset all object scene numbers
	SynchronizedList<InvObject *>::iterator i;
	for (i = _itemList.begin(); i != _itemList.end(); ++i) {
		(*i)->_sceneNumber = 0;
	}

	// Set up default inventory
	setObjectScene(INV_COLT45, 1);
	setObjectScene(INV_HANDCUFFS, 1);
	setObjectScene(INV_AMMO_BELT, 1);
	setObjectScene(INV_ID, 1);

	// Set default room for other objects
	setObjectScene(INV_TICKET_BOOK, 60);
	setObjectScene(INV_MIRANDA_CARD, 60);
	setObjectScene(INV_FOREST_RAP, 320);
	setObjectScene(INV_GREEN_ID, 370);
	setObjectScene(INV_BASEBALL_CARD, 840);
	setObjectScene(INV_BOOKING_GREEN, 390);
	setObjectScene(INV_FLARE, 355);
	setObjectScene(INV_COBB_RAP, 810);
	setObjectScene(INV_22_BULLET, 415);
	setObjectScene(INV_AUTO_RIFLE, 415);
	setObjectScene(INV_WIG, 415);
	setObjectScene(INV_FRANKIE_ID, 410);
	setObjectScene(INV_TYRONE_ID, 410);
	setObjectScene(INV_22_SNUB, 410);
	setObjectScene(INV_FBI_TELETYPE, 320);
	setObjectScene(INV_DA_NOTE, 320);
	setObjectScene(INV_PRINT_OUT, 570);
	setObjectScene(INV_WAREHOUSE_KEYS, 360);
	setObjectScene(INV_CENTER_PUNCH, 0);
	setObjectScene(INV_TRANQ_GUN, 830);
	setObjectScene(INV_HOOK, 350);
	setObjectScene(INV_RAGS, 870);
	setObjectScene(INV_JAR, 870);
	setObjectScene(INV_SCREWDRIVER, 355);
	setObjectScene(INV_D_FLOPPY, 570);
	setObjectScene(INV_BLANK_DISK, 560);
	setObjectScene(INV_STICK, 710);
	setObjectScene(INV_CRATE1, 710);
	setObjectScene(INV_CRATE2, 870);
	setObjectScene(INV_SHOEBOX, 270);
	setObjectScene(INV_BADGE, 560);
	setObjectScene(INV_RENTAL_COUPON, 0);
	setObjectScene(INV_NICKEL, 560);
	setObjectScene(INV_LYLE_CARD, 270);
	setObjectScene(INV_CARTER_NOTE, 830);
	setObjectScene(INV_MUG_SHOT, 810);
	setObjectScene(INV_CLIPPING, 810);
	setObjectScene(INV_MICROFILM, 810);
	setObjectScene(INV_WAVE_KEYS, 840);
	setObjectScene(INV_RENTAL_KEYS, 840);
	setObjectScene(INV_NAPKIN, 115);
	setObjectScene(INV_DMV_PRINTOUT, 810);
	setObjectScene(INV_FISHING_NET, 830);
	setObjectScene(INV_9MM_BULLETS, 930);
	setObjectScene(INV_SCHEDULE, 930);
	setObjectScene(INV_GRENADES, 355);
	setObjectScene(INV_GREENS_KNIFE, 370);
	setObjectScene(INV_JACKET, 880);
	setObjectScene(INV_DOG_WHISTLE, 880);
	setObjectScene(INV_YELLOW_CORD, 910);
	setObjectScene(INV_BLACK_CORD, 910);

	// Set up the select item handler method
	T2_GLOBALS._onSelectItem = SelectItem;
}

void BlueForceInvObjectList::setObjectScene(int objectNum, int sceneNumber) {
	// Find the appropriate object
	int num = objectNum;
	SynchronizedList<InvObject *>::iterator i = _itemList.begin();
	while (num-- > 0) ++i;
	(*i)->_sceneNumber = sceneNumber;

	// If the item is the currently active one, default back to the use cursor
	if (BF_GLOBALS._events.getCursor() == objectNum)
		BF_GLOBALS._events.setCursor(CURSOR_USE);

	// Update the user interface if necessary
	T2_GLOBALS._uiElements.updateInventory();
}

void BlueForceInvObjectList::alterInventory(int mode) {
	// Check for existing specific items in player's inventory
	bool hasPrintout = getObjectScene(INV_PRINT_OUT) == 1;
	bool hasRags = getObjectScene(INV_RAGS) == 1;
	bool hasJar = getObjectScene(INV_JAR) == 1;
	bool hasNickel = getObjectScene(INV_NICKEL) == 1;
	bool hasCrate1 = getObjectScene(INV_CRATE1) == 1;	//di
	bool hasForestRap = getObjectScene(INV_FOREST_RAP) == 1;
	bool hasRentalCoupon = getObjectScene(INV_RENTAL_COUPON) == 1;	//si
	bool hasWarehouseKeys = getObjectScene(INV_WAREHOUSE_KEYS) == 1;
	bool hasCobbRap = getObjectScene(INV_COBB_RAP) == 1;
	bool hasHook = getObjectScene(INV_HOOK) == 1;
	bool hasMugShot = getObjectScene(INV_MUG_SHOT) == 1;

	// Remove any items currently in player's inventory
	SynchronizedList<InvObject *>::iterator i;
	for (i = _itemList.begin(); i != _itemList.end(); ++i) {
		if ((*i)->_sceneNumber == 1)
			(*i)->_sceneNumber = 0;
	}

	// Give basic set of items back into inventory
	setObjectScene(INV_COLT45, 1);
	setObjectScene(INV_HANDCUFFS, 1);
	setObjectScene(INV_AMMO_BELT, 1);
	setObjectScene(INV_ID, 1);

	// Reset ticket book and miranda card back to motorcycle
	setObjectScene(INV_TICKET_BOOK, 60);
	setObjectScene(INV_MIRANDA_CARD, 60);

	BF_GLOBALS._v4CEC4 = 0;

	switch (mode) {
	case 2:
		if (hasPrintout)
			setObjectScene(INV_PRINT_OUT, 1);
		if (hasNickel)
			setObjectScene(INV_NICKEL, 1);
		if (hasForestRap)
			setObjectScene(INV_FOREST_RAP, 1);
		if (hasCrate1)
			setObjectScene(INV_CRATE1, 1);
		if (hasRentalCoupon)
			setObjectScene(INV_RENTAL_COUPON, 1);
		if (hasHook)
			setObjectScene(INV_HOOK, 1);
		break;
	case 3:
		if (hasPrintout)
			setObjectScene(INV_PRINT_OUT, 1);
		if (hasNickel)
			setObjectScene(INV_NICKEL, 1);
		if (hasForestRap)
			setObjectScene(INV_FOREST_RAP, 1);
		if (hasCrate1)
			setObjectScene(INV_CRATE1, 1);
		if (hasRentalCoupon)
			setObjectScene(INV_RENTAL_COUPON, 1);
		if (hasCobbRap)
			setObjectScene(INV_COBB_RAP, 1);
		if (hasHook)
			setObjectScene(INV_HOOK, 1);
		if (hasMugShot)
			setObjectScene(INV_MUG_SHOT, 1);
		break;
	case 4:
		if (hasNickel)
			setObjectScene(INV_NICKEL, 1);
		if (hasRentalCoupon)
			setObjectScene(INV_RENTAL_COUPON, 1);
		if (hasHook)
			setObjectScene(INV_HOOK, 1);
		break;
	case 5:
		if (hasRags)
			setObjectScene(INV_RAGS, 1);
		if (hasJar)
			setObjectScene(INV_JAR, 1);
		if (hasRentalCoupon)
			setObjectScene(INV_RENTAL_COUPON, 1);
		if (hasWarehouseKeys)
			setObjectScene(INV_WAREHOUSE_KEYS, 1);
		break;
	default:
		break;
	}
}

/**
 * When an inventory item is selected, check if it's the gun belt, since that has a specific dialog
 */
bool BlueForceInvObjectList::SelectItem(int objectNumber) {
	if (objectNumber == INV_AMMO_BELT) {
		AmmoBeltDialog *dlg = new AmmoBeltDialog();
		dlg->execute();
		delete dlg;

		return true;
	}

	return false;
}

/*--------------------------------------------------------------------------*/

NamedHotspot::NamedHotspot() : SceneHotspot() {
	_resNum = 0;
	_lookLineNum = _useLineNum = _talkLineNum = -1;
}

bool NamedHotspot::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_WALK:
		// Nothing
		return false;
	case CURSOR_LOOK:
		if (_lookLineNum == -1)
			return SceneHotspot::startAction(action, event);

		SceneItem::display2(_resNum, _lookLineNum);
		return true;
	case CURSOR_USE:
		if (_useLineNum == -1)
			return SceneHotspot::startAction(action, event);

		SceneItem::display2(_resNum, _useLineNum);
		return true;
	case CURSOR_TALK:
		if (_talkLineNum == -1)
			return SceneHotspot::startAction(action, event);

		SceneItem::display2(_resNum, _talkLineNum);
		return true;
	default:
		return SceneHotspot::startAction(action, event);
	}
}

void NamedHotspot::synchronize(Serializer &s) {
	SceneHotspot::synchronize(s);
	s.syncAsSint16LE(_resNum);
	s.syncAsSint16LE(_lookLineNum);
	s.syncAsSint16LE(_useLineNum);

	if (g_vm->getGameID() == GType_BlueForce)
		s.syncAsSint16LE(_talkLineNum);
}

/*--------------------------------------------------------------------------*/

void SceneMessage::remove() {
	SceneExt *scene = (SceneExt *)BF_GLOBALS._sceneManager._scene;
	if (scene->_focusObject == this)
		scene->_focusObject = NULL;

	Action::remove();
}

void SceneMessage::signal() {
	SceneExt *scene = (SceneExt *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_focusObject = this;
		BF_GLOBALS._events.setCursor(CURSOR_WALK);
		draw();
		setDelay(180);
		break;
	case 1:
		clear();
		remove();
		break;
	default:
		break;
	}
}

void SceneMessage::process(Event &event) {
	if ((event.eventType == EVENT_BUTTON_DOWN) ||
		((event.eventType == EVENT_KEYPRESS) && (event.kbd.keycode == Common::KEYCODE_RETURN))) {
		signal();
	}
}


void SceneMessage::draw() {
	GfxSurface &surface = BF_GLOBALS._screenSurface;

	// Clear the game area
	surface.fillRect(Rect(0, 0, SCREEN_WIDTH, UI_INTERFACE_Y), 0);

	// Disable scene fade in
	BF_GLOBALS._paneRefreshFlag[0] = 0;

	// Set up the font
	GfxFont &font = BF_GLOBALS._gfxManagerInstance._font;
	BF_GLOBALS._scenePalette.setEntry(font._colors.foreground, 255, 255, 255);
	BF_GLOBALS._scenePalette.setPalette(font._colors.foreground, 1);

	// Write out the message
	Rect textRect(0, UI_INTERFACE_Y / 2 - (font.getHeight() / 2), SCREEN_WIDTH,
			UI_INTERFACE_Y / 2 + (font.getHeight() / 2));
	BF_GLOBALS._gfxManagerInstance._font.writeLines(_message.c_str(), textRect, ALIGN_CENTER);

	// TODO: Ideally, saving and loading should be disabled here until the message display is complete
}

void SceneMessage::clear() {
	// Fade out the text display
	static const uint32 black = 0;
	BF_GLOBALS._scenePalette.fade((const byte *)&black, false, 100);

	// Refresh the background
	BF_GLOBALS._paneRefreshFlag[0] = 0;

	// Set up to fade in the game scene
	g_globals->_sceneManager._fadeMode = FADEMODE_GRADUAL;
	g_globals->_sceneManager._hasPalette = true;
}

IntroSceneText::IntroSceneText(): SceneText() {
	_action = NULL;
	_frameNumber = 0;
	_diff = 0;
}

void IntroSceneText::setup(const Common::String &msg, Action *action) {
	_frameNumber = BF_GLOBALS._events.getFrameNumber();
	_diff = 180;
	_action = action;
	_fontNumber = 4;
	_width = 300;
	_textMode = ALIGN_CENTER;
	_color1 = BF_GLOBALS._scenePalette._colors.background;
	_color2 = _color3 = 0;

	SceneText::setup(msg);

	// Center the text on-screen
	reposition();
	_bounds.center(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

	// Set the new position
	_position.x = _bounds.left;
	_position.y = _bounds.top;
}

void IntroSceneText::synchronize(Serializer &s) {
	SceneText::synchronize(s);
	SYNC_POINTER(_action);
	s.syncAsUint32LE(_frameNumber);
	s.syncAsSint16LE(_diff);
}

void IntroSceneText::dispatch() {
	if (_diff) {
		uint32 frameNumber = BF_GLOBALS._events.getFrameNumber();
		if (_frameNumber < frameNumber) {
			_diff -= frameNumber - _frameNumber;
			_frameNumber = frameNumber;

			if (_diff <= 0) {
				// Time has expired, so remove the text and signal the designated action
				remove();
				if (_action)
					_action->signal();
			}
		}
	}
}

} // End of namespace BlueForce

} // End of namespace TsAGE

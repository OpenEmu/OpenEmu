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
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"
#include "tsage/ringworld2/ringworld2_logic.h"
#include "tsage/ringworld2/ringworld2_dialogs.h"
#include "tsage/ringworld2/ringworld2_scenes0.h"
#include "tsage/ringworld2/ringworld2_scenes1.h"
#include "tsage/ringworld2/ringworld2_scenes2.h"
#include "tsage/ringworld2/ringworld2_scenes3.h"

namespace TsAGE {

namespace Ringworld2 {

Scene *Ringworld2Game::createScene(int sceneNumber) {
	warning("Switching to scene %d", sceneNumber);

	switch (sceneNumber) {
	/* Scene group #0 */
	case 50:
		// Waking up cutscene
		return new Scene50();
	case 100:
		// Quinn's room
		return new Scene100();
	case 125:
		// Computer console
		return new Scene125();
	case 150:
		// Empty Bedroom #1
		return new Scene150();
	case 160:
		// Credits
		return new Scene160();
	case 175:
		// Empty Bedroom #2
		return new Scene175();
	case 180:
		// Title Screen
		return new Scene180();
	case 200:
		// Deck #2 - By Lift
		return new Scene200();
	case 205:
		// Star-field Credits
		return new Scene205();
	case 250:
		// Lift
		return new Scene250();
	case 300:
		// Bridge
		return new Scene300();
	case 325:
		// Bridge Console
		return new Scene325();
	case 400:
		// Science Lab
		return new Scene400();
	case 500:
		// Lander Bay 2 Storage
		return new Scene500();
	case 525:
		// Cutscene - Walking in hall
		return new Scene525();
	case 600:
		return new Scene600();
	case 700:
		return new Scene700();
	case 800:
		// Sick bay
		return new Scene800();
	case 825:
		// Autodoc
		return new Scene825();
	case 850:
		// Deck #5 - By Lift
		return new Scene850();
	case 900:
		return new Scene900();
	/* Scene group #1 */
	//
	case 1000:
		error("Missing scene %d from group 1", sceneNumber);
	case 1010:
		// Cutscene - trip in space
		return new Scene1010();
	case 1020:
		return new Scene1020();
	case 1100:
		return new Scene1100();
	case 1200:
		return new Scene1200();
	case 1337:
	case 1330:
		// Card Game
		return new Scene1337();
	case 1500:
		// Cutscene: Ship landing
		return new Scene1500();
	case 1525:
		// Cutscene - Ship
		return new Scene1525();
	case 1530:
		// Cutscene - Elevator
		return new Scene1530();
	case 1550:
		return new Scene1550();
	case 1575:
		return new Scene1575();
	case 1580:
		// Inside wreck
		return new Scene1580();
	case 1625:
		// Miranda being questioned
		return new Scene1625();
	case 1700:
		return new Scene1700();
	case 1750:
		return new Scene1750();
	case 1800:
		return new Scene1800();
	case 1850:
		return new Scene1850();
	case 1875:
		return new Scene1875();
	case 1900:
		return new Scene1900();
	case 1925:
		return new Scene1925();
	case 1945:
		return new Scene1945();
	case 1950:
		return new Scene1950();
	/* Scene group #2 */
	//
	case 2000:
		// Ice Maze
		return new Scene2000();
	case 2350:
		// Ice Maze: Balloon Launch Platform
		return new Scene2350();
	case 2400:
		// Ice Maze: Large empty room
		return new Scene2400();
	case 2425:
		// Ice Maze:
		return new Scene2425();
	case 2430:
		// Ice Maze: Bedroom
		return new Scene2430();
	case 2435:
		// Ice Maze: Throne room
		return new Scene2435();
	case 2440:
		// Ice Maze: Another bedroom
		return new Scene2440();
	case 2445:
		// Ice Maze:
		return new Scene2445();
	case 2450:
		// Ice Maze: Another bedroom
		return new Scene2450();
	case 2455:
		// Ice Maze: Inside crevasse
		return new Scene2455();
	case 2500:
		// Ice Maze: Large Cave
		return new Scene2500();
	case 2525:
		// Ice Maze: Furnace room
		return new Scene2525();
	case 2530:
		// Ice Maze: Well
		return new Scene2530();
	case 2535:
		// Ice Maze: Tannery
		return new Scene2535();
	case 2600:
		// Ice Maze: Exit
		return new Scene2600();
	case 2700:
		// Forest Maze
		return new Scene2700();
	case 2750:
		// Forest Maze
		return new Scene2750();
	case 2800:
		// Exiting Forest
		return new Scene2800();
	case 2900:
		error("Missing scene %d from group 2", sceneNumber);
	/* Scene group #3 */
	//
	case 3100:
		return new Scene3100();
	case 3125:
		// Ghouls dormitory
		return new Scene3125();
	case 3150:
		// Jail
		return new Scene3150();
	case 3175:
		// Autopsy room
		return new Scene3175();
	case 3200:
		// Cutscene : Guards - Discussion
		return new Scene3200();
	case 3210:
		// Cutscene : Captain and Private - Discussion
		return new Scene3210();
	case 3220:
		// Cutscene : Guards in cargo zone
		return new Scene3220();
	case 3230:
		// Cutscene : Guards on duty
		return new Scene3230();
	case 3240:
		// Cutscene : Teal monolog
		return new Scene3240();
	case 3245:
		// Cutscene : Discussions with Dr. Tomko
		return new Scene3245();
	case 3250:
		// Room with large stasis field negator
		return new Scene3250();
	case 3255:
		return new Scene3255();
	case 3260:
		// Computer room
		return new Scene3260();
	case 3275:
		// Hall
		return new Scene3275();
	case 3350:
		// Cutscene - Ship landing
		return new Scene3350();
	case 3375:
		return new Scene3375();
	case 3385:
		return new Scene3385();
	case 3395:
		return new Scene3395();
	case 3400:
		return new Scene3400();
	case 3500:
		return new Scene3500();
	case 3600:
		return new Scene3600();
	case 3700:
		// Cutscene - Teleport outside
		return new Scene3700();
	case 3800:
		return new Scene3800();
	case 3900:
		return new Scene3900();
	default:
		error("Unknown scene number - %d", sceneNumber);
		break;
	}
}

/**
 * Returns true if it is currently okay to restore a game
 */
bool Ringworld2Game::canLoadGameStateCurrently() {
	// Don't allow a game to be loaded if a dialog is active
	return g_globals->_gfxManagers.size() == 1;
}

/**
 * Returns true if it is currently okay to save the game
 */
bool Ringworld2Game::canSaveGameStateCurrently() {
	// Don't allow a game to be saved if a dialog is active
	return g_globals->_gfxManagers.size() == 1;
}

/*--------------------------------------------------------------------------*/

SceneExt::SceneExt(): Scene() {
	_stripManager._onBegin = SceneExt::startStrip;
	_stripManager._onEnd = SceneExt::endStrip;

	for (int i = 0; i < 256; i++)
		_field312[i] = 0;
	_field372 = _field37A = 0;
	_savedPlayerEnabled = false;
	_savedUiEnabled = false;
	_savedCanWalk = false;
	_focusObject = NULL;
}

void SceneExt::postInit(SceneObjectList *OwnerList) {
	Scene::postInit(OwnerList);

	// Exclude the bottom area of the screen to allow room for the UI
	T2_GLOBALS._interfaceY = UI_INTERFACE_Y;

	// Initialise fields
	_action = NULL;
	_field12 = 0;
	_sceneMode = 0;

	int prevScene = R2_GLOBALS._sceneManager._previousScene;
	int sceneNumber = R2_GLOBALS._sceneManager._sceneNumber;
	if (((prevScene == -1) && (sceneNumber != 180) && (sceneNumber != 205) && (sceneNumber != 50))
			|| (sceneNumber == 50)
			|| ((prevScene == 205) && (sceneNumber == 100))
			|| ((prevScene == 180) && (sceneNumber == 100))) {
		static_cast<SceneHandlerExt *>(R2_GLOBALS._sceneHandler)->setupPaletteMaps();
		R2_GLOBALS._uiElements._active = true;
		R2_GLOBALS._uiElements.show();
	} else {
		R2_GLOBALS._uiElements.updateInventory();
	}
}

void SceneExt::remove() {
	_sceneAreas.clear();
	Scene::remove();
}

void SceneExt::process(Event &event) {
	if (!event.handled)
		Scene::process(event);
}

void SceneExt::dispatch() {
/*
	_timerList.dispatch();

	if (_field37A) {
		if ((--_field37A == 0) && R2_GLOBALS._dayNumber) {
			if (R2_GLOBALS._uiElements._active && R2_GLOBALS._player._enabled) {
				R2_GLOBALS._uiElements.show();
			}

			_field37A = 0;
		}
	}
*/
	Scene::dispatch();
}

void SceneExt::loadScene(int sceneNum) {
	Scene::loadScene(sceneNum);

	_v51C34.top = 0;
	_v51C34.bottom = 300;

	int prevScene = R2_GLOBALS._sceneManager._previousScene;
	int sceneNumber = R2_GLOBALS._sceneManager._sceneNumber;

	if (((prevScene == -1) && (sceneNumber != 180) && (sceneNumber != 205) && (sceneNumber != 50)) ||
			(sceneNumber == 50) || ((prevScene == 205) && (sceneNumber == 100)) ||
			((prevScene == 180) && (sceneNumber == 100))) {
		// TODO: sub_17875
		R2_GLOBALS._uiElements._active = true;
		R2_GLOBALS._uiElements.show();
	} else {
		// Update the user interface
		R2_GLOBALS._uiElements.updateInventory();
	}
}

bool SceneExt::display(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_CROSSHAIRS:
	case CURSOR_WALK:
		return false;
	case CURSOR_LOOK:
		SceneItem::display2(1, R2_GLOBALS._randomSource.getRandomNumber(4));
		break;
	case CURSOR_USE:
		SceneItem::display2(1, R2_GLOBALS._randomSource.getRandomNumber(4) + 5);
		break;
	case CURSOR_TALK:
		SceneItem::display2(1, R2_GLOBALS._randomSource.getRandomNumber(4) + 10);
		break;
	case R2_NEGATOR_GUN:
		if (R2_GLOBALS.getFlag(1))
			SceneItem::display2(2, action);
		else
			SceneItem::display2(5, 0);
		break;
	case R2_SONIC_STUNNER:
		if ((R2_GLOBALS._v565F1[1] == 2) || ((R2_GLOBALS._v565F1[1] == 1) &&
				(R2_GLOBALS._v565F1[2] == 2) && (R2_GLOBALS._sceneManager._previousScene == 300))) {
			R2_GLOBALS._sound4.stop();
			R2_GLOBALS._sound3.play(46);
			SceneItem::display2(5, 15);
		} else {
			R2_GLOBALS._sound3.play(43, 0);
			SceneItem::display2(2, 0);
		}

		R2_GLOBALS._sound4.play(45);
		break;
	case R2_COM_SCANNER:
	case R2_COM_SCANNER_2:
		R2_GLOBALS._sound3.play(44);
		SceneItem::display2(2, action);
		R2_GLOBALS._sound3.stop();
		break;
	case R2_PHOTON_STUNNER:
		R2_GLOBALS._sound3.play(99);
		SceneItem::display2(2, action);
		break;
	default:
		SceneItem::display2(2, action);
		break;
	}

	event.handled = true;
	return true;
}

void SceneExt::fadeOut() {
	uint32 black = 0;
	R2_GLOBALS._scenePalette.fade((const byte *)&black, false, 100);
}

void SceneExt::startStrip() {
	SceneExt *scene = (SceneExt *)R2_GLOBALS._sceneManager._scene;
	scene->_field372 = 1;
	scene->_savedPlayerEnabled = R2_GLOBALS._player._enabled;

	if (scene->_savedPlayerEnabled) {
		scene->_savedUiEnabled = R2_GLOBALS._player._uiEnabled;
		scene->_savedCanWalk = R2_GLOBALS._player._canWalk;
		R2_GLOBALS._player.disableControl();
/*
		if (!R2_GLOBALS._v50696 && R2_GLOBALS._uiElements._active)
			R2_GLOBALS._uiElements.hide();
*/
	}
}

void SceneExt::endStrip() {
	SceneExt *scene = (SceneExt *)R2_GLOBALS._sceneManager._scene;
	scene->_field372 = 0;

	if (scene->_savedPlayerEnabled) {
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._uiEnabled = scene->_savedUiEnabled;
		R2_GLOBALS._player._canWalk = scene->_savedCanWalk;
/*
		if (!R2_GLOBALS._v50696 && R2_GLOBALS._uiElements._active)
			R2_GLOBALS._uiElements.show();
*/
	}
}

void SceneExt::clearScreen() {
	R2_GLOBALS._screenSurface.fillRect(R2_GLOBALS._screenSurface.getBounds(), 0);
}

void SceneExt::refreshBackground(int xAmount, int yAmount) {
	switch (_activeScreenNumber) {
	case 700:
	case 1020:
	case 1100:
	case 1700:
	case 2600:
	case 2950:
	case 3100:
	case 3101:
	case 3275:
	case 3600:
		// Use traditional style sectioned screen loading
		Scene::refreshBackground(xAmount, yAmount);
		return;
	default:
		// Break out to new style screen loading
		break;
	}

	/* New style background loading */

	// Get the screen data
	byte *dataP = g_resourceManager->getResource(RT18, _activeScreenNumber, 0);
	int screenSize = g_vm->_memoryManager.getSize(dataP);

	// Lock the background for update
	Graphics::Surface s = _backSurface.lockSurface();
	assert(screenSize == (s.w * s.h));

	// Copy the data
	byte *destP = (byte *)s.getBasePtr(0, 0);
	Common::copy(dataP, dataP + (s.w * s.h), destP);
	_backSurface.unlockSurface();

	// Free the resource data
	DEALLOCATE(dataP);
}

/**
 * Saves the current player position and view in the details for the specified character index
 */
void SceneExt::saveCharacter(int characterIndex) {
	R2_GLOBALS._player._characterPos[characterIndex] = R2_GLOBALS._player._position;
	R2_GLOBALS._player._characterStrip[characterIndex] = R2_GLOBALS._player._strip;
	R2_GLOBALS._player._characterFrame[characterIndex] = R2_GLOBALS._player._frame;
}

void SceneExt::scalePalette(int RFactor, int GFactor, int BFactor) {
	byte *tmpPal = R2_GLOBALS._scenePalette._palette;
	byte newR, newG, newB;
	int tmp, varC, varD = 0;

	for (int i = 0; i < 256; i++) {
		newR = (RFactor * tmpPal[(3 * i)]) / 100;
		newG = (GFactor * tmpPal[(3 * i) + 1]) / 100;
		newB = (BFactor * tmpPal[(3 * i) + 2]) / 100;

		varC = 769;
		for (int j = 255; j >= 0; j--) {
			tmp = abs(tmpPal[(3 * j)] - newR);
			if (tmp >= varC)
				continue;

			tmp += abs(tmpPal[(3 * j) + 1] - newG);
			if (tmp >= varC)
				continue;

			tmp += abs(tmpPal[(3 * j) + 2] - newB);
			if (tmp >= varC)
				continue;

			varC = tmp;
			varD = j;
		}
		this->_field312[i] = varD;
	}
}

/*--------------------------------------------------------------------------*/

void SceneHandlerExt::postInit(SceneObjectList *OwnerList) {
	SceneHandler::postInit(OwnerList);
}

void SceneHandlerExt::process(Event &event) {
	if (T2_GLOBALS._uiElements._active && R2_GLOBALS._player._uiEnabled) {
		T2_GLOBALS._uiElements.process(event);
		if (event.handled)
			return;
	}

	SceneExt *scene = static_cast<SceneExt *>(R2_GLOBALS._sceneManager._scene);
	if (scene && R2_GLOBALS._player._uiEnabled) {
		// Handle any scene areas that have been registered
		SynchronizedList<SceneArea *>::iterator saIter;
		for (saIter = scene->_sceneAreas.begin(); saIter != scene->_sceneAreas.end() && !event.handled; ++saIter) {
			(*saIter)->process(event);
		}
	}

	if (!event.handled)
		SceneHandler::process(event);
}

void SceneHandlerExt::setupPaletteMaps() {
	byte *palP = &R2_GLOBALS._scenePalette._palette[0];

	if (!R2_GLOBALS._v1000Flag) {
		R2_GLOBALS._v1000Flag = true;

		for (int idx = 0; idx < 10; ++idx) {
			for (int palIndex = 0; palIndex < 224; ++palIndex) {
				int r, g, b;

				// Get adjusted RGB values
				switch (idx) {
				case 7:
					r = palP[palIndex * 3] * 85 / 100;
					g = palP[palIndex * 3 + 1] * 7 / 10;
					b = palP[palIndex * 3 + 2] * 7 / 10;
					break;
				case 8:
					r = palP[palIndex * 3] * 7 / 10;
					g = palP[palIndex * 3 + 1] * 85 / 100;
					b = palP[palIndex * 3 + 2] * 7 / 10;
					break;
				case 9:
					r = palP[palIndex * 3] * 8 / 10;
					g = palP[palIndex * 3 + 1] * 5 / 10;
					b = palP[palIndex * 3 + 2] * 9 / 10;
					break;
				default:
					r = palP[palIndex * 3] * (10 - idx) / 10;
					g = palP[palIndex * 3 + 1] * (10 - idx) / 12;
					b = palP[palIndex * 3 + 2] * (10 - idx) / 10;
					break;
				}

				// Scan for the palette index with the closest matching colour
				int threshold = 769;
				int foundIndex = -1;
				for (int pIndex2 = 223; pIndex2 >= 0; --pIndex2) {
					int diffSum = ABS(palP[pIndex2 * 3] - r);
					if (diffSum >= threshold)
						continue;

					diffSum += ABS(palP[pIndex2 * 3 + 1] - g);
					if (diffSum >= threshold)
						continue;

					diffSum += ABS(palP[pIndex2 * 3 + 2] - b);
					if (diffSum >= threshold)
						continue;

					threshold = diffSum;
					foundIndex = pIndex2;
				}

				R2_GLOBALS._palIndexList[idx][palIndex] = foundIndex;
			}
		}
	}

	for (int palIndex = 0; palIndex < 224; ++palIndex) {
		int r = palP[palIndex * 3] >> 2;
		int g = palP[palIndex * 3 + 1] >> 2;
		int b = palP[palIndex * 3 + 2] >> 2;

		int idx = (((r << 4) | g) << 4) | b;
		R2_GLOBALS._v1000[idx] = palIndex;
	}

	int vdx = 0;
	int idx = 0;
	int palIndex = 224;

	for (int vIndex = 0; vIndex < 4096; ++vIndex) {
		int v = R2_GLOBALS._v1000[vIndex];
		if (!v) {
			R2_GLOBALS._v1000[vIndex] = idx;
		} else {
			idx = v;
		}

		if (!palIndex) {
			vdx = palIndex;
		} else {
			int idxTemp = palIndex;
			palIndex = (palIndex + vdx) / 2;
			vdx = idxTemp;
		}
	}
}

/*--------------------------------------------------------------------------*/

DisplayHotspot::DisplayHotspot(int regionId, ...) {
	_sceneRegionId = regionId;

	// Load up the actions
	va_list va;
	va_start(va, regionId);

	int param = va_arg(va, int);
	while (param != LIST_END) {
		_actions.push_back(param);
		param = va_arg(va, int);
	}

	va_end(va);
}

bool DisplayHotspot::performAction(int action) {
	for (uint i = 0; i < _actions.size(); i += 3) {
		if (_actions[i] == action) {
			display(_actions[i + 1], _actions[i + 2], SET_WIDTH, 200, SET_EXT_BGCOLOR, 7, LIST_END);
			return true;
		}
	}

	return false;
}

/*--------------------------------------------------------------------------*/

DisplayObject::DisplayObject(int firstAction, ...) {
	// Load up the actions
	va_list va;
	va_start(va, firstAction);

	int param = firstAction;
	while (param != LIST_END) {
		_actions.push_back(param);
		param = va_arg(va, int);
	}

	va_end(va);
}

bool DisplayObject::performAction(int action) {
	for (uint i = 0; i < _actions.size(); i += 3) {
		if (_actions[i] == action) {
			display(_actions[i + 1], _actions[i + 2], SET_WIDTH, 200, SET_EXT_BGCOLOR, 7, LIST_END);
			return true;
		}
	}

	return false;
}

/*--------------------------------------------------------------------------*/

Ringworld2InvObjectList::Ringworld2InvObjectList():
		_none(1, 1),
		_inv1(1, 2),
		_inv2(1, 3),
		_negatorGun(1, 4),
		_steppingDisks(1, 5),
		_inv5(1, 6),
		_inv6(1, 7),
		_inv7(1, 8),
		_inv8(1, 9),
		_inv9(1, 10),
		_inv10(1, 11),
		_inv11(1, 12),
		_inv12(1, 13),
		_inv13(1, 14),
		_inv14(1, 15),
		_inv15(1, 16),
		_inv16(1, 17),
		_inv17(2, 2),
		_inv18(2, 3),
		_inv19(2, 4),
		_inv20(2, 5),
		_inv21(2, 5),
		_inv22(2, 6),
		_inv23(2, 7),
		_inv24(2, 8),
		_inv25(2, 9),
		_inv26(2, 10),
		_inv27(2, 11),
		_inv28(2, 12),
		_inv29(2, 13),
		_inv30(2, 14),
		_inv31(2, 15),
		_inv32(2, 16),
		_inv33(3, 2),
		_inv34(3, 3),
		_inv35(3, 4),
		_inv36(3, 5),
		_inv37(3, 6),
		_inv38(3, 7),
		_inv39(1, 10),
		_inv40(3, 8),
		_inv41(3, 9),
		_inv42(3, 10),
		_inv43(3, 11),
		_inv44(3, 12),
		_inv45(3, 13),
		_inv46(3, 17),
		_inv47(3, 14),
		_inv48(3, 14),
		_inv49(3, 15),
		_inv50(3, 15),
		_inv51(3, 17),
		_inv52(4, 2) {

	// Add the items to the list
	_itemList.push_back(&_none);
	_itemList.push_back(&_inv1);
	_itemList.push_back(&_inv2);
	_itemList.push_back(&_negatorGun);
	_itemList.push_back(&_steppingDisks);
	_itemList.push_back(&_inv5);
	_itemList.push_back(&_inv6);
	_itemList.push_back(&_inv7);
	_itemList.push_back(&_inv8);
	_itemList.push_back(&_inv9);
	_itemList.push_back(&_inv10);
	_itemList.push_back(&_inv11);
	_itemList.push_back(&_inv12);
	_itemList.push_back(&_inv13);
	_itemList.push_back(&_inv14);
	_itemList.push_back(&_inv15);
	_itemList.push_back(&_inv16);
	_itemList.push_back(&_inv17);
	_itemList.push_back(&_inv18);
	_itemList.push_back(&_inv19);
	_itemList.push_back(&_inv20);
	_itemList.push_back(&_inv21);
	_itemList.push_back(&_inv22);
	_itemList.push_back(&_inv23);
	_itemList.push_back(&_inv24);
	_itemList.push_back(&_inv25);
	_itemList.push_back(&_inv26);
	_itemList.push_back(&_inv27);
	_itemList.push_back(&_inv28);
	_itemList.push_back(&_inv29);
	_itemList.push_back(&_inv30);
	_itemList.push_back(&_inv31);
	_itemList.push_back(&_inv32);
	_itemList.push_back(&_inv33);
	_itemList.push_back(&_inv34);
	_itemList.push_back(&_inv35);
	_itemList.push_back(&_inv36);
	_itemList.push_back(&_inv37);
	_itemList.push_back(&_inv38);
	_itemList.push_back(&_inv39);
	_itemList.push_back(&_inv40);
	_itemList.push_back(&_inv41);
	_itemList.push_back(&_inv42);
	_itemList.push_back(&_inv43);
	_itemList.push_back(&_inv44);
	_itemList.push_back(&_inv45);
	_itemList.push_back(&_inv46);
	_itemList.push_back(&_inv47);
	_itemList.push_back(&_inv48);
	_itemList.push_back(&_inv49);
	_itemList.push_back(&_inv50);
	_itemList.push_back(&_inv51);
	_itemList.push_back(&_inv52);

	_selectedItem = NULL;
}

void Ringworld2InvObjectList::reset() {
	// Reset all object scene numbers
	SynchronizedList<InvObject *>::iterator i;
	for (i = _itemList.begin(); i != _itemList.end(); ++i) {
		(*i)->_sceneNumber = 0;
	}

	// Set up default inventory
	setObjectScene(R2_OPTO_DISK, 800);
	setObjectScene(R2_READER, 400);
	setObjectScene(R2_NEGATOR_GUN, 100);
	setObjectScene(R2_STEPPING_DISKS, 100);
	setObjectScene(R2_ATTRACTOR_UNIT, 400);
	setObjectScene(R2_SENSOR_PROBE, 400);
	setObjectScene(R2_SONIC_STUNNER, 500);
	setObjectScene(R2_CABLE_HARNESS, 700);
	setObjectScene(R2_COM_SCANNER, 800);
	setObjectScene(R2_SPENT_POWER_CAPSULE, 100);
	setObjectScene(R2_CHARGED_POWER_CAPSULE, 400);
	setObjectScene(R2_AEROSOL, 500);
	setObjectScene(R2_REMOTE_CONTROL, 1550);
	setObjectScene(R2_OPTICAL_FIBRE, 850);
	setObjectScene(R2_CLAMP, 850);
	setObjectScene(R2_ATTRACTOR_CABLE_HARNESS, 0);
	setObjectScene(R2_FUEL_CELL, 1550);
	setObjectScene(R2_GYROSCOPE, 1550);
	setObjectScene(R2_AIRBAG, 1550);
	setObjectScene(R2_REBREATHER_TANK, 500);
	setObjectScene(R2_RESERVE_REBREATHER_TANK, 500);
	setObjectScene(R2_GUIDANCE_MODULE, 1550);
	setObjectScene(R2_THRUSTER_VALVE, 1580);
	setObjectScene(R2_BALLOON_BACKPACK, 9999);
	setObjectScene(R2_RADAR_MECHANISM, 1550);
	setObjectScene(R2_JOYSTICK, 1550);
	setObjectScene(R2_IGNITOR, 1580);
	setObjectScene(R2_DIAGNOSTICS_DISPLAY, 1550);
	setObjectScene(R2_GLASS_DOME, 2525);
	setObjectScene(R2_WICK_LAMP, 2440);
	setObjectScene(R2_SCRITH_KEY, 2455);
	setObjectScene(R2_TANNER_MASK, 2535);
	setObjectScene(R2_PURE_GRAIN_ALCOHOL, 2530);
	setObjectScene(R2_SAPPHIRE_BLUE, 1950);
	setObjectScene(R2_ANCIENT_SCROLLS, 1950);
	setObjectScene(R2_FLUTE, 9999);
	setObjectScene(R2_GUNPOWDER, 2430);
	setObjectScene(R2_NONAME, 9999);
	setObjectScene(R2_COM_SCANNER_2, 2);
	setObjectScene(R2_SUPERCONDUCTOR_WIRE, 9999);
	setObjectScene(R2_PILLOW, 3150);
	setObjectScene(R2_FOOD_TRAY, 0);
	setObjectScene(R2_LASER_HACKSAW, 3260);
	setObjectScene(R2_PHOTON_STUNNER, 2);
	setObjectScene(R2_BATTERY, 1550);
	setObjectScene(R2_SOAKED_FACEMASK, 0);
	setObjectScene(R2_LIGHT_BULB, 3150);
	setObjectScene(R2_ALCOHOL_LAMP, 2435);
	setObjectScene(R2_ALCOHOL_LAMP_2, 2440);
	setObjectScene(R2_ALCOHOL_LAMP_3, 2435);
	setObjectScene(R2_BROKEN_DISPLAY, 1580);
	setObjectScene(R2_TOOLBOX, 3260);
}

void Ringworld2InvObjectList::setObjectScene(int objectNum, int sceneNumber) {
	// Find the appropriate object
	int num = objectNum;
	SynchronizedList<InvObject *>::iterator i = _itemList.begin();
	while (num-- > 0) ++i;
	(*i)->_sceneNumber = sceneNumber;

	// If the item is the currently active one, default back to the use cursor
	if (R2_GLOBALS._events.getCursor() == objectNum)
		R2_GLOBALS._events.setCursor(CURSOR_USE);

	// Update the user interface if necessary
	T2_GLOBALS._uiElements.updateInventory();
}

/*--------------------------------------------------------------------------*/

void Ringworld2Game::start() {
	int slot = -1;

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
		R2_GLOBALS._sceneHandler->_loadGameSlot = slot;
	else {
		// Switch to the first game scene
		R2_GLOBALS._events.setCursor(CURSOR_WALK);
		R2_GLOBALS._uiElements._active = true;
		R2_GLOBALS._sceneManager.setNewScene(100);
	}

	g_globals->_events.showCursor();
}

void Ringworld2Game::restart() {
	g_globals->_scenePalette.clearListeners();
	g_globals->_soundHandler.stop();

	// Change to the first game scene
	g_globals->_sceneManager.changeScene(100);
}

void Ringworld2Game::endGame(int resNum, int lineNum) {
	g_globals->_events.setCursor(CURSOR_WALK);
	Common::String msg = g_resourceManager->getMessage(resNum, lineNum);
	bool savesExist = g_saver->savegamesExist();

	if (!savesExist) {
		// No savegames exist, so prompt the user to restart or quit
		if (MessageDialog::show(msg, QUIT_BTN_STRING, RESTART_BTN_STRING) == 0)
			g_vm->quitGame();
		else
			restart();
	} else {
		// Savegames exist, so prompt for Restore/Restart
		bool breakFlag;
		do {
			if (g_vm->shouldQuit()) {
				breakFlag = true;
			} else if (MessageDialog::show(msg, RESTART_BTN_STRING, RESTORE_BTN_STRING) == 0) {
				restart();
				breakFlag = true;
			} else {
				handleSaveLoad(false, g_globals->_sceneHandler->_loadGameSlot, g_globals->_sceneHandler->_saveName);
				breakFlag = g_globals->_sceneHandler->_loadGameSlot >= 0;
			}
		} while (!breakFlag);
	}

	g_globals->_events.setCursorFromFlag();
}

void Ringworld2Game::processEvent(Event &event) {
	if (event.eventType == EVENT_KEYPRESS) {
		switch (event.kbd.keycode) {
		case Common::KEYCODE_F1:
			// F1 - Help
			HelpDialog::show();
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

		case Common::KEYCODE_F8:
			// F8 - Credits
			warning("TODO: Show Credits");
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

void Ringworld2Game::rightClick() {
	RightClickDialog *dlg = new RightClickDialog();
	dlg->execute();
	delete dlg;
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

/*--------------------------------------------------------------------------*/

void SceneActor::postInit(SceneObjectList *OwnerList) {
	_lookLineNum = _talkLineNum = _useLineNum = -1;
	SceneObject::postInit();
}

bool SceneActor::startAction(CursorType action, Event &event) {
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
		handled = ((SceneExt *)R2_GLOBALS._sceneManager._scene)->display(action, event);
	return handled;
}

/*--------------------------------------------------------------------------*/

SceneArea::SceneArea(): EventHandler() {
	_enabled = true;
	_insideArea = false;
	_savedCursorNum = CURSOR_NONE;
	_cursorState = 0;
}

void SceneArea::synchronize(Serializer &s) {
	EventHandler::synchronize(s);

	_bounds.synchronize(s);
	s.syncAsSint16LE(_enabled);
	s.syncAsSint16LE(_insideArea);
	s.syncAsSint16LE(_cursorNum);
	s.syncAsSint16LE(_savedCursorNum);
	s.syncAsSint16LE(_cursorState);
}

void SceneArea::remove() {
	static_cast<SceneExt *>(R2_GLOBALS._sceneManager._scene)->_sceneAreas.remove(this);
}

void SceneArea::process(Event &event) {
	if (!R2_GLOBALS._insetUp && _enabled && R2_GLOBALS._events.isCursorVisible()) {
		CursorType cursor = R2_GLOBALS._events.getCursor();

		if (_bounds.contains(event.mousePos)) {
			// Cursor moving in bounded area
			if (cursor != _cursorNum) {
				_savedCursorNum = cursor;
				_cursorState = 0;
				R2_GLOBALS._events.setCursor(_cursorNum);
			}
			_insideArea = true;
		} else if ((event.mousePos.y < 171) && _insideArea && (_cursorNum == cursor) &&
				(_savedCursorNum != CURSOR_NONE)) {
			// Cursor moved outside bounded area
			R2_GLOBALS._events.setCursor(_savedCursorNum);
		}
	}
}

void SceneArea::setDetails(const Rect &bounds, CursorType cursor) {
	_bounds = bounds;
	_cursorNum = cursor;

	static_cast<SceneExt *>(R2_GLOBALS._sceneManager._scene)->_sceneAreas.push_front(this);
}

/*--------------------------------------------------------------------------*/

SceneExit::SceneExit(): SceneArea() {
	_moving = false;
	_destPos = Common::Point(-1, -1);
}

void SceneExit::synchronize(Serializer &s) {
	SceneArea::synchronize(s);

	s.syncAsSint16LE(_moving);
	s.syncAsSint16LE(_destPos.x);
	s.syncAsSint16LE(_destPos.y);
}

void SceneExit::setDetails(const Rect &bounds, CursorType cursor, int sceneNumber) {
	_sceneNumber = sceneNumber;
	SceneArea::setDetails(bounds, cursor);
}

void SceneExit::changeScene() {
	R2_GLOBALS._sceneManager.setNewScene(_sceneNumber);
}

void SceneExit::process(Event &event) {
	if (!R2_GLOBALS._insetUp) {
		SceneArea::process(event);

		if (_enabled) {
			if (event.eventType == EVENT_BUTTON_DOWN) {
				if (!_bounds.contains(event.mousePos))
					_moving = false;
				else if (!R2_GLOBALS._player._canWalk) {
					_moving = false;
					changeScene();
					event.handled = true;
				} else {
					Common::Point dest((_destPos.x == -1) ? event.mousePos.x : _destPos.x,
						(_destPos.y == -1) ? event.mousePos.y : _destPos.y);
					ADD_PLAYER_MOVER(dest.x, dest.y);

					_moving = true;
					event.handled = true;
				}
			}

			if (_moving && (_bounds.contains(R2_GLOBALS._player._position) || (R2_GLOBALS._player._position == _destPos)))
				changeScene();
		}
	}
}

/*--------------------------------------------------------------------------*/

void SceneAreaObject::remove() {
	_object1.remove();
	SceneArea::remove();
	--R2_GLOBALS._insetUp;
}

void SceneAreaObject::process(Event &event) {
	if (_insetCount == R2_GLOBALS._insetUp) {
		CursorType cursor = R2_GLOBALS._events.getCursor();

		if (_bounds.contains(event.mousePos)) {
			// Cursor moving in bounded area
			if (cursor == _cursorNum) {
				R2_GLOBALS._events.setCursor(_savedCursorNum);
			}
		} else if (event.mousePos.y < 168) {
			if (_cursorNum != cursor)
				// Cursor moved outside bounded area
				R2_GLOBALS._events.setCursor(_savedCursorNum);

			if (event.eventType == EVENT_BUTTON_DOWN) {
				R2_GLOBALS._events.setCursor(_savedCursorNum);
				event.handled = true;
			}
		}
	}
}

void SceneAreaObject::setDetails(int visage, int strip, int frameNumber, const Common::Point &pt) {
	_object1.postInit();
	_object1.setup(visage, strip, frameNumber);
	_object1.setPosition(pt);
	_object1.fixPriority(250);

	_cursorNum = CURSOR_INVALID;
	Scene500 *scene = (Scene500 *)R2_GLOBALS._sceneManager._scene;
	scene->_sceneAreas.push_front(this);

	_insetCount = ++R2_GLOBALS._insetUp;
}

void SceneAreaObject::setDetails(int resNum, int lookLineNum, int talkLineNum, int useLineNum) {
	((SceneHotspot *)(this))->setDetails(resNum, lookLineNum, talkLineNum, useLineNum,
		2, (SceneItem *)NULL);
}

/*****************************************************************************/

UnkObject1200::UnkObject1200() {
	_field16 = _field3A = NULL;
	_field12 = _field14 = 0;
	_field26 = _field28 = _field2A = _field2C = _field2E = _field30 = 0;
	_field32 = _field34 = _field36 = _field38 = _field3E = _field40 = 0;
}

void UnkObject1200::synchronize(Serializer &s) {
	SavedObject::synchronize(s);

	_rect1.synchronize(s);
	_rect2.synchronize(s);

	// FIXME: syncrhonize _field16 and _field3A

	s.syncAsSint16LE(_field12);
	s.syncAsSint16LE(_field14);
	s.syncAsSint16LE(_field26);
	s.syncAsSint16LE(_field28);
	s.syncAsSint16LE(_field2A);
	s.syncAsSint16LE(_field2C);
	s.syncAsSint16LE(_field2E);
	s.syncAsSint16LE(_field30);
	s.syncAsSint16LE(_field32);
	s.syncAsSint16LE(_field34);
	s.syncAsSint16LE(_field36);
	s.syncAsSint16LE(_field38);
	s.syncAsSint16LE(_field3E);
	s.syncAsSint16LE(_field40);
}

void UnkObject1200::sub51AE9(int arg1) {
	warning("STUB: UnkObject1200::sub51AE9()");
}

int UnkObject1200::sub51AF8(Common::Point pt) {
	if (!_rect1.contains(pt))
		return -1;

	int tmp1 = (pt.x - _rect1.left + _field2E) / _field2A;
	int tmp2 = (pt.y - _rect1.top + _field30) / _field2C;

	if ((tmp1 >= 0) && (tmp2 >= 0) && (_field26 > tmp1) && (_field28 > tmp2))
		return _field16[(((_field26 * tmp2) + tmp1)* 2)];

	return -1;
}

bool UnkObject1200::sub51AFD(Common::Point pt) {
	int retval = false;

	_field2E = pt.x;
	_field30 = pt.y;

	if (_field2E < _rect2.top) {
		_field2E = _rect2.top;
		retval = true;
	}

	if (_field30 < _rect2.left) {
		_field30 = _rect2.left;
		retval = true;
	}

	if (_field2E + _rect1.width() > _rect2.right) {
		_field2E = _rect2.right - _rect1.width();
		retval = true;
	}

	if (_field30 + _rect1.height() > _rect2.bottom) {
		_field30 = _rect2.bottom - _rect1.height();
		retval = true;
	}

	return retval;
}

void UnkObject1200::sub51B02() {
	warning("STUB: UnkObject1200::sub51B02()");
}

void UnkObject1200::sub9EDE8(Rect rect) {
	_rect1 = rect;
	warning("FIXME: UnkObject1200::sub9EDE8()");
//	_rect1.clip(g_globals->gfxManager()._bounds);
}

int UnkObject1200::sub9EE22(int &arg1, int &arg2) {
	arg1 /= _field2A;
	arg2 /= _field2C;

	if ((arg1 >= 0) && (arg2 >= 0) && (_field26 > arg1) && (_field28 > arg2)) {
		return _field16[(((_field26 * arg2) + arg1) * 2)];
	}

	return -1;
}

void Scene1200::sub9DAD6(int indx) {
	_object1.sub9EE22(R2_GLOBALS._v56AA2, R2_GLOBALS._v56AA4);

	switch (indx) {
	case 0:
		if ( ((_object1.sub51AF8(Common::Point(200, 50)) > 36) || (_object1.sub51AF8(Common::Point(200, 88)) > 36))
			&& ( ((R2_GLOBALS._v56AA2 == 3) && (R2_GLOBALS._v56AA4 == 33) && (_field418 != 4))
				|| ((R2_GLOBALS._v56AA2 == 13) && (R2_GLOBALS._v56AA4 == 21) && (_field418 != 2))
				|| ((R2_GLOBALS._v56AA2 == 29) && (R2_GLOBALS._v56AA4 == 17) && (_field418 != 1))
				|| ((R2_GLOBALS._v56AA2 == 33) && (R2_GLOBALS._v56AA4 == 41)) )
				)	{
			R2_GLOBALS._player.disableControl();
			_sceneMode = 1200;
			setAction(&_sequenceManager, this, 1200, &_actor1, NULL);
		} else if (_object1.sub51AF8(Common::Point(200, 69)) == 36) {
			switch (_field412 - 1) {
			case 0:
				if (R2_GLOBALS._player._visage == 3155)
					_sceneMode = 15;
				else
					_sceneMode = 10;
				break;
			case 1:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 76;
				else
					_sceneMode = 75;
				break;
			case 2:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 101;
				else
					_sceneMode = 100;
				break;
			case 3:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 111;
				else
					_sceneMode = 110;
				break;
			default:
				break;
			}
			R2_GLOBALS._player.disableControl();
			_field412 = 1;
			signal();
		}
		break;
	case 1:
		if ( ((_object1.sub51AF8(Common::Point(120, 50)) > 36) || (_object1.sub51AF8(Common::Point(120, 88)) > 36))
			&& ( ((R2_GLOBALS._v56AA2 == 7) && (R2_GLOBALS._v56AA4 == 33) && (_field418 != 4))
				|| ((R2_GLOBALS._v56AA2 == 17) && (R2_GLOBALS._v56AA4 == 21) && (_field418 != 2))
				|| ((R2_GLOBALS._v56AA2 == 33) && (R2_GLOBALS._v56AA4 == 17) && (_field418 != 1))
				|| ((R2_GLOBALS._v56AA2 == 5) && (R2_GLOBALS._v56AA4 == 5)) )
				)	{
			R2_GLOBALS._player.disableControl();
			_sceneMode = 1201;
			setAction(&_sequenceManager, this, 1201, &_actor1, NULL);
		} else if (_object1.sub51AF8(Common::Point(120, 69)) == 36) {
			switch (_field412 - 1) {
			case 0:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 56;
				else
					_sceneMode = 55;
				break;
			case 1:
				if (R2_GLOBALS._player._visage == 3155)
					_sceneMode = 25;
				else
					_sceneMode = 20;
				break;
			case 2:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 91;
				else
					_sceneMode = 90;
				break;
			case 3:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 121;
				else
					_sceneMode = 120;
				break;
			default:
				break;
			}
			R2_GLOBALS._player.disableControl();
			_field412 = 2;
			signal();
		}
		break;
	case 2:
		if ( ((_object1.sub51AF8(Common::Point(140, 110)) > 36) || (_object1.sub51AF8(Common::Point(178, 110)) > 36))
			&& ( ((R2_GLOBALS._v56AA2 == 17) && (R2_GLOBALS._v56AA4 == 5) && (_field418 != 3))
				|| ((R2_GLOBALS._v56AA2 == 41) && (R2_GLOBALS._v56AA4 == 21)) )
				)	{
			R2_GLOBALS._player.disableControl();
			_sceneMode = 1203;
			setAction(&_sequenceManager, this, 1203, &_actor1, NULL);
		} else if (_object1.sub51AF8(Common::Point(160, 110)) == 36) {
			switch (_field412 - 1) {
			case 0:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 51;
				else
					_sceneMode = 50;
				break;
			case 1:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 81;
				else
					_sceneMode = 80;
				break;
			case 2:
				if (R2_GLOBALS._player._visage == 3155)
					_sceneMode = 35;
				else
					_sceneMode = 30;
				break;
			case 3:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 116;
				else
					_sceneMode = 115;
				break;
			default:
				break;
			}
			R2_GLOBALS._player.disableControl();
			_field412 = 3;
			signal();
		}
		break;
	case 3:
		if ( ((_object1.sub51AF8(Common::Point(140, 30)) > 36) || (_object1.sub51AF8(Common::Point(178, 30)) > 36))
			&& ( ((R2_GLOBALS._v56AA2 == 17) && (R2_GLOBALS._v56AA4 == 9) && (_field418 != 3))
				|| ((R2_GLOBALS._v56AA2 == 35) && (R2_GLOBALS._v56AA4 == 17)) )
				)	{
			R2_GLOBALS._player.disableControl();
			_sceneMode = 1202;
			setAction(&_sequenceManager, this, 1202, &_actor1, NULL);
		} else if (_object1.sub51AF8(Common::Point(160, 30)) == 36) {
			switch (_field412 - 1) {
			case 0:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 61;
				else
					_sceneMode = 60;
				break;
			case 1:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 71;
				else
					_sceneMode = 70;
				break;
			case 2:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 96;
				else
					_sceneMode = 95;
				break;
			case 3:
				if (R2_GLOBALS._player._visage == 3155)
					_sceneMode = 45;
				else
					_sceneMode = 40;
				break;
			default:
				_sceneMode = 1;
				R2_GLOBALS._player.setup(3156, 4, 6);
				break;
			}
			R2_GLOBALS._player.disableControl();
			_field412 = 4;
			signal();
		}
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------*/

void AnimationSlice::load(Common::File &f) {
	f.skip(2);
	_sliceOffset = f.readUint16LE();
	f.skip(6);
	_drawMode = f.readByte();
	_secondaryIndex = f.readByte();
}

/*--------------------------------------------------------------------------*/

AnimationSlices::AnimationSlices() {
	_pixelData = NULL;
}

AnimationSlices::~AnimationSlices() {
	delete[] _pixelData;
}

void AnimationSlices::load(Common::File &f) {
	f.skip(4);
	_dataSize = f.readUint32LE();
	f.skip(8);
	_dataSize2 = f.readUint32LE();
	f.skip(28);

	// Load the four slice indexes
	for (int idx = 0; idx < 4; ++idx)
		_slices[idx].load(f);
}

int AnimationSlices::loadPixels(Common::File &f, int slicesSize) {
	delete[] _pixelData;
	_pixelData = new byte[slicesSize];
	return f.read(_pixelData, slicesSize);
}

/*--------------------------------------------------------------------------*/

void AnimationPlayerSubData::load(Common::File &f) {
	uint32 posStart = f.pos();

	f.skip(6);
	_duration = f.readUint32LE();
	_frameRate = f.readUint16LE();
	_framesPerSlices = f.readUint16LE();
	_drawType = f.readUint16LE();
	f.skip(2);
	_sliceSize = f.readUint16LE();
	_ySlices = f.readUint16LE();
	_field16 = f.readUint32LE();
	f.skip(2);
	_palStart = f.readUint16LE();
	_palSize = f.readUint16LE();
	f.read(_palData, 768);
	_totalSize = f.readSint32LE();
	f.skip(12);
	_slices.load(f);

	uint32 posEnd = f.pos();
	assert((posEnd - posStart) == 0x390);
}

/*--------------------------------------------------------------------------*/

AnimationPlayer::AnimationPlayer(): EventHandler() {
	_endAction = NULL;

	_animData1 = NULL;
	_animData2 = NULL;

	_screenBounds = R2_GLOBALS._gfxManagerInstance._bounds;
	_rect1 = R2_GLOBALS._gfxManagerInstance._bounds;
	_paletteMode = ANIMPALMODE_REPLACE_PALETTE;
	_field3A = 1;
	_sliceHeight = 1;
	_field58 = 1;
	_endAction = NULL;
}

AnimationPlayer::~AnimationPlayer() {
	if (!isCompleted())
		close();
}

void AnimationPlayer::synchronize(Serializer &s) {
	EventHandler::synchronize(s);
	warning("TODO AnimationPlayer::load");
}

void AnimationPlayer::remove() {
	if (_endAction)
		_endAction->signal();

	_endAction = NULL;
}

void AnimationPlayer::process(Event &event) {
	if ((event.eventType == EVENT_KEYPRESS) && (event.kbd.keycode == Common::KEYCODE_ESCAPE) &&
			(_field3A)) {
		// Move the current position to the end
		_position = _subData._duration;
	}
}

void AnimationPlayer::dispatch() {
	uint32 gameFrame = R2_GLOBALS._events.getFrameNumber();
	uint32 gameDiff = gameFrame - _gameFrame;

	if (gameDiff >= _frameDelay) {
		drawFrame(_playbackTick % _subData._framesPerSlices);
		++_playbackTick;
		_position = _playbackTick / _subData._framesPerSlices;

		if (_position == _nextSlicesPosition)
			nextSlices();

		_playbackTickPrior = _playbackTick;
		_gameFrame = gameFrame;
	}
}

bool AnimationPlayer::load(int animId, Action *endAction) {
	// Open up the main resource file for access
	TLib &libFile = g_resourceManager->first();
	if (!_resourceFile.open(libFile.getFilename()))
		error("Could not open resource");

	// Get the offset of the given resource and seek to it in the player's file reference
	ResourceEntry entry;
	uint32 fileOffset = libFile.getResourceStart(RES_IMAGE, animId, 0, entry);
	_resourceFile.seek(fileOffset);

	// At this point, the file is pointing to the start of the resource data

	// Set the end action
	_endAction = endAction;

	// Load the sub data block
	_subData.load(_resourceFile);

	// Set other properties
	_playbackTickPrior = -1;
	_playbackTick = 0;

	// The final multiplication is used to deliberately slow down playback, since the original
	// was slowed down by the amount of time spent to decode and display the frames
	_frameDelay = (60 / _subData._frameRate) * 8;
	_gameFrame = R2_GLOBALS._events.getFrameNumber();

	if (_subData._totalSize) {
		_dataNeeded = _subData._totalSize;
	} else {
		int v = (_subData._sliceSize + 2) * _subData._ySlices * _subData._framesPerSlices;
		_dataNeeded = (_subData._field16 / _subData._framesPerSlices) + v + 96;
	}

	debugC(1, ktSageDebugGraphics, "Data needed %d", _dataNeeded);

	// Set up animation data objects
	_animData1 = new AnimationData();
	_sliceCurrent = _animData1;

	if (_subData._framesPerSlices <= 1) {
		_animData2 = NULL;
		_sliceNext = _sliceCurrent;
	} else {
		_animData2 = new AnimationData();
		_sliceNext = _animData2;
	}

	_position = 0;
	_nextSlicesPosition = 1;

	// Load up the first slices set
	_sliceCurrent->_dataSize = _subData._slices._dataSize;
	_sliceCurrent->_slices = _subData._slices;
	int slicesSize = _sliceCurrent->_dataSize - 96;
	int readSize = _sliceCurrent->_slices.loadPixels(_resourceFile, slicesSize);
	_sliceCurrent->_animSlicesSize = readSize + 96;

	if (_sliceNext != _sliceCurrent) {
		getSlices();
	}

	// Handle starting palette
	switch (_paletteMode) {
	case ANIMPALMODE_REPLACE_PALETTE:
		// Use the palette provided with the animation directly
		_palette.getPalette();
		for (int idx = _subData._palStart; idx < (_subData._palStart + _subData._palSize); ++idx) {
			byte r = _subData._palData[idx * 3];
			byte g = _subData._palData[idx * 3 + 1];
			byte b = _subData._palData[idx * 3 + 2];

			R2_GLOBALS._scenePalette.setEntry(idx, r, g, b);
		}

		R2_GLOBALS._sceneManager._hasPalette = true;
		break;
	case ANIMPALMODE_NONE:
		break;

	default:
		// ANIMPALMODE_CURR_PALETTE
		// Use the closest matching colours in the currently active palette to those specified in the animation
		for (int idx = _subData._palStart; idx < (_subData._palStart + _subData._palSize); ++idx) {
			byte r = _subData._palData[idx * 3];
			byte g = _subData._palData[idx * 3 + 1];
			byte b = _subData._palData[idx * 3 + 2];

			int palIndex = R2_GLOBALS._scenePalette.indexOf(r, g, b);
			_palIndexes[idx] = palIndex;
		}
		break;
	}

	++R2_GLOBALS._animationCtr;
	_field38 = 1;
	return true;
}

void AnimationPlayer::drawFrame(int sliceIndex) {
	assert(sliceIndex < 4);
	AnimationSlices &slices = _sliceCurrent->_slices;
	AnimationSlice &slice = _sliceCurrent->_slices._slices[sliceIndex];

	byte *sliceDataStart = &slices._pixelData[slice._sliceOffset - 96];
	byte *sliceData1 = sliceDataStart;

	Rect playerBounds = _screenBounds;
	int y = _screenBounds.top;
	R2_GLOBALS._screenSurface.addDirtyRect(playerBounds);

	Graphics::Surface surface = R2_GLOBALS._screenSurface.lockSurface();

	// Handle different drawing modes
	switch (slice._drawMode) {
	case 0:
		// Draw from uncompressed source
		for (int sliceNum = 0; sliceNum < _subData._ySlices; ++sliceNum) {
			for (int yIndex = 0; yIndex < _sliceHeight; ++yIndex) {
				// TODO: Check of _subData._drawType was done for two different kinds of
				// line slice drawing in original
				const byte *pSrc = (const byte *)sliceDataStart + READ_LE_UINT16(sliceData1 + sliceNum * 2);
				byte *pDest = (byte *)surface.getBasePtr(playerBounds.left, y++);

				Common::copy(pSrc, pSrc + _subData._sliceSize, pDest);
			}
		}
		break;

	case 1:
		switch (slice._secondaryIndex) {
		case 0xfe:
			// Draw from uncompressed source with optional skipped rows
			for (int sliceNum = 0; sliceNum < _subData._ySlices; ++sliceNum) {
				for (int yIndex = 0; yIndex < _sliceHeight; ++yIndex, playerBounds.top++) {
					int offset = READ_LE_UINT16(sliceData1 + sliceNum * 2);

					if (offset) {
						const byte *pSrc = (const byte *)sliceDataStart + offset;
						byte *pDest = (byte *)surface.getBasePtr(playerBounds.left, playerBounds.top);

						//Common::copy(pSrc, pSrc + playerBounds.width(), pDest);
						rleDecode(pSrc, pDest, playerBounds.width());
					}
				}
			}
			break;
		case 0xff:
			// Draw from RLE compressed source
			for (int sliceNum = 0; sliceNum < _subData._ySlices; ++sliceNum) {
				for (int yIndex = 0; yIndex < _sliceHeight; ++yIndex, playerBounds.top++) {
					// TODO: Check of _subData._drawType was done for two different kinds of
					// line slice drawing in original
					const byte *pSrc = (const byte *)sliceDataStart + READ_LE_UINT16(sliceData1 + sliceNum * 2);
					byte *pDest = (byte *)surface.getBasePtr(playerBounds.left, playerBounds.top);

					rleDecode(pSrc, pDest, _subData._sliceSize);
				}
			}
			break;
		default: {
			// Draw from two slice sets simultaneously
			AnimationSlice &slice2 = _sliceCurrent->_slices._slices[slice._secondaryIndex];
			byte *sliceData2 = &slices._pixelData[slice2._sliceOffset - 96];

			for (int sliceNum = 0; sliceNum < _subData._ySlices; ++sliceNum) {
				for (int yIndex = 0; yIndex < _sliceHeight; ++yIndex) {
					const byte *pSrc1 = (const byte *)sliceDataStart + READ_LE_UINT16(sliceData2 + sliceNum * 2);
					const byte *pSrc2 = (const byte *)sliceDataStart + READ_LE_UINT16(sliceData1 + sliceNum * 2);
					byte *pDest = (byte *)surface.getBasePtr(playerBounds.left, y++);

					if (slice2._drawMode == 0) {
						// Uncompressed background, foreground compressed
						Common::copy(pSrc1, pSrc1 + _subData._sliceSize, pDest);
						rleDecode(pSrc2, pDest, _subData._sliceSize);
					} else {
						// Both background and foreground is compressed
						rleDecode(pSrc1, pDest, _subData._sliceSize);
						rleDecode(pSrc2, pDest, _subData._sliceSize);
					}
				}
			}
			break;
		}
		}
	default:
		break;
	}

	// Unlock the screen surface
	R2_GLOBALS._screenSurface.unlockSurface();

	if (_objectMode == 42) {
		_screenBounds.expandPanes();

		// Copy the drawn frame to the back surface
		Rect srcRect = R2_GLOBALS._screenSurface.getBounds();
		Rect destRect = srcRect;
		destRect.translate(-g_globals->_sceneOffset.x, -g_globals->_sceneOffset.y);
		R2_GLOBALS._sceneManager._scene->_backSurface.copyFrom(R2_GLOBALS._screenSurface,
			srcRect, destRect);

		// Draw any objects into the scene
		R2_GLOBALS._sceneObjects->draw();
	} else {
		if (R2_GLOBALS._sceneManager._hasPalette) {
			R2_GLOBALS._sceneManager._hasPalette = false;
			R2_GLOBALS._scenePalette.refresh();
		}
	}
}

/**
 * Read the next frame's slice set
 */
void AnimationPlayer::nextSlices() {
	_position = _nextSlicesPosition++;
	_playbackTick = _position * _subData._framesPerSlices;
	_playbackTickPrior = _playbackTick - 1;

	if (_sliceNext == _sliceCurrent) {
		int dataSize = _sliceCurrent->_slices._dataSize2;
		_sliceCurrent->_dataSize = dataSize;
		debugC(1, ktSageDebugGraphics, "Next frame size = %xh", dataSize);
		if (dataSize == 0)
			return;

		dataSize -= 96;
		assert(dataSize >= 0);
		_sliceCurrent->_slices.load(_resourceFile);
		_sliceCurrent->_animSlicesSize = _sliceCurrent->_slices.loadPixels(_resourceFile, dataSize);
	} else {
		SWAP(_sliceCurrent, _sliceNext);
		getSlices();
	}
}

bool AnimationPlayer::isCompleted() {
	return (_position >= _subData._duration);
}

void AnimationPlayer::close() {
	if (_field38) {
		switch (_paletteMode) {
		case 0:
			R2_GLOBALS._scenePalette.replace(&_palette);
			changePane();
			R2_GLOBALS._sceneManager._hasPalette = true;
			break;
		case 2:
			closing();
			break;
		default:
			changePane();
			break;
		}
	}

	// Close the resource file
	_resourceFile.close();

	if (_objectMode != 42) {
		// flip screen in original
	}

	// Free animation objects
	delete _animData1;
	delete _animData2;
	_animData1 = NULL;
	_animData2 = NULL;

	_field38 = 0;
	if (g_globals != NULL)
		R2_GLOBALS._animationCtr = MAX(R2_GLOBALS._animationCtr, 0);
}

void AnimationPlayer::rleDecode(const byte *pSrc, byte *pDest, int size) {
	while (size > 0) {
		byte v = *pSrc++;
		if (!(v & 0x80)) {
			// Following uncompressed set of bytes
			Common::copy(pSrc, pSrc + v, pDest);
			pSrc += v;
			pDest += v;
			size -= v;
		} else {
			int count = v & 0x3F;
			size -= count;

			if (!(v & 0x40)) {
				// Skip over a number of bytes
				pDest += count;
			} else {
				// Replicate a number of bytes
				Common::fill(pDest, pDest + count, *pSrc++);
				pDest += count;
			}
		}
	}
}

void AnimationPlayer::getSlices() {
	assert((_sliceNext == _animData1) || (_sliceNext == _animData2));
	assert((_sliceCurrent == _animData1) || (_sliceCurrent == _animData2));

	_sliceNext->_dataSize = _sliceCurrent->_slices._dataSize2;
	if (_sliceNext->_dataSize) {
		if (_sliceNext->_dataSize >= _dataNeeded)
			error("Bogus dataNeeded == %d / %d", _sliceNext->_dataSize, _dataNeeded);
	}

	int dataSize = _sliceNext->_dataSize - 96;
	_sliceNext->_slices.load(_resourceFile);
	_sliceNext->_animSlicesSize = _sliceNext->_slices.loadPixels(_resourceFile, dataSize);
}

/*--------------------------------------------------------------------------*/

AnimationPlayerExt::AnimationPlayerExt(): AnimationPlayer() {
	_v = 0;
	_field3A = 0;
}

void AnimationPlayerExt::synchronize(Serializer &s) {
	AnimationPlayer::synchronize(s);
	s.syncAsSint16LE(_v);
}

} // End of namespace Ringworld2

} // End of namespace TsAGE

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
#include "tsage/ringworld/ringworld_logic.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"
#include "tsage/ringworld/ringworld_demo.h"
#include "tsage/ringworld/ringworld_dialogs.h"
#include "tsage/ringworld/ringworld_scenes1.h"
#include "tsage/ringworld/ringworld_scenes2.h"
#include "tsage/ringworld/ringworld_scenes3.h"
#include "tsage/ringworld/ringworld_scenes4.h"
#include "tsage/ringworld/ringworld_scenes5.h"
#include "tsage/ringworld/ringworld_scenes6.h"
#include "tsage/ringworld/ringworld_scenes8.h"
#include "tsage/ringworld/ringworld_scenes10.h"

namespace TsAGE {

namespace Ringworld {

Scene *RingworldGame::createScene(int sceneNumber) {
	switch (sceneNumber) {
	/* Scene group 1 */
	// Kziniti Palace (Introduction)
	case 10: return new Scene10();
	// Outer Space (Introduction)
	case 15: return new Scene15();
	// Cut-scenes for Ch'mee house in distance
	case 20: return new Scene20();
	// Outside Ch'mee residence
	case 30: return new Scene30();
	// Chmeee Home
	case 40: return new Scene40();
	// By Flycycles
	case 50: return new Scene50();
	// Flycycle controls
	case 60: return new Scene60();
	// Shipyard Entrance
	case 90: return new Scene90();
	// Ship Close-up
	case 95: return new Scene95();
	// Sunflower navigation sequence
	case 6100: return new Scene6100();

	/* Scene group 2 */
	// Title screen
	case 1000: return new Scene1000();
	// Fleeing planet cutscene
	case 1001: return new Scene1001();
	// Unused
	case 1250: return new Scene1250();
	// Ringworld Wall
	case 1400: return new Scene1400();
	// Ringworld Space-port
	case 1500: return new Scene1500();

	/* Scene group 3 - Part #1 */
	// Cockpit cutscenes
	case 2000: return new Scene2000();
	// Starcraft - Cockpit
	case 2100: return new Scene2100();
	// Encyclopedia
	case 2120: return new Scene2120();
	// Starcraft - Level 2
	case 2150: return new Scene2150();
	// Starcraft - AutoDoc
	case 2200: return new Scene2200();
	// Stasis Field Map
	case 2222: return new Scene2222();
	// Starcraft - Quinn's Room
	case 2230: return new Scene2230();

	/* Scene group 3 - Part #2 */
	// Starcraft - Storage Room
	case 2280: return new Scene2280();
	// Starcraft - Hanger Bay
	case 2300: return new Scene2300();
	// Starcraft - Copy Protection Screen
	case 2310: return new Scene2310();
	// Starcraft - Lander Bay
	case 2320: return new Scene2320();
	// Scene 2400 - Descending in Lander
	case 2400: return new Scene2400();

	/* Scene group 4 */
	// Ringworld Scan
	case 3500: return new Scene3500();
	// Remote Viewer
	case 3700: return new Scene3700();

	/* Scene group 5 */
	// Village
	case 4000: return new Scene4000();
	// Village - Outside Lander
	case 4010: return new Scene4010();
	// Village - Puzzle Board
	case 4025: return new Scene4025();
	// Village - Temple Antechamber
	case 4045: return new Scene4045();
	// Village - Temple
	case 4050: return new Scene4050();
	// Village - Hut
	case 4100: return new Scene4100();
	// Village - Bedroom
	case 4150: return new Scene4150();
	// Village - Near Slaver Ship
	case 4250: return new Scene4250();
	// Village - Slaver Ship
	case 4300: return new Scene4300();
	// Village - Slaver Ship Keypad
	case 4301: return new Scene4301();

	/* Scene group 6 */
	// Caverns - Entrance
	case 5000: return new Scene5000();
	// Caverns
	case 5100: return new Scene5100();
	// Caverns - Throne-room
	case 5200: return new Scene5200();
	// Caverns - Pit
	case 5300: return new Scene5300();

	/* Scene group 8 */
	// Landing near beach
	case 7000: return new Scene7000();
	// Underwater: swimming
	case 7100: return new Scene7100();
	// Underwater: Entering the cave
	case 7200: return new Scene7200();
	// Underwater: Lord Poria
	case 7300: return new Scene7300();
	// Floating Buildings: Outside
	case 7600: return new Scene7600();
	// Floating Buildings: In the lab
	case 7700: return new Scene7700();

	/* Scene group 10 */
	// Near beach: Slave washing clothes
	case 9100: return new Scene9100();
	// Castle: Outside the bulwarks
	case 9150: return new Scene9150();
	// Castle: Near the fountain
	case 9200: return new Scene9200();
	// Castle: In front of a large guarded door
	case 9300: return new Scene9300();
	// Castle: In a hallway
	case 9350: return new Scene9350();
	// Castle: In a hallway
	case 9360: return new Scene9360();
	// Castle: Black-Smith room
	case 9400: return new Scene9400();
	// Castle: Dining room
	case 9450: return new Scene9450();
	// Castle: Bedroom
	case 9500: return new Scene9500();
	// Castle: Balcony
	case 9700: return new Scene9700();
	// Castle: In the garden
	case 9750: return new Scene9750();
	// Castle: Dressing room
	case 9850: return new Scene9850();
	// Ending
	case 9900: return new Scene9900();
	// Space travel
	case 9999: return new Scene9999();

	default:
		error("Unknown scene number - %d", sceneNumber);
		break;
	}
}

/**
 * Returns true if it is currently okay to restore a game
 */
bool RingworldGame::canLoadGameStateCurrently() {
	// Don't allow a game to be loaded if a dialog is active
	return !g_globals->getFlag(50) && (g_globals->_gfxManagers.size() == 1);

}

/**
 * Returns true if it is currently okay to save the game
 */
bool RingworldGame::canSaveGameStateCurrently() {
	// Don't allow a game to be saved if a dialog is active
	return !g_globals->getFlag(50) && (g_globals->_gfxManagers.size() == 1);
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

SceneArea::SceneArea() {
	_savedArea = NULL;
	_pt.x = _pt.y = 0;
}

SceneArea::~SceneArea() {
	delete _savedArea;
}

void SceneArea::setup(int resNum, int rlbNum, int subNum, int actionId) {
	_resNum = resNum;
	_rlbNum = rlbNum;
	_subNum = subNum;
	_actionId = actionId;

	_surface = surfaceFromRes(resNum, rlbNum, subNum);
}

void SceneArea::draw2() {
	_surface.draw(Common::Point(_bounds.left, _bounds.top));
}

void SceneArea::display() {
	_bounds.left = _pt.x - (_surface.getBounds().width() / 2);
	_bounds.top = _pt.y + 1 - _surface.getBounds().height();
	_bounds.setWidth(_surface.getBounds().width());
	_bounds.setHeight(_surface.getBounds().height());

	_savedArea = surfaceGetArea(g_globals->_gfxManagerInstance.getSurface(), _bounds);
	draw2();
}

void SceneArea::restore() {
	assert(_savedArea);
	_savedArea->draw(Common::Point(_bounds.left, _bounds.top));
	delete _savedArea;
	_savedArea = NULL;
}

void SceneArea::draw(bool flag) {
	_surface = surfaceFromRes(_resNum, _rlbNum, flag ? _subNum + 1 : _subNum);
	_surface.draw(Common::Point(_bounds.left, _bounds.top));
}

void SceneArea::wait() {
	// Wait until a mouse or keypress
	Event event;
	while (!g_vm->shouldQuit() && !g_globals->_events.getEvent(event)) {
		GLOBALS._screenSurface.updateScreen();
		g_system->delayMillis(10);
	}

	SynchronizedList<SceneItem *>::iterator ii;
	for (ii = g_globals->_sceneItems.begin(); ii != g_globals->_sceneItems.end(); ++ii) {
		SceneItem *sceneItem = *ii;
		if (sceneItem->contains(event.mousePos)) {
			sceneItem->doAction(_actionId);
			break;
		}
	}

	g_globals->_events.setCursor(CURSOR_ARROW);
}

void SceneArea::synchronize(Serializer &s) {
	if (s.getVersion() >= 2)
		SavedObject::synchronize(s);

	s.syncAsSint16LE(_pt.x);
	s.syncAsSint16LE(_pt.y);
	s.syncAsSint32LE(_resNum);
	s.syncAsSint32LE(_rlbNum);
	s.syncAsSint32LE(_subNum);
	s.syncAsSint32LE(_actionId);
	_bounds.synchronize(s);
}

/*--------------------------------------------------------------------------*/

RingworldInvObjectList::RingworldInvObjectList() :
		_stunner(2280, 1, 2, OBJECT_STUNNER, "This is your stunner."),
		_scanner(1, 1, 3, OBJECT_SCANNER, "A combination scanner comm unit."),
		_stasisBox(5200, 1, 4, OBJECT_STASIS_BOX, "A stasis box."),
		_infoDisk(40, 1, 1, OBJECT_INFODISK, "The infodisk you took from the assassin."),
		_stasisNegator(0, 2, 2, OBJECT_STASIS_NEGATOR, "The stasis field negator."),
		_keyDevice(4250, 1, 6, OBJECT_KEY_DEVICE, "A magnetic key device."),
		_medkit(2280, 1, 7, OBJECT_MEDKIT,  "Your medkit."),
		_ladder(4100, 1, 8, OBJECT_LADDER, "The chief's ladder."),
		_rope(4150, 1, 9, OBJECT_ROPE, "The chief's rope."),
		_key(7700, 1, 11, OBJECT_KEY, "A key."),
		_translator(7700, 1, 13, OBJECT_TRANSLATOR,  "The dolphin translator box."),
		_ale(2150, 1, 10, OBJECT_ALE, "A bottle of ale."),
		_paper(7700, 1, 12, OBJECT_PAPER, "A slip of paper with the numbers 2,4, and 3 written on it."),
		_waldos(0, 1, 14, OBJECT_WALDOS, "A pair of waldos from the ruined probe."),
		_stasisBox2(8100, 1, 4, OBJECT_STASIS_BOX2, "A stasis box."),
		_ring(8100, 2, 5, OBJECT_RING, "This is a signet ring sent to you by Louis Wu."),
		_cloak(9850, 2, 6, OBJECT_CLOAK, "A fine silk cloak."),
		_tunic(9450, 2, 7, OBJECT_TUNIC, "The patriarch's soiled tunic."),
		_candle(9500, 2, 8, OBJECT_CANDLE, "A tallow candle."),
		_straw(9400, 2, 9, OBJECT_STRAW, "Clean, dry straw."),
		_scimitar(9850, 1, 18, OBJECT_SCIMITAR, "A scimitar from the Patriarch's closet."),
		_sword(9850, 1, 17, OBJECT_SWORD, "A short sword from the Patriarch's closet."),
		_helmet(9500, 2, 4, OBJECT_HELMET, "Some type of helmet."),
		_items(4300, 2, 10, OBJECT_ITEMS, "Two interesting items from the Tnuctipun vessel."),
		_concentrator(4300, 2, 11, OBJECT_CONCENTRATOR, "The Tnuctipun anti-matter concentrator contained in a stasis field."),
		_nullifier(5200, 2, 12, OBJECT_NULLIFIER, "A purported neural wave nullifier."),
		_peg(4045, 2, 16, OBJECT_PEG, "A peg with a symbol."),
		_vial(5100, 2, 17, OBJECT_VIAL, "A vial of the bat creatures anti-pheromone drug."),
		_jacket(9850, 3, 1, OBJECT_JACKET, "A natty padded jacket."),
		_tunic2(9850, 3, 2, OBJECT_TUNIC2, "A very hairy tunic."),
		_bone(5300, 3, 5, OBJECT_BONE, "A very sharp bone."),
		_jar(7700, 3, 4, OBJECT_JAR, "An jar filled with a green substance."),
		_emptyJar(7700, 3, 3, OBJECT_EMPTY_JAR, "An empty jar.") {

	// Add the items to the list
	_itemList.push_back(&_stunner);
	_itemList.push_back(&_scanner);
	_itemList.push_back(&_stasisBox);
	_itemList.push_back(&_infoDisk);
	_itemList.push_back(&_stasisNegator);
	_itemList.push_back(&_keyDevice);
	_itemList.push_back(&_medkit);
	_itemList.push_back(&_ladder);
	_itemList.push_back(&_rope);
	_itemList.push_back(&_key);
	_itemList.push_back(&_translator);
	_itemList.push_back(&_ale);
	_itemList.push_back(&_paper);
	_itemList.push_back(&_waldos);
	_itemList.push_back(&_stasisBox2);
	_itemList.push_back(&_ring);
	_itemList.push_back(&_cloak);
	_itemList.push_back(&_tunic);
	_itemList.push_back(&_candle);
	_itemList.push_back(&_straw);
	_itemList.push_back(&_scimitar);
	_itemList.push_back(&_sword);
	_itemList.push_back(&_helmet);
	_itemList.push_back(&_items);
	_itemList.push_back(&_concentrator);
	_itemList.push_back(&_nullifier);
	_itemList.push_back(&_peg);
	_itemList.push_back(&_vial);
	_itemList.push_back(&_jacket);
	_itemList.push_back(&_tunic2);
	_itemList.push_back(&_bone);
	_itemList.push_back(&_jar);
	_itemList.push_back(&_emptyJar);

	_selectedItem = NULL;
}

/*--------------------------------------------------------------------------*/

void RingworldGame::start() {
	// Set some default flags
	g_globals->setFlag(12);
	g_globals->setFlag(34);

	// Set the screen to scroll in response to the player moving off-screen
	g_globals->_scrollFollower = &g_globals->_player;

	// Set the object's that will be in the player's inventory by default
	RING_INVENTORY._stunner._sceneNumber = 1;
	RING_INVENTORY._scanner._sceneNumber = 1;
	RING_INVENTORY._ring._sceneNumber = 1;

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
		g_globals->_sceneHandler->_loadGameSlot = slot;
	else
		// Switch to the title screen
		g_globals->_sceneManager.setNewScene(1000);

	g_globals->_events.showCursor();
}

void RingworldGame::restart() {
	g_globals->_scenePalette.clearListeners();
	g_globals->_soundHandler.stop();

	// Reset the flags
	g_globals->reset();
	g_globals->setFlag(34);

	// Clear save/load slots
	g_globals->_sceneHandler->_saveGameSlot = -1;
	g_globals->_sceneHandler->_loadGameSlot = -1;

	g_globals->_stripNum = 0;
	g_globals->_events.setCursor(CURSOR_WALK);

	// Reset item properties
	RING_INVENTORY._stunner._sceneNumber = 1;
	RING_INVENTORY._scanner._sceneNumber = 1;
	RING_INVENTORY._stasisBox._sceneNumber = 5200;
	RING_INVENTORY._infoDisk._sceneNumber = 40;
	RING_INVENTORY._stasisNegator._sceneNumber = 0;
	RING_INVENTORY._keyDevice._sceneNumber = 0;
	RING_INVENTORY._medkit._sceneNumber = 2280;
	RING_INVENTORY._ladder._sceneNumber = 4100;
	RING_INVENTORY._rope._sceneNumber = 4150;
	RING_INVENTORY._key._sceneNumber = 7700;
	RING_INVENTORY._translator._sceneNumber = 2150;
	RING_INVENTORY._paper._sceneNumber = 7700;
	RING_INVENTORY._waldos._sceneNumber = 0;
	RING_INVENTORY._ring._sceneNumber = 1;
	RING_INVENTORY._stasisBox2._sceneNumber = 8100;
	RING_INVENTORY._cloak._sceneNumber = 9850;
	RING_INVENTORY._tunic._sceneNumber = 9450;
	RING_INVENTORY._candle._sceneNumber = 9500;
	RING_INVENTORY._straw._sceneNumber = 9400;
	RING_INVENTORY._scimitar._sceneNumber = 9850;
	RING_INVENTORY._sword._sceneNumber = 9850;
	RING_INVENTORY._helmet._sceneNumber = 9500;
	RING_INVENTORY._items._sceneNumber = 4300;
	RING_INVENTORY._concentrator._sceneNumber = 4300;
	RING_INVENTORY._nullifier._sceneNumber = 4300;
	RING_INVENTORY._peg._sceneNumber = 4045;
	RING_INVENTORY._vial._sceneNumber = 5100;
	RING_INVENTORY._jacket._sceneNumber = 9850;
	RING_INVENTORY._tunic2._sceneNumber = 9850;
	RING_INVENTORY._bone._sceneNumber = 5300;
	RING_INVENTORY._jar._sceneNumber = 7700;
	RING_INVENTORY._emptyJar._sceneNumber = 7700;
	RING_INVENTORY._selectedItem = NULL;

	// Change to the first game scene
	g_globals->_sceneManager.changeScene(30);
}

void RingworldGame::endGame(int resNum, int lineNum) {
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

void RingworldGame::processEvent(Event &event) {
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

void RingworldGame::rightClick() {
	RightClickDialog *dlg = new RightClickDialog();
	dlg->execute();
	delete dlg;
}

/*--------------------------------------------------------------------------*/

NamedHotspot::NamedHotspot() : SceneHotspot() {
	_resNum = 0;
	_lookLineNum = _useLineNum = _talkLineNum = -1;
}

void NamedHotspot::doAction(int action) {
	switch (action) {
	case CURSOR_WALK:
		// Nothing
		return;
	case CURSOR_LOOK:
		if (_lookLineNum == -1)
			break;

		SceneItem::display(_resNum, _lookLineNum, SET_Y, 20, SET_WIDTH, 200, SET_EXT_BGCOLOR, 7, LIST_END);
		return;
	case CURSOR_USE:
		if (_useLineNum == -1)
			break;

		SceneItem::display(_resNum, _useLineNum, SET_Y, 20, SET_WIDTH, 200, SET_EXT_BGCOLOR, 7, LIST_END);
		return;
	case CURSOR_TALK:
		if (_talkLineNum == -1)
			break;

		SceneItem::display(_resNum, _lookLineNum, SET_Y, 20, SET_WIDTH, 200, SET_EXT_BGCOLOR, 7, LIST_END);
		return;
	default:
		break;
	}

	SceneHotspot::doAction(action);
}

void NamedHotspot::synchronize(Serializer &s) {
	SceneHotspot::synchronize(s);
	s.syncAsSint16LE(_resNum);
	s.syncAsSint16LE(_lookLineNum);
	s.syncAsSint16LE(_useLineNum);

	if (g_vm->getGameID() == GType_BlueForce)
		s.syncAsSint16LE(_talkLineNum);
}


} // End of namespace Ringworld

} // End of namespace TsAGE

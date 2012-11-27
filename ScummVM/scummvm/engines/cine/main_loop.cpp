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


#include "common/scummsys.h"
#include "common/events.h"
#include "common/system.h"

#include "cine/main_loop.h"
#include "cine/object.h"
#include "cine/various.h"
#include "cine/bg_list.h"
#include "cine/sound.h"

namespace Cine {

struct MouseStatusStruct {
	int left;
	int right;
};

MouseStatusStruct mouseData;

uint16 mouseRight = 0;
uint16 mouseLeft = 0;

int lastKeyStroke = 0;

uint16 mouseUpdateStatus;
uint16 dummyU16;

static void processEvent(Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN:
		mouseLeft = 1;
		break;
	case Common::EVENT_RBUTTONDOWN:
		mouseRight = 1;
		break;
	case Common::EVENT_LBUTTONUP:
		mouseLeft = 0;
		break;
	case Common::EVENT_RBUTTONUP:
		mouseRight = 0;
		break;
	case Common::EVENT_MOUSEMOVE:
		break;
	case Common::EVENT_KEYDOWN:
		switch (event.kbd.keycode) {
		case Common::KEYCODE_RETURN:
		case Common::KEYCODE_KP_ENTER:
		case Common::KEYCODE_KP5:
			if (allowPlayerInput) {
				mouseLeft = 1;
			}
			break;
		case Common::KEYCODE_ESCAPE:
			if (allowPlayerInput) {
				mouseRight = 1;
			}
			break;
		case Common::KEYCODE_F1:
			if (allowPlayerInput) {
				playerCommand = 0; // EXAMINE
				makeCommandLine();
			}
			break;
		case Common::KEYCODE_F2:
			if (allowPlayerInput) {
				playerCommand = 1; // TAKE
				makeCommandLine();
			}
			break;
		case Common::KEYCODE_F3:
			if (allowPlayerInput) {
				playerCommand = 2; // INVENTORY
				makeCommandLine();
			}
			break;
		case Common::KEYCODE_F4:
			if (allowPlayerInput) {
				playerCommand = 3; // USE
				makeCommandLine();
			}
			break;
		case Common::KEYCODE_F5:
			if (allowPlayerInput) {
				playerCommand = 4; // ACTIVATE
				makeCommandLine();
			}
			break;
		case Common::KEYCODE_F6:
			if (allowPlayerInput) {
				playerCommand = 5; // SPEAK
				makeCommandLine();
			}
			break;
		case Common::KEYCODE_F9:
			if (allowPlayerInput && !inMenu) {
				makeActionMenu();
				makeCommandLine();
			}
			break;
		case Common::KEYCODE_F10:
			if (!inMenu) {
				g_cine->makeSystemMenu();
			}
			break;
		case Common::KEYCODE_F11:
			renderer->showCollisionPage(true);
			break;
		case Common::KEYCODE_MINUS:
		case Common::KEYCODE_KP_MINUS:
			g_cine->modifyGameSpeed(-1); // Slower
			break;
		case Common::KEYCODE_PLUS:
		case Common::KEYCODE_KP_PLUS:
			g_cine->modifyGameSpeed(+1); // Faster
			break;
		case Common::KEYCODE_LEFT:
		case Common::KEYCODE_KP4:
			moveUsingKeyboard(-1, 0); // Left
			break;
		case Common::KEYCODE_RIGHT:
		case Common::KEYCODE_KP6:
			moveUsingKeyboard(+1, 0); // Right
			break;
		case Common::KEYCODE_UP:
		case Common::KEYCODE_KP8:
			moveUsingKeyboard(0, +1); // Up
			break;
		case Common::KEYCODE_DOWN:
		case Common::KEYCODE_KP2:
			moveUsingKeyboard(0, -1); // Down
			break;
		case Common::KEYCODE_KP9:
			moveUsingKeyboard(+1, +1); // Up & Right
			break;
		case Common::KEYCODE_KP7:
			moveUsingKeyboard(-1, +1); // Up & Left
			break;
		case Common::KEYCODE_KP1:
			moveUsingKeyboard(-1, -1); // Down & Left
			break;
		case Common::KEYCODE_KP3:
			moveUsingKeyboard(+1, -1); // Down & Right
			break;
		case Common::KEYCODE_d:
			if (event.kbd.hasFlags(Common::KBD_CTRL)) {
				g_cine->getDebugger()->attach();
				g_cine->getDebugger()->onFrame();
			}
			// No Break to allow fallthrough to process 'd' without CTRL
		default:
			lastKeyStroke = event.kbd.keycode;
			break;
		}
		break;
	case Common::EVENT_KEYUP:
		switch (event.kbd.keycode) {
		case Common::KEYCODE_F11:
			renderer->showCollisionPage(false);
			break;
		case Common::KEYCODE_KP5:   // Emulated left mouse button click
		case Common::KEYCODE_LEFT:  // Left
		case Common::KEYCODE_KP4:   // Left
		case Common::KEYCODE_RIGHT: // Right
		case Common::KEYCODE_KP6:   // Right
		case Common::KEYCODE_UP:    // Up
		case Common::KEYCODE_KP8:   // Up
		case Common::KEYCODE_DOWN:  // Down
		case Common::KEYCODE_KP2:   // Down
		case Common::KEYCODE_KP9:   // Up & Right
		case Common::KEYCODE_KP7:   // Up & Left
		case Common::KEYCODE_KP1:   // Down & Left
		case Common::KEYCODE_KP3:   // Down & Right
			// Stop ego movement made with keyboard when releasing a known key
			moveUsingKeyboard(0, 0);
			break;
		default:
			break;
		}
	default:
		break;
	}
}

void manageEvents() {
	Common::EventManager *eventMan = g_system->getEventManager();

	uint32 nextFrame = g_system->getMillis() + g_cine->getTimerDelay();
	do {
		Common::Event event;
		while (eventMan->pollEvent(event)) {
			processEvent(event);
		}
		g_system->updateScreen();
		g_system->delayMillis(20);
	} while (g_system->getMillis() < nextFrame);

	mouseData.left = mouseLeft;
	mouseData.right = mouseRight;
}

void getMouseData(uint16 param, uint16 *pButton, uint16 *pX, uint16 *pY) {
	Common::Point mouse = g_system->getEventManager()->getMousePos();
	*pX = mouse.x;
	*pY = mouse.y;

	*pButton = 0;

	if (mouseData.right) {
		(*pButton) |= 2;
	}

	if (mouseData.left) {
		(*pButton) |= 1;
	}
}

int getKeyData() {
	int k = lastKeyStroke;

	lastKeyStroke = -1;

	return k;
}

/** Removes elements from seqList that have their member variable var4 set to value -1. */
void purgeSeqList() {
	Common::List<SeqListElement>::iterator it = g_cine->_seqList.begin();
	while (it != g_cine->_seqList.end()) {
		if (it->var4 == -1) {
			// Erase the element and jump to the next element
			it = g_cine->_seqList.erase(it);
		} else {
			// Let the element be and jump to the next element
			it++;
		}
	}
}

void CineEngine::mainLoop(int bootScriptIdx) {
	bool playerAction;
	byte di;
	uint16 mouseButton;

	if (_preLoad == false) {
		resetBgIncrustList();

		setTextWindow(0, 0, 20, 200);

		errorVar = 0;

		addScriptToGlobalScripts(bootScriptIdx);

		menuVar = 0;

//		gfxRedrawPage(page0c, page0, page0c, page0, -1);
//		gfxWaitVBL();
//		gfxRedrawMouseCursor();

		inMenu = false;
		allowPlayerInput = 0;
		checkForPendingDataLoadSwitch = 0;

		fadeRequired = false;
		isDrawCommandEnabled = 0;
		waitForPlayerClick = 0;
		menuCommandLen = 0;

		playerCommand = -1;
		g_cine->_commandBuffer = "";

		g_cine->_globalVars[VAR_MOUSE_X_POS] = 0;
		g_cine->_globalVars[VAR_MOUSE_Y_POS] = 0;
		if (g_cine->getGameType() == Cine::GType_OS) {
			g_cine->_globalVars[VAR_MOUSE_X_POS_2ND] = 0;
			g_cine->_globalVars[VAR_MOUSE_Y_POS_2ND] = 0;
			g_cine->_globalVars[VAR_BYPASS_PROTECTION] = 0; // set to 1 to bypass the copy protection
			g_cine->_globalVars[VAR_LOW_MEMORY] = 0; // set to 1 to disable some animations, sounds etc.
		}

		strcpy(newPrcName, "");
		strcpy(newRelName, "");
		strcpy(newObjectName, "");
		strcpy(newMsgName, "");
		strcpy(currentCtName, "");
		strcpy(currentPartName, "");

		g_sound->stopMusic();
	}

	do {
		// HACK: Force amount of oxygen left to maximum during Operation Stealth's first arcade sequence.
		//       This makes it possible to pass the arcade sequence for now.
		// FIXME: Remove the hack and make the first arcade sequence normally playable.
		/*
		if (g_cine->getGameType() == Cine::GType_OS) {
			Common::String bgName(renderer->getBgName());
			// Check if the background is one of the three backgrounds
			// that are only used during the first arcade sequence.
			if (bgName == "28.PI1" || bgName == "29.PI1" || bgName == "30.PI1") {
				static const uint oxygenObjNum = 202, maxOxygen = 264;
				// Force the amount of oxygen left to the maximum.
				g_cine->_objectTable[oxygenObjNum].x = maxOxygen;
			}
		}*/

		// HACK: In Operation Stealth after the first arcade sequence jump player's position to avoid getting stuck.
		// After the first arcade sequence the player comes up stairs from
		// the water in Santa Paragua's downtown in front of the flower shop.
		// Previously he was completely stuck after getting up the stairs.
		// If the background is the one used in the flower shop scene ("21.PI1")
		// and the player is at the exact location after getting up the stairs
		// then we just nudge him a tiny bit away from the stairs and voila, he's free!
		// Maybe the real problem behind all this is collision data related as it looks
		// like there's some boundary right there near position (204, 110) which we can
		// jump over by moving the character to (204, 109). The script handling the
		// flower shop scene is AIRPORT.PRC's 13th script.
		// FIXME: Remove the hack and solve what's really causing the problem in the first place.
		if (g_cine->getGameType() == Cine::GType_OS) {
			if (scumm_stricmp(renderer->getBgName(), "21.PI1") == 0 && g_cine->_objectTable[1].x == 204 && g_cine->_objectTable[1].y == 110) {
				g_cine->_objectTable[1].y--; // Move the player character upward on-screen by one pixel
			}
		}

		stopMusicAfterFadeOut();
		di = executePlayerInput();

		// Clear the zoneQuery table (Operation Stealth specific)
		if (g_cine->getGameType() == Cine::GType_OS) {
			Common::fill(g_cine->_zoneQuery.begin(), g_cine->_zoneQuery.end(), 0);
		}

		if (g_cine->getGameType() == Cine::GType_OS) {
			processSeqList();
		}
		executeObjectScripts();
		executeGlobalScripts();

		purgeObjectScripts();
		purgeGlobalScripts();
		if (g_cine->getGameType() == Cine::GType_OS) {
			purgeSeqList();
		}

		if (playerCommand == -1) {
			setMouseCursor(MOUSE_CURSOR_NORMAL);
		} else {
			setMouseCursor(MOUSE_CURSOR_CROSS);
		}

		if (renderer->ready()) {
			renderer->drawFrame();
		}

		// NOTE: In the original Future Wars and Operation Stealth messages
		// were removed when running the drawOverlays function which is
		// currently called from the renderer's drawFrame function.
		removeMessages();

		if (waitForPlayerClick) {
			playerAction = false;

			_messageLen <<= 3;
			if (_messageLen < 800)
				_messageLen = 800;

			do {
				manageEvents();
				getMouseData(mouseUpdateStatus, &mouseButton, &dummyU16, &dummyU16);
			} while (mouseButton != 0 && !shouldQuit());

			menuVar = 0;

			do {
				manageEvents();
				getMouseData(mouseUpdateStatus, &mouseButton, &dummyU16, &dummyU16);
				playerAction = (mouseButton != 0) || processKeyboard(menuVar);
				mainLoopSub6();
			} while (!playerAction && !shouldQuit());

			menuVar = 0;

			do {
				manageEvents();
				getMouseData(mouseUpdateStatus, &mouseButton, &dummyU16, &dummyU16);
			} while (mouseButton != 0 && !shouldQuit());

			waitForPlayerClick = 0;
		}

		if (checkForPendingDataLoadSwitch) {
			checkForPendingDataLoad();

			checkForPendingDataLoadSwitch = 0;
		}

		if (di) {
			if ("quit"[menuCommandLen] == (char)di) {
				++menuCommandLen;
				if (menuCommandLen == 4) {
					quitGame();
				}
			} else {
				menuCommandLen = 0;
			}
		}

		manageEvents();

	} while (!shouldQuit() && !_restartRequested);

	hideMouse();
	g_sound->stopMusic();
	//if (g_cine->getGameType() == Cine::GType_OS) {
	//	freeUnkList();
	//}
	closePart();
}

} // End of namespace Cine

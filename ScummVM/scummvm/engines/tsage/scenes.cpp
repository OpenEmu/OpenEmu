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
#include "common/translation.h"
#include "gui/saveload.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"
#include "tsage/ringworld/ringworld_logic.h"
#include "tsage/tsage.h"
#include "tsage/saveload.h"
#include "tsage/staticres.h"

namespace TsAGE {

SceneManager::SceneManager() {
	_scene = NULL;
	_hasPalette = false;
	_sceneNumber = -1;
	_nextSceneNumber = -1;
	_previousScene = 0;
	_fadeMode = FADEMODE_GRADUAL;
	_scrollerRect = Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	g_saver->addListener(this);
	_objectCount = 0;
	_loadMode = 0;
}

SceneManager::~SceneManager() {
	delete _scene;
}

void SceneManager::setNewScene(int sceneNumber) {
	debug(1, "SetNewScene(%d)", sceneNumber);
	_nextSceneNumber = sceneNumber;
}

void SceneManager::checkScene() {
	if (_nextSceneNumber != -1) {
		sceneChange();
		_nextSceneNumber = -1;
	}

	g_globals->dispatchSounds();
}

void SceneManager::sceneChange() {
	int activeScreenNumber = 0;

	// Handle removing the scene
	if (_scene) {
		activeScreenNumber = _scene->_activeScreenNumber;
		_scene->remove();
	}

	// Clear the scene objects
	SynchronizedList<SceneObject *>::iterator io = g_globals->_sceneObjects->begin();
	while (io != g_globals->_sceneObjects->end()) {
		SceneObject *sceneObj = *io;
		++io;
		sceneObj->removeObject();
	}

	// Clear the hotspot list
	SynchronizedList<SceneItem *>::iterator ii = g_globals->_sceneItems.begin();
	while (ii != g_globals->_sceneItems.end()) {
		SceneItem *sceneItem = *ii;
		++ii;
		sceneItem->remove();
	}

	// TODO: Clear _list_45BAA list

	// If there is an active scene, deactivate it
	if (_scene) {
		_previousScene = _sceneNumber;

		delete _scene;
		_scene = NULL;
		_sceneNumber = -1;
	}

	// Set the next scene to be active
	_sceneNumber = _nextSceneNumber;

	// Free any regions
	disposeRegions();

	// Ensure that the same number of objects are registered now as when the scene started
	if (_objectCount > 0) {
		assert(_objectCount == g_saver->getObjectCount());
	}
	_objectCount = g_saver->getObjectCount();
	g_globals->_sceneHandler->_delayTicks = 2;

	// Instantiate and set the new scene
	_scene = getNewScene();

	if (!g_saver->getMacroRestoreFlag())
		_scene->postInit();
	else
		_scene->loadScene(activeScreenNumber);
}

Scene *SceneManager::getNewScene() {
	return g_globals->_game->createScene(_nextSceneNumber);
}

void SceneManager::fadeInIfNecessary() {
	if (_hasPalette) {
		uint32 adjustData = 0;
		for (int percent = 0; percent < 100; percent += 5) {
			if (g_globals->_sceneManager._fadeMode == FADEMODE_IMMEDIATE)
				percent = 100;

			g_globals->_scenePalette.fade((const byte *)&adjustData, false, percent);
			GLOBALS._screenSurface.updateScreen();
			g_system->delayMillis(10);
		}

		g_globals->_scenePalette.refresh();
		_hasPalette = false;
	}
}

void SceneManager::changeScene(int newSceneNumber) {
	debug(1, "changeScene(%d)", newSceneNumber);
	// Fade out the scene
	ScenePalette scenePalette;
	uint32 adjustData = 0;
	g_globals->_scenePalette.clearListeners();
	scenePalette.getPalette();

	for (int percent = 100; percent >= 0; percent -= 5) {
		scenePalette.fade((byte *)&adjustData, false, percent);
		g_system->delayMillis(10);
	}

	// Stop any objects that were animating
	SynchronizedList<SceneObject *>::iterator i;
	for (i = g_globals->_sceneObjects->begin(); i != g_globals->_sceneObjects->end(); ++i) {
		SceneObject *sceneObj = *i;
		Common::Point pt(0, 0);
		sceneObj->addMover(NULL, &pt);
		sceneObj->setObjectWrapper(NULL);
		sceneObj->animate(ANIM_MODE_NONE, 0);

		sceneObj->_flags &= ~OBJFLAG_PANES;
	}

	// Blank out the screen
	g_globals->_screenSurface.fillRect(g_globals->_screenSurface.getBounds(), 0);

	// If there are any fading sounds, wait until fading is complete
	while (g_globals->_soundManager.isFading()) {
		g_system->delayMillis(10);
	}

	// Set the new scene to be loaded
	setNewScene(newSceneNumber);
}

void SceneManager::setup() {
	g_saver->addLoadNotifier(SceneManager::loadNotifier);
	setBackSurface();
}

void SceneManager::setBackSurface() {
	int size = g_globals->_sceneManager._scene->_backgroundBounds.width() *
		g_globals->_sceneManager._scene->_backgroundBounds.height();

	if (size > 96000) {
		if (g_globals->_sceneManager._scene->_backgroundBounds.width() <= SCREEN_WIDTH) {
			// Standard size creation
			g_globals->_sceneManager._scene->_backSurface.create(SCREEN_WIDTH, SCREEN_HEIGHT * 3 / 2);
			g_globals->_sceneManager._scrollerRect = Rect(0, 30, SCREEN_WIDTH, SCREEN_HEIGHT - 30);
		} else {
			// Wide screen needs extra space to allow for scrolling
			g_globals->_sceneManager._scene->_backSurface.create(SCREEN_WIDTH * 3 / 2, SCREEN_HEIGHT);
			g_globals->_sceneManager._scrollerRect = Rect(80, 0, SCREEN_WIDTH - 80, SCREEN_HEIGHT);
		}
	} else {
		g_globals->_sceneManager._scene->_backSurface.create(
			g_globals->_sceneManager._scene->_backgroundBounds.width(),
			g_globals->_sceneManager._scene->_backgroundBounds.height()
		);
		g_globals->_sceneManager._scrollerRect = Rect(80, 20, SCREEN_WIDTH - 80, SCREEN_HEIGHT - 20);
	}
}

void SceneManager::saveListener(int saveMode) {
}

void SceneManager::loadNotifier(bool postFlag) {
	if (postFlag) {
		if (g_globals->_sceneManager._scene->_activeScreenNumber != -1)
			g_globals->_sceneManager._scene->loadSceneData(g_globals->_sceneManager._scene->_activeScreenNumber);
		g_globals->_sceneManager._hasPalette = true;
	}
}

void SceneManager::setBgOffset(const Common::Point &pt, int loadCount) {
	_sceneBgOffset = pt;
	_sceneLoadCount = loadCount;
}

void SceneManager::listenerSynchronize(Serializer &s) {
	s.validate("SceneManager");

	if (s.isLoading() && !g_globals->_sceneManager._scene)
		// Loading a savegame straight from the launcher, so instantiate a blank placeholder scene
		// in order for the savegame loading to work correctly
		g_globals->_sceneManager._scene = new Scene();

	// Depreciated: the background scene objects used to be located here
	uint32 unused = 0;
	s.syncAsUint32LE(unused);

	s.syncAsSint32LE(_sceneNumber);
	s.syncAsUint16LE(g_globals->_sceneManager._scene->_activeScreenNumber);

	if (s.isLoading()) {
		changeScene(_sceneNumber);

		if (_nextSceneNumber != -1) {
			sceneChange();
			_nextSceneNumber = -1;
		}
	}

	g_globals->_sceneManager._scrollerRect.synchronize(s);
	SYNC_POINTER(g_globals->_scrollFollower);
	s.syncAsSint16LE(_loadMode);
}

/*--------------------------------------------------------------------------*/

Scene::Scene() : _sceneBounds(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT),
			_backgroundBounds(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT) {
	_sceneMode = 0;
	_activeScreenNumber = 0;
	_oldSceneBounds = Rect(4000, 4000, 4100, 4100);
	Common::fill(&_zoomPercents[0], &_zoomPercents[256], 0);
}

Scene::~Scene() {
}

void Scene::synchronize(Serializer &s) {
	if (s.getVersion() >= 2)
		StripCallback::synchronize(s);

	s.syncAsSint32LE(_field12);
	s.syncAsSint32LE(_screenNumber);
	s.syncAsSint32LE(_activeScreenNumber);
	s.syncAsSint32LE(_sceneMode);
	_backgroundBounds.synchronize(s);
	_sceneBounds.synchronize(s);
	_oldSceneBounds.synchronize(s);
	s.syncAsSint16LE(_fieldA);
	s.syncAsSint16LE(_fieldE);

	for (int i = 0; i < 256; ++i)
		s.syncAsUint16LE(_enabledSections[i]);
	for (int i = 0; i < 256; ++i)
		s.syncAsSint16LE(_zoomPercents[i]);

	if (s.getVersion() >= 7)
		_bgSceneObjects.synchronize(s);
}

void Scene::postInit(SceneObjectList *OwnerList) {
	_action = NULL;
	_field12 = 0;
	_sceneMode = 0;
}

void Scene::process(Event &event) {
	if (_action)
		_action->process(event);
}

void Scene::dispatch() {
	if (_action)
		_action->dispatch();
}

void Scene::loadScene(int sceneNum) {
	debug(1, "loadScene(%d)", sceneNum);
	_screenNumber = sceneNum;
	if (g_globals->_scenePalette.loadPalette(sceneNum))
		g_globals->_sceneManager._hasPalette = true;

	loadSceneData(sceneNum);
}

void Scene::loadSceneData(int sceneNum) {
	_activeScreenNumber = sceneNum;

	if (g_vm->getGameID() == GType_Ringworld2) {
		// Most scenes in Ringworld 2 don't have a scene size resource, but rather just have
		// a standard 320x200 size. Only read the scene size data for the specific few scenes
		switch (sceneNum) {
		case 700:
		case 1020:
		case 1100:
		case 1700:
		case 2600:
		case 2950:
		case 3100:
		case 3101:
		case 3275:
		case 3600: {
			// Get the basic scene size from the resource
			byte *data = g_resourceManager->getResource(RES_BITMAP, sceneNum, 9999);
			_backgroundBounds = Rect(0, 0, READ_LE_UINT16(data), READ_LE_UINT16(data + 2));
			DEALLOCATE(data);
			break;
		}
		default:
			// For all other scenes, use a standard screen size
			_backgroundBounds = Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			break;
		}
	} else {
		// Get the basic scene size
		byte *data = g_resourceManager->getResource(RES_BITMAP, sceneNum, 9999);
		_backgroundBounds = Rect(0, 0, READ_LE_UINT16(data), READ_LE_UINT16(data + 2));
		DEALLOCATE(data);
	}

	g_globals->_sceneManager._scene->_sceneBounds.contain(_backgroundBounds);

	// Set up a surface for storing the scene background
	SceneManager::setBackSurface();

	// Load the data lists for the scene
	g_globals->_walkRegions.load(sceneNum);

	// Load the item regions of the scene
	g_globals->_sceneRegions.load(sceneNum);

	// Load the priority regions
	_priorities.load(sceneNum);

	// Initialize the section enabled list
	Common::fill(&_enabledSections[0], &_enabledSections[16 * 16], 0xffff);

	g_globals->_sceneOffset.x = (_sceneBounds.left / 160) * 160;
	g_globals->_sceneOffset.y = (_sceneBounds.top / 100) * 100;
	g_globals->_paneRefreshFlag[0] = 1;
	g_globals->_paneRefreshFlag[1] = 1;
	g_globals->_sceneManager._loadMode = 1;
	g_globals->_sceneManager._sceneLoadCount = 0;
	g_globals->_sceneManager._sceneBgOffset = Common::Point(0, 0);

	// Load the background for the scene
	loadBackground(0, 0);
}

void Scene::loadBackground(int xAmount, int yAmount) {
	// Adjust the scene bounds by the passed scroll amounts
	_sceneBounds.translate(xAmount, yAmount);
	_sceneBounds.contain(_backgroundBounds);
	_sceneBounds.left &= ~3;
	_sceneBounds.right &= ~3;
	g_globals->_sceneOffset.x &= ~3;

	if ((_sceneBounds.top != _oldSceneBounds.top) || (_sceneBounds.left != _oldSceneBounds.left)) {
		if (g_globals->_sceneManager._loadMode == 0) {
			g_globals->_paneRefreshFlag[0] = 2;
			g_globals->_paneRefreshFlag[1] = 2;
			g_globals->_sceneManager._loadMode = 2;
		}
		_oldSceneBounds = _sceneBounds;
	}

	g_globals->_sceneOffset.x = (_sceneBounds.left / 160) * 160;
	g_globals->_sceneOffset.y = (_sceneBounds.top / 100) * 100;

	if ((_backgroundBounds.width() / 160) == 3)
		g_globals->_sceneOffset.x = 0;
	if ((_backgroundBounds.height() / 100) == 3)
		g_globals->_sceneOffset.y = 0;

	if ((g_globals->_sceneOffset.x != g_globals->_prevSceneOffset.x) ||
		(g_globals->_sceneOffset.y != g_globals->_prevSceneOffset.y)) {
		// Change has happend, so refresh background
		g_globals->_prevSceneOffset = g_globals->_sceneOffset;
		refreshBackground(xAmount, yAmount);
	}
}

void Scene::refreshBackground(int xAmount, int yAmount) {
	if (g_globals->_sceneManager._scene->_activeScreenNumber == -1)
		return;

	// Set the quadrant ranges
	int xHalfCount = MIN(_backSurface.getBounds().width() / 160, _backgroundBounds.width() / 160);
	int yHalfCount = MIN(_backSurface.getBounds().height() / 100, _backgroundBounds.height() / 100);
	int xHalfOffset = (_backgroundBounds.width() / 160) == 3 ? 0 : _sceneBounds.left / 160;
	int yHalfOffset = (_backgroundBounds.height() / 100) == 3 ? 0 : _sceneBounds.top / 100;

	// Set the limits and increment amounts
	int xInc = (xAmount < 0) ? -1 : 1;
	int xSectionStart = (xAmount < 0) ? 15 : 0;
	int xSectionEnd = (xAmount < 0) ? -1 : 16;
	int yInc = (yAmount < 0) ? -1 : 1;
	int ySectionStart = (yAmount < 0) ? 15 : 0;
	int ySectionEnd = (yAmount < 0) ? -1 : 16;
	bool changedFlag = false;

	for (int yp = ySectionStart; yp != ySectionEnd; yp += yInc) {
		for (int xp = xSectionStart; xp != xSectionEnd; xp += xInc) {
			if ((yp < yHalfOffset) || (yp >= (yHalfOffset + yHalfCount)) ||
				(xp < xHalfOffset) || (xp >= (xHalfOffset + xHalfCount))) {
				// Flag section as enabled
				_enabledSections[xp * 16 + yp] = 0xffff;
			} else {
				// Check if the section is already loaded
				if ((_enabledSections[xp * 16 + yp] == 0xffff) || ((xAmount == 0) && (yAmount == 0))) {
					// Chunk isn't loaded, so load it in
					Graphics::Surface s = _backSurface.lockSurface();
					GfxSurface::loadScreenSection(s, xp - xHalfOffset, yp - yHalfOffset, xp, yp);
					_backSurface.unlockSurface();
					changedFlag = true;
				} else {
					int yv = (_enabledSections[xp * 16 + yp] == ((xp - xHalfOffset) << 4)) ? 0 : 1;
					if (yv | (yp - yHalfOffset)) {
						// Copy an existing 160x100 screen section previously loaded
						int xSectionDest = xp - xHalfOffset;
						int ySectionDest = yp - yHalfOffset;
						int xSectionSrc = _enabledSections[xp * 16 + yp] >> 4;
						int ySectionSrc = _enabledSections[xp * 16 + yp] & 0xf;

						Rect srcBounds(xSectionSrc * 160, ySectionSrc * 100,
								(xSectionSrc + 1) * 160, (ySectionSrc + 1) * 100);
						Rect destBounds(xSectionDest * 160, ySectionDest * 100,
								(xSectionDest + 1) * 160, (ySectionDest + 1) * 100);
						if (g_vm->getGameID() != GType_Ringworld) {
							// For Blue Force and Return to Ringworld, if the scene has an interface area,
							// exclude it from the copy
							srcBounds.bottom = MIN<int16>(srcBounds.bottom, BF_GLOBALS._interfaceY);
							destBounds.bottom = MIN<int16>(destBounds.bottom, BF_GLOBALS._interfaceY);
						}

						_backSurface.copyFrom(_backSurface, srcBounds, destBounds);
					}
				}

				_enabledSections[xp * 16 + yp] =
					((xp - xHalfOffset) << 4) | (yp - yHalfOffset);
			}
		}
	}

	if (changedFlag) {
		drawBackgroundObjects();
	}
}

void Scene::drawBackgroundObjects() {
	Common::Array<SceneObject *> objList;

	// Initial loop to set the priority for entries in the list
	for (SynchronizedList<SceneObject *>::iterator i = _bgSceneObjects.begin(); i != _bgSceneObjects.end(); ++i) {
		SceneObject *obj = *i;
		objList.push_back(obj);

		// Handle updating object priority
		if (!(obj->_flags & OBJFLAG_FIXED_PRIORITY)) {
			obj->_priority = MIN((int)obj->_position.y - 1,
				(int)g_globals->_sceneManager._scene->_backgroundBounds.bottom);
		}
	}

	// Sort the list by priority
	_bgSceneObjects.sortList(objList);

	// Drawing loop
	for (uint objIndex = 0; objIndex < objList.size(); ++objIndex) {
		SceneObject *obj = objList[objIndex];

		obj->reposition();
		obj->draw();
	}
}

void Scene::setZoomPercents(int yStart, int minPercent, int yEnd, int maxPercent) {
	int currDiff = 0;
	int v = 0;
	while (v < yStart)
		_zoomPercents[v++] = minPercent;

	int diff1 = ABS(maxPercent - minPercent);
	int diff2 = ABS(yEnd - yStart);
	int remainingDiff = MAX(diff1, diff2);

	while (remainingDiff-- != 0) {
		_zoomPercents[v] = minPercent;
		if (diff2 <= diff1) {
			++minPercent;
			currDiff += diff2;
			if (currDiff >= diff1) {
				currDiff -= diff1;
				++v;
			}
		} else {
			++v;
			currDiff += diff1;
			if (currDiff >= diff2) {
				currDiff -= diff2;
				++minPercent;
			}
		}
	}

	while (yEnd < 256)
		_zoomPercents[yEnd++] = minPercent;
}

/*--------------------------------------------------------------------------*/

void Game::restartGame() {
	if (MessageDialog::show(RESTART_MSG, CANCEL_BTN_STRING, RESTART_BTN_STRING) == 1)
		g_globals->_game->restart();
}

void Game::saveGame() {
	if (!g_vm->canSaveGameStateCurrently())
		MessageDialog::show(SAVING_NOT_ALLOWED_MSG, OK_BTN_STRING);
	else {
		// Show the save dialog
		handleSaveLoad(true, g_globals->_sceneHandler->_saveGameSlot, g_globals->_sceneHandler->_saveName);
	}
}

void Game::restoreGame() {
	if (!g_vm->canLoadGameStateCurrently())
		MessageDialog::show(RESTORING_NOT_ALLOWED_MSG, OK_BTN_STRING);
	else {
		// Show the load dialog
		handleSaveLoad(false, g_globals->_sceneHandler->_loadGameSlot, g_globals->_sceneHandler->_saveName);
	}
}

void Game::quitGame() {
	if (MessageDialog::show(QUIT_CONFIRM_MSG, CANCEL_BTN_STRING, QUIT_BTN_STRING) == 1)
		g_vm->quitGame();
}

void Game::handleSaveLoad(bool saveFlag, int &saveSlot, Common::String &saveName) {
	GUI::SaveLoadChooser *dialog;
	if (saveFlag)
		dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), saveFlag);
	else
		dialog = new GUI::SaveLoadChooser(_("Load game:"), _("Load"), saveFlag);

	saveSlot = dialog->runModalWithCurrentTarget();
	saveName = dialog->getResultString();

	delete dialog;
}

void Game::execute() {
	// Main game loop
	bool activeFlag = false;
	do {
		// Process all currently atcive game handlers
		activeFlag = false;
		for (SynchronizedList<GameHandler *>::iterator i = _handlers.begin(); i != _handlers.end(); ++i) {
			GameHandler *gh = *i;
			if (gh->_lockCtr.getCtr() == 0) {
				gh->execute();
				activeFlag = true;
			}
		}
	} while (activeFlag && !g_vm->shouldQuit());
}

} // End of namespace TsAGE

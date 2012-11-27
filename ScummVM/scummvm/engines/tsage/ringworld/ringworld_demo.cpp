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

#include "tsage/ringworld/ringworld_demo.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace TsAGE {

namespace Ringworld {

void RingworldDemoGame::start() {
	// Start the demo's single scene
	g_globals->_sceneManager.changeScene(1);

	g_globals->_events.setCursor(CURSOR_NONE);
}

Scene *RingworldDemoGame::createScene(int sceneNumber) {
	// The demo only has a single scene, so ignore the scene number and always return it
	return new RingworldDemoScene();
}

bool RingworldDemoGame::canLoadGameStateCurrently() {
	return false;
}

bool RingworldDemoGame::canSaveGameStateCurrently() {
	return false;
}

void RingworldDemoGame::quitGame() {
	if (MessageDialog::show(DEMO_EXIT_MSG, EXIT_BTN_STRING, DEMO_BTN_STRING) == 0)
		g_vm->quitGame();
}

void RingworldDemoGame::pauseGame() {
	g_globals->_events.setCursor(CURSOR_ARROW);
	MessageDialog *dlg = new MessageDialog(DEMO_PAUSED_MSG, EXIT_BTN_STRING, DEMO_RESUME_BTN_STRING);
	dlg->draw();

	GfxButton *selectedButton = dlg->execute(&dlg->_btn2);
	bool exitFlag  =  selectedButton != &dlg->_btn2;

	delete dlg;
	g_globals->_events.hideCursor();

	if (exitFlag)
		g_vm->quitGame();
}

void RingworldDemoGame::processEvent(Event &event) {
	if (event.eventType == EVENT_KEYPRESS) {
		switch (event.kbd.keycode) {
		case Common::KEYCODE_F1:
			// F1 - Help
			MessageDialog::show(DEMO_HELP_MSG, OK_BTN_STRING);
			break;

		case Common::KEYCODE_F2: {
			// F2 - Sound Options
			ConfigDialog *dlg = new ConfigDialog();
			dlg->runModal();
			delete dlg;
			g_globals->_soundManager.syncSounds();
			g_globals->_events.setCursorFromFlag();
			break;
		}

		case Common::KEYCODE_F3:
			// F3 - Quit
			quitGame();
			event.handled = false;
			break;

		default:
			break;
		}
	} else if (event.eventType == EVENT_BUTTON_DOWN) {
		pauseGame();
		event.handled = true;
	}
}

/*--------------------------------------------------------------------------
 * Ringworld Demo scene
 *
 *--------------------------------------------------------------------------*/

void RingworldDemoScene::postInit(SceneObjectList *OwnerList) {
	signal();
}

void RingworldDemoScene::signal() {
	_soundHandler.play(4);
	_actor1.postInit();
	_actor2.postInit();
	_actor3.postInit();
	_actor4.postInit();
	_actor5.postInit();
	_actor6.postInit();

	setAction(&_sequenceManager, this, 22, &_actor1, &_actor2, &_actor3, &_actor4, &_actor5, &_actor6, NULL);
}

void RingworldDemoScene::process(Event &event) {

}

} // End of namespace Ringworld

} // End of namespace TsAGE

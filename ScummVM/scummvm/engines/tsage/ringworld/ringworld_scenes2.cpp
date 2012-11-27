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
#include "tsage/ringworld/ringworld_scenes2.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace TsAGE {

namespace Ringworld {

/*--------------------------------------------------------------------------
 * Scene 1000 - Title Screen
 *
 *--------------------------------------------------------------------------*/

void Scene1000::Action1::signal() {
	Scene1000 *scene = (Scene1000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		setDelay(10);
		break;
	case 1:
		scene->_object4.postInit();
		scene->_object4.setVisage(1001);
		scene->_object4._frame = 1;
		scene->_object4.setStrip2(5);
		scene->_object4.changeZoom(100);
		scene->_object4.animate(ANIM_MODE_2, NULL);
		scene->_object4.setPosition(Common::Point(403, 163));
		setDelay(90);
		break;
	case 2: {
		SceneItem::display(0, 0);
		scene->_object4.remove();
		scene->_object1.changeZoom(-1);
		NpcMover *mover = new NpcMover();
		Common::Point pt(180, 100);
		scene->_object1.addMover(mover, &pt, this);
		break;
	}
	case 3:
		g_globals->_sceneManager.changeScene(1400);
		break;
	}

}

void Scene1000::Action2::signal() {
	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		setDelay(10);
		break;
	case 1:
		SceneItem::display(1000, 0, SET_Y, 20, SET_FONT, 2, SET_BG_COLOR, -1,
				SET_EXT_BGCOLOR, 35, SET_WIDTH, 200, SET_KEEP_ONSCREEN, 1, LIST_END);
		setDelay(180);
		break;
	case 2:
		SceneItem::display(0, 0);
		g_globals->_sceneManager.changeScene(2000);
		break;
	default:
		break;
	}
}

void Scene1000::Action3::signal() {
	Scene1000 *scene = (Scene1000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_sceneManager._scene->loadBackground(0, 0);
		setDelay(60);
		break;
	case 1: {
		NpcMover *mover = new NpcMover();
		Common::Point pt(158, 31);
		scene->_object3.addMover(mover, &pt, this);
		break;
	}
	case 2:
	case 3:
		setDelay(60);
		break;
	case 4:
		g_globals->_player.show();
		setDelay(240);
		break;
	case 5: {
		g_globals->_player.enableControl();

		const char *SEEN_INTRO = "seen_intro";
		if (!ConfMan.hasKey(SEEN_INTRO) || !ConfMan.getBool(SEEN_INTRO)) {
			// First time being played, so show the introduction
			ConfMan.setBool(SEEN_INTRO, true);
			ConfMan.flushToDisk();

			setDelay(1);
		} else {
			// Prompt user for whether to start play or watch introduction
			g_globals->_player.enableControl();

			if (MessageDialog::show2(WATCH_INTRO_MSG, START_PLAY_BTN_STRING, INTRODUCTION_BTN_STRING) == 0) {
				_actionIndex = 20;
				g_globals->_soundHandler.fadeOut(this);
			} else {
				setDelay(1);
			}
		}

		g_globals->_player.disableControl();
		break;
	}
	case 6: {
		scene->_object3.remove();
		g_globals->_player.setStrip2(2);
		NpcMover *mover = new NpcMover();
		Common::Point pt(480, 100);
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 7:
		g_globals->_scenePalette.loadPalette(1002);
		g_globals->_scenePalette.refresh();
		g_globals->_scenePalette.addRotation(80, 95, -1);
		scene->_object3.postInit();
		scene->_object3.setVisage(1002);
		scene->_object3.setStrip(1);
		scene->_object3.setPosition(Common::Point(284, 122));
		scene->_object3.changeZoom(1);

		zoom(true);
		setDelay(200);
		break;
	case 8:
		zoom(false);
		setDelay(10);
		break;
	case 9:
		scene->_object3.setStrip(2);
		scene->_object3.setPosition(Common::Point(285, 155));

		zoom(true);
		setDelay(400);
		break;
	case 10:
		zoom(false);
		setDelay(10);
		break;
	case 11:
		scene->_object3.setStrip(3);
		scene->_object3.setPosition(Common::Point(279, 172));

		zoom(true);
		setDelay(240);
		break;
	case 12:
		zoom(false);
		setDelay(10);
		break;
	case 13:
		scene->_object3.setStrip(4);
		scene->_object3.setPosition(Common::Point(270, 128));

		zoom(true);
		setDelay(300);
		break;
	case 14:
		zoom(false);
		setDelay(10);
		break;
	case 15:
		scene->_object3.setStrip(1);
		scene->_object3.setFrame(2);
		scene->_object3.setPosition(Common::Point(283, 137));

		zoom(true);
		setDelay(300);
		break;
	case 16:
		zoom(false);
		setDelay(10);
		break;
	case 17:
		scene->_object3.setStrip(5);
		scene->_object3.setFrame(1);
		scene->_object3.setPosition(Common::Point(292, 192));

		zoom(true);
		setDelay(300);
		break;
	case 18:
		zoom(false);
		g_globals->_scenePalette.clearListeners();
		g_globals->_soundHandler.fadeOut(this);
		break;
	case 19:
		g_globals->_sceneManager.changeScene(10);
		break;
	case 20:
		g_globals->_sceneManager.changeScene(30);
		break;
	default:
		break;
	}
}

void Scene1000::Action3::zoom(bool up) {
	Scene1000 *scene = (Scene1000 *)g_globals->_sceneManager._scene;

	if (up) {
		while ((scene->_object3._percent < 100) && !g_vm->shouldQuit()) {
			scene->_object3.changeZoom(MIN(scene->_object3._percent + 5, 100));
			g_globals->_sceneObjects->draw();
			g_globals->_events.delay(1);
		}
	} else {
		while ((scene->_object3._percent > 0) && !g_vm->shouldQuit()) {
			scene->_object3.changeZoom(MAX(scene->_object3._percent - 5, 0));
			g_globals->_sceneObjects->draw();
			g_globals->_events.delay(1);
		}
	}
}

/*--------------------------------------------------------------------------*/

void Scene1000::postInit(SceneObjectList *OwnerList) {
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);
	loadScene(1000);

	if (g_globals->_sceneManager._previousScene == 2000) {
		setZoomPercents(150, 10, 180, 100);
		_object1.postInit();
		_object1.setVisage(1001);
		_object1._strip = 7;
		_object1.animate(ANIM_MODE_2, 0);
		_object1._moveDiff = Common::Point(1, 1);
		_object1.setPosition(Common::Point(120, 180));

		setAction(&_action2);

		g_globals->_sceneManager._scene->_sceneBounds.center(_object1._position.x, _object1._position.y);
		g_globals->_sceneManager._scene->_sceneBounds.contain(g_globals->_sceneManager._scene->_backgroundBounds);

		g_globals->_sceneOffset.x = (g_globals->_sceneManager._scene->_sceneBounds.left / 160) * 160;
		g_globals->_soundHandler.play(114);
	} else if (g_globals->_sceneManager._previousScene == 2222) {
		setZoomPercents(150, 10, 180, 100);
		_object1.postInit();
		_object1.setVisage(1001);
		_object1._strip = 7;
		_object1.animate(ANIM_MODE_2, 0);
		_object1._moveDiff = Common::Point(2, 2);
		_object1.setPosition(Common::Point(120, 180));

		g_globals->_sceneManager._scene->_sceneBounds.center(_object1._position.x, _object1._position.y);
		g_globals->_sceneManager._scene->_sceneBounds.contain(g_globals->_sceneManager._scene->_backgroundBounds);
		g_globals->_sceneOffset.x = (g_globals->_sceneManager._scene->_sceneBounds.left / 160) * 160;

		setAction(&_action1);
	} else {
		g_globals->_soundHandler.play(4);
		setZoomPercents(0, 10, 30, 100);
		_object3.postInit();
		_object3.setVisage(1050);
		_object3.changeZoom(-1);
		_object3.setPosition(Common::Point(158, 0));

		g_globals->_player.postInit();
		g_globals->_player.setVisage(1050);
		g_globals->_player.setStrip(3);
		g_globals->_player.setPosition(Common::Point(160, 191));
		g_globals->_player._moveDiff.x = 12;
		g_globals->_player.hide();
		g_globals->_player.disableControl();

		g_globals->_sceneManager._scene->_sceneBounds.center(_object3._position.x, _object3._position.y);

		setAction(&_action3);
	}
}

/*--------------------------------------------------------------------------
 * Scene 1001 - Fleeing planet cutscene
 *
 *--------------------------------------------------------------------------*/

void Scene1001::Action1::signal() {
	Scene1001 *scene = (Scene1001 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(10);
		break;
	case 1:
		scene->_object3.animate(ANIM_MODE_5, this);
		break;
	case 2: {
		Common::Point pt(108, 171);
		NpcMover *mover = new NpcMover();
		scene->_object3.addMover(mover, &pt, this);
		break;
	}
	case 3: {
		Common::Point pt(170, 159);
		NpcMover *mover = new NpcMover();
		scene->_object3.addMover(mover, &pt, this);
		break;
	}
	case 4: {
		scene->_object2.postInit();
		scene->_object2.setVisage(16);
		scene->_object2.setStrip2(4);
		scene->_object2.setPosition(Common::Point(61, 177));
		scene->_object2.animate(ANIM_MODE_5, this);

		Common::Point pt(320, 100);
		NpcMover *mover = new NpcMover();
		scene->_object3.addMover(mover, &pt, this);
		break;
	}
	case 5: {
		Common::Point pt(82, 166);
		NpcMover *mover = new NpcMover();
		scene->_object2.addMover(mover, &pt, this);
		break;
	}
	case 6: {
		Common::Point pt(64, 149);
		NpcMover *mover = new NpcMover();
		scene->_object2.addMover(mover, &pt, this);
		break;
	}
	case 7: {
		Common::Point pt(15, 136);
		NpcMover *mover = new NpcMover();
		scene->_object2.addMover(mover, &pt, this);
		break;
	}
	case 8: {
		Common::Point pt(-5, 120);
		NpcMover *mover = new NpcMover();
		scene->_object2.addMover(mover, &pt, this);
		break;
	}
	case 9: {
		scene->_object1.postInit();
		scene->_object1.setVisage(16);
		scene->_object1.setStrip2(1);
		scene->_object1.setFrame(1);
		scene->_object1.setPosition(Common::Point(-75, 87));
		scene->_object1.animate(ANIM_MODE_2, NULL);

		Common::Point pt(0, 100);
		NpcMover *mover = new NpcMover();
		scene->_object1.addMover(mover, &pt, this);
		break;
	}
	case 10: {
		Common::Point pt1(107, 115);
		NpcMover *mover1 = new NpcMover();
		scene->_object1.addMover(mover1, &pt1, NULL);

		scene->_object3.setVisage(16);
		scene->_object3.setStrip2(5);
		scene->_object3.setFrame2(2);
		scene->_object3.setPosition(Common::Point(220, 200));

		Common::Point pt2(187, 181);
		NpcMover *mover2 = new NpcMover();
		scene->_object3.addMover(mover2, &pt2, this);
		break;
	}
	case 11: {
		scene->_object2.setVisage(16);
		scene->_object2.setStrip2(5);
		scene->_object2.setFrame2(1);
		scene->_object2.setPosition(Common::Point(211, 0));

		Common::Point pt(189, 30);
		NpcMover *mover = new NpcMover();
		scene->_object2.addMover(mover, &pt, this);
		break;
	}
	case 12:
		scene->_stripManager.start(100, this);
		break;
	case 13: {
		scene->_object4.postInit();
		scene->_object4.setVisage(16);
		scene->_object4.setStrip2(2);
		scene->_object4.setFrame(4);
		scene->_object4.setPosition(Common::Point(360, 80));
		scene->_object4.animate(ANIM_MODE_2, NULL);

		Common::Point pt(303, 97);
		NpcMover *mover = new NpcMover();
		scene->_object4.addMover(mover, &pt, this);
		break;
	}
	case 14:
		scene->_stripManager.start(110, this);
		break;
	case 15:
		setDelay(10);
		break;
	case 16: {
		scene->_soundHandler1.play(90);

		scene->_object6.postInit();
		scene->_object6.setVisage(16);
		scene->_object6.setStrip2(6);
		scene->_object6.setFrame2(2);
		scene->_object6._moveDiff = Common::Point(20, 20);
		scene->_object6.fixPriority(20);
		scene->_object6.setPosition(Common::Point(scene->_object2._position.x - 6, scene->_object2._position.y + 7));
		scene->_object6.animate(ANIM_MODE_5, NULL);

		Common::Point pt(scene->_object6._position.x - 70, scene->_object6._position.y + 70);
		NpcMover *mover = new NpcMover();
		scene->_object6.addMover(mover, &pt, this);
		break;
	}
	case 17: {
		scene->_soundHandler1.play(90);
		scene->_object6.remove();

		scene->_object7.postInit();
		scene->_object7.setVisage(16);
		scene->_object7.setStrip2(6);
		scene->_object7.setFrame2(1);
		scene->_object7._moveDiff = Common::Point(20, 20);
		scene->_object7.setPosition(Common::Point(scene->_object3._position.x - 28, scene->_object3._position.y - 11));
		scene->_object7.fixPriority(200);
		scene->_object7.animate(ANIM_MODE_5, NULL);

		Common::Point pt(scene->_object7._position.x - 70, scene->_object7._position.y - 70);
		NpcMover *mover = new NpcMover();
		scene->_object7.addMover(mover, &pt, this);
		break;
	}
	case 18:
		scene->_object7.remove();

		scene->_object5.postInit();
		scene->_object5.setVisage(16);
		scene->_object5.setPosition(Common::Point(306, 93));
		scene->_object5._strip = 3;
		scene->_object5.fixPriority(200);
		scene->_object5.animate(ANIM_MODE_2, NULL);
		setDelay(30);
		break;
	case 19: {
		g_globals->_soundHandler.play(91);
		byte adjustData[4] = {0xff, 0xff, 0xff, 0};
		g_globals->_scenePalette.fade(adjustData, false, 0);

		scene->_object1._strip = 7;
		scene->_object1._frame = 1;
		scene->_object1.setPosition(Common::Point(314, 112));
		scene->_object1.addMover(NULL);
		setDelay(2);
		break;
	}
	case 20:
		g_globals->_scenePalette.loadPalette(16);
		g_globals->_scenePalette.refresh();
		setDelay(6);
		break;
	case 21:
		scene->_object1._numFrames = 15;
		scene->_object1.animate(ANIM_MODE_5, this);
		break;
	case 22:
		g_globals->_soundHandler.play(92);
		scene->_stripManager.start(111, this);
		break;
	case 23:
		setDelay(60);
		break;
	case 24:
		g_globals->_sceneManager.changeScene(2000);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene1001::postInit(SceneObjectList *OwnerList) {
	loadScene(16);
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);

	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerCText);
	_stripManager.addSpeaker(&_speakerCR);
	_stripManager.addSpeaker(&_speakerSL);
	_speakerQText._color1 = 11;

	_object3.postInit();
	_object3.setVisage(16);
	_object3.setStrip2(4);
	_object3.setPosition(Common::Point(61, 177));

	g_globals->_soundHandler.play(85);
	setAction(&_action1);
}


/*--------------------------------------------------------------------------
 * Scene 1250 -
 *
 *--------------------------------------------------------------------------*/

void Scene1250::Action1::signal() {
	Scene1250 *scene = (Scene1250 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(g_globals->_randomSource.getRandomNumber(120) + 60);
		break;
	case 1:
		scene->_object1.animate(ANIM_MODE_5, this);
		_actionIndex = 0;
		break;
	}
}

void Scene1250::Action2::signal() {
	Scene1250 *scene = (Scene1250 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		switch (g_globals->_randomSource.getRandomNumber(2)) {
		case 0:
			scene->_object2.setPosition(Common::Point(163, 75));
			break;
		case 1:
			scene->_object2.setPosition(Common::Point(109, 65));
			break;
		case 2:
			scene->_object2.setPosition(Common::Point(267, 20));
			break;
		}

		setDelay(30);
		break;
	case 1:
		scene->_object2.animate(ANIM_MODE_5, this);
		_actionIndex = 0;
		break;
	}
}

void Scene1250::Action3::signal() {
	Scene1250 *scene = (Scene1250 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(30);
		break;
	case 1:
		scene->_stripManager.start(1251, this);
		break;
	case 2:
		setDelay(6);
		break;
	case 3:
		g_globals->_sceneManager.changeScene(1000);
		break;
	}
}

void Scene1250::Action4::signal() {
	Scene1250 *scene = (Scene1250 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(3);
		break;
	case 1:
		scene->_stripManager.start(1250, this);
		break;
	case 2:
		setDelay(6);
		break;
	case 3:
		g_globals->_sceneManager.changeScene(2000);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene1250::postInit(SceneObjectList *OwnerList) {
	loadScene(1250);
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);

	_stripManager.addSpeaker(&_speakerQText);
	_speakerQText._textPos = Common::Point(120, 120);
	_speakerQText._textWidth = 180;

	_object1.postInit();
	_object1.setVisage(1250);
	_object1.setPosition(Common::Point(126, 69));
	_object1.setStrip2(1);
	_object1._frame = 1;
	_object1.setAction(&_action1);

	_object2.postInit();
	_object2.setVisage(1250);
	_object2.setPosition(Common::Point(126, 69));
	_object2.setStrip2(2);
	_object2.fixPriority(255);
	_object2._frame = 1;
	_object2.setAction(&_action2);

	g_globals->_sceneManager._scene->_sceneBounds.contain(g_globals->_sceneManager._scene->_backgroundBounds);
	g_globals->_sceneOffset.x = (g_globals->_sceneManager._scene->_sceneBounds.left / 160) * 160;

	if ((g_globals->_sceneManager._previousScene != 2000) || (g_globals->_stripNum != 1250)) {
		setAction(&_action4);
	} else {
		setAction(&_action3);
		g_globals->_soundHandler.play(114);
	}
}

/*--------------------------------------------------------------------------
 * Scene 1400 - Ringworld Wall
 *
 *--------------------------------------------------------------------------*/

void Scene1400::Action1::signal() {
	Scene1400 *scene = (Scene1400 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(5);
		break;
	case 1: {
		SceneItem::display(1400, 0, SET_X, 120, SET_Y, 610, SET_FONT, 2, SET_EXT_BGCOLOR, 23, SET_KEEP_ONSCREEN, -1, LIST_END);

		Common::Point pt(160, 700);
		NpcMover *mover = new NpcMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 2: {
		g_globals->_player.setStrip2(3);
		g_globals->_player.changeZoom(100);

		Common::Point pt(160, 100);
		NpcMover *mover = new NpcMover();
		g_globals->_player.addMover(mover, &pt, this);

		SceneItem::display(0, 0);
		setDelay(360);
		break;
	}
	case 3:
		SceneItem::display(1400, 2, SET_X, 60, SET_Y, g_globals->_sceneManager._scene->_sceneBounds.bottom - 80,
			SET_FONT, 2, SET_FG_COLOR, 13, SET_POS_MODE, 0, SET_KEEP_ONSCREEN, -1, LIST_END);
		setDelay(420);
		break;
	case 4:
		SceneItem::display(0, 0);
		setDelay(360);
		break;
	case 5:
		SceneItem::display(1400, 3, SET_X, 60, SET_Y, g_globals->_sceneManager._scene->_sceneBounds.bottom - 80,
			SET_FONT, 2, SET_FG_COLOR, 23, SET_POS_MODE, 0, SET_KEEP_ONSCREEN, -1, LIST_END);
		setDelay(360);
		break;
	case 6:
		SceneItem::display(0, 0);
		break;
	case 7: {
		g_globals->_player._frame = 1;
		g_globals->_player.setStrip2(1);
		g_globals->_player._numFrames = 5;
		g_globals->_player.animate(ANIM_MODE_5, this);

		Common::Point pt(205, 70);
		NpcMover *mover = new NpcMover();
		g_globals->_player.addMover(mover, &pt, NULL);
		g_globals->_sceneManager._fadeMode = FADEMODE_NONE;

		scene->loadScene(1402);
		break;
	}
	case 8:
		g_globals->_player.setStrip2(2);
		g_globals->_player._numFrames = 10;
		g_globals->_player.animate(ANIM_MODE_2, NULL);

		SceneItem::display(1400, 4, SET_X, 30, SET_Y, g_globals->_player._position.y + 10, SET_FONT, 2,
			SET_FG_COLOR, 13, SET_POS_MODE, 0, SET_KEEP_ONSCREEN, -1, LIST_END);
		setDelay(300);
		break;
	case 9: {
		SceneItem::display(0, 0);
		Common::Point pt(450, 45);
		NpcMover *mover = new NpcMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 10:
		g_globals->_sceneManager._scrollerRect = Rect(40, 20, 280, 180);
		g_globals->_sceneManager._fadeMode = FADEMODE_GRADUAL;
		g_globals->_stripNum = 1500;
		g_globals->_soundHandler.stop();

		g_globals->_sceneManager.changeScene(1500);
		break;
	}
}

void Scene1400::Action1::dispatch() {
	Action::dispatch();

	if ((_actionIndex > 3) && (_actionIndex < 9))
		g_globals->_sceneText.setPosition(Common::Point(60, g_globals->_sceneManager._scene->_sceneBounds.bottom - 80));

	if ((_actionIndex <= 2) && (g_globals->_player._percent > 22))
		g_globals->_player.changeZoom(100 - (800 - g_globals->_player._position.y));

	if ((_actionIndex >= 9) && (g_globals->_player._percent > 22))
		g_globals->_player.changeZoom(100 - (g_globals->_player._position.x - 205));
}

/*--------------------------------------------------------------------------*/

void Scene1400::postInit(SceneObjectList *OwnerList) {
	if (g_globals->_stripNum != 1400) {
		loadScene(1401);
	} else {
		loadScene(1402);
	}
	Scene::postInit();

	g_globals->_sceneManager._scrollerRect = Rect(40, 90, 280, 180);
	g_globals->_player.postInit();
	g_globals->_player.setVisage(1401);
	g_globals->_player.animate(ANIM_MODE_2, 0);
	g_globals->_player.setStrip2(4);
	g_globals->_player.fixPriority(4);
	g_globals->_player.disableControl();

	g_globals->_player._moveDiff = Common::Point(4, 2);
	g_globals->_player.setPosition(Common::Point(160, 800));
	g_globals->_sceneManager._scene->_sceneBounds.center(g_globals->_player._position);
	g_globals->_sceneManager._scene->_sceneBounds.contain(g_globals->_sceneManager._scene->_backgroundBounds);
	g_globals->_sceneOffset.y = (g_globals->_sceneManager._scene->_sceneBounds.top / 100) * 100;

	setAction(&_action1);
	g_globals->_soundHandler.play(118);
}

/*--------------------------------------------------------------------------
 * Scene 1500 - Ringworld Space-port
 *
 *--------------------------------------------------------------------------*/

void Scene1500::Action1::signal() {
	Scene1500 *scene = (Scene1500 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		scene->_object1.postInit();
		scene->_object1.setVisage(1501);
		scene->_object1._moveDiff = Common::Point(2, 1);
		scene->_object1.setPosition(Common::Point(204, 85));
		scene->_object1.animate(ANIM_MODE_2, NULL);
		scene->_object1._numFrames = 3;
		scene->_object1.changeZoom(-1);

		Common::Point pt(238, 121);
		PlayerMover *mover = new PlayerMover();
		scene->_object1.addMover(mover, &pt, this);
		break;
	}
	case 1: {
		Common::Point pt(312, 145);
		PlayerMover *mover = new PlayerMover();
		scene->_object1.addMover(mover, &pt, this);
		break;
	}
	case 2: {
		scene->_object1.setStrip(2);
		scene->_object1.setFrame(1);
		scene->_object1._moveDiff.y = 2;
		scene->_object1._numFrames = 5;

		Common::Point pt(310, 150);
		PlayerMover *mover = new PlayerMover();
		scene->_object1.addMover(mover, &pt, this);
		break;
	}
	case 3: {
		Common::Point pt(304, 165);
		PlayerMover *mover = new PlayerMover();
		scene->_object1.addMover(mover, &pt, this);
		break;
	}
	case 4: {
		scene->_object1._numFrames = 3;
		scene->_object1.setStrip2(3);
		scene->_object1.animate(ANIM_MODE_2, this);

		Common::Point pt(94, 175);
		PlayerMover *mover = new PlayerMover();
		scene->_object1.addMover(mover, &pt, this);
		break;
	}
	case 5:
		setDelay(30);
		break;
	case 6:
		scene->_soundHandler.play(123);
		scene->_object1.setStrip2(4);
		scene->_object1.setFrame(1);
		scene->_object1.animate(ANIM_MODE_5, this);
		break;
	case 7:
		scene->_object1.setStrip2(5);
		scene->_object1.animate(ANIM_MODE_2, NULL);
		scene->_soundHandler.play(124, this);
		break;
	case 8:
		g_globals->_soundHandler.play(126, this);
		break;
	case 9:
		g_globals->_soundHandler.play(127);
		g_globals->_sceneManager.changeScene(2000);
		break;
	}
}

void Scene1500::Action2::signal() {
	Scene1500 *scene = (Scene1500 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(6);
		break;
	case 1: {
		scene->_object2.postInit();
		scene->_object2.setVisage(1502);
		scene->_object2.fixPriority(255);
		scene->_object2.changeZoom(5);
		scene->_object2._frame = 1;
		scene->_object2._moveDiff = Common::Point(1, 1);
		scene->_object2.setPosition(Common::Point(104, 184));
		scene->_object2.animate(ANIM_MODE_2, NULL);

		Common::Point pt(118, 147);
		NpcMover *mover = new NpcMover();
		scene->_object2.addMover(mover, &pt, this);
		break;
	}
	case 2: {
		scene->_object2._moveDiff.x = 5;
		scene->_object2.changeZoom(-1);
		Common::Point pt(-55, 200);
		NpcMover *mover = new NpcMover();
		scene->_object2.addMover(mover, &pt, this);
		break;
	}
	case 3:
		scene->_soundHandler.release();
		g_globals->_stripNum = 1505;
		g_globals->_sceneManager.changeScene(2400);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene1500::postInit(SceneObjectList *OwnerList) {
	loadScene(1500);
	Scene::postInit();

	if ((g_globals->_stripNum == 1500) || ((g_globals->_stripNum != 1504) && (g_globals->_stripNum != 2751))) {
		g_globals->_soundHandler.play(120);
		setZoomPercents(105, 20, 145, 100);

		setAction(&_action1);
	} else {
		setZoomPercents(150, 5, 200, 100);

		_object1.postInit();
		_object1.setVisage(1501);
		_object1.setStrip2(5);
		_object1.setPosition(Common::Point(94, 175));
		_object1.animate(ANIM_MODE_2, NULL);

		setAction(&_action2);
	}
}

} // End of namespace Ringworld

} // End of namespace TsAGE

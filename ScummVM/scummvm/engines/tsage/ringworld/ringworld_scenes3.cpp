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
#include "tsage/ringworld/ringworld_scenes3.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace TsAGE {

namespace Ringworld {

/*--------------------------------------------------------------------------
 * Scene 2000 - Cockpit cutscenes
 *
 *--------------------------------------------------------------------------*/

void Scene2000::Action1::signal() {
	Scene2000 *scene = (Scene2000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		error("Old stuff");
		break;
	case 1:
		scene->_stripManager.start(2075, this);
		break;
	case 2:
		setDelay(4);
		break;
	case 3:
		g_globals->_stripNum = 0;
		g_globals->_sceneManager.changeScene(1250);
		break;
	}
}

void Scene2000::Action2::signal() {
	Scene2000 *scene = (Scene2000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_object2.animate(ANIM_MODE_6, NULL);
		setDelay(g_globals->_randomSource.getRandomNumber(179) + 60);
		break;
	case 1:
		setDelay(g_globals->_randomSource.getRandomNumber(179) + 60);
		if (g_globals->_randomSource.getRandomNumber(4) >= 2)
			_actionIndex = 0;
		break;
	case 2:
		setDelay(g_globals->_randomSource.getRandomNumber(179) + 60);
		_actionIndex = g_globals->_randomSource.getRandomNumber(1);
		break;
	}
}

void Scene2000::Action3::signal() {
	Scene2000 *scene = (Scene2000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_object6.animate(ANIM_MODE_5, NULL);
		setDelay(g_globals->_randomSource.getRandomNumber(179) + 60);
		break;
	case 1:
		scene->_object6.animate(ANIM_MODE_6, NULL);
		setDelay(g_globals->_randomSource.getRandomNumber(179) + 60);
		_actionIndex = 0;
		break;
	}
}

void Scene2000::Action4::signal() {
	Scene2000 *scene = (Scene2000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_object4.animate(ANIM_MODE_5, NULL);
		setDelay(g_globals->_randomSource.getRandomNumber(179) + 60);
		break;
	case 1:
		scene->_object4.animate(ANIM_MODE_6, NULL);
		setDelay(g_globals->_randomSource.getRandomNumber(179) + 60);
		_actionIndex = 0;
		break;
	}
}

void Scene2000::Action5::signal() {
	Scene2000 *scene = (Scene2000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_object3.animate(ANIM_MODE_5, NULL);
		setDelay(g_globals->_randomSource.getRandomNumber(125) + 300);
		break;
	case 1:
		scene->_object3.animate(ANIM_MODE_6, NULL);
		setDelay(g_globals->_randomSource.getRandomNumber(125) + 300);
		_actionIndex = 0;
		break;
	}
}

void Scene2000::Action6::signal() {
	Scene2000 *scene = (Scene2000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(130);
		break;
	case 1:
		scene->_soundHandler2.play(79);
		scene->_stripManager.start(2000, this);
		break;
	case 2:
		g_globals->_soundHandler.play(81);
		scene->_object6.postInit();
		scene->_object6.setVisage(2003);
		scene->_object6.setAction(NULL);
		scene->_object6.setStrip2(2);
		scene->_object6.setPosition(Common::Point(184, 137));
		scene->_object6.animate(ANIM_MODE_5, this);
		break;
	case 3:
		scene->_stripManager.start(95, this);
		break;
	case 4:
		scene->_object6.animate(ANIM_MODE_6, this);
		break;
	case 5:
		g_globals->_soundHandler.play(80);
		scene->_object6.remove();
		g_globals->_sceneManager.changeScene(1001);
		break;
	}
}

void Scene2000::Action7::signal() {
	Scene2000 *scene = (Scene2000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(30);
		break;
	case 1:
		scene->_stripManager.start(2072, this);
		break;
	case 2:
		setDelay(3);
		break;
	case 3:
		g_globals->_sceneManager.changeScene(2222);
		break;
	}
}

void Scene2000::Action8::signal() {
	Scene2000 *scene = (Scene2000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		error("Old stuff");
		break;
	case 1:
		scene->_stripManager.start(2073, this);
		break;
	case 2:
		setDelay(10);
		break;
	case 3:
		g_globals->_stripNum = 2005;
		g_globals->_sceneManager.changeScene(1000);
		break;
	}
}

void Scene2000::Action9::signal() {
	Scene2000 *scene = (Scene2000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		error("Old stuff");
		break;
	case 1:
		scene->_stripManager.start(2074, this);
		break;
	case 2:
		setDelay(3);
		break;
	case 3:
		g_globals->_stripNum = 2008;
		g_globals->_sceneManager.changeScene(9999);
		break;
	}
}

void Scene2000::Action10::signal() {
	switch (_actionIndex++) {
	case 0:
		setDelay(30);
		break;
	case 1:
		error("Old stuff");
		break;
	case 2:
		SceneItem::display(2000, 17, SET_Y, 20, SET_X, 110, SET_FONT, 2, SET_BG_COLOR, -1,
			SET_FG_COLOR, 17, SET_WIDTH, 200, SET_POS_MODE, 0, SET_KEEP_ONSCREEN, 1, LIST_END);
		break;
	case 3:
		SceneItem::display(0, 0);
		g_globals->_stripNum = 0;
		g_globals->_sceneManager.changeScene(9999);
		break;
	}
}

void Scene2000::Action11::signal() {
	Scene2000 *scene = (Scene2000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		error("Old stuff");
		break;
	case 1:
		scene->_stripManager.start(2076, this);
		break;
	case 2:
		scene->_stripManager.start(2077, this);
		break;
	case 3:
		g_globals->_stripNum = 0;
		g_globals->_sceneManager.changeScene(1400);
		break;
	}
}

void Scene2000::Action12::signal() {
	Scene2000 *scene = (Scene2000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(60);
		break;
	case 1:
		g_globals->_events.setCursor(CURSOR_WALK);
		scene->_stripManager.start(2020, this);
		break;
	case 2:
		g_globals->_player.disableControl();
		setDelay(10);
		break;
	case 3:
		g_globals->_sceneManager.changeScene(2300);
		break;
	}
}

void Scene2000::Action13::signal() {
	Scene2000 *scene = (Scene2000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(30);
		break;
	case 1:
		scene->_stripManager.start(2078, this);
		break;
	case 2:
		SceneItem::display(0, 0);
		g_globals->_stripNum = 2751;
		g_globals->_sceneManager.changeScene(1500);
		break;
	}
}

void Scene2000::Action14::signal() {
	Scene2000 *scene = (Scene2000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		setDelay(30);
		break;
	case 1:
		scene->_stripManager.start(2070, this);
		break;
	case 2:
		setDelay(60);
		break;
	case 3:
		g_globals->_soundHandler.play(99);
		scene->_object8.show();
		scene->_object8.animate(ANIM_MODE_5, this);
		break;
	case 4:
		g_globals->_soundHandler.play(12);
		scene->_object8.setStrip(2);
		scene->_object8.setFrame(1);
		scene->_object9.show();
		scene->_object10.show();
		setDelay(60);
		break;
	case 5:
		scene->_stripManager.start(2001, this, scene);
		break;
	case 6:
		g_globals->_soundHandler.fadeOut(NULL);
		scene->_object8.setStrip(1);
		scene->_object8.setFrame(scene->_object8.getFrameCount());
		scene->_object8.animate(ANIM_MODE_6, this);

		scene->_object9.remove();
		scene->_object10.remove();
		break;
	case 7:
		g_globals->_soundHandler.play(111);
		scene->_object8.remove();
		setDelay(5);
		break;
	case 8:
		scene->_stripManager.start(2071, this);
		break;
	case 9:
		g_globals->_stripNum = 1250;
		g_globals->_sceneManager.changeScene(1000);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene2000::postInit(SceneObjectList *OwnerList) {
	loadScene(2000);
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);

	_stripManager.addSpeaker(&_speakerQR);
	_stripManager.addSpeaker(&_speakerSL);
	_stripManager.addSpeaker(&_speakerMR);
	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerMText);
	_stripManager.addSpeaker(&_speakerSText);
	_stripManager.addSpeaker(&_speakerGameText);
	_stripManager.addSpeaker(&_speakerHText);

	_speakerQText._npc = &_object2;
	_speakerSText._npc = &_object3;
	_speakerMText._npc = &_object6;
	_speakerHText._npc = &_object6;
	_stripManager.setCallback(this);

	_object3.postInit();
	_object3.setVisage(2002);
	_object3.setPosition(Common::Point(65, 199));

	_object4.postInit();
	_object4.setVisage(2002);
	_object4.setStrip(2);
	_object4.setPosition(Common::Point(125, 199));
	_object4.setAction(&_action4);

	_object2.postInit();
	_object2.setVisage(2001);
	_object2.setPosition(Common::Point(43, 115));
	_object2.setAction(&_action2);

	g_globals->_player.disableControl();

	_object6.postInit();
	_object6.setVisage(2003);
	_object6.setPosition(Common::Point(267, 170));
	_object6.setAction(&_action3);

	_object8.postInit();
	_object8.setVisage(2005);
	_object8.setPosition(Common::Point(169, 133));
	_object8.setPriority(133);
	_object8.hide();

	_object9.postInit();
	_object9.setVisage(2005);
	_object9.setStrip2(3);
	_object9.setFrame(4);
	_object9.setPosition(Common::Point(136, 86));
	_object9.fixPriority(190);
	_object9.hide();

	_object10.postInit();
	_object10.setVisage(2005);
	_object10.setStrip2(5);
	_object10.setFrame(4);
	_object10.setPosition(Common::Point(202, 86));
	_object10.fixPriority(195);
	_object10.hide();

	switch (g_globals->_sceneManager._previousScene) {
	case 1000:
		setAction(&_action7);
		break;
	case 1001:
		_object6.remove();
		setAction(&_action12);
		break;
	case 1500:
		setAction(&_action13);
		break;
	case 2200:
		g_globals->_soundHandler.play(111);
		setAction(&_action14);
		break;
	case 2222:
		g_globals->_soundHandler.play(115);
		setAction(&_action8);
		break;
	case 3500:
		setAction(&_action11);
		break;
	default:
		_object6.remove();
		g_globals->_soundHandler.play(80);
		setAction(&_action6);
		break;
	}

	_soundHandler1.play(78);
	g_globals->_sceneManager._scene->_sceneBounds.contain(g_globals->_sceneManager._scene->_backgroundBounds);
	g_globals->_sceneOffset.x = (g_globals->_sceneManager._scene->_sceneBounds.left / 160) * 160;
}

void Scene2000::stripCallback(int v) {
	switch (v) {
	case 0:
		_object9.setStrip(3);
		_object9.animate(ANIM_MODE_7, 0, NULL);
		_object10.setStrip(6);
		_object10.setFrame(1);
		_object10.animate(ANIM_MODE_5, NULL);
		break;
	case 1:
		_object10.setStrip(5);
		_object10.animate(ANIM_MODE_7, 0, NULL);
		_object9.setStrip(4);
		_object9.setFrame(1);
		_object9.animate(ANIM_MODE_5, NULL);
		break;
	case 2:
		_object9.animate(ANIM_MODE_NONE, NULL);
		break;
	case 3:
		_object10.setStrip(6);
		_object10.setFrame(1);
		_object10.animate(ANIM_MODE_5, NULL);
		_object9.setStrip(4);
		_object9.setFrame(1);
		_object9.animate(ANIM_MODE_5, NULL);
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 2100 - Starcraft Cockpit
 *
 *--------------------------------------------------------------------------*/

void Scene2100::Action1::signal() {
	Scene2100 *scene = (Scene2100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		if (!scene->_sitFl)
			setDelay(1);
		else {
			setAction(&scene->_sequenceManager, this, 2102, &g_globals->_player, NULL);
			scene->_sitFl = 0;
		}
		break;
	case 1: {
		Common::Point pt(157, 62);
		PlayerMover *mover = new PlayerMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 2:
		scene->_soundHandler.play(162);
		scene->_object1.animate(ANIM_MODE_5, this);
		break;
	case 3: {
		Common::Point pt(157, 56);
		NpcMover *mover = new NpcMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 4:
		g_globals->_player._strip = 3;
		setDelay(3);
		break;
	case 5:
		g_globals->_player.fixPriority(1);
		scene->_area1.display();
		scene->_area2.display();
		scene->_area3.display();
		scene->_area4.display();

		scene->_area1.draw(true);
		_state = 0;
		g_globals->_events.setCursor(CURSOR_USE);

		while (!_state && !g_vm->shouldQuit()) {
			// Wait for an event
			Event event;
			if (!g_globals->_events.getEvent(event)) {
				GLOBALS._screenSurface.updateScreen();
				g_system->delayMillis(10);
				continue;
			}

			if (scene->_area1._bounds.contains(event.mousePos)) {
				scene->_area1.draw(true);
				_state = scene->_area1._actionId;
			}
			if (scene->_area2._bounds.contains(event.mousePos)) {
				scene->_area1.draw(false);
				scene->_area2.draw(true);
				_state = scene->_area2._actionId;
			}
			if (scene->_area3._bounds.contains(event.mousePos)) {
				scene->_area1.draw(false);
				scene->_area3.draw(true);
				_state = scene->_area3._actionId;
			}
		}

		scene->_soundHandler.play(161);
		scene->_area1.restore();
		scene->_area2.restore();
		scene->_area3.restore();
		scene->_area4.restore();

		if (_state == 2100) {
			setDelay(1);
		} else {
			scene->_soundHandler.play(162);
			scene->_object1.animate(ANIM_MODE_6, this);
		}
		break;
	case 6:
		if (_state == 2100) {
			Common::Point pt(157, 65);
			NpcMover *mover = new NpcMover();
			g_globals->_player.addMover(mover, &pt, this);
			break;
		} else {
			g_globals->_sceneManager.changeScene(_state);
		}
		break;
	case 7:
		g_globals->_player.fixPriority(-1);
		scene->_soundHandler.play(162);
		scene->_object1.animate(ANIM_MODE_6, this);
		break;
	case 8:
		g_globals->_player.enableControl();
		remove();
		break;
	}
}

void Scene2100::Action2::signal() {
	Scene2100 *scene = (Scene2100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_object3._numFrames = 5;
		setDelay(g_globals->_randomSource.getRandomNumber(59));
		break;
	case 1:
		scene->_object3.animate(ANIM_MODE_5, this);
		break;
	case 2:
		setDelay(g_globals->_randomSource.getRandomNumber(59));
		break;
	case 3:
		scene->_object3.animate(ANIM_MODE_6, this);
		_actionIndex = 0;
		break;
	}
}

void Scene2100::Action3::signal() {
	Scene2100 *scene = (Scene2100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
	case 2:
		setDelay(g_globals->_randomSource.getRandomNumber(119));
		break;
	case 1:
		scene->_object2.animate(ANIM_MODE_5, this);
		break;
	case 3:
		scene->_object2.animate(ANIM_MODE_6, this);
		_actionIndex = 0;
		break;
	}
}

void Scene2100::Action4::signal() {
	Scene2100 *scene = (Scene2100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		if (!scene->_sitFl)
			setDelay(1);
		else
			setAction(&scene->_sequenceManager, this, 2102, &g_globals->_player, NULL);
		break;
	case 1: {
		Common::Point pt(80, 66);
		PlayerMover *mover = new PlayerMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 2:
		g_globals->_player.setVisage(2109);
		g_globals->_player._frame = 1;
		g_globals->_player._strip = 2;
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 3:
		g_globals->_sceneManager.changeScene(2120);
		break;
	}
}

void Scene2100::Action5::signal() {
	// Quinn enters the cokpit after Seeker decided to enter the cave alone
	Scene2100 *scene = (Scene2100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(30);
		break;
	case 1:
		setAction(&scene->_sequenceManager, this, 2104, &g_globals->_player, &scene->_object1, NULL);
		break;
	case 2: {
		Common::Point pt(272, 127);
		NpcMover *mover = new NpcMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 3:
		g_globals->_player.checkAngle(&scene->_object3);
		setDelay(30);
		break;
	case 4:
		g_globals->_sceneManager.changeScene(3700);
		break;
	}
}

void Scene2100::Action6::signal() {
	// Seeker stands up and walks to the elevator
	Scene2100 *scene = (Scene2100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		scene->_object2.setVisage(2806);
		scene->_object2.setStrip(1);
		scene->_object2.setStrip2(-1);
		scene->_object2.changeZoom(-1);
		scene->_object2.setPosition(Common::Point(155, 116));
		scene->_object2.setObjectWrapper(new SceneObjectWrapper());
		scene->_object2.setAction(NULL);
		scene->_object2.animate(ANIM_MODE_1, NULL);

		Common::Point pt(130, 116);
		NpcMover *mover = new NpcMover();
		scene->_object2.addMover(mover, &pt, this);
		break;
	}
	case 1: {
		scene->_object2.fixPriority(-1);
		Common::Point pt(153, 67);
		NpcMover *mover = new NpcMover();
		scene->_object2.addMover(mover, &pt, this);
		break;
	}
	case 2:
		remove();
		break;
	}
}

void Scene2100::Action7::signal() {
	Scene2100 *scene = (Scene2100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(60);
		break;
	case 1:
		setAction(&scene->_sequenceManager, this, 2104, &g_globals->_player, &scene->_object1, NULL);
		break;
	case 2:
		setAction(&scene->_sequenceManager, this, 2104, &g_globals->_player, NULL);
		break;
	case 3:
		g_globals->_sceneManager.changeScene(8100);
		break;
	}
}

void Scene2100::Action8::signal() {
	Scene2100 *scene = (Scene2100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(60);
		break;
	case 1:
		setAction(&scene->_sequenceManager, this, 2104, &g_globals->_player, &scene->_object1, NULL);
		break;
	case 2: {
		Common::Point pt(200, 174);
		PlayerMover *mover = new PlayerMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 3:
		g_globals->_player.checkAngle(&scene->_object3);
		scene->_stripManager.start((RING_INVENTORY._translator._sceneNumber == 1) ? 7720 : 7710, this);
		break;
	case 4:
		if (RING_INVENTORY._translator._sceneNumber != 1)
			g_globals->_sceneManager.changeScene(7600);
		else {
			g_globals->setFlag(24);
			g_globals->_player.enableControl();
			remove();
		}
		break;
	}
}

void Scene2100::Action9::signal() {
	Scene2100 *scene = (Scene2100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(60);
		break;
	case 1:
		scene->_stripManager.start(6050, this);
		break;
	case 2:
		scene->_soundHandler.play(99);
		scene->_object4.show();
		scene->_object4.animate(ANIM_MODE_5, this);
		break;
	case 3:
		scene->_soundHandler.play(12);
		scene->_object4.setStrip(2);
		scene->_stripManager.start(6051, this, scene);
		break;
	case 4:
		scene->_soundHandler.fadeOut(NULL);
		scene->_object4.setStrip(1);
		scene->_object4.setFrame(scene->_object4.getFrameCount());
		scene->_object4.animate(ANIM_MODE_6, this);
		break;
	case 5:
		scene->_object4.hide();
		g_globals->_events.setCursor(CURSOR_WALK);
		scene->_stripManager.start(6010, this);
		break;
	case 6:
		if (scene->_stripManager._field2E8 != 165)
			setAction(&scene->_action10);
		else
			setAction(&scene->_action11);
		break;
	}
}

void Scene2100::Action10::signal() {
	Scene2100 *scene = (Scene2100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(60);
		break;
	case 1:
		setAction(&scene->_sequenceManager, this, 2102, &g_globals->_player, NULL);
		break;
	case 2: {
		g_globals->_player.disableControl();
		Common::Point pt(155, 64);
		PlayerMover *mover = new PlayerMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 3:
		scene->_object3.setVisage(2105);
		scene->_object3.setStrip(2);
		scene->_object3.setFrame(1);
		scene->_object3._numFrames = 10;
		scene->_object3.setAction(NULL);
		scene->_object3.animate(ANIM_MODE_5, this);
		break;
	case 4: {
		scene->_object3.setVisage(2705);
		scene->_object3.setStrip2(-1);
		scene->_object3.changeZoom(-1);
		scene->_object3.fixPriority(-1);
		scene->_object3.setPosition(Common::Point(260, 156));
		scene->_object3.setObjectWrapper(new SceneObjectWrapper());
		scene->_object3.animate(ANIM_MODE_1, NULL);

		Common::Point pt(166, 64);
		PlayerMover *mover = new PlayerMover();
		scene->_object3.addMover(mover, &pt, this);

		setAction(&scene->_action6, NULL);
		break;
	}
	case 5:
		scene->_soundHandler.play(162);
		scene->_object1.animate(ANIM_MODE_5, this);
		break;
	case 6: {
		g_globals->_player.fixPriority(1);
		Common::Point pt(144, 54);
		NpcMover *mover = new NpcMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 7: {
		scene->_object3.fixPriority(2);
		Common::Point pt1(163, 55);
		NpcMover *mover1 = new NpcMover();
		scene->_object3.addMover(mover1, &pt1, NULL);

		scene->_object2.fixPriority(2);
		Common::Point pt2(158, 55);
		NpcMover *mover2 = new NpcMover();
		scene->_object2.addMover(mover2, &pt2, this);
		break;
	}
	case 8:
		g_globals->_player.fixPriority(1);
		g_globals->_player.setStrip(1);
		scene->_object3.fixPriority(1);
		scene->_object3.setStrip(2);
		scene->_object2.fixPriority(2);
		scene->_object2.setStrip(3);

		setDelay(45);
		break;
	case 9:
		scene->_soundHandler.play(162);
		scene->_object1.animate(ANIM_MODE_6, this);
		break;
	case 10:
		g_globals->setFlag(70);
		g_globals->_stripNum = 2101;
		g_globals->_sceneManager.changeScene(2320);
		break;
	}
}

void Scene2100::Action11::signal() {
	// Miranda stands up and walks to the elevator
	Scene2100 *scene = (Scene2100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(60);
		break;
	case 1:
		scene->_object3._numFrames = 10;
		scene->_object3.setAction(NULL);
		scene->_object3.setVisage(2105);
		scene->_object3.setStrip(2);
		scene->_object3.setFrame(1);
		scene->_object3.animate(ANIM_MODE_5, this);
		break;
	case 2: {
		scene->_object3.setVisage(2705);
		scene->_object3.setStrip2(-1);
		scene->_object3.changeZoom(-1);
		scene->_object3.fixPriority(-1);
		scene->_object3.setPosition(Common::Point(260, 156));
		scene->_object3.setObjectWrapper(new SceneObjectWrapper());
		scene->_object3.animate(ANIM_MODE_1, NULL);

		Common::Point pt(158, 62);
		PlayerMover *mover = new PlayerMover();
		scene->_object3.addMover(mover, &pt, this);

		setAction(&scene->_action6, NULL);
		break;
	}
	case 3:
		scene->_soundHandler.play(162);
		scene->_object1.animate(ANIM_MODE_5, this);
		break;
	case 4: {
		scene->_object3.fixPriority(1);
		Common::Point pt1(163, 55);
		NpcMover *mover1 = new NpcMover();
		scene->_object3.addMover(mover1, &pt1, NULL);

		scene->_object2.fixPriority(1);
		Common::Point pt2(158, 55);
		NpcMover *mover2 = new NpcMover();
		scene->_object2.addMover(mover2, &pt2, this);
		break;
	}
	case 5:
		scene->_object3.setStrip(2);
		scene->_object2.setStrip(3);
		setDelay(45);
		break;
	case 6:
		scene->_soundHandler.play(162);
		scene->_object1.animate(ANIM_MODE_6, this);
		break;
	case 7:
		scene->_object3.remove();
		scene->_object2.remove();
		g_globals->setFlag(70);
		g_globals->_stripNum = 2102;
		g_globals->_player.enableControl();
		g_globals->_player._canWalk = false;
		break;
	}
}

void Scene2100::Action12::signal() {
	Scene2100 *scene = (Scene2100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(10);
		break;
	case 1:
		scene->_stripManager.start(6000, this);
		break;
	case 2:
		scene->_soundHandler.play(162);
		scene->_object1.animate(ANIM_MODE_5, this);
		break;
	case 3: {
		Common::Point pt1(158, 74);
		NpcMover *mover1 = new NpcMover();
		g_globals->_player.addMover(mover1, &pt1, this);

		Common::Point pt2(158, 68);
		NpcMover *mover2 = new NpcMover();
		scene->_object2.addMover(mover2, &pt2, NULL);
		break;
	}
	case 4: {
		scene->_soundHandler.play(162);
		scene->_object1.animate(ANIM_MODE_6, NULL);

		g_globals->_player.fixPriority(-1);
		Common::Point pt1(277, 84);
		PlayerMover *mover1 = new PlayerMover();
		g_globals->_player.addMover(mover1, &pt1, this);

		scene->_object2.fixPriority(-1);
		Common::Point pt2(255, 76);
		PlayerMover *mover2 = new PlayerMover();
		scene->_object2.addMover(mover2, &pt2, this);
		break;
	}
	case 6:
		g_globals->_player.setStrip(4);
		scene->_object2.setStrip(4);
		setDelay(60);
		break;
	case 7:
		g_globals->_events.setCursor(CURSOR_WALK);
		scene->_stripManager.start(6052, this);
		break;
	case 8:
		if (scene->_stripManager._field2E8 == 320)
			g_globals->setFlag(74);
		setDelay(30);
		break;
	case 9:
		g_globals->_events.setCursor(OBJECT_STUNNER);
		scene->_object2.setAction(&scene->_action13);
		setDelay(60);
		break;
	case 10:
		if (g_globals->getFlag(74))
			setDelay(1);
		else
			setAction(&scene->_sequenceManager, this, 2101, &g_globals->_player, NULL);
		break;
	case 11:
		scene->_stripManager.start(2170, this);
		break;
	case 12:
		setDelay(5);
		break;
	case 13:
		scene->_stripManager.start(g_globals->getFlag(74) ? 2172 : 2174, this);
		break;
	case 14:
		if (g_globals->getFlag(74)) {
			g_globals->_stripNum = 6100;
			g_globals->_sceneManager.changeScene(2320);
		} else {
			g_globals->_sceneManager.changeScene(6100);
		}
		remove();
		break;
	}
}

void Scene2100::Action13::signal() {
	Scene2100 *scene = (Scene2100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(6);
		break;
	case 1: {
		scene->_object2.fixPriority(113);
		Common::Point pt(178, 116);
		PlayerMover *mover = new PlayerMover();
		scene->_object2.addMover(mover, &pt, this);
		break;
	}
	case 2: {
		Common::Point pt(150, 116);
		NpcMover *mover = new NpcMover();
		scene->_object2.addMover(mover, &pt, this);
		break;
	}
	case 3:
		scene->_object2.setVisage(2108);
		scene->_object2._strip = 3;
		scene->_object2.setPosition(Common::Point(150, 100));
		scene->_object2.animate(ANIM_MODE_NONE, NULL);
		scene->_object2.changeZoom(100);
		scene->_object2.setAction(&scene->_action3);
		setDelay(15);
		break;
	case 4:
		remove();
		break;
	}
}

void Scene2100::Action14::signal() {
	Scene2100 *scene = (Scene2100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(6);
		break;
	case 1:
		setAction(&scene->_sequenceManager, this, 2104, &g_globals->_player, &scene->_object1, NULL);
		break;
	case 2:
		setAction(&scene->_sequenceManager, this, 2101, &g_globals->_player, NULL);
		break;
	case 3:
		scene->_stripManager.start(6008, this);
		break;
	case 4:
		scene->_soundHandler.play(99);
		scene->_object4.show();
		scene->_object4.animate(ANIM_MODE_5, this);
		break;
	case 5:
		scene->_object4.setStrip(2);
		scene->_stripManager.start(6009, this, scene);
		break;
	case 6:
		scene->_soundHandler.fadeOut(NULL);
		scene->_object4.setStrip(1);
		scene->_object4.setFrame(scene->_object4.getFrameCount());
		scene->_object4.animate(ANIM_MODE_6, this);
		break;
	case 7:
		scene->_stripManager.start(6060, this);
		break;
	case 8:
		scene->_object3._numFrames = 10;
		scene->_object3.setAction(NULL);
		scene->_object3.setVisage(2105);
		scene->_object3.setStrip(2);
		scene->_object3.setFrame(1);
		scene->_object3.animate(ANIM_MODE_5, this);
		break;
	case 9: {
		scene->_object3.setVisage(2705);
		scene->_object3.setStrip2(-1);
		scene->_object3.changeZoom(-1);
		scene->_object3.fixPriority(-1);
		scene->_object3.setPosition(Common::Point(260, 156));
		scene->_object3.setObjectWrapper(new SceneObjectWrapper());
		scene->_object3.animate(ANIM_MODE_1, NULL);

		Common::Point pt(157, 65);
		PlayerMover *mover = new PlayerMover();
		scene->_object3.addMover(mover, &pt, this);
		break;
	}
	case 10:
		scene->_soundHandler.play(162);
		scene->_object1.animate(ANIM_MODE_5, this);
		break;
	case 11: {
		Common::Point pt(159, 51);
		NpcMover *mover = new NpcMover();
		scene->_object3.addMover(mover, &pt, this);
		break;
	}
	case 12:
		scene->_object3.setStrip(2);
		setDelay(30);
		break;
	case 13:
		scene->_object3.fixPriority(1);
		scene->_soundHandler.play(162);
		scene->_object1.animate(ANIM_MODE_6, this);
		break;
	case 14:
		setDelay(90);
		break;
	case 15:
		g_globals->_sceneManager.changeScene(7000);
		remove();
		break;
	}
}

void Scene2100::Action15::signal() {
	Scene2100 *scene = (Scene2100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(6);
		break;
	case 1:
		scene->_object3.postInit();
		scene->_object3.setVisage(2705);
		scene->_object3.animate(ANIM_MODE_1, NULL);
		scene->_object3.setObjectWrapper(new SceneObjectWrapper());
		scene->_object3.setPosition(Common::Point(157, 56));
		scene->_object3.fixPriority(1);
		scene->_object3.changeZoom(-1);

		scene->_soundHandler.play(162);
		scene->_object1.animate(ANIM_MODE_5, this);
		break;
	case 2: {
		scene->_object3.fixPriority(-1);
		Common::Point pt(177, 68);
		NpcMover *mover = new NpcMover();
		scene->_object3.addMover(mover, &pt, this);
		break;
	}
	case 3: {
		scene->_soundHandler.play(162);
		scene->_object1.animate(ANIM_MODE_6, this);

		Common::Point pt(272, 140);
		NpcMover *mover = new NpcMover();
		scene->_object3.addMover(mover, &pt, this);
		break;
	}
	case 4: {
		Common::Point pt(266, 150);
		NpcMover *mover = new NpcMover();
		scene->_object3.addMover(mover, &pt, this);
		break;
	}
	case 5: {
		scene->_object3.fixPriority(156);

		Common::Point pt(260, 156);
		NpcMover *mover = new NpcMover();
		scene->_object3.addMover(mover, &pt, this);
		break;
	}
	case 6:
		scene->_object3.setVisage(2105);
		scene->_object3._strip = 1;
		scene->_object3._frame = 1;
		scene->_object3.setPosition(Common::Point(256, 156));
		scene->_object3.animate(ANIM_MODE_5, this);
		scene->_object3.changeZoom(100);

		scene->_object3.animate(ANIM_MODE_NONE, NULL);
		break;
	case 7:
		remove();
		break;
	}
}

void Scene2100::Action16::signal() {
	Scene2100 *scene = (Scene2100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
	case 4:
		setDelay(3);
		break;
	case 1:
		scene->_stripManager.start(7001, this);
		break;
	case 2:
		setAction(&scene->_sequenceManager, this, 2102, &g_globals->_player, NULL);
		break;
	case 3: {
		g_globals->_player.disableControl();
		Common::Point pt(155, 63);
		PlayerMover *mover = new PlayerMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 5:
		scene->_soundHandler.play(162);
		scene->_object1.animate(ANIM_MODE_5, this);
		break;
	case 6: {
		Common::Point pt(160, 54);
		NpcMover *mover = new NpcMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 7:
		g_globals->_player.fixPriority(1);
		g_globals->_player.setStrip(3);
		setDelay(45);
		break;
	case 8:
		scene->_soundHandler.play(162);
		scene->_object1.animate(ANIM_MODE_6, this);
		break;
	case 9:
		g_globals->setFlag(15);
		g_globals->setFlag(36);
		g_globals->_sceneManager.changeScene(7000);
		remove();
		break;
	}
}

void Scene2100::Action17::signal() {
	Scene2100 *scene = (Scene2100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(60);
		break;
	case 1:
		setAction(&scene->_sequenceManager, this, 2104, &g_globals->_player, &scene->_object1, NULL);
		break;
	case 2:
		setAction(&scene->_sequenceManager, this, 2101, &g_globals->_player, NULL);
		break;
	case 3:
		scene->_stripManager.start(7070, this);
		break;
	case 4:
		scene->_soundHandler.play(99);
		scene->_object4.show();
		scene->_object4.animate(ANIM_MODE_5, this);
		break;
	case 5:
		scene->_soundHandler.play(12);
		scene->_object4.setStrip(2);
		scene->_stripManager.start(7071, this, scene);
		break;
	case 6:
		scene->_soundHandler.fadeOut(NULL);
		scene->_object4.setStrip(1);
		scene->_object4.setFrame(scene->_object4.getFrameCount());
		scene->_object4.animate(ANIM_MODE_6, this);
		break;
	case 7:
		scene->_stripManager.start(7072, this);
		break;
	case 8:
		RING_INVENTORY._stasisNegator._sceneNumber = 1;
		g_globals->_sceneManager.changeScene(9100);
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene2100::Hotspot2::doAction(int action) {
	Scene2100 *scene = (Scene2100 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2100, 3);
		break;
	case CURSOR_USE:
		if (g_globals->getFlag(13))
			SceneItem::display2(2100, 29);
		else {
			g_globals->_player.disableControl();
			scene->setAction(&scene->_action4);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2100::Hotspot3::doAction(int action) {
	// Computer, on the left
	Scene2100 *scene = (Scene2100 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2100, 4);
		break;
	case CURSOR_USE:
		if (g_globals->getFlag(13))
			SceneItem::display2(2100, 29);
		else {
			g_globals->_player.disableControl();
			scene->setAction(&scene->_action4);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2100::Hotspot4::doAction(int action) {
	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2100, 5);
		break;
	case CURSOR_USE:
		SceneItem::display2(2100, 6);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2100::Hotspot8::doAction(int action) {
	Scene2100 *scene = (Scene2100 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2100, 12);
		break;
	case CURSOR_USE:
		if (g_globals->getFlag(13))
			SceneItem::display2(2100, 29);
		else {
			g_globals->_player.disableControl();
			scene->setAction(&scene->_action4);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2100::Hotspot10::doAction(int action) {
	// Quinn's Console
	Scene2100 *scene = (Scene2100 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2100, 13);
		break;
	case CURSOR_USE:
		if (scene->_sitFl) {
			g_globals->_player.disableControl();
			scene->_sceneMode = 2102;
			scene->setAction(&scene->_sequenceManager, scene, 2102, &g_globals->_player, NULL);
		} else if (g_globals->getFlag(13)) {
			SceneItem::display2(2100, 28);
		} else {
			g_globals->_player.disableControl();
			scene->_sceneMode = 2101;
			scene->setAction(&scene->_sequenceManager, scene, 2101, &g_globals->_player, NULL);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2100::Hotspot14::doAction(int action) {
	switch (action) {
	case CURSOR_LOOK:
		if (g_globals->getFlag(0))
			SceneItem::display2(2100, 19);
		else
			SceneItem::display2(2100, 18);
		break;
	case CURSOR_USE:
		if (g_globals->getFlag(1))
			SceneItem::display2(2100, 21);
		else
			SceneItem::display2(2100, 20);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2100::Object1::doAction(int action) {
	// Elevator
	Scene2100 *scene = (Scene2100 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2100, 1);
		break;
	case CURSOR_USE:
		scene->setAction(&scene->_action1);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2100::Object2::doAction(int action) {
	Scene2100 *scene = (Scene2100 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2100, 30);
		break;
	case CURSOR_TALK:
		if (g_globals->getFlag(72)) {
			g_globals->_player.disableControl();
			if (!g_globals->getFlag(52)) {
				scene->_sceneMode = 2111;
				scene->setAction(&scene->_sequenceManager, scene, 2111, NULL);
			} else {
				scene->_sceneMode = g_globals->getFlag(53) ? 2112 : 2110;
				scene->setAction(&scene->_sequenceManager, scene, scene->_sceneMode, NULL);
			}
		} else if (g_globals->getFlag(13)) {
				SceneItem::display2(2100, 31);
		} else if (g_globals->getFlag(14)) {
				SceneItem::display2(2100, 32);
		} else {
				g_globals->setFlag(14);
				g_globals->_player.disableControl();
				scene->_sceneMode = 2108;
				scene->setAction(&scene->_sequenceManager, scene, 2109, NULL);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2100::Object3::doAction(int action) {
	// Miranda
	Scene2100 *scene = (Scene2100 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		if (g_globals->getFlag(59))
			SceneItem::display2(2100, 34);
		else
			error("***I have no response.");
		break;

	case CURSOR_TALK:
		if (g_globals->getFlag(59)) {
			g_globals->_player.disableControl();
			scene->_sceneMode = 2108;
			scene->setAction(&scene->_sequenceManager, scene, 2108, NULL);
		} else {
			error("***I have no response.");
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene2100::Scene2100() :
		_hotspot1(0, CURSOR_LOOK, 2100, 2, LIST_END),
		_hotspot5(0, CURSOR_LOOK, 2100, 9, LIST_END),
		_hotspot6(0, CURSOR_LOOK, 2100, 7, CURSOR_USE, 2100, 8, LIST_END),
		_hotspot7(0, CURSOR_LOOK, 2100, 7, CURSOR_USE, 2100, 11, LIST_END),
		_hotspot9(0, CURSOR_LOOK, 2100, 14, LIST_END),
		_hotspot11(0, CURSOR_LOOK, 2100, 15, CURSOR_USE, 2100, 16, LIST_END),
		_hotspot12(0, CURSOR_LOOK, 2100, 24, CURSOR_USE, 2100, 25, LIST_END),
		_hotspot13(0, CURSOR_LOOK, 2100, 17, LIST_END),
		_hotspot15(0, CURSOR_LOOK, 2100, 22, CURSOR_USE, 2100, 23, LIST_END) {
	_area1.setup(2153, 2, 1, 2100);
	_area1._pt = Common::Point(200, 31);
	_area2.setup(2153, 3, 1, 2150);
	_area2._pt = Common::Point(200, 50);
	_area3.setup(2153, 4, 1, 2320);
	_area3._pt = Common::Point(200, 75);
	_area4.setup(2153, 1, 1, OBJECT_TRANSLATOR);
	_area4._pt = Common::Point(237, 77);
}

void Scene2100::postInit(SceneObjectList *OwnerList) {
	loadScene(2100);
	Scene::postInit();
	setZoomPercents(60, 80, 200, 100);

	_stripManager.addSpeaker(&_speakerMText);
	_stripManager.addSpeaker(&_speakerMR);
	_stripManager.addSpeaker(&_speakerQL);
	_stripManager.addSpeaker(&_speakerQR);
	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerSText);
	_stripManager.addSpeaker(&_speakerSL);
	_stripManager.addSpeaker(&_speakerSAL);
	_stripManager.addSpeaker(&_speakerHText);
	_stripManager.addSpeaker(&_speakerGameText);
	_speakerMText._npc = &_object3;
	_speakerQText._npc = &g_globals->_player;
	_speakerSText._npc = &_object2;

	_object1.postInit();
	_object1.setVisage(2100);
	_object1.animate(ANIM_MODE_NONE, NULL);
	_object1.setPosition(Common::Point(157, 57));
	_object1.fixPriority(5);

	_hotspot3.postInit();
	_hotspot3.setVisage(2101);
	_hotspot3._frame = 1;
	_hotspot3.animate(ANIM_MODE_2, NULL);
	_hotspot3.setPosition(Common::Point(53, 44));
	_hotspot3.changeZoom(100);
	_hotspot3.fixPriority(1);

	_hotspot4.postInit();
	_hotspot4.setVisage(2101);
	_hotspot4._frame = 1;
	_hotspot4._strip = 3;
	_hotspot4.animate(ANIM_MODE_8, 0, NULL);
	_hotspot4.setPosition(Common::Point(274, 52));
	_hotspot4.changeZoom(100);
	_hotspot4.fixPriority(1);

	_hotspot5.postInit();
	_hotspot5.setVisage(2101);
	_hotspot5._frame = 1;
	_hotspot5._strip = 4;
	_hotspot5.animate(ANIM_MODE_8, 0, NULL);
	_hotspot5.setPosition(Common::Point(219, 141));
	_hotspot5.changeZoom(100);
	_hotspot5.fixPriority(160);

	_hotspot6.postInit();
	_hotspot6.setVisage(2101);
	_hotspot6._frame = 1;
	_hotspot6._strip = 5;
	_hotspot6.fixPriority(175);
	_hotspot6.animate(ANIM_MODE_8, 0, NULL);
	_hotspot6.setPosition(Common::Point(97, 142));
	_hotspot6.changeZoom(100);

	_hotspot7.postInit();
	_hotspot7.setVisage(2101);
	_hotspot7._frame = 1;
	_hotspot7._strip = 6;
	_hotspot7.animate(ANIM_MODE_NONE, NULL);
	_hotspot7.setPosition(Common::Point(133, 46));
	_hotspot7.changeZoom(100);
	_hotspot7.fixPriority(1);

	_hotspot8.postInit();
	_hotspot8.setVisage(2101);
	_hotspot8._frame = 1;
	_hotspot8._strip = 7;
	_hotspot8.animate(ANIM_MODE_8, 0, NULL);
	_hotspot8.setPosition(Common::Point(20, 45));
	_hotspot8.changeZoom(100);
	_hotspot8.fixPriority(1);

	_hotspot2.postInit();
	_hotspot2.setVisage(2101);
	_hotspot2._frame = 1;
	_hotspot2._strip = 8;
	_hotspot2.animate(ANIM_MODE_8, 0, NULL);
	_hotspot2.setPosition(Common::Point(88, 41));
	_hotspot2.changeZoom(100);
	_hotspot2.fixPriority(1);

	_hotspot11.setBounds(Rect(139, 74, 173, 96));
	_hotspot10.setBounds(Rect(71, 100, 91, 135));
	_hotspot9.setBounds(Rect(225, 110, 251, 136));
	_hotspot14.setBounds(Rect(100, 97, 216, 130));
	_hotspot13.setBounds(Rect(13, 124, 94, 168));
	_hotspot12.setBounds(Rect(217, 141, 307, 155));
	_hotspot15.setBounds(Rect(14, 90, 46, 107));
	_hotspot1.setBounds(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));

	if (!g_globals->getFlag(36) && !g_globals->getFlag(70) && !g_globals->getFlag(43)) {
		_object3.postInit();
		_object3.setPosition(Common::Point(246, 156));
		_object3.animate(ANIM_MODE_NONE, NULL);
		_object3.changeZoom(100);
		_object3.fixPriority(156);
		_object3.setVisage(2107);
		_object3.setStrip(1);
		_object3.setAction(&_action2);
		g_globals->_sceneItems.push_back(&_object3);
	}

	if (!g_globals->getFlag(59) && !g_globals->getFlag(70) && !g_globals->getFlag(37) && !g_globals->getFlag(114)) {
		_object2.postInit();
		_object2.setVisage(2108);
		_object2._strip = 3;
		_object2.setPosition(Common::Point(150, 100));
		_object2.animate(ANIM_MODE_NONE, NULL);
		_object2.changeZoom(100);
		_object2.fixPriority(113);
		_object2.setAction(&_action3);
		g_globals->_sceneItems.push_back(&_object2);
	}

	g_globals->_sceneItems.addItems(&_hotspot15, &_hotspot11, &_hotspot10, &_hotspot9, &_hotspot14,
		&_hotspot13, &_hotspot12, &_hotspot8, &_object1, &_hotspot2, &_hotspot3, &_hotspot4, &_hotspot5,
		&_hotspot6, &_hotspot7, &_hotspot1, NULL);

	g_globals->_player.postInit();
	if (g_globals->getFlag(13)) {
		g_globals->_player.setVisage(2170);
		g_globals->_player._moveDiff.y = 1;
	} else {
		g_globals->_player.setVisage(0);
		g_globals->_player._moveDiff.y = 3;
	}

	g_globals->_player.setObjectWrapper(new SceneObjectWrapper());
	g_globals->_player.animate(ANIM_MODE_1, NULL);
	g_globals->_player._moveDiff.x = 4;
	g_globals->_player.changeZoom(-1);
	g_globals->_player.disableControl();
	_sitFl = 0;

	switch (g_globals->_sceneManager._previousScene) {
	case 2120:
		g_globals->_soundHandler.play(160);
		g_globals->_soundHandler.holdAt(true);
		_object1.fixPriority(-1);
		g_globals->_player.fixPriority(-1);
		g_globals->_player.setPosition(Common::Point(80, 66));
		g_globals->_player.enableControl();
		break;
	case 2150:
		g_globals->_player.fixPriority(1);
		g_globals->_player.setPosition(Common::Point(157, 56));
		_sceneMode = 2104;
		setAction(&_sequenceManager, this, 2104, &g_globals->_player, &_object1, NULL);
		break;
	case 2222:
		if (g_globals->_sceneObjects->contains(&_object3))
			_object3.remove();

		g_globals->_player.fixPriority(1);
		g_globals->_player.setPosition(Common::Point(144, 55));

		_object2.setVisage(2806);
		_object2.changeZoom(-1);
		_object2.setPosition(Common::Point(158, 55));
		_object2.fixPriority(1);
		_object2.setAction(NULL);
		_object2.setObjectWrapper(new SceneObjectWrapper());
		_object2.animate(ANIM_MODE_1, NULL);
		_object2.setStrip(3);
		setAction(&_action12);
		break;
	case 2320:
		if (g_globals->_stripNum == 2321) {
			if (g_globals->_sceneObjects->contains(&_object3))
				_object3.remove();

			g_globals->_player.fixPriority(1);
			g_globals->_player.setPosition(Common::Point(144, 55));

			_object2.postInit();
			_object2.setVisage(2806);
			_object2.setStrip(1);
			_object2.changeZoom(-1);
			_object2.setPosition(Common::Point(158, 55));
			_object2.fixPriority(1);
			_object2.setAction(NULL);
			_object2.setObjectWrapper(new SceneObjectWrapper());
			_object2.animate(ANIM_MODE_1, NULL);

			setAction(&_action12);
		} else if (g_globals->_stripNum == 6100) {
			g_globals->_player.setPosition(Common::Point(157, 56));
			g_globals->_player.fixPriority(1);

			_object4.postInit();
			_object4.setVisage(2102);
			_object4.setPosition(Common::Point(160, 199));
			_object4.hide();

			setAction(&_action14);
		} else {
			g_globals->_player.disableControl();
			g_globals->_player.fixPriority(1);
			g_globals->_player.setPosition(Common::Point(157, 56));
			_sceneMode = 2104;

			setAction(&_sequenceManager, this, 2104, &g_globals->_player, &_object1, NULL);
		}
		break;
	case 3700:
		g_globals->_soundHandler.play(160);
		g_globals->_soundHandler.holdAt(true);
		Scene::setZoomPercents(80, 75, 100, 90);

		if (g_globals->_sceneObjects->contains(&_object2))
			_object2.remove();

		g_globals->_player._angle = 225;
		g_globals->_player.setStrip(6);
		g_globals->_player.setFrame(1);
		g_globals->_player.fixPriority(-1);
		g_globals->_player.setPosition(Common::Point(272, 127));

		_object3.setPosition(Common::Point(246, 156));
		_object3.fixPriority(156);
		_sceneMode = 2105;
		setAction(&_sequenceManager, this, 2105, &_object3, NULL);
		break;
	case 4250:
		g_globals->_soundHandler.play(160);
		g_globals->_soundHandler.holdAt(true);
		g_globals->clearFlag(43);

		g_globals->_player.setVisage(2104);
		g_globals->_player.setFrame(1);
		g_globals->_player.setPosition(Common::Point(65, 149));
		g_globals->_player.fixPriority(152);
		g_globals->_player.setStrip(2);

		_object4.postInit();
		_object4.setVisage(2102);
		_object4.setPosition(Common::Point(160, 199));
		_object4.hide();

		_sceneMode = 2107;
		setAction(&_sequenceManager, this, 2107, &_object4, NULL);
		break;
	case 5000:
		g_globals->_soundHandler.play(160);
		g_globals->_soundHandler.holdAt(true);

		if (g_globals->_sceneObjects->contains(&_object2))
			_object2.remove();

		g_globals->_player.setStrip(3);
		g_globals->_player.setFrame(1);
		g_globals->_player.fixPriority(1);
		g_globals->_player.setPosition(Common::Point(157, 56));

		_object3.setPosition(Common::Point(246, 156));
		_object3.fixPriority(156);

		setAction(&_action5);
		break;
	case 5100:
		g_globals->_soundHandler.play(160);
		g_globals->_soundHandler.holdAt(true);
		g_globals->_player.setVisage(2104);
		g_globals->_player.setFrame(1);
		g_globals->_player.setPosition(Common::Point(65, 149));
		g_globals->_player.fixPriority(152);
		g_globals->_player.setStrip(2);

		_sitFl = 1;

		_object4.postInit();
		_object4.setVisage(2102);
		_object4.setPosition(Common::Point(160, 199));
		_object4.hide();

		RING_INVENTORY._stasisBox._sceneNumber = 0;
		setAction(&_action9);
		break;
	case 7000:
		g_globals->_soundHandler.play(160);
		g_globals->_soundHandler.holdAt(true);

		if (RING_INVENTORY._stasisBox2._sceneNumber == 1) {
			g_globals->_player.fixPriority(1);
			g_globals->_player.setPosition(Common::Point(157, 56));

			_object4.postInit();
			_object4.setVisage(2102);
			_object4.setPosition(Common::Point(160, 199));
			_object4.hide();
			g_globals->clearFlag(15);
			g_globals->clearFlag(109);
			g_globals->clearFlag(72);

			setAction(&_action17);
		} else {
			g_globals->_player.setVisage(2104);
			g_globals->_player.setFrame(1);
			g_globals->_player.setPosition(Common::Point(65, 149));
			g_globals->_player.fixPriority(152);
			g_globals->_player.setStrip(2);

			_sitFl = 1;
			setAction(&_action16);
		}
		break;
	case 7600:
		g_globals->_soundHandler.play(160);
		g_globals->_soundHandler.holdAt(true);

		if (g_globals->_sceneObjects->contains(&_object2))
			_object2.remove();

		g_globals->_player.fixPriority(1);
		g_globals->_player.setPosition(Common::Point(157, 56));

		setAction(&_action8);
		break;
	case 8100:
		g_globals->_soundHandler.play(160);
		g_globals->_soundHandler.holdAt(true);

		g_globals->_player.setVisage(2104);
		g_globals->_player.setFrame(1);
		g_globals->_player.setPosition(Common::Point(65, 149));
		g_globals->_player.fixPriority(152);
		g_globals->_player.setStrip(2);

		_sceneMode = 2106;
		setAction(&_sequenceManager, this, 2106, NULL);
		break;
	case 9750:
		g_globals->_soundHandler.play(160);
		g_globals->_soundHandler.holdAt(true);

		g_globals->_player.setVisage(2104);
		g_globals->_player.setFrame(1);
		g_globals->_player.setPosition(Common::Point(65, 149));
		g_globals->_player.fixPriority(152);
		g_globals->_player.setStrip(2);

		_object4.postInit();
		_object4.setVisage(2102);
		_object4.setPosition(Common::Point(160, 199));
		_object4.hide();

		_sceneMode = 2103;
		setAction(&_sequenceManager, this, 2103, &_object4, NULL);
		break;
	default:
		g_globals->_soundHandler.play(160);
		g_globals->_soundHandler.holdAt(true);

		g_globals->_player._uiEnabled = true;
		break;
	}

	g_globals->_sceneManager._scene->_sceneBounds.contain(g_globals->_sceneManager._scene->_backgroundBounds);
	g_globals->_sceneOffset.x = (g_globals->_sceneManager._scene->_sceneBounds.left / 160) * 160;
}

void Scene2100::stripCallback(int v) {
	switch (v) {
	case 1:
		_object4._numFrames = 4;
		_object4.animate(ANIM_MODE_7, 0, NULL);
		break;
	case 2:
		_object4.animate(ANIM_MODE_NONE, NULL);
		break;
	}
}

void Scene2100::signal() {
	switch (_sceneMode) {
	case 2101:
		_sitFl = 1;
		g_globals->_player._uiEnabled = true;
		g_globals->_events.setCursor(CURSOR_USE);
		break;
	case 2102:
		_sitFl = 0;
		g_globals->_player.enableControl();
		break;
	case 2103:
		g_globals->_stripNum = 9000;
		g_globals->_sceneManager.changeScene(4000);
		break;
	case 2106:
		g_globals->_sceneManager.changeScene(7000);
		break;
	case 2107:
		g_globals->_sceneManager.changeScene(5000);
		break;
	case 2104:
	case 2105:
	case 2108:
	case 2110:
	case 2111:
	case 2112:
		g_globals->_player.enableControl();
		break;
	}
}

void Scene2100::synchronize(Serializer &s) {
	Scene::synchronize(s);
	if (s.getVersion() >= 3)
		s.syncAsSint16LE(_sitFl);
}

/*--------------------------------------------------------------------------
 * Scene 2120 - Encyclopedia
 *
 *--------------------------------------------------------------------------*/

Scene2120::Action1::Action1() {
	_entries.push_back(Entry(0, 0, 0));
	_entries.push_back(Entry(0, 0, 0));
	_entries.push_back(Entry(4, 1, 2123));
	_entries.push_back(Entry(1, 6, 0));
	_entries.push_back(Entry(2, 8, 0));
	_entries.push_back(Entry(1, 11, 0));
	_entries.push_back(Entry(4, 13, 2131));
	_entries.push_back(Entry(2, 18, 0));
	_entries.push_back(Entry(4, 21, 0));
	_entries.push_back(Entry(7, 26, 2128));
	_entries.push_back(Entry(3, 34, 0));
	_entries.push_back(Entry(0, 38, 0));
	_entries.push_back(Entry(3, 39, 2126));
	_entries.push_back(Entry(3, 43, 0));
	_entries.push_back(Entry(4, 47, 2125));
	_entries.push_back(Entry(3, 52, 0));
	_entries.push_back(Entry(4, 56, 2129));
	_entries.push_back(Entry(7, 61, 0));
	_entries.push_back(Entry(2, 69, 2127));
	_entries.push_back(Entry(7, 72, 2122));
	_entries.push_back(Entry(0, 80, 2124));
	_entries.push_back(Entry(0, 81, 0));
	_entries.push_back(Entry(0, 82, 0));
	_entries.push_back(Entry(1, 83, 0));
	_entries.push_back(Entry(2, 85, 2132));
	_entries.push_back(Entry(1, 88, 2133));
	_entries.push_back(Entry(2, 90, 2136));
	_entries.push_back(Entry(1, 93, 0));
	_entries.push_back(Entry(10, 95, 2135));
	_entries.push_back(Entry(5, 106, 0));
	_entries.push_back(Entry(2, 112, 2134));
	_entries.push_back(Entry(1, 115, 2130));
	_entries.push_back(Entry(0, 117, 0));
}

void Scene2120::Action1::signal() {
	Scene2120 *scene = (Scene2120 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(30);
		g_globals->_events.setCursor(CURSOR_WALK);
		break;
	case 1:
		// First page of index
		SceneItem::display(2120, 0, SET_X, 120, SET_FONT, 1, SET_EXT_BGCOLOR, 7, SET_BG_COLOR, -1,
			SET_WIDTH, 200, SET_KEEP_ONSCREEN, -1, SET_TEXT_MODE, 0, LIST_END);
		break;
	case 2:
		// Second page of index
		SceneItem::display(2120, 1, SET_X, 120, SET_FONT, 1, SET_EXT_BGCOLOR, 7, SET_BG_COLOR, -1,
			SET_WIDTH, 200, SET_KEEP_ONSCREEN, -1, SET_TEXT_MODE, 0, LIST_END);
		break;
	case 3:
		// Display an image associated with the encyclopedia entry
		SceneItem::display(0, 0);

		scene->_visageHotspot.postInit();
		scene->_visageHotspot.setVisage(_entries[scene->_subjectIndex]._visage);
		scene->_visageHotspot.setPosition(Common::Point(129, 180));
		scene->_visageHotspot.animate(ANIM_MODE_NONE, NULL);
		scene->_visageVisable = true;
		break;
	case 4:
		// Display page of text
		SceneItem::display(2121, _entries[scene->_subjectIndex]._lineNum + scene->_lineOffset,
			SET_X, 130, SET_FONT, 1, SET_EXT_BGCOLOR, 7, SET_BG_COLOR, -1, SET_WIDTH, 200,
			SET_KEEP_ONSCREEN, -1, SET_TEXT_MODE, 0, LIST_END);
		_actionIndex = 4;
		break;
	}
}

void Scene2120::Action1::dispatch() {
	Scene2120 *scene = (Scene2120 *)g_globals->_sceneManager._scene;

	Event event;
	if (g_globals->_events.getEvent(event) && (event.eventType == EVENT_BUTTON_DOWN)) {
		if (scene->_listRect.contains(event.mousePos) && (scene->_dbMode != 2)) {
			scene->_topicArrowHotspot.setPosition(Common::Point(scene->_topicArrowHotspot._position.x, event.mousePos.y));
		}

		// Subject button handling
		if (scene->_subjectButton._bounds.contains(event.mousePos) && (scene->_dbMode != 2)) {
			scene->_arrowHotspot.setPosition(Common::Point(291, 34));
			scene->_arrowHotspot._strip = 1;
			scene->_arrowHotspot.animate(ANIM_MODE_5, NULL);

			if (scene->_dbMode == 0)
				scene->_subjectIndex = (scene->_topicArrowHotspot._position.y - 48) / 8;
			else
				scene->_subjectIndex = (scene->_topicArrowHotspot._position.y - 44) / 8 + 16;

			if ((scene->_subjectIndex == 27) && g_globals->getFlag(70))
				g_globals->setFlag(75);

			scene->_topicArrowHotspot.hide();
			scene->_prevDbMode = scene->_dbMode;
			scene->_dbMode = 2;
			scene->_lineOffset = 0;

			_actionIndex = !_entries[scene->_subjectIndex]._visage ? 4 : 3;
			setDelay(30);
			scene->_soundHandler.play(159);
		}

		// Next Page button handling
		if (scene->_nextPageButton._bounds.contains(event.mousePos)) {
			if (!scene->_dbMode) {
				scene->_arrowHotspot._strip = 2;
				scene->_arrowHotspot.setPosition(Common::Point(291, 76));
				scene->_arrowHotspot.animate(ANIM_MODE_5, NULL);
				scene->_dbMode = 1;

				_actionIndex = 2;
				setDelay(30);
			}

			if ((scene->_dbMode == 2) && (scene->_lineOffset < _entries[scene->_subjectIndex]._size)) {
				if (!scene->_visageVisable) {
					++scene->_lineOffset;
				} else {
					scene->_visageVisable = false;
					scene->_visageHotspot.remove();
				}
				setDelay(30);
			}

			if ((scene->_subjectIndex == 20) && scene->_visageVisable) {
				scene->_visageVisable = false;
				scene->_visageHotspot.remove();
				setDelay(30);
			}

			scene->_soundHandler.play(159);
		}

		// Previous Page button handling
		if (scene->_previousPageButton._bounds.contains(event.mousePos)) {
			switch (scene->_dbMode) {
			case 1:
				scene->_arrowHotspot._strip = 3;
				scene->_arrowHotspot.setPosition(Common::Point(291, 117));
				scene->_arrowHotspot.animate(ANIM_MODE_5, NULL);

				scene->_dbMode = 0;
				_actionIndex = 1;
				setDelay(30);
				break;
			case 2:
				if (scene->_lineOffset > 0) {
					--scene->_lineOffset;
					setDelay(20);
				}
				if ((_entries[scene->_subjectIndex]._visage != 0) && (scene->_lineOffset == 0)) {
					_actionIndex = 3;
					setDelay(30);
				}
				break;
			}

			scene->_soundHandler.play(159);
		}

		// Exit button handling
		if (scene->_exitButton._bounds.contains(event.mousePos)) {
			if (scene->_dbMode != 2) {
				// In the index, so return to the previous scene
				setAction(NULL);
				SceneItem::display(0, 0);

				g_globals->_gfxManagerInstance._font.setFontNumber(2);
				g_globals->_sceneText._fontNumber = 2;
				g_globals->_sceneManager.changeScene(g_globals->_sceneManager._previousScene);
			} else {
				// Exit out of topic display to index
				SceneItem::display(0, 0);

				if (_entries[scene->_subjectIndex]._visage)
					scene->_visageHotspot.remove();

				scene->_arrowHotspot._strip = 4;
				scene->_arrowHotspot.setPosition(Common::Point(291, 159));
				scene->_arrowHotspot.animate(ANIM_MODE_5, NULL);
				scene->_dbMode = scene->_prevDbMode;
				_actionIndex = scene->_prevDbMode + 1;

				scene->_topicArrowHotspot.show();
				setDelay(1);
			}

			scene->_soundHandler.play(159);
		}
	}

	Action::dispatch();
}

/*--------------------------------------------------------------------------*/

Scene2120::Scene2120(): Scene() {
	_listRect = Rect(18, 48, 260, 177);
	_dbMode = 0;
	_prevDbMode = 0;
	_visageVisable = false;
	_subjectIndex = 0;
}

void Scene2120::postInit(SceneObjectList *OwnerList) {
	loadScene(2120);
	setZoomPercents(0, 100, 200, 100);
	g_globals->_player.disableControl();

	_subjectButton.setBounds(Rect(266, 13, 320, 56));
	_nextPageButton.setBounds(Rect(266, 56, 320, 98));
	_previousPageButton.setBounds(Rect(266, 98, 320, 140));
	_exitButton.setBounds(Rect(266, 140, 320, 182));

	_topicArrowHotspot.postInit();
	_topicArrowHotspot.setVisage(2120);
	_topicArrowHotspot.animate(ANIM_MODE_NONE, NULL);
	_topicArrowHotspot.setPosition(Common::Point(240, 55));

	_arrowHotspot.postInit();
	_arrowHotspot.setVisage(2121);
	_arrowHotspot.animate(ANIM_MODE_NONE, NULL);
	_arrowHotspot._frame = 1;
	_arrowHotspot.setPosition(Common::Point(400, 200));

	setAction(&_action1);
	g_globals->_sceneManager._scene->_sceneBounds.contain(g_globals->_sceneManager._scene->_backgroundBounds);
	g_globals->_sceneOffset.x = (g_globals->_sceneManager._scene->_sceneBounds.left / 160) * 160;
}

void Scene2120::synchronize(Serializer &s) {
	Scene::synchronize(s);

	s.syncAsSint16LE(_dbMode);
	s.syncAsSint16LE(_prevDbMode);
	s.syncAsSint16LE(_visageVisable);
	s.syncAsSint16LE(_subjectIndex);
	s.syncAsSint16LE(_lineOffset);
}

/*--------------------------------------------------------------------------
 * Scene 2150 - Starcraft Level 2
 *
 *--------------------------------------------------------------------------*/

void Scene2150::Action1::signal() {
	Scene2150 *scene = (Scene2150 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		g_globals->_player.disableControl();
		Common::Point pt(158, 103);
		PlayerMover *mover = new PlayerMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 1:
		scene->_soundHandler.play(162);
		scene->_hotspot1.animate(ANIM_MODE_5, this);
		break;
	case 2: {
		g_globals->_player.setStrip2(4);
		Common::Point pt(158, 95);
		NpcMover *mover = new NpcMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 3:
		g_globals->_player.setStrip2(-1);
		g_globals->_player._strip = 3;
		setDelay(10);
		break;
	case 4:
		scene->_area1.display();
		scene->_area2.display();
		scene->_area3.display();
		scene->_area4.display();

		scene->_area2.draw(true);
		_state = 0;
		g_globals->_events.setCursor(CURSOR_USE);

		while (!_state && !g_vm->shouldQuit()) {
			// Wait for an event
			Event event;
			if (!g_globals->_events.getEvent(event)) {
				GLOBALS._screenSurface.updateScreen();
				g_system->delayMillis(10);
				continue;
			}

			if (scene->_area1._bounds.contains(event.mousePos)) {
				scene->_area2.draw(false);
				scene->_area1.draw(true);
				_state = scene->_area1._actionId;
			}
			if (scene->_area2._bounds.contains(event.mousePos)) {
				scene->_area2.draw(true);
				_state = scene->_area2._actionId;
			}
			if (scene->_area3._bounds.contains(event.mousePos)) {
				scene->_area2.draw(false);
				scene->_area3.draw(true);
				_state = scene->_area3._actionId;
			}
		}

		scene->_soundHandler.play(161);
		scene->_area1.restore();
		scene->_area2.restore();
		scene->_area3.restore();
		scene->_area4.restore();

		if (_state == 2150) {
			setDelay(1);
		} else {
			scene->_soundHandler.play(162);
			scene->_hotspot1.animate(ANIM_MODE_6, this);
		}
		break;
	case 5:
		if (_state == 2150) {
			Common::Point pt(158, 103);
			PlayerMover *mover = new PlayerMover();
			g_globals->_player.addMover(mover, &pt, this);
			break;
		} else {
			g_globals->_sceneManager.changeScene(_state);
		}
		break;
	case 6:
		scene->_soundHandler.play(162);
		scene->_hotspot1.animate(ANIM_MODE_6, this);
		break;
	case 7:
		g_globals->_player.enableControl();
		remove();
		break;
	}
}

void Scene2150::Action2::signal() {
	Scene2150 *scene = (Scene2150 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		g_globals->_player.disableControl();
		Common::Point pt(47, 85);
		PlayerMover *mover = new PlayerMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 1:
		g_globals->_player.setVisage(2152);
		g_globals->_player.setFrame(1);
		g_globals->_player.setStrip(8);
		g_globals->_player.animate(ANIM_MODE_8, 1, this);

		scene->_soundHandler.play(163);
		break;
	case 2:
		scene->_soundHandler.play(164);
		scene->_hotspot10.animate(ANIM_MODE_5, this);
		break;
	case 3:
		g_globals->_events.setCursor(CURSOR_WALK);
		scene->_stripManager.start((RING_INVENTORY._ale._sceneNumber == 2150) ? 2151 : 2152, this);
		break;
	case 4:
		scene->_hotspot14.postInit();
		scene->_hotspot14.setVisage(2152);
		scene->_hotspot14.setStrip(6);
		scene->_hotspot14.setPosition(Common::Point(59, 54));

		if (scene->_stripManager._field2E8 == 15) {
			scene->_hotspot14.setFrame(5);
			RING_INVENTORY._ale._sceneNumber = 1;
		} else {
			scene->_hotspot14.setFrame(scene->_stripManager._field2E8 - 5);
		}

		g_globals->_player.setFrame(1);
		g_globals->_player.setStrip(7);
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 5:
		scene->_soundHandler.play(164);
		scene->_hotspot10.animate(ANIM_MODE_6, NULL);
		scene->_hotspot14.remove();

		g_globals->_player.animate(ANIM_MODE_6, this);
		break;
	case 6:
		g_globals->_player.setVisage(0);
		g_globals->_player.animate(ANIM_MODE_1, NULL);
		g_globals->_player.enableControl();

		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene2150::Hotspot1::doAction(int action) {
	Scene2150 *scene = (Scene2150 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2150, 0);
		break;
	case CURSOR_USE:
		scene->setAction(&scene->_action1);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2150::Hotspot2::doAction(int action) {
	Scene2150 *scene = (Scene2150 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2150, 1);
		break;
	case CURSOR_USE:
		g_globals->_player.disableControl();
		scene->_sceneMode = 2156;
		scene->setAction(&scene->_sequenceManager, scene, 2156, &g_globals->_player, this, NULL);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2150::Hotspot4::doAction(int action) {
	Scene2150 *scene = (Scene2150 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2150, 3);
		break;
	case CURSOR_USE:
		g_globals->_player.disableControl();
		scene->_sceneMode = 2155;
		scene->setAction(&scene->_sequenceManager, scene, 2155, &g_globals->_player, &scene->_hotspot4, NULL);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2150::Hotspot7::doAction(int action) {
	Scene2150 *scene = (Scene2150 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2150, 7);
		break;
	case CURSOR_USE:
		if (g_globals->getFlag(13))
			SceneItem::display2(2150, 19);
		else {
			g_globals->_player.disableControl();
			scene->_sceneMode = 2154;
			scene->setAction(&scene->_sequenceManager, scene, 2154, &g_globals->_player, NULL);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2150::Hotspot10::doAction(int action) {
	Scene2150 *scene = (Scene2150 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2150, 10);
		break;
	case CURSOR_USE:
		if (g_globals->getFlag(13))
			SceneItem::display2(2150, 19);
		else
			scene->setAction(&scene->_action2);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene2150::Scene2150() :
		_hotspot3(18, CURSOR_LOOK, 2150, 2, CURSOR_USE, 2150, 18, LIST_END),
		_hotspot5(17, CURSOR_LOOK, 2150, 4, CURSOR_USE, 2150, 18, LIST_END),
		_hotspot6(0, CURSOR_LOOK, 2150, 5, CURSOR_USE, 2150, 18, LIST_END),
		_hotspot8(16, CURSOR_LOOK, 2150, 8, LIST_END),
		_hotspot9(0, CURSOR_LOOK, 2150, 9, CURSOR_USE, 2150, 13, LIST_END),
		_hotspot11(0, CURSOR_LOOK, 2150, 12, LIST_END) {
	_rect1 = Rect(260, 70, 270, 77);
	_rect2 = Rect(222, 142, 252, 150);
	_area1.setup(2153, 2, 1, 2100);
	_area1._pt = Common::Point(200, 31);
	_area2.setup(2153, 3, 1, 2150);
	_area2._pt = Common::Point(200, 50);
	_area3.setup(2153, 4, 1, 2320);
	_area3._pt = Common::Point(200, 75);
	_area4.setup(2153, 1, 1, 10);
	_area4._pt = Common::Point(237, 77);

}

void Scene2150::postInit(SceneObjectList *OwnerList) {
	Scene::postInit();
	setZoomPercents(55, 85, 200, 100);
	loadScene(2150);

	_stripManager.addSpeaker(&_speakerGameText);

	_hotspot7.postInit();
	_hotspot7.setVisage(2152);
	_hotspot7._frame = 1;
	_hotspot7._strip = 2;
	_hotspot7.animate(ANIM_MODE_8, 0, NULL);
	_hotspot7.setPosition(Common::Point(122, 62));
	_hotspot7.changeZoom(100);
	_hotspot7.fixPriority(76);

	_hotspot2.postInit();
	_hotspot2.setVisage(2151);
	_hotspot2._frame = 1;
	_hotspot2._strip = 3;
	_hotspot2.animate(ANIM_MODE_NONE, NULL);
	_hotspot2.setPosition(Common::Point(257, 67));
	_hotspot2.changeZoom(100);
	_hotspot2.fixPriority(60);

	_hotspot1.postInit();
	_hotspot1.setVisage(2151);
	_hotspot1._frame = 1;
	_hotspot1._strip = 2;
	_hotspot1.animate(ANIM_MODE_NONE, NULL);
	_hotspot1.setPosition(Common::Point(158, 99));
	_hotspot1.changeZoom(100);
	_hotspot1.fixPriority(99);

	_hotspot4.postInit();
	_hotspot4.setVisage(2150);
	_hotspot4._frame = 1;
	_hotspot4._strip = 2;
	_hotspot4.animate(ANIM_MODE_NONE, NULL);
	_hotspot4.setPosition(Common::Point(218, 200));
	_hotspot4.changeZoom(100);
	_hotspot4.fixPriority(200);

	_hotspot10.postInit();
	_hotspot10.setVisage(2152);
	_hotspot10.setStrip(5);
	_hotspot10.setPosition(Common::Point(59, 56));

	g_globals->_player.postInit();
	g_globals->_player.setVisage(g_globals->getFlag(13) ? 2170 : 0);
	g_globals->_player.animate(ANIM_MODE_1, NULL);
	g_globals->_player.setObjectWrapper(new SceneObjectWrapper());
	g_globals->_player.fixPriority(-1);
	g_globals->_player.changeZoom(-1);
	g_globals->_player._moveDiff.y = 3;

	_hotspot8.setBounds(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
	_hotspot9.setBounds(Rect(133, 0, 198, 91));
	_hotspot11.setBounds(Rect(142, 119, 176, 158));

	g_globals->_sceneItems.addItems(&_hotspot1, &_hotspot2, &_hotspot3, &_hotspot4, &_hotspot5,
		&_hotspot6, &_hotspot7, &_hotspot10, &_hotspot9, &_hotspot11, &_hotspot8, NULL);

	switch (g_globals->_sceneManager._previousScene) {
	case 2120:
		g_globals->_soundHandler.play(160);
		g_globals->_soundHandler.holdAt(true);
		g_globals->_player.setPosition(Common::Point(108, 99));
		break;
	case 2200:
		g_globals->_player.disableControl();
		g_globals->_player.setPosition(Common::Point(159, 240));
		_sceneMode = 2152;
		setAction(&_sequenceManager, this, 2152, &g_globals->_player, NULL);
		break;
	case 2280:
		g_globals->_player.disableControl();
		g_globals->_player._angle = 180;
		g_globals->_player.setPosition(Common::Point(265, 80));

		_hotspot2._frame = _hotspot2.getFrameCount();
		_sceneMode = 2157;
		setAction(&_sequenceManager, this, 2157, &_hotspot2, NULL);
		break;
	case 2230:
		g_globals->_events.setCursor(CURSOR_WALK);
		g_globals->_player._strip = 4;
		g_globals->_player.setPosition(Common::Point(229, 139));
		break;
	case 2100:
	default:
		g_globals->_player.disableControl();
		g_globals->_player.setPosition(Common::Point(158, 95));
		g_globals->_player.setStrip(3);
		_sceneMode = 2151;

		setAction(&_sequenceManager, this, 2151, &g_globals->_player, &_hotspot1, NULL);
		break;
	}

	g_globals->_sceneManager._scene->_sceneBounds.contain(g_globals->_sceneManager._scene->_backgroundBounds);
	g_globals->_sceneOffset.x = (g_globals->_sceneManager._scene->_sceneBounds.left / 160) * 160;
}

void Scene2150::synchronize(Serializer &s) {
	Scene::synchronize(s);
	_rect1.synchronize(s);
	_rect2.synchronize(s);
}

void Scene2150::signal() {
	switch (_sceneMode) {
	case 2151:
		g_globals->_player.animate(ANIM_MODE_1, NULL);
		g_globals->_player.enableControl();
		break;
	case 2152:
	case 2153:
	case 2157:
		g_globals->_player.enableControl();
		break;
	case 2154:
		g_globals->_sceneManager.changeScene(2120);
		break;
	case 2155:
		g_globals->_sceneManager.changeScene(2230);
		break;
	case 2156:
		g_globals->_sceneManager.changeScene(2280);
		break;
	}
}

void Scene2150::dispatch() {
	Scene::dispatch();

	if (!_action) {
		if (_rect1.contains(g_globals->_player._position)) {
			g_globals->_player.disableControl();
			_sceneMode = 2156;
			setAction(&_sequenceManager, this, 2156, &g_globals->_player, &_hotspot2, NULL);
		}
		if (_rect2.contains(g_globals->_player._position)) {
			g_globals->_player.disableControl();
			_sceneMode = 2155;
			setAction(&_sequenceManager, this, 2155, &g_globals->_player, &_hotspot4, NULL);
		}
		if (g_globals->_player._position.y >= 196) {
			g_globals->_player.disableControl();
			SceneItem::display2(2150, 20);

			_sceneMode = 2153;
			setAction(&_sequenceManager, this, 2153, &g_globals->_player, NULL);
		}
	}
}

/*--------------------------------------------------------------------------
 * Scene 2200 - Starcraft - AutoDoc
 *
 *--------------------------------------------------------------------------*/

void Scene2200::Action1::signal() {
	Scene2200 *scene = (Scene2200 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(6);
		break;
	case 1:
		scene->_hotspot4.animate(ANIM_MODE_5, this);
		break;
	case 2:
		SceneItem::display2(2200, 7);
		g_globals->_sceneManager.changeScene(2150);
		remove();
		break;
	}
}

void Scene2200::Action2::signal() {
	Scene2200 *scene = (Scene2200 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(3);
		break;
	case 1:
		setDelay(30);
		break;
	case 2:
		scene->_hotspot4.animate(ANIM_MODE_5, this);
		break;
	case 3:
		scene->_hotspot4.setStrip(4);
		setDelay(30);
		break;
	case 4:
		g_globals->_events.setCursor(CURSOR_WALK);
		scene->_stripManager.start(2040, this, scene);
		break;
	case 5:
		scene->_hotspot4.setStrip(4);
		scene->_hotspot4.animate(ANIM_MODE_NONE, NULL);
		g_globals->_player._uiEnabled = true;
		remove();
		break;
	}
}

void Scene2200::Action3::signal() {
	Scene2200 *scene = (Scene2200 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		scene->_soundHandler2.play(103);

		scene->_hotspot4.setStrip(4);
		scene->_hotspot4.animate(ANIM_MODE_NONE, NULL);
		g_globals->_player.disableControl();

		scene->_hotspot2.postInit();
		scene->_hotspot2.setVisage(2201);
		scene->_hotspot2._strip = 2;
		scene->_hotspot2.setPosition(Common::Point(218, 0));

		Common::Point pt(218, 63);
		NpcMover *mover = new NpcMover();
		scene->_hotspot2.addMover(mover, &pt, this);
		break;
	}
	case 1:
		scene->_soundHandler2.play(104);
		scene->_hotspot4.setStrip(2);
		scene->_hotspot4.setFrame(2);
		setDelay(120);
		break;
	case 2:
		if (g_globals->getFlag(83)) {
			_actionIndex = 8;
			setDelay(5);
		} else {
			for (SynchronizedList<SceneObject *>::iterator i = g_globals->_sceneObjects->begin();
					i != g_globals->_sceneObjects->end(); ++i) {
				(*i)->hide();
			}

			g_globals->_sceneManager._scene->loadScene(66);

			scene->_hotspot6.postInit();
			scene->_hotspot6.setVisage(66);
			scene->_hotspot6.setPosition(Common::Point(160, 197));

			scene->_hotspot7.postInit();
			scene->_hotspot7.setVisage(65);
			scene->_hotspot7.setStrip(4);
			scene->_hotspot7.setFrame(1);
			scene->_hotspot7.setPosition(Common::Point(145, 165));

			SceneItem::display(60, 1, SET_Y, 40, SET_X, 25, SET_FONT, 75, SET_BG_COLOR, -1, SET_FG_COLOR, 34,
				SET_POS_MODE, 0, SET_WIDTH, 280, SET_KEEP_ONSCREEN, 1, LIST_END);
			_state = 1;
			setDelay(1800);
		}
		break;
	case 3:
	case 4:
	case 5:
		SceneItem::display(60, _actionIndex - 2, SET_Y, 40, SET_X, 25, SET_FONT, 75, SET_BG_COLOR, -1, SET_FG_COLOR, 34,
			SET_POS_MODE, 0, SET_WIDTH, 280, SET_KEEP_ONSCREEN, 1, LIST_END);
		setDelay(1800);
		break;
	case 6:
		scene->_hotspot7.remove();
		SceneItem::display(0, 0);
		_state = 0;
		setDelay(5);
		break;
	case 7:
		for (SynchronizedList<SceneObject *>::iterator i = g_globals->_sceneObjects->begin();
				i != g_globals->_sceneObjects->end(); ++i)
			(*i)->show();

		scene->_hotspot6.remove();
		g_globals->_sceneManager._scene->loadScene(2200);
		setDelay(5);
		break;
	case 8:
		scene->_stripManager.start(2060, this, scene);
		break;
	case 9:
		scene->_hotspot4.setStrip(4);
		scene->_hotspot4.animate(ANIM_MODE_NONE, NULL);
		break;
	case 10:
		g_globals->_sceneManager.changeScene(2000);
		remove();
		break;
	}
}

void Scene2200::Action3::process(Event &event) {
	if (!event.handled && ((event.eventType == EVENT_KEYPRESS) || (event.eventType == EVENT_BUTTON_DOWN))) {
		_state = 0;
		event.handled = true;
		setDelay(1);
	}

	Action::process(event);
}

void Scene2200::Action4::signal() {
	Scene2200 *scene = (Scene2200 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(5);
		break;
	case 1:
		scene->_stripManager.start(2202, this, scene);
		break;
	case 2:
		scene->_hotspot4.setStrip(4);
		scene->_hotspot4.animate(ANIM_MODE_NONE, NULL);
		setDelay(10);
		break;
	case 3:
		g_globals->_player._uiEnabled = true;
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene2200::Hotspot3::doAction(int action) {
	Scene2200 *scene = (Scene2200 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2200, 10);
		break;
	case CURSOR_USE:
		SceneItem::display2(2200, 11);
		break;
	case CURSOR_TALK:
		g_globals->_player._uiEnabled = false;
		scene->setAction(&scene->_action4);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2200::Hotspot5::doAction(int action) {
	Scene2200 *scene = (Scene2200 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2200, 8);
		break;
	case CURSOR_USE:
		SceneItem::display2(2200, 9);
		break;
	case CURSOR_TALK:
		scene->_sceneMode = 2201;
		g_globals->_player._uiEnabled = false;
		scene->setAction(&scene->_sequenceManager, scene, 2201, NULL);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2200::Hotspot9::doAction(int action) {
	Scene2200 *scene = (Scene2200 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2200, g_globals->getFlag(8) ? 1 : 0);
		break;
	case CURSOR_USE:
		SceneItem::display2(2200, 3);
		break;
	case OBJECT_INFODISK:
		if (g_globals->_sceneManager._previousScene == 2310) {
			scene->_soundHandler2.play(35);
			g_globals->_player.disableControl();
			scene->setAction(&scene->_action3);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

/*--------------------------------------------------------------------------*/


Scene2200::Scene2200() :
		_hotspot1(0, CURSOR_LOOK, 2200, 5, CURSOR_USE, 2200, 6, LIST_END),
		_hotspot10(0, CURSOR_LOOK, 2200, 4, LIST_END) {
}

void Scene2200::postInit(SceneObjectList *OwnerList) {
	Scene::postInit();
	setZoomPercents(0, 200, 200, 200);
	loadScene(2200);

	_stripManager.addSpeaker(&_speakerMText);
	_stripManager.addSpeaker(&_speakerSText);
	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerSL);
	_stripManager.addSpeaker(&_speakerQR);
	_stripManager.addSpeaker(&_speakerQL);
	_stripManager.addSpeaker(&_speakerMR);
	_stripManager.addSpeaker(&_speakerGameText);

	_speakerQText._npc = &g_globals->_player;
	_speakerSText._npc = &_hotspot5;
	_speakerMText._npc = &_hotspot3;

	_stripManager.setCallback(this);

	_hotspot5.postInit();
	_hotspot5.setVisage(2840);
	_hotspot5.setPosition(Common::Point(50, 235));

	_hotspot8.postInit();
	_hotspot8.setVisage(2840);
	_hotspot8.setStrip(2);
	_hotspot8.setPosition(Common::Point(96, 184));
	_hotspot8.fixPriority(236);

	g_globals->_player.postInit();
	g_globals->_player.setVisage(2640);
	g_globals->_player.setFrame2(3);
	g_globals->_player.setPosition(Common::Point(110, 233));
	g_globals->_player.disableControl();

	switch (g_globals->_sceneManager._previousScene) {
	case 2150:
		_hotspot5.remove();
		_hotspot8.remove();
		break;
	case 4000:
		g_globals->_soundHandler.play(100);
		g_globals->_soundHandler.holdAt(true);
		g_globals->_player.remove();
		_hotspot5.remove();
		_hotspot8.remove();

		_hotspot1.postInit();
		_hotspot1.setVisage(2202);
		_hotspot1.setPosition(Common::Point(175, 173));
		_hotspot1.fixPriority(99);

		_hotspot3.postInit();
		_hotspot3.setVisage(2202);
		_hotspot3._strip = 2;
		_hotspot3.setPosition(Common::Point(152, 76));
		_hotspot3.fixPriority(100);

		_hotspot4.postInit();
		_hotspot4.setVisage(2202);
		_hotspot4._strip = 3;
		_hotspot4.setPosition(Common::Point(115, 76));
		_hotspot4.fixPriority(200);

		setAction(&_action1);
		break;
	case 2310:
	default:
		RING_INVENTORY._infoDisk._sceneNumber = 1;

		_hotspot3.postInit();
		_hotspot3.setVisage(2215);
		_hotspot3.setPosition(Common::Point(144, 132));
		g_globals->_sceneItems.push_back(&_hotspot3);

		_hotspot4.postInit();
		_hotspot4.setVisage(2215);
		_hotspot4._strip = 2;
		_hotspot4.setPosition(Common::Point(120, 78));
		_hotspot4.fixPriority(255);
		g_globals->_sceneItems.push_back(&_hotspot4);

		_soundHandler1.play(101);
		_soundHandler2.play(100);
		g_globals->_soundHandler.holdAt(true);

		g_globals->_sceneItems.push_back(&_hotspot5);
		setAction(&_action2);
		break;
	}

	_exitRect = Rect(0, 0, 35, SCREEN_HEIGHT);
	_hotspot9.setBounds(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
	_hotspot10.setBounds(Rect(87, 43, 149, 109));

	g_globals->_sceneItems.addItems(&_hotspot10, &_hotspot9, NULL);
	g_globals->_sceneManager._scene->_sceneBounds.contain(g_globals->_sceneManager._scene->_backgroundBounds);
	g_globals->_sceneOffset.x = (g_globals->_sceneManager._scene->_sceneBounds.left / 160) * 160;
}

void Scene2200::stripCallback(int v) {
	switch (v) {
	case 1:
		_hotspot4.setStrip(3);
		_hotspot4.animate(ANIM_MODE_7, 0, NULL);
		break;
	case 2:
		_hotspot4.setStrip(4);
		_hotspot4.animate(ANIM_MODE_NONE, NULL);
		break;
	}
}

void Scene2200::synchronize(Serializer &s) {
	Scene::synchronize(s);
	_exitRect.synchronize(s);
}

void Scene2200::signal() {
	if ((_sceneMode == 2201) || (_sceneMode == 2202))
		g_globals->_player._uiEnabled = true;
}

void Scene2200::dispatch() {
	Scene::dispatch();

	if (!_action) {
		if (_exitRect.contains(g_globals->_player._position))
			g_globals->_sceneManager.changeScene(2150);
	}
}

/*--------------------------------------------------------------------------
 * Scene 2222 - Stasis Field Map
 *
 *--------------------------------------------------------------------------*/

void Scene2222::Action1::signal() {
	Scene2222 *scene = (Scene2222 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(5);
		break;
	case 1:
		scene->_stripManager.start(2222, this);
		break;
	case 2:
		setDelay(30);
		break;
	case 3:
		g_globals->_sceneManager.changeScene(1000);
		break;
	}
}

void Scene2222::Action2::signal() {
	Scene2222 *scene = (Scene2222 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(60);
		break;
	case 1:
		scene->_stripManager.start(5700, this);
		break;
	case 2:
		setDelay(120);
		break;
	case 3:
		g_globals->_sceneManager._fadeMode = FADEMODE_GRADUAL;
		g_globals->_sceneManager.changeScene(2100);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene2222::postInit(SceneObjectList *OwnerList) {
	loadScene((g_globals->_sceneManager._previousScene == 2000) ? 3400 : 3450);
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);

	_stripManager.addSpeaker(&_speakerSText);
	_stripManager.addSpeaker(&_speakerMText);
	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerSR);
	_stripManager.addSpeaker(&_speakerML);

	_hotspot1.postInit();
	_hotspot1.setVisage(3401);
	_hotspot1.setStrip2(1);
	_hotspot1._frame = 1;
	_hotspot1.animate(ANIM_MODE_2, 0);

	_hotspot2.postInit();
	_hotspot2.setVisage(3401);
	_hotspot2.setStrip2(2);
	_hotspot2._frame = 2;
	_hotspot2.animate(ANIM_MODE_2, 0);

	_hotspot3.postInit();
	_hotspot3.setVisage(3401);
	_hotspot3.setStrip2(2);
	_hotspot3._frame = 3;
	_hotspot3.animate(ANIM_MODE_2, 0);

	_hotspot4.postInit();
	_hotspot4.setVisage(3401);
	_hotspot4.setStrip2(2);
	_hotspot4._frame = 4;
	_hotspot4.animate(ANIM_MODE_2, 0);

	_hotspot5.postInit();
	_hotspot5.setVisage(3401);
	_hotspot5.setStrip2(2);
	_hotspot5.setFrame(5);
	_hotspot5.animate(ANIM_MODE_2, 0);

	if (g_globals->_sceneManager._previousScene == 2100) {
		_hotspot1.setPosition(Common::Point(61, 101));
		_hotspot2.setPosition(Common::Point(239, 149));
		_hotspot3.setPosition(Common::Point(184, 85));
		_hotspot4.setPosition(Common::Point(105, 165));
		_hotspot5.remove();

		setAction(&_action2);
	} else {
		_hotspot1.setPosition(Common::Point(110, 108));
		_hotspot2.setPosition(Common::Point(175, 136));
		_hotspot3.setPosition(Common::Point(162, 96));
		_hotspot4.setPosition(Common::Point(118, 141));
		_hotspot5.setPosition(Common::Point(124, 107));

		setAction(&_action1);
	}

	_soundHandler.play(116);
	g_globals->_sceneManager._scene->_sceneBounds.center(_hotspot1._position);

	g_globals->_sceneManager._scene->_sceneBounds.contain(g_globals->_sceneManager._scene->_backgroundBounds);
	g_globals->_sceneOffset.x = (g_globals->_sceneManager._scene->_sceneBounds.left / 160) * 160;
}


/*--------------------------------------------------------------------------
 * Scene 2230 - Starcraft - Quinn's Room
 *
 *--------------------------------------------------------------------------*/

void Scene2230::Action1::signal() {
	Scene2230 *scene = (Scene2230 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		switch (scene->_field30A) {
		case 1:
			scene->setAction(&scene->_action3);
			break;
		case 2:
			scene->setAction(&scene->_action6);
			break;
		default:
			setDelay(10);
			break;
		}
		break;
	case 1: {
		Common::Point pt(84, 74);
		NpcMover *mover = new NpcMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 2:
		scene->_hotspot1.animate(ANIM_MODE_5, this);
		break;
	case 3: {
		Common::Point pt(63, 60);
		NpcMover *mover = new NpcMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 4:
		scene->_hotspot1.animate(ANIM_MODE_6, this);
		break;
	case 5:
		g_globals->_sceneManager.changeScene(2150);
		break;
	}
}

void Scene2230::Action2::signal() {
	Scene2230 *scene = (Scene2230 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		switch (scene->_field30A) {
		case 2:
			scene->setAction(&scene->_action6, this);
			break;
		default:
			setDelay(10);
			break;
		}
		break;
	case 1: {
		g_globals->_player.disableControl();
		scene->_field30A = 1;
		g_globals->_player._regionBitList |= ~0x80;
		Common::Point pt(160, 96);
		NpcMover *mover = new NpcMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 2:
		g_globals->_player.setVisage(2235);
		g_globals->_player.setStrip2(1);
		g_globals->_player.fixPriority(100);
		g_globals->_player._frame = 1;
		g_globals->_player.setPosition(Common::Point(200, 68));
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 3:
		scene->_soundHandler.play(157);
		g_globals->_player._moveDiff = Common::Point(1, 1);
		g_globals->_player.setAction(&scene->_action4);
		g_globals->_player._uiEnabled = true;

		g_globals->_events.setCursor(CURSOR_USE);
		remove();
		break;
	}
}

void Scene2230::Action3::signal() {
	Scene2230 *scene = (Scene2230 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		g_globals->_player.setAction(NULL);
		g_globals->_player._moveDiff = Common::Point(5, 3);
		g_globals->_player.setStrip2(g_globals->_player._strip);

		Common::Point pt(190, 74);
		NpcMover *mover = new NpcMover();
		g_globals->_player.addMover(mover, &pt, this);

		scene->_soundHandler.stop();
		break;
	}
	case 1:
		g_globals->_player._strip = 3;
		g_globals->_player._frame = 1;
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 2:
		g_globals->_player._regionBitList |= 0x80;
		scene->_field30A = 0;

		g_globals->_player.setVisage(0);
		g_globals->_player.setStrip2(-1);
		g_globals->_player.fixPriority(-1);
		g_globals->_player.setPosition(Common::Point(164, 96));
		g_globals->_player.animate(ANIM_MODE_1, NULL);
		g_globals->_player._canWalk = true;

		g_globals->_events.setCursor(CURSOR_USE);
		remove();
	}
}

void Scene2230::Action4::signal() {
	switch (_actionIndex++) {
	case 0:
		Common::Point pt(190 + g_globals->_randomSource.getRandomNumber(9), 68);
		NpcMover *mover = new NpcMover();
		g_globals->_player.addMover(mover, &pt, this);
		_actionIndex = 0;
		break;
	}
}

void Scene2230::Action5::signal() {
	Scene2230 *scene = (Scene2230 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		switch (scene->_field30A) {
		case 2:
			scene->setAction(&scene->_action3, this);
			break;
		default:
			setDelay(10);
			break;
		}
		break;
	case 1: {
		g_globals->_player.disableControl();
		Common::Point pt(209, 124);
		NpcMover *mover = new NpcMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 2:
		g_globals->_player.fixPriority(1430);
		g_globals->_player.setVisage(2232);
		g_globals->_player._strip = 1;
		g_globals->_player._frame = 1;
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 3:
		g_globals->_events.setCursor(CURSOR_USE);
		g_globals->_player._uiEnabled = true;
		scene->_field30A = 2;
		remove();
		break;
	}
}

void Scene2230::Action6::signal() {
	Scene2230 *scene = (Scene2230 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player._strip = 2;
		g_globals->_player._frame = 1;
		g_globals->_player.fixPriority(-1);
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 1:
		scene->_field30A = 0;
		g_globals->_player.setVisage(0);
		g_globals->_player._strip = 1;
		g_globals->_player._canWalk = true;
		g_globals->_player.animate(ANIM_MODE_1, NULL);
		remove();
		break;
	}
}

void Scene2230::Action7::signal() {
	Scene2230 *scene = (Scene2230 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();

		switch (scene->_field30A) {
		case 1:
			scene->setAction(&scene->_action3, this);
			break;
		case 2:
			scene->setAction(&scene->_action6, this);
			break;
		default:
			setDelay(10);
			break;
		}
		break;
	case 1: {
		Common::Point pt(140, 119);
		NpcMover *mover = new NpcMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 2: {
		scene->_soundHandler.play(158);
		scene->_hotspot8.setStrip2(2);

		Common::Point pt(scene->_hotspot8._position.x, 97);
		NpcMover *mover = new NpcMover();
		scene->_hotspot8.addMover(mover, &pt, this);
		break;
	}
	case 3:
		scene->_hotspot2.postInit();
		scene->_hotspot2.setVisage(2231);
		scene->_hotspot2._strip = 3;
		scene->_hotspot2.setPosition(Common::Point(166, 116));
		scene->_hotspot2.fixPriority(131);
		scene->_hotspot2.animate(ANIM_MODE_5, this);

		scene->_hotspot8._frame = 2;
		break;
	case 4:
		g_globals->_sceneItems.push_front(&scene->_hotspot10);
		g_globals->_sceneItems.push_front(&scene->_hotspot11);
		g_globals->_sceneItems.push_front(&scene->_hotspot12);

		scene->_hotspot2.remove();
		g_globals->_player.enableControl();
		remove();
		break;
	}
}

void Scene2230::Action8::signal() {
	Scene2230 *scene = (Scene2230 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();

		switch (scene->_field30A) {
		case 1:
			scene->setAction(&scene->_action3, this);
			break;
		case 2:
			scene->setAction(&scene->_action6, this);
			break;
		default:
			setDelay(10);
			break;
		}
		break;
	case 1: {
		Common::Point pt(140, 119);
		NpcMover *mover = new NpcMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 2: {
		g_globals->_sceneItems.remove(&scene->_hotspot10);
		g_globals->_sceneItems.remove(&scene->_hotspot11);
		g_globals->_sceneItems.remove(&scene->_hotspot12);

		switch (scene->_field30A) {
		case 1:
			scene->setAction(&scene->_action3, this);
			break;
		case 2:
			scene->setAction(&scene->_action6, this);
			break;
		default:
			setDelay(10);
			break;
		}
		break;
	}
	case 3:
		scene->_hotspot2.postInit();
		scene->_hotspot2.setVisage(2231);
		scene->_hotspot2._strip = 3;
		scene->_hotspot2._frame = scene->_hotspot2.getFrameCount();
		scene->_hotspot2.setPosition(Common::Point(166, 116));
		scene->_hotspot2.fixPriority(131);
		scene->_hotspot2.animate(ANIM_MODE_6, this);
		break;
	case 4: {
		scene->_soundHandler.play(158);
		scene->_hotspot2.remove();
		scene->_hotspot8._frame = 1;

		Common::Point pt(scene->_hotspot8._position.x, 133);
		NpcMover *mover = new NpcMover();
		scene->_hotspot8.addMover(mover, &pt, this);
		break;
	}
	case 5:
		scene->_hotspot8.setStrip2(1);
		g_globals->_player.enableControl();

		remove();
		break;
	}
}


/*--------------------------------------------------------------------------*/

void Scene2230::Hotspot1::doAction(int action) {
	Scene2230 *scene = (Scene2230 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2230, 0);
		break;
	case CURSOR_USE:
		scene->setAction(&scene->_action1);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2230::Hotspot3::doAction(int action) {
	switch (action) {
	case CURSOR_LOOK:
		if (g_globals->getFlag(4))
			SceneItem::display2(2230, 1);
		else {
			SceneItem::display2(2230, 1);
			SceneItem::display2(2230, 2);
			SceneItem::display2(2230, 3);
			g_globals->setFlag(4);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2230::Hotspot4::doAction(int action) {
	switch (action) {
	case CURSOR_LOOK:
		if (g_globals->getFlag(30))
			SceneItem::display2(2230, 6);
		else if (g_globals->getFlag(29)) {
			SceneItem::display2(2230, 5);
			g_globals->setFlag(30);
		} else {
			SceneItem::display2(2230, 4);
			g_globals->setFlag(29);
		}
		break;
	case CURSOR_USE:
		SceneItem::display2(2230, 7);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2230::Hotspot5::doAction(int action) {
	switch (action) {
	case CURSOR_LOOK:
		if (g_globals->getFlag(5))
			SceneItem::display2(2230, 9);
		else {
			g_globals->setFlag(5);
			SceneItem::display2(2230, 8);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2230::Hotspot6::doAction(int action) {
	Scene2230 *scene = (Scene2230 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		if (g_globals->getFlag(6))
			SceneItem::display2(2230, 11);
		else {
			g_globals->setFlag(6);
			SceneItem::display2(2230, 10);
		}
		break;
	case CURSOR_USE:
		if (scene->_field30A == 1)
			scene->setAction(&scene->_action3);
		else if (g_globals->getFlag(13))
			SceneItem::display2(2230, 28);
		else
			scene->setAction(&scene->_action2);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2230::Hotspot7::doAction(int action) {
	Scene2230 *scene = (Scene2230 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		if (g_globals->getFlag(7))
			SceneItem::display2(2230, 13);
		else {
			g_globals->setFlag(6);
			SceneItem::display2(2230, 12);
		}
		break;
	case CURSOR_USE:
		if (scene->_field30A == 2)
			scene->setAction(&scene->_action6);
		else if (g_globals->getFlag(13))
			SceneItem::display2(2230, 29);
		else
			scene->setAction(&scene->_action5);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2230::Hotspot8::doAction(int action) {
	Scene2230 *scene = (Scene2230 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		if (scene->_hotspot8._strip == 1) {
			if (g_globals->getFlag(22))
				SceneItem::display2(2230, 19);
			else {
				g_globals->setFlag(22);
				SceneItem::display2(2230, 14);
			}
		} else {
			if (g_globals->getFlag(25))
				SceneItem::display2(2230, 21);
			else {
				g_globals->setFlag(25);
				SceneItem::display2(2230, 20);
			}
		}
		break;
	case CURSOR_USE:
		if (scene->_hotspot8._strip == 1)
			scene->setAction(&scene->_action7);
		else
			scene->setAction(&scene->_action8);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2230::Hotspot10::doAction(int action) {
	switch (action) {
	case CURSOR_LOOK:
		if (g_globals->getFlag(26))
			SceneItem::display2(2230, 25);
		else {
			g_globals->setFlag(26);
			SceneItem::display2(2230, 24);
		}
		break;
	case CURSOR_USE:
		SceneItem::display2(2230, 28);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2230::Hotspot11::doAction(int action) {
	switch (action) {
	case CURSOR_LOOK:
		if (g_globals->getFlag(27))
			SceneItem::display2(2230, 23);
		else {
			g_globals->setFlag(27);
			SceneItem::display2(2230, 22);
		}
		break;
	case CURSOR_USE:
		SceneItem::display2(2230, 28);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2230::Hotspot12::doAction(int action) {
	switch (action) {
	case CURSOR_LOOK:
		if (g_globals->getFlag(28))
			SceneItem::display2(2230, 27);
		else {
			g_globals->setFlag(28);
			SceneItem::display2(2230, 26);
		}
		break;
	case CURSOR_USE:
		SceneItem::display2(2230, 28);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene2230::Scene2230() :
		_hotspot9(0, CURSOR_LOOK, 2230, 16, CURSOR_USE, 2230, 18, LIST_END) {
}

void Scene2230::postInit(SceneObjectList *OwnerList) {
	loadScene(2230);
	Scene::postInit();
	setZoomPercents(75, 80, 120, 100);

	_hotspot1.postInit();
	_hotspot1.setVisage(2230);
	_hotspot1._frame = 1;
	_hotspot1.animate(ANIM_MODE_NONE, 0);
	_hotspot1.setPosition(Common::Point(72, 69));
	_hotspot1.changeZoom(100);

	_hotspot8.postInit();
	_hotspot8.setVisage(2231);
	_hotspot8._frame = 1;
	_hotspot8.animate(ANIM_MODE_NONE, 0);
	_hotspot8.setPosition(Common::Point(164, 133));
	_hotspot8.changeZoom(100);
	_hotspot8.fixPriority(129);

	_rect1 = Rect(59, 64, 89, 74);

	g_globals->_player.postInit();
	g_globals->_player.setVisage(g_globals->getFlag(13) ? 2170 : 0);
	g_globals->_player.animate(ANIM_MODE_1, 0);
	g_globals->_player.setObjectWrapper(new SceneObjectWrapper());
	g_globals->_player.setPosition(Common::Point(80, 80));
	g_globals->_player._moveDiff.y = 3;
	g_globals->_player._regionBitList |= 0x80;
	g_globals->_player.changeZoom(-1);

	_field30A = 0;
	g_globals->_player.enableControl();

	_hotspot5.setBounds(Rect(108, 34, 142, 76));
	_hotspot4.setBounds(Rect(92, 14, 106, 57));
	_hotspot6.setBounds(Rect(169, 58, 261, 95));
	_hotspot7.setBounds(Rect(111, 117, 222, 158));
	_hotspot3.setBounds(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
	_hotspot10.setBounds(Rect(170, 96, 176, 106));
	_hotspot11.setBounds(Rect(158, 109, 164, 115));
	_hotspot12.setBounds(Rect(170, 109, 177, 115));

	g_globals->_sceneItems.addItems(&_hotspot8, &_hotspot5, &_hotspot4, &_hotspot6, &_hotspot7,
			&_hotspot1, &_hotspot3, NULL);

	g_globals->_sceneManager._scene->_sceneBounds.contain(g_globals->_sceneManager._scene->_backgroundBounds);
	g_globals->_sceneOffset.x = (g_globals->_sceneManager._scene->_sceneBounds.left / 160) * 160;
}

void Scene2230::synchronize(Serializer &s) {
	Scene::synchronize(s);
	_rect1.synchronize(s);
	s.syncAsSint16LE(_field30A);
}

void Scene2230::dispatch() {
	Scene::dispatch();

	if (!_action) {
		if (_rect1.contains(g_globals->_player._position))
			setAction(&_action1);
	}
}

/*--------------------------------------------------------------------------
 * Scene 2280 - Starcraft - Storage Room
 *
 *--------------------------------------------------------------------------*/

void Scene2280::Action1::signal() {
	Scene2280 *scene = (Scene2280 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		g_globals->_player.disableControl();
		Common::Point pt(192, 97);
		PlayerMover *mover = new PlayerMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 1:
		scene->_soundHandler.play(162);
		scene->_hotspot16.animate(ANIM_MODE_5, this);
		break;
	case 2: {
		Common::Point pt(218, 87);
		NpcMover *mover = new NpcMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 3:
		g_globals->_player.setStrip(3);
		setDelay(10);
		break;
	case 4:
		g_globals->_player.fixPriority(1);
		scene->_soundHandler.play(162);
		scene->_hotspot16.animate(ANIM_MODE_6, this);
		break;
	case 5:
		g_globals->_sceneManager.changeScene(7000);
		break;
	}
}

void Scene2280::Action2::signal() {
	Scene2280 *scene = (Scene2280 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		g_globals->_player.disableControl();
		Common::Point pt(94, 117);
		PlayerMover *mover = new PlayerMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 1:
		scene->_soundHandler.play(265);

		g_globals->_player.setVisage(2162);
		g_globals->_player._frame = 1;
		g_globals->_player._strip = 1;
		g_globals->_player.animate(ANIM_MODE_5, this);

		scene->_hotspot8.remove();
		scene->_hotspot10.remove();
		break;
	case 2:
		g_globals->_player._frame = 1;
		g_globals->_player._strip = 2;
		g_globals->_player.animate(ANIM_MODE_4, 3, 1, this);
		break;
	case 3:
		g_globals->_player.animate(ANIM_MODE_5, this);
		scene->_hotspot12.remove();
		break;
	case 4:
		scene->_soundHandler.play(266);
		g_globals->_player.setVisage(2170);
		g_globals->_player._frame = 1;
		g_globals->_player._strip = 4;
		g_globals->_player.animate(ANIM_MODE_1, NULL);
		g_globals->_player._moveDiff = Common::Point(4, 1);

		g_globals->setFlag(13);
		SceneItem::display2(2280, 37);

		g_globals->_player.enableControl();
		remove();
		break;
	}
}

void Scene2280::Action3::signal() {
	Scene2280 *scene = (Scene2280 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		g_globals->_player.disableControl();
		Common::Point pt(94, 117);
		PlayerMover *mover = new PlayerMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 1:
		scene->_soundHandler.play(265);

		g_globals->_player.setVisage(2162);
		g_globals->_player._frame = 6;
		g_globals->_player._strip = 2;
		g_globals->_player.animate(ANIM_MODE_4, 3, -1, this);
		break;
	case 2:
		scene->_hotspot12.postInit();
		scene->_hotspot12.setVisage(2280);
		scene->_hotspot12._strip = 2;
		scene->_hotspot12._frame = 3;
		scene->_hotspot12.setPosition(Common::Point(88, 76));
		scene->_hotspot12.fixPriority(1);

		g_globals->_player.animate(ANIM_MODE_6, this);
		break;
	case 3:
		g_globals->_player._strip = 1;
		g_globals->_player._frame = 12;

		g_globals->_player.animate(ANIM_MODE_6, this);
		break;
	case 4:
		scene->_hotspot8.postInit();
		scene->_hotspot8.setVisage(2280);
		scene->_hotspot8._strip = 2;
		scene->_hotspot8._frame = 1;
		scene->_hotspot8.animate(ANIM_MODE_NONE, NULL);
		scene->_hotspot8.setPosition(Common::Point(79, 108));
		scene->_hotspot8.changeZoom(100);
		scene->_hotspot8.fixPriority(1);

		scene->_hotspot10.postInit();
		scene->_hotspot10.setVisage(2280);
		scene->_hotspot10._strip = 2;
		scene->_hotspot10._frame = 2;
		scene->_hotspot10.animate(ANIM_MODE_NONE, NULL);
		scene->_hotspot10.setPosition(Common::Point(79, 104));
		scene->_hotspot10.changeZoom(100);
		scene->_hotspot10.fixPriority(1);

		g_globals->clearFlag(13);

		g_globals->_player.setVisage(0);
		g_globals->_player.animate(ANIM_MODE_1, NULL);
		g_globals->_player._strip = 4;
		g_globals->_player._moveDiff = Common::Point(3, 5);

		SceneItem::display2(2280, 38);
		g_globals->_player.enableControl();
		remove();
		break;
	}
}

void Scene2280::Action4::signal() {
	Scene2280 *scene = (Scene2280 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		switch (_state) {
		case 1:
		case 51: {
			Common::Point pt(163, 67);
			NpcMover *mover = new NpcMover();
			g_globals->_player.addMover(mover, &pt, this);
			break;
		}
		case 6:
		case 50: {
			Common::Point pt(173, 63);
			NpcMover *mover = new NpcMover();
			g_globals->_player.addMover(mover, &pt, this);
			break;
		}
		}
		break;
	}
	case 1:
		g_globals->_player.setVisage(2161);
		g_globals->_player._strip = 1;
		g_globals->_player._frame = 1;
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 2:
		switch (_state) {
		case 1:
			scene->_hotspot18.remove();
			RING_INVENTORY._scanner._sceneNumber = 1;
			break;
		case 6:
			scene->_hotspot17.remove();
			RING_INVENTORY._medkit._sceneNumber = 1;
			break;
		case 50:
			scene->_hotspot17.postInit();
			scene->_hotspot17.setVisage(2161);
			scene->_hotspot17._strip = 2;
			scene->_hotspot17._frame = 1;
			scene->_hotspot17.animate(ANIM_MODE_NONE, NULL);
			scene->_hotspot17.setPosition(Common::Point(162, 39));
			scene->_hotspot17.changeZoom(100);
			scene->_hotspot17.fixPriority(1);

			RING_INVENTORY._medkit._sceneNumber = 2280;
			g_globals->_sceneItems.push_front(&scene->_hotspot17);
			break;
		case 51:
			scene->_hotspot18.postInit();
			scene->_hotspot18.setVisage(2161);
			scene->_hotspot18._strip = 2;
			scene->_hotspot18._frame = 2;
			scene->_hotspot18.animate(ANIM_MODE_NONE, NULL);
			scene->_hotspot18.setPosition(Common::Point(152, 43));
			scene->_hotspot18.changeZoom(100);
			scene->_hotspot18.fixPriority(1);

			RING_INVENTORY._scanner._sceneNumber = 2280;
			g_globals->_sceneItems.push_front(&scene->_hotspot18);
			break;
		}

		g_globals->_player.animate(ANIM_MODE_6, this);
		break;
	case 3:
		g_globals->_player.enableControl();
		g_globals->_player.setVisage(0);
		g_globals->_player.animate(ANIM_MODE_1, NULL);
		g_globals->_player._strip = 2;

		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene2280::Hotspot1::doAction(int action) {
	Scene2280 *scene = (Scene2280 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		if (g_globals->getFlag(16))
			SceneItem::display2(2280, 22);
		else {
			g_globals->setFlag(16);
			SceneItem::display2(2230, 21);
		}
		break;
	case CURSOR_USE:
		if (!g_globals->getFlag(15))
			SceneItem::display2(2280, 23);
		else if (!g_globals->getFlag(13))
			SceneItem::display2(2280, 46);
		else {
			g_globals->_player.disableControl();
			scene->setAction(&scene->_action1);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2280::Hotspot2::doAction(int action) {
	switch (action) {
	case CURSOR_LOOK:
		if (g_globals->getFlag(2))
			SceneItem::display2(2280, 1);
		else {
			g_globals->setFlag(2);
			SceneItem::display2(2280, 0);
		}
		break;
	case CURSOR_USE:
		SceneItem::display2(2280, 2);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2280::Hotspot4::doAction(int action) {
	switch (action) {
	case CURSOR_LOOK:
		if (g_globals->getFlag(21))
			SceneItem::display2(2280, 33);
		else {
			g_globals->setFlag(21);
			SceneItem::display2(2280, 32);
		}
		break;
	case CURSOR_USE:
		SceneItem::display2(2280, 34);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2280::Hotspot7::doAction(int action) {
	Scene2280 *scene = (Scene2280 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2280, 24);
		break;
	case OBJECT_SCANNER:
		if (g_globals->getFlag(13))
			SceneItem::display2(2280, 43);
		else {
			g_globals->_player.disableControl();
			scene->_action4._state = 51;
			g_globals->_events.setCursor(CURSOR_WALK);
			g_globals->_events._currentCursor = CURSOR_700;
			scene->setAction(&scene->_action4);
		}
		break;
	case OBJECT_MEDKIT:
		if (g_globals->getFlag(13))
			SceneItem::display2(2280, 43);
		else {
			g_globals->_player.disableControl();
			scene->_action4._state = 50;
			g_globals->_events.setCursor(CURSOR_WALK);
			g_globals->_events._currentCursor = CURSOR_700;
			scene->setAction(&scene->_action4);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2280::Hotspot8::doAction(int action) {
	Scene2280 *scene = (Scene2280 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2280, 3);
		break;
	case CURSOR_USE:
		g_globals->_player.disableControl();
		scene->setAction(&scene->_action2);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2280::Hotspot10::doAction(int action) {
	Scene2280 *scene = (Scene2280 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2280, 4);
		break;
	case CURSOR_USE:
		g_globals->_player.disableControl();
		scene->setAction(&scene->_action2);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2280::Hotspot12::doAction(int action) {
	Scene2280 *scene = (Scene2280 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2280, 11);
		break;
	case CURSOR_USE:
		g_globals->_player.disableControl();
		scene->setAction(&scene->_action2);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2280::Hotspot14::doAction(int action) {
	Scene2280 *scene = (Scene2280 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2280, 3);
		break;
	case CURSOR_USE:
		g_globals->_player.disableControl();
		if (g_globals->getFlag(13))
			scene->setAction(&scene->_action3);
		else
			scene->setAction(&scene->_action2);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2280::Hotspot17::doAction(int action) {
	Scene2280 *scene = (Scene2280 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		if (g_globals->getFlag(18))
			SceneItem::display2(2280, 26);
		else {
			g_globals->setFlag(18);
			SceneItem::display2(2280, 25);
		}
		break;
	case CURSOR_USE:
		if (g_globals->getFlag(13))
			SceneItem::display2(2280, 29);
		else {
			g_globals->_player.disableControl();
			scene->_action4._state = 6;
			scene->setAction(&scene->_action4);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2280::Hotspot18::doAction(int action) {
	Scene2280 *scene = (Scene2280 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		if (g_globals->getFlag(19))
			SceneItem::display2(2280, 28);
		else {
			g_globals->setFlag(19);
			SceneItem::display2(2280, 27);
		}
		break;
	case CURSOR_USE:
		if (g_globals->getFlag(13))
			SceneItem::display2(2280, 29);
		else {
			g_globals->_player.disableControl();
			scene->_action4._state = 1;
			scene->setAction(&scene->_action4);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene2280::Scene2280() :
		_hotspot3(0, CURSOR_LOOK, 2280, 30, CURSOR_USE, 2280, 31, LIST_END),
		_hotspot5(0, CURSOR_LOOK, 2280, 35, CURSOR_USE, 2280, 36, LIST_END),
		_hotspot6(0, CURSOR_LOOK, 2280, 19, CURSOR_USE, 2280, 20, LIST_END),
		_hotspot9(0, CURSOR_LOOK, 2280, 5, CURSOR_USE, 2280, 6, LIST_END),
		_hotspot11(0, CURSOR_LOOK, 2280, 7, CURSOR_USE, 2280, 8, LIST_END),
		_hotspot13(0, CURSOR_LOOK, 2280, 9, CURSOR_USE, 2280, 10, LIST_END),
		_hotspot15(0, CURSOR_LOOK, 2280, 45, LIST_END),
		_hotspot16(0, CURSOR_LOOK, 2280, 16, CURSOR_USE, 2280, 42, LIST_END) {
}

void Scene2280::postInit(SceneObjectList *OwnerList) {
	loadScene(2280);
	Scene::postInit();
	setZoomPercents(0, 70, 200, 100);

	_hotspot16.postInit();
	_hotspot16.setVisage(2281);
	_hotspot16.setPosition(Common::Point(208, 90));
	_hotspot16.fixPriority(80);

	if (RING_INVENTORY._medkit._sceneNumber == 2280) {
		_hotspot17.postInit();
		_hotspot17.setVisage(2161);
		_hotspot17._strip = 2;
		_hotspot17.setPosition(Common::Point(162, 39));
		_hotspot17.fixPriority(1);

		g_globals->_sceneItems.push_back(&_hotspot17);
	}

	if (RING_INVENTORY._scanner._sceneNumber == 2280) {
		_hotspot18.postInit();
		_hotspot18.setVisage(2161);
		_hotspot18._strip = 2;
		_hotspot18._frame = 2;
		_hotspot17.setPosition(Common::Point(152, 43));
		_hotspot17.fixPriority(1);

		g_globals->_sceneItems.push_back(&_hotspot17);
	}

	if (!g_globals->getFlag(13)) {
		_hotspot8.postInit();
		_hotspot8.setVisage(2280);
		_hotspot8._strip = 2;
		_hotspot8.setPosition(Common::Point(79, 108));
		_hotspot8.fixPriority(1);

		_hotspot10.postInit();
		_hotspot10.setVisage(2280);
		_hotspot10._strip = 2;
		_hotspot10._frame = 2;
		_hotspot10.setPosition(Common::Point(79, 104));
		_hotspot10.fixPriority(2);

		_hotspot12.postInit();
		_hotspot12.setVisage(2280);
		_hotspot12._strip = 2;
		_hotspot12._frame = 3;
		_hotspot12.setPosition(Common::Point(88, 76));
		_hotspot12.fixPriority(1);

		g_globals->_sceneItems.addItems(&_hotspot8, &_hotspot10, &_hotspot12, NULL);
	}

	_hotspot14.setBounds(Rect(70, 50, 90, 104));

	_hotspot9.postInit();
	_hotspot9.setVisage(2280);
	_hotspot9.setPosition(Common::Point(104, 96));
	_hotspot9.fixPriority(1);

	_hotspot11.postInit();
	_hotspot11.setVisage(2280);
	_hotspot11._frame = 2;
	_hotspot11.setPosition(Common::Point(130, 79));
	_hotspot11.fixPriority(1);

	_hotspot13.postInit();
	_hotspot13.setVisage(2280);
	_hotspot13._frame = 3;
	_hotspot13.setPosition(Common::Point(113, 63));
	_hotspot13.fixPriority(1);

	_hotspot1.setBounds(Rect(225, 70, 234, 80));
	_hotspot2.setBounds(Rect(44, 78, 56, 105));
	_hotspot3.setBounds(Rect(47, 108, 56, 117));
	_hotspot4.setBounds(Rect(56, 78, 69, 101));
	_hotspot5.setBounds(Rect(56, 108, 66, 113));
	_hotspot6.setBounds(Rect(261, 39, 276, 90));
	_hotspot7.setBounds(Rect(142, 19, 174, 38));
	_hotspot15.setBounds(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));

	_exitRect = Rect(145, 180, 195, 195);

	g_globals->_player.postInit();
	g_globals->_player.setVisage(g_globals->getFlag(13) ? 2170 : 0);
	g_globals->_player.animate(ANIM_MODE_1, NULL);
	g_globals->_player.setObjectWrapper(new SceneObjectWrapper());
	g_globals->_player.setPosition(Common::Point(146, 161));
	g_globals->_player.changeZoom(-1);
	g_globals->_player._moveDiff = Common::Point(4, 3);

	if ((g_globals->_sceneManager._previousScene != 7300) && (g_globals->_sceneManager._previousScene != 7000)) {
		g_globals->_player.enableControl();
	} else {
		g_globals->setFlag(109);
		g_globals->_player.fixPriority(76);
		g_globals->_player.disableControl();

		_sceneMode = 2281;
		setAction(&_sequenceManager, this, 2281, &g_globals->_player, &_hotspot16, NULL);

		g_globals->_soundHandler.play(160);
		g_globals->_soundHandler.holdAt(true);
	}

	g_globals->_sceneItems.addItems(&_hotspot13, &_hotspot11, &_hotspot9, &_hotspot14, &_hotspot7,
		&_hotspot6, &_hotspot2, &_hotspot3, &_hotspot4, &_hotspot5, &_hotspot1, &_hotspot16, &_hotspot15, NULL);

	g_globals->_sceneManager._scene->_sceneBounds.contain(g_globals->_sceneManager._scene->_backgroundBounds);
	g_globals->_sceneOffset.x = (g_globals->_sceneManager._scene->_sceneBounds.left / 160) * 160;
}

void Scene2280::signal() {
	if (_sceneMode == 2281) {
		g_globals->_player.fixPriority(-1);
		g_globals->_player.enableControl();
	}
}

void Scene2280::dispatch() {
	Scene::dispatch();
	if (!_action) {
		if (_exitRect.contains(g_globals->_player._position))
			g_globals->_sceneManager.changeScene(2150);
	}
}

void Scene2280::synchronize(Serializer &s) {
	Scene::synchronize(s);
	_exitRect.synchronize(s);
}

/*--------------------------------------------------------------------------
 * Scene 2300 - Starcraft - Hanger Bay
 *
 *--------------------------------------------------------------------------*/

void Scene2300::Action1::signal() {
	// Quinn and Seeker
	Scene2300 *scene = (Scene2300 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(10);
		break;
	case 1: {
		Common::Point pt(SCREEN_WIDTH, SCREEN_HEIGHT);
		NpcMover *mover = new NpcMover();
		scene->_hotspot2.addMover(mover, &pt, this);
		break;
	}
	case 2: {
		scene->_hotspot8.setAction(&scene->_action4);
		scene->_soundHandler2.play(21);

		Common::Point pt1(95, scene->_hotspot5._position.y);
		NpcMover *mover1 = new NpcMover();
		scene->_hotspot5.addMover(mover1, &pt1, this);

		Common::Point pt2(99, scene->_hotspot6._position.y);
		NpcMover *mover2 = new NpcMover();
		scene->_hotspot6.addMover(mover2, &pt2, NULL);
		break;
	}
	case 3:
		setDelay(2);
		g_globals->_player._uiEnabled = true;
		g_globals->_events.setCursor(CURSOR_WALK);
		break;
	case 4: {
		scene->_hotspot5.setVisage(93);
		scene->_hotspot6.setVisage(94);
		scene->_hotspot5.setStrip(1);
		scene->_hotspot6.setStrip(1);
		scene->_hotspot5.changeZoom(50);
		scene->_hotspot6.changeZoom(50);
		scene->_hotspot5._moveDiff.x = 5;
		scene->_hotspot6._moveDiff.x = 5;
		scene->_hotspot5.animate(ANIM_MODE_1, NULL);
		scene->_hotspot6.animate(ANIM_MODE_1, NULL);

		Common::Point pt1(178, 147);
		NpcMover *mover1 = new NpcMover();
		scene->_hotspot5.addMover(mover1, &pt1, this);

		Common::Point pt2(170, 145);
		NpcMover *mover2 = new NpcMover();
		scene->_hotspot6.addMover(mover2, &pt2, this);
		break;
	}
	case 6:
		scene->_soundHandler1.play(28);
		g_globals->_player.disableControl();

		scene->_hotspot2.setVisage(40);
		scene->_hotspot2.setStrip(4);
		scene->_hotspot2.setFrame(1);
		scene->_hotspot2.animate(ANIM_MODE_5, NULL);

		g_globals->_player.setVisage(40);
		g_globals->_player.setStrip(2);
		g_globals->_player.setFrame(1);
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 7:
		g_globals->_soundHandler.play(77, this);
		break;
	case 8:
		g_globals->_game->endGame(2300, 0);
		remove();
		break;
	case 9:
		if (scene->_hotspot5._mover)
			scene->_hotspot5.addMover(NULL);
		if (scene->_hotspot6._mover)
			scene->_hotspot6.addMover(NULL);

		scene->_hotspot5.animate(ANIM_MODE_NONE, NULL);
		scene->_hotspot6.animate(ANIM_MODE_NONE, NULL);

		g_globals->_player.disableControl();
		g_globals->_player.setVisage(2672);
		g_globals->_player._strip = 5;
		g_globals->_player._frame = 1;
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 10:
		g_globals->_player.setVisage(2674);
		g_globals->_player._strip = 5;
		g_globals->_player._frame = 1;
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 11:
		scene->_soundHandler1.play(28);

		scene->_hotspot5._strip = 2;
		scene->_hotspot6._strip = 2;
		scene->_hotspot5._frame = 1;
		scene->_hotspot6._frame = 1;
		scene->_hotspot5.animate(ANIM_MODE_5, NULL);
		scene->_hotspot6.animate(ANIM_MODE_5, NULL);
		scene->_hotspot5.fixPriority(20);
		scene->_hotspot6.fixPriority(20);

		g_globals->_player.setVisage(2672);
		g_globals->_player._strip = 5;
		g_globals->_player._frame = g_globals->_player.getFrameCount();
		g_globals->_player.animate(ANIM_MODE_6, this);
		break;
	case 12:
		scene->_soundHandler1.play(77);
		g_globals->_player.setVisage(0);
		g_globals->_player.animate(ANIM_MODE_1, NULL);
		g_globals->_player.setStrip(4);
		g_globals->_player._uiEnabled = false;
		setDelay(60);
		break;
	case 13:
		scene->_stripManager.start(2300, this);
		break;
	case 14:
		setDelay(5);
		_actionIndex = 16;
		break;
	case 15: {
		Common::Point pt1(101, 148);
		PlayerMover *mover1 = new PlayerMover();
		g_globals->_player.addMover(mover1, &pt1, this);

		Common::Point pt2(scene->_hotspot5._position.x + 5, scene->_hotspot5._position.y + 5);
		NpcMover *mover2 = new NpcMover();
		scene->_hotspot2.addMover(mover2, &pt2, NULL);
		break;
	}
	case 16:
		g_globals->_sceneManager.changeScene(2000);
		remove();
		break;
	}
}

void Scene2300::Action2::signal() {
	// Miranda tearing cables
	Scene2300 *scene = (Scene2300 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		setDelay(30);
		break;
	case 1: {
		Common::Point pt(153, 135);
		PlayerMover *mover = new PlayerMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 2:
		g_globals->_events.setCursor(CURSOR_WALK);
		scene->_stripManager.start(2030, this);
		break;
	case 3:
		g_globals->_player.enableControl();
		setDelay(600);
		break;
	case 4: {
		g_globals->_player.disableControl();

		scene->_hotspot2.postInit();
		scene->_hotspot2.setVisage(2801);
		scene->_hotspot2.setPosition(Common::Point(109, 145));
		scene->_hotspot2.changeZoom(50);
		scene->_hotspot2.animate(ANIM_MODE_1, NULL);
		scene->_hotspot2.setObjectWrapper(new SceneObjectWrapper());

		Common::Point pt(160, 145);
		NpcMover *mover = new NpcMover();
		scene->_hotspot2.addMover(mover, &pt, this);
		break;
	}
	case 5:
		scene->_stripManager.start(2032, this);
		break;
	case 6:
		scene->_hotspot2.setVisage(93);
		scene->_hotspot2.setStrip(3);
		scene->_hotspot2.setFrame(1);
		scene->_hotspot2.animate(ANIM_MODE_5, this);

		scene->_soundHandler1.play(28);
		scene->_soundHandler2.play(97);
		break;
	case 7:
		scene->_hotspot7._strip = 2;
		scene->_hotspot7._frame = 1;
		scene->_hotspot7.animate(ANIM_MODE_5, this);
		break;
	case 8:
		scene->_hotspot2.animate(ANIM_MODE_6, this);
		break;
	case 9:
		scene->_stripManager.start(2035, this);
		break;
	case 10:
		g_globals->_sceneManager.changeScene(2310);
		break;
	}
}

void Scene2300::Action3::signal() {
	// Stunned Miranda
	Scene2300 *scene = (Scene2300 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		Common::Point pt(153, 135);
		PlayerMover *mover = new PlayerMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 1:
		g_globals->_player.disableControl();
		g_globals->_player.setVisage(2672);
		g_globals->_player._strip = 2;
		g_globals->_player._frame = 1;
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 2:
		g_globals->_player.setVisage(2674);
		g_globals->_player._strip = 2;
		g_globals->_player._frame = 1;
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 3:
		scene->_soundHandler1.play(97);
		scene->_soundHandler2.play(28);

		scene->_hotspot7._strip = 2;
		scene->_hotspot7._frame = 1;
		scene->_hotspot7.animate(ANIM_MODE_5, this);

		g_globals->_player.setVisage(2672);
		g_globals->_player._strip = 2;
		g_globals->_player._frame = g_globals->_player.getFrameCount();
		g_globals->_player.animate(ANIM_MODE_6, NULL);
		break;
	case 4:
		scene->_soundHandler2.play(97);
		g_globals->_player.setVisage(0);
		g_globals->_player.animate(ANIM_MODE_1, NULL);
		g_globals->_player.setStrip(1);
		scene->_stripManager.start(2034, this);
		break;
	case 5:
		setDelay(10);
		break;
	case 6:
		g_globals->_sceneManager.changeScene(2310);
		break;
	}
}

void Scene2300::Action4::signal() {
	// Ennemies coming
	Scene2300 *scene = (Scene2300 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_hotspot8.animate(ANIM_MODE_5, this);
		scene->_soundHandler1.play(11);
		break;
	case 1:
		scene->_hotspot9.postInit();
		scene->_hotspot9.setVisage(2301);
		scene->_hotspot9.setStrip2(2);
		scene->_hotspot9.setFrame(3);
		scene->_hotspot9.setPosition(Common::Point(273, 199));
		scene->_hotspot9.fixPriority(19);
		scene->_hotspot9.animate(ANIM_MODE_5, this);
		scene->_soundHandler1.play(11);
		break;
	case 2:
		scene->_hotspot8.remove();

		scene->_hotspot10.postInit();
		scene->_hotspot10.setVisage(2301);
		scene->_hotspot10.setStrip(3);
		scene->_hotspot10.setFrame(4);
		scene->_hotspot10.setPosition(Common::Point(292, 113));
		scene->_hotspot10.animate(ANIM_MODE_5, this);
		scene->_soundHandler1.play(11);
		break;
	case 3:
		scene->_soundHandler1.play(13);
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene2300::Hotspot5::doAction(int action) {
	// Ennemies
	Scene2300 *scene = (Scene2300 *)g_globals->_sceneManager._scene;

	switch (action) {
	case OBJECT_STUNNER:
		g_globals->_player.disableControl();
		g_globals->_player.addMover(NULL);
		scene->_action1.setActionIndex(9);
		scene->_action1.setDelay(1);
		break;
	case CURSOR_LOOK:
		SceneItem::display2(2300, 3);
		break;
	case CURSOR_USE:
		SceneItem::display2(2300, 4);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2300::Hotspot7::doAction(int action) {
	// Miranda
	Scene2300 *scene = (Scene2300 *)g_globals->_sceneManager._scene;

	switch (action) {
	case OBJECT_STUNNER:
		scene->setAction(&scene->_action3);
		break;
	case CURSOR_LOOK:
		SceneItem::display2(2300, 2);
		break;
	case CURSOR_USE:
		SceneItem::display2(2300, 21);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene2300::Scene2300() :
		_hotspot11(0, CURSOR_LOOK, 2300, 6, CURSOR_USE, 2300, 14, LIST_END),
		_hotspot12(0, CURSOR_LOOK, 2300, 5, CURSOR_USE, 2300, 13, LIST_END),
		_hotspot13(0, CURSOR_LOOK, 2300, 9, CURSOR_USE, 2300, 20, LIST_END),
		_hotspot14(0, CURSOR_LOOK, 2300, 7, CURSOR_USE, 2300, 15, LIST_END),
		_hotspot15(0, CURSOR_LOOK, 2300, 1, LIST_END) {
}

void Scene2300::postInit(SceneObjectList *OwnerList) {
	loadScene(2300);
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);

	_stripManager.addSpeaker(&_speakerSL);
	_stripManager.addSpeaker(&_speakerMText);
	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerSText);

	RING_INVENTORY._stunner._sceneNumber = 1;
	RING_INVENTORY._infoDisk._sceneNumber = 1;

	_hotspot8.postInit();
	_hotspot8.setVisage(2301);
	_hotspot8.setPosition(Common::Point(288, 74));

	g_globals->_soundHandler.play(96);
	if (g_globals->_sceneManager._previousScene == 2000) {
		_hotspot8.remove();

		_hotspot9.postInit();
		_hotspot9.setVisage(2301);
		_hotspot9.setStrip2(2);
		_hotspot9._frame = _hotspot9.getFrameCount();
		_hotspot9.setPosition(Common::Point(273, 199));
		_hotspot9.fixPriority(1);

		_hotspot10.postInit();
		_hotspot10.setVisage(2301);
		_hotspot10.setStrip(3);
		_hotspot10._frame = _hotspot10.getFrameCount();
		_hotspot10.setPosition(Common::Point(292, 113));

		g_globals->_player.postInit();
		g_globals->_player.setVisage(0);
		g_globals->_player.setPosition(Common::Point(109, 139));
		g_globals->_player.changeZoom(40);
		g_globals->_player.animate(ANIM_MODE_1, NULL);
		g_globals->_player.setObjectWrapper(new SceneObjectWrapper());
		g_globals->_player.disableControl();

		_hotspot7.postInit();
		_hotspot7.setVisage(2302);
		_hotspot7.animate(ANIM_MODE_2, NULL);
		_hotspot7.setPosition(Common::Point(229, 125));
		_hotspot7._numFrames = 5;

		_soundHandler1.play(95);
		_soundHandler2.play(96);
		g_globals->_sceneItems.push_back(&_hotspot7);

		setAction(&_action2);
	} else {
		_hotspot8._numFrames = 3;

		_hotspot3.postInit();
		_hotspot3.setVisage(2331);
		_hotspot3.setStrip(7);
		_hotspot3.setPosition(Common::Point(231, 190));
		_hotspot3.changeZoom(40);

		_hotspot2.postInit();
		_hotspot2.setVisage(2801);
		_hotspot2.setPosition(Common::Point(269, 195));
		_hotspot2.changeZoom(50);
		_hotspot2.animate(ANIM_MODE_1, NULL);
		_hotspot2.setObjectWrapper(new SceneObjectWrapper());

		_hotspot1.postInit();
		_hotspot1.setVisage(2331);
		_hotspot1.setStrip(7);
		_hotspot1.setPosition(Common::Point(255, 190));
		_hotspot1.changeZoom(40);

		g_globals->_player.postInit();
		g_globals->_player.setVisage(0);
		g_globals->_player.setPosition(Common::Point(203, 191));
		g_globals->_player.changeZoom(40);
		g_globals->_player.animate(ANIM_MODE_1, NULL);
		g_globals->_player.setObjectWrapper(new SceneObjectWrapper());
		g_globals->_player.disableControl();

		_hotspot5.postInit();
		_hotspot5.setVisage(91);
		_hotspot5.setPosition(Common::Point(343, 145));
		_hotspot5._moveDiff = Common::Point(35, 35);
		_hotspot5.changeZoom(40);
		_hotspot5.setObjectWrapper(new SceneObjectWrapper());

		_hotspot6.postInit();
		_hotspot6.setVisage(92);
		_hotspot6.setPosition(Common::Point(343, 130));
		_hotspot6._moveDiff = Common::Point(35, 35);
		_hotspot6.changeZoom(40);
		_hotspot6.setObjectWrapper(new SceneObjectWrapper());

		g_globals->_sceneItems.push_back(&_hotspot5);
		setAction(&_action1);
	}

	_hotspot12._sceneRegionId = 8;
	_hotspot11._sceneRegionId = 9;
	_hotspot13._sceneRegionId = 10;
	_hotspot14._sceneRegionId = 11;

	_hotspot15.setBounds(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
	g_globals->_sceneItems.addItems(&_hotspot12, &_hotspot11, &_hotspot13, &_hotspot14, &_hotspot15, NULL);
}

/*--------------------------------------------------------------------------
 * Scene 2310 - Starcraft - Copy Protection Screen
 *
 *--------------------------------------------------------------------------*/

Scene2310::Scene2310() {
	_pageList[0].set(0, 0, 0, 0, 0, 0);
	_pageList[1].set(1, 0, 3, 2, 1, 4);
	_pageList[2].set(2, 1, 4, 3, 2, 0);
	_pageList[3].set(3, 2, 0, 4, 3, 1);
	_pageList[4].set(4, 3, 1, 0, 2, 4);
	_pageList[5].set(5, 4, 2, 1, 3, 0);
	_pageList[6].set(6, 0, 4, 2, 3, 1);
	_pageList[7].set(7, 1, 0, 2, 4, 3);
	_pageList[8].set(8, 2, 1, 3, 0, 4);
	_pageList[9].set(9, 3, 2, 4, 1, 0);
	_pageList[10].set(10, 4, 1, 2, 0, 3);
	_pageList[11].set(11, 0, 2, 3, 4, 1);
	_pageList[12].set(12, 1, 2, 0, 4, 3);
	_pageList[13].set(13, 2, 4, 0, 3, 1);
	_pageList[14].set(14, 3, 4, 1, 0, 2);
	_pageList[15].set(15, 4, 3, 1, 2, 0);
	_pageList[16].set(16, 0, 1, 4, 2, 3);
	_pageList[17].set(17, 1, 3, 4, 0, 2);
	_pageList[18].set(18, 2, 3, 0, 1, 4);
	_pageList[19].set(19, 3, 0, 1, 4, 2);
	_pageList[20].set(20, 4, 0, 3, 1, 2);

	_rectList[0].set(135, 70, 151, 140);
	_rectList[1].set(151, 70, 167, 140);
	_rectList[2].set(167, 70, 183, 140);
	_rectList[3].set(183, 70, 199, 140);
	_rectList[4].set(199, 70, 215, 140);

	_wireIndex = 5;
}

void Scene2310::postInit(SceneObjectList *OwnerList) {
	Scene::postInit();
	loadScene(2301);
	Common::Point pointList[5] = { Common::Point(142, 82), Common::Point(158, 82), Common::Point(174, 82),
		Common::Point(190, 82), Common::Point(205, 82) };

	for (int idx = 0; idx < 5; ++idx) {
		_wireList[idx].postInit();
		_wireList[idx].setVisage(2300);
		_wireList[idx]._strip = idx + 1;
		_wireList[idx]._frame = 1;
		_wireList[idx].setPosition(pointList[idx]);
	}

	g_globals->_player.disableControl();
	g_globals->_events.setCursor(CURSOR_WALK);

	if (g_vm->getFeatures() & GF_CD)
		_pageIndex = g_globals->_randomSource.getRandomNumber(14) + 2;
	else
		_pageIndex = g_globals->_randomSource.getRandomNumber(19) + 1;
	signal();
}

void Scene2310::signal() {
	switch (_sceneMode++) {
	case 0: {
		Common::String fmtString = g_resourceManager->getMessage(2300, 22);
		Common::String msg = Common::String::format(fmtString.c_str(), _pageList[_pageIndex]._pageNumber);

		_sceneText._width = 280;
		_sceneText._textMode = ALIGN_CENTER;
		_sceneText._color1 = 35;
		_sceneText._fontNumber = 2;
		_sceneText.setup(msg);
		_sceneText.fixPriority(255);
		_sceneText.setPosition(Common::Point(30, 20));
		break;
	}
	case 1: {
		Common::String msg = g_resourceManager->getMessage(2300, 23);
		_sceneText.setup(msg);
		_sceneText.fixPriority(255);
		_sceneText.setPosition(Common::Point(30, 170));

		g_globals->_sceneObjects->draw();
		g_globals->_events.waitForPress();

		_sceneText.hide();
		g_globals->_sceneObjects->draw();

		g_globals->_sceneManager.changeScene(2200);
		break;
	}
	}
}

void Scene2310::synchronize(Serializer &s) {
	Scene::synchronize(s);

	s.syncAsSint16LE(_wireIndex);
	s.syncAsSint16LE(_pageIndex);
}

void Scene2310::process(Event &event) {
	int frameNum = 0;

	if (!event.handled && (event.eventType == EVENT_BUTTON_DOWN)) {
		int idx = 0;
		while (idx < 5) {
			if (_rectList[idx].contains(event.mousePos))
				break;
			++idx;
		}

		if (idx < 5) {
			// In handled rectangle area
			if (_wireIndex == 5) {
				// No wire is currently active, so start moving designated wire
				_wireIndex = idx;
				frameNum = idx + 2;

				if (event.mousePos.y > 105)
					idx = findObject(idx);

				if (idx != 5) {
					_wireList[idx].hide();
					g_globals->_sceneObjects->draw();
					_wireList[idx].setFrame(frameNum);

					_wireList[idx].show();
					g_globals->_sceneObjects->draw();
				}
			} else {
				// End the moving of the currently active wire
				_wireList[_wireIndex].setFrame(idx + 2);
				_wireIndex = 5;

				// Check if solution has been reached
				int idx2 = 0;
				do {
					int objIndex = findObject(idx2);
					if (_pageList[_pageIndex]._connectionList[idx2] != objIndex)
						// Mismatch
						break;
				} while (++idx2 < 5);

				if (idx2 == 5)
					// All the entries are correct
					signal();
			}

			event.handled = true;
		} else if (_wireIndex != 5) {
			// Reset the active wire back to unplugged
			_wireList[_wireIndex].setFrame(1);
			_wireIndex = 5;
		}
	}
}

void Scene2310::dispatch() {
	if (_wireIndex != 5) {
		for (int idx = 0; idx < 5; ++idx) {
			if (_rectList[idx].contains(g_globals->_events._mousePos)) {
				_wireList[_wireIndex].setFrame(idx + 2);
				return;
			}
		}

		_wireList[_wireIndex].setFrame(1);
	}
}

int Scene2310::findObject(int objIndex) {
	for (int idx = 0; idx < 5; ++idx) {
		if (_wireList[idx]._frame == (objIndex + 2))
			return idx;
	}

	return 5;
}

/*--------------------------------------------------------------------------
 * Scene 2320 - Starcraft - Lander Bay
 *
 *--------------------------------------------------------------------------*/

void Scene2320::Action1::signal() {
	Scene2320 *scene = (Scene2320 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(10);
		if (g_globals->_sceneManager._previousScene == 2120)
			_actionIndex = 3;
		break;
	case 1: {
		Common::Point pt(513, 144);
		NpcMover *mover = new NpcMover();
		scene->_hotspot11.addMover(mover, &pt, this);
		break;
	}
	case 2: {
		Common::Point pt(510, 164);
		NpcMover *mover = new NpcMover();
		scene->_hotspot11.addMover(mover, &pt, this);
		break;
	}
	case 3:
		setAction(&scene->_sequenceManager2, this, 2328, &scene->_hotspot11, NULL);
		break;
	case 4:
		scene->_hotspot11.animate(ANIM_MODE_NONE);
		setDelay(120);
		_actionIndex = 3;
		break;
	}
}

void Scene2320::Action2::signal() {
	Scene2320 *scene = (Scene2320 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		scene->_soundHandler.play(253);
		scene->_hotspot13.fixPriority(99);

		Common::Point pt(scene->_hotspot13._position.x, 200);
		NpcMover *mover = new NpcMover();
		scene->_hotspot13.addMover(mover, &pt, this);
		break;
	}
	case 1:
		scene->_hotspot13.hide();
		remove();
		break;
	}
}

void Scene2320::Action3::signal() {
	Scene2320 *scene = (Scene2320 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		Common::Point pt(320, 86);
		PlayerMover *mover = new PlayerMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 1:
		scene->_soundHandler.play(162);
		scene->_hotspot6.animate(ANIM_MODE_5, this);
		break;
	case 2: {
		Common::Point pt(320, 79);
		NpcMover *mover = new NpcMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 3:
		g_globals->_player.fixPriority(scene->_hotspot6._priority - 1);
		g_globals->_player._strip = 3;
		setDelay(10);
		break;
	case 4:
		scene->_area1.display();
		scene->_area2.display();
		scene->_area3.display();
		scene->_area4.display();

		scene->_area3.draw(true);
		_state = 0;
		g_globals->_events.setCursor(CURSOR_USE);

		while (!_state && !g_vm->shouldQuit()) {
			// Wait for an event
			Event event;
			if (!g_globals->_events.getEvent(event)) {
				GLOBALS._screenSurface.updateScreen();
				g_system->delayMillis(10);
				continue;
			}

			if (scene->_area1._bounds.contains(event.mousePos)) {
				scene->_area1.draw(true);
				scene->_area3.draw(false);
				_state = scene->_area1._actionId;
			}
			if (scene->_area2._bounds.contains(event.mousePos)) {
				scene->_area3.draw(false);
				scene->_area2.draw(true);
				_state = scene->_area2._actionId;
			}
			if (scene->_area3._bounds.contains(event.mousePos)) {
				scene->_area3.draw(true);
				_state = scene->_area3._actionId;
			}
		}

		scene->_soundHandler.play(161);
		scene->_area1.restore();
		scene->_area2.restore();
		scene->_area3.restore();
		scene->_area4.restore();

		if (_state == 2320) {
			setDelay(10);
		} else {
			scene->_soundHandler.play(162);
			scene->_hotspot6.animate(ANIM_MODE_6, this);
		}
		break;
	case 5: {
		if (_state == 2320)
			g_globals->_player.fixPriority(-1);
		else
			g_globals->_sceneManager.changeScene(_state);

		Common::Point pt(320, 86);
		NpcMover *mover = new NpcMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 6:
		scene->_soundHandler.play(162);
		scene->_hotspot6.animate(ANIM_MODE_6, this);
		break;
	case 7:
		g_globals->_player.enableControl();
		remove();
		break;
	}
}

void Scene2320::Action4::signal() {
	// Fly Cycle actions
	Scene2320 *scene = (Scene2320 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		setDelay(3);
		break;
	case 1: {
		Common::Point pt(213, 84);
		PlayerMover *mover = new PlayerMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 2:
	case 16:
		g_globals->_player.setVisage(2109);
		g_globals->_player.setStrip(2);
		g_globals->_player.setFrame(1);
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 3:
		g_globals->_player.animate(ANIM_MODE_6, NULL);
		setAction(&scene->_action2, this);
		break;
	case 4: {
		scene->_hotspot16.postInit();
		scene->_hotspot16.setVisage(2331);
		scene->_hotspot16.setStrip(3);
		scene->_hotspot16.fixPriority(149);
		scene->_hotspot16.setPosition(Common::Point(320, 202));
		scene->_hotspot16.show();

		Common::Point pt(320, 121);
		NpcMover *mover = new NpcMover();
		scene->_hotspot16.addMover(mover, &pt, this);
		break;
	}
	case 5: {
		scene->_hotspot16.fixPriority(200);
		Common::Point pt(320, 180);
		NpcMover *mover = new NpcMover();
		scene->_hotspot16.addMover(mover, &pt, this);
		break;
	}
	case 6: {
		scene->_hotspot16.fixPriority(-1);
		g_globals->_player.setVisage(0);
		g_globals->_player.animate(ANIM_MODE_1, NULL);

		Common::Point pt(233, 176);
		PlayerMover *mover = new PlayerMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 7: {
		Common::Point pt(291, 194);
		PlayerMover *mover = new PlayerMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 8:
		g_globals->_player.setStrip(5);
		g_globals->_player.setFrame(1);
		setDelay(13);
		break;
	case 9:
		// Quinn sits in the flycycle
		scene->_hotspot16.hide();
		g_globals->_player.setVisage(2323);
		g_globals->_player.setPosition(Common::Point(303, 176));
		g_globals->_player.setStrip(2);
		g_globals->_player.setFrame(1);
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 10:
		if (g_globals->getFlag(109)) {
			g_globals->_soundHandler.play(40);
			g_globals->_soundHandler.holdAt(true);

			Common::Point pt(303, 240);
			NpcMover *mover = new NpcMover();
			g_globals->_player.addMover(mover, &pt, this);
		} else {
			setDelay(3);
		}
		break;
	case 11:
		if (g_globals->getFlag(109)) {
			g_globals->_sceneManager.changeScene(7600);
		} else {
			SceneItem::display2(2320, 19);
			setDelay(3);
		}
		break;
	case 12:
		g_globals->_player.animate(ANIM_MODE_6, this);
		break;
	case 13:
		g_globals->_player.setVisage(0);
		g_globals->_player.setPosition(Common::Point(291, 194));
		g_globals->_player.setStrip(5);
		g_globals->_player.setFrame(1);
		g_globals->_player.animate(ANIM_MODE_1, NULL);

		scene->_hotspot16.show();
		setDelay(3);
		break;
	case 14: {
		Common::Point pt(233, 176);
		PlayerMover *mover = new PlayerMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 15: {
		Common::Point pt(213, 85);
		PlayerMover *mover = new PlayerMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 17: {
		g_globals->_player.animate(ANIM_MODE_6, NULL);
		scene->_hotspot16.fixPriority(160);

		Common::Point pt(320, 121);
		NpcMover *mover = new NpcMover();
		scene->_hotspot16.addMover(mover, &pt, this);
		break;
	}
	case 18: {
		scene->_hotspot16.fixPriority(149);
		Common::Point pt(320, 202);
		PlayerMover *mover = new PlayerMover();
		scene->_hotspot16.addMover(mover, &pt, this);
		break;
	}
	case 19: {
		scene->_hotspot16.remove();
		scene->_soundHandler.play(253);

		scene->_hotspot13.show();
		Common::Point pt(319, 157);
		NpcMover *mover = new NpcMover();
		scene->_hotspot13.addMover(mover, &pt, this);
		break;
	}
	case 20:
		g_globals->_player.enableControl();
		g_globals->_player.setVisage(0);
		g_globals->_player.animate(ANIM_MODE_1, NULL);

		scene->_hotspot13.fixPriority(1);
		remove();
		break;
	}
}

void Scene2320::Action5::signal() {
	Scene2320 *scene = (Scene2320 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		ADD_PLAYER_MOVER(163, 126);
		break;
	case 1:
		ADD_PLAYER_MOVER(165, 132);
		break;
	case 2:
		setDelay(2);
		break;
	case 3:
		if (!g_globals->getFlag(59))
			setDelay(10);
		else
			scene->_stripManager.start(2323, this);
		break;
	case 4:
		g_globals->_player.setVisage(2347);
		g_globals->_player.setStrip(1);
		g_globals->_player.setFrame(1);
		g_globals->_player.fixPriority(137);
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 5:
		g_globals->_player.setStrip(2);
		g_globals->_player.setFrame(1);
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 6:
		if (g_globals->getFlag(72))
			g_globals->_sceneManager.changeScene(7000);
		else if (g_globals->getFlag(59))
			g_globals->_sceneManager.changeScene(5000);
		else if (!g_globals->getFlag(43) || (RING_INVENTORY._ale._sceneNumber != 1))
			setDelay(10);
		else {
			scene->_hotspot11.setAction(NULL);
			scene->_hotspot11.setVisage(2705);
			scene->_hotspot11.animate(ANIM_MODE_1, NULL);
			scene->_hotspot11.setObjectWrapper(new SceneObjectWrapper());

			Common::Point pt(185, 135);
			NpcMover *mover = new NpcMover();
			scene->_hotspot11.addMover(mover, &pt, NULL);

			g_globals->clearFlag(43);
			scene->_stripManager.start(2325, this);
		}
		break;
	case 7:
		setDelay(10);
		break;
	case 8:
		g_globals->_sceneManager.changeScene(4000);
		break;
	}
}

void Scene2320::Action6::signal() {
	Scene2320 *scene = (Scene2320 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(30);
		break;
	case 1:
		g_globals->_player.animate(ANIM_MODE_6, this);
		break;
	case 2:
		g_globals->_player.setStrip(1);
		g_globals->_player.setFrame(1);
		g_globals->_player.animate(ANIM_MODE_6, this);
		break;
	case 3:
		g_globals->_player.setVisage(0);
		g_globals->_player.setStrip(3);
		g_globals->_player.fixPriority(-1);
		g_globals->_player.setObjectWrapper(new SceneObjectWrapper());
		g_globals->_player.animate(ANIM_MODE_1, NULL);

		setDelay(60);
		break;
	case 4:
		if ((g_globals->_sceneManager._previousScene != 4000) || g_globals->getFlag(43))
			setDelay(3);
		else if (g_globals->getFlag(35)) {
			g_globals->setFlag(43);
			scene->_stripManager.start(4200, this);
			g_globals->setFlag(69);
		} else if (g_globals->getFlag(36)) {
			setDelay(3);
		} else {
			g_globals->setFlag(43);
			scene->_stripManager.start(4210, this);
			break;
		}
		break;
	case 5:
		if (g_globals->_sceneObjects->contains(&scene->_hotspot11)) {
			scene->_hotspot11.setAction(&scene->_action1);

			if (g_globals->_sceneObjects->contains(&scene->_hotspot10)) {
				ADD_MOVER(scene->_hotspot10, 491, 160);
			} else {
				setDelay(60);
			}

			g_globals->_sceneItems.push_front(&scene->_hotspot11);
		} else {
			setDelay(3);
		}
		break;
	case 6:
		g_globals->_events.setCursor(CURSOR_WALK);
		g_globals->_player.enableControl();
		remove();
		break;
	}
}

void Scene2320::Action7::signal() {
	Scene2320 *scene = (Scene2320 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(30);
		break;
	case 1:
		g_globals->_soundHandler.play(162);
		scene->_hotspot6.animate(ANIM_MODE_5, this);
		break;
	case 2:
		scene->_hotspot10.fixPriority(-1);
		ADD_MOVER_NULL(scene->_hotspot10, 321, 94);
		scene->_hotspot11.fixPriority(-1);
		ADD_MOVER_NULL(scene->_hotspot11, 346, 85);

		g_globals->_player.fixPriority(-1);
		ADD_MOVER(g_globals->_player, 297, 89);
		break;
	case 3:
		ADD_PLAYER_MOVER(462, 182);
		break;
	case 4:
		ADD_MOVER(scene->_hotspot11, 391, 88);
		break;
	case 5:
		ADD_MOVER(scene->_hotspot11, 500, 164);
		ADD_MOVER(scene->_hotspot10, 382, 93);
		g_globals->_player.setStrip(3);
		break;
	case 6:
		ADD_MOVER_NULL(scene->_hotspot10, 491, 160);
		ADD_MOVER(g_globals->_player, 391, 88);
		break;
	case 7:
		ADD_PLAYER_MOVER(462, 182);
		break;
	case 8:
		g_globals->_player.setStrip(7);
		setDelay(15);
		break;
	case 9:
		scene->_stripManager.start(6020, this);
		break;
	case 10:
		setDelay(6);
		break;
	case 11:
		scene->_stripManager.start(6022, this);
		break;
	case 12:
		g_globals->_player.enableControl();
		RING_INVENTORY._stasisBox._sceneNumber = 2320;
		break;
	}
}

void Scene2320::Action8::signal() {
	Scene2320 *scene = (Scene2320 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(5);
		break;
	case 1:
		ADD_PLAYER_MOVER(462, 182);
		break;
	case 2:
		g_globals->_player.setStrip(7);
		setDelay(5);
		break;
	case 3:
		scene->_speakerGameText.setTextPos(Common::Point(g_globals->_sceneManager._scene->_sceneBounds.left + 30, 10));
		scene->_stripManager.start(g_globals->getFlag(75) ? 6030 : 2320, this);
		break;
	case 4:
		if (g_globals->getFlag(75))
			setDelay(3);
		else
			g_globals->_player.enableControl();
		break;
	case 5:
		RING_INVENTORY._stasisBox._sceneNumber = 2320;
		scene->_sceneMode = 2326;
		scene->_speakerGameText.setTextPos(Common::Point(g_globals->_sceneManager._scene->_sceneBounds.left + 30, 10));
		scene->setAction(&scene->_sequenceManager1, scene, 2326, &g_globals->_player, &scene->_hotspot11, NULL);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene2320::Hotspot5::doAction(int action) {
	Scene2320 *scene = (Scene2320 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2320, 13);
		break;
	case CURSOR_USE:
		if (g_globals->getFlag(70))
			SceneItem::display2(2320, 33);
		else if (g_globals->getFlag(13))
			SceneItem::display2(2320, 18);
		else
			scene->setAction(&scene->_action5);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2320::Hotspot6::doAction(int action) {
	Scene2320 *scene = (Scene2320 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2320, 2);
		break;
	case CURSOR_USE:
		g_globals->_player.disableControl();
		scene->setAction(&scene->_action3);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2320::Hotspot8::doAction(int action) {
	Scene2320 *scene = (Scene2320 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2320, 23);
		break;
	case CURSOR_USE:
		scene->_sceneMode = 2336;
		scene->setAction(&scene->_sequenceManager1, scene, 2336, &g_globals->_player, this, NULL);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2320::Hotspot10::doAction(int action) {
	// Seeker
	Scene2320 *scene = (Scene2320 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2320, 26);
		break;
	case CURSOR_TALK:
		g_globals->_player.disableControl();
		scene->_sceneMode = 2329;

		if (g_globals->getFlag(13)) {
			scene->_stripManager.start(2337, scene);
		} else if (g_globals->getFlag(70)) {
			scene->setAction(&scene->_action8);
		} else if (g_globals->getFlag(109)) {
			scene->setAction(&scene->_sequenceManager1, scene, 2337, NULL);
		} else if (!_state) {
			++_state;
			scene->setAction(&scene->_sequenceManager1, scene, 2334, NULL);
		} else {
			scene->setAction(&scene->_sequenceManager1, scene, 2335, NULL);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2320::Hotspot11::doAction(int action) {
	Scene2320 *scene = (Scene2320 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2320, 25);
		break;
	case CURSOR_TALK:
		g_globals->_player.disableControl();
		scene->_sceneMode = 2329;

		if (g_globals->getFlag(13)) {
			g_globals->_player.disableControl();
			scene->_sceneMode = 5000;
			scene->_stripManager.start(2336, scene);
		} else if (g_globals->getFlag(70)) {
			scene->setAction(&scene->_action8);
		} else {
			scene->_sceneMode = 2329;

			if (RING_INVENTORY._ale._sceneNumber == 1) {
				scene->setAction(&scene->_sequenceManager1, scene, 2329, NULL);
			} else if (!g_globals->getFlag(110)) {
				g_globals->setFlag(110);
				scene->setAction(&scene->_sequenceManager1, scene, 2330, NULL);
			} else if (RING_INVENTORY._peg._sceneNumber != 1) {
				scene->setAction(&scene->_sequenceManager1, scene, 2331, NULL);
			} else if (!_state) {
				++_state;
				scene->setAction(&scene->_sequenceManager1, scene, 2332, NULL);
			} else {
				scene->setAction(&scene->_sequenceManager1, scene, 2333, NULL);
			}
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2320::Hotspot12::doAction(int action) {
	Scene2320 *scene = (Scene2320 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2320, 5);
		break;
	case CURSOR_USE:
		if (g_globals->getFlag(13))
			SceneItem::display2(2320, 24);
		else {
			g_globals->_player.disableControl();
			scene->_sceneMode = 2322;
			scene->setAction(&scene->_sequenceManager1, scene, 2322, &g_globals->_player, NULL);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2320::Hotspot14::doAction(int action) {
	// Right Console
	Scene2320 *scene = (Scene2320 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2320, 17);
		break;
	case CURSOR_USE:
		if (g_globals->getFlag(24)) {
			g_globals->clearFlag(24);
			g_globals->_player.disableControl();

			scene->_hotspot8.postInit();
			scene->_hotspot8.setVisage(2345);
			scene->_hotspot8.setPosition(Common::Point(634, 65));
			scene->_hotspot8.hide();

			g_globals->_sceneItems.push_front(&scene->_hotspot8);
			RING_INVENTORY._waldos._sceneNumber = 2320;

			scene->_hotspot9.postInit();
			scene->_hotspot9.setVisage(2345);
			scene->_hotspot9._strip = 6;
			scene->_hotspot9.setPosition(Common::Point(536, 103));
			scene->_hotspot9.fixPriority(200);
			scene->_hotspot9.hide();

			scene->_hotspot16.postInit();
			scene->_hotspot16.setVisage(2345);
			scene->_hotspot16.setStrip(8);
			scene->_hotspot16.setPosition(Common::Point(536, 103));
			scene->_hotspot16.hide();

			scene->_sceneMode = 2324;
			scene->setAction(&scene->_sequenceManager1, scene, 2324, &g_globals->_player, &scene->_hotspot7,
				&scene->_hotspot8, &scene->_hotspot9, &scene->_hotspot16, NULL);
		} else if (g_globals->getFlag(13)) {
			SceneItem::display2(2320, 24);
		} else if (!g_globals->getFlag(76)) {
			SceneItem::display2(2320, 28);
		} else if (!RING_INVENTORY._waldos._sceneNumber) {
			SceneItem::display2(2320, 27);
		} else {
			SceneItem::display2(2320, 29);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2320::Hotspot15::doAction(int action) {
	// Left console (Flycycle console)
	Scene2320 *scene = (Scene2320 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2320, 16);
		break;
	case CURSOR_USE:
		if (g_globals->getFlag(13))
			SceneItem::display2(2320, 24);
		else
			scene->setAction(&scene->_action4);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}


/*--------------------------------------------------------------------------*/

Scene2320::Scene2320() :
		_hotspot1(0, CURSOR_LOOK, 2320, 0, LIST_END),
		_hotspot2(0, CURSOR_LOOK, 2320, 1, LIST_END),
		_hotspot3(0, CURSOR_LOOK, 2320, 11, LIST_END),
		_hotspot4(0, CURSOR_LOOK, 2320, 14, LIST_END),
		_hotspot13(0, CURSOR_LOOK, 2320, 12, LIST_END)
{
	_area1.setup(2153, 2, 1, 2100);
	_area1._pt = Common::Point(200, 31);
	_area2.setup(2153, 3, 1, 2150);
	_area2._pt = Common::Point(200, 50);
	_area3.setup(2153, 4, 1, 2320);
	_area3._pt = Common::Point(200, 75);
	_area4.setup(2153, 1, 1, 10);
	_area4._pt = Common::Point(237, 77);
}

void Scene2320::postInit(SceneObjectList *OwnerList) {
	Scene::postInit();
	setZoomPercents(60, 75, 100, 100);

	_stripManager.addSpeaker(&_speakerMR);
	_stripManager.addSpeaker(&_speakerML);
	_stripManager.addSpeaker(&_speakerMText);
	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerQL);
	_stripManager.addSpeaker(&_speakerQR);
	_stripManager.addSpeaker(&_speakerSR);
	_stripManager.addSpeaker(&_speakerSL);
	_stripManager.addSpeaker(&_speakerSAL);
	_stripManager.addSpeaker(&_speakerSText);
	_stripManager.addSpeaker(&_speakerGameText);

	_speakerMText._npc = &_hotspot11;
	_speakerQText._npc = &g_globals->_player;
	_hotspotPtr = &_hotspot10;

	_hotspot6.postInit();
	_hotspot6.setVisage(2321);
	_hotspot6.setPosition(Common::Point(320, 67));

	_hotspot7.postInit();
	_hotspot7.setVisage(2344);
	_hotspot7.setPosition(Common::Point(604, 92));

	_hotspot13.postInit();
	_hotspot13.setVisage(2323);
	_hotspot13.setPosition(Common::Point(319, 157));
	_hotspot13.fixPriority(1);

	_hotspot12.postInit();
	_hotspot12.setVisage(2321);
	_hotspot12._strip = 4;
	_hotspot12.animate(ANIM_MODE_8, 0, NULL);
	_hotspot12.setPosition(Common::Point(412, 46));

	if (RING_INVENTORY._waldos._sceneNumber == 2320) {
		_hotspot8.postInit();
		_hotspot8.setVisage(2345);
		_hotspot8.setStrip(5);
		_hotspot8.setFrame(8);
		_hotspot8.setPosition(Common::Point(541, 103));
		_hotspot8.fixPriority(201);

		g_globals->_sceneItems.push_back(&_hotspot8);
	}

	if (g_globals->getFlag(43)) {
		_hotspot11.postInit();
		_hotspot11.setVisage(2705);
		_hotspot11._strip = 3;
		_hotspot11.setPosition(Common::Point(510, 156));
		g_globals->_sceneItems.push_back(&_hotspot11);
		_hotspot11._state = 0;
		_hotspot11.setAction(&_action1);
	}

	g_globals->_player.postInit();
	if (g_globals->getFlag(13))
		g_globals->_player.setVisage(2170);
	else
		g_globals->_player.setVisage(0);
	g_globals->_player.animate(ANIM_MODE_1, NULL);
	g_globals->_player.setObjectWrapper(new SceneObjectWrapper());
	g_globals->_player.setPosition(Common::Point(320, 79));
	g_globals->_player.fixPriority(10);
	g_globals->_player.changeZoom(-1);
	g_globals->_player._moveDiff.y = 3;
	g_globals->_player.disableControl();

	if (g_globals->getFlag(114)) {
		_hotspot10.postInit();
		_hotspot10.setVisage(2806);
		_hotspot10.setPosition(Common::Point(481, 162));
		_hotspot10.changeZoom(-1);
		_hotspot10.setStrip(5);
		_hotspot10._state = 0;

		g_globals->_sceneItems.push_back(&_hotspot10);
	}

	if (g_globals->getFlag(70)) {
		_hotspot11.postInit();
		_hotspot11.setVisage(2705);
		_hotspot11.setPosition(Common::Point(500, 164));

		_hotspot10.postInit();
		_hotspot10.setVisage(2806);
		_hotspot10.setPosition(Common::Point(481, 162));
		_hotspot10.changeZoom(-1);
		_hotspot10.setStrip(5);

		g_globals->_sceneItems.addItems(&_hotspot11, &_hotspot10, NULL);
	}

	switch (g_globals->_sceneManager._previousScene) {
	case 2120:
		g_globals->_soundHandler.play(160);
		g_globals->_soundHandler.holdAt(true);

		g_globals->_player.fixPriority(-1);
		g_globals->_player.setPosition(Common::Point(389, 72));
		g_globals->_player.enableControl();
		break;
	case 4000:
		if (!g_globals->getFlag(36) && !g_globals->getFlag(43)) {
			_hotspot11.postInit();
			_hotspot11.setVisage(2705);
			_hotspot11.setPosition(Common::Point(178, 118));
			_hotspot11.animate(ANIM_MODE_1, NULL);
		}
		// Deliberate fall-through
	case 4250:
	case 5000:
	case 7000:
		g_globals->_soundHandler.play(160);
		g_globals->_soundHandler.holdAt(true);
		if ((g_globals->_sceneManager._previousScene == 7000) && !g_globals->getFlag(80))
			g_globals->setFlag(36);

		g_globals->_player.disableControl();
		g_globals->_player.animate(ANIM_MODE_NONE, NULL);
		g_globals->_player.setObjectWrapper(NULL);
		g_globals->_player.setVisage(2347);
		g_globals->_player.setStrip(2);
		g_globals->_player.setFrame(5);
		g_globals->_player.fixPriority(137);
		g_globals->_player.setPosition(Common::Point(165, 132));

		setAction(&_action6);
		break;
	case 6100:
		g_globals->_soundHandler.play(160);
		g_globals->_soundHandler.holdAt(true);

		_hotspot8.postInit();
		_hotspot8.setVisage(2345);
		_hotspot8.setPosition(Common::Point(634, 65));
		_hotspot8.hide();

		_sceneMode = 2325;
		setAction(&_sequenceManager1, this, 2325, &g_globals->_player, &_hotspot6, &_hotspot8, &_hotspot7, NULL);
		break;
	case 7600:
		g_globals->_soundHandler.play(160);
		g_globals->_soundHandler.holdAt(true);
		_soundHandler.play(21);

		g_globals->_player.setVisage(2323);
		g_globals->_player.setStrip(2);
		g_globals->_player.setFrame(g_globals->_player.getFrameCount());
		g_globals->_player.setPosition(Common::Point(303, 176));
		g_globals->_player.fixPriority(-1);
		g_globals->_player.disableControl();

		_hotspot13.setPosition(Common::Point(319, 199));

		_hotspot16.postInit();
		_hotspot16.setVisage(2331);
		_hotspot16._strip = 3;
		_hotspot16.fixPriority(160);
		_hotspot16.setPosition(Common::Point(320, 202));
		_hotspot16.hide();

		_sceneMode = 2338;
		setAction(&_sequenceManager1, this, 2338, &g_globals->_player, &_hotspot16, &_hotspot13, NULL);
		break;
	default:
		switch (g_globals->_stripNum) {
		case 2101:
			g_globals->_player.disableControl();
			g_globals->_player.setStrip(3);
			g_globals->_player.setPosition(Common::Point(307, 84));

			_hotspot11.postInit();
			_hotspot11.setVisage(2705);
			_hotspot11._strip = 2;
			_hotspot11.fixPriority(10);
			_hotspot11.setPosition(Common::Point(322, 80));
			_hotspot11.setObjectWrapper(new SceneObjectWrapper());
			_hotspot11.animate(ANIM_MODE_1, NULL);

			_hotspot10.postInit();
			_hotspot10.setVisage(2806);
			_hotspot10.setObjectWrapper(new SceneObjectWrapper());
			_hotspot10.changeZoom(-1);
			_hotspot10.fixPriority(10);
			_hotspot10.setPosition(Common::Point(318, 89));
			_hotspot10._strip = 3;
			_hotspot10.animate(ANIM_MODE_1, NULL);

			setAction(&_action7);
			break;
		case 6100:
			_hotspot8.postInit();
			_hotspot8.setVisage(2345);
			_hotspot8.setPosition(Common::Point(634, 65));
			_hotspot8.hide();

			_sceneMode = 2325;
			setAction(&_sequenceManager1, this, 2325, &g_globals->_player, &_hotspot6, &_hotspot8, &_hotspot7, NULL);
			break;
		default:
			g_globals->_soundHandler.play(160);
			g_globals->_soundHandler.holdAt(true);
			_sceneMode = 2321;

			g_globals->_player.setStrip(3);
			setAction(&_sequenceManager1, this, 2321, &g_globals->_player, &_hotspot6, NULL);
			break;
		}
	}

	g_globals->_stripNum = 0;
	g_globals->_sceneManager._scene->_sceneBounds.center(g_globals->_player._position);
	loadScene(2320);

	_hotspot14._sceneRegionId = 8;
	_hotspot1.setBounds(Rect(0, 0, 640, 200));
	_hotspot2.setBounds(Rect(278, 0, 362, 61));
	_hotspot3.setBounds(Rect(282, 62, 367, 98));
	_hotspot4.setBounds(Rect(67, 38, 112, 113));
	_hotspot5.setBounds(Rect(104, 122, 174, 157));
	_hotspot15.setBounds(Rect(191, 53, 205, 63));

	g_globals->_sceneItems.addItems(&_hotspot14, &_hotspot15, &_hotspot5, &_hotspot6, &_hotspot12,
		&_hotspot13, &_hotspot4, &_hotspot3, &_hotspot2, &_hotspot1, NULL);
}

void Scene2320::synchronize(Serializer &s) {
	Scene::synchronize(s);
	SYNC_POINTER(_hotspotPtr);
}

void Scene2320::signal() {
	switch (_sceneMode) {
	case 2321:
	case 2327:
	case 2329:
		if (g_globals->getFlag(43) && !_hotspot11._action)
			_hotspot11.setAction(&_action1);
		g_globals->_player.enableControl();
		break;
	case 2322:
		g_globals->_sceneManager.changeScene(2120);
		break;
	case 2323:
		g_globals->_player.disableControl();
		break;
	case 2338:
	case 2324:
		_hotspot16.remove();
		g_globals->_player.enableControl();
		break;
	case 2325:
		g_globals->setFlag(76);
		g_globals->clearFlag(70);
		g_globals->_stripNum = 6100;
		g_globals->_sceneManager.changeScene(2100);
		break;
	case 2326:
		g_globals->clearFlag(70);
		RING_INVENTORY._nullifier._sceneNumber = 1;
		g_globals->_stripNum = 2321;
		g_globals->_sceneManager.changeScene(2100);
		break;
	case 2336:
		g_globals->setFlag(77);
		RING_INVENTORY._waldos._sceneNumber = 1;
		_hotspot8.remove();
		break;
	case 5000:
		g_globals->_player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 2400 - Descending in Lander
 *
 *--------------------------------------------------------------------------*/

void Scene2400::Action1::signal() {
	switch (_actionIndex++) {
	case 0:
		setDelay(60);
		break;
	case 1:
		ADD_MOVER(g_globals->_player, 160, 71);
		break;
	case 2:
		ADD_MOVER(g_globals->_player, 160, 360);
		break;
	case 3:
		g_globals->_player._moveDiff = Common::Point(1, 1);
		ADD_MOVER(g_globals->_player, 140, 375);
		break;
	case 4:
		ADD_MOVER(g_globals->_player, 87, 338);
		break;
	case 5:
		g_globals->_player.hide();
		setDelay(60);
		break;
	case 6:
		g_globals->_sceneManager.changeScene(4000);
		break;
	}
}

void Scene2400::Action1::dispatch() {
	Action::dispatch();
	if ((_actionIndex == 4) && (g_globals->_player._percent > 5))
		g_globals->_player.changeZoom(g_globals->_player._percent - 2);
}

/*--------------------------------------------------------------------------*/

void Scene2400::postInit(SceneObjectList *OwnerList) {
	loadScene(2400);
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);

	g_globals->_player.postInit();
	g_globals->_player.setVisage(2410);
	g_globals->_player.setPosition(Common::Point(340, -10));
	g_globals->_player.animate(ANIM_MODE_2, NULL);
	g_globals->_player.disableControl();

	setAction(&_action1);

	g_globals->_sceneManager._scene->_sceneBounds.center(g_globals->_player._position.x, g_globals->_player._position.y);
	g_globals->_sceneManager._scene->_sceneBounds.contain(g_globals->_sceneManager._scene->_backgroundBounds);
	g_globals->_sceneOffset.x = (g_globals->_sceneManager._scene->_sceneBounds.left / 160) * 160;

	g_globals->_soundHandler.play(153);
}

} // End of namespace Ringworld

} // End of namespace TsAGE

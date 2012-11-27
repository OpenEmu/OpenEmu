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

#include "tsage/ringworld/ringworld_scenes6.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace TsAGE {

namespace Ringworld {

/*--------------------------------------------------------------------------
 * Scene 5000 - Caverns - Entrance
 *
 *--------------------------------------------------------------------------*/

void Scene5000::Action1::signal() {
	// Ship landing
	Scene5000 *scene = (Scene5000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(10);
		break;
	case 1:
		scene->_soundHandler.play(190);
		scene->_soundHandler.holdAt(true);
		ADD_MOVER(scene->_hotspot1, 283, 12);
		break;
	case 2:
		break;
	case 3:
		scene->_hotspot1._moveDiff.y = 1;
		scene->_hotspot4.setPosition(Common::Point(scene->_hotspot1._position.x,
			scene->_hotspot1._position.y + 15));
		scene->_hotspot4.show();
		setDelay(15);
		break;
	case 4:
		scene->_soundHandler.release();
		ADD_MOVER(scene->_hotspot1, 233, 80);
		break;
	case 5:
		scene->_hotspot3.animate(ANIM_MODE_8, 0, NULL);
		scene->_hotspot3.show();
		ADD_MOVER(scene->_hotspot1, 233, 90);
		break;
	case 6:
		scene->_hotspot3.remove();
		scene->_hotspot2.setPosition(Common::Point(233, 76));
		scene->_hotspot2.show();
		scene->_hotspot2.animate(ANIM_MODE_5, this);

		scene->_hotspot4.remove();
		break;
	case 7:
		setDelay(60);
		scene->_hotspot2.remove();
		break;
	case 8:
		scene->_hotspot5.show();
		scene->_hotspot5.animate(ANIM_MODE_5, this);
		break;
	case 9:
		scene->setAction(&scene->_action2);
		break;
	}
}

void Scene5000::Action1::dispatch() {
	Scene5000 *scene = (Scene5000 *)g_globals->_sceneManager._scene;
	Action::dispatch();

	if (_actionIndex == 3) {
		if (scene->_hotspot1._percent % 2 == 0) {
			++scene->_hotspot1._position.y;
			if (scene->_hotspot1._position.x > 233)
				--scene->_hotspot1._position.x;
		}

		scene->_hotspot1.changeZoom(++scene->_hotspot1._percent);
		scene->_hotspot1._flags |= OBJFLAG_PANES;

		if (scene->_hotspot1._percent >= 100)
			signal();
	}

	if ((_actionIndex == 5) || (_actionIndex == 6)) {
		scene->_hotspot4.setPosition(Common::Point(scene->_hotspot1._position.x,
			scene->_hotspot1._position.y + 15));
	}
}

void Scene5000::Action2::signal() {
	Scene5000 *scene = (Scene5000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.setPosition(Common::Point(217, 76));
		setDelay(10);
		break;
	case 1:
		g_globals->_player.setStrip2(3);
		g_globals->_player.animate(ANIM_MODE_1, NULL);
		ADD_MOVER(g_globals->_player, 214, 89);
		break;
	case 2:
		if (!g_globals->getFlag(59))
			setAction(&scene->_action3, this);
		g_globals->_player.fixPriority(15);
		ADD_MOVER(g_globals->_player, 208, 100);
		break;
	case 3:
		ADD_MOVER(g_globals->_player, 213, 98);
		break;
	case 4:
		g_globals->_player.fixPriority(20);
		ADD_MOVER(g_globals->_player, 215, 115);
		break;
	case 5:
		g_globals->_player.changeZoom(47);
		ADD_MOVER(g_globals->_player, 220, 125);
		break;
	case 6:
		ADD_MOVER(g_globals->_player, 229, 115);
		break;
	case 7:
		g_globals->_player.changeZoom(-1);
		g_globals->_player.fixPriority(35);
		ADD_MOVER(g_globals->_player, 201, 166);
		break;
	case 8:
		g_globals->_player.updateZoom();
		ADD_MOVER(g_globals->_player, 205, 146);
		break;
	case 9:
		g_globals->_player.changeZoom(-1);
		g_globals->_player.fixPriority(50);
		ADD_MOVER(g_globals->_player, 220, 182);
		break;
	case 10:
		g_globals->_player.updateZoom();
		ADD_MOVER(g_globals->_player, 208, 163);
		break;
	case 11:
		g_globals->_player.changeZoom(-1);
		g_globals->_player.setStrip2(-1);
		g_globals->_player.fixPriority(-1);
		ADD_MOVER(g_globals->_player, 208, 175);
		break;
	case 12:
		g_globals->_player.setStrip(8);
		g_globals->_player.setFrame(1);
		setDelay(10);
		break;
	case 13:
		if (!g_globals->_sceneObjects->contains(&scene->_hotspot7))
			setDelay(10);
		break;
	case 14:
		setDelay(30);
		break;
	case 15:
		g_globals->_player.enableControl();
		remove();
		break;
	}
}

void Scene5000::Action3::signal() {
	Scene5000 *scene = (Scene5000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(10);
		break;
	case 1:
		scene->_hotspot7.postInit();
		scene->_hotspot7.setVisage(2809);
		scene->_hotspot7.setObjectWrapper(new SceneObjectWrapper());
		scene->_hotspot7.setPosition(Common::Point(217, 76));
		scene->_hotspot7.changeZoom(10);
		scene->_hotspot7.setStrip2(3);
		scene->_hotspot7.fixPriority(200);
		scene->_hotspot7._moveDiff.y = 2;
		scene->_hotspot7.animate(ANIM_MODE_1, NULL);
		ADD_MOVER(scene->_hotspot7, 214, 89);

		g_globals->_sceneItems.push_front(&scene->_hotspot7);
		break;
	case 2:
		scene->_hotspot7.changeZoom(-1);
		scene->_hotspot7.fixPriority(14);
		ADD_MOVER(scene->_hotspot7, 208, 100);
		break;
	case 3:
		ADD_MOVER(scene->_hotspot7, 213, 98);
		break;
	case 4:
		scene->_hotspot7.fixPriority(19);
		ADD_MOVER(scene->_hotspot7, 215, 115);
		break;
	case 5:
		scene->_hotspot7.changeZoom(46);
		ADD_MOVER(scene->_hotspot7, 220, 125);
		break;
	case 6:
		ADD_MOVER(scene->_hotspot7, 229, 115);
		break;
	case 7:
		scene->_hotspot7.changeZoom(-1);
		scene->_hotspot7.fixPriority(34);
		ADD_MOVER(scene->_hotspot7, 201, 166);
		break;
	case 8:
		scene->_hotspot7.updateZoom();
		ADD_MOVER(scene->_hotspot7, 205, 146);
		break;
	case 9:
		scene->_hotspot7.changeZoom(-1);
		scene->_hotspot7.fixPriority(49);
		ADD_MOVER(scene->_hotspot7, 210, 182);
		break;
	case 10:
		scene->_hotspot7.updateZoom();
		ADD_MOVER(scene->_hotspot7, 208, 163);
		break;
	case 11:
		scene->_hotspot7.changeZoom(-1);
		scene->_hotspot7.setStrip2(-1);
		scene->_hotspot7.fixPriority(-1);
		ADD_MOVER(scene->_hotspot7, 175, 166);
		break;
	case 12:
		ADD_MOVER(scene->_hotspot7, 126, 146);
		break;
	case 13:
		scene->_hotspot7.setStrip(2);
		remove();
		break;
	}
}

void Scene5000::Action4::signal() {
	Scene5000 *scene = (Scene5000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(1);
		break;
	case 1:
		g_globals->_player.fixPriority(50);
		g_globals->_player.setStrip2(4);
		ADD_MOVER(g_globals->_player, 210, 182);
		break;
	case 2:
		ADD_MOVER(g_globals->_player, 205, 146);
		break;
	case 3:
		g_globals->_player.fixPriority(35);
		ADD_MOVER(g_globals->_player, 201, 166);
		break;
	case 4:
		ADD_MOVER(g_globals->_player, 229, 115);
		break;
	case 5:
		g_globals->_player.fixPriority(20);
		g_globals->_player.changeZoom(47);
		ADD_MOVER(g_globals->_player, 220, 125);
		break;
	case 6:
		ADD_MOVER(g_globals->_player, 215, 115);
		break;
	case 7:
		g_globals->_player.changeZoom(-1);
		ADD_MOVER(g_globals->_player, 213, 98);
		break;
	case 8:
		g_globals->_player.fixPriority(15);
		ADD_MOVER(g_globals->_player, 208, 100);
		break;
	case 9:
		ADD_MOVER(g_globals->_player, 214, 89);
		break;
	case 10:
		ADD_MOVER(g_globals->_player, 217, 76);
		break;
	case 11:
		g_globals->_player.hide();
		setDelay(60);
		break;
	case 12:
		if (!g_globals->_sceneObjects->contains(&scene->_hotspot7))
			g_globals->_sceneManager.changeScene(2320);
		remove();
		break;
	}
}

void Scene5000::Action5::signal() {
	Scene5000 *scene = (Scene5000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		ADD_MOVER(g_globals->_player, 91, 155);
		break;
	case 1:
		g_globals->_player.setVisage(2670);
		g_globals->_player._strip = 4;
		g_globals->_player._frame = 1;
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 2:
		SceneItem::display2(5000, g_globals->_sceneObjects->contains(&scene->_hotspot7) ? 17 : 13);
		g_globals->_player.animate(ANIM_MODE_6, this);
		break;
	case 3:
		g_globals->_player.setVisage(0);
		g_globals->_player._strip = 8;
		g_globals->_player.animate(ANIM_MODE_1, NULL);

		g_globals->_player.enableControl();
		remove();
	}
}

void Scene5000::Action6::signal() {
	// Discussion between the hero and Seeker, then the hero goes back to the lander
	Scene5000 *scene = (Scene5000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		setDelay(3);
		break;
	case 1:
		g_globals->_events.setCursor(CURSOR_WALK);
		scene->_stripManager.start(2150, this);
		break;
	case 2:
		g_globals->_events.setCursor(CURSOR_NONE);
		scene->setAction(&scene->_sequenceManager, this, 5001, &scene->_hotspot7, NULL);
		break;
	case 3:
		ADD_PLAYER_MOVER(208, 163);
		break;
	case 4:
		g_globals->_player.fixPriority(50);
		g_globals->_player.setStrip2(4);
		ADD_MOVER(g_globals->_player, 210, 182);
		break;
	case 5:
		g_globals->_sceneManager.changeScene(2100);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene5000::Hotspot7::doAction(int action) {
	Scene5000 *scene = (Scene5000 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(5000, 12);
		break;
	case CURSOR_TALK:
		scene->setAction(&scene->_action6);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene5000::Hotspot8::doAction(int action) {
	// Cave
	Scene5000 *scene = (Scene5000 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(5000, 10);
		break;
	case OBJECT_STUNNER:
		SceneItem::display2(5000, 14);
		break;
	case OBJECT_SCANNER:
		scene->setAction(&scene->_action5);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene5000::HotspotGroup1::doAction(int action) {
	Scene5000 *scene = (Scene5000 *)g_globals->_sceneManager._scene;

	if (g_globals->_sceneObjects->contains(&scene->_hotspot7))
		scene->setAction(&scene->_action6);
	else
		SceneItem::display2(5000, 11);
}
/*--------------------------------------------------------------------------*/

Scene5000::Scene5000() :
		_hotspot1(0, CURSOR_LOOK, 5000, 3, LIST_END),
		_hotspot12(0, CURSOR_LOOK, 5000, 6, CURSOR_USE, 5000, 7, OBJECT_STUNNER, 5000, 14,
				OBJECT_SCANNER, 5000, 16, LIST_END),
		_hotspot13(0, CURSOR_LOOK, 5000, 8, CURSOR_USE, 5000, 9, OBJECT_STUNNER, 5000, 0,
				OBJECT_SCANNER, 5000, 16, LIST_END),
		_hotspot14(0, CURSOR_LOOK, 5000, 8, CURSOR_USE, 5000, 9, OBJECT_STUNNER, 5000, 0,
				OBJECT_SCANNER, 5000, 16, LIST_END),
		_hotspot15(0, CURSOR_LOOK, 5000, 2, OBJECT_STUNNER, 5000, 15, LIST_END),
		_hotspot16(0, CURSOR_LOOK, 5000, 4, CURSOR_USE, 5000, 5, LIST_END),
		_hotspot17(0, CURSOR_LOOK, 5000, 1, LIST_END),
		_hotspot18(0, CURSOR_LOOK, 5000, 0, LIST_END) {
}


void Scene5000::postInit(SceneObjectList *OwnerList) {
	Scene::postInit();
	loadScene(5000);

	_stripManager.addSpeaker(&_speakerSText);
	_stripManager.addSpeaker(&_speakerQText);

	g_globals->_player.postInit();
	g_globals->_player.setObjectWrapper(new SceneObjectWrapper());
	g_globals->_player.setVisage(0);
	g_globals->_player.animate(ANIM_MODE_1, NULL);
	g_globals->_player._moveDiff = Common::Point(4, 2);
	g_globals->_player.changeZoom(-1);
	g_globals->_player.disableControl();

	_hotspot1.postInit();
	_hotspot1.setVisage(5001);
	_hotspot1.setFrame2(1);
	_hotspot1._moveDiff = Common::Point(5, 5);
	_hotspot1.fixPriority(10);
	_hotspot1.changeZoom(10);

	_hotspot4.postInit();
	_hotspot4.setVisage(5001);
	_hotspot4.setStrip2(2);
	_hotspot4._moveDiff = Common::Point(5, 1);
	_hotspot4.fixPriority(10);
	_hotspot4.changeZoom(100);
	_hotspot4.animate(ANIM_MODE_8, 0, NULL);
	_hotspot4.hide();

	_hotspot2.postInit();
	_hotspot2.setVisage(5001);
	_hotspot2.setStrip2(3);
	_hotspot2._numFrames = 5;
	_hotspot2.hide();

	_hotspot3.postInit();
	_hotspot3.setVisage(5001);
	_hotspot3.setStrip2(5);
	_hotspot3._numFrames = 5;
	_hotspot3.setPosition(Common::Point(233, 76));
	_hotspot3.hide();

	_hotspot5.postInit();
	_hotspot5.setVisage(5001);
	_hotspot5.setStrip2(4);
	_hotspot5._numFrames = 5;
	_hotspot5.fixPriority(15);
	_hotspot5.setPosition(Common::Point(218, 76));
	_hotspot5.hide();

	_hotspot9.postInit();
	_hotspot9.setVisage(5002);
	_hotspot9.fixPriority(80);
	_hotspot9.setPosition(Common::Point(71, 174));

	_hotspot10.postInit();
	_hotspot10.setVisage(5002);
	_hotspot10.setStrip2(2);
	_hotspot10.setPosition(Common::Point(87, 120));

	_hotspot11.postInit();
	_hotspot11.setVisage(5002);
	_hotspot11.setStrip2(2);
	_hotspot11.setFrame(3);
	_hotspot10.setPosition(Common::Point(93, 118));

	setZoomPercents(95, 10, 145, 100);

	_hotspot8.setBounds(Rect(0, 73, 87, 144));
	_hotspot18.setBounds(Rect(54, 0, 319, 85));
	_hotspot17.setBounds(Rect(184, 0, 199, 79));
	_hotspot13.setBounds(Rect(0, 164, 135, 200));
	_hotspot14.setBounds(Rect(0, 0, 105, 140));
	_hotspot15.setBounds(Rect(266, 70, 291, 85));
	_hotspot16.setBounds(Rect(0, 86, 319, 200));
	_hotspot12.setBounds(Rect(230, 143, 244, 150));

	g_globals->_sceneItems.addItems(&_hotspot9, &_hotspot10, &_hotspot11, &_hotspot8, &_hotspot13,
		&_hotspot14, &_hotspot12, &_hotspot15, &_hotspot17, &_hotspot18, &_hotspot16, NULL);

	switch (g_globals->_sceneManager._previousScene) {
	case 1000:
	case 2100:
	case 2320:
		if (g_globals->getFlag(59)) {
			_hotspot1.setPosition(Common::Point(233, 90));
			_hotspot1.changeZoom(100);
			_hotspot1.show();

			_hotspot5.setFrame(1);
			_hotspot5.animate(ANIM_MODE_5, NULL);
			_hotspot5.setPosition(Common::Point(218, 76));
			_hotspot5.show();

			g_globals->_player.setPosition(Common::Point(217, -10));
			g_globals->_player.disableControl();

			setAction(&_action2);
		} else {
			g_globals->_player.setPosition(Common::Point(217, -10));
			_hotspot1.setPosition(Common::Point(320, -10));
			g_globals->_player.disableControl();

			setAction(&_action1);
		}
		break;
	default:
		g_globals->_player.disableControl();
		g_globals->_player.setPosition(Common::Point(0, 146));

		_hotspot1.changeZoom(100);
		_hotspot1.setPosition(Common::Point(233, 90));
		_hotspot1.show();

		_hotspot5.setFrame(_hotspot5.getFrameCount());
		_hotspot5.show();

		_sceneMode = 5004;
		setAction(&_sequenceManager, this, 5004, &g_globals->_player, NULL);
		break;
	}

	g_globals->_soundHandler.play(190);
}

void Scene5000::signal() {
	switch (_sceneMode) {
	case 5002:
	case 5003:
	case 5004:
		g_globals->_player.enableControl();
		break;
	case 5005:
		g_globals->_sceneManager.changeScene(5100);
		break;
	}
}

void Scene5000::dispatch() {
	Scene::dispatch();

	if (!_action) {
		if (!g_globals->_sceneObjects->contains(&_hotspot7) && (g_globals->_player.getRegionIndex() == 10)) {
			g_globals->_player.disableControl();
			_sceneMode = 5005;
			setAction(&_sequenceManager, this, 5005, &g_globals->_player, NULL);
		}

		if (g_globals->_player.getRegionIndex() == 8) {
			g_globals->_player.disableControl();

			if (g_globals->_sceneObjects->contains(&_hotspot7)) {
				_sceneMode = 5003;
				g_globals->_player.addMover(NULL);
				setAction(&_sequenceManager, this, 5003, &g_globals->_player, NULL);
			} else {
				setAction(&_action4);
			}
		}

		if (g_globals->_sceneObjects->contains(&_hotspot7) && (g_globals->_player.getRegionIndex() == 15)) {
			_sceneMode = 5002;
			g_globals->_player.disableControl();
			g_globals->_player.addMover(NULL);
			setAction(&_sequenceManager, this, 5002, &g_globals->_player, NULL);
		}
	}
}

/*--------------------------------------------------------------------------
 * Scene 5100 - Caverns
 *
 *--------------------------------------------------------------------------*/

void Scene5100::Action1::signal() {
	// Quinn enters the cave for the first time
	Scene5100 *scene = (Scene5100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(5);
		break;
	case 1:
		ADD_MOVER(g_globals->_player, 1111, 165);
		break;
	case 2:
		g_globals->_player.setStrip(6);
		g_globals->_player.setFrame(1);
		setDelay(60);
		break;
	case 3:
		if (g_globals->getFlag(10)) {
			g_globals->_player.enableControl();
			remove();
		} else {
			g_globals->setFlag(10);
			scene->_stripManager.start(5102, this);
		}
		break;
	case 4:
		scene->_soundHandler.play(206);

		scene->_hotspot5.postInit();
		scene->_hotspot5.setVisage(5362);
		scene->_hotspot5.setPosition(Common::Point(1160, 34));
		scene->_hotspot5.setStrip2(2);
		scene->_hotspot5.animate(ANIM_MODE_1, NULL);
		scene->_hotspot5.fixPriority(10);

		g_globals->_sceneItems.push_front(&scene->_hotspot5);
		ADD_MOVER(scene->_hotspot5, 999, 14);
		break;
	case 5:
		scene->_hotspot5.setStrip2(4);
		scene->_hotspot5._frame = 1;
		scene->_hotspot5.animate(ANIM_MODE_5, this);
		break;
	case 6:
		g_globals->_player.enableControl();
		remove();
		break;
	}
}

void Scene5100::Action2::signal() {
	// Quinn and Seeker exit the cave
	Scene5100 *scene = (Scene5100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		setDelay(5);
		break;
	case 1:
		if (scene->_hotspot8._visage == 2806)
			setDelay(1);
		else
			scene->_hotspot8.animate(ANIM_MODE_5, this);
		break;
	case 2:
		if (scene->_hotspot8._visage != 2806) {
			scene->_hotspot8.setVisage(2806);
			scene->_hotspot8.setPosition(Common::Point(548, 193));
			scene->_hotspot8.animate(ANIM_MODE_1, NULL);
			scene->_hotspot8._angle = 325;
			scene->_hotspot8.setObjectWrapper(new SceneObjectWrapper());
			scene->_hotspot8.setStrip(8);
		}

		scene->_stripManager.start(5129, this);
		break;
	case 3:
		if (g_globals->_player._position.x >= 966) {
			ADD_PLAYER_MOVER_NULL(scene->_hotspot8, 1215, 155);
		} else {
			ADD_PLAYER_MOVER_NULL(scene->_hotspot8, 966, 185);
		}

		if (g_globals->_player._position.x >= 966) {
			setDelay(1);
		} else {
			ADD_PLAYER_MOVER(966, 190);
		}
		break;
	case 4:
		ADD_PLAYER_MOVER_NULL(scene->_hotspot8, 1215, 155);
		ADD_PLAYER_MOVER(1215, 155);
		break;
	case 5:
		g_globals->_sceneManager.changeScene(2100);
		break;
	}
}

void Scene5100::Action3::signal() {
	// Quinns shots flesheater
	Scene5100 *scene = (Scene5100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->setFlag(62);
		g_globals->_player.disableControl();
		g_globals->_player.addMover(NULL);

		scene->_hotspot2.addMover(NULL);
		scene->_hotspot3.addMover(NULL);
		g_globals->_player.setVisage(2672);

		if (static_cast<SceneObject *>(_owner)->_position.x >= g_globals->_player._position.x)
			g_globals->_player._strip = 4;
		else
			g_globals->_player._strip = 3;
		g_globals->_player._frame = 1;
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 1:
		g_globals->_player.setVisage(2674);
		g_globals->_player.setFrame(1);
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 2:
		scene->_soundHandler.play(28);
		if (static_cast<SceneObject *>(_owner)->_position.x < g_globals->_player._position.x) {
			scene->_hotspot2.setVisage(5130);
			scene->_hotspot2._strip = 1;
			scene->_hotspot2._frame = 1;
			scene->_hotspot2.animate(ANIM_MODE_5, this);
			scene->_hotspot2.setAction(NULL);

			scene->_hotspot3.setStrip2(1);
			ADD_PLAYER_MOVER_NULL(scene->_hotspot3, 1200, 100);
		} else {
			scene->_hotspot3.setVisage(5130);
			scene->_hotspot3._strip = 1;
			scene->_hotspot3._frame = 1;
			scene->_hotspot3.animate(ANIM_MODE_5, this);
			scene->_hotspot3.setAction(NULL);

			scene->_hotspot2.setStrip2(2);
			ADD_PLAYER_MOVER_NULL(scene->_hotspot2, 10, 140);

		}

		g_globals->_player.setVisage(2672);
		g_globals->_player._frame = g_globals->_player.getFrameCount();
		g_globals->_player.animate(ANIM_MODE_6, this);
		break;
	case 3:
		break;
	case 4:
		SceneItem::display2(5100, 38);
		g_globals->_player.enableControl();

		g_globals->_player.setVisage(0);
		g_globals->_player._strip = 8;
		g_globals->_player.animate(ANIM_MODE_1, NULL);
		remove();
		break;
	}
}

void Scene5100::Action4::signal() {
	Scene5100 *scene = (Scene5100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		g_globals->_player.disableControl();
		scene->_soundHandler.play(208);
		SceneItem::display2(5100, 15);

		ObjectMover3 *mover = new ObjectMover3();
		scene->_hotspot2.addMover(mover, &g_globals->_player, 20, this);
		break;
	}
	case 1:
		scene->_hotspot1.postInit();
		scene->_hotspot1.setVisage(5120);
		scene->_hotspot1.setPosition(Common::Point(795, 161));
		scene->_hotspot1._strip = 4;
		scene->_hotspot1.animate(ANIM_MODE_5, this);
		break;
	case 2:
		setDelay(15);
		break;
	case 3:
		g_globals->_sceneManager.changeScene(5200);
		break;
	}
}

void Scene5100::Action5::signal() {
	// Quinns forgot the statis box in the throne room, and goes back
	Scene5100 *scene = (Scene5100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		setDelay(60);
		break;
	case 1:
		scene->_stripManager.start(5128, this);
		break;
	case 2:
		setDelay(15);
		break;
	case 3:
		scene->_sceneMode = 5106;
		scene->setAction(&scene->_sequenceManager, scene, 5106, &g_globals->_player, &scene->_hotspot14, NULL);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene5100::HotspotGroup1::doAction(int action) {
	// Flesh Eaters
	Scene5100 *scene = (Scene5100 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(5100, g_globals->getFlag(62) ? 41 : 26);
		break;
	case CURSOR_USE:
		SceneItem::display2(5100, 11);
		break;
	case OBJECT_SCANNER:
		SceneItem::display2(5100, 17);
		break;
	case OBJECT_STUNNER:
		if (g_globals->getFlag(62))
			SceneItem::display2(5100, 42);
		else
			setAction(&scene->_action3);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene5100::Hotspot4::doAction(int action) {
	Scene5100 *scene = (Scene5100 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(5100, 31);
		break;
	case CURSOR_USE:
		g_globals->_player.disableControl();
		scene->_sceneMode = 5110;
		scene->setAction(&scene->_sequenceManager, scene, 5110, &g_globals->_player, this, &scene->_hotspot7, NULL);
		break;
	case CURSOR_TALK:
		SceneItem::display2(5100, 34);
		break;
	case OBJECT_STUNNER:
		SceneItem::display2(5100, 36);
		break;
	case OBJECT_SCANNER:
		SceneItem::display2(5100, 37);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene5100::HotspotGroup2::doAction(int action) {
	// Bat
	Scene5100 *scene = (Scene5100 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(5100, g_globals->getFlag(108) ? 47 : 23);
		break;
	case CURSOR_USE:
		SceneItem::display2(5100, 29);
		break;
	case CURSOR_TALK:
		if (_position.x >= 600)
			SceneItem::display2(5100, 28);
		else {
			g_globals->_player.disableControl();
			scene->_sceneMode = 5114;
			scene->setAction(&scene->_sequenceManager, scene, 5114, NULL);
		}
		break;
	case OBJECT_SCANNER:
		SceneItem::display2(5100, 43);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene5100::Hotspot9::doAction(int action) {
	// Rope
	Scene5100 *scene = (Scene5100 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(5100, 32);
		break;
	case CURSOR_USE:
		g_globals->_player.disableControl();
		scene->_sceneMode = 5117;
		scene->setAction(&scene->_sequenceManager, scene, 5117, &g_globals->_player, NULL);
		break;
	case OBJECT_STUNNER:
		SceneItem::display2(5100, 35);
		break;
	case OBJECT_SCANNER:
		SceneItem::display2(5100, 44);
		break;
	case OBJECT_BONE:
		g_globals->_player.disableControl();
		scene->_sceneMode = 5116;
		scene->setAction(&scene->_sequenceManager, scene, 5116, &g_globals->_player, this, &scene->_hotspot10,
				&scene->_hotspot4, NULL);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene5100::Hotspot17::doAction(int action) {
	// Rock blocking pit entrance
	Scene5100 *scene = (Scene5100 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(5100, 18);
		break;
	case CURSOR_USE:
		if (g_globals->getFlag(67))
			SceneItem::display2(5100, 19);
		else {
			g_globals->_player.disableControl();
			scene->_sceneMode = 5101;
			scene->setAction(&scene->_sequenceManager, scene, 5101, &g_globals->_player, this, NULL);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene5100::Hotspot18::doAction(int action) {
	switch (action) {
	case CURSOR_LOOK:
		if (g_globals->_sceneManager._previousScene != 5000) {
			SceneItem::display2(5100, 3);
		} else {
			switch (_index1) {
			case 0:
				SceneItem::display2(5100, 0);
				++_index1;
				break;
			case 1:
				SceneItem::display2(5100, 1);
				++_index1;
				break;
			default:
				SceneItem::display2(5100, 2);
				break;
			}
		}
		break;
	case CURSOR_USE:
		switch (_index2) {
		case 0:
			SceneItem::display2(5100, 8);
			++_index2;
			break;
		case 1:
			SceneItem::display2(5100, 9);
			++_index2;
			break;
		default:
			SceneItem::display2(5100, 10);
			break;
		}
		break;

	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene5100::Hotspot19::doAction(int action) {
	// Pillar
	Scene5100 *scene = (Scene5100 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(5100, g_globals->_sceneObjects->contains(&scene->_hotspot14) ? 27 : 20);
		break;
	case CURSOR_USE:
		g_globals->_player.disableControl();
		scene->_sceneMode = 5106;

		if (g_globals->getFlag(66))
			scene->setAction(&scene->_sequenceManager, scene, 5113, &g_globals->_player, NULL);
		else {
			g_globals->setFlag(66);
			scene->setAction(&scene->_sequenceManager, scene, 5106, &g_globals->_player, &scene->_hotspot14, NULL);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene5100::Hotspot20::doAction(int action) {
	Scene5100 *scene = (Scene5100 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(5100, 21);
		break;
	case CURSOR_USE:
		g_globals->_player.disableControl();

		if (g_globals->getFlag(67)) {
			scene->_sceneMode = 5112;
			scene->setAction(&scene->_sequenceManager, scene, 5112, &g_globals->_player, NULL);
		} else {
			scene->_sceneMode = 5101;
			scene->setAction(&scene->_sequenceManager, scene, 5106, &g_globals->_player, &scene->_hotspot17, NULL);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene5100::Scene5100() :
		_hotspot16(0, CURSOR_LOOK, 5100, 48, LIST_END),
		_hotspot21(0, CURSOR_LOOK, 5100, 4, CURSOR_USE, 5100, 5, LIST_END) {
}

void Scene5100::postInit(SceneObjectList *OwnerList) {
	Scene::postInit();
	setZoomPercents(50, 10, 200, 100);
	_sceneMode = 0;

	_stripManager.addSpeaker(&_speakerMText);
	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerSText);
	_stripManager.addSpeaker(&_speakerGameText);
	_stripManager.addSpeaker(&_speakerBatText);
	_speakerQText._npc = &g_globals->_player;
	_speakerMText._npc = &g_globals->_player;
	_speakerSText._npc = &_hotspot8;

	g_globals->_player.postInit();
	g_globals->_player.setVisage(0);
	g_globals->_player.setObjectWrapper(new SceneObjectWrapper());
	g_globals->_player.animate(ANIM_MODE_1, NULL);
	g_globals->_player.disableControl();

	if ((!g_globals->getFlag(66)) || (RING_INVENTORY._stasisBox._sceneNumber != 1)) {
		_hotspot14.postInit();
		_hotspot14.setVisage(5101);
		_hotspot14.setPosition(Common::Point(498, 147));
		_hotspot14.fixPriority(200);
		_hotspot14._moveDiff.y = 10;
	}

	_hotspot17.postInit();
	_hotspot17.setVisage(5101);
	_hotspot17._strip = 2;
	_hotspot17.fixPriority(200);

	if (g_globals->getFlag(67))
		_hotspot17.setPosition(Common::Point(554, 192));
	else
		_hotspot17.setPosition(Common::Point(539, 179));

	_hotspot19.setBounds(Rect(488, 115, 508, 148));
	_hotspot21.setBounds(Rect(1150, 85, 1173, 112));
	_hotspot20.setBounds(Rect(517, 193, 562, 200));
	g_globals->_sceneItems.addItems(&_hotspot19, &_hotspot21, NULL);

	if (g_globals->getFlag(67)) {
		g_globals->_sceneItems.addItems(&_hotspot20, &_hotspot17, NULL);
	} else {
		g_globals->_sceneItems.addItems(&_hotspot17, &_hotspot20, NULL);
	}

	if (!g_globals->getFlag(105)) {
		_hotspot4.postInit();
		_hotspot4.setVisage(5363);
		_hotspot4.setPosition(Common::Point(1025, 65));
		_hotspot4.setStrip(4);
		_hotspot4.animate(ANIM_MODE_7, 0, NULL);
		g_globals->_sceneItems.push_back(&_hotspot4);

		_hotspot9.postInit();
		_hotspot9.setVisage(5363);
		_hotspot9.setPosition(Common::Point(966, 120));
		g_globals->_sceneItems.push_back(&_hotspot9);

		_hotspot10.postInit();
		_hotspot10.setVisage(5363);
		_hotspot10.setPosition(Common::Point(999, 68));
		_hotspot10.setStrip(2);

		_hotspot6.postInit();
		_hotspot6.setVisage(5362);
		_hotspot6.setPosition(Common::Point(1017, 34));
		_hotspot6._strip = 4;
		_hotspot6._frame = _hotspot6.getFrameCount();
		g_globals->_sceneItems.push_back(&_hotspot6);
	}

	_hotspot16._sceneRegionId = 15;
	g_globals->_sceneItems.push_back(&_hotspot16);

	if (!g_globals->getFlag(62)) {
		_hotspot2.postInit();
		_hotspot2.setVisage(5110);
		_hotspot2.animate(ANIM_MODE_1, NULL);
		_hotspot2._moveDiff.x = 4;
		g_globals->_sceneItems.push_back(&_hotspot2);

		_hotspot3.postInit();
		_hotspot3.setVisage(5110);
		_hotspot3.animate(ANIM_MODE_1, NULL);
		_hotspot3._moveDiff.x = 4;
		g_globals->_sceneItems.push_back(&_hotspot3);

		if (g_globals->getFlag(61)) {
			_hotspot2.setPosition(Common::Point(483, 189));
			_hotspot3.setPosition(Common::Point(811, 182));
		} else {
			_hotspot2.setPosition(Common::Point(610, 170));
			_hotspot3.setPosition(Common::Point(600, 180));
		}
	}

	if (g_globals->getFlag(60) && (RING_INVENTORY._stasisBox._sceneNumber == 1) &&
			g_globals->getFlag(107) && g_globals->getFlag(67)) {
		_hotspot8.postInit();
		_hotspot8.setVisage(2806);
		_hotspot8.setPosition(Common::Point(557, 178));
		_hotspot8.animate(ANIM_MODE_1, NULL);
		_hotspot8.setObjectWrapper(new SceneObjectWrapper());

		g_globals->clearFlag(59);
	}

	switch (g_globals->_sceneManager._previousScene) {
	case 5000:
	default:
		g_globals->_player.setPosition(Common::Point(1184, 160));
		setAction(&_action1);
		break;
	case 5200:
		if (g_globals->_stripNum == 5200) {
			g_globals->_player.setVisage(5101);
			g_globals->_player.fixPriority(200);
			g_globals->_player.setStrip(5);
			g_globals->_player.setFrame(1);
			g_globals->_player.setPosition(Common::Point(513, 199));

			_sceneMode = 5108;
			setAction(&_sequenceManager, this, 5108, &g_globals->_player, NULL);
		} else {
			g_globals->_player.setPosition(Common::Point(20, 175));

			_hotspot13.postInit();
			_hotspot13.setVisage(5110);
			_hotspot13.setPosition(Common::Point(578, 192));
			_hotspot13._strip = 2;

			_hotspot11.postInit();
			_hotspot11.setVisage(5110);
			_hotspot11.setPosition(Common::Point(5, 162));
			_hotspot11.setStrip2(1);
			_hotspot11._moveDiff = Common::Point(4, 2);
			_hotspot11.animate(ANIM_MODE_1, NULL);

			ObjectMover2 *mover = new ObjectMover2();
			_hotspot11.addMover(mover, 15, 20, &g_globals->_player);

			_hotspot12.postInit();
			_hotspot12.setVisage(5110);
			_hotspot12.setPosition(Common::Point(15, 164));
			_hotspot12.setStrip2(1);
			_hotspot12._moveDiff = Common::Point(4, 2);
			_hotspot12.animate(ANIM_MODE_1, NULL);

			ObjectMover2 *mover2 = new ObjectMover2();
			_hotspot12.addMover(mover2, 25, 50, &g_globals->_player);

			_hotspot17.setPosition(Common::Point(554, 182));
			g_globals->_player.disableControl();

			_sceneMode = 5105;
			setAction(&_sequenceManager, this, 5105, &g_globals->_player, NULL);
		}
		break;
	case 5300:
		switch (g_globals->_stripNum) {
		case 5300:
			_sceneMode = 5111;

			g_globals->_player.setVisage(5101);
			g_globals->_player.setStrip(6);
			g_globals->_player.fixPriority(170);
			g_globals->_player.setPosition(Common::Point(1168, 110));

			setAction(&_sequenceManager, this, 5111, &g_globals->_player, NULL);
			_hotspot8.setPosition(Common::Point(1107, 178));
			break;
		case 5302:
			g_globals->_player.setPosition(Common::Point(512, 190));

			if (g_globals->_sceneObjects->contains(&_hotspot8))
				setAction(&_action2);
			else
				g_globals->_player.enableControl();
			break;
		case 5303:
			_hotspot8.setVisage(5102);
			_hotspot8.setPosition(Common::Point(533, 172));
			g_globals->_player.setPosition(Common::Point(512, 190));
			setAction(&_action2);
			break;
		}

		if (g_globals->getFlag(108)) {
			_hotspot6.postInit();
			_hotspot6.setVisage(5362);
			_hotspot6.setPosition(Common::Point(542, 19));
			_hotspot6.setStrip(6);
			_hotspot6.setFrame(1);
			g_globals->_sceneItems.push_back(&_hotspot6);
		} else if (RING_INVENTORY._vial._sceneNumber != 5100) {
			_hotspot6.postInit();
			_hotspot6.setVisage(5362);
			_hotspot6.setPosition(Common::Point(1152, 70));
			_hotspot6.fixPriority(170);
			_hotspot6.setStrip(6);
			_hotspot6.setFrame(1);
			g_globals->_sceneItems.push_back(&_hotspot6);
		}
		break;
	}

	_hotspot18.setBounds(Rect(0, 0, 1280, 200));
	g_globals->_sceneItems.push_back(&_hotspot18);

	_hotspot15.postInit();
	_hotspot15.setVisage(5140);
	_hotspot15.setStrip(3);
	_hotspot15.setPosition(Common::Point(977, 173));
	_hotspot15.fixPriority(1);

	g_globals->_sceneManager._scene->_sceneBounds.center(g_globals->_player._position);
	loadScene(5100);
	g_globals->_soundHandler.play(205);
}

void Scene5100::signal() {
	switch (_sceneMode) {
	case 5101:
	case 5112:
		g_globals->setFlag(67);
		g_globals->_sceneManager.changeScene(5300);
		break;
	case 5102:
	case 5114:
		g_globals->_player.enableControl();
		break;
	case 5103:
		if (g_globals->getFlag(61)) {
			SceneItem::display2(5100, 46);
			g_globals->_sceneManager.changeScene(5300);
		} else {
			SceneItem::display2(5100, 45);
			g_globals->_sceneManager.changeScene(5200);
		}
		break;
	case 5105:
		g_globals->_sceneManager.changeScene(5300);
		break;
	case 5106:
		g_globals->_stripNum = 5111;
		g_globals->_sceneManager.changeScene(5200);
		break;
	case 5108:
		if (!g_globals->getFlag(60))
			g_globals->_player.enableControl();
		else if (RING_INVENTORY._stasisBox._sceneNumber == 1)
			setAction(&_action2);
		else
			setAction(&_action5);
		break;
	case 5110:
		SceneItem::display2(5100, 30);
		g_globals->_player._angle = 325;
		g_globals->_player.enableControl();
		break;
	case 5111:
		g_globals->_player.setObjectWrapper(new SceneObjectWrapper());
		g_globals->_player.setVisage(0);
		g_globals->_player.setStrip(6);
		g_globals->_player.fixPriority(-1);
		g_globals->_player.animate(ANIM_MODE_1, NULL);

		if ((RING_INVENTORY._vial._sceneNumber != 5100) && !g_globals->getFlag(108)) {
			g_globals->setFlag(108);
			_sceneMode = 5130;
			g_globals->_player.disableControl();

			_speakerBatText.setTextPos(Common::Point(g_globals->_sceneManager._scene->_sceneBounds.left + 20, 30));
			setAction(&_sequenceManager, this, 5130, &g_globals->_player, &_hotspot6, NULL);
		} else if (g_globals->_sceneObjects->contains(&_hotspot8)) {
			setAction(&_action2);
		} else {
			g_globals->_player.enableControl();
		}
		break;
	case 5116:
		g_globals->setFlag(105);
		RING_INVENTORY._bone._sceneNumber = 0;

		g_globals->_player.setObjectWrapper(new SceneObjectWrapper());
		g_globals->_player.setVisage(0);
		g_globals->_player.setStrip(6);
		g_globals->_player.fixPriority(-1);
		g_globals->_player.animate(ANIM_MODE_1, NULL);
	// No break on purpose
	case 5117:
		g_globals->_player.enableControl();
		break;
	case 5130:
		_hotspot6.setVisage(5362);
		_hotspot6.setPosition(Common::Point(542, 25));
		_hotspot6.setStrip(6);
		_hotspot6.setFrame(1);
		g_globals->_player.enableControl();
		break;
	case 5150:
		g_globals->clearFlag(60);
		g_globals->_sceneManager.changeScene(5300);
		break;
	}
}

void Scene5100::dispatch() {
	// Flesheater trap
	if (_hotspot15._bounds.contains(g_globals->_player._position) && !g_globals->_player._visage) {
		g_globals->_player.disableControl();
		g_globals->_player.addMover(NULL);

		_soundHandler.play(207);
		_sceneMode = 5103;
		setAction(&_sequenceManager, this, (g_globals->_player._position.x >= 966) ? 5104 : 5103,
				&g_globals->_player, &_hotspot15, NULL);
	}

	if (g_globals->getFlag(61) && !g_globals->getFlag(62) &&
			((g_globals->_player._position.x - _hotspot2._position.x) < 160) &&
			(g_globals->_sceneManager._previousScene != 5200) && (_sceneMode != 5150)) {
		setAction(NULL);
		_sceneMode = 5150;
		_soundHandler.play(208);

		if (RING_INVENTORY._vial._sceneNumber == 5100) {
			g_globals->_player.addMover(NULL);
			g_globals->_player.disableControl();
			SceneItem::display2(5100, 39);
		}

		_hotspot2.setStrip2(1);
		_hotspot3.setStrip2(2);

		ObjectMover3 *mover1 = new ObjectMover3();
		_hotspot2.addMover(mover1, &g_globals->_player, 20, this);
		ObjectMover3 *mover2 = new ObjectMover3();
		_hotspot3.addMover(mover2, &g_globals->_player, 20, this);
	}

	if (!_action) {
		if (((g_globals->_player._position.x - _hotspot2._position.x) < 130) && !g_globals->getFlag(61)) {
			g_globals->_player._canWalk = false;
			g_globals->_player.addMover(NULL);

			Common::Point pt(20, 25);
			PlayerMover2 *mover = new PlayerMover2();
			_hotspot3.addMover(mover, &pt, &g_globals->_player);
			setAction(&_action4);
		}

		if ((g_globals->_sceneManager._scene->_backgroundBounds.right - 85) <= g_globals->_player._position.x)
			g_globals->_sceneManager.changeScene(5000);

		if (g_globals->_player.getRegionIndex() == 8) {
			_sceneMode = 5114;
			setAction(&_sequenceManager, this, 5115, &g_globals->_player, NULL);
		}
	}

	Scene::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 5200 - Caverns - Throne Room
 *
 *--------------------------------------------------------------------------*/

void Scene5200::Action1::signal() {
	Scene5200 *scene = (Scene5200 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(g_globals->_randomSource.getRandomNumber(59) + 120);
		break;
	case 1:
		scene->_hotspot3.animate(ANIM_MODE_8, 1, this);
		_actionIndex = 0;
		break;
	}
}

void Scene5200::Action2::signal() {
	// Quinn obtains the stasis box from the flesheater throne room
	Scene5200 *scene = (Scene5200 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		ADD_PLAYER_MOVER(92, 52);
		break;
	case 1:
		g_globals->_player.setVisage(5202);
		g_globals->_player._strip = 4;
		g_globals->_player._frame = 1;
		g_globals->_player.animate(ANIM_MODE_4, 3, 1, this);
		break;
	case 2:
		scene->_soundHandler.stop();
		scene->_hotspot14.remove();

		RING_INVENTORY._stasisBox._sceneNumber = 1;
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 3:
		g_globals->_player.setVisage(0);
		g_globals->_player._strip = 3;
		g_globals->_player.animate(ANIM_MODE_1, NULL);

		scene->_hotspot8.animate(ANIM_MODE_2, NULL);
		ADD_MOVER(scene->_hotspot8, 141, 77);
		break;
	case 4:
		scene->_soundHandler.play(303);

		scene->_hotspot8._strip = 2;
		scene->_hotspot8._frame = 1;
		scene->_hotspot8.animate(ANIM_MODE_5, this);
		break;
	case 5:
		g_globals->_player.enableControl();
		remove();
		break;
	}
}

void Scene5200::Action3::signal() {
	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		setDelay(5);
		break;
	case 1:
		ADD_PLAYER_MOVER(285, 62);
		break;
	case 2:
		g_globals->_player.enableControl();
		remove();
		break;
	}
}

void Scene5200::Action4::signal() {
	Scene5200 *scene = (Scene5200 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(120);
		break;
	case 1:
		g_globals->_soundHandler.play(209);
		scene->_stripManager.start(5202, this, scene);
		break;
	case 2:
		scene->_hotspot6.animate(ANIM_MODE_5, this);
		break;
	case 3:
		setDelay(10);
		break;
	case 4:
		scene->_stripManager.start(5204, this, scene);
		break;
	case 5:
		setDelay(60);
		break;
	case 6:
		g_globals->_sceneManager.changeScene(5100);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene5200::Hotspot9::doAction(int action) {
	switch (action) {
	case CURSOR_LOOK:
		if (!_state) {
			++_state;
			SceneItem::display2(5200, 5);
		} else {
			SceneItem::display2(5200, 6);
		}
		break;
	case CURSOR_USE:
		SceneItem::display2(5200, 14);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene5200::Hotspot10::doAction(int action) {
	switch (action) {
	case CURSOR_LOOK:
		if (!_state) {
			++_state;
			SceneItem::display2(5200, 7);
		} else {
			SceneItem::display2(5200, 8);
		}
		break;
	default:
		SceneObject::doAction(action);
		break;
	}
}

void Scene5200::Hotspot14::doAction(int action) {
	Scene5200 *scene = (Scene5200 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(5200, 4);
		break;
	case CURSOR_USE:
		scene->setAction(&scene->_action2);
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene5200::Scene5200() :
		_hotspot11(0, CURSOR_LOOK, 5200, 0, CURSOR_USE, 5200, 13, LIST_END),
		_hotspot12(10, CURSOR_LOOK, 5200, 9, LIST_END),
		_hotspot13(8, CURSOR_LOOK, 5200, 0, CURSOR_USE, 5200, 13, LIST_END) {
}


void Scene5200::postInit(SceneObjectList *OwnerList) {
	Scene::postInit();
	setZoomPercents(0, 150, 199, 150);
	_fieldA = 5600;
	_fieldE = 5100;

	_hotspot9._state = 0;
	_hotspot10._state = 0;
	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerGameText);
	_stripManager.addSpeaker(&_speakerFLText);
	_stripManager.addSpeaker(&_speakerFLL);
	_stripManager.addSpeaker(&_speakerQL);
	_stripManager.setCallback(this);

	_speakerFLText._textPos.x = 160;
	_speakerQText._textPos.x = 20;

	if (RING_INVENTORY._stasisBox._sceneNumber == 5200) {
		_soundHandler.play(216);
		_soundHandler.holdAt(true);

		_hotspot14.postInit();
		_hotspot14.setVisage(5202);
		_hotspot14._strip = 3;
		_hotspot14.setPosition(Common::Point(105, 52));
		_hotspot14.fixPriority(90);

		_hotspot8.postInit();
		_hotspot8.setVisage(5202);
		_hotspot8._strip = 1;
		_hotspot8.setPosition(Common::Point(96, 53));
		_hotspot8.fixPriority(90);
		g_globals->_sceneItems.push_back(&_hotspot14);
	}

	if (g_globals->_stripNum == 5111) {
		// Happens when the player enters the throne room via the secret passage,
		// after talking with the bat. No NPCs are around and the player can
		// obtain the stasis box.
		g_globals->_soundHandler.play(205);
		g_globals->_player.disableControl();

		g_globals->_player.postInit();
		g_globals->_player.setVisage(0);
		g_globals->_player.changeZoom(75);
		g_globals->_player.setObjectWrapper(new SceneObjectWrapper());
		g_globals->_player.animate(ANIM_MODE_1, NULL);
		g_globals->_player.setStrip(1);
		g_globals->_player.setFrame(3);
		g_globals->_player.setPosition(Common::Point(307, 62));

		setAction(&_action3);
	} else {
		// Happens when the player is captured by the flesh eaters the first time.
		g_globals->_player.postInit();
		g_globals->_player.setVisage(2640);
		g_globals->_player._strip = 1;
		g_globals->_player._frame = 4;
		g_globals->_player.setPosition(Common::Point(105, 199));

		_hotspot5.postInit();
		_hotspot5.setVisage(5210);
		_hotspot5._frame = 2;
		_hotspot5.setPosition(Common::Point(189, 167));

		_hotspot6.postInit();
		_hotspot6.setVisage(5210);
		_hotspot6._frame = 1;
		_hotspot6._strip = 2;
		_hotspot6.setPosition(Common::Point(159, 137));

		_hotspot7.postInit();
		_hotspot7.setVisage(5210);
		_hotspot7._frame = 1;
		_hotspot7._strip = 4;
		_hotspot7.fixPriority(168);
		_hotspot7.setPosition(Common::Point(186, 106));

		_hotspot1.postInit();
		_hotspot1.setVisage(5212);
		_hotspot1._frame = 1;
		_hotspot1.setPosition(Common::Point(62, 132));

		_hotspot2.postInit();
		_hotspot2.setVisage(5212);
		_hotspot2._strip = 3;
		_hotspot2.setPosition(Common::Point(148, 141));
		_hotspot2.fixPriority(90);

		_hotspot3.postInit();
		_hotspot3.setVisage(5212);
		_hotspot3._strip = 2;
		_hotspot3.setPosition(Common::Point(62, 109));
		_hotspot3.fixPriority(138);
		_hotspot3.setAction(&_action1);

		_hotspot4.postInit();
		_hotspot4.setVisage(5212);
		_hotspot4._strip = 4;
		_hotspot4.setPosition(Common::Point(146, 110));
		_hotspot4.fixPriority(90);

		g_globals->_player.disableControl();
		g_globals->setFlag(61);

		setAction(&_action4);
	}

	_hotspot11.setBounds(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
	_hotspot9._sceneRegionId = 11;
	_hotspot10._sceneRegionId = 9;
	_hotspot12._sceneRegionId = 10;
	_hotspot13._sceneRegionId = 8;

	g_globals->_sceneItems.addItems(&_hotspot12, &_hotspot13, &_hotspot9, &_hotspot10, &_hotspot11, NULL);
	g_globals->_sceneManager._scene->_sceneBounds.center(g_globals->_player._position);
	loadScene(5200);
}

void Scene5200::stripCallback(int v) {
	switch (v) {
	case 1:
		_hotspot7.animate(ANIM_MODE_2, NULL);
		break;
	case 2:
		_hotspot7.animate(ANIM_MODE_NONE, NULL);
		break;
	}
}

void Scene5200::dispatch() {
	Scene::dispatch();

	if (!_action && (g_globals->_player.getRegionIndex() == 12)) {
		g_globals->_stripNum = 5200;
		g_globals->_sceneManager.changeScene(5100);
	}
}

/*--------------------------------------------------------------------------
 * Scene 5300 - Caverns - Pit
 *
 *--------------------------------------------------------------------------*/

void Scene5300::Action1::signal() {
	// Seeker waking up
	Scene5300 *scene = (Scene5300 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		setDelay(3);
		break;
	case 1:
		scene->_hotspot2._numFrames = 4;
		scene->_hotspot2.animate(ANIM_MODE_8, 1, NULL);
		setDelay(120);
		break;
	case 2:
		scene->_hotspot2.animate(ANIM_MODE_NONE, NULL);

		g_globals->_player.animate(ANIM_MODE_1, NULL);
		g_globals->_player.setStrip2(-1);
		g_globals->_player.setObjectWrapper(new SceneObjectWrapper());
		ADD_MOVER(g_globals->_player, 85, 170);
		break;
	case 3:
		scene->_hotspot2.fixPriority(-1);
		g_globals->_player.checkAngle(&scene->_hotspot2);
		setAction(&scene->_sequenceManager, this, 5305, &scene->_hotspot2, NULL);
		break;
	case 4:
		scene->_stripManager.start(5316, this);
		break;
	case 5:
		if (!g_globals->getFlag(106) || !g_globals->getFlag(107) || (RING_INVENTORY._stasisBox._sceneNumber != 1)) {
			g_globals->_player.enableControl();
			remove();
		} else {
			g_globals->setFlag(60);
			scene->_hotspot2._numFrames = 10;

			if (g_globals->getFlag(67)) {
				scene->_sceneMode = 5310;
				scene->setAction(&scene->_sequenceManager, scene, 5310, &g_globals->_player, &scene->_hotspot2, NULL);
			} else {
				scene->_sceneMode = 5347;
				scene->setAction(&scene->_sequenceManager, scene, 5347, NULL);
			}
		}
		break;
	}
}

void Scene5300::Action2::signal() {
	Scene5300 *scene = (Scene5300 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		setDelay(60);
		break;
	case 1:
		g_globals->setFlag(60);
		scene->_stripManager.start(5328, this);
		break;
	case 2:
		if (RING_INVENTORY._stasisBox._sceneNumber == 1) {
			g_globals->_stripNum = 5303;
			setDelay(5);
		} else {
			g_globals->_stripNum = 5302;
			scene->_stripManager.start(5329, this);
		}
		break;
	case 3:
		g_globals->_sceneManager.changeScene(5100);
		remove();
		break;
	}
}

void Scene5300::Action3::signal() {
	Scene5300 *scene = (Scene5300 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		setDelay(30);
		break;
	case 1:
		scene->_stripManager.start(g_globals->_stripNum, this);
		break;
	case 2:
		g_globals->_player.enableControl();
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene5300::Hotspot1::doAction(int action) {
	Scene5300 *scene = (Scene5300 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(5300, 24);
		break;
	case CURSOR_TALK:
		g_globals->_stripNum = 5312;
		scene->setAction(&scene->_action3);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}
void Scene5300::Hotspot2::doAction(int action) {
	// Seeker
	Scene5300 *scene = (Scene5300 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(5300, 23);
		break;
	case CURSOR_USE:
		if (!g_globals->getFlag(106)) {
			g_globals->_player.disableControl();
			scene->_sceneMode = 5302;
			scene->setAction(&scene->_sequenceManager, scene, 5302, &g_globals->_player, NULL);
		} else {
			SceneItem::display2(5300, g_globals->getFlag(107) ? 25 : 20);
		}
		break;
	case CURSOR_TALK:
		if (!g_globals->getFlag(106)) {
			g_globals->_player.disableControl();
			scene->_sceneMode = 5302;
			scene->setAction(&scene->_sequenceManager, scene, 5302, &g_globals->_player, NULL);
		} else if (!g_globals->getFlag(107)) {
			SceneItem::display2(5300, 11);
		} else {
			g_globals->_player.disableControl();

			if (RING_INVENTORY._stasisBox._sceneNumber != 1) {
				scene->_sceneMode = 5316;
				scene->setAction(&scene->_sequenceManager, scene, 5316, NULL);
			} else {
				g_globals->setFlag(60);
				if (g_globals->getFlag(67)) {
					scene->_sceneMode = 5315;
					scene->setAction(&scene->_sequenceManager, scene, 5315, this, NULL);
				} else {
					scene->_sceneMode = 5347;
					scene->setAction(&scene->_sequenceManager, scene, 5347, NULL);
				}
			}
		}
		break;
	case OBJECT_SCANNER:
		SceneItem::display2(5300, g_globals->getFlag(107) ? 10 : 9);
		break;
	case OBJECT_MEDKIT:
		if (g_globals->getFlag(106))
			SceneItem::display2(5300, 7);
		else {
			g_globals->setFlag(106);
			g_globals->_player.disableControl();

			scene->_sceneMode = 5303;
			scene->setAction(&scene->_sequenceManager, scene, 5303, &g_globals->_player, NULL);
		}
		break;
	case OBJECT_VIAL:
		if (g_globals->getFlag(107)) {
			SceneItem::display2(5300, 8);
		} else {
			RING_INVENTORY._vial._sceneNumber = 5300;
			g_globals->setFlag(107);
			g_globals->_player.disableControl();
			scene->_sceneMode = 5304;

			scene->setAction(&scene->_sequenceManager, scene, 5304, &g_globals->_player, NULL);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene5300::Hotspot5::doAction(int action) {
	// Sharp bone
	Scene5300 *scene = (Scene5300 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(5300, 27);
		break;
	case CURSOR_USE:
		RING_INVENTORY._bone._sceneNumber = 1;
		g_globals->_player.disableControl();

		scene->_sceneMode = 5309;
		scene->setAction(&scene->_sequenceManager, scene, 5309, &g_globals->_player, this, NULL);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene5300::Hotspot6::doAction(int action) {
	// Left Hole
	Scene5300 *scene = (Scene5300 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		if (!g_globals->getFlag(105) || (RING_INVENTORY._vial._sceneNumber == 1))
			SceneItem::display2(5300, 4);
		else
			SceneItem::display2(5300, 26);
		break;
	case CURSOR_USE:
		if (!g_globals->getFlag(105) || (RING_INVENTORY._vial._sceneNumber != 5100)) {
			g_globals->_player.disableControl();
			scene->_sceneMode = 5301;
			scene->setAction(&scene->_sequenceManager, scene, 5301, &g_globals->_player, NULL);
		} else {
			g_globals->_player.disableControl();
			scene->_sceneMode = 5307;
			RING_INVENTORY._vial._sceneNumber = 1;

			scene->setAction(&scene->_sequenceManager, scene, 5307, &scene->_hotspot1, &g_globals->_player,
				&scene->_hotspot4, NULL);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene5300::Hotspot7::doAction(int action) {
	switch (action) {
	case CURSOR_LOOK:
		if (g_globals->getFlag(63))
			SceneItem::display2(5300, 2);
		else {
			g_globals->setFlag(63);
			SceneItem::display2(5300, 0);
			SceneItem::display2(5300, 1);
		}
		break;
	case CURSOR_USE:
		if (g_globals->getFlag(64))
			SceneItem::display2(5300, 15);
		else {
			g_globals->setFlag(64);
			SceneItem::display2(5300, 14);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene5300::Hotspot8::doAction(int action) {
	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(5300, g_globals->getFlag(65) ? 6 : 5);
		break;
	case CURSOR_USE:
		SceneItem::display2(5300, 18);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene5300::Scene5300() :
		_hotspot3(0, CURSOR_LOOK, 5300, 3, CURSOR_USE, 5300, 16, LIST_END) {
}

void Scene5300::postInit(SceneObjectList *OwnerList) {
	Scene::postInit();
	loadScene(5300);
	setZoomPercents(0, 100, 199, 100);

	_stripManager.addSpeaker(&_speakerQR);
	_stripManager.addSpeaker(&_speakerQL);
	_stripManager.addSpeaker(&_speakerBatR);
	_stripManager.addSpeaker(&_speakerSR);
	_stripManager.addSpeaker(&_speakerSL);
	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerBatText);
	_stripManager.addSpeaker(&_speakerSText);
	_stripManager.addSpeaker(&_speakerGameText);

	if (g_globals->getFlag(106) && g_globals->getFlag(107)) {
		_hotspot2.postInit();
		_hotspot2.setVisage(2806);
		_hotspot2.setObjectWrapper(new SceneObjectWrapper());
		_hotspot2.setPosition(Common::Point(63, 170));
		_hotspot2.animate(ANIM_MODE_1, NULL);
	} else {
		_hotspot2.postInit();
		_hotspot2.setVisage(5310);
		_hotspot2.setPosition(Common::Point(63, 170));
		_hotspot2.animate(ANIM_MODE_1, NULL);
		_hotspot2.fixPriority(98);
	}

	_hotspot1.postInit();
	_hotspot1.setVisage(5362);
	_hotspot1.setStrip2(3);
	_hotspot1.setFrame2(5);
	_hotspot1.setPosition(Common::Point(76, 48));

	_hotspot4.postInit();
	_hotspot4.setVisage(5316);
	_hotspot4.setStrip(2);
	_hotspot4.setPosition(Common::Point(89, 65));
	_hotspot4.animate(ANIM_MODE_2, NULL);
	_hotspot4.hide();

	if (g_globals->getFlag(67)) {
		g_globals->_player.postInit();
		g_globals->_player.setVisage(5316);
		g_globals->_player.setPosition(Common::Point(191, 27));
		g_globals->_player.disableControl();

		if (g_globals->getFlag(107) && g_globals->getFlag(106)) {
			_hotspot2.setVisage(2806);
			_hotspot2.postInit();
			_hotspot2.setObjectWrapper(new SceneObjectWrapper());
			_hotspot2.animate(ANIM_MODE_1, NULL);

			setAction(&_action2);
		} else {
			g_globals->_player.setVisage(5315);
			g_globals->_player.setPosition(Common::Point(204, 86));
			g_globals->_player.animate(ANIM_MODE_2, NULL);
			g_globals->_player.setStrip2(1);
			g_globals->_player._moveDiff.y = 12;

			_sceneMode = 5308;
			setAction(&_sequenceManager, this, 5308, &g_globals->_player, NULL);
		}
	} else {
		_hotspot3.postInit();
		_hotspot3.setVisage(5301);
		_hotspot3.setPosition(Common::Point(172, 32));
		_hotspot3.fixPriority(1);
		_hotspot3.animate(ANIM_MODE_NONE, NULL);

		g_globals->_player.postInit();
		g_globals->_player.setVisage(5315);
		g_globals->_player.setPosition(Common::Point(204, 86));
		g_globals->_player.animate(ANIM_MODE_2, NULL);
		g_globals->_player.setStrip2(1);
		g_globals->_player._moveDiff.y = 12;
		g_globals->_player.disableControl();

		_sceneMode = 5306;
		setAction(&_sequenceManager, this, 5306, &g_globals->_player, &_hotspot3, NULL);
	}

	_field1B0A = 1;
	if (RING_INVENTORY._bone._sceneNumber == 5300) {
		_hotspot5.postInit();
		_hotspot5.setVisage(5301);
		_hotspot5.setStrip(2);
		_hotspot5.setPosition(Common::Point(190, 147));
		g_globals->_sceneItems.push_back(&_hotspot5);
	}

	_hotspot6.setBounds(Rect(74, 51, 114, 69));
	_hotspot7.setBounds(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
	_hotspot8._sceneRegionId = 8;

	g_globals->_sceneItems.addItems(&_hotspot8, &_hotspot2, &_hotspot6, &_hotspot3, &_hotspot7, NULL);
	g_globals->_soundHandler.play(212);
}

void Scene5300::signal() {
	switch (_sceneMode) {
	case 5301:
		g_globals->_stripNum = 5300;
		g_globals->_sceneManager.changeScene(5100);
		break;
	case 5307:
		_soundHandler.fadeOut(NULL);
	// No break on purpose
	case 5302:
	case 5308:
	case 5316:
	case 5347:
		g_globals->_player.setStrip2(-1);
		g_globals->_player.animate(ANIM_MODE_1, NULL);
		g_globals->_player.enableControl();
		break;
	case 5303:
		g_globals->_player.animate(ANIM_MODE_1, NULL);
		g_globals->_player.enableControl();

		if (g_globals->getFlag(107))
			setAction(&_action1);
		else
			SceneItem::display2(5300, 28);
		break;
	case 5304:
		g_globals->_player.animate(ANIM_MODE_1, NULL);
		g_globals->_player.enableControl();

		if (g_globals->getFlag(106))
			setAction(&_action1);
		else
			SceneItem::display2(5300, 28);
		break;
	case 5306:
		g_globals->clearFlag(67);
		g_globals->_player.setStrip2(-1);

		if ((RING_INVENTORY._vial._sceneNumber == 1) || (RING_INVENTORY._vial._sceneNumber == 5300))
			_stripManager.start(5303, this);
		else
			_stripManager.start(5302, this);
		_sceneMode = 5302;
		break;
	case 5309:
		_hotspot5.remove();
		g_globals->_player.enableControl();
		break;
	case 5310:
		_hotspot2.fixPriority(41);
		_sceneMode = 5315;

		setAction(&_sequenceManager, this, 5315, &_hotspot2, NULL);
		break;
	case 5315:
		g_globals->_stripNum = 5302;
		g_globals->_sceneManager.changeScene(5100);
		break;
	}
}

} // End of namespace Ringworld

} // End of namespace TsAGE

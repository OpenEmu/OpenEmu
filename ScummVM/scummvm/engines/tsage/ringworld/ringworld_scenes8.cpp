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

#include "graphics/cursorman.h"
#include "tsage/ringworld/ringworld_scenes8.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace TsAGE {

namespace Ringworld {

void NamedHotspotMult::synchronize(Serializer &s) {
	SceneHotspot::synchronize(s);
	s.syncAsSint16LE(_useLineNum);
	s.syncAsSint16LE(_lookLineNum);
}

void SceneObject7700::synchronize(Serializer &s) {
	SceneObject::synchronize(s);
	if (s.getVersion() >= 3) {
		s.syncAsSint16LE(_lookLineNum);
		s.syncAsSint16LE(_defltLineNum);
	}
}

/*--------------------------------------------------------------------------
 * Scene 7000 - Landing near beach
 *
 *--------------------------------------------------------------------------*/

void Scene7000::Action1::signal() {
	// Quinn walks from the lander to the seaside (action6) then discuss with Skeenar
	Scene7000 *scene = (Scene7000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		setDelay(3);
		break;
	case 1:
		setAction(&scene->_action6, this);
		break;
	case 2:
		scene->_soundHandler.play(252);
		scene->_object8.remove();
		scene->_object1.postInit();
		scene->_object1.setVisage(7003);
		scene->_object1.animate(ANIM_MODE_5, this);
		scene->_object1.setPosition(Common::Point(151, 182));
		scene->_object1.fixPriority(205);
		g_globals->_sceneItems.push_front(&scene->_object1);
		break;
	case 3:
		scene->_object1._numFrames = 4;
		scene->_object1.setStrip(2);
		scene->_object1.animate(ANIM_MODE_8, 0, NULL);
		scene->_stripManager.start(7005, this);
		break;
	case 4:
		scene->_object1.animate(ANIM_MODE_2, NULL);
		setDelay(3);
		break;
	case 5:
		g_globals->_player.enableControl();
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene7000::Action2::signal() {
	Scene7000 *scene = (Scene7000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		setDelay(3);
		break;
	case 1:
		g_globals->_player.addMover(NULL);
		g_globals->_player.setVisage(7006);
		g_globals->_player.setStrip(1);
		g_globals->_player.setFrame(1);
		g_globals->_player.setPosition(Common::Point(g_globals->_player._position.x, g_globals->_player._position.y + 13));
		g_globals->_player.changeZoom(68);
		g_globals->_player.animate(ANIM_MODE_5, this);
		scene->_object1.remove();
		break;
	case 2:
		g_globals->_sceneManager.changeScene(7100);
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene7000::Action3::dispatch() {
	Scene7000 *scene = (Scene7000 *)g_globals->_sceneManager._scene;

	Action::dispatch();
	if (_actionIndex == 4)
		scene->_object4.setPosition(Common::Point(scene->_object3._position.x, scene->_object3._position.y + 15));
}

/*--------------------------------------------------------------------------*/

void Scene7000::Action3::signal() {
	// Lander is landing
	Scene7000 *scene = (Scene7000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(30);
		scene->setZoomPercents(10, 10, 62, 100);
		scene->_object4.postInit();
		scene->_object4.setVisage(5001);
		scene->_object4.setStrip2(2);
		scene->_object4.animate(ANIM_MODE_8, 0, NULL);
		scene->_object4.setPosition(Common::Point(10, 18));
		scene->_object4.fixPriority(10);
		scene->_object4.changeZoom(100);
		scene->_object4.hide();
		break;
	case 1: {
		NpcMover *mover = new NpcMover();
		Common::Point pt(107, 65);
		scene->_object3.addMover(mover, &pt, this);
		break;
	}
	case 2:
		scene->_object3._moveDiff.y = 1;
		scene->_object3.fixPriority(10);
		scene->_object4.setPosition(Common::Point(scene->_object3._position.x, scene->_object3._position.y + 15));
		scene->_object4.show();
		setDelay(30);
		break;
	case 3: {
		NpcMover *mover = new NpcMover();
		Common::Point pt(107, 92);
		scene->_object3.addMover(mover, &pt, this);
		break;
	}
	case 4:
		scene->_object4.remove();
		g_globals->_sceneManager.changeScene(2100);
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene7000::Action4::signal() {
	Scene7000 *scene = (Scene7000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		scene->_object1.animate(ANIM_MODE_6, this);
		break;
	case 1:
		scene->_object1.hide();
		setDelay(300);
		break;
	case 2:
		g_globals->_soundHandler.play(252);
		scene->_object1.show();
		scene->_object1.setStrip(3);
		scene->_object1.setFrame(1);
		scene->_object1.animate(ANIM_MODE_5, this);
		break;
	case 3:
		scene->_object1.setStrip(4);
		scene->_object1.animate(ANIM_MODE_8, 0, NULL);
		g_globals->setFlag(81);
		g_globals->_player.enableControl();
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene7000::Action5::signal() {
	Scene7000 *scene = (Scene7000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		NpcMover *playerMover = new NpcMover();
		Common::Point pt(88, 121);
		g_globals->_player.addMover(playerMover, &pt, this);
		break;
	}
	case 1:
		g_globals->_player.checkAngle(&scene->_object1);
		g_globals->_soundHandler.play(252);
		scene->_object1.setStrip(2);
		scene->_stripManager.start(7015, this);
		break;
	case 2:
		scene->_object1.setStrip(1);
		scene->_object1.setFrame(4);
		scene->_object1.animate(ANIM_MODE_6, this);
		break;
	case 3: {
		scene->_object1.remove();
		NpcMover *mover = new NpcMover();
		Common::Point pt(31, 94);
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 4: {
		NpcMover *mover = new NpcMover();
		Common::Point pt(11, 94);
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 5: {
		g_globals->_player.fixPriority(10);
		NpcMover *mover = new NpcMover();
		Common::Point pt(11, 89);
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 6: {
		NpcMover *mover = new NpcMover();
		Common::Point pt(41, 89);
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 7:
		g_globals->clearFlag(36);
		g_globals->clearFlag(37);
		g_globals->clearFlag(72);
		g_globals->clearFlag(13);
		g_globals->_sceneManager.changeScene(2100);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene7000::Action6::signal() {
	// Quinn walks from the lander to the seaside
	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		setDelay(3);
		break;
	case 1: {
		NpcMover *mover = new NpcMover();
		Common::Point pt(12, 91);
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 2: {
		NpcMover *mover = new NpcMover();
		Common::Point pt(8, 91);
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 3: {
		NpcMover *mover = new NpcMover();
		Common::Point pt(31, 96);
		g_globals->_player.addMover(mover, &pt, this);
		g_globals->_player.fixPriority(-1);
		break;
	}
	case 4: {
		NpcMover *mover = new NpcMover();
		Common::Point pt(83, 117);
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 5: {
		NpcMover *mover = new NpcMover();
		Common::Point pt(95, 121);
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 6:
		g_globals->_player.setStrip(3);
		g_globals->_player.setFrame(1);
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene7000::Action7::signal() {
	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		setDelay(3);
		break;
	case 1:
		setDelay(3);
		break;
	case 2: {
		NpcMover *mover = new NpcMover();
		Common::Point pt(31, 94);
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 3: {
		NpcMover *mover = new NpcMover();
		Common::Point pt(11, 94);
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 4: {
		g_globals->_player.fixPriority(10);
		NpcMover *mover = new NpcMover();
		Common::Point pt(11, 89);
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 5: {
		NpcMover *mover = new NpcMover();
		Common::Point pt(41, 89);
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 6:
		if (g_globals->getFlag(13))
			g_globals->_sceneManager.changeScene(2280);
		else
			g_globals->_sceneManager.changeScene(2320);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene7000::Hotspot1::doAction(int action) {
	if (action == CURSOR_LOOK)
		SceneItem::display2(7000, 2);
	else
		SceneHotspot::doAction(action);
}

/*--------------------------------------------------------------------------*/

void Scene7000::Object1::doAction(int action) {
	// Skeenar
	Scene7000 *scene = (Scene7000 *)g_globals->_sceneManager._scene;

	switch (action) {
	case OBJECT_TRANSLATOR:
		g_globals->_player.disableControl();
		RING_INVENTORY._translator._sceneNumber = 7000;

		if ((RING_INVENTORY._waldos._sceneNumber != 7000) && (RING_INVENTORY._jar._sceneNumber != 7000)) {
			scene->_sceneMode = 7004;
			scene->setAction(&scene->_sequenceManager, scene, 7004, &g_globals->_player, this, NULL);
		} else if (RING_INVENTORY._waldos._sceneNumber != 7000) {
			scene->_sceneMode = 7011;
			scene->setAction(&scene->_sequenceManager, scene, 7010, &g_globals->_player, &scene->_object1, NULL);
		} else if (RING_INVENTORY._jar._sceneNumber != 7000) {
			scene->_sceneMode = 7012;
			scene->setAction(&scene->_sequenceManager, scene, 7010, &g_globals->_player, &scene->_object1, NULL);
		} else {
			scene->_sceneMode = 7015;
			scene->setAction(&scene->_sequenceManager, scene, 7017, &g_globals->_player, NULL);
		}
		break;
	case OBJECT_WALDOS:
		g_globals->_player.disableControl();
		RING_INVENTORY._waldos._sceneNumber = 7000;
		if (RING_INVENTORY._translator._sceneNumber == 7000) {
			if (RING_INVENTORY._jar._sceneNumber == 7000) {
				scene->_sceneMode = 7015;
				scene->setAction(&scene->_sequenceManager, scene, 7015, &g_globals->_player, NULL);
			} else {
				scene->_sceneMode = 7006;
				scene->setAction(&scene->_sequenceManager, scene, 7006, &g_globals->_player, NULL);
			}
		} else {
			scene->_sceneMode = 7009;
			scene->setAction(&scene->_sequenceManager, scene, 7009, &g_globals->_player, NULL);
		}
		break;
	case OBJECT_JAR:
		g_globals->_player.disableControl();
		RING_INVENTORY._jar._sceneNumber = 7000;

		if (RING_INVENTORY._translator._sceneNumber == 7000) {
			if (RING_INVENTORY._waldos._sceneNumber != 7000) {
				scene->_sceneMode = 7007;
				scene->setAction(&scene->_sequenceManager, scene, 7007, &g_globals->_player, &scene->_object1, NULL);
			} else {
				scene->_sceneMode = 7015;
				scene->setAction(&scene->_sequenceManager, scene, 7016, &g_globals->_player, NULL);
			}
		} else {
			scene->_sceneMode = 7008;
			scene->setAction(&scene->_sequenceManager, scene, 7008, &g_globals->_player, NULL);
		}
		break;
	case CURSOR_LOOK:
		if (g_globals->getFlag(81))
			SceneItem::display2(7000, 1);
		else
			SceneItem::display2(7000, 0);
		break;
	case CURSOR_USE:
		if (g_globals->getFlag(81)) {
			RING_INVENTORY._stasisBox2._sceneNumber = 1;
			g_globals->_player.disableControl();
			scene->setAction(&scene->_action5);
		} else {
			SceneItem::display2(7000, 5);
		}
		break;
	case CURSOR_TALK:
		if (g_globals->getFlag(81)) {
			RING_INVENTORY._stasisBox2._sceneNumber = 1;
			g_globals->_player.disableControl();
			scene->setAction(&scene->_action5);
		} else if (g_globals->getFlag(52)) {
			scene->_sceneMode = 7005;
			scene->setAction(&scene->_sequenceManager, scene, 7013, NULL);
		} else if (g_globals->getFlag(13)) {
			scene->_sceneMode = 7002;
			scene->setAction(&scene->_sequenceManager, scene, 7014, NULL);
		} else {
			scene->_sceneMode = 7002;
			scene->setAction(&scene->_sequenceManager, scene, 7002, NULL);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene7000::dispatch() {
	Scene7000 *scene = (Scene7000 *)g_globals->_sceneManager._scene;

	if (!_action) {
		if (g_globals->_sceneRegions.indexOf(g_globals->_player._position) == 8) {
			if (!g_globals->getFlag(13)) {
				g_globals->_player.disableControl();
				g_globals->_player.addMover(NULL);
				SceneItem::display2(7000, 3);
				_sceneMode = 7001;
				setAction(&scene->_sequenceManager, this, 7001, &g_globals->_player, NULL);
			} else if (!g_globals->getFlag(52)) {
				setAction(&_action2);
			} else {
				g_globals->_player.disableControl();
				_sceneMode = 7003;
				setAction(&scene->_sequenceManager, this, 7003, &g_globals->_player, NULL);
			}
		}
		if (g_globals->_sceneRegions.indexOf(g_globals->_player._position) == 9)
			scene->setAction(&scene->_action7);
	}
	Scene::dispatch();
}

/*--------------------------------------------------------------------------*/

void Scene7000::postInit(SceneObjectList *OwnerList) {
	loadScene(7000);
	Scene::postInit();
	setZoomPercents(93, 25, 119, 55);
	_stripManager.addSpeaker(&_speakerSKText);
	_stripManager.addSpeaker(&_speakerSKL);
	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerQL);
	_stripManager.addSpeaker(&_speakerQR);

	_speakerSKText._npc = &_object1;
	_speakerQText._npc = &g_globals->_player;

	_object5.postInit();
	_object5.setVisage(7001);
	_object5.setStrip2(1);
	_object5.animate(ANIM_MODE_2, NULL);
	_object5.setPosition(Common::Point(49, 147));
	_object5.fixPriority(1);

	_object6.postInit();
	_object6.setVisage(7001);
	_object6.setStrip2(2);
	_object6.animate(ANIM_MODE_2, NULL);
	_object6.setPosition(Common::Point(160, 139));
	_object6.fixPriority(1);

	_object7.postInit();
	_object7.setVisage(7001);
	_object7.setStrip2(3);
	_object7.animate(ANIM_MODE_2, NULL);
	_object7.setPosition(Common::Point(272, 129));
	_object7.fixPriority(1);

	_object8.postInit();
	_object8.setVisage(7001);
	_object8.setStrip2(4);
	_object8.animate(ANIM_MODE_2, NULL);
	_object8.setPosition(Common::Point(176, 175));
	_object8.fixPriority(1);

	if (g_globals->getFlag(72)) {
		_object3.postInit();
		_object3.setVisage(5001);
		_object3.setStrip2(1);
		_object3.setPosition(Common::Point(107, 92));
		_object3.changeZoom(100);
		_object3.fixPriority(10);

		_object1.postInit();
		_object1.setVisage(7003);
		if (g_globals->getFlag(81))
			_object1.setStrip(4);
		else
			_object1.setStrip(2);
		_object1.setPosition(Common::Point(87, 129));
		_object1._numFrames = 4;
		_object1.changeZoom(45);
		_object1.animate(ANIM_MODE_8, 0, NULL);
		g_globals->_sceneItems.push_back(&_object1);
	}
	_soundHandler.play(251);
	if (g_globals->_sceneManager._previousScene == 2100) {
		if (g_globals->getFlag(72)) {
			g_globals->_player.postInit();
			g_globals->_player.setVisage(0);
			g_globals->_player.animate(ANIM_MODE_1, NULL);
			SceneObjectWrapper *wrapper = new SceneObjectWrapper();
			g_globals->_player.setObjectWrapper(wrapper);
			g_globals->_player.setPosition(Common::Point(57, 94));
			g_globals->_player.changeZoom(-1);
			g_globals->_player.fixPriority(10);
			if (g_globals->getFlag(81)) {
				setAction(&_action4);
			} else {
				_object1.setPosition(Common::Point(151, 182));
				_object1.changeZoom(100);
				setAction(&_action1);
			}
		} else {
			g_globals->_soundHandler.play(250);
			g_globals->setFlag(72);

			_object3.postInit();
			_object3.setVisage(5001);
			_object3.setStrip2(1);
			_object3.animate(ANIM_MODE_1, NULL);
			_object3.setPosition(Common::Point(307, 0));
			_object3.changeZoom(-1);
			setAction(&_action3);
		}
	} else if (g_globals->_sceneManager._previousScene == 2280) {
		g_globals->_player.postInit();
		g_globals->_player.setVisage(2170);
		g_globals->_player.animate(ANIM_MODE_1, NULL);
		g_globals->_player.setObjectWrapper(new SceneObjectWrapper());
		g_globals->_player.setPosition(Common::Point(57, 94));
		g_globals->_player.changeZoom(-1);
		g_globals->_player.fixPriority(10);
		g_globals->_player.disableControl();
		_sceneMode = 7001;
		setAction(&_action6, this);
		if (!g_globals->getFlag(81)) {
			_object1.setPosition(Common::Point(151, 182));
			_object1.changeZoom(100);
		}
		_object8.remove();
		_object9.remove();
	} else if (g_globals->_sceneManager._previousScene == 2320) {
		g_globals->_player.postInit();
		g_globals->_player.setVisage(0);
		g_globals->_player.animate(ANIM_MODE_1, NULL);
		SceneObjectWrapper *wrapper = new SceneObjectWrapper();
		g_globals->_player.setObjectWrapper(wrapper);
		g_globals->_player.setPosition(Common::Point(57, 94));
		g_globals->_player.changeZoom(-1);
		g_globals->_player.fixPriority(10);
		_sceneMode = 7001;
		setAction(&_action6, this);
	} else {
		g_globals->setFlag(72);

		_object3.postInit();
		_object3.setVisage(5001);
		_object3.setStrip2(1);
		_object3.setPosition(Common::Point(307, 0));
		_soundHandler.play(151);
		_soundHandler.holdAt(true);
		g_globals->_soundHandler.play(250);
		setAction(&_action3);
	}

	_hotspot1.setBounds(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
	g_globals->_sceneItems.push_back(&_hotspot1);
}

/*--------------------------------------------------------------------------*/

void Scene7000::signal() {
	Scene7000 *scene = (Scene7000 *)g_globals->_sceneManager._scene;
	switch (_sceneMode) {
	case 7001:
	case 7002:
	case 7004:
	case 7005:
	case 7006:
	case 7007:
	case 7008:
	case 7009:
		g_globals->_player.enableControl();
		break;
	case 7003:
		_sceneMode = 7001;
		setAction(&scene->_sequenceManager, this, 7001, &g_globals->_player, NULL);
		break;
	case 7011:
		_sceneMode = 7005;
		setAction(&scene->_sequenceManager, this, 7005, &g_globals->_player, NULL);
		break;
	case 7012:
		_sceneMode = 7005;
		setAction(&scene->_sequenceManager, this, 7012, &g_globals->_player, NULL);
		break;
	case 7015:
		setAction(&_action4);
		break;
	}
}


/*--------------------------------------------------------------------------
 * Scene 7100 - Underwater: swimming
 *
 *--------------------------------------------------------------------------*/

void Scene7100::Action3::signal() {
	Scene7100 *scene = (Scene7100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(1);
		break;
	case 1: {
		Common::Point pt(433, 308);
		NpcMover *mover = new NpcMover();
		scene->_object4.addMover(mover, &pt, this);
		break;
	}
	case 2:
		scene->_object4.remove();
		remove();
		break;
	}
}

void Scene7100::Action4::signal() {
	Scene7100 *scene = (Scene7100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(g_globals->_randomSource.getRandomNumber(1) + 1);
		break;
	case 1: {
		scene->_object5.setStrip(3);
		Common::Point pt(85, 52);
		NpcMover *mover = new NpcMover();
		scene->_object5.addMover(mover, &pt, this);
		break;
	}
	case 2: {
		scene->_object5.setStrip(4);
		Common::Point pt(20, 52);
		NpcMover *mover = new NpcMover();
		scene->_object5.addMover(mover, &pt, this);
		break;
	}
	case 3:
		_actionIndex = 0;
		setDelay(1);
		break;
	}
}

void Scene7100::Action5::signal() {
	Scene7100 *scene = (Scene7100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(1);
		break;
	case 1: {
		Common::Point pt(59, 151);
		NpcMover *mover = new NpcMover();
		scene->_object9.addMover(mover, &pt, this);
		break;
	}
	case 2: {
		scene->_object9.setStrip2(1);
		Common::Point pt(127, 144);
		NpcMover *mover = new NpcMover();
		scene->_object9.addMover(mover, &pt, this);
		break;
	}
	case 3: {
		scene->_object9.setStrip2(2);
		scene->_object9.fixPriority(180);
		Common::Point pt(8, 181);
		NpcMover *mover = new NpcMover();
		scene->_object9.addMover(mover, &pt, this);
		break;
	}
	case 4: {
		scene->_object9.remove();
		remove();
	}
	}
}

void Scene7100::Action6::signal() {
	Scene7100 *scene = (Scene7100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(1);
		scene->_object10.fixPriority(8);
		scene->_object10.setPosition(Common::Point(155, 187));

		scene->_object11.fixPriority(8);
		scene->_object11.setPosition(Common::Point(155, 190));

		scene->_object12.fixPriority(8);
		scene->_object12.setPosition(Common::Point(151, 193));
		break;
	case 1: {
		Common::Point pt1(167, 187);
		NpcMover *mover1 = new NpcMover();
		scene->_object10.addMover(mover1, &pt1, this);

		Common::Point pt2(165, 185);
		NpcMover *mover2 = new NpcMover();
		scene->_object11.addMover(mover2, &pt2, NULL);

		Common::Point pt3(163, 183);
		NpcMover *mover3 = new NpcMover();
		scene->_object12.addMover(mover3, &pt3, NULL);
		break;
	}
	case 2: {
		scene->_object10.setStrip2(6);
		Common::Point pt1(91, 187);
		NpcMover *mover1 = new NpcMover();
		scene->_object10.addMover(mover1, &pt1, this);

		scene->_object11.setStrip2(6);
		scene->_object11.fixPriority(50);
		Common::Point pt2(89, 185);
		NpcMover *mover2 = new NpcMover();
		scene->_object11.addMover(mover2, &pt2, NULL);

		scene->_object12.setStrip2(6);
		scene->_object12.fixPriority(50);
		Common::Point pt3(87, 183);
		NpcMover *mover3 = new NpcMover();
		scene->_object12.addMover(mover3, &pt3, NULL);
		break;
	}
	case 3:
		_actionIndex = 0;
		setDelay(1);
		break;
	}
}

void Scene7100::Action7::signal() {
	Scene7100 *scene = (Scene7100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(1);
		break;
	case 1: {
		scene->_object13.setStrip2(8);
		Common::Point pt(324, 87);
		NpcMover *mover = new NpcMover();
		scene->_object13.addMover(mover, &pt, this);
		break;
	}
	case 2: {
		scene->_object13.setStrip2(2);
		Common::Point pt(524, 104);
		NpcMover *mover = new NpcMover();
		scene->_object13.addMover(mover, &pt, NULL);
		break;
	}
	case 3:
		_actionIndex = 0;
		setDelay(1);
		break;
	}
}

void Scene7100::Action8::signal() {
	Scene7100 *scene = (Scene7100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(1);
		break;
	case 1: {
		scene->_object17.setStrip2(4);
		scene->_object18.setStrip2(4);
		scene->_object19.setStrip2(4);

		Common::Point pt1(482, 153);
		NpcMover *mover1 = new NpcMover();
		scene->_object17.addMover(mover1, &pt1, this);

		Common::Point pt2(480, 146);
		NpcMover *mover2 = new NpcMover();
		scene->_object18.addMover(mover2, &pt2, NULL);

		Common::Point pt3(470, 153);
		NpcMover *mover3 = new NpcMover();
		scene->_object19.addMover(mover3, &pt3, NULL);
		break;
	}
	case 2: {
		scene->_object17.setStrip2(3);
		scene->_object18.setStrip2(3);
		scene->_object19.setStrip2(3);

		Common::Point pt1(506, 186);
		NpcMover *mover1 = new NpcMover();
		scene->_object17.addMover(mover1, &pt1, this);

		Common::Point pt2(502, 179);
		NpcMover *mover2 = new NpcMover();
		scene->_object18.addMover(mover2, &pt2, NULL);

		Common::Point pt3(495, 184);
		NpcMover *mover3 = new NpcMover();
		scene->_object19.addMover(mover3, &pt3, NULL);
		break;
	}
	case 3: {
		scene->_object17.setStrip2(4);
		scene->_object18.setStrip2(4);
		scene->_object19.setStrip2(4);

		Common::Point pt1(386, 167);
		NpcMover *mover1 = new NpcMover();
		scene->_object17.addMover(mover1, &pt1, this);

		Common::Point pt2(379, 161);
		NpcMover *mover2 = new NpcMover();
		scene->_object18.addMover(mover2, &pt2, NULL);

		Common::Point pt3(373, 167);
		NpcMover *mover3 = new NpcMover();
		scene->_object19.addMover(mover3, &pt3, NULL);
		break;
	}
	case 4: {
		scene->_object17.setStrip2(3);
		scene->_object18.setStrip2(3);
		scene->_object19.setStrip2(3);

		Common::Point pt1(479, 193);
		NpcMover *mover1 = new NpcMover();
		scene->_object17.addMover(mover1, &pt1, this);

		Common::Point pt2(473, 187);
		NpcMover *mover2 = new NpcMover();
		scene->_object18.addMover(mover2, &pt2, NULL);

		Common::Point pt3(466, 192);
		NpcMover *mover3 = new NpcMover();
		scene->_object19.addMover(mover3, &pt3, NULL);
		break;
	}
	case 5: {
		Common::Point pt1(552, 183);
		NpcMover *mover1 = new NpcMover();
		scene->_object17.addMover(mover1, &pt1, this);

		Common::Point pt2(552, 178);
		NpcMover *mover2 = new NpcMover();
		scene->_object18.addMover(mover2, &pt2, NULL);

		Common::Point pt3(541, 183);
		NpcMover *mover3 = new NpcMover();
		scene->_object19.addMover(mover3, &pt3, NULL);

		_actionIndex = 0;
		break;
	}
	}
}

void Scene7100::Action9::signal() {
	Scene7100 *scene = (Scene7100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(1);
		break;
	case 1: {
		scene->_object24.setStrip2(1);
		Common::Point pt(64, 159);
		NpcMover *mover = new NpcMover();
		scene->_object24.addMover(mover, &pt, this);
		break;
	}
	case 2: {
		scene->_object24.setStrip2(2);
		scene->_object24.fixPriority(160);
		Common::Point pt(34, 159);
		NpcMover *mover = new NpcMover();
		scene->_object24.addMover(mover, &pt, this);
		break;
	}
	case 3: {
		scene->_object24.setStrip2(1);
		Common::Point pt(64, 159);
		NpcMover *mover = new NpcMover();
		scene->_object24.addMover(mover, &pt, this);
		break;
	}
	case 4: {
		scene->_object24.setStrip2(2);
		scene->_object24.fixPriority(180);
		Common::Point pt(-12, 182);
		NpcMover *mover = new NpcMover();
		scene->_object24.addMover(mover, &pt, this);
		break;
	}
	case 5: {
		_actionIndex = 0;
		setDelay(1);
		break;
	}
	}
}

void Scene7100::Action10::signal() {
	Scene7100 *scene = (Scene7100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(1000);
		break;
	case 1: {
		Common::Point pt(610, -60);
		NpcMover *mover = new NpcMover();
		scene->_object25.addMover(mover, &pt, this);
		break;
	}
	case 2:
		scene->_object25.remove();
		remove();
		break;
	}
}

void Scene7100::Action11::signal() {
	Scene7100 *scene = (Scene7100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(30);
		break;
	case 1: {
		Common::Point pt(154, 175);
		NpcMover *mover = new NpcMover();
		g_globals->_player.addMover(mover, &pt, this);
		ObjectMover2 *mover2 = new ObjectMover2();
		scene->_object1.addMover(mover2, 25, 35, &g_globals->_player);
		break;
	}
	case 2: {
		Common::Point pt(700, 155);
		NpcMover *mover = new NpcMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 3:
		g_globals->_sceneManager.changeScene(7200);
		remove();
		break;
	}
}

void Scene7100::postInit(SceneObjectList *OwnerList) {
	loadScene(7100);
	Scene::postInit();
	setZoomPercents(60, 85, 200, 100);

	_object2.postInit();
	_object2.setVisage(7161);
	_object2.animate(ANIM_MODE_2, NULL);
	_object2.setPosition(Common::Point(10, 140));
	_object2._numFrames = 1;
	_object2.fixPriority(180);
	_object2.setAction(&_action1, NULL);

	_object3.postInit();
	_object3.setVisage(7161);
	_object3.animate(ANIM_MODE_2, NULL);
	_object3.setPosition(Common::Point(34, 115));
	_object3._numFrames = 1;
	_object3.fixPriority(180);
	_object3.setAction(&_action2, NULL);

	_object4.postInit();
	_object4.setVisage(7164);
	_object4.animate(ANIM_MODE_2, NULL);
	_object4.setPosition(Common::Point(-10, 159));
	_object4._numFrames = 2;
	_object4.fixPriority(250);
	_object4.setAction(&_action3, NULL);

	_object5.postInit();
	_object5.setVisage(7162);
	_object5.setStrip(3);
	_object5.animate(ANIM_MODE_2, NULL);
	_object5.setPosition(Common::Point(20, 52));
	_object5.setAction(&_action4, NULL);

	_object9.postInit();
	_object9.setVisage(7160);
	_object5.setStrip(2);
	_object9.animate(ANIM_MODE_2, NULL);
	_object9.setPosition(Common::Point(110, 168));
	_object9._numFrames = 2;
	_object9.fixPriority(16);
	_object9.setAction(&_action5, NULL);

	_object13.postInit();
	_object13.setVisage(7161);
	_object13.setStrip(8);
	_object13.animate(ANIM_MODE_2, NULL);
	_object13.setPosition(Common::Point(524, 104));
	_object13._numFrames = 5;
	_object13.fixPriority(250);
	_object13.setAction(&_action7, NULL);

	_object17.postInit();
	_object17.setVisage(7160);
	_object17.setStrip(3);
	_object17.animate(ANIM_MODE_2, NULL);
	_object17.setPosition(Common::Point(552, 183));
	_object17._numFrames = 4;
	_object17._moveDiff.x = 12;
	_object17._moveDiff.y = 12;
	_object17.setAction(&_action8, NULL);

	_object18.postInit();
	_object18.setVisage(7160);
	_object18.setStrip(3);
	_object18.animate(ANIM_MODE_2, NULL);
	_object18.setPosition(Common::Point(552, 178));
	_object18._numFrames = 4;
	_object18._moveDiff.x = 12;
	_object18._moveDiff.y = 12;

	_object19.postInit();
	_object19.setVisage(7160);
	_object19.setStrip(3);
	_object19.animate(ANIM_MODE_2, NULL);
	_object19.setPosition(Common::Point(541, 183));
	_object19._numFrames = 4;
	_object19._moveDiff.x = 12;
	_object19._moveDiff.y = 12;

	_object24.postInit();
	_object24.setVisage(7162);
	_object24.setStrip(1);
	_object24.animate(ANIM_MODE_2, NULL);
	_object24.setPosition(Common::Point(-12, 182));
	_object24._numFrames = 4;
	_object24.fixPriority(180);
	_object24.setAction(&_action9, NULL);

	_object25.postInit();
	_object25.setVisage(7163);
	_object25.animate(ANIM_MODE_2, NULL);
	_object25.setPosition(Common::Point(551, 145));
	_object25._numFrames = 5;
	_object25.fixPriority(160);
	_object25.setAction(&_action10, NULL);

	// Swimmer 1
	g_globals->_player.postInit();
	g_globals->_player.setVisage(7101);
	g_globals->_player.animate(ANIM_MODE_2, NULL);
	g_globals->_player._moveDiff.x = 4;
	g_globals->_player._moveDiff.y = 2;
	g_globals->_player.setPosition(Common::Point(135, 135));
	g_globals->_player.fixPriority(200);
	g_globals->_player.disableControl();

	// Swimmer 2
	_object1.postInit();
	_object1.setVisage(7110);
	_object1.animate(ANIM_MODE_1, NULL);
	_object1._moveDiff.x = 4;
	_object1._moveDiff.y = 2;
	_object1.setPosition(Common::Point(100, 100));

	setAction(&_action11);
	_soundHandler1.play(270);
	_soundHandler2.play(275);
	g_globals->_soundHandler.play(270);
}
/*--------------------------------------------------------------------------
 * Scene 7200 - Underwater: Entering the cave
 *
 *--------------------------------------------------------------------------*/

void Scene7200::Action1::signal() {
	Scene7200 *scene = (Scene7200 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(15);
		break;
	case 1: {
		PlayerMover *mover1 = new PlayerMover();
		Common::Point pt1(165, 147);
		scene->_swimmer.addMover(mover1, &pt1, this);
		Common::Point pt2(207, 138);
		PlayerMover *mover2 = new PlayerMover();
		g_globals->_player.addMover(mover2, &pt2, this);
		break;
	}
	case 2:
		break;
	case 3:
		g_globals->_sceneManager.changeScene(7300);
		remove();
	break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene7200::Action2::signal() {
	Scene7200 *scene = (Scene7200 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(3);
		break;
	case 1: {
		scene->_object2.fixPriority(25);
		scene->_object3.fixPriority(25);
		scene->_object4.fixPriority(25);
		scene->_object2.setStrip(1);
		scene->_object3.setStrip(1);
		scene->_object4.setStrip(1);
		NpcMover *mover1 = new NpcMover();
		Common::Point pt1(54, 90);
		scene->_object2.addMover(mover1, &pt1, this);
		NpcMover *mover2 = new NpcMover();
		Common::Point pt2(56, 85);
		scene->_object3.addMover(mover2, &pt2, NULL);
		NpcMover *mover3 = new NpcMover();
		Common::Point pt3(54, 80);
		scene->_object4.addMover(mover3, &pt3, NULL);
		break;
	}
	case 2: {
		scene->_object2.fixPriority(160);
		scene->_object3.fixPriority(160);
		scene->_object4.fixPriority(160);
		scene->_object2.setStrip(2);
		scene->_object3.setStrip(2);
		scene->_object4.setStrip(2);
		NpcMover *mover1 = new NpcMover();
		Common::Point pt1(10, 89);
		scene->_object2.addMover(mover1, &pt1, this);
		NpcMover *mover2 = new NpcMover();
		Common::Point pt2(12, 84);
		scene->_object3.addMover(mover2, &pt2, NULL);
		NpcMover *mover3 = new NpcMover();
		Common::Point pt3(10, 79);
		scene->_object4.addMover(mover3, &pt3, NULL);
		break;
	}
	case 3:
		_actionIndex = 0;
		setDelay(1);
		remove();
	break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene7200::postInit(SceneObjectList *OwnerList) {
	loadScene(7200);
	Scene::postInit();
	_object2.postInit();
	_object2.setVisage(7160);
	_object2.animate(ANIM_MODE_2, NULL);
	_object2.setZoom(10);
	_object2.setPosition(Common::Point(53, 88));
	_object2.setAction(&_action2);

	_object3.postInit();
	_object3.setVisage(7160);
	_object3.animate(ANIM_MODE_2, NULL);
	_object3.setZoom(10);
	_object3.setPosition(Common::Point(55, 83));

	_object4.postInit();
	_object4.setVisage(7160);
	_object4.animate(ANIM_MODE_2, NULL);
	_object4.setZoom(10);
	_object4.setPosition(Common::Point(57, 78));

	_object5.postInit();
	_object5.setVisage(7201);
	_object5.setPosition(Common::Point(300, 172));
	_object5.setPriority(172);
	_object5.animate(ANIM_MODE_2, NULL);
	_object5._numFrames = 3;

	_object6.postInit();
	_object6.setVisage(7201);
	_object6.setStrip2(3);
	_object6.setPosition(Common::Point(144, 97));
	_object6.setPriority(199);
	_object6.animate(ANIM_MODE_2, NULL);
	_object6._numFrames = 3;

	_object7.postInit();
	_object7.setVisage(7201);
	_object7.setStrip2(4);
	_object7.setPosition(Common::Point(115, 123));
	_object7.setPriority(199);
	_object7.animate(ANIM_MODE_2, NULL);
	_object7._numFrames = 3;

	_object8.postInit();
	_object8.setVisage(7201);
	_object8.setStrip2(6);
	_object8.setPosition(Common::Point(140, 173));
	_object8.setPriority(199);
	_object8.animate(ANIM_MODE_2, NULL);
	_object8._numFrames = 3;

	_object9.postInit();
	_object9.setVisage(7201);
	_object9.setStrip2(7);
	_object9.setPosition(Common::Point(215, 196));
	_object9.setPriority(199);
	_object9.animate(ANIM_MODE_2, NULL);
	_object9._numFrames = 3;

	// Orange swimmer
	g_globals->_player.postInit();
	g_globals->_player.setVisage(7110);
	g_globals->_player.animate(ANIM_MODE_1, NULL);
	g_globals->_player.setObjectWrapper(new SceneObjectWrapper());
	g_globals->_player.setZoom(50);
	g_globals->_player.setPosition(Common::Point(-18, 16));
	g_globals->_player.disableControl();

	_swimmer.postInit();
	_swimmer.setVisage(7101);
	_swimmer.animate(ANIM_MODE_1, NULL);
	_swimmer.setObjectWrapper(new SceneObjectWrapper());
	_swimmer.setZoom(50);
	_swimmer.setPosition(Common::Point(-8, 16));

	setAction(&_action1);
	_soundHandler.play(271);
}

/*--------------------------------------------------------------------------
 * Scene 7300 - Underwater: Lord Poria
 *
 *--------------------------------------------------------------------------*/

void Scene7300::Action1::signal() {
	Scene7300 *scene = (Scene7300 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
	case 1:
	case 3:
		setDelay(30);
		break;
	case 2:
		scene->_stripManager.start(7300, this);
		break;
	case 4: {
		NpcMover *mover = new NpcMover();
		Common::Point pt(102, 122);
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 5:
		g_globals->_player.setStrip(2);
		g_globals->_player.setFrame(1);
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 6:
		g_globals->_player.setStrip(3);
		g_globals->_player._numFrames = 5;
		g_globals->_player.animate(ANIM_MODE_2, this);
		if (RING_INVENTORY._translator._sceneNumber == 1)
			scene->_stripManager.start(7310, this);
		else
			scene->_stripManager.start(7305, this);
		break;
	case 7:
		setDelay(3);
		g_globals->_soundHandler.fadeOut(NULL);
		break;
	case 8:
		g_globals->_sceneManager.changeScene(2280);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene7300::Action2::signal() {
	Scene7300 *scene = (Scene7300 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(5);
		break;
	case 1:
		NpcMover *mover1 = new NpcMover();
		Common::Point pt(g_globals->_randomSource.getRandomNumber(3) + 203, g_globals->_randomSource.getRandomNumber(3) + 96);
		scene->_object3.addMover(mover1, &pt, this);
		_actionIndex = 0;
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene7300::Action3::signal() {
	Scene7300 *scene = (Scene7300 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(5);
		break;
	case 1:
		NpcMover *mover1 = new NpcMover();
		Common::Point pt(g_globals->_randomSource.getRandomNumber(5) + 76, g_globals->_randomSource.getRandomNumber(5) + 78);
		scene->_object1.addMover(mover1, &pt, this);
		_actionIndex = 0;
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene7300::Action4::signal() {
	switch (_actionIndex++) {
	case 0:
		setDelay(5);
		break;
	case 1:
		_actionIndex = 0;
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene7300::dispatch() {
	Scene7300 *scene = (Scene7300 *)g_globals->_sceneManager._scene;
	scene->_object4.setPosition(Common::Point(scene->_object3._position.x + 15, scene->_object3._position.y + 61));
	scene->_object2.setPosition(Common::Point(scene->_object1._position.x + 1, scene->_object1._position.y - 31));

	Scene::dispatch();
}

/*--------------------------------------------------------------------------*/

void Scene7300::postInit(SceneObjectList *OwnerList) {
	loadScene(7300);

	Scene::postInit();
	setZoomPercents(60, 85, 200, 100);

	g_globals->setFlag(52);
	g_globals->setFlag(24);
	g_globals->setFlag(109);

	_stripManager.addSpeaker(&_speakerPOR);
	_stripManager.addSpeaker(&_speakerPOText);
	_stripManager.addSpeaker(&_speakerSKText);
	_stripManager.addSpeaker(&_speakerQU);

	_speakerSKText.setTextPos(Common::Point(100, 20));
	_speakerPOText.setTextPos(Common::Point(100, 160));

	_object4.postInit();
	_object4.setVisage(7311);
	_object4.setStrip(1);
	_object4.setFrame(1);
	_object4.setPosition(Common::Point(218, 157));

	_object3.postInit();
	_object3.setVisage(7311);
	_object3.setStrip(2);
	_object3.setFrame(1);
	_object3.setPosition(Common::Point(203, 96));
	_object3._numFrames = 2;
	_object3._moveDiff = Common::Point(1, 1);
	_object3.animate(ANIM_MODE_8, 0, NULL);
	_object3._moveRate = 2;
	_object3.setAction(&_action2);

	g_globals->_player.postInit();
	g_globals->_player.setVisage(7305);
	g_globals->_player.animate(ANIM_MODE_1, NULL);
	g_globals->_player.setPosition(Common::Point(-100, 100));
	g_globals->_player.disableControl();

	_object1.postInit();
	_object1.setVisage(7312);
	_object1.animate(ANIM_MODE_1, NULL);
	_object1._moveDiff = Common::Point(1, 1);
	_object1.setPosition(Common::Point(76, 78));
	_object1._moveRate = 1;
	_object1.setAction(&_action3);

	_object2.postInit();
	_object2.setVisage(7312);
	_object2.setStrip(2);
	_object2.animate(ANIM_MODE_2, NULL);
	_object2.setPosition(Common::Point(77, 47));
	_object2.fixPriority(190);

	_object5.postInit();
	_object5.setVisage(7300);
	_object5.setPosition(Common::Point(106, 45));
	_object5.animate(ANIM_MODE_2, NULL);
	_object5._numFrames = 5;

	_object6.postInit();
	_object6.setVisage(7300);
	_object6.setStrip2(2);
	_object6.setPosition(Common::Point(283, 193));
	_object6.animate(ANIM_MODE_2, NULL);
	_object6._numFrames = 3;

	_object7.postInit();
	_object7.setVisage(7300);
	_object7.setStrip(4);
	_object7.setPosition(Common::Point(295, 77));
	_object7.animate(ANIM_MODE_2, NULL);
	_object7._numFrames = 3;

	_object8.postInit();
	_object8.setVisage(7300);
	_object8.setStrip(5);
	_object8.setPosition(Common::Point(1, 147));
	_object8.animate(ANIM_MODE_2, NULL);
	_object8._numFrames = 2;

	setAction(&_action1);
	g_globals->_soundHandler.play(272);
}

/*--------------------------------------------------------------------------
 * Scene 7600 - Floating Buildings: Outside
 *
 *--------------------------------------------------------------------------*/

void Scene7600::Action1::signal() {
	switch (_actionIndex++) {
	case 0:
		setDelay(120);
		break;
	case 1: {
		PlayerMover *mover = new PlayerMover();
		Common::Point pt(389, 57);
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 2:
		g_globals->_sceneManager.changeScene(7700);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene7600::Action2::signal() {
	switch (_actionIndex++) {
	case 0:
		setDelay(30);
		break;
	case 1: {
		NpcMover *mover = new NpcMover();
		Common::Point pt(-30, 195);
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 2:
		g_globals->_sceneManager.changeScene(2320);
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene7600::postInit(SceneObjectList *OwnerList) {
	Scene::postInit();
	setZoomPercents(100, 0, 200, 100);

	_object2.postInit();
	_object2.setVisage(7601);
	_object2.setStrip(1);
	_object2.animate(ANIM_MODE_2, NULL);
	_object2.setPosition(Common::Point(48, 135));
	_object2.fixPriority(1);

	_object3.postInit();
	_object3.setVisage(7601);
	_object3.setStrip(2);
	_object3.animate(ANIM_MODE_2, NULL);
	_object3.setPosition(Common::Point(158, 136));
	_object3.fixPriority(1);

	_object4.postInit();
	_object4.setVisage(7601);
	_object4.setStrip(3);
	_object4.animate(ANIM_MODE_2, NULL);
	_object4.setPosition(Common::Point(293, 141));
	_object4.fixPriority(1);

	_object5.postInit();
	_object5.setVisage(7601);
	_object5.setStrip(4);
	_object5.animate(ANIM_MODE_2, NULL);
	_object5.setPosition(Common::Point(405, 143));
	_object5.fixPriority(1);

	_object6.postInit();
	_object6.setVisage(7601);
	_object6.setStrip(5);
	_object6.animate(ANIM_MODE_2, NULL);
	_object6.setPosition(Common::Point(379, 191));
	_object6.fixPriority(1);

	g_globals->_player.postInit();
	g_globals->_player.setVisage(2333);
	g_globals->_player.animate(ANIM_MODE_1, NULL);
	g_globals->_player.setObjectWrapper(new SceneObjectWrapper());
	g_globals->_player.setStrip(1);
	g_globals->_player._moveDiff = Common::Point(16, 16);
	g_globals->_player.changeZoom(-1);
	g_globals->_player.disableControl();

	if (g_globals->_sceneManager._previousScene == 7700) {
		g_globals->_player.setPosition(Common::Point(389, 57));
		setAction(&_action2);
	} else {
		g_globals->_player.setPosition(Common::Point(-50, 195));
		setAction(&_action1);
	}
	_sceneBounds.center(g_globals->_player._position.x, g_globals->_player._position.y);
	loadScene(7600);
	_soundHandler2.play(255);
	_soundHandler1.play(251);
}

/*--------------------------------------------------------------------------
 * Scene 7700 - Floating Buildings: In the lab
 *
 *--------------------------------------------------------------------------*/

void Scene7700::Action1::signal() {
	SceneObjectExt *fmtObj = (SceneObjectExt *) _endHandler;
	switch (_actionIndex++) {
	case 0: {
		PlayerMover *mover1 = new PlayerMover();
		Common::Point pt = Common::Point(fmtObj->_position.x, fmtObj->_position.y + 30);
		g_globals->_player.addMover(mover1, &pt, this);
		break;
	}
	case 1:
		g_globals->_player.checkAngle(fmtObj);
		if (fmtObj->_state == 0)
			fmtObj->animate(ANIM_MODE_5, this);
		else
			fmtObj->animate(ANIM_MODE_6, this);
		break;
	case 2:
		remove();
		break;
	}
}

void Scene7700::Action2::signal() {
	Scene7700 *scene = (Scene7700 *)g_globals->_sceneManager._scene;
	switch (_actionIndex++) {
	case 0:
	case 2:
		setDelay(g_globals->_randomSource.getRandomNumber(60) + 60);
		break;
	case 1:
		scene->_prof.animate(ANIM_MODE_7, 0, NULL);
		setDelay(20);
		break;
	case 3:
		scene->_prof.animate(ANIM_MODE_6, this);
		_actionIndex = 0;
		break;
	}
}

void Scene7700::Action3::signal() {
	Scene7700 *scene = (Scene7700 *)g_globals->_sceneManager._scene;
	switch (_actionIndex++) {
	case 0:
		scene->_object15.animate(ANIM_MODE_5, this);
		break;
	case 1:
		scene->_sceneItem10.remove();
		scene->_object15.remove();
		scene->_sceneHotspot8.remove();
		scene->_sceneHotspot9.remove();
		scene->_object19.remove();
		setDelay(60);
		// No break on purpose!
	case 2:
		scene->_soundHandler.play(260);
		scene->_object8.setVisage(7703);
		scene->_object8.setPosition(Common::Point(177, 97));
		scene->_object8.setStrip2(3);
		scene->_object8.animate(ANIM_MODE_5, this);
		scene->_object8._numFrames = 3;
		break;
	case 3:
		g_globals->_player.enableControl();
		remove();
		break;
	}
}

void Scene7700::Action4::signal() {
	Scene7700 *scene = (Scene7700 *)g_globals->_sceneManager._scene;
	switch (_actionIndex++) {
	case 2:
		scene->_object13.remove();
		// No break on purpose!
	case 0:
		setDelay(3);
		break;
	case 1:
		CursorMan.showMouse(false);
		scene->_object13.postInit();
		scene->_object13.setVisage(7700);
		scene->_object13.setStrip2(7);
		scene->_object13.setPosition(Common::Point(151, 33));
		scene->_object13.animate(ANIM_MODE_5, this);
		break;
	case 3:
		CursorMan.showMouse(true);
		SceneItem::display2(7700, 11);
		g_globals->_player.enableControl();
		remove();
		break;
	}
}

void Scene7700::Action5::signal() {
	Scene7700 *scene = (Scene7700 *)g_globals->_sceneManager._scene;
	switch (_actionIndex++) {
	case 0:
		setDelay(g_globals->_randomSource.getRandomNumber(240));
		break;
	case 1: {
		scene->_cloud.setFrame(g_globals->_randomSource.getRandomNumber(1) + 1);
		scene->_cloud.setPosition(Common::Point(133, 145 + g_globals->_randomSource.getRandomNumber(54)));

		Common::Point pt(360, scene->_cloud._position.y);
		NpcMover *mover = new NpcMover();
		scene->_cloud.addMover(mover, &pt, this);
		_actionIndex = 0;
		break;
	}
	}
}

void Scene7700::Action6::signal() {
	Scene7700 *scene = (Scene7700 *)g_globals->_sceneManager._scene;
	switch (_actionIndex++) {
	case 0:
		setDelay(5);
		break;
	case 1: {
		Common::Point pt(2, 66);
		NpcMover *mover = new NpcMover();
		scene->_easterEgg1.addMover(mover, &pt, this);
		break;
	}
	case 2:
		scene->_easterEgg1.setStrip(2);
		scene->_easterEgg1.setPosition(Common::Point(43, 65));

		scene->_easterEgg2.postInit();
		scene->_easterEgg2.setVisage(7708);
		scene->_easterEgg2.setStrip(3);
		scene->_easterEgg2.setPosition(Common::Point(116, 54));
		scene->_easterEgg2.animate(ANIM_MODE_2, NULL);

		setDelay(120);
		break;
	case 3:
		scene->_easterEgg1.remove();
		scene->_easterEgg2.remove();
		remove();
		break;
	}
}

void Scene7700::SceneHotspot1::doAction(int action) {
	if (action == CURSOR_LOOK)
		SceneItem::display2(7700, 4);
	else
		SceneHotspot::doAction(action);
}

void Scene7700::SceneHotspot2::doAction(int action) {
	if (action == CURSOR_LOOK)
		SceneItem::display2(7700, 6);
	else
		SceneHotspot::doAction(action);
}

void Scene7700::SceneHotspot3::doAction(int action) {
	Scene7700 *scene = (Scene7700 *)g_globals->_sceneManager._scene;

	switch (action) {
	case OBJECT_KEY:
		SceneItem::display2(7702, 3);
		RING_INVENTORY._key._sceneNumber = 7700;
		break;
	case CURSOR_LOOK:
		if (RING_INVENTORY._key._sceneNumber == 7700)
			scene->setAction(&scene->_action4, NULL);
		else
			SceneItem::display2(7700, 53);
		break;
	case CURSOR_USE:
		if (!g_globals->getFlag(78)) {
			scene->_sceneMode = 7712;
			scene->setAction(&scene->_sequenceManager, scene, 7715, NULL);
		} else if (RING_INVENTORY._key._sceneNumber == 7700) {
			g_globals->_player.disableControl();
			scene->_sceneMode = 7705;
			scene->setAction(&scene->_sequenceManager, scene, 7705, &g_globals->_player, NULL);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene7700::SceneHotspot4::doAction(int action) {
	Scene7700 *scene = (Scene7700 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(7700, 12);
		break;
	case CURSOR_USE:
		if (!g_globals->getFlag(78)) {
			scene->_sceneMode = 7712;
			scene->setAction(&scene->_sequenceManager, scene, 7715, NULL);
		} else {
			SceneItem::display2(7700, 12);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene7700::SceneHotspot5::doAction(int action) {
	Scene7700 *scene = (Scene7700 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(7700, 28);
		break;
	case CURSOR_USE:
		if (g_globals->getFlag(78)) {
			if (RING_INVENTORY._paper._sceneNumber == 7700) {
				g_globals->_player.disableControl();
				scene->_sceneMode = 7708;
				scene->setAction(&scene->_sequenceManager, scene, 7708, &g_globals->_player, NULL);
			}
		} else {
			scene->_sceneMode = 7712;
			scene->setAction(&scene->_sequenceManager, scene, 7715, NULL);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene7700::SceneHotspot6::doAction(int action) {
	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(7700, 43);
		break;
	case CURSOR_USE:
		SceneItem::display2(7700, 56);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene7700::SceneItem7::doAction(int action) {
	if (action == CURSOR_LOOK)
		SceneItem::display2(7700, 51);
}

void Scene7700::SceneHotspot8::doAction(int action) {
	Scene7700 *scene = (Scene7700 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(7700, 48);
		break;
	case CURSOR_USE:
		scene->_sceneMode = 7709;
		scene->_soundHandler.play(259);
		scene->_object15.setFrame(scene->_object15.getFrameCount());
		scene->_object15.animate(ANIM_MODE_6, scene);
		if ((scene->_seatCountLeft1 == 2) && (scene->_seatCountLeft2 == 0)) {
			scene->_seatCountRight++;
		} else {
			scene->_seatCountLeft2 = 0;
			scene->_seatCountRight = 0;
			scene->_seatCountLeft1 = 0;
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene7700::SceneHotspot9::doAction(int action) {
	Scene7700 *scene = (Scene7700 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(7700, 48);
		break;
	case CURSOR_USE:
		scene->_sceneMode = 7709;
		scene->_soundHandler.play(259);
		scene->_object15.setFrame(1);
		scene->_object15.animate(ANIM_MODE_5, scene);
		if (scene->_seatCountLeft1 > 2) {
			scene->_seatCountLeft2 = 0;
			scene->_seatCountRight = 0;
			scene->_seatCountLeft1 = 0;
		}

		if (scene->_seatCountRight != 0) {
			if (scene->_seatCountRight != 4) {
				scene->_seatCountLeft2 = 0;
				scene->_seatCountRight = 0;
				scene->_seatCountLeft1 = 0;
			} else {
				scene->_seatCountLeft2++;
				if (scene->_seatCountLeft2 == 3) {
					g_globals->_player.disableControl();
					scene->setAction(&scene->_action3);
				}
			}
		} else {
			scene->_seatCountLeft1++;
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene7700::SceneItem10::doAction(int action) {
}

void Scene7700::Object1::doAction(int action) {
	Scene7700 *scene = (Scene7700 *)g_globals->_sceneManager._scene;

	if (action == CURSOR_LOOK) {
		SceneItem::display2(7700, _lookLineNum);
	} else if (action == CURSOR_USE) {
		if (g_globals->getFlag(78)) {
			g_globals->_player.disableControl();
			scene->setAction(&scene->_action1, this);
		} else {
			scene->_sceneMode = 7712;
			scene->setAction(&scene->_sequenceManager, scene, 7715, NULL);
		}
	} else {
		SceneHotspot::doAction(action);
	}
}

void Scene7700::SceneHotspot11::doAction(int action) {
	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(7700, _lookLineNum);
		break;
	case CURSOR_USE:
		SceneItem::display2(7701, _useLineNum);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene7700::Object1::signal() {
	if (_state == 0) {
		_state = 1;
		SceneItem::display2(7701, _defltLineNum);
	} else {
		_state = 0;
	}
	g_globals->_player.enableControl();
}

void Scene7700::Object3::doAction(int action) {
	Scene7700 *scene = (Scene7700 *)g_globals->_sceneManager._scene;

	if (action == CURSOR_LOOK) {
		SceneItem::display2(7700, 34);
	} else if (action == CURSOR_USE) {
		if (g_globals->getFlag(78)) {
			if (scene->_object3._frame == 1) {
				g_globals->_player.disableControl();
				scene->_sceneMode = 7707;
				scene->setAction(&scene->_sequenceManager, scene, 7707, &g_globals->_player, this, NULL);
			} else {
				SceneItem::display2(7700, 60);
			}
		} else {
			scene->_sceneMode = 7712;
			scene->setAction(&scene->_sequenceManager, scene, 7715, NULL);
		}
	} else {
		SceneHotspot::doAction(action);
	}
}

void Scene7700::Object7::doAction(int action) {
	Scene7700 *scene = (Scene7700 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		if (g_globals->getFlag(78))
			SceneItem::display2(7700, 45);
		else
			SceneItem::display2(7700, 44);
		break;
	case CURSOR_USE:
		if (g_globals->getFlag(78)) {
			SceneItem::display2(7701, 41);
		} else {
			scene->_sceneMode = 7712;
			scene->setAction(&scene->_sequenceManager, scene, 7714, NULL);
		}
		break;
	case CURSOR_TALK:
		if (g_globals->getFlag(78)) {
			SceneItem::display2(7702, 1);
		} else {
			g_globals->_player.disableControl();
			if (_state == 0) {
				_state = 1;
				scene->_sceneMode = 7703;
				scene->setAction(&scene->_sequenceManager, scene, 7703, NULL);
			} else {
				scene->_sceneMode = 7712;
				scene->setAction(&scene->_sequenceManager, scene, 7712, NULL);
			}
		}
		break;
	case OBJECT_STUNNER:
		if (!g_globals->getFlag(78)) {
			g_globals->_soundHandler.stop();
			g_globals->setFlag(78);
			setAction(NULL);
			g_globals->_player.disableControl();
			scene->_sceneMode = 7704;
			scene->setAction(&scene->_sequenceManager, scene, 7704, &g_globals->_player, this, NULL);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene7700::Object8::doAction(int action) {
	Scene7700 *scene = (Scene7700 *)g_globals->_sceneManager._scene;

	if ((action == CURSOR_LOOK) || (action == CURSOR_USE)) {
		if (_strip == 3) {
			scene->_object9.postInit();
			scene->_object9.setVisage(7701);
			scene->_object9.setStrip2(3);
			scene->_object9.setPosition(Common::Point(91, 166));
			scene->_object9.fixPriority(200);

			scene->_object14.postInit();
			scene->_object14.setVisage(7701);
			scene->_object14.setStrip(2);
			scene->_object14.fixPriority(250);
			scene->_object14.setPosition(Common::Point(139, 151));

			scene->_gfxButton.setText(EXIT_MSG);
			scene->_gfxButton._bounds.center(140, 189);
			scene->_gfxButton.draw();

			g_globals->_sceneItems.push_front(&scene->_sceneItem10);
			g_globals->_sceneItems.push_front(&scene->_object9);
			g_globals->_player._canWalk = false;
		} else if (g_globals->getFlag(78)) {
			scene->_object15.postInit();
			scene->_object15.setVisage(7701);
			scene->_object15.setPosition(Common::Point(140, 165));
			scene->_object15.fixPriority(200);

			scene->_gfxButton.setText(EXIT_MSG);
			scene->_gfxButton._bounds.center(140, 186);
			scene->_gfxButton.draw();
			scene->_gfxButton._bounds.expandPanes();

			scene->_object19.postInit();
			scene->_object19.setVisage(7700);
			scene->_object19.setStrip(6);
			scene->_object19.setPosition(Common::Point(140, 192));

			g_globals->_sceneItems.push_front(&scene->_sceneItem10);
			g_globals->_sceneItems.push_front(&scene->_sceneHotspot8);
			g_globals->_sceneItems.push_front(&scene->_sceneHotspot9);
			g_globals->_events.setCursor(CURSOR_WALK);
			g_globals->_player._canWalk = false;
		} else {
			scene->setAction(&scene->_sequenceManager, scene, 7715, NULL);
		}
	} else {
		SceneHotspot::doAction(action);
	}
}

void Scene7700::Object9::doAction(int action) {
	Scene7700 *scene = (Scene7700 *)g_globals->_sceneManager._scene;

	switch (action) {
		case CURSOR_LOOK:
			SceneItem::display2(7700, 49);
			break;
		case CURSOR_USE:
			SceneItem::display2(7701, 42);
			break;
		case CURSOR_TALK:
			SceneItem::display2(7702, 4);
			break;
		case OBJECT_KEY:
			if (_frame == 1) {
				if (!g_globals->getFlag(80)) {
					scene->_object10.postInit();
					scene->_object10.setVisage(7701);
					scene->_object10.setStrip(4);
					scene->_object10.setPosition(Common::Point(159, 136));
					g_globals->_sceneItems.push_front(&scene->_object10);
					scene->_object10.fixPriority(240);
				}

				scene->_soundHandler.play(262);
				scene->_object14.animate(ANIM_MODE_5, NULL);
			}
			g_globals->_events.setCursor(CURSOR_WALK);
			break;
		default:
			SceneHotspot::doAction(action);
	}
}

void Scene7700::Object10::doAction(int action) {
	Scene7700 *scene = (Scene7700 *)g_globals->_sceneManager._scene;

	if (action == CURSOR_LOOK) {
		SceneItem::display2(7700, 50);
	} else if (action == CURSOR_USE) {
		g_globals->_player._canWalk = true;
		RING_INVENTORY._translator._sceneNumber = 1;
		g_globals->setFlag(80);
		scene->_sceneItem10.remove();
		scene->_gfxButton._bounds.expandPanes();
		scene->_object14.remove();
		scene->_object9.remove();
		remove();
	} else {
		SceneHotspot::doAction(action);
	}
}

void Scene7700::Object11::doAction(int action) {
	Scene7700 *scene = (Scene7700 *)g_globals->_sceneManager._scene;

	switch (action) {
		case OBJECT_SCANNER:
			if (_frame != 1)
				SceneItem::display2(7701, 44);
			else
				SceneItem::doAction(action);
			break;
		case CURSOR_LOOK:
			if (_frame != 1)
				SceneItem::display2(7700, 9);
			else
				SceneItem::display2(7700, 52);
			break;
		case CURSOR_USE:
			if (_frame != 1) {
				SceneItem::display2(7701, 8);
			} else {
				g_globals->setFlag(49);
				g_globals->_player.disableControl();
				scene->_sceneMode = 7706;
				scene->setAction(&scene->_sequenceManager, scene, 7706, &g_globals->_player, this, NULL);
			}
			break;
		case OBJECT_EMPTY_JAR:
			RING_INVENTORY._emptyJar._sceneNumber = 0;
			RING_INVENTORY._jar._sceneNumber = 1;
			g_globals->_player.disableControl();
			scene->_sceneMode = 7710;
			scene->setAction(&scene->_sequenceManager, scene, 7710, &g_globals->_player, NULL);
			break;
		default:
			SceneHotspot::doAction(action);
	}
}

void Scene7700::Object12::doAction(int action) {
	Scene7700 *scene = (Scene7700 *)g_globals->_sceneManager._scene;

	if (action == CURSOR_LOOK) {
		SceneItem::display2(7700, 15);
	} else if (action == CURSOR_USE) {
		if (g_globals->getFlag(78)) {
			scene->_sceneMode = 7713;
			scene->setAction(&scene->_sequenceManager, scene, 7713, &g_globals->_player, NULL);
		} else {
			scene->_sceneMode = 7712;
			scene->setAction(&scene->_sequenceManager, scene, 7715, NULL);
		}
	} else {
		SceneHotspot::doAction(action);
	}
}

void Scene7700::signal() {
	switch (_sceneMode) {
	case 7701:
		g_globals->_player.fixPriority(-1);
		g_globals->_player.setStrip2(-1);
		if (g_globals->getFlag(78)) {
			g_globals->_player.enableControl();
		} else {
			_sceneMode = 7711;
			setAction(&_sequenceManager, this, 7711, NULL);
		}
		break;
	case 7702:
		g_globals->_soundHandler.fadeOut(NULL);
		g_globals->_sceneManager.changeScene(7600);
		break;
	case 7703:
	case 7706:
	case 7707:
	case 7711:
	case 7712:
		g_globals->_player.enableControl();
		break;
	case 7704:
		g_globals->_soundHandler.play(256);
		_prof.setStrip2(4);
		_prof.setFrame2(1);
		_prof.setPosition(Common::Point(159, 87));
		g_globals->_player.enableControl();
		break;
	case 7705:
		RING_INVENTORY._key._sceneNumber = 1;
		g_globals->_player.enableControl();
		break;
	case 7708:
		RING_INVENTORY._paper._sceneNumber = 1;
		g_globals->_player.enableControl();
		break;
	case 7709:
		g_globals->_events.setCursor(CURSOR_USE);
		break;
	case 7710:
		g_globals->_player.enableControl();
		SceneItem::display2(7700, 62);
		break;
	case 7713:
		_emptyJar.remove();
		RING_INVENTORY._emptyJar._sceneNumber = 1;
		break;
	default:
		break;
	}
}

void Scene7700::process(Event &event) {
	Scene::process(event);

	if (contains<SceneItem *>(g_globals->_sceneItems, &_sceneItem10)) {
		if (_gfxButton.process(event)) {
			_sceneItem10.remove();
			_object15.remove();
			_object9.remove();
			if (g_globals->_sceneObjects->contains(&_object10))
				_object10.remove();
			if (g_globals->_sceneObjects->contains(&_object14))
				_object14.remove();
			_object19.remove();
			_gfxButton._bounds.expandPanes();
			g_globals->_player._canWalk = true;
		}
	}
	if ((event.eventType == EVENT_KEYPRESS) && (event.kbd.keycode == Common::KEYCODE_p)) {
		event.handled = true;
		if (!g_globals->_sceneObjects->contains(&_easterEgg1)) {
			_easterEgg1.postInit();
			_easterEgg1.setVisage(7708);
			_easterEgg1.setPosition(Common::Point(163, 50));
			_easterEgg1.fixPriority(1);
			_easterEgg1.animate(ANIM_MODE_2, NULL);
			_easterEgg1.setAction(&_action6);
		}
	}
}

void Scene7700::dispatch() {
	if ((g_globals->_sceneRegions.indexOf(g_globals->_player._position) == 6) || (g_globals->_player._position.x < 10))
		g_globals->_player.changeZoom(100 - ((g_globals->_player._position.y - 68) / 2));
	else
		g_globals->_player.changeZoom(-1);

	if ((_action == 0) && (g_globals->_sceneRegions.indexOf(g_globals->_player._position) == 30)) {
		g_globals->_player.disableControl();
		_sceneMode = 7702;
		setAction(&_sequenceManager, this, 7702, &g_globals->_player, NULL);
	}
	Scene::dispatch();
}

void Scene7700::postInit(SceneObjectList *OwnerList) {
	loadScene(7700);
	Scene::postInit();
	setZoomPercents(100, 80, 200, 100);
	g_globals->setFlag(53);
	_seatCountLeft2 = 0;
	_seatCountRight = 0;
	_seatCountLeft1 = 0;

	_stripManager.addSpeaker(&_speakerEText);
	_stripManager.addSpeaker(&_speakerQText);
	_speakerQText._npc = &g_globals->_player;
	_speakerEText._npc = &_prof;

	g_globals->_player.postInit();
	g_globals->_player.setVisage(4201);
	g_globals->_player.animate(ANIM_MODE_1, NULL);
	SceneObjectWrapper *wrapper = new SceneObjectWrapper();
	g_globals->_player.setObjectWrapper(wrapper);
	g_globals->_player.setPosition(Common::Point(-19, 68));
	g_globals->_player.setStrip2(7);
	g_globals->_player.fixPriority(95);
	g_globals->_player.changeZoom(80);
	g_globals->_player._moveDiff.x = 6;
	g_globals->_player._moveDiff.y = 3;
	g_globals->_player.disableControl();

	_prof.postInit();
	_prof.setVisage(7706);

	if (g_globals->getFlag(78)) {
		_prof.setStrip2(4);
		_prof.fixPriority(80);
		_prof.setPosition(Common::Point(159, 87));
	} else {
		_prof.setPosition(Common::Point(203, 87));
		_prof.setStrip2(2);
		_prof._numFrames = 6;
		_prof.setAction(&_action2);
	}

	_cloud.postInit();
	_cloud.setVisage(7700);
	_cloud.setStrip2(5);
	_cloud.fixPriority(1);
	_cloud.setPosition(Common::Point(133, 160));
	_cloud._moveDiff.x = 1;
	_cloud._moveRate = 7;
	_cloud.setAction(&_action5);

	_object1.postInit();
	_object1.setVisage(7700);
	_object1.setPosition(Common::Point(184, 61));
	_object1._lookLineNum = 18;
	_object1._defltLineNum = 16;

	_object2.postInit();
	_object2.setVisage(7700);
	_object2.setPosition(Common::Point(184, 70));
	_object2.fixPriority(60);
	_object2._lookLineNum = 19;
	_object2._defltLineNum = 17;

	_object3.postInit();
	_object3.setVisage(7703);
	_object3.setPosition(Common::Point(288, 36));
	_object3.setStrip(2);

	_object4.postInit();
	_object4.setVisage(7700);
	_object4.setPosition(Common::Point(268, 59));
	_object4.setStrip(2);
	_object4._lookLineNum = 37;
	_object4._defltLineNum = 35;

	_object5.postInit();
	_object5.setVisage(7700);
	_object5.setPosition(Common::Point(268, 67));
	_object5.fixPriority(58);
	_object5.setStrip2(3);
	_object5._lookLineNum = 38;
	_object5._defltLineNum = 36;

	_object6.postInit();
	_object6.setVisage(7700);
	_object6.setPosition(Common::Point(268, 75));
	_object6.fixPriority(57);
	_object6.setStrip2(4);
	_object6._lookLineNum = 40;
	_object6._defltLineNum = 43;

	_object8.postInit();
	_object8.setVisage(7703);
	_object8.setPosition(Common::Point(203, 91));
	_object8.setStrip2(4);
	_object8.fixPriority(86);

	_sceneHotspot8.setBounds(82, 141, 161, 92);
	_sceneHotspot9.setBounds(82, 187, 161, 141);

	_cork.postInit();
	_cork.setVisage(7703);
	_cork.setPosition(Common::Point(32, 128));

	if (g_globals->getFlag(49))
		_cork.setFrame(_cork.getFrameCount());

	if (RING_INVENTORY._emptyJar._sceneNumber == 7700) {
		_emptyJar.postInit();
		_emptyJar.setVisage(7700);
		_emptyJar.setStrip(8);
		_emptyJar.setPosition(Common::Point(189, 48));
		g_globals->_sceneItems.addItems(&_emptyJar, NULL);
	}
	_sceneHotspot1._sceneRegionId = 28;
	_sceneHotspot2._sceneRegionId = 6;
	_sceneHotspot3._sceneRegionId = 10;
	_sceneHotspot4._sceneRegionId = 11;
	_sceneHotspot5._sceneRegionId = 9;
	_sceneHotspot6._sceneRegionId = 7;

	_sceneItem7.setBounds(0, 320, 200, 0);
	_sceneItem10.setBounds(0, 320, 200, 0);

	_sceneHotspot11._sceneRegionId = 57;
	_sceneHotspot11._useLineNum = 0;
	_sceneHotspot11._lookLineNum = 0;
	_sceneHotspot12._sceneRegionId = 2;
	_sceneHotspot12._useLineNum = 2;
	_sceneHotspot12._lookLineNum = 2;
	_sceneHotspot13._sceneRegionId = 12;
	_sceneHotspot13._useLineNum = 3;
	_sceneHotspot13._lookLineNum = 3;
	_sceneHotspot14._sceneRegionId = 18;
	_sceneHotspot14._useLineNum = 4;
	_sceneHotspot14._lookLineNum = 5;
	_sceneHotspot15.setBounds(0, 55, 50, 8);
	_sceneHotspot15._useLineNum = 6;
	_sceneHotspot15._lookLineNum = 7;
	_sceneHotspot16.setBounds(0, 130, 34, 103);
	_sceneHotspot16._useLineNum = 7;
	_sceneHotspot16._lookLineNum = 8;
	_sceneHotspot17.setBounds(41, 180, 46, 170);
	_sceneHotspot17._useLineNum = 11;
	_sceneHotspot17._lookLineNum = 13;
	_sceneHotspot18.setBounds(38, 187, 46, 180);
	_sceneHotspot18._useLineNum = 12;
	_sceneHotspot18._lookLineNum = 14;
	_sceneHotspot19._sceneRegionId = 3;
	_sceneHotspot19._useLineNum = 14;
	_sceneHotspot19._lookLineNum = 16;
	_sceneHotspot20._sceneRegionId = 14;
	_sceneHotspot20._useLineNum = 15;
	_sceneHotspot20._lookLineNum = 17;
	_sceneHotspot21.setBounds(9, 215, 26, 210);
	_sceneHotspot21._useLineNum = 18;
	_sceneHotspot21._lookLineNum = 20;
	_sceneHotspot22.setBounds(10, 221, 26, 215);
	_sceneHotspot22._useLineNum = 19;
	_sceneHotspot22._lookLineNum = 21;
	_sceneHotspot23.setBounds(6, 230, 26, 225);
	_sceneHotspot23._useLineNum = 20;
	_sceneHotspot23._lookLineNum = 22;
	_sceneHotspot24._sceneRegionId = 13;
	_sceneHotspot24._useLineNum = 21;
	_sceneHotspot24._lookLineNum = 23;
	_sceneHotspot25._sceneRegionId = 21;
	_sceneHotspot25._useLineNum = 22;
	_sceneHotspot25._lookLineNum = 24;
	_sceneHotspot26._sceneRegionId = 19;
	_sceneHotspot26._useLineNum = 23;
	_sceneHotspot26._lookLineNum = 25;
	_sceneHotspot27._sceneRegionId = 27;
	_sceneHotspot27._useLineNum = 24;
	_sceneHotspot27._lookLineNum = 26;
	_sceneHotspot28._sceneRegionId = 15;
	_sceneHotspot28._useLineNum = 25;
	_sceneHotspot28._lookLineNum = 27;
	_sceneHotspot29._sceneRegionId = 26;
	_sceneHotspot29._useLineNum = 27;
	_sceneHotspot29._lookLineNum = 29;
	_sceneHotspot30.setBounds(0, 317, 34, 310);
	_sceneHotspot30._useLineNum = 28;
	_sceneHotspot30._lookLineNum = 30;
	_sceneHotspot31._sceneRegionId = 17;
	_sceneHotspot31._useLineNum = 29;
	_sceneHotspot31._lookLineNum = 31;
	_sceneHotspot32._sceneRegionId = 25;
	_sceneHotspot32._useLineNum = 30;
	_sceneHotspot32._lookLineNum = 32;
	_sceneHotspot33._sceneRegionId = 5;
	_sceneHotspot33._useLineNum = 31;
	_sceneHotspot33._lookLineNum = 33;
	_sceneHotspot34.setBounds(42, 292, 48, 281);
	_sceneHotspot34._useLineNum = 32;
	_sceneHotspot34._lookLineNum = 35;
	_sceneHotspot35._sceneRegionId = 24;
	_sceneHotspot35._useLineNum = 38;
	_sceneHotspot35._lookLineNum = 41;
	_sceneHotspot36._sceneRegionId = 1;
	_sceneHotspot36._useLineNum = 39;
	_sceneHotspot36._lookLineNum = 42;

	g_globals->_sceneItems.addItems(&_prof, &_sceneHotspot35, &_object8, &_sceneHotspot34, &_sceneHotspot33, &_sceneHotspot32, NULL);
	g_globals->_sceneItems.addItems(&_sceneHotspot31, &_sceneHotspot30, &_sceneHotspot29, &_sceneHotspot5, &_sceneHotspot28, &_sceneHotspot27, NULL);
	g_globals->_sceneItems.addItems(&_sceneHotspot26, &_sceneHotspot25, &_sceneHotspot24, &_sceneHotspot23, &_sceneHotspot22, &_sceneHotspot21, NULL);
	g_globals->_sceneItems.addItems(&_sceneHotspot20, &_sceneHotspot19, &_sceneHotspot18, &_sceneHotspot17, &_sceneHotspot4, &_sceneHotspot3, NULL);
	g_globals->_sceneItems.addItems(&_sceneHotspot16, &_sceneHotspot15, &_sceneHotspot2, &_sceneHotspot14, &_cork, &_sceneHotspot1, NULL);
	g_globals->_sceneItems.addItems(&_sceneHotspot13, &_sceneHotspot12, &_sceneHotspot11, &_object2, &_object1, &_object3, NULL);
	g_globals->_sceneItems.addItems(&_object6, &_object5, &_object4, &_sceneHotspot6, &_sceneHotspot36, &_sceneItem7, NULL);

	_sceneMode = 7701;
	setAction(&_sequenceManager, this, 7701, &g_globals->_player, NULL);
	_soundHandler.play(256);
}

Scene7700::Scene7700() {
	_object1._state = 0;
	_object2._state = 0;
	_object4._state = 0;
	_object5._state = 0;
	_object6._state = 0;
	_prof._state = 0;
}

void Scene7700::synchronize(Serializer &s) {
	Scene::synchronize(s);
	if (s.getVersion() >= 3) {
		s.syncAsSint16LE(_seatCountLeft1);
		s.syncAsSint16LE(_seatCountRight);
		s.syncAsSint16LE(_seatCountLeft2);
	}
}

} // End of namespace Ringworld

} // End of namespace TsAGE

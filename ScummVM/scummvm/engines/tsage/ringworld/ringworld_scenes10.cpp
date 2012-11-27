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

#include "tsage/ringworld/ringworld_scenes10.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace TsAGE {

namespace Ringworld {

Scene2::Scene2() : Scene() {
	_sceneState = 0;
}

/*--------------------------------------------------------------------------*/

void Object9350::postInit(SceneObjectList *OwnerList) {
	//SceneObject::postInit(&g_globals->_sceneManager._bgSceneObjects);
	SceneObject::postInit(OwnerList);
}

void Object9350::draw() {
	reposition();
	Rect destRect = _bounds;
	destRect.translate(-g_globals->_sceneOffset.x, -g_globals->_sceneOffset.y);
	Region *priorityRegion = g_globals->_sceneManager._scene->_priorities.find(g_globals->_sceneManager._scene->_stripManager._stripNum);
	GfxSurface frame = getFrame();
	g_globals->gfxManager().copyFrom(frame, destRect, priorityRegion);
}

/*--------------------------------------------------------------------------
 * Scene 9100 - Near beach: Slave washing clothes
 *
 *--------------------------------------------------------------------------*/
// Slave hotspot
void Scene9100::SceneHotspot1::doAction(int action) {
	Scene9100 *scene = (Scene9100 *)g_globals->_sceneManager._scene;

	if (action == CURSOR_TALK) {
		if (g_globals->getFlag(23)) {
			g_globals->_player.disableControl();
			scene->_sceneMode = 9104;
		} else {
			g_globals->setFlag(23);
			g_globals->_player.disableControl();
			scene->_sceneMode = 9105;
		}
		scene->setAction(&scene->_sequenceManager, scene, scene->_sceneMode, &g_globals->_player, &scene->_object5, &scene->_object6, NULL);
	} else {
		NamedHotspot::doAction(action);
	}
}

void Scene9100::dispatch() {
	Scene9100 *scene = (Scene9100 *)g_globals->_sceneManager._scene;

	if (!_action) {
		if (g_globals->_player._position.x < 25) {
			g_globals->_player.disableControl();
			if (!g_globals->getFlag(23) || g_globals->getFlag(11))
				_sceneMode = 9106;
			else {
				_sceneMode = 9108;
				g_globals->setFlag(11);
			}

			scene->setAction(&scene->_sequenceManager, scene, scene->_sceneMode, &g_globals->_player, NULL);
		}
	} else {
		Scene::dispatch();
	}
}

void Scene9100::signal() {
	Scene9100 *scene = (Scene9100 *)g_globals->_sceneManager._scene;

	switch (scene->_sceneMode) {
	case 9102:
	case 9106:
	case 9108:
		g_globals->_sceneManager.changeScene(9150);
		break;
	case 9105:
		_hotspotCleanedClothes.remove();
	// No break on purpose
	case 9103:
	case 9104:
	case 9107:
	case 9109:
	default:
		g_globals->_player.enableControl();
		break;
	}
}

void Scene9100::postInit(SceneObjectList *OwnerList) {
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);
	_object1.postInit();
	_object1.setVisage(9100);
	_object1._strip = 1;
	_object1._numFrames = 6;
	_object1.setPosition(Common::Point(279, 132));
	_object1.animate(ANIM_MODE_2, NULL);
	_object1.fixPriority(10);

	g_globals->_player.postInit();

	_object2.postInit();
	_object2.hide();

	_object3.postInit();
	_object3.hide();

	_object4.postInit();
	_object4.hide();

	_object5.postInit();
	_object5.hide();

	if (!g_globals->getFlag(23)) {
		_object6.postInit();
		_object6.setVisage(9111);
		_object6.setStrip(6);
		_object6.setFrame(1);
		_object6.setPosition(Common::Point(138, 166));
		_hotspotCleanedClothes.setDetails(145, 125, 166, 156, 9100, 40, 43);
	}
	_hotspotSlave.setDetails(140, 176, 185, 215, 9100, 36, 37);
	_hotspotSoiledClothes.setDetails(161, 138, 182, 175, 9100, 38, 39);
	_hotspotIsland.setDetails(37, 196, 47, 320, 9100, 44, -1);
	_hotspotBoulders.setDetails(69, 36, 121, 272, 9100, 45, 46);
	_hotspotTrees.setDetails(127, 0, 200, 52, 9100, 47, 48);

	g_globals->_soundHandler.play(251);
	if (g_globals->_sceneManager._previousScene == 9150) {
		if (g_globals->getFlag(20)) {
			g_globals->_player.disableControl();
			if (g_globals->getFlag(11))
				_sceneMode = 9107;
			else
				_sceneMode = 9109;
			setAction(&_sequenceManager, this, _sceneMode, &g_globals->_player, &_object5, NULL);
		} else {
			_sceneMode = 9103;
			g_globals->_player.disableControl();
			setAction(&_sequenceManager, this, _sceneMode, &g_globals->_player, &_object2, &_object3, &_object4, &_object5, NULL);
			g_globals->setFlag(20);
		}
	} else {
		_sceneMode = 9102;
		g_globals->_player.disableControl();
		setAction(&_sequenceManager, this, _sceneMode, &g_globals->_player, &_object2, &_object3, &_object4, &_object5, NULL);
	}
}

/*--------------------------------------------------------------------------
 * Scene 9150 - Castle: Outside the bulwarks
 *
 *--------------------------------------------------------------------------*/
void Scene9150::Object3::signal() {
	switch (_signalFlag++) {
	case 0:
		_timer = 10 + g_globals->_randomSource.getRandomNumber(90);
		break;
	default:
		animate(ANIM_MODE_5, this);
		_signalFlag = 0;
		break;
	}
}

void Scene9150::Object3::dispatch() {
	SceneObject::dispatch();
	if ((_timer != 0) && (--_timer == 0))
		signal();
}

void Scene9150::signal() {
	switch (_sceneMode) {
	case 9151:
	case 9157:
		g_globals->_sceneManager.changeScene(9100);
		break;
	case 9153:
		g_globals->_sceneManager.changeScene(9300);
		break;
	case 9152:
	case 9155:
	case 9156:
		g_globals->_player.enableControl();
		break;
	case 9154:
	default:
		break;
	}
}

void Scene9150::dispatch() {

	if ((_sceneState != 0) && (_sceneBounds.left == 0)) {
		_object3._timer = 0;
		_sceneState = 0;
		_object3.setAction(&_sequenceManager2, NULL, 9154, &_object3, NULL);
		_sceneHotspot10.remove();
	}

	if (_action) {
		_action->dispatch();
	} else {
		if (g_globals->_player._position.x >= 160) {
			if (g_globals->_player._position.x > 630) {
				g_globals->_player.disableControl();
				_sceneMode = 9157;
				setAction(&_sequenceManager1, this, _sceneMode, &g_globals->_player, NULL);
			}
		} else {
			g_globals->_player.disableControl();
			if (g_globals->getFlag(11)) {
				g_globals->_soundHandler.play(286);
				_sceneMode = 9153;
			} else {
				_sceneMode = 9156;
			}
			setAction(&_sequenceManager1, this, _sceneMode, &g_globals->_player, NULL);
		}
	}
}

void Scene9150::postInit(SceneObjectList *OwnerList) {
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);
	g_globals->_player.postInit();

	_object3.postInit();
	_sceneState = 1;
	_object3.setVisage(9151);
	_object3._strip = 1;
	_object3._frame = 1;
	_object3.setPosition(Common::Point(312, 95));
	_object3.signal();

	_sceneHotspot1.setDetails(0, 0, 200, 94, 9150, 46, -1);
	_sceneHotspot2.setDetails(51, 90, 118, 230, 9150, 47, -1);
	_sceneHotspot3.setDetails(182, 104, 200, 320, 9150, 48, 49);
	_sceneHotspot4.setDetails(103, 292, 152, 314, 9150, 50, 51);
	_sceneHotspot5.setDetails(115, 350, 160, 374, 9150, 52, 53);
	_sceneHotspot6.setDetails(0, 471, 200, 531, 9150, 54, 55);
	_sceneHotspot7.setDetails(170, 320, 185, 640, 9150, 56, -1);
	_sceneHotspot9.setDetails(157, 107, 186, 320, 9150, 56, -1);
	_sceneHotspot8.setDetails(133, 584, 142, 640, 9150, 57, -1);
	_sceneHotspot10.setDetails(83, 304, 103, 323, 9150, 58, 59);

	g_globals->_soundHandler.play(285);
	g_globals->_player.disableControl();

	if (g_globals->getFlag(20)) {
		// Walking alone
		g_globals->_scrollFollower = &g_globals->_player;
		if (g_globals->getFlag(11))
			// Hero wearing peasan suit
			_sceneMode = 9155;
		else
			// Hero wearing Purple suit
			_sceneMode = 9152;
		setAction(&_sequenceManager1, this, _sceneMode, &g_globals->_player, NULL);
	} else {
		// Walking with the tiger
		_sceneMode = 9151;
		_object2.postInit();
		_object2.hide();
		_object1.postInit();
		setAction(&_sequenceManager1, this, _sceneMode, &g_globals->_player, &_object1, &_object2, NULL);
	}
}

/*--------------------------------------------------------------------------
 * Scene 9200 - Castle: Near the fountain
 *
 *--------------------------------------------------------------------------*/
void Scene9200::SceneHotspot1::doAction(int action) {
	Scene9200 *scene = (Scene9200 *)g_globals->_sceneManager._scene;

	if (action == OBJECT_TUNIC) {
		g_globals->_player.disableControl();
		if (g_globals->getFlag(93)) {
			scene->_sceneState = 9214;
			scene->setAction(&scene->_sequenceManager, scene, 9214, &g_globals->_player, &scene->_object2, NULL);
		} else {
			g_globals->setFlag(93);
			scene->_sceneState = 9213;
			scene->setAction(&scene->_sequenceManager, scene, 9213, &g_globals->_player, &scene->_object2, NULL);
		}
	} else if (action <= 100) {
		g_globals->_player.disableControl();
		scene->_sceneState = 9214;
		scene->setAction(&scene->_sequenceManager, scene, 9214, &g_globals->_player, &scene->_object2, NULL);
	} else {
		NamedHotspot::doAction(action);
	}
}

void Scene9200::signal() {
	switch (_sceneState++) {
	case 9207:
		g_globals->_sceneManager.changeScene(9700);
		break;
	case 9208:
	case 9211:
	case 9212:
		g_globals->_sceneManager.changeScene(9500);
		break;
	case 9209:
		g_globals->_sceneManager.changeScene(9360);
		break;
	case 9210:
		_hotspot1.remove();
	// No break on purpose
	case 9201:
	case 9202:
	case 9203:
	case 9204:
	case 9205:
	case 9206:
	default:
		g_globals->_player.enableControl();
		break;
	}
}

void Scene9200::process(Event &event) {
	Scene::process(event);
}

void Scene9200::dispatch() {
//	Rect rect9200 = Rect(320, 175, 250, 154);
	Rect rect9200 = Rect(250, 154, 320, 175);

	if (_action) {
		_action->dispatch();
	} else {
		if ( (g_globals->_player._position.x <= 0) || ((g_globals->_player._position.x < 100) && (g_globals->_player._position.y > 199))) {
				g_globals->_player.disableControl();
				_sceneState = 9209;
				setAction(&_sequenceManager, this, 9209, &g_globals->_player, &_object2, &_object3, NULL);
		} else {
			if (rect9200.contains(g_globals->_player._position)) {
				if (g_globals->getFlag(93)) {
					if (g_globals->getFlag(86)) {
						_sceneState = 9215;
						setAction(&_sequenceManager, this, 9215, &g_globals->_player, &_object2, &_object3, NULL);
					} else {
						_sceneState = 9208;
						setAction(&_sequenceManager, this, 9208, &g_globals->_player, &_object2, &_object3, NULL);
					}
				} else {
					g_globals->_player.disableControl();
					_sceneState = 9204;
					setAction(&_sequenceManager, this, 9204, &g_globals->_player, &_object2, &_object3, NULL);
				}
			} else {
				if (g_globals->_player._position.y < 140) {
					g_globals->_player.disableControl();
					_sceneState = 9207;
					setAction(&_sequenceManager, this, 9207, &g_globals->_player, &_object2, &_object3, NULL);
				}
			}
		}
	}
}

void Scene9200::postInit(SceneObjectList *OwnerList) {
	Scene::postInit();
	setZoomPercents(130, 50, 200, 150);

	g_globals->_player.postInit();
	_object3.postInit();
	_object3.hide();
	_object1.postInit();
	// Water animation
	_object1.setVisage(9200);
	_object1._strip = 3;
	_object1.animate(ANIM_MODE_2, NULL);
	_object1.setPosition(Common::Point(132, 114));
	_object1.fixPriority(140);
	_soundHandler.play(297);

	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerGR);
	_stripManager.addSpeaker(&_speakerGText);

	if (!g_globals->getFlag(86)) {
		_object2.postInit();
		_hotspot1.setDetails(96, 194, 160, 234, 9200, 29, 31);
	}
	_hotspot2.setDetails(164, 0, 200, 282, 9200, 0, 1);
	_hotspot3.setDetails(140, 39, 165, 153, 9200, 2, 3);
	_hotspot4.setDetails(92, 122, 139, 152, 9200, 4, 5);
	_hotspot5.setDetails(33, 20, 142, 115, 9200, 6, 7);
	_hotspot6.setDetails(104, 235, 153, 265, 9200, 8, 9);
	_hotspot7.setDetails(107, 262, 153, 286, 9200, 10, 11);
	_hotspot8.setDetails(69, 276, 164, 320, 9200, 12, 13);

	g_globals->_events.setCursor(CURSOR_WALK);
	g_globals->_player.disableControl();

	switch (g_globals->_sceneManager._previousScene) {
	case 9500:
		if (g_globals->getFlag(85)) {
			if (RING_INVENTORY._helmet._sceneNumber == 1) {
				g_globals->setFlag(86);
				g_globals->_player.disableControl();
				_sceneState = 9210;
				setAction(&_sequenceManager, this, 9210, &g_globals->_player, &_object2, &_object3, NULL);
			} else {
				g_globals->_player.disableControl();
				_sceneState = 9212;
				setAction(&_sequenceManager, this, 9212, &g_globals->_player, &_object2, &_object3, NULL);
			}
		} else {
			if (RING_INVENTORY._helmet._sceneNumber == 1) {
				g_globals->_player.disableControl();
				_sceneState = 9211;
				setAction(&_sequenceManager, this, 9211, &g_globals->_player, &_object2, &_object3, NULL);
			} else {
				g_globals->_player.disableControl();
				_sceneState = 9202;
				setAction(&_sequenceManager, this, 9202, &g_globals->_player, &_object2, &_object3, NULL);
			}
		}
		break;
	case 9700:
		if (g_globals->getFlag(86)) {
			_sceneState = 9206;
			setAction(&_sequenceManager, this, 9206, &g_globals->_player, &_object2, &_object3, NULL);
		} else {
			_sceneState = 9203;
			setAction(&_sequenceManager, this, 9203, &g_globals->_player, &_object2, &_object3, NULL);
		}
		break;
	case 9360:
	default:
		if (g_globals->getFlag(86)) {
			_sceneState = 9205;
			setAction(&_sequenceManager, this, 9205, &g_globals->_player, &_object2, &_object3, NULL);
		} else {
			_sceneState = 9201;
			setAction(&_sequenceManager, this, 9201, &g_globals->_player, &_object2, &_object3, NULL);
		}
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 9300 - Castle: In front of a large guarded door
 *
 *--------------------------------------------------------------------------*/
void Scene9300::signal() {
	switch (_sceneMode++) {
	case 9301:
		g_globals->setFlag(84);
		// No break on purpose
	case 9303:
		g_globals->_soundHandler.play(295);
		g_globals->_sceneManager.changeScene(9350);
		break;
	case 9302:
		g_globals->_player.enableControl();
		break;
	default:
		break;
	}
}

void Scene9300::dispatch() {
	if (_action) {
		_action->dispatch();
	} else if (g_globals->_player._position.y < 145) {
		g_globals->_player.disableControl();
		_sceneMode = 9303;
		setAction(&_sequenceManager, this, 9303, &g_globals->_player, &_object1, &_object2, NULL);
	}
}

void Scene9300::postInit(SceneObjectList *OwnerList) {
	Scene::postInit();
	setZoomPercents(130, 75, 230, 150);

	_sceneMode = 0;
	g_globals->_player.postInit();
	g_globals->_player.changeZoom(-1);
	_object1.postInit();
	_object2.postInit();
	g_globals->_soundHandler.play(289);

	_hotspot1.setDetails(35, 142, 76, 212, 9300, 0, 1);
	_hotspot2.setDetails(28, 90, 81, 143, 9300, 2, 3);
	_hotspot3.setDetails(78, 142, 146, 216, 9300, 4, 5);
	_hotspot4.setDetails(3, 43, 91, 74, 9300, 6, 7);
	_hotspot5.setDetails(82, 19, 157, 65, 9300, 8, 9);
	_hotspot6.setDetails(5, 218, 84, 274, 9300, 10, 11);
	_hotspot7.setDetails(86, 233, 168, 293, 9300, 12, 13);
	_hotspot8.setDetails(157, 0, 200, 230, 9300, 14, 15);
	_hotspot9.setDetails(169, 227, 200, 320, 9300, 16, 17);
	_hotspot10.setDetails(145, 97, 166, 225, 9300, 18, 19);
	_hotspot11.setDetails(81, 75, 145, 145, 9300, 20, 21);
	_hotspot12.setDetails(0, 0, 94, 35, 9300, 22, 23);
	_hotspot13.setDetails(12, 268, 149, 320, 9300, 24, 25);

	if (g_globals->_sceneManager._previousScene == 9350) {
		g_globals->_player.disableControl();
		_sceneMode = 9302;
		setAction(&_sequenceManager, this, 9302, &g_globals->_player, &_object1, &_object2, NULL);
	} else {
		g_globals->_player.disableControl();
		_sceneMode = 9301;
		setAction(&_sequenceManager, this, 9301, &g_globals->_player, &_object1, &_object2, NULL);
	}
}

/*--------------------------------------------------------------------------
 * Scene 9350 - Castle: In a hallway
 *
 *--------------------------------------------------------------------------*/

void Scene9350::signal() {
	switch (_sceneState ++) {
	case 0:
	case 9352:
	case 9353:
	case 9354:
		g_globals->_player.enableControl();
		break;
	case 9355:
		g_globals->_sceneManager.changeScene(9300);
		break;
	case 9356:
		g_globals->_sceneManager.changeScene(9360);
		break;
	case 9357:
	case 9359:
		g_globals->_sceneManager.changeScene(9400);
		break;
	default:
		break;
	}
}

void Scene9350::dispatch() {
	if (_action == 0) {
		if ((g_globals->_player._position.x > 300) && (g_globals->_player._position.y < 160)) {
			g_globals->_player.disableControl();
			_sceneState = 9356;
			setAction(&_sequenceManager, this, 9356, &g_globals->_player, &_object2, NULL);
		} else if ((g_globals->_player._position.x > 110) && (g_globals->_player._position.y >= 195)) {
			g_globals->_player.disableControl();
			_sceneState = 9357;
			setAction(&_sequenceManager, this, 9357, &g_globals->_player, &_object2, NULL);
		} else if ((g_globals->_player._position.x < 10) || ((g_globals->_player._position.x <= 110) && (g_globals->_player._position.y >= 195))) {
			g_globals->_player.disableControl();
			_sceneState = 9355;
			setAction(&_sequenceManager, this, 9355, &g_globals->_player, &_object2, NULL);
		}
	} else {
		Scene::dispatch();
	}
}

void Scene9350::postInit(SceneObjectList *OwnerList) {
	Scene::postInit();
	setZoomPercents(95, 80, 200, 100);
	g_globals->_player.postInit();

	_object1.setup(9351, 1, 3, 139, 97, 0);
	_sceneHotspot1.setDetails(42, 0, 97, 60, 9350, 0, -1);
	_sceneHotspot2.setDetails(37, 205, 82, 256, 9350, 0, -1);
	_sceneHotspot3.setDetails(29, 93, 92, 174, 9350, 1, -1);
	_sceneHotspot4.setDetails(0, 308, 109, 320, 9350, 2, -1);
	_sceneHotspot5.setDetails(0, 0, 200, 320, 9350, 3, -1);

	g_globals->_events.setCursor(CURSOR_WALK);
	g_globals->_player.disableControl();

	if (g_globals->_sceneManager._previousScene == 9360) {
		g_globals->_player.disableControl();
		_sceneState = 9352;
		setAction(&_sequenceManager, this, 9352, &g_globals->_player, &_object2, NULL);
	} else if (g_globals->_sceneManager._previousScene == 9400) {
		g_globals->_player.disableControl();
		_sceneState = 9353;
		setAction(&_sequenceManager, this, 9353, &g_globals->_player, &_object2, NULL);
	} else {
		if (g_globals->getFlag(84)) {
			g_globals->clearFlag(84);
			_object2.postInit();
			g_globals->_player.disableControl();
			_sceneState = 9359;
			setAction(&_sequenceManager, this, 9359, &g_globals->_player, &_object2, NULL);
		} else {
			g_globals->_player.disableControl();
			_sceneState = 9354;
			setAction(&_sequenceManager, this, 9354, &g_globals->_player, &_object2, NULL);
		}
	}
}

/*--------------------------------------------------------------------------
 * Scene 9360 - Castle: In a hallway
 *
 *--------------------------------------------------------------------------*/

void Scene9360::signal() {
	switch (_sceneState ++) {
	case 0:
	case 9362:
	case 9363:
	case 9364:
		g_globals->_player.enableControl();
		break;
	case 9365:
		g_globals->_sceneManager.changeScene(9350);
		break;
	case 9366:
		g_globals->_sceneManager.changeScene(9200);
		break;
	case 9367:
		g_globals->_sceneManager.changeScene(9450);
		break;
	default:
		break;
	}
}

void Scene9360::dispatch() {
	if (_action == 0) {
		if ((g_globals->_player._position.x > 300) && (g_globals->_player._position.y < 160)) {
			g_globals->_player.disableControl();
			_sceneState = 9366;
			setAction(&_sequenceManager, this, 9366, &g_globals->_player, NULL);
		} else if ((g_globals->_player._position.x > 110) && (g_globals->_player._position.y >= 195)) {
			g_globals->_player.disableControl();
			_sceneState = 9367;
			setAction(&_sequenceManager, this, 9367, &g_globals->_player, NULL);
		} else if ((g_globals->_player._position.x < 10) || ((g_globals->_player._position.x <= 110) && (g_globals->_player._position.y >= 195))) {
			g_globals->_player.disableControl();
			_sceneState = 9365;
			setAction(&_sequenceManager, this, 9365, &g_globals->_player, NULL);
		}
	} else {
		Scene::dispatch();
	}
}

void Scene9360::postInit(SceneObjectList *OwnerList) {
	Scene::postInit();
	setZoomPercents(95, 80, 200, 100);
	g_globals->_player.postInit();

	_hotspot1.setDetails(37, 92, 93, 173, 9360, 0, 1);
	_hotspot2.setDetails(42, 0, 100, 63, 9360, 2, -1);
	_hotspot3.setDetails(36, 205, 82, 260, 9360, 3, -1);
	_hotspot4.setDetails(103, 2, 200, 320, 9360, 4, -1);
	_hotspot5.setDetails(0, 0, 37, 320, 9360, 4, -1);
	_hotspot6.setDetails(35, 61, 103, 92, 9360, 4, -1);
	_hotspot7.setDetails(33, 174, 93, 207, 9360, 4, -1);
	_hotspot8.setDetails(28, 257, 149, 320, 9360, 4, -1);
	g_globals->_events.setCursor(CURSOR_WALK);
	g_globals->_player.disableControl();
	if (g_globals->_sceneManager._previousScene == 9350) {
		g_globals->_player.disableControl();
		_sceneState = 9364;
		setAction(&_sequenceManager, this, 9364, &g_globals->_player, NULL);
	} else if (g_globals->_sceneManager._previousScene == 9450) {
		g_globals->_player.disableControl();
		_sceneState = 9363;
		setAction(&_sequenceManager, this, 9363, &g_globals->_player, NULL);
	} else {
		g_globals->_player.disableControl();
		_sceneState = 9362;
		setAction(&_sequenceManager, this, 9362, &g_globals->_player, NULL);
	}
	_object1.setup(9351, 1, 1, 131, 90, 0);
}

/*--------------------------------------------------------------------------
 * Scene 9400 - Castle: Black-Smith room
 *
 *--------------------------------------------------------------------------*/
Scene9400::Scene9400() {
	_hittingAnvil = false;
}

void Scene9400::SceneHotspot7::doAction(int action) {
	Scene9400 *scene = (Scene9400 *)g_globals->_sceneManager._scene;

	if ((action == CURSOR_USE) && (RING_INVENTORY._straw._sceneNumber != 1)) {
		scene->_sceneState = 1;
		RING_INVENTORY._straw._sceneNumber = 1;
		scene->setAction(&scene->_sequenceManager, scene, 9408, &g_globals->_player, NULL);
	} else {
		NamedHotspot::doAction(action);
	}
}

void Scene9400::SceneHotspot8::doAction(int action) {
	Scene9400 *scene = (Scene9400 *)g_globals->_sceneManager._scene;

	if (action == CURSOR_TALK) {
		g_globals->_player.disableControl();
		scene->_sceneState = 2;
		scene->signal();
	} else {
		NamedHotspot::doAction(action);
	}
}

void Scene9400::signal() {
	switch (_sceneState ++) {
	case 0:
		_object1._numFrames = 6;
		_stripManager.start(9400, this);
		break;
	case 1:
		_object1._numFrames = 6;
		_object1.animate(ANIM_MODE_2, NULL);
		g_globals->_player.enableControl();
		break;
	case 2:
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 3:
		_stripManager.start(9405, this);
		break;
	case 4:
		_object1.animate(ANIM_MODE_2, this);
		g_globals->_player.enableControl();
		break;
	case 9350:
		g_globals->_sceneManager.changeScene(9350);
		break;
	default:
		break;
	}
}

void Scene9400::dispatch() {
	if ((_object1._animateMode == 2) && (_object1._strip == 1) && (_object1._frame == 4)){
		if (_hittingAnvil == false) {
			_soundHandler.play(296);
			_hittingAnvil = true;
		}
	} else {
		_hittingAnvil = false;
	}
	if (_action == 0) {
		if (g_globals->_player._position.y < 120) {
			_sceneState = 9350;
			g_globals->_player.disableControl();
			setAction(&_action1);
			Common::Point pt(-45, 88);
			NpcMover *mover = new NpcMover();
			g_globals->_player.addMover(mover, &pt, this);
		}
	} else {
		Scene::dispatch();
	}
}

void Scene9400::postInit(SceneObjectList *OwnerList) {
	Scene::postInit();
	_screenNumber = 9400;
	setZoomPercents(0, 100, 200, 100);
	g_globals->_player.postInit();
	_object1.postInit();
	_object3.postInit();
	_speakerQText._textPos.x = 20;

	_hotspot7.setDetails(157, 66, 180, 110, 9400, 21, 23);
	_hotspot5.setDetails(130, 133, 152, 198, 9400, 22, -1);
	_hotspot1.setDetails(33, 280, 69, 297, 9400, 1, 2);
	_hotspot2.setDetails(73, 96, 87, 159, 9400, 3, 4);
	_hotspot3.setDetails(89, 253, 111, 305, 9400, 5, 6);
	_hotspot4.setDetails(46, 0, 116, 35, 9400, 7, 8);
	_hotspot8.setDetails(58, 169, 122, 200, 9400, 9, 10);
	_hotspot6.setDetails(0, 0, 199, 319, 9400, 16, 0);

	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerOR);
	_stripManager.addSpeaker(&_speakerOText);

	g_globals->_events.setCursor(CURSOR_WALK);
	g_globals->_player.disableControl();

	// Useless check (skipped) : if (g_globals->_sceneManager._previousScene == 9350)
	_sceneState = 2;
	if (!g_globals->getFlag(89)) {
		g_globals->setFlag(89);
		_sceneState = 0;
	}

	setAction(&_sequenceManager, this, 9400, &g_globals->_player, &_object1, &_object3, NULL);
}

void Scene9400::synchronize(Serializer &s) {
	Scene::synchronize(s);
	if (s.getVersion() >= 3)
		s.syncAsSint16LE(_hittingAnvil);
}

/*--------------------------------------------------------------------------
 * Scene 9450 - Castle: Dining room
 *
 *--------------------------------------------------------------------------*/
void Scene9450::Object2::signal() {
	Scene9450 *scene = (Scene9450 *)g_globals->_sceneManager._scene;

	this->setAction(&scene->_sequenceManager3, this, 9458, this, &scene->_object1, NULL);
}

void Scene9450::Object3::dispatch() {
	SceneObject::dispatch();
	_percent = (_percent * 20) / 30;
}

void Scene9450::Hotspot1::doAction(int action) {
	Scene9450 *scene = (Scene9450 *)g_globals->_sceneManager._scene;

	if (action == CURSOR_USE) {
		if (scene->_object2._action)
			scene->_object2._action->remove();
		scene->_sceneMode = 9459;
		g_globals->_player.disableControl();
		scene->setAction(&scene->_sequenceManager1, scene, 9459, &scene->_object2, &scene->_object1, &scene->_object3, &g_globals->_player, NULL);
	} else {
		NamedHotspot::doAction(action);
	}
}

void Scene9450::Hotspot3::doAction(int action) {
	Scene9450 *scene = (Scene9450 *)g_globals->_sceneManager._scene;

	switch (action) {
	case OBJECT_CLOAK:
	case OBJECT_JACKET:
	case OBJECT_TUNIC2:
		scene->_sceneMode = 9460;
		g_globals->_player.disableControl();
		scene->setAction(&scene->_sequenceManager1, scene, 9460, &g_globals->_player, &scene->_object2, &scene->_object1, NULL);
		break;
	case OBJECT_TUNIC:
		SceneItem::display(9450, 49, SET_Y, 20, SET_WIDTH, 200, SET_EXT_BGCOLOR, 7, LIST_END);
		break;
	case CURSOR_WALK:
		// nothing
		break;
	case CURSOR_LOOK:
		SceneItem::display(9450, 41, SET_Y, 20, SET_WIDTH, 200, SET_EXT_BGCOLOR, 7, LIST_END);
		break;
	case CURSOR_USE:
	case CURSOR_TALK:
		if (RING_INVENTORY._tunic._sceneNumber == 9450) {
			if (scene->_object2._action)
				scene->_object2._action->remove();
			scene->_sceneMode = 9459;
			g_globals->_player.disableControl();
			scene->setAction(&scene->_sequenceManager1, scene, 9459, &scene->_object2, &scene->_object1, &scene->_object3, &g_globals->_player, NULL);
		} else if ((RING_INVENTORY._cloak._sceneNumber != 1) && (RING_INVENTORY._jacket._sceneNumber != 1) && (RING_INVENTORY._tunic2._sceneNumber != 1)) {
			SceneItem::display(9450, 38, SET_Y, 20, SET_WIDTH, 200, SET_EXT_BGCOLOR, 7, LIST_END);
		} else {
			scene->_sceneMode = 9460;
			g_globals->_player.disableControl();
			scene->setAction(&scene->_sequenceManager1, scene, 9460, &g_globals->_player, &scene->_object2, &scene->_object1, NULL);
		}
		break;
	default:
		SceneItem::display(9450, 45, SET_Y, 20, SET_WIDTH, 200, SET_EXT_BGCOLOR, 7, LIST_END);
		break;
	}
}

void Scene9450::signal() {
	switch (_sceneMode++) {
	case 1002:
	case 1004:
		// Drink
		setAction(&_sequenceManager1, this, 9456, &_object2, &_object1, &_object3, NULL);
		break;
	case 1005:
		// Bring me more wine
		setAction(&_sequenceManager1, this, 9457, &_object2, &_object1, &_object3, NULL);
		break;
	case 9451:
		if (g_globals->getFlag(87)) {
			g_globals->_player.enableControl();
		} else {
			_sceneMode = 1001;
			if (_object2._action)
				_object2._action->remove();
			// Eat
			setAction(&_sequenceManager1, this, 9455, &_object2, &_object1, &_object3, NULL);
		}
		break;
	case 1001:
	case 1003:
		// Eat
		setAction(&_sequenceManager1, this, 9455, &_object2, &_object1, &_object3, NULL);
		break;
	case 9453:
		g_globals->_sceneManager.changeScene(9360);
		break;
	case 9459:
		RING_INVENTORY._tunic._sceneNumber = 1;
		_object2.signal();
		g_globals->_player.enableControl();
		g_globals->_events.setCursor(CURSOR_WALK);
		_hotspot1.remove();
		break;
	case 1006:
		g_globals->setFlag(87);
		// No break on purpose
	default:
		g_globals->_player.enableControl();
		break;
	}
}

void Scene9450::dispatch() {
	if (_action) {
		_action->dispatch();
	} else {
		if ((g_globals->_player._position.y < 98) && (g_globals->_player._position.x > 241) && (g_globals->_player._position.x < 282)) {
			g_globals->_player.disableControl();
			_sceneMode = 9452;
			setAction(&_sequenceManager1, this, 9452, &g_globals->_player, NULL);
		} else if ((g_globals->_player._position.y < 99) && (g_globals->_player._position.x > 68) && (g_globals->_player._position.x < 103)) {
			g_globals->_player.disableControl();
			_sceneMode = 9453;
			setAction(&_sequenceManager1, this, 9453, &g_globals->_player, NULL);
		}
	}
}

void Scene9450::postInit(SceneObjectList *OwnerList) {
	Scene::postInit();
	setZoomPercents(84, 75, 167, 150);
	g_globals->_events.setCursor(CURSOR_WALK);
	g_globals->_player.postInit();

	_object2.postInit();
	_object1.postInit();
	_object1.hide();

	g_globals->_player.disableControl();
	_sceneMode = 9451;
	setAction(&_sequenceManager1, this, 9451, &g_globals->_player, NULL);

	if (g_globals->getFlag(87)) {
		if (RING_INVENTORY._tunic._sceneNumber == 1) {
			_object2.signal();
		} else {
			_object2.setPosition(Common::Point(184, 144));
			_object2.setVisage(9451);
			_object2.fixPriority(250);
			_object2._strip = 5;
			_object2._frame = 10;
		}
	} else {
		_object3.postInit();
		_object3.hide();
		_object3.setAction(&_sequenceManager2, NULL, 9455, &_object2, &_object1, NULL);
	}

	if (RING_INVENTORY._tunic._sceneNumber != 1)
		_hotspot1.setDetails(123, 139, 138, 170, 9450, 37, -1);

	_hotspot2.setDetails(153, 102, 176, 141, 9450, 39, 40);
	_hotspot3.setDetails(97, 198, 130, 229, 9450, 41, 42);
	_hotspot15.setDetails(131, 190, 145, 212, 9450, 43, 44);
	_hotspot4.setDetails(33, 144, 105, 192, 9450, 0, 1);
	_hotspot5.setDetails(20, 236, 106, 287, 9450, 2, 3);
	_hotspot6.setDetails(137, 119, 195, 320, 9450, 4, 5);
	_hotspot7.setDetails(20, 59, 99, 111, 9450, 6, -1);
	_hotspot8.setDetails(110, 0, 199, 117, 9450, 7, 8);
	_hotspot9.setDetails(101, 104, 130, 174, 9450, 9, 10);
	_hotspot10.setDetails(110, 246, 149, 319, 9450, 11, 12);
	_hotspot11.setDetails(16, 34, 74, 62, 6450, 13, 14);
	_hotspot12.setDetails(19, 108, 72, 134, 9450, 15, 16);
	_hotspot13.setDetails(18, 215, 71, 237, 9450, 17, 18);
	_hotspot14.setDetails(15, 288, 76, 314, 9450, 19, 20);
	_hotspot16.setDetails(0, 0, 200, 320, 9450, 46, -1);
}

/*--------------------------------------------------------------------------
 * Scene 9500 - Castle: Bedroom
 *
 *--------------------------------------------------------------------------*/
void Scene9500::Hotspot1::doAction(int action) {
	Scene9500 *scene = (Scene9500 *)g_globals->_sceneManager._scene;

	if (action == OBJECT_SWORD) {
		scene->_sceneMode = 9510;
		g_globals->setFlag(92);
		RING_INVENTORY._sword._sceneNumber = 9500;
		g_globals->_player.disableControl();
		g_globals->_sceneItems.remove(this);
		scene->_hotspot2.setDetails(87, 294, 104, 314, 9400, 17, -1);
		scene->setAction(&scene->_sequenceManager, scene, 9510, &g_globals->_player, &scene->_object2, NULL);
	} else {
		NamedHotspot::doAction(action);
	}
}

void Scene9500::Hotspot2::doAction(int action) {
	Scene9500 *scene = (Scene9500 *)g_globals->_sceneManager._scene;

	if (action == CURSOR_USE) {
		scene->_sceneMode = 9511;
		g_globals->_player.disableControl();
		g_globals->_sceneItems.remove(this);
		scene->setAction(&scene->_sequenceManager, scene, 9511, &g_globals->_player, &scene->_object2, NULL);
	} else {
		NamedHotspot::doAction(action);
	}
}

void Scene9500::Hotspot3::doAction(int action) {
	Scene9500 *scene = (Scene9500 *)g_globals->_sceneManager._scene;

	if ((action == CURSOR_USE) && (RING_INVENTORY._candle._sceneNumber != 1)){
		scene->_sceneMode = 9505;
		g_globals->_player.disableControl();
		g_globals->_sceneItems.remove(this);
		scene->setAction(&scene->_sequenceManager, scene, 9505, &g_globals->_player, &scene->_candle, NULL);
	} else {
		NamedHotspot::doAction(action);
	}
}

void Scene9500::Hotspot4::doAction(int action) {
	Scene9500 *scene = (Scene9500 *)g_globals->_sceneManager._scene;

	if (action == OBJECT_CANDLE) {
		g_globals->_player.disableControl();
		if (RING_INVENTORY._straw._sceneNumber == 9500) {
			scene->_sceneMode = 9506;
			g_globals->_sceneItems.remove(&scene->_hotspot5);
			g_globals->_sceneItems.remove(this);
			scene->setAction(&scene->_sequenceManager, scene, 9506, &g_globals->_player, &scene->_object3, NULL);
			RING_INVENTORY._candle._sceneNumber = 9850;
		} else {
			scene->_sceneMode = 9507;
			scene->setAction(&scene->_sequenceManager, scene, 9507, &g_globals->_player, &scene->_object3, NULL);
		}
	} else if (action == OBJECT_STRAW) {
		scene->_sceneMode = 9512;
		g_globals->_player.disableControl();
		RING_INVENTORY._straw._sceneNumber = 9500;
		scene->setAction(&scene->_sequenceManager, scene, 9512, &g_globals->_player, &scene->_object3, NULL);
	} else {
		NamedHotspot::doAction(action);
	}
}

void Scene9500::signal() {
	switch (_sceneMode) {
	case 9503:
		g_globals->_sceneManager.changeScene(9200);
		g_globals->_soundHandler.play(295);
		break;
	case 9504:
		g_globals->_sceneManager.changeScene(9850);
		break;
	case 9505:
		_candle.setStrip(2);
		RING_INVENTORY._candle._sceneNumber = 1;
		g_globals->_player.enableControl();
		break;
	case 9506:
		g_globals->setFlag(85);
		g_globals->_player.enableControl();
		break;
	case 9511:
		RING_INVENTORY._helmet._sceneNumber = 1;
		g_globals->_player.enableControl();
		if (!g_globals->getFlag(51)) {
			g_globals->setFlag(51);
			g_globals->_player.disableControl();
			_sceneMode = 9514;
			setAction(&_sequenceManager, this, 9514, &g_globals->_player, NULL, NULL, NULL, NULL);
		}
		break;
	case 0:
	case 9514:
	default:
		g_globals->_player.enableControl();
		break;
	}
}

void Scene9500::dispatch() {
	if (_action) {
		_action->dispatch();
	} else {
		if (g_globals->_player._position.y >= 199) {
			g_globals->_player.disableControl();
			_sceneMode = 9503;
			setAction(&_sequenceManager, this, 9503, &g_globals->_player, NULL, NULL, NULL, NULL);
		} else if (g_globals->_player._position.y < 127) {
			g_globals->_player.disableControl();
			_sceneMode = 9504;
			setAction(&_sequenceManager, this, 9504, &g_globals->_player, NULL, NULL, NULL, NULL);
		}
	}

}

void Scene9500::process(Event &event) {
	Scene::process(event);
}

void Scene9500::postInit(SceneObjectList *OwnerList) {
	Scene::postInit();
	setZoomPercents(110, 75, 200, 150);

	g_globals->_player.postInit();
	g_globals->_soundHandler.play(305);

	_candle.postInit();
	_candle.setVisage(9500);
	_candle.setStrip(1);
	_candle.animate(ANIM_MODE_2);
	_candle.setPosition(Common::Point(30, 105));
	if (RING_INVENTORY._candle._sceneNumber != 9500)
		_candle.setStrip(2);

	_object3.postInit();
	_object3.hide();
	_object3.fixPriority(150);
	_object3.setPosition(Common::Point(166, 133));
	if (RING_INVENTORY._straw._sceneNumber == 9500) {
		_object3.show();
		_object3.setVisage(5);
		_object3._strip = 2;
		_object3._frame = 9;
		_object3.setPosition(Common::Point(168, 128));
		if (g_globals->getFlag(85)) {
			_object3.setVisage(9500);
			_object3.setStrip(4);
			_object3.animate(ANIM_MODE_8, 0, NULL);
			_object3.setPosition(Common::Point(166, 133));
		}
	}

	_object2.postInit();
	_object2.hide();
	if (g_globals->getFlag(92)) {
		_object2.show();
		_object2.setVisage(9501);
		_object2.setStrip(1);
		_object2.setFrame(_object2.getFrameCount());
		_object2.setPosition(Common::Point(303, 130));
		_object2.fixPriority(132);
		if (RING_INVENTORY._helmet._sceneNumber != 1) {
			_hotspot2.setDetails(87, 294, 104, 314, 9400, 17, -1);
		} else {
			_object2.setStrip(2);
			_object2.setFrame(1);
		}
	} else {
		_hotspot1.setDetails(105, 295, 134, 313, 9500, 9, 10);
	}

	_hotspot17.setDetails(101, 293, 135, 315, 9500, 9, 10);
	_hotspot3.setDetails(84, 12, 107, 47, 9500, 15, 15);
	_hotspot6.setDetails(93, 11, 167, 46, 9500, 0, 1);
	_hotspot7.setDetails(100, 70, 125, 139, 9500, 2, 3);

	if (!g_globals->getFlag(85)) {
		_hotspot5.setDetails(111, 68, 155, 244, 9500, 17, -1);
		_hotspot4.setDetails(57, 71, 120, 126, 9500, 16, -1);
	}

	_hotspot8.setDetails(60, 24, 90, 53, 9500, 4, 5);
	_hotspot9.setDetails(72, 143, 93, 163, 9500, 4, 5);
	_hotspot10.setDetails(70, 205, 92, 228, 9500, 4, 5);
	_hotspot11.setDetails(66, 291, 90, 317, 9500, 4, 5);
	_hotspot12.setDetails(22, 58, 101, 145, 9500, 6, 7);
	_hotspot13.setDetails(121, 57, 163, 249, 9500, 6, 7);
	_hotspot14.setDetails(115, 133, 135, 252, 9500, 6, 7);
	_hotspot15.setDetails(55, 240, 125, 254, 9500, 6, 7);
	_hotspot16.setDetails(53, 251, 132, 288, 9500, 8, -1);
	_hotspot19.setDetails(101, 207, 120, 225, 9500, 9, 10);
	_hotspot18.setDetails(98, 144, 117, 162, 9500, 9, 10);
	_hotspot20.setDetails(102, 27, 132, 50, 9500, 9, 10);

	g_globals->_events.setCursor(CURSOR_WALK);
	g_globals->_player.disableControl();

	if ((g_globals->_sceneManager._previousScene == 9200) || (g_globals->_sceneManager._previousScene != 9850)) {
		_sceneMode = 0;
		if (RING_INVENTORY._helmet._sceneNumber != 1) {
			setAction(&_sequenceManager, this, 9501, &g_globals->_player, &_candle, NULL);
		} else {
			RING_INVENTORY._helmet._sceneNumber = 9500;
			_hotspot2.setDetails(87, 294, 104, 314, 9400, 17, -1);
			setAction(&_sequenceManager, this, 9513, &g_globals->_player, &_object2, NULL);
		}
	} else {
		_sceneMode = 0;
		setAction(&_sequenceManager, this, 9502, &g_globals->_player, &_candle, NULL);
	}
}

/*--------------------------------------------------------------------------
 * Scene 9700 - Castle: Balcony
 *
 *--------------------------------------------------------------------------*/
void Scene9700::signal() {
	switch (_sceneMode ++) {
	case 9703:
		g_globals->setFlag(88);
		// No break on purpose
	case 9701:
	case 9702:
		_gfxButton1.setText(EXIT_MSG);
		_gfxButton1._bounds.center(50, 190);
		_gfxButton1.draw();
		_gfxButton1._bounds.expandPanes();
		g_globals->_player.enableControl();
		g_globals->_player._canWalk = false;
		g_globals->_events.setCursor(CURSOR_USE);
		break;
	case 9704:
		g_globals->_soundHandler.play(323);
		g_globals->_sceneManager.changeScene(9750);
		break;
	}
}

void Scene9700::process(Event &event) {
	Scene::process(event);
	if ((event.eventType == EVENT_BUTTON_DOWN) && !_action) {
		if (_gfxButton1.process(event)) {
			g_globals->_sceneManager.changeScene(9200);
		} else if (g_globals->_events._currentCursor == OBJECT_SCANNER) {
			event.handled = true;
			if (RING_INVENTORY._helmet._sceneNumber == 1) {
				g_globals->_player.disableControl();
				_sceneMode = 9704;
				setAction(&_sequenceManager, this, 9704, &g_globals->_player, &_object1, NULL);
			} else {
				g_globals->_player.disableControl();
				_sceneMode = 9703;
				setAction(&_sequenceManager, this, 9703, &g_globals->_player, &_object1, NULL);
			}
		}
	}
}

void Scene9700::postInit(SceneObjectList *OwnerList) {
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);

	_sceneHotspot1.setDetails(84, 218, 151, 278, 9700, 14, -1);
	_sceneHotspot2.setDetails(89, 11, 151, 121, 9700, 14, -1);
	_sceneHotspot3.setDetails(69, 119, 138, 216, 9700, 15, 16);
	_sceneHotspot4.setDetails(34, 13, 88, 116, 9700, 17, -1);
	_sceneHotspot5.setDetails(52, 119, 68, 204, 9700, 17, -1);
	_sceneHotspot6.setDetails(0, 22, 56, 275, 9700, 18, -1);

	_object1.postInit();
	_object1.hide();
	g_globals->_player.postInit();
	if (!g_globals->getFlag(97)) {
		g_globals->_player.disableControl();
		_sceneMode = 9701;
		setAction(&_sequenceManager, this, 9701, &g_globals->_player, &_object1, NULL);
		g_globals->setFlag(97);
	} else {
		g_globals->_player.disableControl();
		_sceneMode = 9702;
		setAction(&_sequenceManager, this, 9702, &g_globals->_player, &_object1, NULL);
	}
}

/*--------------------------------------------------------------------------
 * Scene 9750 - Castle: In the garden
 *
 *--------------------------------------------------------------------------*/
void Scene9750::signal() {
	switch (_sceneMode ++) {
	case 9751:
		g_globals->_soundHandler.fadeOut(this);
		break;
	case 9752:
		g_globals->_sceneManager.changeScene(2100);
	default:
		break;
	}
}

void Scene9750::dispatch() {
	Scene::dispatch();
}

void Scene9750::postInit(SceneObjectList *OwnerList) {
	loadScene(9750);
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);

	g_globals->_player.postInit();
	_object1.postInit();
	_object1.hide();
	_object2.postInit();
	_object2.hide();
	g_globals->_player.disableControl();
	_sceneMode = 9751;
	setAction(&_sequenceManager, this, 9751, &g_globals->_player, &_object1, &_object2, NULL);
}


/*--------------------------------------------------------------------------
 * Scene 9850 - Castle: Dressing room
 *
 *--------------------------------------------------------------------------*/
void Scene9850::Object6::doAction(int action) {
	if ((_flags & OBJFLAG_HIDE) == 0) {
		if (action == CURSOR_LOOK) {
			SceneItem::display(9850, 27, SET_Y, 20, SET_WIDTH, 200, SET_EXT_BGCOLOR, 7, LIST_END);
		} else if (action == CURSOR_USE) {
			RING_INVENTORY._scimitar._sceneNumber = 1;
			hide();
		} else {
			SceneHotspot::doAction(action);
		}
	}
}
void Scene9850::Object7::doAction(int action) {
	if ((_flags & OBJFLAG_HIDE) == 0) {
		if (action == CURSOR_LOOK) {
			SceneItem::display(9850, 28, SET_Y, 20, SET_WIDTH, 200, SET_EXT_BGCOLOR, 7, LIST_END);
		} else if (action == CURSOR_USE) {
			RING_INVENTORY._sword._sceneNumber = 1;
			hide();
		} else {
			SceneHotspot::doAction(action);
		}
	}
}

// Hair covered tunic
void Scene9850::Hotspot12::doAction(int action) {
	Scene9850 *scene = (Scene9850 *)g_globals->_sceneManager._scene;

	if (action == CURSOR_USE) {
		if (RING_INVENTORY._tunic2._sceneNumber != 1) {
			RING_INVENTORY._tunic2._sceneNumber = 1;
			g_globals->_player.disableControl();
			scene->_sceneMode = 9858;
			scene->setAction(&scene->_sequenceManager, scene, 9858, &g_globals->_player, &scene->_objTunic2, NULL);
		} else {
			RING_INVENTORY._tunic2._sceneNumber = 9850;
			g_globals->_player.disableControl();
			scene->_sceneMode = 9861;
			scene->setAction(&scene->_sequenceManager, scene, 9861, &g_globals->_player, &scene->_objTunic2, NULL);
		}
	} else if ((action != CURSOR_LOOK) || (RING_INVENTORY._tunic2._sceneNumber != 1)) {
		NamedHotspot::doAction(action);
	} else {
		SceneItem::display(9850, 30, SET_Y, 20, SET_WIDTH, 200, SET_EXT_BGCOLOR, 7, LIST_END);
	}
}

void Scene9850::Hotspot14::doAction(int action) {
	Scene9850 *scene = (Scene9850 *)g_globals->_sceneManager._scene;

	if (action == CURSOR_USE) {
		if (RING_INVENTORY._jacket._sceneNumber != 1) {
			RING_INVENTORY._jacket._sceneNumber = 1;
			g_globals->_player.disableControl();
			scene->_sceneMode = 9857;
			scene->setAction(&scene->_sequenceManager, scene, 9857, &g_globals->_player, &scene->_objJacket, NULL);
		} else {
			RING_INVENTORY._jacket._sceneNumber = 9850;
			g_globals->_player.disableControl();
			scene->_sceneMode = 9860;
			scene->setAction(&scene->_sequenceManager, scene, 9860, &g_globals->_player, &scene->_objJacket, NULL);
		}
	} else if ((action != CURSOR_LOOK) || (RING_INVENTORY._jacket._sceneNumber != 1)) {
		NamedHotspot::doAction(action);
	} else {
		SceneItem::display(9850, 30, SET_Y, 20, SET_WIDTH, 200, SET_EXT_BGCOLOR, 7, LIST_END);
	}
}

void Scene9850::Hotspot16::doAction(int action) {
	Scene9850 *scene = (Scene9850 *)g_globals->_sceneManager._scene;

	if (action == CURSOR_USE) {
		if (RING_INVENTORY._cloak._sceneNumber != 1) {
			RING_INVENTORY._cloak._sceneNumber = 1;
			g_globals->_player.disableControl();
			scene->_sceneMode = 9862;
			scene->setAction(&scene->_sequenceManager, scene, 9862, &g_globals->_player, &scene->_objCloak, NULL);
		} else {
			RING_INVENTORY._cloak._sceneNumber = 9850;
			g_globals->_player.disableControl();
			scene->_sceneMode = 9859;
			scene->setAction(&scene->_sequenceManager, scene, 9859, &g_globals->_player, &scene->_objCloak, NULL);
		}
	} else if ((action != CURSOR_LOOK) || (RING_INVENTORY._cloak._sceneNumber != 1)) {
		NamedHotspot::doAction(action);
	} else {
		SceneItem::display(9850, 30, SET_Y, 20, SET_WIDTH, 200, SET_EXT_BGCOLOR, 7, LIST_END);
	}
}

void Scene9850::Hotspot17::doAction(int action) {
	Scene9850 *scene = (Scene9850 *)g_globals->_sceneManager._scene;

	if (action == OBJECT_SCANNER) {
		SceneItem::display(9850, 32, SET_Y, 20, SET_WIDTH, 200, SET_EXT_BGCOLOR, 7, LIST_END);
	} else {
		if (action == CURSOR_USE)
			scene->_soundHandler.play(306);
		NamedHotspot::doAction(action);
	}
}

void Scene9850::Hotspot18::doAction(int action) {
	Scene9850 *scene = (Scene9850 *)g_globals->_sceneManager._scene;

	if (action == OBJECT_SCANNER) {
		SceneItem::display(9850, 32, SET_Y, 20, SET_WIDTH, 200, SET_EXT_BGCOLOR, 7, LIST_END);
	} else {
		if (action == CURSOR_USE)
			scene->_soundHandler.play(306);
		NamedHotspot::doAction(action);
	}
}

void Scene9850::Hotspot19::doAction(int action) {
	Scene9850 *scene = (Scene9850 *)g_globals->_sceneManager._scene;

	if (action == OBJECT_SCANNER) {
		SceneItem::display(9850, 31, SET_Y, 20, SET_WIDTH, 200, SET_EXT_BGCOLOR, 7, LIST_END);
	} else {
		if (action == CURSOR_USE)
			scene->_soundHandler.play(313);
		NamedHotspot::doAction(action);
	}
}

// Arrow on Statue
void Scene9850::Hotspot20::doAction(int action) {
	Scene9850 *scene = (Scene9850 *)g_globals->_sceneManager._scene;

	if (action == CURSOR_USE) {
		g_globals->_player.disableControl();
		if (scene->_objSword._state == 0) {
			if (RING_INVENTORY._scimitar._sceneNumber == 9850)
				scene->_objScimitar.show();
			if (RING_INVENTORY._sword._sceneNumber == 9850)
				scene->_objSword.show();
			scene->_sceneMode = 11;
			setAction(&scene->_sequenceManager, scene, 9853, &g_globals->_player, &scene->_objDoor, &scene->_objLever, NULL);
		} else {
			scene->_sceneMode = 10;
			setAction(&scene->_sequenceManager, scene, 9854, &g_globals->_player, &scene->_objDoor, &scene->_objLever, NULL);
		}
		scene->_objSword._state ^= 1;
	} else {
		NamedHotspot::doAction(action);
	}
}

void Scene9850::signal() {
	switch (_sceneMode ++) {
	case 10:
		// Hidden closet closed
		if (RING_INVENTORY._scimitar._sceneNumber == 9850)
			_objScimitar.hide();
		if (RING_INVENTORY._sword._sceneNumber == 9850)
			_objSword.hide();
		g_globals->_sceneItems.remove(&_objScimitar);
		g_globals->_sceneItems.remove(&_objSword);
		g_globals->_sceneItems.addItems(&_hotspot19, NULL);
		g_globals->_player.enableControl();
		break;
	case 11:
		// Hidden closet opened
		if (RING_INVENTORY._scimitar._sceneNumber == 9850)
			g_globals->_sceneItems.addItems(&_objScimitar, NULL);
		if (RING_INVENTORY._sword._sceneNumber == 9850)
			g_globals->_sceneItems.addItems(&_objSword, NULL);
		g_globals->_sceneItems.remove(&_hotspot19);
		g_globals->_player.enableControl();
		break;
	case 9500:
		g_globals->_sceneManager.changeScene(_sceneMode - 1);
		break;
	case 0:
	default:
		g_globals->_player.enableControl();
		break;
	}
}

void Scene9850::process(Event &event) {
	Scene::process(event);
	if ((event.eventType == EVENT_KEYPRESS) && (event.kbd.keycode == Common::KEYCODE_4)) {
		event.handled = true;
		g_globals->_player.disableControl();
		if (_objSword._state == 0) {
			_sceneMode = 0;
			setAction(&_sequenceManager, this, 9853, &_objLever, &_objDoor, &_objScimitar, &_objSword, NULL);
		} else {
			_sceneMode = 10;
			setAction(&_sequenceManager, this, 9854, &_objLever, &_objDoor, &_objScimitar, &_objSword, NULL);
		}
		_objSword._state ^= 1;
	}
}

void Scene9850::dispatch() {
	if (_action) {
		_action->dispatch();
	} else if (g_globals->_player._position.y >= 198) {
		g_globals->_player.disableControl();
		_sceneMode = 9500;
		setAction(&_sequenceManager, this, 9852, &g_globals->_player, NULL);
	}
}

void Scene9850::postInit(SceneObjectList *OwnerList) {
	Scene::postInit();
	_objSword._state = 0;

	_objDoor.postInit();
	_objDoor.setVisage(9850);
	_objDoor.setStrip(1);
	_objDoor.setFrame(1);
	_objDoor.setPosition(Common::Point(28, 118));
	_objDoor.fixPriority(90);

	_objLever.postInit();
	_objLever.setVisage(9850);
	_objLever.setStrip(4);
	_objLever.setFrame(1);
	_objLever.setPosition(Common::Point(256, 35));

	_objCloak.postInit();
	_objCloak.setVisage(9850);
	_objCloak.setStrip(5);
	_objCloak.setFrame(1);
	_objCloak.fixPriority(90);
	_objCloak.setPosition(Common::Point(157, 81));
	if (RING_INVENTORY._cloak._sceneNumber != 9850)
		_objCloak.hide();

	_objJacket.postInit();
	_objJacket.setVisage(9850);
	_objJacket.setStrip(5);
	_objJacket.setFrame(2);
	_objJacket.fixPriority(90);
	_objJacket.setPosition(Common::Point(201, 84));
	if (RING_INVENTORY._jacket._sceneNumber != 9850)
		_objJacket.hide();

	_objTunic2.postInit();
	_objTunic2.setVisage(9850);
	_objTunic2.setStrip(5);
	_objTunic2.setFrame(3);
	_objTunic2.fixPriority(90);
	_objTunic2.setPosition(Common::Point(295, 90));
	if (RING_INVENTORY._tunic2._sceneNumber != 9850)
		_objTunic2.hide();

	if (RING_INVENTORY._scimitar._sceneNumber == 9850) {
		_objScimitar.postInit();
		_objScimitar.setVisage(9850);
		_objScimitar.setStrip(2);
		_objScimitar.setFrame(1);
		_objScimitar.setPosition(Common::Point(55, 83));
		_objScimitar.fixPriority(80);
		_objScimitar.hide();
	}

	if (RING_INVENTORY._sword._sceneNumber == 9850) {
		_objSword.postInit();
		_objSword.setVisage(9850);
		_objSword.setStrip(3);
		_objSword.setFrame(1);
		_objSword.setPosition(Common::Point(56, 101));
		_objSword.fixPriority(80);
		_objSword.hide();
	}

	_spotLever.setDetails(30, 251, 45, 270, 9850, 26, -1);
	_hotspot1.setDetails(123, 0, 200, 320, 9850, 0, 1);
	_hotspot2.setDetails(107, 87, 133, 308, 9850, 0, 1);
	_hotspot3.setDetails(2, 28, 53, 80, 9850, 2, 3);
	_hotspot4.setDetails(13, 0, 55, 27, 9850, 2, 3);
	_hotspot5.setDetails(8, 74, 27, 91, 9850, 4, 5);
	_hotspot17.setDetails(61, 0, 125, 28, 9850, 6, 7);
	_hotspot18.setDetails(51, 95, 105, 145, 9850, 6, 7);
	_hotspot19.setDetails(56, 28, 115, 97, 9850, 6, 8);
	_hotspot6.setDetails(0, 223, 115, 257, 9850, 9, 10);
	_hotspot7.setDetails(15, 254, 33, 268, 9850, 9, -1);
	_hotspot8.setDetails(17, 218, 37, 233, 9850, 9, 10);
	_hotspot9.setDetails(8, 113, 26, 221, 9850, 11, 12);
	_hotspot10.setDetails(14, 94, 53, 112, 9850, 13, 14);
	_hotspot11.setDetails(5, 269, 29, 303, 9850, 15, 16);
	_hotspot12.setDetails(43, 278, 91, 317, 9850, 17, 18);
	_hotspot13.setDetails(47, 263, 112, 282, 9850, 19, 20);
	_hotspot14.setDetails(43, 188, 86, 224, 9850, 21, 22);
	_hotspot15.setDetails(43, 162, 92, 191, 9850, 23, 24);
	_hotspot16.setDetails(40, 146, 90, 169, 9850, 25, -1);

	g_globals->_player.postInit();
	g_globals->_player.disableControl();
	_sceneMode = 0;
	setAction(&_sequenceManager, this, 9851, &g_globals->_player, NULL);
}

/*--------------------------------------------------------------------------
 * Scene 9900 - Ending
 *
 *--------------------------------------------------------------------------*/
void Scene9900::strAction1::signal() {
	const byte mask1[3] = {0xff, 0xff, 0xff};
	const byte mask2[3] = {0, 0, 0};

	Scene9900 *scene = (Scene9900 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_soundHandler.play(351);
		_object9.postInit();
		_object9.setVisage(18);
		_object9._frame = 1;
		_object9._strip = 6;
		_object9.fixPriority(250);
		_object9.setPosition(Common::Point(171, 59));
		_object9.animate(ANIM_MODE_5, NULL);
		g_globals->_scenePalette.addRotation(67, 111, 1, 1, this);
		scene->_object2.hide();
		break;
	case 1:
		_palette1.getPalette();
		g_globals->_scenePalette.addFader(&mask1[0], 1, 10, this);
		break;
	case 2:
		_object9.remove();
		g_globals->_scenePalette.addFader(&mask2[0], 1, 5, this);
		break;
	case 3:
		g_globals->_soundHandler.play(377);
		setDelay(120);
		break;
	case 4:
		g_globals->_scenePalette.addFader(_palette1._palette, 256, 1, this);
		break;
	case 5:
		remove();
		break;
	default:
		break;
	}
}

void Scene9900::strAction2::signal() {
	switch (_actionIndex++) {
	case 0:
		_lineNum = 0;
		_txtArray1Index = 0;
		_txtArray1[0]._position.y = 200;
		_txtArray1[1]._position.y = 300;
		_txtArray2[0]._position.y = 400;
		_txtArray2[1]._position.y = 500;
		_var3 = 0;
		// No break on purpose
	case 1: {
		Common::String msg = g_resourceManager->getMessage(8030, _lineNum++);
		if (msg.compareTo("LASTCREDIT")) {
			if (_var3) {
				// Not used?
				// int x = _txtArray1[_txtArray1Index].getFrame().getBounds().height();
				_txtArray1[_txtArray1Index]._moveDiff.y = 10;

				NpcMover *mover = new NpcMover();
				Common::Point pt(_txtArray1[_txtArray1Index]._position.x, -100);
				_txtArray1[_txtArray1Index].addMover(mover, &pt, NULL);

				// Not used?
				// int x = _txtArray2[_txtArray1Index].getFrame().getBounds().height();
				_txtArray2[_txtArray1Index]._moveDiff.y = 10;
				_txtArray1Index = (_txtArray1Index + 1) % 2;
			}
			_var3 = 1;
			_txtArray1[_txtArray1Index]._textMode = ALIGN_CENTER;
			_txtArray1[_txtArray1Index]._width = 240;
			_txtArray1[_txtArray1Index]._fontNumber = 2;
			_txtArray1[_txtArray1Index]._color1 = 7;
			_txtArray1[_txtArray1Index].setup(msg);
			_txtArray1[_txtArray1Index]._moveRate = 20;
			_txtArray1[_txtArray1Index]._moveDiff.y = 2;
			_txtArray1[_txtArray1Index].fixPriority(255);
			int frameWidth = _txtArray1[_txtArray1Index].getFrame().getBounds().width();
			int frameHeight = _txtArray1[_txtArray1Index].getFrame().getBounds().height();
			_txtArray1[_txtArray1Index].setPosition(Common::Point((320 - frameWidth) / 2, 200));

			NpcMover *mover2 = new NpcMover();
			Common::Point pt2(_txtArray1[_txtArray1Index]._position.x, 100);
			_txtArray1[_txtArray1Index].addMover(mover2, &pt2, this);

			_txtArray2[_txtArray1Index]._textMode = ALIGN_CENTER;
			_txtArray2[_txtArray1Index]._width = 240;
			_txtArray2[_txtArray1Index]._fontNumber = 2;
			_txtArray2[_txtArray1Index]._color1 = 23;

			msg = g_resourceManager->getMessage(8030, _lineNum++);
			_txtArray2[_txtArray1Index].setup(msg);
			_txtArray2[_txtArray1Index]._moveRate = 20;
			_txtArray2[_txtArray1Index]._moveDiff.y = 2;
			_txtArray2[_txtArray1Index].fixPriority(255);
			frameWidth = _txtArray2[_txtArray1Index].getFrame().getBounds().width();
			_txtArray2[_txtArray1Index].setPosition(Common::Point((320 - frameWidth) / 2, 200 + frameHeight));
		} else {
			// WORKAROUND: Fix inventory becoming available at end of credits
			g_globals->_events.setCursor(CURSOR_WALK);
			_actionIndex = 3;
			signal();
		}
		break;
	}
	case 2:
		setDelay(60);
		_actionIndex = 1;
		break;
	case 3:
		setDelay(7200);
		break;
	case 4:
		_txtArray1[0].remove();
		_txtArray1[1].remove();
		_txtArray2[0].remove();
		_txtArray2[1].remove();
		remove();
		break;
	default:
		break;
	}
}
void Scene9900::strAction2::dispatch() {
//	if (this->_txtArray1[0]._textSurface != 0) {
		int frameHeight = _txtArray1[0].getFrame().getBounds().height();
		_txtArray2[0]._position.y = frameHeight + _txtArray1[0]._position.y;
		_txtArray2[0]._flags |= OBJFLAG_PANES;
//	}
//	if (this->_txtArray1[1]._textSurface != 0) {
		frameHeight = _txtArray1[1].getFrame().getBounds().height();
		_txtArray2[1]._position.y = frameHeight + _txtArray1[1]._position.y;
		_txtArray2[1]._flags |= OBJFLAG_PANES;
//	}
	Action::dispatch();
}

void Scene9900::strAction2::synchronize(Serializer &s) {
	Action::synchronize(s);
	if (s.getVersion() >= 3) {
		s.syncAsSint16LE(_lineNum);
		s.syncAsSint16LE(_txtArray1Index);
		s.syncAsSint16LE(_var3);
	}
}

void Scene9900::strAction3::signal() {
	const byte mask3[3] = {0xff, 0, 0};
	const byte mask4[3] = {0, 0, 0};

	switch (_actionIndex++) {
	case 0:
		_palette2.getPalette();
		_palette3.loadPalette(2003);
		g_globals->_scenePalette.addFader(_palette3._palette, 256, 5, this);
		break;
	case 1:
		g_globals->_scenePalette.addFader(&mask3[0], 1, 10, this);
		break;
	case 2:
		g_globals->_scenePalette.addFader(&mask4[0], 1, 1, this);
		break;
	case 3:
		_palette2.loadPalette(17);
		g_globals->_sceneManager._scene->loadScene(17);
		g_globals->_scenePalette.addFader(_palette2._palette, 256, 5, this);
		break;
	case 4:
		g_globals->_game->endGame(9900, 61);
		remove();
	default:
		break;
	}
}

void Scene9900::signal() {
	if ((_sceneMode != 9913) && (_sceneMode != 9905) && (_sceneMode != 9904) && (_sceneMode != 9912)) {
		_object1.hide();
		_object2.hide();
		_object3.hide();
		_object4.hide();
		_object5.hide();
		_object6.hide();
	}

	_object1.animate(ANIM_MODE_NONE, NULL);
	_object2.animate(ANIM_MODE_NONE, NULL);
	_object3.animate(ANIM_MODE_NONE, NULL);
	_object4.animate(ANIM_MODE_NONE, NULL);
	_object5.animate(ANIM_MODE_NONE, NULL);
	_object6.animate(ANIM_MODE_NONE, NULL);

	_object1.setObjectWrapper(NULL);
	_object2.setObjectWrapper(NULL);
	_object3.setObjectWrapper(NULL);
	_object4.setObjectWrapper(NULL);
	_object5.setObjectWrapper(NULL);
	_object6.setObjectWrapper(NULL);

	_object1.addMover(NULL);
	_object2.addMover(NULL);
	_object3.addMover(NULL);
	_object4.addMover(NULL);
	_object5.addMover(NULL);
	_object6.addMover(NULL);

	switch (_sceneMode){
	case 150:
		g_globals->_soundHandler.play(380);
		_object8.postInit();
		_object8.setVisage(2002);
		_object8.setStrip(1);
		_object8.setFrame(1);
		_object8.fixPriority(200);
		_object8.setPosition(Common::Point(64, 199));
		g_globals->_player.disableControl();
		_sceneMode = 9908;
		setAction(&_sequenceManager, this, 9908, &_object1, &_object2, &_object3, &_object4, &_object5, &_object6);
		break;
	case 162:
		warning("TBC: shutdown();");
		g_globals->_game->quitGame();
		break;
	case 9901:
		g_globals->_player.disableControl();
		_sceneMode = 9906;
		setAction(&_sequenceManager, this, 9906, &_object1, &_object2, &_object3, &_object4, &_object5, &_object6);
		g_globals->_player._uiEnabled = true;
		g_globals->_events.setCursor(CURSOR_USE);
		break;
	case 9902:
		g_globals->_player.disableControl();
		_sceneMode = 9901;
		setAction(&_sequenceManager, this, 9901, &_object1, &_object2, &_object3, &_object4, &_object5, &_object6);
		break;
	case 9903:
		g_globals->_player.disableControl();
		_sceneMode = 9902;
		setAction(&_sequenceManager, this, 9902, &_object1, &_object2, &_object3, &_object4, &_object5, &_object6);
		break;
	case 9904:
		g_globals->_soundHandler.play(390);
		_sceneMode = 9912;
		setAction(&_strAction2, this);
		break;
	case 9905:
		_sceneMode = 150;
		setAction(&_strAction1, this);
		break;
	case 9906:
		if (_object8._state == 0) {
			g_globals->_player.disableControl();
			_sceneMode = 9913;
			setAction(&_sequenceManager, this, 9913, &_object1, &_object2, &_object3, &_object4, &_object5, &_object6);
		} else {
			g_globals->_player.disableControl();
			_sceneMode = 9905;
			setAction(&_sequenceManager, this, 9905, &_object1, &_object2, &_object3, &_object4, &_object5, &_object6);
		}
		break;
	case 9907:
		g_globals->_player.disableControl();
		_sceneMode = 9903;
		setAction(&_sequenceManager, this, 9903, &_object1, &_object2, &_object3, &_object4, &_object5, &_object6);
		break;
	case 9908:
		_object8.remove();
		g_globals->_player.disableControl();
		_sceneMode = 9904;
		setAction(&_sequenceManager, this, 9904, &_object1, &_object2, &_object3, &_object4, &_object5, &_object6);
		break;
	case 9909:
		g_globals->_soundHandler.play(375);
		g_globals->_player.disableControl();
		_sceneMode = 9907;
		setAction(&_sequenceManager, this, 9907, &_object1, &_object2, &_object3, &_object4, &_object5, &_object6);
		break;
	case 9910:
		g_globals->_player.disableControl();
		_sceneMode = 9911;
		setAction(&_sequenceManager, this, 9911, &_object1, &_object2, &_object3, &_object4, &_object5, &_object6);
		break;
	case 9911:
		g_globals->_soundHandler.play(367);
		g_globals->_player.disableControl();
		_sceneMode = 9909;
		setAction(&_sequenceManager, this, 9909, &_object1, &_object2, &_object3, &_object4, &_object5, &_object6);
		break;
	case 9912:
		g_globals->_player.disableControl();
		_sceneMode = 9912;
		setAction(&_sequenceManager, this, 9912, &_object1, &_object2, &_object3, &_object4, &_object5, &_object6);
		_sceneMode = 162;
		g_globals->_player.enableControl();
		g_globals->_player._canWalk = false;
		break;
	case 9913:
		_sceneMode = 200;
		setAction(&_strAction3, this);
		break;
	default:
		break;
	}
}

void Scene9900::process(Event &event) {
	if (event.handled)
		return;
	Scene::process(event);
	if (_sceneMode == 9906) {
		if ((event.eventType == EVENT_BUTTON_DOWN) && (g_globals->_events.getCursor() == OBJECT_ITEMS)) {
			_object8._state = 1;
			RING_INVENTORY._items._sceneNumber = 9900;
			g_globals->_events.setCursor(CURSOR_USE);
		}
	}
}

void Scene9900::dispatch() {
	if (_action)
		_action->dispatch();
}

void Scene9900::postInit(SceneObjectList *OwnerList) {
	_object1.postInit();
	_object1.hide();
	_object2.postInit();
	_object2.hide();
	_object3.postInit();
	_object3.hide();
	_object4.postInit();
	_object4.hide();
	_object5.postInit();
	_object5.hide();
	_object6.postInit();
	_object6.hide();

	_object8._state = 0;

	RING_INVENTORY._concentrator._sceneNumber = 9900;
	RING_INVENTORY._items._rlbNum = 3;
	RING_INVENTORY._items._cursorNum = 6;
	RING_INVENTORY._items._description = Common::String("One of the items from the stasis ship. The other is on the Lance's bridge.");

	_stripManager.addSpeaker(&_speakerMR);
	g_globals->_player.disableControl();
	_sceneMode = 9910;
	setAction(&_sequenceManager, this, 9910, &_object1, &_object2, &_object3, &_object4, &_object5, &_object6);
}

/*--------------------------------------------------------------------------
 * Scene 9999 - Space travel
 *
 *--------------------------------------------------------------------------*/

void Scene9999::Action1::signal() {
	switch (_actionIndex++) {
	case 0:
		setDelay(600);
		break;
	case 1:
		g_globals->_sceneManager.changeScene(3500);
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene9999::Action2::signal() {
	switch (_actionIndex++) {
	case 0:
		setDelay(10);
		break;
	case 1:
		SceneItem::display(9999, 0, SET_Y, 10, SET_X, 30, SET_FONT, 2, SET_BG_COLOR, -1, SET_EXT_BGCOLOR, 23, SET_WIDTH, 260, SET_KEEP_ONSCREEN, 1, LIST_END);
		setDelay(300);
		break;
	case 2:
		g_globals->_stripNum = 3600;
		g_globals->_sceneManager.changeScene(3600);
	default:
		break;
	}
}

void Scene9999::postInit(SceneObjectList *OwnerList) {
	loadScene(9998);
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);

	_object1.postInit();
	_object1.setVisage(1303);
	_object1.setStrip2(3);
	_object1.setPosition(Common::Point(160, 152));

	g_globals->_player.postInit();
	g_globals->_player.setVisage(1303);
	g_globals->_player.setStrip2(1);
	g_globals->_player.fixPriority(250);
	g_globals->_player.animate(ANIM_MODE_2, NULL);
	g_globals->_player.setPosition(Common::Point(194, 98));
	g_globals->_player._numFrames = 20;
	g_globals->_player.disableControl();

	_object2.postInit();
	_object2.setVisage(1303);
	_object2.setStrip2(2);
	_object2.fixPriority(2);
	_object2.setPosition(Common::Point(164, 149));

	_object3.postInit();
	_object3.setVisage(1303);
	_object3.setStrip2(2);
	_object3.fixPriority(2);
	_object3.setFrame(2);
	_object3.setPosition(Common::Point(292, 149));
	_object3.setAction(&_action3);

	if (g_globals->_sceneManager._previousScene == 3500)
		setAction(&_action2);
	else
		setAction(&_action1);

	g_globals->_sceneManager._scene->_sceneBounds.center(g_globals->_player._position.x, g_globals->_player._position.y);
	g_globals->_sceneManager._scene->_sceneBounds.contain(g_globals->_sceneManager._scene->_backgroundBounds);
	g_globals->_sceneOffset.x = (g_globals->_sceneManager._scene->_sceneBounds.left / 160) * 160;

	if (g_globals->_sceneManager._previousScene == 3500)
		g_globals->_stripNum = 2222;
	else
		g_globals->_stripNum = 2121;

	g_globals->_soundHandler.play(118);

}

} // End of namespace Ringworld

} // End of namespace TsAGE

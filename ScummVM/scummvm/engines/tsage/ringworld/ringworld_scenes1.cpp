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

#include "tsage/ringworld/ringworld_scenes1.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace TsAGE {

namespace Ringworld {

/*--------------------------------------------------------------------------
 * Scene 10 - Kziniti Palace (Introduction)
 *
 *--------------------------------------------------------------------------*/

void Scene10::Action1::signal() {
	Scene10 *scene = (Scene10 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(6);
		break;
	case 1:
		g_globals->_scenePalette.addRotation(240, 254, -1);
		scene->_stripManager.start(10, this);
		break;
	case 2:
		scene->_speakerSText.setTextPos(Common::Point(20, 20));
		scene->_speakerSText._color1 = 10;
		scene->_speakerSText._textWidth = 160;
		scene->_stripManager.start(11, this, scene);
		break;
	case 3:
		scene->_object2.hide();
		scene->_object3.hide();
		scene->_object3.setAction(NULL);
		scene->_object4.animate(ANIM_MODE_5, this);
		break;
	case 4:
	case 9:
		scene->_object1.animate(ANIM_MODE_5, this);
		break;
	case 5:
		scene->_object2.setStrip(3);
		scene->_object2.setFrame(1);
		scene->_object2.setPosition(Common::Point(240, 51));
		scene->_object2.show();

		scene->_object3.setStrip(6);
		scene->_object3.setFrame(1);
		scene->_object3.setPosition(Common::Point(200, 76));
		scene->_object3._numFrames = 20;
		scene->_object3.show();

		scene->_stripManager.start(12, this, scene);
		break;
	case 6:
		scene->_object2.hide();
		scene->_object3.hide();
		scene->_object1.animate(ANIM_MODE_6, this);
		break;
	case 7:
		scene->_object3.show();
		scene->_object3.setStrip2(5);
		scene->_object3._numFrames = 10;
		scene->_object3.setPosition(Common::Point(180, 87));
		scene->_object3.setAction(&scene->_action2);

		scene->_object2.setStrip(4);
		scene->_object2.setFrame(1);
		scene->_object2.setPosition(Common::Point(204, 59));
		scene->_object2.show();

		scene->_stripManager.start(13, this, scene);
		break;
	case 8:
		scene->_object2.hide();
		scene->_object3.hide();
		scene->_object4.animate(ANIM_MODE_6, this);
		break;
	case 10:
		g_globals->_soundHandler.fadeOut(this);
		break;
	case 11:
		g_globals->_scenePalette.clearListeners();
		g_globals->_sceneManager.changeScene(15);
		break;
	}
}

void Scene10::Action2::signal() {
	Scene10 *scene = (Scene10 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(g_globals->_randomSource.getRandomNumber(179));
		break;
	case 1:
		scene->_object3.setFrame(1);
		scene->_object3.animate(ANIM_MODE_5, this);
		_actionIndex = 0;
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene10::postInit(SceneObjectList *OwnerList) {
	loadScene(10);
	setZoomPercents(0, 100, 200, 100);

	_stripManager.addSpeaker(&_speakerSText);
	_stripManager.addSpeaker(&_speakerQText);
	_speakerSText._speakerName = "STEXT";
	_speakerQText._speakerName = "QTEXT";
	_speakerSText._hideObjects = false;
	_speakerQText._hideObjects = false;
	_speakerQText.setTextPos(Common::Point(140, 120));
	_speakerQText._color1 = 4;
	_speakerQText._textWidth = 160;
	_speakerSText.setTextPos(Common::Point(20, 20));
	_speakerSText._color1 = 7;
	_speakerSText._textWidth = 320;

	_stripManager.setCallback(this);

	_object1.postInit();
	_object1.setVisage(10);
	_object1.setPosition(Common::Point(232, 90));
	_object1.fixPriority(1);

	_object2.postInit();
	_object2.setVisage(10);
	_object2.setStrip(4);
	_object2.setFrame(1);
	_object2.setPosition(Common::Point(204, 59));
	_object2.fixPriority(198);

	_object3.postInit();
	_object3.setVisage(10);
	_object3.setStrip2(5);
	_object3.setPosition(Common::Point(180, 87));
	_object3.fixPriority(196);
	_object3.setAction(&_action2);

	_object4.postInit();
	_object4.setVisage(10);
	_object4.setStrip(2);
	_object4.setPosition(Common::Point(0, 209));
	_object4.animate(ANIM_MODE_1, NULL);

	_object5.postInit();
	_object5.setVisage(11);
	_object5.setPosition(Common::Point(107, 146));
	_object5.animate(ANIM_MODE_2, NULL);
	_object5._numFrames = 5;

	_object6.postInit();
	_object6.setVisage(11);
	_object6.setStrip(2);
	_object6.setPosition(Common::Point(287, 149));
	_object6.animate(ANIM_MODE_2, NULL);
	_object6._numFrames = 5;

	g_globals->_sceneManager._scene->_sceneBounds.contain(g_globals->_sceneManager._scene->_backgroundBounds);
	g_globals->_sceneOffset.x = (g_globals->_sceneManager._scene->_sceneBounds.left / 160) * 160;

	setAction(&_action1);
	g_globals->_soundHandler.play(5);
}

void Scene10::stripCallback(int v) {
	switch (v) {
	case 1:
		_object2.animate(ANIM_MODE_7, -1, NULL);
		break;
	case 2:
		_object2.animate(ANIM_MODE_NONE);
		break;
	case 3:
		_object2.animate(ANIM_MODE_7, -1, NULL);
		_object3.animate(ANIM_MODE_5, NULL);
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 15 - Outer Space (Introduction)
 *
 *--------------------------------------------------------------------------*/

void Scene15::Action1::signal() {
	Scene15 *scene = (Scene15 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(60);
		break;
	case 1:
		SceneItem::display(15, 0, SET_Y, 20, SET_FONT, 2, SET_BG_COLOR, -1, SET_EXT_BGCOLOR, 7,
				SET_WIDTH, 320, SET_KEEP_ONSCREEN, 1, LIST_END);
		setDelay(300);
		break;
	case 2: {
		SceneItem::display(15, 1, SET_Y, 20, SET_FONT, 2, SET_BG_COLOR, -1, SET_EXT_BGCOLOR, 7,
				SET_WIDTH, 320, SET_KEEP_ONSCREEN, 1, LIST_END);
		scene->_object1.postInit();
		scene->_object1.setVisage(15);
		scene->_object1.setPosition(Common::Point(160, -10));
		scene->_object1.animate(ANIM_MODE_2, NULL);
		Common::Point pt(160, 100);
		NpcMover *mover = new NpcMover();
		scene->_object1.addMover(mover, &pt, this);
		scene->_soundHandler.play(7);
		break;
	}
	case 3:
		SceneItem::display(0, 0);
		g_globals->_sceneManager.changeScene(20);
		break;
	}
}

void Scene15::Action1::dispatch() {
	Scene15 *scene = (Scene15 *)g_globals->_sceneManager._scene;

	if (scene->_object1._position.y < 100)
		scene->_object1.changeZoom(100 - scene->_object1._position.y);
	Action::dispatch();
}

/*--------------------------------------------------------------------------*/

void Scene15::postInit(SceneObjectList *OwnerList) {
	loadScene(15);
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);
	g_globals->_soundHandler.play(6);
	setAction(&_action1);
}

/*--------------------------------------------------------------------------
 * Scene 20 - Cut-scenes where House Chmeee is in the distance
 *
 *--------------------------------------------------------------------------*/

void Scene20::Action1::signal() {
	Scene20 *scene = (Scene20 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(120);
		break;
	case 1:
		scene->_stripManager.start(20, this);
		break;
	case 2:
		g_globals->_soundHandler.fadeOut(this);
		break;
	case 3:
		g_globals->_sceneManager._fadeMode = FADEMODE_GRADUAL;
		g_globals->_sceneManager.changeScene(30);	// First game scene
		break;
	default:
		break;
	}
}

void Scene20::Action2::signal() {
	Scene20 *scene = (Scene20 *)g_globals->_sceneManager._scene;
	NpcMover *npcMover;

	switch (_actionIndex++) {
	case 0:
		setDelay(10);
		break;
	case 1:
		SceneItem::display(20, 1, SET_WIDTH, 200, SET_Y, 20, SET_X, 160, SET_KEEP_ONSCREEN, true,
			SET_EXT_BGCOLOR, 4, LIST_END);
		setDelay(120);
		break;
	case 2: {
		NpcMover *mover = new NpcMover();
		Common::Point pt(455, 77);
		g_globals->_player.addMover(mover, &pt, this);
		ObjectMover2 *mover2 = new ObjectMover2();
		scene->_SceneObjectExt.addMover(mover2, 5, 10, &g_globals->_player);
		ObjectMover2 *mover3 = new ObjectMover2();
		scene->_sceneObject3.addMover(mover3, 10, 15, &g_globals->_player);
		break;
	}
	case 3: {
		npcMover = new NpcMover();
		Common::Point pt(557, 100);
		g_globals->_player.addMover(npcMover, &pt, this);
		break;
	}
	case 4: {
		npcMover = new NpcMover();
		Common::Point pt(602, 90);
		g_globals->_player.addMover(npcMover, &pt, this);
		break;
	}
	case 5: {
		npcMover = new NpcMover();
		Common::Point pt(618, 90);
		g_globals->_player.addMover(npcMover, &pt, this);
		break;
	}
	case 6: {
		npcMover = new NpcMover();
		Common::Point pt(615, 81);
		g_globals->_player.addMover(npcMover, &pt, this);
		break;
	}
	case 7: {
		npcMover = new NpcMover();
		Common::Point pt(588, 79);
		g_globals->_player.addMover(npcMover, &pt, this);
		break;
	}
	case 8:
		scene->_sound.release();
		g_globals->_soundHandler.fadeOut(this);
		break;
	case 9:
		SceneItem::display(0, 0, LIST_END);
		g_globals->_sceneManager._fadeMode = FADEMODE_GRADUAL;
		g_globals->_sceneManager.changeScene(40);
		break;
	default:
		break;
	}
}

void Scene20::Action3::signal() {
	Scene20 *scene = (Scene20 *)g_globals->_sceneManager._scene;
	NpcMover *npcMover;

	switch (_actionIndex++) {
	case 0:
		setDelay(120);
		break;
	case 1: {
		npcMover = new NpcMover();
		Common::Point pt(615, 81);
		g_globals->_player.addMover(npcMover, &pt, this);
		ObjectMover2 *mover1 = new ObjectMover2();
		scene->_SceneObjectExt.addMover(mover1, 5, 10, &g_globals->_player);
		ObjectMover2 *mover2 = new ObjectMover2();
		scene->_sceneObject3.addMover(mover2, 20, 25, &g_globals->_player);
		break;
	}
	case 2: {
		npcMover = new NpcMover();
		Common::Point pt(618, 90);
		g_globals->_player.addMover(npcMover, &pt, this);
		break;
	}
	case 3: {
		g_globals->_player._moveDiff = Common::Point(10, 10);
		scene->_SceneObjectExt._moveDiff = Common::Point(10, 10);
		scene->_sceneObject3._moveDiff = Common::Point(10, 10);
		npcMover = new NpcMover();
		Common::Point pt(445, 132);
		g_globals->_player.addMover(npcMover, &pt, this);
		break;
	}
	case 4: {
		npcMover = new NpcMover();
		Common::Point pt(151, 137);
		g_globals->_player.addMover(npcMover, &pt, this);
		break;
	}
	case 5: {
		npcMover = new NpcMover();
		Common::Point pt(-15, 137);
		g_globals->_player.addMover(npcMover, &pt, this);
		break;
	}
	case 6:
		scene->_sound.play(60, this, 127);
		g_globals->_soundHandler.release();
		break;
	case 7:
		g_globals->_sceneManager._fadeMode = FADEMODE_GRADUAL;
		g_globals->_sceneManager.changeScene(90);
		break;
	default:
		break;
	}
}

void Scene20::Action4::signal() {
	Scene20 *scene = (Scene20 *)g_globals->_sceneManager._scene;
	NpcMover *npcMover;

	switch (_actionIndex++) {
	case 0:
		setDelay(60);
		break;
	case 1: {
		npcMover = new NpcMover();
		Common::Point pt(486, 134);
		g_globals->_player.addMover(npcMover, &pt, this);
		ObjectMover2 *mover1 = new ObjectMover2();
		scene->_SceneObjectExt.addMover(mover1, 20, 35, &g_globals->_player);
		break;
	}
	case 2: {
		g_globals->_player._moveDiff = Common::Point(12, 12);
		scene->_SceneObjectExt._moveDiff = Common::Point(12, 12);
		NpcMover *mover1 = new NpcMover();
		Common::Point pt(486, 134);
		scene->_sceneObject3.addMover(mover1, &pt, this);
		NpcMover *mover2 = new NpcMover();
		pt = Common::Point(-15, 134);
		g_globals->_player.addMover(mover2, &pt, NULL);
		NpcMover *mover3 = new NpcMover();
		pt = Common::Point(-15, 134);
		scene->_SceneObjectExt.addMover(mover3, &pt, NULL);
		break;
	}
	case 3: {
		scene->_sceneObject3._moveDiff = Common::Point(20, 20);
		npcMover = new NpcMover();
		Common::Point pt(320, 134);
		scene->_sceneObject3.addMover(npcMover, &pt, this);
		break;
	}
	case 4: {
		scene->_sound.play(28);
		scene->_sceneObject4.postInit();
		scene->_sceneObject4.setVisage(21);
		scene->_sceneObject4.setStrip(3);
		scene->_sceneObject4.setPosition(Common::Point(scene->_sceneObject3._position.x - 36,
			scene->_sceneObject3._position.y - 1));
		scene->_sceneObject4._moveDiff.x = 48;

		ObjectMover3 *mover = new ObjectMover3();
		scene->_sceneObject4.addMover(mover, &scene->_SceneObjectExt, 4, this);
		break;
	}
	case 5: {
		scene->_sound.play(42);
		scene->_sceneObject4.remove();
		scene->_SceneObjectExt.setVisage(21);
		scene->_SceneObjectExt.setStrip(1);
		scene->_SceneObjectExt.setFrame(1);
		scene->_SceneObjectExt.animate(ANIM_MODE_5, NULL);

		scene->_SceneObjectExt._moveDiff.x = 4;
		NpcMover *mover1 = new NpcMover();
		Common::Point pt(scene->_SceneObjectExt._position.x - 12, scene->_SceneObjectExt._position.y + 5);
		scene->_SceneObjectExt.addMover(mover1, &pt, NULL);

		scene->_sceneObject5.postInit();
		scene->_sceneObject5.setVisage(21);
		scene->_sceneObject5.setStrip(3);
		scene->_sceneObject5.setPosition(Common::Point(scene->_sceneObject3._position.x - 36,
			scene->_sceneObject3._position.y - 1));
		scene->_sceneObject5._moveDiff.x = 48;

		ObjectMover3 *mover = new ObjectMover3();
		scene->_sceneObject5.addMover(mover, &g_globals->_player, 4, this);
		break;
	}
	case 6: {
		scene->_sound.play(42);
		scene->_SceneObjectExt.setStrip(2);
		scene->_SceneObjectExt.animate(ANIM_MODE_2, NULL);

		scene->_sceneObject5.remove();
		g_globals->_player.setVisage(21);
		g_globals->_player.setStrip(1);
		g_globals->_player.setFrame(1);
		g_globals->_player.animate(ANIM_MODE_5, this);
		g_globals->_player._moveDiff.x = 4;

		npcMover = new NpcMover();
		Common::Point pt(g_globals->_player._position.x - 25, g_globals->_player._position.y + 5);
		g_globals->_player.addMover(npcMover, &pt, this);
		break;
	}
	case 7:
		g_globals->_player.setStrip(2);
		g_globals->_player.animate(ANIM_MODE_2, NULL);
		scene->_sound.play(77, this, 127);
		break;
	case 8:
		g_globals->_game->endGame(20, 0);
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene20::Scene20() {
}

void Scene20::postInit(SceneObjectList *OwnerList) {
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);

	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerGameText);
	_speakerQText._npc = &g_globals->_player;

	if (g_globals->_sceneManager._previousScene == 30) {
		// Cut scene: Assassins are coming
		g_globals->_player.postInit();
		g_globals->_player.setVisage(20);
		g_globals->_player.setPosition(Common::Point(405, 69));
		g_globals->_player._moveDiff = Common::Point(10, 10);
		g_globals->_player.animate(ANIM_MODE_1, NULL);

		_SceneObjectExt.postInit();
		_SceneObjectExt.setVisage(20);
		_SceneObjectExt.setPosition(Common::Point(400, 69));
		_SceneObjectExt.animate(ANIM_MODE_1, NULL);

		_sceneObject3.postInit();
		_sceneObject3.setVisage(20);
		_sceneObject3.setPosition(Common::Point(395, 69));
		_sceneObject3.animate(ANIM_MODE_1, NULL);

		_SceneObjectExt._moveDiff = Common::Point(10, 10);
		_sceneObject3._moveDiff = Common::Point(10, 10);
		g_globals->_soundHandler.play(20);
		_sound.play(21);
		_sound.holdAt(true);
		setAction(&_action2);

		_sceneBounds = Rect(320, 0, 640, 200);
	} else if (g_globals->_sceneManager._previousScene == 60) {
		// Evasion
		_sound.play(30);
		g_globals->_player.postInit();
		g_globals->_player.setVisage(20);
		g_globals->_player.setPosition(Common::Point(588, 79));
		g_globals->_player._moveDiff = Common::Point(5, 5);
		g_globals->_player.fixPriority(50);
		g_globals->_player.animate(ANIM_MODE_1, NULL);

		_SceneObjectExt.postInit();
		_SceneObjectExt.setVisage(20);
		_SceneObjectExt.setPosition(Common::Point(583, 79));
		_SceneObjectExt.animate(ANIM_MODE_1, NULL);

		_sceneObject3.postInit();
		_sceneObject3.setVisage(20);
		_sceneObject3.setStrip2(2);
		_sceneObject3.setPosition(Common::Point(595, 79));
		_sceneObject3.animate(ANIM_MODE_1, NULL);

		if ((g_globals->getFlag(120) && g_globals->getFlag(116)) ||
				(g_globals->getFlag(117) && g_globals->getFlag(119))) {
			// Successful evasion
			setAction(&_action3);
		} else if (g_globals->getFlag(104)) {
			_sceneMode = 21;
			setAction(&_sequenceManager, this, 21, &g_globals->_player, &_SceneObjectExt, NULL);
		} else {
			// Failed evasion
			_sceneObject3._moveDiff = Common::Point(8, 8);
			setAction(&_action4);
		}
		_sceneBounds.center(g_globals->_player._position.x, g_globals->_player._position.y);
	} else {
		// Intro: Quinn looking at the monaster
		g_globals->_player.postInit();
		g_globals->_player.setVisage(2640);
		g_globals->_player.animate(ANIM_MODE_NONE, NULL);
		g_globals->_player.setStrip2(1);
		g_globals->_player.setFrame2(4);
		g_globals->_player.fixPriority(200);
		g_globals->_player.setPosition(Common::Point(425, 233));

		setAction(&_action1);
		_speakerQText.setTextPos(Common::Point(350, 20));
		_speakerQText._textWidth = 260;
		_speakerGameText.setTextPos(Common::Point(350, 20));
		_speakerGameText._textWidth = 260;

		g_globals->_soundHandler.play(8);
		_sceneBounds = Rect(320, 0, 640, 200);
	}

	g_globals->_player.disableControl();
	loadScene(20);
}

void Scene20::signal() {
	if (_sceneMode == 21)
		g_globals->_sceneManager.changeScene(90);
}

/*--------------------------------------------------------------------------
 * Scene 30 - First game scene (Outside Ch'mee house)
 *
 *--------------------------------------------------------------------------*/

void Scene30::BeamObject::doAction(int action) {
	if (action == OBJECT_SCANNER)
		display2(30, 14);
	else if (action == CURSOR_LOOK)
		display2(30, 2);
	else if (action == CURSOR_USE) {
		Scene30 *parent = (Scene30 *)g_globals->_sceneManager._scene;
		parent->setAction(&parent->_beamAction);
	} else
		SceneObject::doAction(action);
}

void Scene30::DoorObject::doAction(int action) {
	if (action == OBJECT_SCANNER)
		display2(30, 13);
	else if (action == CURSOR_LOOK)
		display2(30, 1);
	else if (action == CURSOR_USE)
		display2(30, 7);
	else
		SceneObject::doAction(action);
}

void Scene30::BeamAction::signal() {
	Scene30 *scene = (Scene30 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		// Disable control and move player to the doorway beam
		g_globals->_player.disableControl();
		NpcMover *mover = new NpcMover();
		Common::Point pt(114, 198);
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}

	case 1:
		// Perform the animation of player raising hand
		g_globals->_player.setVisage(31);
		g_globals->_player.setStrip(1);
		g_globals->_player.setFrame(1);
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;

	case 2:
		// Hide the beam and lower the player's hand
		scene->_sound.play(10, NULL, 127);
		g_globals->_player.animate(ANIM_MODE_6, this);
		scene->_beam.remove();
		break;

	case 3: {
		// Bring the Kzin to the doorway
		g_globals->_player.setVisage(0);
		g_globals->_player.animate(ANIM_MODE_1, NULL);
		g_globals->_player.setStrip(7);
		scene->_kzin.postInit();
		scene->_kzin.setVisage(2801);
		scene->_kzin.animate(ANIM_MODE_1, NULL);
		scene->_kzin.setObjectWrapper(new SceneObjectWrapper());
		scene->_kzin.setPosition(Common::Point(334, 1));
		NpcMover *mover = new NpcMover();
		Common::Point pt(158, 170);
		scene->_kzin.addMover(mover, &pt, this);
		g_globals->_sceneItems.push_front(&scene->_kzin);
		break;
	}

	case 4:
		// Open the door
		scene->_sound.play(11, NULL, 127);
		scene->_door.animate(ANIM_MODE_5, this);
		break;

	case 5:
		// Run the Kzin's talk sequence
		scene->_sound.play(13, NULL, 127);
		g_globals->_soundHandler.play(12, NULL, 127);
		scene->_stripManager.start((scene->_sceneMode == 0) ? 30 : 37, this);
		break;

	case 6:
		// Slight delay
		setDelay(3);
		break;

	case 7:
		// Re-activate player control
		scene->_sceneMode = 31;
		scene->_kzin.setAction(&scene->_kzinAction);
		g_globals->_player.enableControl();

		// End this action
		remove();
		break;

	default:
		break;
	}
}

void Scene30::KzinAction::signal() {
	Scene30 *scene = (Scene30 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(1200);
		break;
	case 1:
		g_globals->_soundHandler.fadeOut(NULL);
		g_globals->_player.disableControl();
		setAction(&scene->_sequenceManager, g_globals->_sceneManager._scene, 31, &scene->_kzin, &scene->_door, NULL);
		break;
	case 2:
		g_globals->_player.enableControl();
		remove();
		break;
	default:
		break;
	}
}

void Scene30::RingAction::signal() {
	Scene30 *scene = (Scene30 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		g_globals->_player.disableControl();
		scene->_kzin.setAction(NULL);
		NpcMover *mover = new NpcMover();
		Common::Point pt(114, 198);
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}

	case 1:
		g_globals->_player.checkAngle(&scene->_kzin);
		scene->_stripManager.start(32, this);
		break;

	case 2: {
		g_globals->_player.animate(ANIM_MODE_1, NULL);
		NpcMover *mover = new NpcMover();
		Common::Point pt(143, 177);
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}

	case 3:
		scene->_sound.play(11, NULL, 127);
		scene->_door.animate(ANIM_MODE_6, this);
		break;

	case 4: {
		scene->_sound.play(13, NULL, 127);
		NpcMover *kzinMover = new NpcMover();
		Common::Point pt(354, 5);
		scene->_kzin.addMover(kzinMover, &pt, this);
		NpcMover *playerMover = new NpcMover();
		pt = Common::Point(335, 36);
		g_globals->_player.addMover(playerMover, &pt, this);
		break;
	}

	case 5:
		break;

	case 6:
		g_globals->_sceneManager.changeScene(20);
		break;

	default:
		break;
	}
}

void Scene30::TalkAction::signal() {
	Scene30 *scene = (Scene30 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		g_globals->_player.disableControl();
		scene->_kzin.setAction(NULL);
		NpcMover *mover = new NpcMover();
		Common::Point pt(114, 198);
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 1:
		g_globals->_player.checkAngle(&scene->_kzin);
		scene->_stripManager.start(34, this);
		break;
	case 2:
		setDelay(5);
		break;
	case 3:
		scene->_kzin.setAction(&scene->_kzinAction);
		g_globals->_player.enableControl();
		remove();
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene30::KzinObject::doAction(int action) {
	Scene30 *scene = (Scene30 *)g_globals->_sceneManager._scene;

	switch (action) {
	case OBJECT_STUNNER:
		display2(30, 12);
		break;
	case OBJECT_SCANNER:
		display2(30, 11);
		break;
	case OBJECT_RING:
		RING_INVENTORY._ring._sceneNumber = 30;
		scene->setAction(&scene->_ringAction);
		break;
	case CURSOR_LOOK:
		display2(30, 6);
		break;
	case CURSOR_USE:
		display2(30, 10);
		break;
	case CURSOR_TALK:
		g_globals->_player.disableControl();
		scene->setAction(&scene->_talkAction);
		break;
	default:
		SceneObject::doAction(action);
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene30::Scene30() :
	_groundHotspot(9, OBJECT_SCANNER, 50, 17, CURSOR_LOOK, 30, 3, CURSOR_USE, 30, 8, LIST_END),
	_wallsHotspot(8, OBJECT_SCANNER, 50, 13, CURSOR_LOOK, 30, 0, CURSOR_USE, 30, 7, LIST_END),
	_courtyardHotspot(0, CURSOR_LOOK, 30, 4, LIST_END),
	_treeHotspot(10, OBJECT_SCANNER, 40, 39, CURSOR_LOOK, 30, 5, CURSOR_USE, 30, 9, LIST_END) {
}

void Scene30::postInit(SceneObjectList *OwnerList) {
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);

	// Add the speaker classes to the strip manager
	_stripManager.addSpeaker(&_speakerQL);
	_stripManager.addSpeaker(&_speakerSR);
	_stripManager.addSpeaker(&_speakerSText);
	_stripManager.addSpeaker(&_speakerQText);
	_speakerSText._npc = &_kzin;
	_speakerQText._npc = &g_globals->_player;


	// Setup player
	g_globals->_player.postInit();
	g_globals->_player.setVisage(0);
	g_globals->_player.animate(ANIM_MODE_1);
	g_globals->_player.setObjectWrapper(new SceneObjectWrapper());
	g_globals->_player.setStrip(7);
	g_globals->_player.setFrame(1);
	g_globals->_player.setPosition(Common::Point(114, 198));
	g_globals->_player.changeZoom(75);
	g_globals->_player.enableControl();

	// Set up beam object
	_beam.postInit();
	_beam.setVisage(31);
	_beam.setStrip(2);
	_beam.setPosition(Common::Point(124, 178));
	_beam.fixPriority(188);

	// Set up door object
	_door.postInit();
	_door.setVisage(30);
	_door.setPosition(Common::Point(150, 183));

	// Final processing and add of scene items
	_courtyardHotspot.setBounds(Rect(0, 0, 320, 200));

	// Add the objects and hotspots to the scene
	g_globals->_sceneItems.addItems(&_beam, &_wallsHotspot, &_door, &_treeHotspot, &_groundHotspot,
		&_courtyardHotspot, NULL);

	// Load the scene data
	loadScene(30);
	_sceneMode = 0;
}

void Scene30::signal() {
	if (_sceneMode == 31) {
		// Re-activate beam if the Kzin goes back inside
		_beam.postInit();
		_beam.setVisage(31);
		_beam.setStrip(2);
		_beam.setPosition(Common::Point(124, 178));
		_beam.fixPriority(188);
		g_globals->_sceneItems.push_front(&_beam);
		g_globals->_player.enableControl();
	} else if (_sceneMode == 32) {
		g_globals->_player.disableControl();
		_sceneMode = 31;
		setAction(&_sequenceManager, g_globals->_sceneManager._scene, 31, &_kzin, &_door, NULL);
	}
}

/*--------------------------------------------------------------------------
 * Scene 40 - Chmeee Home
 *
 *--------------------------------------------------------------------------*/

void Scene40::Action1::signal() {
	Scene40 *scene = (Scene40 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(120);
		break;
	case 1:
		g_globals->_events.setCursor(CURSOR_WALK);
		scene->_stripManager.start(40, this);
		break;
	case 2:
		scene->_doorway.postInit();
		scene->_doorway.setVisage(46);
		scene->_doorway.setPosition(Common::Point(305, 61));
		scene->_doorway.animate(ANIM_MODE_5, this);
		scene->_soundHandler.play(25);
		break;
	case 3:
		scene->_doorway.hide();
		scene->_dyingKzin.setPosition(Common::Point(296, 62));
		g_globals->_player.animate(ANIM_MODE_5, NULL);
		scene->_object1.setVisage(43);
		scene->_object1.setStrip(3);
		scene->_object1.animate(ANIM_MODE_5, NULL);
		scene->_object2.hide();
		scene->_object3.hide();
		scene->_stripManager.start(45, this);
		break;
	case 4:
		scene->_object2.remove();
		scene->_object3.remove();
		scene->_assassin.setVisage(42);
		scene->_assassin.setStrip(2);
		scene->_assassin.setFrame(1);
		scene->_assassin.setPosition(Common::Point(13, 171));
		scene->_assassin.animate(ANIM_MODE_5, this);
		scene->_soundHandler.play(25);
		break;
	case 5:
		scene->_doorway.show();
		scene->_doorway.setVisage(42);
		scene->_doorway.setStrip(3);
		scene->_doorway.setFrame(1);
		scene->_doorway.setPosition(Common::Point(41, 144));
		scene->_assassin.animate(ANIM_MODE_6, NULL);
		setDelay(6);
		break;
	case 6:
		scene->_doorway.setPosition(Common::Point(178, 101));
		setDelay(6);
		break;
	case 7:
		scene->_doorway.setPosition(Common::Point(271, 69));
		setDelay(6);
		break;
	case 8:
		scene->_doorway.remove();
		scene->_dyingKzin.animate(ANIM_MODE_5, this);
		break;
	case 9: {
		scene->_dyingKzin.setStrip(1);
		//Workaround: The original uses setFrame(1) but it's completely wrong.
		scene->_dyingKzin.setFrame(2);
		scene->_dyingKzin._moveDiff.y = 15;
		scene->_dyingKzin.animate(ANIM_MODE_5, NULL);
		Common::Point pt(223, 186);
		NpcMover *mover = new NpcMover();
		scene->_dyingKzin.addMover(mover, &pt, this);
		break;
	}
	case 10: {
		scene->_soundHandler.play(27);
		Common::Point pt(223, 184);
		NpcMover *mover = new NpcMover();
		scene->_dyingKzin.addMover(mover, &pt, this);
		break;
	}
	case 11: {
		Common::Point pt(223, 186);
		NpcMover *mover = new NpcMover();
		scene->_dyingKzin.addMover(mover, &pt, this);
		break;
	}
	case 12: {
		g_globals->_soundHandler.play(26);
		g_globals->_player._uiEnabled = true;
		scene->_assassin.setVisage(42);
		scene->_assassin.setPosition(Common::Point(4, 191));
		scene->_assassin.setStrip(1);
		scene->_assassin.animate(ANIM_MODE_1, NULL);
		Common::Point pt(230, 187);
		NpcMover *mover = new NpcMover();
		scene->_assassin.addMover(mover, &pt, this);
		break;
	}
	case 13:
		setDelay(180);
		break;
	case 14:
		scene->_assassin.setVisage(45);
		scene->_assassin.setStrip(1);
		scene->_assassin.setFrame(1);
		scene->_assassin.animate(ANIM_MODE_5, this);
		scene->_soundHandler.play(28);
		break;
	case 15:
		g_globals->_player.disableControl();
		scene->_object1.setVisage(40);
		scene->_object1.setStrip(4);
		scene->_object1.setFrame(1);
		scene->_object1.animate(ANIM_MODE_5, NULL);
		g_globals->_player.setVisage(40);
		g_globals->_player.setStrip(2);
		g_globals->_player.setFrame(1);
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 16:
		g_globals->_soundHandler.play(77, this);
		break;
	case 17:
		g_globals->_game->endGame(40, 20);
		remove();
		break;
	}
}

void Scene40::Action2::signal() {
	Scene40 *scene = (Scene40 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		if (scene->_assassin._position.x < 229)
			_actionIndex = 0;
		setDelay(1);
		break;
	case 1:
		scene->_assassin.animate(ANIM_MODE_NONE, NULL);
		g_globals->_player.setStrip(2);
		g_globals->_player.setFrame(1);
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 2: {
		scene->_soundHandler.play(28);
		scene->_doorway.postInit();
		scene->_doorway.setVisage(16);
		scene->_doorway.setStrip2(6);
		scene->_doorway.fixPriority(200);
		scene->_doorway.setPosition(Common::Point(159, 191));
		scene->_doorway._moveDiff = Common::Point(40, 40);
		scene->_doorway._moveRate = 60;
		scene->_doorway.animate(ANIM_MODE_5, NULL);

		Common::Point pt(271, 165);
		NpcMover *mover = new NpcMover();
		scene->_doorway.addMover(mover, &pt, this);
		break;
	}
	case 3:
		scene->_doorway.remove();
		scene->_assassin.setVisage(44);
		scene->_assassin._frame = 1;
		scene->_assassin.animate(ANIM_MODE_5, this);
		scene->_soundHandler.play(29);
		RING_INVENTORY._infoDisk._sceneNumber = 40;
		break;
	case 4:
		g_globals->_player.animate(ANIM_MODE_6, this);
		break;
	case 5: {
		g_globals->_player.setVisage(0);
		g_globals->_player.animate(ANIM_MODE_1, NULL);
		g_globals->_player.setStrip(1);
		Common::Point pt(230, 195);
		PlayerMover *mover = new PlayerMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 6: {
		g_globals->_player.setStrip(7);
		scene->_object1.setVisage(2806);
		scene->_object1.animate(ANIM_MODE_1, NULL);
		SceneObjectWrapper *wrapper = new SceneObjectWrapper();
		scene->_object1.setObjectWrapper(wrapper);
		Common::Point pt(200, 190);
		NpcMover *mover = new NpcMover();
		scene->_object1.addMover(mover, &pt, this);
		break;
	}
	case 7:
		scene->_stripManager.start(44, this);
		break;
	case 8: {
		Common::Point pt(170, 260);
		NpcMover *mover = new NpcMover();
		scene->_object1.addMover(mover, &pt, this);
		break;
	}
	case 9:
		scene->_dyingKzin.setAction(&scene->_action7);
		scene->_object1.remove();
		g_globals->_stripNum = 88;
		g_globals->_events.setCursor(CURSOR_WALK);
		g_globals->_player.enableControl();
		scene->_assassin.setAction(&scene->_action8);
		break;
	}
}

void Scene40::Action3::signal() {
	Scene40 *scene = (Scene40 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		g_globals->_player.setAction(NULL);
		g_globals->_stripNum = 99;
		g_globals->_player.disableControl();
		Common::Point pt(240, 195);
		NpcMover *mover = new NpcMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 1:
		g_globals->_player.setVisage(5010);
		g_globals->_player._strip = 2;
		g_globals->_player._frame = 1;
		g_globals->_player.animate(ANIM_MODE_4, 5, 1, this);
		break;
	case 2:
		scene->_assassin.setStrip(2);
		scene->_assassin.setFrame(1);
		RING_INVENTORY._infoDisk._sceneNumber = 1;
		g_globals->_player.animate(ANIM_MODE_6, this);
		break;
	case 3:
		g_globals->_player.setVisage(0);
		g_globals->_player.animate(ANIM_MODE_1, NULL);
		g_globals->_player.setStrip(7);
		g_globals->_stripNum = 88;
		g_globals->_player.enableControl();
		remove();
		break;
	}
}

void Scene40::Action4::signal() {
	switch (_actionIndex++) {
	case 0: {
		Common::Point pt(178, 190);
		NpcMover *mover = new NpcMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 1:
		g_globals->_stripNum = 88;
		g_globals->_player.enableControl();
		break;
	}
}

void Scene40::Action5::signal() {
	Scene40 *scene = (Scene40 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(g_globals->_randomSource.getRandomNumber(119) + 120);
		break;
	case 1:
		scene->_object2.animate(ANIM_MODE_8, 1, this);
		_actionIndex = 0;
	}
}

void Scene40::Action6::signal() {
	Scene40 *scene = (Scene40 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		scene->_object1.postInit();
		scene->_object1.setVisage(16);
		scene->_object1.setStrip2(6);
		scene->_object1._moveDiff = Common::Point(40, 40);
		scene->_object1.setPosition(Common::Point(313, 53));
		scene->_object1._moveRate = 60;

		Common::Point pt(141, 194);
		NpcMover *mover = new NpcMover();
		scene->_object1.addMover(mover, &pt, NULL);
		scene->_object1.animate(ANIM_MODE_5, NULL);

		scene->_doorway.postInit();
		scene->_doorway.setVisage(46);
		scene->_doorway.setPosition(Common::Point(305, 61));
		scene->_doorway.animate(ANIM_MODE_5, this);
		scene->_soundHandler.play(25);
		break;
	}
	case 1:
		scene->_soundHandler.play(28);
		scene->_doorway.setPosition(Common::Point(148, 74));
		scene->_doorway.setFrame(1);
		scene->_doorway.setStrip(2);
		scene->_doorway.animate(ANIM_MODE_5, this);
		break;
	case 2:
		remove();
		break;
	}
}

void Scene40::Action7::signal() {
	Scene40 *scene = (Scene40 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(g_globals->_randomSource.getRandomNumber(499) + 500);
		break;
	case 1:
		scene->_object7.postInit();
		scene->_object7.setVisage(46);

		if (g_globals->_randomSource.getRandomNumber(32767) >= 16384) {
			scene->_object7.setStrip(3);
			scene->_object7.setPosition(Common::Point(15, 185));
		} else {
			scene->_object7.setPosition(Common::Point(305, 61));
			scene->_object7.setFrame(15);
		}
		scene->_object7.animate(ANIM_MODE_5, this);
		scene->_soundHandler.play(25);
		break;
	case 2:
		scene->_object7.remove();
		_actionIndex = 0;
		setDelay(60);
		break;
	}
}

void Scene40::Action8::signal() {
	Scene40 *scene = (Scene40 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(300);
		break;
	case 1:
		g_globals->_player.disableControl();

		if ((g_globals->_player._position.y >= 197) || (g_globals->_player._visage)) {
			_actionIndex = 1;
			setDelay(30);
		} else {
			scene->_doorway.postInit();
			scene->_doorway.setVisage(16);
			scene->_doorway.setStrip2(6);
			scene->_doorway.fixPriority(200);
			scene->_doorway._moveRate = 60;

			if (g_globals->_player._position.x >= 145) {
				scene->_doorway.fixPriority(-1);
				scene->_doorway.setPosition(Common::Point(6, 157));
			} else {
				scene->_doorway.setPosition(Common::Point(313, 53));
			}

			scene->_doorway._moveDiff = Common::Point(40, 40);
			Common::Point pt(g_globals->_player._position.x, g_globals->_player._position.y - 18);
			NpcMover *mover = new NpcMover();
			scene->_doorway.addMover(mover, &pt, this);
			scene->_doorway.animate(ANIM_MODE_5, NULL);
		}
		break;
	case 2:
		scene->_doorway.remove();
		g_globals->_player.setVisage(40);
		g_globals->_player.setStrip(2);
		g_globals->_player.setFrame(1);
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 3:
		g_globals->_soundHandler.play(77, this);
		break;
	case 4:
		g_globals->_game->endGame(40, 45);
		remove();
		break;
	}
}

void Scene40::Action8::dispatch() {
	if (_action)
		_action->dispatch();

	if (_delayFrames) {
		uint32 frameNumber = g_globals->_events.getFrameNumber();
		if ((_startFrame + 60) < frameNumber) {
			--_delayFrames;
			_startFrame = frameNumber;

			if (_delayFrames <= 0) {
				_delayFrames = 0;
				signal();
			}
		}
	}
}

/*--------------------------------------------------------------------------*/

void Scene40::DyingKzin::doAction(int action) {
	switch (action) {
	case OBJECT_STUNNER:
		SceneItem::display2(40, 44);
		break;
	case OBJECT_SCANNER:
		SceneItem::display2(40, 43);
		break;
	case CURSOR_LOOK:
		SceneItem::display2(40, 12);
		break;
	case CURSOR_USE:
		SceneItem::display2(40, 18);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene40::Assassin::doAction(int action) {
	Scene40 *scene = (Scene40 *)g_globals->_sceneManager._scene;

	switch (action) {
	case OBJECT_STUNNER:
		if (scene->_assassin._visage == 44)
			SceneItem::display2(40, 21);
		else {
			g_globals->_player.disableControl();
			Common::Point pt(230, 187);
			NpcMover *mover = new NpcMover();
			addMover(mover, &pt, NULL);
			scene->setAction(&scene->_action2);
		}
		break;
	case OBJECT_SCANNER:
		SceneItem::display2(40, (scene->_assassin._visage == 44) ? 22 : 23);
		break;
	case CURSOR_LOOK:
		if (scene->_assassin._visage != 44)
			SceneItem::display2(40, 13);
		else
			SceneItem::display2(40, (RING_INVENTORY._infoDisk._sceneNumber == 1) ? 19 : 14);
		break;
	case CURSOR_USE:
		if (scene->_assassin._visage != 44)
			SceneItem::display2(40, 15);
		else if (RING_INVENTORY._infoDisk._sceneNumber == 1)
			SceneItem::display2(40, 19);
		else {
			g_globals->_player.disableControl();
			setAction(&scene->_action3);
		}
		break;
	case CURSOR_TALK:
		SceneItem::display2(40, 38);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene40::Item2::doAction(int action) {
	switch (action) {
	case OBJECT_STUNNER:
		SceneItem::display2(40, 35);
		g_globals->_events.setCursor(CURSOR_WALK);
		break;
	case OBJECT_SCANNER:
		SceneItem::display2(40, 34);
		break;
	case CURSOR_LOOK:
		SceneItem::display2(40, 8);
		break;
	case CURSOR_USE:
		SceneItem::display2(40, 36);
		break;
	case CURSOR_TALK:
		SceneItem::display2(40, 37);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene40::Item6::doAction(int action) {
	switch (action) {
	case OBJECT_STUNNER:
		SceneItem::display2(40, 25);
		g_globals->_events.setCursor(CURSOR_WALK);
		break;
	case OBJECT_SCANNER:
		SceneItem::display2(40, 42);
		break;
	case CURSOR_LOOK:
		SceneItem::display2(40, 6);
		break;
	case CURSOR_USE:
		SceneItem::display2(40, 36);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene40::Scene40() :
	_item1(2, OBJECT_SCANNER, 40, 24, OBJECT_STUNNER, 40, 25, CURSOR_LOOK, 40, 7, CURSOR_USE, 40, 16, LIST_END),
	_item3(5, OBJECT_SCANNER, 40, 28, OBJECT_STUNNER, 40, 27, CURSOR_LOOK, 40, 2, CURSOR_USE, 40, 30, LIST_END),
	_item4(6, OBJECT_SCANNER, 40, 31, OBJECT_STUNNER, 40, 32, CURSOR_LOOK, 40, 5, CURSOR_USE, 40, 33, LIST_END),
	_item5(0, CURSOR_LOOK, 40, 11, LIST_END),
	_item7(4, OBJECT_SCANNER, 40, 26, OBJECT_STUNNER, 40, 27, CURSOR_LOOK, 40, 9, CURSOR_USE, 40, 17, LIST_END),
	_item8(8, OBJECT_SCANNER, 40, 39, OBJECT_STUNNER, 40, 40, CURSOR_LOOK, 40, 3, CURSOR_USE, 40, 41, LIST_END) {
}

void Scene40::postInit(SceneObjectList *OwnerList) {
	loadScene(40);
	Scene::postInit();

	setZoomPercents(0, 100, 200, 100);
	g_globals->_stripNum = 99;

	_stripManager.addSpeaker(&_speakerQR);
	_stripManager.addSpeaker(&_speakerSL);
	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerSText);
	_stripManager.addSpeaker(&_speakerGameText);

	_speakerGameText._color1 = 9;
	_speakerGameText.setTextPos(Common::Point(160, 30));
	_speakerQText._npc = &g_globals->_player;
	_speakerSText._npc = &_object1;

	g_globals->_player.postInit();
	g_globals->_player.setVisage(0);
	g_globals->_player.animate(ANIM_MODE_1, NULL);
	g_globals->_player.setObjectWrapper(new SceneObjectWrapper());
	g_globals->_player.setPosition(Common::Point(130, 220));
	g_globals->_player.disableControl();

	if (g_globals->_sceneManager._previousScene == 20) {
		g_globals->_soundHandler.play(24);
		g_globals->_player.setVisage(43);

		_object1.postInit();
		_object1.setVisage(41);
		_object1.setPosition(Common::Point(105, 220));
		_object2.postInit();
		_object2.setVisage(41);
		_object2.setStrip(6);
		_object2.fixPriority(200);
		_object2.setPosition(Common::Point(94, 189));
		_object2.setAction(&_action5);

		_object3.postInit();
		_object3.setVisage(41);
		_object3.setStrip(5);
		_object3.fixPriority(205);
		_object3.setPosition(Common::Point(110, 186));
		_object3._numFrames = 2;
		_object3.animate(ANIM_MODE_8, NULL, NULL);

		_assassin.postInit();
		_assassin.setPosition(Common::Point(-40, 191));
		g_globals->_sceneItems.push_back(&_assassin);

		_dyingKzin.postInit();
		_dyingKzin.setVisage(40);
		_dyingKzin.setStrip(6);
		_dyingKzin.setPosition(Common::Point(-90, 65));
		_dyingKzin.fixPriority(170);

		setAction(&_action1);
	} else {
		_doorway.postInit();
		_doorway.setVisage(46);
		_doorway.setPosition(Common::Point(148, 74));
		_doorway.setStrip(2);
		_doorway.setFrame(_doorway.getFrameCount());

		_dyingKzin.postInit();
		_dyingKzin.setVisage(40);
		_dyingKzin.setPosition(Common::Point(205, 183));
		_dyingKzin.fixPriority(170);
		_dyingKzin._frame = 9;
		_dyingKzin.setAction(&_action7);

		_assassin.postInit();
		_assassin.setVisage(44);
		_assassin.setPosition(Common::Point(230, 187));
		_assassin.setAction(&_action8);

		if (RING_INVENTORY._infoDisk._sceneNumber == 40) {
			_assassin.setStrip(1);
			_assassin.setFrame(_assassin.getFrameCount());
		} else {
			_assassin.setStrip(2);
		}

		g_globals->_sceneItems.push_back(&_assassin);
		g_globals->_player.setPosition(Common::Point(170, 220));

		setAction(&_action4);
	}

	_item5.setBounds(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
	_item6._sceneRegionId = 3;
	_item2._sceneRegionId = 7;

	g_globals->_sceneItems.addItems(&_dyingKzin, &_item8, &_item1, &_item2, &_item3, &_item4,
			&_item6, &_item7, &_item5, NULL);
}

void Scene40::signal() {
	if (_sceneMode == 41)
		g_globals->_sceneManager.changeScene(50);
}

void Scene40::dispatch() {
	if ((g_globals->_stripNum == 88) && (g_globals->_player._position.y >= 197)) {
		g_globals->_player.disableControl();
		g_globals->_stripNum = 0;
		g_globals->_player.setAction(NULL);
		_sceneMode = 41;
		setAction(&_sequenceManager, this, 41, &g_globals->_player, NULL);

		if (g_globals->_sceneManager._previousScene == 20) {
			_dyingKzin.setAction(&_action6);
		}
	}

	Scene::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 50 - By Flycycles
 *
 *--------------------------------------------------------------------------*/

void Scene50::Action1::signal() {
	Scene50 *scene = (Scene50 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setAction(&scene->_sequenceManager, this, 54, &g_globals->_player, NULL);
		break;
	case 1:
		g_globals->_events.setCursor(CURSOR_WALK);
		scene->_stripManager.start(63, this);
		break;
	case 2:
		if (scene->_stripManager._field2E8 != 107) {
			g_globals->_player.enableControl();
			remove();
		} else {
			Common::Point pt(282, 139);
			NpcMover *mover = new NpcMover();
			g_globals->_player.addMover(mover, &pt, this);
		}
		break;
	case 3:
		g_globals->_stripNum = -1;
		g_globals->_sceneManager.changeScene(60);
		break;
	}
}

void Scene50::Action2::signal() {
	Scene50 *scene = (Scene50 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		scene->_stripManager.start(66, this);
		break;
	case 1: {
		Common::Point pt(141, 142);
		NpcMover *mover = new NpcMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 2:
		g_globals->_sceneManager.changeScene(40);
		remove();
		break;
	}
}

void Scene50::Action3::signal() {
	switch (_actionIndex++) {
	case 0: {
		g_globals->_player.disableControl();
		Common::Point pt(136, 185);
		NpcMover *mover = new NpcMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 1:
		g_globals->_sceneManager.changeScene(60);
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene50::Object1::doAction(int action) {
	Scene50 *scene = (Scene50 *)g_globals->_sceneManager._scene;

	switch (action) {
	case OBJECT_STUNNER:
		SceneItem::display2(50, 20);
		break;
	case OBJECT_SCANNER:
		SceneItem::display2(50, 19);
		break;
	case CURSOR_LOOK:
		SceneItem::display2(50, 4);
		break;
	case CURSOR_USE:
		SceneItem::display2(50, 21);
		break;
	case CURSOR_TALK:
		g_globals->_player.disableControl();
		scene->_sceneMode = 52;
		scene->setAction(&scene->_sequenceManager, scene, 52, NULL);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene50::Object2::doAction(int action) {
	Scene50 *scene = (Scene50 *)g_globals->_sceneManager._scene;

	switch (action) {
	case OBJECT_STUNNER:
		SceneItem::display2(50, 11);
		break;
	case OBJECT_SCANNER:
		SceneItem::display2(50, 10);
		break;
	case CURSOR_LOOK:
		SceneItem::display2(50, 1);
		break;
	case OBJECT_INFODISK:
	case CURSOR_USE:
		g_globals->_stripNum = 50;
		scene->setAction(&scene->_action3);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene50::Object3::doAction(int action) {
	Scene50 *scene = (Scene50 *)g_globals->_sceneManager._scene;

	switch (action) {
	case OBJECT_STUNNER:
		SceneItem::display2(50, 11);
		break;
	case OBJECT_SCANNER:
		SceneItem::display2(50, 10);
		break;
	case CURSOR_LOOK:
		SceneItem::display2(50, 1);
		break;
	case OBJECT_INFODISK:
	case CURSOR_USE:
		SceneItem::display2(50, 8);
		break;
	case CURSOR_TALK:
		g_globals->_player.disableControl();
		scene->_sceneMode = 52;
		scene->setAction(&scene->_sequenceManager, scene, 52, NULL);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene50::Object4::doAction(int action) {
	Scene50 *scene = (Scene50 *)g_globals->_sceneManager._scene;

	switch (action) {
	case OBJECT_STUNNER:
		SceneItem::display2(50, 11);
		break;
	case OBJECT_SCANNER:
		SceneItem::display2(50, 10);
		break;
	case CURSOR_LOOK:
		SceneItem::display2(50, 1);
		break;
	case OBJECT_INFODISK:
	case CURSOR_USE:
		g_globals->_player.disableControl();
		g_globals->_stripNum = 0;
		scene->_sceneMode = 51;
		scene->setAction(&scene->_sequenceManager, scene, 51, &g_globals->_player, NULL);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene50::Scene50() :
		_item0(0, CURSOR_LOOK, 50, 3, LIST_END),
		_item1(0, OBJECT_SCANNER, 50, 15, CURSOR_USE, 50, 16, CURSOR_LOOK, 50, 3, LIST_END),
		_item2(0, CURSOR_LOOK, 50, 7, LIST_END),
		_item3(8, OBJECT_STUNNER, 50, 14, OBJECT_SCANNER, 50, 13, CURSOR_LOOK, 50, 3, LIST_END),
		_item4(9, OBJECT_SCANNER, 40, 39, OBJECT_STUNNER, 40, 40, CURSOR_USE, 40, 41, CURSOR_LOOK, 50, 5, LIST_END),
		_item5(10, OBJECT_SCANNER, 50, 17, OBJECT_STUNNER, 50, 18, CURSOR_LOOK, 50, 6, CURSOR_USE, 30, 8, LIST_END) {

	_doorwayRect = Rect(80, 108, 160, 112);
}

void Scene50::postInit(SceneObjectList *OwnerList) {
	loadScene(50);
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);

	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerSText);

	g_globals->_player.postInit();
	g_globals->_player.setVisage(0);
	g_globals->_player.animate(ANIM_MODE_1, NULL);
	g_globals->_player.setObjectWrapper(new SceneObjectWrapper());
	g_globals->_player._canWalk = false;
	g_globals->_player.changeZoom(75);
	g_globals->_player._moveDiff.y = 3;

	if (g_globals->_sceneManager._previousScene == 40) {
		g_globals->_player.setPosition(Common::Point(128, 123));
	} else if (g_globals->_stripNum == 50) {
		g_globals->_player.setPosition(Common::Point(136, 185));
	} else {
		g_globals->_player.setPosition(Common::Point(270, 143));
	}

	_object2.postInit();
	_object2.setVisage(2331);
	_object2.setStrip(6);
	_object2.setPosition(Common::Point(136, 192));
	_object2.fixPriority(200);

	_object3.postInit();
	_object3.setVisage(2337);
	_object3.setStrip(6);
	_object3.setPosition(Common::Point(260, 180));
	_object3.fixPriority(200);

	_object4.postInit();
	_object4.setVisage(2331);
	_object4.setStrip(6);
	_object4.setPosition(Common::Point(295, 144));
	_object4.fixPriority(178);

	g_globals->_sceneItems.addItems(&_object2, &_object3, &_object4, NULL);

	if (!g_globals->getFlag(101)) {
		g_globals->_player.disableControl();
		g_globals->setFlag(101);
		setAction(&_action1);
	} else {
		g_globals->_player.enableControl();

		if (g_globals->_sceneManager._previousScene == 40) {
			g_globals->_player.disableControl();
			_sceneMode = 54;
			setAction(&_sequenceManager, this, 54, &g_globals->_player, NULL);
		}
	}

	_item0.setBounds(Rect(200, 0, 320, 200));
	g_globals->_sceneItems.addItems(&_item3, &_item4, &_item5, &_item0, NULL);
}

void Scene50::signal() {
	switch (_sceneMode) {
	case 51:
		g_globals->_sceneManager.changeScene(60);
		break;
	case 55:
		g_globals->_sceneManager.changeScene(40);
		break;
	case 52:
	case 54:
		g_globals->_player.enableControl();
		break;
	}
}

void Scene50::dispatch() {
	Scene::dispatch();

	if ((_sceneMode != 55) && _doorwayRect.contains(g_globals->_player._position)) {
		// Player in house doorway, start player moving to within
		g_globals->_player.disableControl();
		_sceneMode = 55;
		Common::Point pt(89, 111);
		NpcMover *mover = new NpcMover();
		g_globals->_player.addMover(mover, &pt, this);
	}
}

/*--------------------------------------------------------------------------
 * Scene 60 - Flycycle controls
 *
 *--------------------------------------------------------------------------*/

void Scene60::Action1::signal() {
	Scene60 *scene = (Scene60 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(3);
		break;
	case 1:
		scene->_floppyDrive.postInit();
		scene->_floppyDrive.setVisage(60);
		scene->_floppyDrive.setStrip(7);
		scene->_floppyDrive.setPosition(Common::Point(136, 65));
		scene->_floppyDrive.animate(ANIM_MODE_5, this);

		scene->_soundHandler1.play(35);
		break;
	case 2:
		scene->_redLights.postInit();
		scene->_redLights.setVisage(60);
		scene->_redLights.setPosition(Common::Point(199, 186));
		scene->_redLights.animate(ANIM_MODE_8, 0, NULL);
		scene->_redLights._numFrames = 5;

		scene->_controlButton.animate(ANIM_MODE_2, NULL);

		if (!g_globals->getFlag(83)) {
			scene->_message.postInit();
			scene->_message.setVisage(60);
			scene->_message.setStrip2(3);
			scene->_message.setFrame(2);
			scene->_message.setPosition(Common::Point(148, 85));
			scene->_message.animate(ANIM_MODE_2, NULL);
			scene->_message._numFrames = 5;

			g_globals->_sceneItems.push_front(&scene->_message);
			scene->_soundHandler2.play(38);
		}

		g_globals->_events.setCursor(CURSOR_USE);
		break;
	case 3:
		scene->_soundHandler2.play(37);
		scene->loadScene(65);
		scene->_message.remove();

		if (g_globals->_sceneObjects->contains(&scene->_redLights))
			scene->_redLights.remove();

		scene->_controlButton.remove();
		scene->_slaveButton.remove();
		scene->_masterButton.remove();
		scene->_item1.remove();
		scene->_item2.remove();

		scene->_nextButton.postInit();
		scene->_nextButton.setVisage(65);
		scene->_nextButton.setPosition(Common::Point(118, 197));

		scene->_prevButton.postInit();
		scene->_prevButton.setVisage(65);
		scene->_prevButton.setStrip(2);
		scene->_prevButton.setPosition(Common::Point(160, 197));

		scene->_exitButton.postInit();
		scene->_exitButton.setVisage(65);
		scene->_exitButton.setStrip(3);
		scene->_exitButton.setPosition(Common::Point(202, 197));

		scene->_rose.postInit();
		scene->_rose.setVisage(65);
		scene->_rose.setStrip(4);
		scene->_rose.setFrame(1);
		scene->_rose.setPosition(Common::Point(145, 165));

		g_globals->_sceneItems.push_front(&scene->_nextButton);
		g_globals->_sceneItems.push_front(&scene->_prevButton);
		g_globals->_sceneItems.push_front(&scene->_exitButton);
		setDelay(10);

		g_globals->_events.setCursor(CURSOR_USE);
		break;
	case 4:
		g_globals->setFlag(90);
		// Deliberate fall-through
	case 5:
	case 6:
	case 7:
		SceneItem::display(60, _actionIndex - 4, SET_Y, 40, SET_X, 25, SET_FONT, 75,
			SET_EXT_BGCOLOR, -1, SET_FG_COLOR, 34, SET_POS_MODE, 0,
			SET_WIDTH, 280, SET_KEEP_ONSCREEN, 1, LIST_END);
		g_globals->_events.setCursor(CURSOR_USE);
		break;
	case 9:
		g_globals->_player._uiEnabled = false;
		RING_INVENTORY._infoDisk._sceneNumber = 1;

		if (g_globals->_sceneObjects->contains(&scene->_message))
			scene->_message.remove();

		scene->_controlButton.animate(ANIM_MODE_NONE);
		scene->_controlButton.setFrame(1);
		scene->_redLights.remove();

		scene->_floppyDrive.postInit();
		scene->_floppyDrive.setVisage(60);
		scene->_floppyDrive.setStrip(7);
		scene->_floppyDrive.setPosition(Common::Point(136, 65));
		scene->_floppyDrive.setFrame(scene->_floppyDrive.getFrameCount());
		scene->_floppyDrive.animate(ANIM_MODE_6, this);

		scene->_soundHandler1.play(35);
		scene->_soundHandler3.stop();

		scene->_masterButton.setFrame(1);
		scene->_masterButton._state = 0;

		g_globals->clearFlag(103);
		g_globals->clearFlag(!g_globals->_stripNum ? 116 : 119);
		break;
	case 10:
		setDelay(60);
		break;
	case 11:
		g_globals->_player._uiEnabled = true;
		scene->_floppyDrive.remove();
		remove();
		break;
	case 8:
	default:
		break;
	}
}

void Scene60::Action2::signal() {
	Scene60 *scene = (Scene60 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
	case 2:
		setDelay(3);
		break;
	case 1:
		scene->_stripManager.start(66, this);
		break;
	case 3:
		g_globals->_sceneManager.changeScene(50);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene60::PrevObject::doAction(int action) {
	Scene60 *scene = (Scene60 *)g_globals->_sceneManager._scene;

	if (action == CURSOR_LOOK) {
		SceneItem::display2(60, 16);
	} else if (action == CURSOR_USE) {
		animate(ANIM_MODE_8, 1, NULL);

		if (scene->_action1.getActionIndex() > 5) {
			scene->_soundHandler3.play(36);
			scene->_action1.setActionIndex(scene->_action1.getActionIndex() - 2);
			scene->_action1.setDelay(1);
		}
	} else {
		SceneHotspot::doAction(action);
	}
}

void Scene60::NextObject::doAction(int action) {
	Scene60 *scene = (Scene60 *)g_globals->_sceneManager._scene;

	if (action == CURSOR_LOOK) {
		SceneItem::display2(60, 17);
	} else if (action == CURSOR_USE) {
		animate(ANIM_MODE_8, 1, NULL);

		if (scene->_action1.getActionIndex() < 8) {
			scene->_soundHandler3.play(36);
			scene->_action1.setDelay(1);
		}
	} else {
		SceneHotspot::doAction(action);
	}
}

void Scene60::ExitObject::doAction(int action) {
	Scene60 *scene = (Scene60 *)g_globals->_sceneManager._scene;

	if (action == CURSOR_LOOK) {
		SceneItem::display2(60, 18);
	} else if (action == CURSOR_USE) {
		scene->_soundHandler3.play(36);
		animate(ANIM_MODE_8, 1, NULL);
		scene->_nextButton.remove();
		scene->_prevButton.remove();
		scene->_exitButton.remove();
		scene->_rose.remove();

		SceneItem::display(0, 0);
		scene->loadScene(60);

		scene->_controlButton.postInit();
		scene->_controlButton.setVisage(60);
		scene->_controlButton.setStrip(5);
		scene->_controlButton.setPosition(Common::Point(233, 143));
		scene->_controlButton.animate(ANIM_MODE_2, NULL);

		scene->_slaveButton.postInit();
		scene->_slaveButton.setVisage(60);
		scene->_slaveButton.setStrip(8);
		scene->_slaveButton.setPosition(Common::Point(143, 125));

		scene->_masterButton.postInit();
		scene->_masterButton.setVisage(60);
		scene->_masterButton.setStrip(8);
		scene->_masterButton.setPosition(Common::Point(143, 105));

		g_globals->_sceneItems.push_front(&scene->_masterButton);
		g_globals->_sceneItems.push_front(&scene->_slaveButton);

		scene->_redLights.postInit();
		scene->_redLights.setVisage(60);
		scene->_redLights.setPosition(Common::Point(199, 186));
		scene->_redLights.animate(ANIM_MODE_8, 0, NULL);
		scene->_redLights._numFrames = 5;
		scene->_redLights.setAction(&scene->_sequenceManager, scene, 61, NULL);

		if (scene->_slaveButton._state)
			scene->_slaveButton.setFrame(2);
		if (scene->_masterButton._state)
			scene->_masterButton.setFrame(2);

		g_globals->_sceneItems.push_front(&scene->_item1);
		g_globals->_sceneItems.push_front(&scene->_controlButton);
		g_globals->_sceneItems.push_front(&scene->_slaveButton);
		g_globals->_sceneItems.push_front(&scene->_masterButton);
		g_globals->_sceneItems.push_back(&scene->_item2);

		g_globals->gfxManager()._font.setFontNumber(2);
		g_globals->_sceneText._fontNumber = 2;

		scene->_action1.setActionIndex(2);
		scene->_action1.setDelay(1);
		scene->_sceneMode = 9999;
		scene->signal();
	} else {
		SceneHotspot::doAction(action);
	}
}

void Scene60::MessageObject::doAction(int action) {
	Scene60 *scene = (Scene60 *)g_globals->_sceneManager._scene;

	if (action == CURSOR_LOOK) {
		SceneItem::display2(60, 9);
	} else if (action == CURSOR_USE) {
		scene->_action1.setDelay(1);
		g_globals->setFlag(83);
	} else {
		SceneHotspot::doAction(action);
	}
}

void Scene60::ControlObject::doAction(int action) {
	Scene60 *scene = (Scene60 *)g_globals->_sceneManager._scene;

	if (action == CURSOR_LOOK) {
		SceneItem::display2(60, 11);
	} else if (action == CURSOR_USE) {
		if (_animateMode == ANIM_MODE_NONE)
			SceneItem::display2(60, 14);
		else if (!scene->_slaveButton._state) {
			g_globals->_soundHandler.play(40);
			g_globals->_soundHandler.holdAt(true);
			g_globals->_sceneManager.changeScene(20);
		} else {
			scene->_sceneMode = 15;
			setAction(&scene->_sequenceManager, scene, 62, NULL);
		}
	} else {
		SceneHotspot::doAction(action);
	}
}

void Scene60::SlaveObject::doAction(int action) {
	Scene60 *scene = (Scene60 *)g_globals->_sceneManager._scene;

	if (action == CURSOR_LOOK) {
		SceneItem::display2(60, 8);
	} else if (action == CURSOR_USE) {
		if (scene->_masterButton._state)
			scene->_sceneMode = 19;
		else if (_state) {
			scene->_soundHandler3.stop();
			animate(ANIM_MODE_6, NULL);
			g_globals->clearFlag(102);
			g_globals->clearFlag(!g_globals->_stripNum ? 117 : 120);
			_state = 0;
			scene->_sceneMode = 9998;
		} else {
			scene->_soundHandler3.play(39);
			g_globals->setFlag(102);
			g_globals->setFlag(!g_globals->_stripNum ? 117 : 120);
			animate(ANIM_MODE_5, NULL);
			_state = 1;
			scene->_sceneMode = 9998;
		}

		setAction(&scene->_sequenceManager, scene, 62, NULL);
	} else {
		SceneHotspot::doAction(action);
	}
}

void Scene60::MasterObject::doAction(int action) {
	Scene60 *scene = (Scene60 *)g_globals->_sceneManager._scene;

	if (action == CURSOR_LOOK) {
		SceneItem::display2(60, 7);
	} else if (action == CURSOR_USE) {
		if (!scene->_controlButton._animateMode)
			scene->_sceneMode = 14;
		else if (scene->_slaveButton._state)
			scene->_sceneMode = 20;
		else if (_state) {
			scene->_soundHandler3.stop();
			animate(ANIM_MODE_6, NULL);
			_state = 0;
			g_globals->clearFlag(103);
			g_globals->clearFlag(!g_globals->_stripNum ? 116 : 119);
			scene->_sceneMode = 9998;
		} else {
			scene->_soundHandler3.play(39);
			animate(ANIM_MODE_5, NULL);
			_state = 1;
			g_globals->setFlag(103);
			g_globals->setFlag(!g_globals->_stripNum ? 116 : 119);
			scene->_sceneMode = 9998;
		}

		setAction(&scene->_sequenceManager, scene, 62, NULL);
	} else {
		SceneHotspot::doAction(action);
	}
}

void Scene60::FloppyDrive::doAction(int action) {
	Scene60 *scene = (Scene60 *)g_globals->_sceneManager._scene;

	if (action == CURSOR_LOOK) {
		SceneItem::display2(60, 13);
	} else if (action == CURSOR_USE) {
		g_globals->setFlag(!g_globals->_stripNum ? 118 : 121);
		scene->setAction(&scene->_action1);
	} else {
		SceneHotspot::doAction(action);
	}
}

/*--------------------------------------------------------------------------*/

void Scene60::Item1::doAction(int action) {
	Scene60 *scene = (Scene60 *)g_globals->_sceneManager._scene;

	switch (action) {
	case OBJECT_INFODISK:
		RING_INVENTORY._infoDisk._sceneNumber = 60;
		g_globals->setFlag(!g_globals->_stripNum ? 118 : 121);
		scene->_sceneMode = 0;
		scene->setAction(&scene->_action1);
		break;
	case CURSOR_LOOK:
		SceneItem::display2(60, 10);
		break;
	case CURSOR_USE:
		if (RING_INVENTORY._infoDisk._sceneNumber == 60) {
			if (g_globals->getFlag(118) && !g_globals->_stripNum) {
				g_globals->clearFlag(118);
				scene->setAction(&scene->_action1);
				scene->_action1.setActionIndex(9);
				scene->_action1.setDelay(1);
			}
			if (g_globals->getFlag(121) && !g_globals->_stripNum) {
				g_globals->clearFlag(121);
				scene->setAction(&scene->_action1);
				scene->_action1.setActionIndex(9);
				scene->_action1.setDelay(1);
			}
		} else if (RING_INVENTORY._infoDisk._sceneNumber == 1) {
			scene->_sceneMode = 0;
			setAction(&scene->_sequenceManager, scene, 62, NULL);
		} else {
			scene->setAction(&scene->_action2);
		}
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene60::Item::doAction(int action) {
	Scene60 *scene = (Scene60 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(60, _messageNum);
		break;
	case CURSOR_USE:
		scene->_sceneMode = _sceneMode;
		setAction(&scene->_sequenceManager, this, 62, NULL);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene60::Scene60() :
		_item2(0, 12, 12),
		_item3(8, 22, 23),
		_item4(9, 24, 25),
		_item5(10, 26, 27),
		_item6(11, 28, 29) {
}

void Scene60::postInit(SceneObjectList *OwnerList) {
	loadScene(60);
	Scene::postInit();
	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerSText);

	g_globals->_player._uiEnabled = true;
	g_globals->_events.setCursor(CURSOR_USE);

	_slaveButton.postInit();
	_slaveButton.setVisage(60);
	_slaveButton.setStrip(8);
	_slaveButton.setPosition(Common::Point(143, 125));
	_slaveButton._state = 0;

	_masterButton.postInit();
	_masterButton.setVisage(60);
	_masterButton.setStrip(8);
	_masterButton.setPosition(Common::Point(143, 105));
	_masterButton._state = 0;

	g_globals->_sceneItems.push_back(&_masterButton);
	g_globals->_sceneItems.push_back(&_slaveButton);

	_controlButton.postInit();
	_controlButton.setVisage(60);
	_controlButton.setStrip(5);
	_controlButton.setPosition(Common::Point(233, 143));
	g_globals->_sceneItems.push_back(&_controlButton);

	if (g_globals->_stripNum == -1) {
		g_globals->_stripNum = 0;
	} else {
		g_globals->_player.disableControl();
		_sceneMode = 9999;
		setAction(&_sequenceManager, this, 61, NULL);
	}

	_item1.setBounds(Rect(130, 55, 174, 70));
	_item2.setBounds(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));

	if (g_globals->_stripNum == 0) {
		if (g_globals->getFlag(117)) {
			_slaveButton._state = 1;
			_slaveButton.setFrame(2);
		}

		if (g_globals->getFlag(116)) {
			_masterButton._state = 1;
			_masterButton.setFrame(2);
		}

		if (g_globals->getFlag(118)) {
			_controlButton.animate(ANIM_MODE_2, NULL);

			_redLights.postInit();
			_redLights.setVisage(60);
			_redLights.setPosition(Common::Point(199, 186));
			_redLights.animate(ANIM_MODE_8, 0, NULL);

			_soundHandler1.play(35);

			if (!g_globals->getFlag(83)) {
				_message.postInit();
				_message.setVisage(60);
				_message.setStrip2(3);
				_message.setFrame(2);
				_message.setPosition(Common::Point(148, 85));
				_message.animate(ANIM_MODE_2, NULL);
				_message._numFrames = 5;
				g_globals->_sceneItems.push_front(&_message);

				_soundHandler2.play(38);
			}
		}
	} else {
		if (g_globals->getFlag(120)) {
			_slaveButton._state = 1;
			_slaveButton.setFrame(2);
		}

		if (g_globals->getFlag(119)) {
			_masterButton._state = 1;
			_masterButton.setFrame(2);
		}

		if (g_globals->getFlag(121)) {
			_controlButton.animate(ANIM_MODE_2, NULL);

			_redLights.postInit();
			_redLights.setVisage(60);
			_redLights.setPosition(Common::Point(199, 186));
			_redLights.animate(ANIM_MODE_8, 0, NULL);
			_redLights._numFrames = 5;

			_soundHandler1.play(35);

			if (!g_globals->getFlag(83)) {
				_message.postInit();
				_message.setVisage(60);
				_message.setStrip2(3);
				_message.setFrame(2);
				_message.setPosition(Common::Point(148, 85));
				_message.animate(ANIM_MODE_2, NULL);
				_message._numFrames = 5;
				g_globals->_sceneItems.push_front(&_message);

				_soundHandler2.play(38);
			}
		}
	}

	g_globals->_sceneItems.addItems(&_item3, &_item4, &_item5, &_item6,
		&_item1, &_item2, NULL);
}

void Scene60::signal() {
	if (_sceneMode != 0) {
		if (_sceneMode == 9998) {
			g_globals->_events.setCursor(CURSOR_USE);
		} else if (_sceneMode == 9999) {
			g_globals->_player._uiEnabled = true;
			g_globals->_events.setCursor(CURSOR_USE);

			_gfxButton.setText(EXIT_MSG);
			_gfxButton._bounds.center(160, 193);
			_gfxButton.draw();
			_gfxButton._bounds.expandPanes();
		} else {
			SceneItem::display2(60, _sceneMode);
			g_globals->_events.setCursor(CURSOR_USE);
		}
	}
}

void Scene60::process(Event &event) {
	Scene::process(event);

	if (_screenNumber == 60) {
		if (_gfxButton.process(event))
			g_globals->_sceneManager.changeScene(50);
	}
}

/*--------------------------------------------------------------------------
 * Scene 90 - Shipyard Entrance
 *
 *--------------------------------------------------------------------------*/

void Scene90::Action1::signal() {
	Scene90 *scene = (Scene90 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(10);
		break;
	case 1:
		g_globals->_scenePalette.addRotation(64, 72, -1);
		g_globals->_events.setCursor(CURSOR_WALK);
		scene->_stripManager.start(90, this);
		break;
	case 2:
		setDelay(2);
		break;
	case 3: {
		Common::Point pt(278, 191);
		NpcMover *mover = new NpcMover();
		scene->_object2.addMover(mover, &pt, this);
		break;
	}
	case 4:
		scene->_object2.setStrip(3);
		setDelay(2);
		break;
	case 5:
		scene->_soundHandler2.play(58);

		if (scene->_stripManager._field2E8 == 220)
			scene->_stripManager.start(91, this, scene);
		else {
			scene->_stripManager.start(g_globals->getFlag(104) ? 93 : 92, this, scene);
			_actionIndex = 7;
		}
		break;
	case 6:
		scene->_object2.animate(ANIM_MODE_NONE);
		g_globals->_player._uiEnabled = true;
		break;
	case 7:
		scene->_object2.animate(ANIM_MODE_NONE);
		g_globals->_soundHandler.play(56);
		scene->_object3.animate(ANIM_MODE_5, this);
		break;
	case 8: {
		Common::Point pt(215, 127);
		PlayerMover *mover = new PlayerMover();
		scene->_object5.addMover(mover, &pt, this);
		break;
	}
	case 9: {
		Common::Point pt1(215, 127);
		PlayerMover *mover1 = new PlayerMover();
		scene->_object1.addMover(mover1, &pt1, this);
		Common::Point pt2(86, 62);
		PlayerMover *mover2 = new PlayerMover();
		scene->_object5.addMover(mover2, &pt2, this);
		break;
	}
	case 10: {
		PlayerMover2 *mover = new PlayerMover2();
		scene->_object1.addMover(mover, 10, 15, &scene->_object5);

		if (!g_globals->getFlag(104)) {
			mover = new PlayerMover2();
			scene->_object4.addMover(mover, 10, 15, &scene->_object1);
		}
		setDelay(60);
		break;
	}
	case 11:
		g_globals->_soundHandler.play(57);
		g_globals->_soundHandler.play(68);
		scene->_object3.animate(ANIM_MODE_6, NULL);

		SceneItem::display(90, g_globals->getFlag(104) ? 15 : 14,
			SET_EXT_BGCOLOR, 13, SET_KEEP_ONSCREEN, -1, SET_X, 120, SET_Y, 20, LIST_END);
		break;
	case 12:
		SceneItem::display(0, 0);
		g_globals->_scenePalette.clearListeners();
		g_globals->_sceneManager.changeScene(95);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene90::Object1::doAction(int action) {
	Scene90 *scene = (Scene90 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(90, 7);
		break;
	case CURSOR_USE:
		g_globals->_player.disableControl();
		scene->_sceneMode = 97;
		setAction(&scene->_sequenceManager, scene, 97, this, NULL);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene90::Object2::doAction(int action) {
	Scene90 *scene = (Scene90 *)g_globals->_sceneManager._scene;

	switch (action) {
	case OBJECT_STUNNER:
	case CURSOR_USE:
		g_globals->_player.disableControl();
		scene->_object6.postInit();
		scene->_object6.setVisage(90);
		scene->_object6.setStrip(6);
		scene->_object6.setPosition(Common::Point(184, 210));
		scene->_object6.hide();

		scene->_sceneMode = 91;
		scene->_soundHandler1.play(59);
		scene->_soundHandler1.holdAt(true);
		scene->setAction(&scene->_sequenceManager, scene, 91, this, &scene->_object6, NULL);
		break;
	case CURSOR_LOOK:
		SceneItem::display2(90, 8);
		break;
	case CURSOR_TALK:
		g_globals->_player.disableControl();
		setAction(&scene->_sequenceManager, scene, 96, this, NULL);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene90::Scene90() :
		_item1(0, CURSOR_LOOK, 90, 9, LIST_END),
		_item2(0, CURSOR_LOOK, 90, 10, LIST_END),
		_item3(0, CURSOR_LOOK, 90, 11, LIST_END),
		_object3(OBJECT_STUNNER, 90, 13, CURSOR_LOOK, 90, 12, CURSOR_USE, 90, 16, LIST_END),
		_object4(CURSOR_LOOK, 90, 17, LIST_END),
		_object5(CURSOR_LOOK, 90, 18, CURSOR_USE, 90, 19, LIST_END) {
}

void Scene90::stripCallback(int v) {
	Scene90 *scene = (Scene90 *)g_globals->_sceneManager._scene;

	if (v == 1)
		scene->_object2.animate(ANIM_MODE_7, 0, NULL);
	else if (v == 2)
		scene->_object2.animate(ANIM_MODE_NONE);
}

void Scene90::postInit(SceneObjectList *OwnerList) {
	loadScene(90);
	Scene::postInit();

	setZoomPercents(70, 10, 180, 100);
	_stripManager.addSpeaker(&_speakerSText);
	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerMText);
	_stripManager.addSpeaker(&_speakerQL);
	_stripManager.addSpeaker(&_speakerSR);

	_speakerMText._npc = &_object2;
	_speakerQText._textWidth = 160;
	_speakerQText._npc = &_object5;
	_speakerSText._npc = &_object1;

	_object5.postInit();
	_object5.setVisage(2333);
	_object5.setObjectWrapper(new SceneObjectWrapper());
	_object5._strip = 7;
	_object5._moveDiff = Common::Point(22, 22);
	_object5.setPosition(Common::Point(151, 177));
	_object5.changeZoom(-1);
	g_globals->_sceneItems.push_back(&_object5);

	_object1.postInit();
	_object1.setVisage(2337);
	_object1.setObjectWrapper(new SceneObjectWrapper());
	_object1._strip = 4;
	_object1._moveDiff = Common::Point(20, 20);
	_object1.setPosition(Common::Point(212, 183));
	_object1.changeZoom(-1);
	g_globals->_sceneItems.push_back(&_object1);

	if (!g_globals->getFlag(104)) {
		_object4.postInit();
		_object4.setVisage(2331);
		_object4.setObjectWrapper(new SceneObjectWrapper());
		_object4._strip = 4;
		_object4._moveDiff = Common::Point(20, 20);
		_object4.setPosition(Common::Point(251, 207));
		_object4.changeZoom(-1);
		g_globals->_sceneItems.push_back(&_object4);
	}

	_object2.postInit();
	_object2.setVisage(90);
	_object2.animate(ANIM_MODE_1, NULL);
	_object2.setPosition(Common::Point(315, 185));
	_object2._strip = 2;
	g_globals->_sceneItems.push_back(&_object2);

	_object3.postInit();
	_object3.setVisage(90);
	_object3.animate(ANIM_MODE_1, NULL);
	_object3.setPosition(Common::Point(196, 181));
	_object3.fixPriority(175);
	g_globals->_sceneItems.push_back(&_object3);

	g_globals->_player.disableControl();
	g_globals->_soundHandler.play(55);
	_soundHandler1.play(52);
	_soundHandler1.holdAt(true);

	setAction(&_action1);

	_item3.setBounds(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
	_item1.setBounds(Rect(271, 65, 271, 186));
	_item2.setBounds(Rect(0, 17, 124, 77));

	g_globals->_sceneItems.addItems(&_item1, &_item2, &_item3, NULL);
}

void Scene90::signal() {
	switch (_sceneMode) {
	case 91:
		_sceneMode = 92;
		g_globals->_soundHandler.play(77, this);
		break;
	case 92:
		g_globals->_scenePalette.clearListeners();
		g_globals->_game->endGame(90, 6);
		break;
	case 96:
		g_globals->_player.enableControl();
		break;
	case 97:
		_stripManager._field2E8 = 0;
		_action1.setActionIndex(5);
		_action1.setDelay(1);
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 95 - Ship Close-up
 *
 *--------------------------------------------------------------------------*/

void Scene95::Action1::signal() {
	Scene95 *scene = (Scene95 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		_state = 6;
		setDelay(60);
		break;
	case 1:
		if (_state) {
			SceneItem::display(95, _state % 2, SET_FONT, 2,
				SET_EXT_BGCOLOR, -1, SET_EXT_BGCOLOR, 20,
				SET_WIDTH, 200, SET_KEEP_ONSCREEN, 1, SET_TEXT_MODE, 1, LIST_END);
			--_state;
			_actionIndex = 1;
		}
		setDelay(60);
		break;
	case 2: {
		scene->_soundHandler.play(66);
		scene->_object3._numFrames = 5;
		scene->_object3.animate(ANIM_MODE_5, NULL);
		SceneItem::display(0, 0);

		Common::Point pt1(5, 198);
		NpcMover *mover1 = new NpcMover();
		g_globals->_player.addMover(mover1, &pt1, this);

		Common::Point pt2(9, 190);
		NpcMover *mover2 = new NpcMover();
		scene->_object1.addMover(mover2, &pt2, NULL);
		break;
	}
	case 3: {
		scene->_soundHandler.play(21);

		Common::Point pt1(235, 72);
		PlayerMover *mover1 = new PlayerMover();
		g_globals->_player.addMover(mover1, &pt1, NULL);

		Common::Point pt2(235, 72);
		PlayerMover *mover2 = new PlayerMover();
		scene->_object1.addMover(mover2, &pt2, NULL);

		SceneItem::display(95, 2, SET_Y, 45, SET_FONT, 2,
			SET_BG_COLOR, -1, SET_EXT_BGCOLOR, 13, SET_WIDTH, 200,
			SET_KEEP_ONSCREEN, 1, LIST_END);
		setDelay(240);
		break;
	}
	case 4:
		scene->_object3.remove();

		SceneItem::display(95, 3, SET_Y, 45, SET_FONT, 2,
			SET_BG_COLOR, -1, SET_EXT_BGCOLOR, 35, SET_WIDTH, 200,
			SET_KEEP_ONSCREEN, 1, LIST_END);
		setDelay(240);
		break;
	case 5:
		SceneItem::display(95, 4, SET_Y, 45, SET_FONT, 2,
			SET_BG_COLOR, -1, SET_EXT_BGCOLOR, 35, SET_WIDTH, 200,
			SET_KEEP_ONSCREEN, 1, LIST_END);
		setDelay(240);
		break;
	case 6:
		setDelay(20);
		break;
	case 7: {
		SceneItem::display(0, 0);
		g_globals->_player.setVisage(92);
		g_globals->_player.setPosition(Common::Point(-25, 200));
		scene->_object1.setVisage(91);
		scene->_object1.setPosition(Common::Point(-22, 220));

		scene->_soundHandler.play(21);

		Common::Point pt1(5, 198);
		NpcMover *mover1 = new NpcMover();
		g_globals->_player.addMover(mover1, &pt1, this);

		Common::Point pt2(9, 190);
		NpcMover *mover2 = new NpcMover();
		scene->_object1.addMover(mover2, &pt2, NULL);
		break;
	}
	case 8: {
		Common::Point pt1(108, 112);
		PlayerMover *mover1 = new PlayerMover();
		g_globals->_player.addMover(mover1, &pt1, this);

		Common::Point pt2(108, 112);
		PlayerMover *mover2 = new PlayerMover();
		scene->_object1.addMover(mover2, &pt2, NULL);
		break;
	}
	case 9:
		g_globals->_sceneManager.changeScene(2300);
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene95::Scene95() {
}

void Scene95::postInit(SceneObjectList *OwnerList) {
	loadScene(95);
	Scene::postInit();
	setZoomPercents(100, 10, 200, 100);

	g_globals->_player.postInit();
	g_globals->_player.setVisage(2337);
	g_globals->_player.setObjectWrapper(new SceneObjectWrapper());
	g_globals->_player._strip = 4;
	g_globals->_player._moveDiff = Common::Point(30, 30);
	g_globals->_player.setPosition(Common::Point(-35, 200));
	g_globals->_player.changeZoom(-1);
	g_globals->_player.disableControl();

	_object1.postInit();
	_object1.setVisage(2333);
	_object1.setPosition(Common::Point(-22, 220));
	_object1.animate(ANIM_MODE_1, NULL);
	_object1.setObjectWrapper(new SceneObjectWrapper());
	_object1._moveDiff = Common::Point(30, 30);
	_object1.changeZoom(-1);

	_object3.postInit();
	_object3.setVisage(96);
	_object3.setPosition(Common::Point(29, 198));

	_soundHandler.play(67);
	setAction(&_action1);
}

/*--------------------------------------------------------------------------
 * Scene 6100 - Sunflower navigation sequence
 *
 *--------------------------------------------------------------------------*/

void Scene6100::Action1::signal() {
	Scene6100 *scene = (Scene6100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->showMessage(SCENE6100_CAREFUL, 13, this);
		break;
	case 1:
		scene->showMessage(SCENE6100_TOUGHER, 35, this);
		break;
	case 2:
		scene->showMessage(NULL, 0, NULL);
		remove();
		break;
	}
}

void Scene6100::Action2::signal() {
	Scene6100 *scene = (Scene6100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->showMessage(SCENE6100_ONE_MORE_HIT, 13, this);
		break;
	case 1:
		scene->showMessage(SCENE6100_DOING_BEST, 35, this);
		break;
	case 2:
		scene->showMessage(NULL, 0, NULL);
		remove();
		break;
	}
}

void Scene6100::Action3::signal() {
	Scene6100 *scene = (Scene6100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_speed = 0;
		setDelay(60);
		break;
	case 1:
		g_globals->_scenePalette.clearListeners();
		scene->_fadePercent = 100;
		g_globals->_scenePalette.refresh();
		scene->loadScene(9997);
		scene->_object1.hide();
		scene->_object2.hide();
		scene->_object3.hide();
		scene->_sunflower1.hide();
		scene->_sunflower2.hide();
		scene->_sunflower3.hide();
		scene->_rocks.hide();
		scene->_sceneText.hide();

		g_globals->_events.setCursor(CURSOR_WALK);
		scene->_stripManager.start(8120, this);
		break;
	case 2:
		scene->showMessage(SCENE6100_REPAIR, 7, this);
		break;
	case 3:
		scene->showMessage(NULL, 0, NULL);
		g_globals->_events.setCursor(CURSOR_WALK);
		scene->_stripManager.start(8130, this);
		break;
	case 4:
		g_globals->setFlag(76);
		g_globals->_sceneManager.changeScene(
			(scene->_stripManager._field2E8 == 135) ? 6100 : 2320);
		remove();
		break;
	}
}

void Scene6100::Action4::signal() {
	Scene6100 *scene = (Scene6100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->showMessage(SCENE6100_ROCKY_AREA, 13, this);
		break;
	case 1:
		scene->showMessage(SCENE6100_REPLY, 35, this);
		break;
	case 2:
		scene->showMessage(NULL, 0, NULL);
		remove();
		break;
	}
}

void Scene6100::Action5::dispatch() {
	Scene6100 *scene = (Scene6100 *)g_globals->_sceneManager._scene;
	FloatSet zeroSet;
	const double MULTIPLY_FACTOR = 0.01744;		// 2 * pi / 360

	if (scene->_turnAmount) {
		scene->_angle = (scene->_turnAmount + scene->_angle) % 360;

		for (int objIndex = 1; objIndex <= 3; ++objIndex) {
			SceneObject *obj = &scene->_object1;
			if (objIndex == 2) obj = &scene->_object2;
			if (objIndex == 3) obj = &scene->_object3;

			obj->_position.x += scene->_turnAmount * 2;
			if (obj->_position.x >= 320)
				obj->_position.x -= 480;
			if (obj->_position.x < -160)
				obj->_position.x += 480;
		}
	}

	scene->_object1._flags |= OBJFLAG_PANES;
	scene->_object2._flags |= OBJFLAG_PANES;
	scene->_object3._flags |= OBJFLAG_PANES;

	double distance = scene->_speed;
	double angle = (double)scene->_angle * MULTIPLY_FACTOR;
	scene->_probe._floats._float1 += sin(angle) * distance;
	scene->_probe._floats._float2 += cos(angle) * distance;

	for (int idx = 0; idx < 4; ++idx) {
		FloatSet tempSet = scene->_objList[idx]->_floats;
		tempSet.add(-scene->_probe._floats._float1, -scene->_probe._floats._float2,
			-scene->_probe._floats._float3);

		tempSet.proc1(scene->_angle * MULTIPLY_FACTOR);

		double sqrtVal = tempSet.sqrt(zeroSet);
		if (sqrtVal != 0.0) {
			scene->_objList[idx]->_position.y = static_cast<int>(13800.0 / sqrtVal + 62.0);
		}

		scene->_objList[idx]->_position.x = static_cast<int>(
			 160.0 - (330.0 / (tempSet._float2 + 330.0) * tempSet._float1));
		scene->_objList[idx]->dispatch();

		if (tempSet._float2 < 0) {
			scene->_objList[idx]->_position.y = 300;

			if (idx != 3) {
				scene->_objList[idx]->_floats._float1 =
					g_globals->_randomSource.getRandomNumber(199);
				scene->_objList[idx]->_floats._float2 =
					g_globals->_randomSource.getRandomNumber(999) + 750.0;

				scene->_objList[idx]->_floats.proc1(
					-(scene->_turnAmount * 10 + scene->_angle) * MULTIPLY_FACTOR);
				scene->_objList[idx]->_floats.add(scene->_probe._floats._float1,
					scene->_probe._floats._float2, scene->_probe._floats._float3);
			}
		}

		if (idx == 3) {
			scene->_rocksCheck = (ABS((int)tempSet._float1) < 100) && (tempSet._float2 > 0);
		}

		scene->_objList[idx]->_flags |= OBJFLAG_PANES;

		if ((idx != 3) && (scene->_fadePercent == 100) &&
				(tempSet.sqrt(zeroSet) < 150.0)) {
			switch (scene->_hitCount++) {
			case 0:
				scene->_soundHandler.play(233);
				scene->showMessage(NULL, 0, NULL);

				if (!g_globals->getFlag(76))
					scene->_probe.setAction(&scene->_action1);
				break;
			case 1:
				scene->_soundHandler.play(233);
				scene->showMessage(NULL, 0, NULL);

				if (!g_globals->getFlag(76))
					scene->_probe.setAction(&scene->_action2);
				break;
			case 2:
				scene->_soundHandler.play(234);
				scene->showMessage(NULL, 0, NULL);

				if (!g_globals->getFlag(76))
					scene->_probe.setAction(NULL);
				scene->setAction(&scene->_action3);
				break;
			}

			g_globals->_scenePalette.clearListeners();
			scene->_fadePercent = 0;
		}
	}
}

void Scene6100::GetBoxAction::signal() {
	Scene6100 *scene = (Scene6100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		scene->_turnAmount = 0;
		Common::Point pt(scene->_rocks._position.x, scene->_rocks._position.y + 10);
		ProbeMover *mover = new ProbeMover();
		scene->_probe.addMover(mover, &pt, NULL);
		scene->_probe.show();
		break;
	}
	case 1: {
		scene->showMessage(SCENE6100_TAKE_CONTROLS, 35, this);
		g_globals->_scenePalette.clearListeners();

		Common::Point pt(scene->_rocks._position.x, scene->_rocks._position.y - 10);
		NpcMover *mover = new NpcMover();
		scene->_probe.addMover(mover, &pt, NULL);
		break;
	}
	case 2:
		scene->_probe._percent = 4;
		scene->showMessage(SCENE6100_SURPRISE, 13, this);
		break;
	case 3:
		scene->showMessage(SCENE6100_SWEAT, 35, this);
		break;
	case 4:
		scene->showMessage(SCENE6100_VERY_WELL, 13, this);
		break;
	case 5:
		scene->showMessage(NULL, 0, NULL);
		g_globals->_sceneManager.changeScene(2320);
		remove();
	}
}

void Scene6100::GetBoxAction::dispatch() {
	Scene6100 *scene = (Scene6100 *)g_globals->_sceneManager._scene;

	if (!scene->_probe._mover && (scene->_getBoxAction._actionIndex >= 1)) {
		if (scene->_getBoxAction._actionIndex == 1) {
			scene->_speed = 0;
			scene->_getBoxAction.signal();
		}

		if (scene->_probe._percent > 4)
			// Handle the probe disappearing into the rocks
			scene->_probe._percent = scene->_probe._percent * 7 / 8;
		scene->_probe._flags |= OBJFLAG_PANES;
	}

	Action::dispatch();
}

void Scene6100::Action7::signal() {
	switch (_actionIndex++) {
	case 0:
		setDelay(30);
		break;
	case 1:
		setDelay(90);
		break;
	case 2:
		g_globals->_sceneManager.changeScene(2100);
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene6100::Object::synchronize(Serializer &s) {
	SceneObject::synchronize(s);

	// Save the double fields of the FloatSet
	s.syncAsDouble(_floats._float1);
	s.syncAsDouble(_floats._float2);
	s.syncAsDouble(_floats._float3);
	s.syncAsDouble(_floats._float4);
}

/*--------------------------------------------------------------------------*/

void Scene6100::ProbeMover::dispatch() {
	Scene6100 *scene = (Scene6100 *)g_globals->_sceneManager._scene;

	if (!dontMove()) {
		if (scene->_speed > 0) {
			scene->_action5.dispatch();
			scene->_speed = (scene->_speed * 4) / 5;
		}
	}

	NpcMover::dispatch();
}

/*--------------------------------------------------------------------------*/

void Scene6100::Item1::doAction(int action) {
	SceneItem::display2(4000, 0);
}

/*--------------------------------------------------------------------------*/

Scene6100::Scene6100(): Scene() {
	_objList[0] = &_sunflower1;
	_objList[1] = &_sunflower2;
	_objList[2] = &_sunflower3;
	_objList[3] = &_rocks;

	_speed = 30;
	_fadePercent = 100;
	_rocksCheck = false;
	_hitCount = 0;
	_turnAmount = 0;
	_angle = 0;
	_msgActive = false;

	g_globals->_sceneHandler->_delayTicks = 8;

	g_globals->_player.disableControl();
	g_globals->_events.setCursor(CURSOR_WALK);
}

void Scene6100::synchronize(Serializer &s) {
	Scene::synchronize(s);

	s.syncAsSint16LE(_speed);
	s.syncAsSint16LE(_fadePercent);
	s.syncAsByte(_rocksCheck);
	s.syncAsByte(_msgActive);
	s.syncAsSint16LE(_hitCount);
	s.syncAsSint16LE(_turnAmount);
	s.syncAsSint16LE(_angle);
}

void Scene6100::postInit(SceneObjectList *OwnerList) {
	loadScene(6100);
	Scene::postInit();
	setZoomPercents(62, 2, 200, 425);

	_stripManager.addSpeaker(&_speakerQR);
	_stripManager.addSpeaker(&_speakerSL);

	_object1.postInit();
	_object1.setVisage(6100);
	_object1._frame = 1;
	_object1._strip = 4;
	_object1.setPosition(Common::Point(0, 60));
	_object1.fixPriority(1);

	_object2.postInit();
	_object2.setVisage(6100);
	_object2._frame = 1;
	_object2._strip = 4;
	_object2.setPosition(Common::Point(160, 60));
	_object2.fixPriority(1);

	_object3.postInit();
	_object3.setVisage(6100);
	_object3._frame = 1;
	_object3._strip = 4;
	_object3.setPosition(Common::Point(320, 60));
	_object3.fixPriority(1);

	_rocks.postInit();
	_rocks.setVisage(6100);
	_rocks._frame = 1;
	_rocks._strip = 3;
	_rocks.setPosition(Common::Point(320, 0));
	_rocks.fixPriority(2);
	_rocks.changeZoom(-1);
	_rocks._floats._float1 = 320.0;
	_rocks._floats._float2 = 25000.0;
	_rocks._floats._float3 = 0.0;

	_probe.postInit();
	_probe._moveDiff = Common::Point(15, 15);
	_probe.setVisage(6100);
	_probe._frame = 1;
	_probe._strip = 5;
	_probe.setPosition(Common::Point(160, 260));
	_probe.fixPriority(3);
	_probe._floats._float1 = 320.0;
	_probe._floats._float2 = 0.0;
	_probe._floats._float3 = 0.0;
	_probe.hide();

	int baseVal = 2000;
	for (int idx = 0; idx < 3; ++idx) {
		_objList[idx]->_floats._float1 = g_globals->_randomSource.getRandomNumber(999);
		_objList[idx]->_floats._float2 = baseVal;
		_objList[idx]->_floats._float3 = 0.0;
		baseVal += g_globals->_randomSource.getRandomNumber(499);

		_objList[idx]->postInit();
		_objList[idx]->setVisage(6100);
		_objList[idx]->_frame = 1;
		_objList[idx]->_strip = 2;

		_objList[idx]->setPosition(Common::Point(
			g_globals->_randomSource.getRandomNumber(319), 60));
		_objList[idx]->fixPriority(1);
		_objList[idx]->changeZoom(-1);
	}

	setAction(&_action5);
	g_globals->_scenePalette.addRotation(96, 143, -1);

	if (!g_globals->getFlag(76))
		_probe.setAction(&_action4);

	g_globals->_soundHandler.play(231);
}

void Scene6100::remove() {
	g_globals->_player.disableControl();
	g_globals->_scenePalette.clearListeners();
	Scene::remove();
}

void Scene6100::process(Event &event) {
	Scene::process(event);

	if (event.eventType == EVENT_KEYPRESS) {
		// Handle incremental turning speeds with arrow keys
		if ((event.kbd.keycode == Common::KEYCODE_LEFT) || (event.kbd.keycode == Common::KEYCODE_KP4)) {
			_turnAmount = MIN(_turnAmount + 1, 8);
		} else if ((event.kbd.keycode == Common::KEYCODE_RIGHT) || (event.kbd.keycode == Common::KEYCODE_KP6)) {
			_turnAmount = MAX(_turnAmount - 1, -8);
		}
	}

	if (_probe._action)
		_probe._action->process(event);
}

void Scene6100::dispatch() {
	Scene::dispatch();

	if (_probe._action)
		_probe._action->dispatch();

	// Handle mouse controlling the turning
	int changeAmount = (g_globals->_events._mousePos.x - 160) / -20;
	_turnAmount += (changeAmount - _turnAmount) / 2;

	if (_fadePercent < 100) {
		_fadePercent += 10;
		if (_fadePercent >= 100) {
			g_globals->_scenePalette.addRotation(96, 143, -1);
			_fadePercent = 100;
		}

		byte adjustData[] = {0xff, 0xff, 0xff, 0};
		g_globals->_scenePalette.fade(adjustData, false, _fadePercent);
	}

	if (_action != &_action3) {
		// Display the distance remaining to the target
		int distance = (int)_probe._floats.sqrt(_rocks._floats);
		Common::String s = Common::String::format("%06d", distance);

		_sceneText.setPosition(Common::Point(24, 160));
		_sceneText._fontNumber = 0;
		_sceneText._color1 = 35;
		_sceneText.setup(s);
	}

	if (_rocksCheck && (_action == &_action5)) {
		// Check whether the probe is close enough to the rocks
		double distance = _probe._floats.sqrt(_rocks._floats);

		if ((distance >= 300.0) && (distance <= 500.0))
			setAction(&_getBoxAction);
	}
}

void Scene6100::showMessage(const Common::String &msg, int color, Action *action) {
	if (_msgActive) {
		_msgActive = false;
		_speaker1.removeText();
	}

	if (!msg.empty()) {
		_msgActive = true;
		_speaker1._textPos.x = 20;
		_speaker1._textWidth = 280;
		_speaker1._color1 = color;
		_speaker1._action = action;
		_speaker1.setText(msg);
	}
}

} // End of namespace Ringworld

} // End of namespace TsAGE

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
#include "tsage/ringworld/ringworld_scenes5.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace TsAGE {

namespace Ringworld {

/*--------------------------------------------------------------------------
 * Scene 4000 - Village
 *
 *--------------------------------------------------------------------------*/

void Scene4000::Action1::signal() {
	// Quinn has the peg. Everybody enter the screen.
	Scene4000 *scene = (Scene4000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		scene->_hotspot5.postInit();
		scene->_hotspot5.setVisage(2870);
		scene->_hotspot5.setObjectWrapper(new SceneObjectWrapper());
		scene->_hotspot5.animate(ANIM_MODE_1, NULL);
		scene->_hotspot5.setPosition(Common::Point(116, 160));

		Common::Point pt(208, 169);
		NpcMover *mover = new NpcMover();
		scene->_hotspot5.addMover(mover, &pt, this);

		RING_INVENTORY._ale._sceneNumber = 0;
		g_globals->clearFlag(42);
		g_globals->clearFlag(36);
		g_globals->clearFlag(43);
		g_globals->clearFlag(37);
		break;
	}
	case 1: {
		scene->_guardRock.postInit();
		scene->_guardRock.setVisage(4001);
		scene->_guardRock.animate(ANIM_MODE_1, NULL);
		scene->_guardRock.setObjectWrapper(new SceneObjectWrapper());
		scene->_guardRock.setPosition(Common::Point(314, 132));
		scene->_guardRock._moveDiff = Common::Point(4, 2);

		ADD_PLAYER_MOVER_NULL(scene->_guardRock, 288, 167);

		scene->_olo.postInit();
		scene->_olo.setVisage(4006);
		scene->_olo.animate(ANIM_MODE_1, NULL);
		scene->_olo.setStrip(1);
		scene->_olo.setPosition(Common::Point(207, 136));

		ADD_PLAYER_MOVER_NULL(scene->_olo, 220, 151);

		scene->_miranda.postInit();
		scene->_miranda.setVisage(2701);
		scene->_miranda.animate(ANIM_MODE_1, NULL);
		scene->_miranda.setObjectWrapper(new SceneObjectWrapper());
		scene->_miranda._moveDiff = Common::Point(4, 2);
		scene->_miranda.setPosition(Common::Point(300, 135));

		ADD_PLAYER_MOVER_THIS(scene->_miranda, 266, 169);

		ADD_PLAYER_MOVER_NULL(g_globals->_player, 241, 155);
		break;
	}
	case 2:
		scene->_stripManager.start(4400, this);
		break;
	case 3: {
		Common::Point pt1(30, 86);
		PlayerMover *mover1 = new PlayerMover();
		scene->_miranda.addMover(mover1, &pt1, this);

		ADD_PLAYER_MOVER_NULL(scene->_hotspot5, 3, 86);
		break;
	}
	case 4:
		ADD_MOVER(scene->_miranda, -30, 86);
		ADD_MOVER_NULL(scene->_hotspot5, -40, 86);
		break;
	case 5:
		g_globals->_soundHandler.play(155);
		g_globals->setFlag(43);
		g_globals->setFlag(114);
		scene->_stripManager.start(4430, this);
		break;
	case 6:
		ADD_PLAYER_MOVER_THIS(scene->_olo, 277, 175);
		ADD_PLAYER_MOVER_NULL(g_globals->_player, 258, 187);
		break;
	case 7:
		scene->_stripManager.start(4440, this);
		break;
	case 8:
		setDelay(30);
		break;
	case 9:
		g_globals->setFlag(96);
		g_globals->_sceneManager.changeScene(4025);
		break;
	}
}

void Scene4000::Action2::signal() {
	// Quinn, Seeker and Miranda walks down to the village
	// Then, they talk to Rock, and enter the priest hut
	Scene4000 *scene = (Scene4000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setAction(&scene->_sequenceManager1, this, 4001, &g_globals->_player, NULL);

		scene->_hotspot5.postInit();
		scene->_hotspot5.setVisage(2801);
		scene->_hotspot5.animate(ANIM_MODE_1, NULL);
		scene->_hotspot5.setObjectWrapper(new SceneObjectWrapper());
		scene->_hotspot5._moveDiff.x = 5;
		scene->_hotspot5.setPosition(Common::Point(-8, 88));

		scene->_lander.setAction(&scene->_sequenceManager3, NULL, 4003, &scene->_hotspot5, NULL);
		scene->_miranda.setAction(&scene->_sequenceManager2, NULL, 4002, &scene->_miranda, NULL);
		break;
	case 1:
		g_globals->_player.disableControl();

		scene->_lander.remove();
		ADD_MOVER(scene->_guardRock, scene->_hotspot5._position.x + 30, scene->_hotspot5._position.y - 10);
		break;
	case 2:
		g_globals->_player.checkAngle(&scene->_guardRock);
		scene->_hotspot5.checkAngle(&scene->_guardRock);
		scene->_miranda.checkAngle(&scene->_guardRock);
		scene->_stripManager.start(4000, this);
		break;
	case 3:
		scene->_hotspot2.setVisage(4017);
		scene->_hotspot2.animate(ANIM_MODE_1, NULL);
		scene->_hotspot2.setStrip(2);

		ADD_MOVER(scene->_hotspot2, 116, 160);
		ADD_MOVER(scene->_hotspot5, 116, 160);

		g_globals->setFlag(37);
		break;
	case 4:
		break;
	case 5:
		scene->_stripManager.start(4010, this);
		break;
	case 6:
		ADD_PLAYER_MOVER_NULL(scene->_guardRock, 230, 149);
		ADD_PLAYER_MOVER(210, 136);
		ADD_PLAYER_MOVER_NULL(scene->_miranda, 210, 133);
		break;
	case 7:
		g_globals->_sceneManager.changeScene(4045);
		break;
	}
}

void Scene4000::Action3::signal() {
	// The guard walks to the left and exits the screen
	Scene4000 *scene = (Scene4000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_hotspot8.setVisage(4017);
		scene->_hotspot8.setFrame2(-1);
		scene->_hotspot8.animate(ANIM_MODE_1, NULL);
		scene->_hotspot8.setObjectWrapper(new SceneObjectWrapper());
		ADD_MOVER(scene->_hotspot8, 118, 145);
		break;
	case 1:
		scene->_hotspot8.remove();
		remove();
		break;
	}
}

void Scene4000::Action4::signal() {
	// Quinn ties the rope to the rock
	Scene4000 *scene = (Scene4000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player._uiEnabled = false;
		ADD_MOVER(g_globals->_player, 257, 57);
		break;
	case 1:
		g_globals->_player.setVisage(4008);
		g_globals->_player.setPosition(Common::Point(258, 83));
		g_globals->_player._frame = 1;
		g_globals->_player._strip = 3;
		g_globals->_player.animate(ANIM_MODE_4, 2, 1, this);
		break;
	case 2:
		scene->_rope.postInit();
		scene->_rope.setVisage(4000);
		scene->_rope.setStrip(7);
		scene->_rope.setFrame(3);
		scene->_rope.setPosition(Common::Point(268, 44));

		RING_INVENTORY._rope._sceneNumber = 4000;
		g_globals->_events.setCursor(CURSOR_USE);
		g_globals->_player.animate(ANIM_MODE_6, this);
		break;
	case 3:
		g_globals->_player.setVisage(2602);
		g_globals->_player.setPosition(Common::Point(257, 57));
		g_globals->_player.animate(ANIM_MODE_1, NULL);
		g_globals->_player._uiEnabled = true;

		g_globals->setFlag(41);
		remove();
		break;
	}
}

void Scene4000::Action5::signal() {
	// Chat with Miranda
	Scene4000 *scene = (Scene4000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		setDelay(15);
		break;
	case 1:
		scene->_stripManager.start(g_globals->_stripNum, this);
		break;
	case 2:
		setDelay(10);
		break;
	case 3:
		scene->_hotspot8.setVisage(4017);
		scene->_hotspot8.animate(ANIM_MODE_1, NULL);
		scene->_hotspot8.setFrame2(-1);
		scene->_hotspot8.setAction(&scene->_action3);

		g_globals->_player.enableControl();
		remove();
		break;
	}
}

void Scene4000::Action6::signal() {
	// Quinn and Miranda enter the screen and walk to the village.
	// Rock comes and notices the alcohol. They all enter his hut.
	Scene4000 *scene = (Scene4000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		setDelay(30);
		break;
	case 1:
		setAction(&scene->_sequenceManager1, this, 4001, &g_globals->_player, NULL);

		if (!g_globals->getFlag(36))
			scene->_miranda.setAction(&scene->_sequenceManager2, NULL, 4002, &scene->_miranda, NULL);
		break;
	case 2:
		g_globals->_player.disableControl();
		ADD_MOVER(scene->_guardRock, g_globals->_player._position.x + 30, g_globals->_player._position.y - 5);
		break;
	case 3:
		scene->_stripManager.start(g_globals->getFlag(35) ? 4500 : 4502, this);
		break;
	case 4:
		g_globals->clearFlag(35);
		ADD_MOVER_NULL(scene->_guardRock, 292, 138);
		ADD_PLAYER_MOVER(283, 147);

		if (!g_globals->getFlag(36))
			ADD_PLAYER_MOVER_NULL(scene->_miranda, 280, 150);
		RING_INVENTORY._ale._sceneNumber = 4100;
		break;
	case 5:
		g_globals->_sceneManager.changeScene(4100);
		break;
	}
}

void Scene4000::Action7::signal() {
	// Climb down left Chimney using a rope
	Scene4000 *scene = (Scene4000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();

		scene->_rope.setFrame(1);
		ADD_MOVER(g_globals->_player, 247, 53);
		break;
	case 1:
		g_globals->_player.setVisage(4008);
		g_globals->_player.setStrip(4);
		g_globals->_player.setFrame(1);
		g_globals->_player.fixPriority(16);
		g_globals->_player.setPosition(Common::Point(260, 55));
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 2:
		g_globals->_sceneManager.changeScene(4050);
		break;
	}
}

void Scene4000::Action8::signal() {
	// Climb down right Chimney using a rope
	Scene4000 *scene = (Scene4000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		if (g_globals->getFlag(41))
			scene->_rope.setFrame(2);

		ADD_MOVER(g_globals->_player, 289, 53);
		break;
	case 1:
		g_globals->_player.setVisage(4008);
		g_globals->_player.setStrip(5);
		g_globals->_player.fixPriority(16);
		g_globals->_player.setFrame(1);
		g_globals->_player.setPosition(Common::Point(283, 52));
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 2:
		g_globals->_player.remove();
		setDelay(60);
		break;
	case 3:
		g_globals->_soundHandler.play(170);
		scene->_smoke2.setVisage(4000);
		scene->_smoke2.setStrip(6);
		scene->_smoke2.animate(ANIM_MODE_2, NULL);
		setDelay(60);
		break;
	case 4:
		g_globals->_soundHandler.play(77, this);
		break;
	case 5:
		g_globals->_game->endGame(4000, 15);
		remove();
		break;
	}
}

void Scene4000::Action9::signal() {
	// Villager animations
	switch (_actionIndex++) {
	case 0:
		setDelay(g_globals->_randomSource.getRandomNumber(119) + 240);
		break;
	case 1:
		static_cast<SceneObject *>(_owner)->animate(ANIM_MODE_8, 1, this);
		_actionIndex = 0;
		break;
	}
}

void Scene4000::Action10::signal() {
	// Villager animations
	switch (_actionIndex++) {
	case 0:
		setDelay(g_globals->_randomSource.getRandomNumber(119) + 240);
		break;
	case 1:
		static_cast<SceneObject *>(_owner)->animate(ANIM_MODE_8, 1, this);
		_actionIndex = 0;
		break;
	}
}

void Scene4000::Action11::signal() {
	Scene4000 *scene = (Scene4000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		ADD_MOVER(scene->_lander, -30, 70);
		break;
	case 1:
		setDelay(60);
		break;
	case 2:
		scene->_hotspot5.postInit();
		scene->_hotspot5.setVisage(2801);
		scene->_hotspot5.animate(ANIM_MODE_1, NULL);
		scene->_hotspot5.setObjectWrapper(new SceneObjectWrapper());
		scene->_hotspot5._moveDiff.x = 4;
		scene->_hotspot5.setPosition(Common::Point(-8, 88));

		setAction(&scene->_sequenceManager1, this, 4001, &g_globals->_player, NULL);

		scene->_miranda.setPosition(Common::Point(-210, 139));
		scene->_miranda.setAction(&scene->_sequenceManager2, NULL, 4002, &scene->_miranda, NULL);
		scene->_lander.setAction(&scene->_sequenceManager3, NULL, 4003, &scene->_hotspot5, NULL);
		break;
	case 3:
		scene->_stripManager.start(8000, this);
		break;
	case 4:
		ADD_MOVER(scene->_olo, 263, 187);
		scene->_olo.animate(ANIM_MODE_1, NULL);
		break;
	case 5:
		scene->_soundHandler1.stop();
		scene->_forceField.remove();

		ADD_MOVER(g_globals->_player, 340, 163);
		ADD_MOVER_NULL(scene->_miranda, 340, 169);
		ADD_MOVER_NULL(scene->_hotspot5, 340, 165);
		break;
	case 6:
		g_globals->_sceneManager.changeScene(4250);
		break;
	}
}

void Scene4000::Action12::signal() {
	// Quinn enter Rock's hut
	Scene4000 *scene = (Scene4000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		setDelay(5);
		break;
	case 1:
		g_globals->_events.setCursor(CURSOR_WALK);
		scene->_stripManager.start(4015, this);
		break;
	case 2:
		g_globals->setFlag(32);
		if (scene->_stripManager._field2E8 == 275) {
			g_globals->setFlag(82);
			ADD_MOVER_NULL(scene->_guardRock, 292, 138);
			ADD_PLAYER_MOVER(283, 147);
		} else {
			setDelay(30);
		}
		break;
	case 3:
		if (scene->_stripManager._field2E8 == 275) {
			g_globals->_sceneManager.changeScene(4100);
		} else {
			ADD_PLAYER_MOVER_THIS(scene->_guardRock, 300, 132);
		}
		break;
	case 4:
		scene->_guardRock.hide();
		scene->_stripManager.start(4020, this);
		break;
	case 5:
		g_globals->setFlag(35);
		g_globals->_player.enableControl();
		remove();
		break;
	}
}

void Scene4000::Action13::signal() {
	// Lander is landing
	Scene4000 *scene = (Scene4000 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		setDelay(3);
		break;
	case 1:
		scene->_soundHandler2.play(151);
		scene->_soundHandler2.holdAt(true);
		ADD_MOVER(scene->_lander, -30, 70);
		break;
	case 2:
		scene->_soundHandler2.release();
		g_globals->_sceneManager.changeScene(4010);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene4000::Miranda::doAction(int action) {
	Scene4000 *scene = (Scene4000 *)g_globals->_sceneManager._scene;

	switch (action) {
	case OBJECT_STUNNER:
		SceneItem::display2(4000, 31);
		break;
	case CURSOR_LOOK:
		SceneItem::display2(4000, 29);
		break;
	case CURSOR_TALK:
		if (g_globals->getFlag(31)) {
			if (!g_globals->getFlag(111)) {
				g_globals->setFlag(111);
				g_globals->_stripNum = 4070;
			} else if (!g_globals->getFlag(33))
				g_globals->_stripNum = 4094;
			else if (!g_globals->getFlag(112)) {
				g_globals->setFlag(112);
				g_globals->_stripNum = 4300;
			} else if (!g_globals->getFlag(113)) {
				g_globals->setFlag(113);
				g_globals->_stripNum = 4093;
			} else
				g_globals->_stripNum = 4094;
		} else {
			if (!g_globals->getFlag(33))
				g_globals->_stripNum = 4094;
			else if (!g_globals->getFlag(112)) {
				g_globals->setFlag(112);
				g_globals->_stripNum = 4300;
			} else
				g_globals->_stripNum = 4094;
		}

		scene->setAction(&scene->_action5);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene4000::Hotspot8::doAction(int action) {
	// Guard
	Scene4000 *scene = (Scene4000 *)g_globals->_sceneManager._scene;

	switch (action) {
	case OBJECT_STUNNER:
		SceneItem::display2(4000, 26);
		break;
	case CURSOR_LOOK:
		SceneItem::display2(4000, 25);
		break;
	case CURSOR_TALK:
		if (RING_INVENTORY._peg._sceneNumber == 1)
			SceneItem::display2(4000, 34);
		else {
			switch (_ctr) {
			case 0:
				g_globals->_stripNum = 4090;
				break;
			case 1:
				g_globals->_stripNum = 4091;
				break;
			case 2:
				g_globals->_stripNum = 4092;
				break;
			default:
				SceneItem::display2(4000, 34);
				break;
			}

			if (g_globals->_stripNum) {
				setAction(NULL);
				addMover(NULL);
				++_ctr;
				scene->setAction(&scene->_action5);
			}
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene4000::GuardRock::doAction(int action) {
	switch (action) {
	case OBJECT_STUNNER:
		SceneItem::display2(4000, 28);
		break;
	case CURSOR_LOOK:
		SceneItem::display2(4000, 27);
		break;
	case CURSOR_TALK:
		error("*** Do we need dialog.");
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene4000::Ladder::doAction(int action) {
	Scene4000 *scene = (Scene4000 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4000, 30);
		break;
	case CURSOR_USE:
		g_globals->_player.disableControl();
		if (g_globals->getFlag(40)) {
			scene->_sceneMode = 4005;
			scene->setAction(&scene->_sequenceManager1, scene, 4005, &g_globals->_player, NULL);
		} else {
			scene->_sceneMode = 4004;
			scene->setAction(&scene->_sequenceManager1, scene, 4004, &g_globals->_player, &scene->_ladder, NULL);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene4000::TheTech::doAction(int action) {
	Scene4000 *scene = (Scene4000 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4000, 13);
		break;
	case OBJECT_SCANNER:
		SceneItem::display2(4000, 19);
		break;
	case OBJECT_STUNNER:
		SceneItem::display2(4000, 20);
		break;
	case CURSOR_USE:
		if (g_globals->getFlag(40))
			SceneItem::display2(4000, 37);
		else {
			g_globals->_player.disableControl();
			if (g_globals->_sceneObjects->contains(&scene->_miranda))
				g_globals->clearFlag(96);

			scene->_sceneMode = 4012;
			scene->setAction(&scene->_sequenceManager1, scene, 4012, &g_globals->_player, NULL);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene4000::Hotspot13::doAction(int action) {
	// Rock between the two chimneys
	Scene4000 *scene = (Scene4000 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4000, 21);
		break;
	case OBJECT_ROPE:
		if (g_globals->getFlag(40))
			scene->setAction(&scene->_action4);
		else
			SceneItem::display2(4000, 22);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene4000::Hotspot::doAction(int action) {
	// Wall between the two doors
	Scene4000 *scene = (Scene4000 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4000, 16);
		break;
	case OBJECT_SCANNER:
		SceneItem::display2(4000, 17);
		break;
	case OBJECT_STUNNER:
		SceneItem::display2(4000, 18);
		break;
	case OBJECT_LADDER: {
		g_globals->_player.disableControl();

		scene->_ladder.postInit();
		scene->_ladder.setVisage(4000);
		scene->_ladder.setStrip(5);
		scene->_ladder.setPosition(Common::Point(245, 147));
		scene->_ladder.hide();
		g_globals->_sceneItems.push_front(&scene->_ladder);

		if (g_globals->_sceneObjects->contains(&scene->_hotspot8)) {
			scene->_hotspot8.setAction(NULL);
			Common::Point pt(118, 145);
			NpcMover *mover = new NpcMover();
			scene->_hotspot18.addMover(mover, &pt, NULL);
		}

		scene->_sceneMode = 4004;
		scene->setAction(&scene->_sequenceManager1, scene, 4011, &g_globals->_player, &scene->_ladder, NULL);
		break;
	}
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene4000::Hotspot17::doAction(int action) {
	// Left Chimney
	Scene4000 *scene = (Scene4000 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4000, 23);
		break;
	case CURSOR_USE:
		if (g_globals->getFlag(40))
			scene->setAction(&scene->_action7);
		else
			SceneItem::display2(4000, 24);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene4000::Hotspot18::doAction(int action) {
	// Right Chimney
	Scene4000 *scene = (Scene4000 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4000, 36);
		break;
	case CURSOR_USE:
		if (g_globals->getFlag(40))
			scene->setAction(&scene->_action8);
		else
			SceneItem::display2(4000, 24);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene4000::Hotspot23::doAction(int action) {
	// Door of the temple
	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4000, g_globals->getFlag(31) ? 10 : 9);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene4000::Scene4000() :
	_forceField(0, CURSOR_LOOK, 4000, 14, CURSOR_USE, 4000, 32, OBJECT_STUNNER, 4000, 33,
		OBJECT_SCANNER, 4000, 19, LIST_END),
	_hotspot19(0, CURSOR_LOOK, 4000, 7, LIST_END),
	_hotspot20(0, CURSOR_LOOK, 4000, 3, LIST_END),
	_hotspot21(0, CURSOR_LOOK, 4000, 1, LIST_END),
	_hotspot22(0, CURSOR_LOOK, 4000, 8, LIST_END),
	_hotspot24(0, CURSOR_LOOK, 4000, 11, LIST_END),
	_hotspot25(0, CURSOR_LOOK, 4000, 4, LIST_END),
	_hotspot26(0, CURSOR_LOOK, 4000, 0, LIST_END) {
}

void Scene4000::postInit(SceneObjectList *OwnerList) {
	loadScene(4000);
	Scene::postInit();
	setZoomPercents(0, 20, 70, 50);

	_stripManager.addSpeaker(&_speakerQR);
	_stripManager.addSpeaker(&_speakerML);
	_stripManager.addSpeaker(&_speakerMR);
	_stripManager.addSpeaker(&_speakerSR);
	_stripManager.addSpeaker(&_speakerCHFL);
	_stripManager.addSpeaker(&_speakerPL);
	_stripManager.addSpeaker(&_speakerPText);
	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerCHFR);
	_stripManager.addSpeaker(&_speakerQL);
	_stripManager.addSpeaker(&_speakerCHFText);
	_stripManager.addSpeaker(&_speakerSText);
	_stripManager.addSpeaker(&_speakerMText);

	_speakerCHFText._npc = &_guardRock;
	_speakerSText._npc = &_hotspot5;
	_speakerMText._npc = &_miranda;
	_speakerPText._npc = &_olo;
	_speakerQText._npc = &g_globals->_player;

	_hotspot13.setBounds(Rect(263, 41, 278, 55));
	_hotspot14.setBounds(Rect(96 /*140*/, 177, 140 /*96*/, 204));
	_hotspot15.setBounds(Rect(227, 101, 264, 143));
	_hotspot16.setBounds(Rect(306, 100, 319, 148));
	_hotspot17.setBounds(Rect(231, 53, 254, 60));
	_hotspot18.setBounds(Rect(285, 51, 310, 60));
	_hotspot26.setBounds(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
	_hotspot21.setBounds(Rect(28, 7, 53, 64));
	_hotspot22.setBounds(Rect(125, 155, 140, 189));
	_hotspot23.setBounds(Rect(205, 102, 220, 142));
	_hotspot24.setBounds(Rect(270, 111, 297, 147));

	_hotspot19._sceneRegionId = 16;
	_hotspot20._sceneRegionId = 18;
	_hotspot25._sceneRegionId = 17;

	_smoke1.postInit();
	_smoke1.setVisage(4000);
	_smoke1.fixPriority(1);
	_smoke1.setFrame(2);
	_smoke1.setPosition(Common::Point(242, 59));
	_smoke1.animate(ANIM_MODE_2, NULL);

	_smoke2.postInit();
	_smoke2.setVisage(4000);
	_smoke2.setStrip(2);
	_smoke2.fixPriority(1);
	_smoke2.setFrame(2);
	_smoke2.setPosition(Common::Point(299, 59));
	_smoke2.animate(ANIM_MODE_2, NULL);

	if (RING_INVENTORY._ladder._sceneNumber != 4000) {
	// if ladder is not in the scene, activate the hotspot on the wall
		_hotspot8.postInit();
		_hotspot8.setVisage(4018);
		_hotspot8.setObjectWrapper(new SceneObjectWrapper());
		_hotspot8._strip = 2;
		_hotspot8._numFrames = 5;
		_hotspot8.setPosition(Common::Point(306, 154));
		_hotspot8.setAction(&_action9);
	}

	_theTech.postInit();
	_theTech.setVisage(4000);
	_theTech.setStrip(3);
	_theTech.setFrame(3);
	_theTech.fixPriority(200);
	_theTech.setPosition(Common::Point(281, 176));

	if (g_globals->getFlag(34)) {
		_soundHandler1.play(156);

		_forceField.postInit();
		_forceField.setVisage(4000);
		_forceField.setStrip(4);
		_forceField.setPosition(Common::Point(312, 174));
		_forceField.fixPriority(200);
		_forceField.animate(ANIM_MODE_8, 0, NULL);

		g_globals->_sceneItems.push_back(&_forceField);
	}

	g_globals->_player.postInit();
	g_globals->_player.setVisage(2602);
	g_globals->_player.animate(ANIM_MODE_1, NULL);
	g_globals->_player.setObjectWrapper(new SceneObjectWrapper());
	g_globals->_player.setPosition(Common::Point(-28, 86));

	if (!g_globals->getFlag(36) && !g_globals->getFlag(43)) {
		_miranda.postInit();
		_miranda.setVisage(2701);
		_miranda.animate(ANIM_MODE_1, NULL);
		_miranda.setObjectWrapper(new SceneObjectWrapper());
		_miranda._moveDiff = Common::Point(4, 2);
		_miranda.setPosition(Common::Point(-210, 139));

		g_globals->_sceneItems.push_back(&_miranda);
	}

	g_globals->clearFlag(40);

	switch (g_globals->_sceneManager._previousScene) {
	case 2320:
		g_globals->_soundHandler.play(155);

		if (RING_INVENTORY._ale._sceneNumber == 1) {
			_guardRock.postInit();
			_guardRock.setVisage(4001);
			_guardRock.animate(ANIM_MODE_1, NULL);
			_guardRock.setObjectWrapper(new SceneObjectWrapper());
			_guardRock.setPosition(Common::Point(314, 132));

			setAction(&_action6);
		} else {
			g_globals->_player.disableControl();
			_sceneMode = 4001;
			setAction(&_sequenceManager1, this, 4001, &g_globals->_player, NULL);

			if (!g_globals->getFlag(36) && !g_globals->getFlag(43))
				_miranda.setAction(&_sequenceManager2, NULL, 4002, &_miranda, NULL);
		}

		if (g_globals->getFlag(42))
			_hotspot8.setAction(&_action3);
		else if (g_globals->getFlag(91))
			_hotspot8.remove();
		break;

	case 4010:
		_miranda.setPosition(Common::Point(-210, 139));

		_guardRock.postInit();
		_guardRock.setVisage(4001);
		_guardRock.animate(ANIM_MODE_1, NULL);
		_guardRock.setObjectWrapper(new SceneObjectWrapper());
		_guardRock.setPosition(Common::Point(314, 132));

		_hotspot2.postInit();
		_hotspot2.setVisage(4018);
		_hotspot2._strip = 1;
		_hotspot2._numFrames = 5;
		_hotspot2.setPosition(Common::Point(182, 146));
		_hotspot2.setAction(&_action10);

		setAction(&_action2);
		break;

	case 4025:
		if (RING_INVENTORY._ladder._sceneNumber == 4000)
			_hotspot8.remove();

		g_globals->_player.setPosition(Common::Point(260, 185));

		if (!g_globals->getFlag(36) && !g_globals->getFlag(43))
			_miranda.setPosition(Common::Point(246, 146));

		if (g_globals->getFlag(96)) {
			_olo.postInit();
			_olo.setVisage(4006);
			_olo.animate(ANIM_MODE_1, NULL);
			_olo.setObjectWrapper(new SceneObjectWrapper());
			_olo.setPosition(Common::Point(290, 163));
		}

		if (g_globals->_stripNum == 4025) {
			_soundHandler1.play(182);
			_forceField.remove();

			_hotspot5.postInit();
			_hotspot5.setVisage(2801);
			_hotspot5.animate(ANIM_MODE_1, NULL);
			_hotspot5.setObjectWrapper(new SceneObjectWrapper());
			_hotspot5._moveDiff.x = 4;
			_hotspot5.setPosition(Common::Point(-18, 86));

			g_globals->_player.disableControl();

			if (!g_globals->getFlag(96)) {
				_olo.postInit();
				_olo.setVisage(4006);
				_olo.animate(ANIM_MODE_1, NULL);
				_olo.setObjectWrapper(new SceneObjectWrapper());
				_olo.setPosition(Common::Point(195, 128));

				Common::Point pt(268, 157);
				PlayerMover *mover = new PlayerMover();
				_olo.addMover(mover, &pt, NULL);
			}

			_sceneMode = 4003;
			setAction(&_sequenceManager1, this, 4003, &_hotspot5, NULL);
		} else if (g_globals->getFlag(96)) {
			g_globals->_player.disableControl();
			_sceneMode = 4013;
			setAction(&_sequenceManager1, this, 4013, &_olo, NULL);
		}

		g_globals->clearFlag(96);
		break;

	case 4045:
		g_globals->_player.enableControl();

		if (RING_INVENTORY._ladder._sceneNumber != 4000) {
			_hotspot8.setVisage(4017);
			_hotspot8.animate(ANIM_MODE_1, NULL);
			_hotspot8.setPosition(Common::Point(199, 188));
			_hotspot8.setAction(&_action3);
		}

		g_globals->_player.setPosition(Common::Point(208, 153));
		if (!g_globals->getFlag(36) && !g_globals->getFlag(43))
			_miranda.setPosition(Common::Point(246, 146));

		if (g_globals->getFlag(39)) {
			// Ollo follows Quinn and gives explanations on the Tech.
			g_globals->clearFlag(39);

			_olo.postInit();
			_olo.setVisage(4006);
			_olo.animate(ANIM_MODE_1, NULL);
			_olo.setObjectWrapper(new SceneObjectWrapper());
			_olo.setPosition(Common::Point(219, 150));

			_sceneMode = 4010;
			g_globals->_player.disableControl();
			// This is the buggy animation where Miranda comments the Tech even
			// if she's not in the room but in the lander.
			setAction(&_sequenceManager1, this, 4010, &g_globals->_player, &_olo, NULL);
		}

		if (g_globals->_stripNum == 4000) {
			g_globals->_stripNum = 0;

			_guardRock.postInit();
			_guardRock.setVisage(4001);
			_guardRock.animate(ANIM_MODE_1, NULL);
			_guardRock.setObjectWrapper(new SceneObjectWrapper());
			_guardRock.setPosition(Common::Point(231, 159));
			_guardRock.setStrip(4);

			setAction(&_action12);
		}
		break;

	case 4050:
		g_globals->_soundHandler.play(155);
		g_globals->_player.disableControl();

		if (g_globals->_stripNum == 4050) {
			g_globals->_player.setVisage(4008);
			g_globals->_player.setStrip(4);
			g_globals->_player.setFrame(g_globals->_player.getFrameCount());
			g_globals->_player.fixPriority(16);
			g_globals->_player.setPosition(Common::Point(260, 55));

			_sceneMode = 4007;
			setAction(&_sequenceManager1, this, 4007, &g_globals->_player, NULL);
		} else {
			g_globals->_player.setPosition(Common::Point(208, 153));
			g_globals->_player.enableControl();
		}

		if (RING_INVENTORY._ladder._sceneNumber != 4000)
			_hotspot8.remove();
		break;

	case 4100:
		g_globals->_player.enableControl();
		g_globals->_player.setPosition(Common::Point(270, 155));

		if (g_globals->getFlag(42) && (RING_INVENTORY._ladder._sceneNumber != 4000)) {
			_hotspot8.setVisage(4017);
			_hotspot8.animate(ANIM_MODE_1, NULL);
			_hotspot8.setPosition(Common::Point(244, 151));
			_hotspot8.setAction(&_action3);
		}

		if (!g_globals->getFlag(36) && !g_globals->getFlag(43))
			_miranda.setPosition(Common::Point(246, 146));
		break;

	default:
		g_globals->_soundHandler.play(155);

		_lander.postInit();
		_lander.setVisage(4002);
		_lander._moveDiff = Common::Point(10, 10);
		_lander.setPosition(Common::Point(-100, 80));
		_lander.changeZoom(-1);
		_lander.setPosition(Common::Point(130, -1));
		_lander.animate(ANIM_MODE_2, NULL);

		if (g_globals->_stripNum == 9000) {
			_olo.postInit();
			_olo.setVisage(4006);
			_olo.setPosition(Common::Point(235, 153));

			_guardRock.postInit();
			_guardRock.setVisage(4001);
			_guardRock.setStrip(3);
			_guardRock.setPosition(Common::Point(255, 153));

			setAction(&_action11);
			RING_INVENTORY._ladder._sceneNumber = 4100;
			RING_INVENTORY._rope._sceneNumber = 4150;

			_soundHandler1.play(156);

			_forceField.postInit();
			_forceField.setVisage(4000);
			_forceField.setStrip(4);
			_forceField.setPosition(Common::Point(312, 174));
			_forceField.fixPriority(200);
			_forceField.animate(ANIM_MODE_8, 0, NULL);
		} else {
			if (!g_globals->getFlag(37)) {
				_hotspot2.postInit();
				_hotspot2.setVisage(4018);
				_hotspot2._strip = 1;
				_hotspot2._numFrames = 5;
				_hotspot2.setPosition(Common::Point(182, 146));
				_hotspot2.setAction(&_action10);
			}
			_miranda.setPosition(Common::Point(-210, 139));
			setAction(&_action13);
		}

		break;
	}

	if (RING_INVENTORY._ladder._sceneNumber == 4000) {
		_ladder.postInit();
		_ladder.setVisage(4000);
		_ladder.setStrip(5);
		_ladder.setPosition(Common::Point(245, 147));

		g_globals->_sceneItems.push_back(&_ladder);
	}

	if (RING_INVENTORY._rope._sceneNumber == 4000) {
		_rope.postInit();
		_rope.setVisage(4000);
		_rope.setStrip(7);
		_rope.fixPriority(1);
		_rope.setPosition(Common::Point(268, 44));
	}

	g_globals->_sceneItems.addItems(&_hotspot8, &_hotspot17, &_hotspot18, &_hotspot14, &_hotspot15,
		&_hotspot16, &_theTech, &_hotspot13, &_hotspot21, &_hotspot20, &_hotspot22, &_hotspot23,
		&_hotspot24, &_hotspot25, &_hotspot19, &_hotspot26, NULL);
}

void Scene4000::signal() {
	switch (_sceneMode) {
	case 4010:
		g_globals->setFlag(38);
		_olo.remove();
		// Deliberate fall-through
	case 4001:
		g_globals->_player.enableControl();
		break;
	case 4002:
	case 4011:
		break;
	case 4003:
		_sceneMode = 4014;
		setAction(&_sequenceManager1, this, 4014, &g_globals->_player, &_hotspot5, NULL);
		break;
	case 4004:
		RING_INVENTORY._ladder._sceneNumber = 4000;
		// Deliberate fall-through
	case 4007:
		g_globals->_player._uiEnabled = true;
		g_globals->_events.setCursor(CURSOR_USE);
		g_globals->setFlag(40);
		break;
	case 4005:
		g_globals->_player.enableControl();
		g_globals->_events.setCursor(CURSOR_WALK);
		g_globals->clearFlag(40);
		break;
	case 4006:
		g_globals->_sceneManager.changeScene(4045);
		break;
	case 4008:
		g_globals->_sceneManager.changeScene(2320);
		break;
	case 4009:
		g_globals->_sceneManager.changeScene(2200);
		break;
	case 4012:
		g_globals->_player.checkAngle(&_theTech);
		g_globals->_sceneManager.changeScene(4025);
		break;
	case 4013:
		g_globals->_player.enableControl();
		_olo.remove();
		break;
	case 4014:
		g_globals->_sceneManager.changeScene(4250);
		break;
	case 4015:
		ADD_MOVER_NULL(_miranda, 0, _miranda._position.y - 5);
		break;
	}
}

void Scene4000::dispatch() {
	Scene::dispatch();

	if ((g_globals->_player.getRegionIndex() == 10) || (g_globals->_player.getRegionIndex() == 6))
		g_globals->_player.fixPriority(200);
	if (g_globals->_player.getRegionIndex() == 11)
		g_globals->_player.fixPriority(-1);
	if (g_globals->_player.getRegionIndex() == 5)
		g_globals->_player.fixPriority(94);

	if (g_globals->_sceneObjects->contains(&_hotspot5)) {
		if ((_hotspot5.getRegionIndex() == 10) || (_hotspot5.getRegionIndex() == 6))
			_hotspot5.fixPriority(200);
		if (_hotspot5.getRegionIndex() == 11)
			_hotspot5.fixPriority(-1);
		if (_hotspot5.getRegionIndex() == 5)
			_hotspot5.fixPriority(94);
	}

	if (g_globals->_sceneObjects->contains(&_miranda)) {
		if (!_miranda._mover)
			_miranda.checkAngle(&g_globals->_player);
		if (!_action && g_globals->_player.getRegionIndex() == 23) {
			ADD_MOVER_NULL(_miranda, 204, 186);
		}

		if ((_miranda.getRegionIndex() == 10) || (_miranda.getRegionIndex() == 6))
			_miranda.fixPriority(200);
		if (_miranda.getRegionIndex() == 11)
			_miranda.fixPriority(-1);
		if (_miranda.getRegionIndex() == 5)
			_miranda.fixPriority(94);
	}

	if (!_action) {
		if ((RING_INVENTORY._peg._sceneNumber == 1) && g_globals->getFlag(34) &&
				g_globals->getFlag(37) && !g_globals->getFlag(40)) {
			g_globals->_player.disableControl();
			_soundHandler1.play(177);
			g_globals->_soundHandler.play(178);

			setAction(&_action1);
		}

		if (g_globals->_player.getRegionIndex() == 2)
			g_globals->_sceneManager.changeScene(4045);
		if (g_globals->_player.getRegionIndex() == 15)
			g_globals->_sceneManager.changeScene(4100);

		if ((g_globals->_player._position.x <= 5) && (g_globals->_player._position.y < 100)) {
			g_globals->_player.disableControl();

			if (!g_globals->_sceneObjects->contains(&_miranda) || (_miranda._position.y <= 100)) {
				_sceneMode = 4008;
				setAction(&_sequenceManager1, this, 4008, &g_globals->_player, NULL);
			} else {
				_sceneMode = 4015;
				g_globals->_player.addMover(NULL);
				setAction(&_sequenceManager1, this, 4015, &g_globals->_player, &_miranda, NULL);
			}
		}
	}
}

/*--------------------------------------------------------------------------
 * Scene 4010 - Village - Outside Lander
 *
 *--------------------------------------------------------------------------*/

void Scene4010::postInit(SceneObjectList *OwnerList) {
	loadScene(4010);
	Scene::postInit();
	setZoomPercents(0, 20, 70, 50);

	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerSText);
	_stripManager.addSpeaker(&_speakerMText);
	_speakerSText._npc = &_hotspot1;
	_speakerMText._npc = &_hotspot2;
	_speakerQText._npc = &g_globals->_player;

	g_globals->_player.postInit();
	g_globals->_player.setVisage(0);
	g_globals->_player.animate(ANIM_MODE_1, NULL);
	g_globals->_player.setObjectWrapper(new SceneObjectWrapper());
	g_globals->_player.setPosition(Common::Point(-38, 175));
	g_globals->_player.changeZoom(75);

	_hotspot2.postInit();
	_hotspot2.setVisage(2705);
	_hotspot2.animate(ANIM_MODE_1, NULL);
	_hotspot2.setObjectWrapper(new SceneObjectWrapper());
	_hotspot2._moveDiff = Common::Point(4, 2);
	_hotspot2.setPosition(Common::Point(-50, 185));
	_hotspot2.changeZoom(75);

	_hotspot1.postInit();
	_hotspot1.setVisage(2806);
	_hotspot1.setPosition(Common::Point(-20, 175));
	_hotspot1.changeZoom(75);
	_hotspot1.animate(ANIM_MODE_1, NULL);
	_hotspot1.setObjectWrapper(new SceneObjectWrapper());

	g_globals->_player.disableControl();
	setAction(&_sequenceManager, this, 4017, &g_globals->_player, &_hotspot1, &_hotspot2, NULL);
}

void Scene4010::signal() {
	g_globals->_sceneManager.changeScene(4000);
}

/*--------------------------------------------------------------------------
 * Scene 4025 - Village - Puzzle Board
 *
 *--------------------------------------------------------------------------*/

void Scene4025::Action1::signal() {
	Scene4025 *scene = (Scene4025 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_armHotspot._strip = scene->_pegPtr->_armStrip;
		scene->_armHotspot._frame = 4;
		scene->_armHotspot.animate(ANIM_MODE_4, 2, -1, this);

		if (scene->_pegPtr->_armStrip > 3) {
			if (scene->_hole1._armStrip == scene->_pegPtr->_armStrip)
				scene->_hole1._pegPtr = NULL;
			if (scene->_hole2._armStrip == scene->_pegPtr->_armStrip)
				scene->_hole2._pegPtr = NULL;
			if (scene->_hole3._armStrip == scene->_pegPtr->_armStrip)
				scene->_hole3._pegPtr = NULL;
			if (scene->_hole4._armStrip == scene->_pegPtr->_armStrip)
				scene->_hole4._pegPtr = NULL;
			if (scene->_hole5._armStrip == scene->_pegPtr->_armStrip)
				scene->_hole5._pegPtr = NULL;
		}
		break;
	case 1:
		scene->_pegPtr->hide();

		if (scene->_pegPtr2) {
			if (scene->_pegPtr->_armStrip == 3)
				scene->_pegPtr2->_strip = 2;

			scene->_pegPtr2->setPosition(scene->_pegPtr->_position);
			scene->_pegPtr2->show();
			scene->_pegPtr2->_armStrip = scene->_pegPtr->_armStrip;
		}

		scene->_pegPtr->_armStrip = 0;
		scene->_pegPtr->setPosition(Common::Point(-10, -10));
		scene->_pegPtr2 = scene->_pegPtr;
		scene->_armHotspot.animate(ANIM_MODE_5, this);
		break;

	case 2:
		g_globals->_player._uiEnabled = true;
		g_globals->_events.setCursor(CURSOR_USE);
		remove();
		break;
	}
}

void Scene4025::Action2::signal() {
	Scene4025 *scene = (Scene4025 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_armHotspot._strip = scene->_holePtr->_armStrip;
		scene->_armHotspot.animate(ANIM_MODE_4, 2, -1, this);
		break;
	case 1:
		if (!scene->_pegPtr2) {
			// Getting a peg from a hole
			scene->_holePtr->_pegPtr->hide();
			scene->_pegPtr = scene->_holePtr->_pegPtr;
			scene->_pegPtr->_armStrip = 0;
			scene->_pegPtr->setPosition(Common::Point(-10, -10));
			scene->_pegPtr2 = scene->_holePtr->_pegPtr;
			scene->_holePtr->_pegPtr = NULL;
		} else {
			// Placing a peg into a hole
			scene->_pegPtr2 = NULL;
			if (scene->_holePtr->_pegPtr) {
				scene->_holePtr->_pegPtr->hide();
				scene->_pegPtr2 = scene->_holePtr->_pegPtr;
			}

			assert(scene->_pegPtr);
			scene->_pegPtr->setPosition(scene->_holePtr->_newPosition);
			scene->_pegPtr->setStrip(1);
			scene->_pegPtr->show();
			scene->_pegPtr->_armStrip = scene->_holePtr->_armStrip;

			scene->_holePtr->_pegPtr = scene->_pegPtr;
			scene->_pegPtr = scene->_pegPtr2;
		}
		scene->_armHotspot.animate(ANIM_MODE_5, this);
		break;
	case 2:
		g_globals->_player._uiEnabled = true;
		g_globals->_events.setCursor(CURSOR_USE);
		remove();
		break;
	}
}

void Scene4025::Action3::signal() {
	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		g_globals->_scenePalette.addRotation(64, 111, -1);
		setDelay(120);
		break;
	case 1:
		g_globals->clearFlag(34);
		g_globals->_stripNum = 4025;
		g_globals->_sceneManager.changeScene(4000);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene4025::Hole::synchronize(Serializer &s) {
	SceneObject::synchronize(s);
	SYNC_POINTER(_pegPtr);
	s.syncAsSint16LE(_armStrip);
	s.syncAsSint16LE(_newPosition.x);
	s.syncAsSint16LE(_newPosition.y);
}

void Scene4025::Hole::doAction(int action) {
	Scene4025 *scene = (Scene4025 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4025, 3);
		break;
	case CURSOR_USE:
		if (!scene->_pegPtr && !_pegPtr) {
			setAction(&scene->_sequenceManager, scene, 4028, NULL);
		} else {
			g_globals->_player.disableControl();
			scene->_holePtr = this;
			scene->setAction(&scene->_action2);
		}
		break;
	case OBJECT_PEG:
		if (!scene->_pegPtr2) {
			g_globals->_player.disableControl();
			g_globals->_events.setCursor(CURSOR_USE);
			RING_INVENTORY._peg._sceneNumber = 4025;

			scene->_pegPtr = &scene->_peg5;
			scene->_holePtr = this;
			scene->_pegPtr->_armStrip = 0;
			scene->_pegPtr2 = scene->_pegPtr;

			scene->setAction(&scene->_action2);
		} else {
			scene->_sceneMode = 4027;
			scene->setAction(&scene->_sequenceManager, scene, 4027, NULL);
		}
		break;
	}
}

void Scene4025::Peg::synchronize(Serializer &s) {
	SceneObject::synchronize(s);
	s.syncAsSint16LE(_pegId);
	s.syncAsSint16LE(_armStrip);
}

void Scene4025::Peg::doAction(int action) {
	Scene4025 *scene = (Scene4025 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4025, 1);
		break;
	case CURSOR_USE:
		g_globals->_player.disableControl();
		scene->_pegPtr = this;
		scene->setAction(&scene->_action1);
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene4025::Scene4025() : Scene() {
	_holePtr = NULL;
	_pegPtr = _pegPtr2 = NULL;
}

void Scene4025::postInit(SceneObjectList *OwnerList) {
	loadScene(4025);
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);

	g_globals->_events.setCursor(CURSOR_USE);
	_pegPtr = _pegPtr2 = NULL;

	_peg1.postInit();
	_peg1._pegId = 1;
	_peg1.setVisage(4025);
	_peg1.setStrip(2);
	_peg1.setFrame(1);
	_peg1.setPosition(Common::Point(203, 61));

	_peg2.postInit();
	_peg2._pegId = 4;
	_peg2.setVisage(4025);
	_peg2.setStrip(2);
	_peg2.setFrame(2);
	_peg2.setPosition(Common::Point(195, 57));

	_peg3.postInit();
	_peg3._pegId = 0;
	_peg3.setVisage(4025);
	_peg3.setStrip(2);
	_peg3.setFrame(3);
	_peg3.setPosition(Common::Point(202, 66));

	_peg4.postInit();
	_peg4._pegId = 3;
	_peg4.setVisage(4025);
	_peg4.setStrip(2);
	_peg4.setFrame(4);
	_peg4.setPosition(Common::Point(194, 68));

	_peg5.postInit();
	_peg5._pegId = 2;
	_peg5.setVisage(4025);
	_peg5.setStrip(1);
	_peg5.setFrame(5);
	_peg5.hide();

	// Hole N-W
	_hole1.postInit();
	_hole1.setVisage(4025);
	_hole1.setStrip(1);
	_hole1.setFrame2(6);
	_hole1.setPosition(Common::Point(123, 51));
	_hole1._pegPtr = NULL;
	_hole1._newPosition = Common::Point(123, 44);
	_hole1._armStrip = 8;

	// Hole N-E
	_hole2.postInit();
	_hole2.setVisage(4025);
	_hole2.setStrip(1);
	_hole2.setFrame2(7);
	_hole2.setPosition(Common::Point(167, 51));
	_hole2._pegPtr = NULL;
	_hole2._newPosition = Common::Point(166, 44);
	_hole2._armStrip = 7;

	// Hole Center
	_hole3.postInit();
	_hole3.setVisage(4025);
	_hole3.setStrip(1);
	_hole3.setFrame2(8);
	_hole3.setPosition(Common::Point(145, 69));
	_hole3._pegPtr = NULL;
	_hole3._newPosition = Common::Point(145, 60);
	_hole3._armStrip = 6;

	// Hole S-W
	_hole4.postInit();
	_hole4.setVisage(4025);
	_hole4.setStrip(1);
	_hole4.setFrame2(9);
	_hole4.setPosition(Common::Point(123, 87));
	_hole4._pegPtr = NULL;
	_hole4._newPosition = Common::Point(123, 80);
	_hole4._armStrip = 5;

	// Hole S-E
	_hole5.postInit();
	_hole5.setVisage(4025);
	_hole5.setStrip(1);
	_hole5.setFrame2(10);
	_hole5.setPosition(Common::Point(167, 87));
	_hole5._pegPtr = NULL;
	_hole5._newPosition = Common::Point(166, 80);
	_hole5._armStrip = 4;

	_hole1.fixPriority(1);
	_hole2.fixPriority(1);
	_hole3.fixPriority(1);
	_hole4.fixPriority(1);
	_hole5.fixPriority(1);

	_armHotspot.postInit();
	_armHotspot.setVisage(4025);
	_armHotspot.setPosition(Common::Point(190, 161));
	_armHotspot.setStrip(3);
	_armHotspot.setFrame(4);

	g_globals->_sceneItems.addItems(&_hole1, &_hole2, &_hole3, &_hole4, &_hole5,
		&_peg1, &_peg2, &_peg3, &_peg4, &_peg5, NULL);

	g_globals->_player._uiEnabled = true;
	g_globals->_player.disableControl();
	setAction(&_sequenceManager, this, 4026, NULL);
}

void Scene4025::synchronize(Serializer &s) {
	Scene::synchronize(s);
	SYNC_POINTER(_pegPtr);
	SYNC_POINTER(_pegPtr2);
	SYNC_POINTER(_holePtr);
}

void Scene4025::remove() {
	g_globals->_scenePalette.clearListeners();
	Scene::remove();
}

void Scene4025::signal() {
	if (_sceneMode != 4027) {
		if (_sceneMode != 4028) {
			_gfxButton.setText(EXIT_MSG);
			_gfxButton._bounds.center(144, 107);
			_gfxButton.draw();
			_gfxButton._bounds.expandPanes();
		}

		g_globals->_player._uiEnabled = true;
	}

	g_globals->_events.setCursor(CURSOR_USE);
}

void Scene4025::process(Event &event) {
	Scene::process(event);

	if (_gfxButton.process(event)) {
		if (RING_INVENTORY._peg._sceneNumber == 4025)
			RING_INVENTORY._peg._sceneNumber = 1;

		g_globals->_sceneManager.changeScene(4000);
	}
}

void Scene4025::dispatch() {
	if (!_action && (_peg1._armStrip == 7) && (_peg2._armStrip == 4) && (_peg3._armStrip == 8) &&
			(_peg4._armStrip == 5) && (_peg5._armStrip == 6))
		setAction(&_action3);

	Scene::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 4045 - Village - Temple Antechamber
 *
 *--------------------------------------------------------------------------*/

void Scene4045::Action1::signal() {
	Scene4045 *scene = (Scene4045 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		setDelay(60);
		break;
	case 1:
		g_globals->_events.setCursor(CURSOR_WALK);
		scene->_stripManager.start(4040, this, scene);
		break;
	case 2:
		scene->_olloFace.animate(ANIM_MODE_NONE, NULL);
		scene->_hotspot4.animate(ANIM_MODE_5, this);
		break;
	case 3:
		setDelay(60);
		break;
	case 4:
		scene->_hotspot4.animate(ANIM_MODE_6, this);
		break;
	case 5:
		ADD_MOVER(g_globals->_player, 150, 300);
		break;
	case 6:
		g_globals->_stripNum = 4000;
		g_globals->_sceneManager.changeScene(4000);
		remove();
		break;
	}
}

void Scene4045::Action2::signal() {
	Scene4045 *scene = (Scene4045 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		setDelay(15);
		break;
	case 1:
		scene->_stripManager.start(g_globals->_stripNum, this, scene);
		break;
	case 2:
		scene->_olloFace.animate(ANIM_MODE_NONE, NULL);
		setDelay(10);
		break;
	case 3:
		if (g_globals->getFlag(38)) {
			g_globals->_player.enableControl();
			remove();
		} else {
			ADD_MOVER(g_globals->_player, 150, 300);
		}
		break;
	case 4:
		g_globals->setFlag(39);
		g_globals->_sceneManager.changeScene(4000);
		break;
	}
}

void Scene4045::Action3::signal() {
	Scene4045 *scene = (Scene4045 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		scene->_hotspot4.animate(ANIM_MODE_5, this);
		break;
	case 1:
		scene->_stripManager.start(4504, this, scene);
		break;
	case 2:
		scene->_hotspot4.animate(ANIM_MODE_6, NULL);
		scene->_olloFace.animate(ANIM_MODE_NONE, NULL);
		ADD_MOVER(g_globals->_player, 91, 1264);
		break;
	case 3:
		g_globals->_player.enableControl();
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene4045::OlloStand::doAction(int action) {
	Scene4045 *scene = (Scene4045 *)g_globals->_sceneManager._scene;

	switch (action) {
	case OBJECT_STUNNER:
		SceneItem::display2(4045, 19);
		break;
	case CURSOR_LOOK:
		SceneItem::display2(4045, (_strip == 1) ? 5 : 14);
		break;
	case CURSOR_USE:
		SceneItem::display2(4045, 18);
		break;
	case CURSOR_TALK:
		if (_strip == 5) {
			setStrip(6);
			animate(ANIM_MODE_NONE, NULL);
		}
		if (g_globals->_player._position.y < 135) {
			scene->_sceneMode = 4046;
			_numFrames = 10;
			g_globals->_player.disableControl();

			scene->setAction(&scene->_sequenceManager, this, 4046, &g_globals->_player, this, NULL);
		} else {
			if (!g_globals->getFlag(31)) {
				g_globals->setFlag(31);
				g_globals->_stripNum = 4080;
			} else if (!g_globals->getFlag(38))
				g_globals->_stripNum = 4060;
			else
				g_globals->_stripNum = 4503;

			scene->setAction(&scene->_action2);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene4045::Miranda::doAction(int action) {
	Scene4045 *scene = (Scene4045 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4045, 9);
		break;
	case CURSOR_TALK:
		scene->_sceneMode = 4102;
		g_globals->_player.disableControl();
		scene->setAction(&scene->_sequenceManager, scene, 4104, NULL);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene4045::Necklace::doAction(int action) {
	Scene4045 *scene = (Scene4045 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4045, 17);
		break;
	case CURSOR_USE:
		if (g_globals->_player._position.y < 135) {
			SceneItem::display2(4045, 16);
			RING_INVENTORY._peg._sceneNumber = 1;
			g_globals->_events.setCursor(CURSOR_WALK);
			remove();
		} else {
			scene->_sceneMode = 4047;
			g_globals->_player.disableControl();
			scene->setAction(&scene->_sequenceManager, scene, 4047, &g_globals->_player, &scene->_olloStand, NULL);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene4045::Scene4045() :
	_flame(0, CURSOR_LOOK, 4045, 1, CURSOR_USE, 4100, 21, LIST_END),
	_hotspot7(9, CURSOR_LOOK, 4045, 0, CURSOR_USE, 4045, 15, LIST_END),
	_hotspot8(10, CURSOR_LOOK, 4045, 2, LIST_END),
	_hotspot9(11, CURSOR_LOOK, 4045, 3, CURSOR_USE, 4045, 15, LIST_END),
	_hotspot10(12, CURSOR_LOOK, 4045, 4, CURSOR_USE, 4100, 19, LIST_END),
	_hotspot11(13, CURSOR_LOOK, 4045, 6, CURSOR_USE, 4045, 15, LIST_END),
	_hotspot12(14, CURSOR_LOOK, 4045, 7, CURSOR_USE, 4150, 29, LIST_END),
	_hotspot13(15, CURSOR_LOOK, 4045, 8, CURSOR_USE, 4100, 19, LIST_END),
	_hotspot14(0, CURSOR_LOOK, 4045, 10, LIST_END) {

	_hotspot14.setBounds(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
}

void Scene4045::postInit(SceneObjectList *OwnerList) {
	loadScene(4045);
	Scene::postInit();
	setZoomPercents(100, 60, 200, 100);

	_stripManager.addSpeaker(&_speakerQR);
	_stripManager.addSpeaker(&_speakerML);
	_stripManager.addSpeaker(&_speakerPR);
	_stripManager.addSpeaker(&_speakerPText);
	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerQL);
	_stripManager.setCallback(this);

	_speakerQText._textPos.y = 140;

	_flame.postInit();
	_flame.setVisage(4045);
	_flame.setPosition(Common::Point(47, 111));
	_flame.animate(ANIM_MODE_2, NULL);
	_flame.fixPriority(156);
	g_globals->_sceneItems.push_back(&_flame);

	g_globals->_player.postInit();
	g_globals->_player.setVisage(4200);
	g_globals->_player.setObjectWrapper(new SceneObjectWrapper());
	g_globals->_player.animate(ANIM_MODE_1, NULL);
	g_globals->_player._moveDiff = Common::Point(7, 4);

	_olloStand.postInit();
	_olloStand.setVisage(4051);

	_olloFace.postInit();
	_olloFace.setVisage(4051);
	_olloFace.setStrip(4);
	_olloFace.fixPriority(152);

	if (g_globals->_sceneManager._previousScene == 4050) {
		g_globals->_soundHandler.play(155);
		g_globals->_player.setPosition(Common::Point(72, 128));
		g_globals->_player.enableControl();

		_olloStand.setStrip(5);
		_olloStand.setPosition(Common::Point(173, 99));
		_olloStand._numFrames = 1;
		_olloStand.animate(ANIM_MODE_2, NULL);

		_olloFace.setPosition(Common::Point(177, 40));

		if (RING_INVENTORY._peg._sceneNumber == 4045) {
			_necklace.postInit();
			_necklace.setVisage(4045);
			_necklace.setStrip(2);
			_necklace.setPosition(Common::Point(108, 82));
			g_globals->_sceneItems.push_back(&_necklace);
		}
	} else {
		g_globals->_player.setPosition(Common::Point(108, 192));
		g_globals->_player.setStrip(4);

		if (!g_globals->getFlag(36) && !g_globals->getFlag(43)) {
			_miranda.postInit();
			_miranda.setVisage(4102);
			_miranda.animate(ANIM_MODE_NONE, NULL);
			_miranda.setStrip(3);
			_miranda.setFrame(2);
			_miranda.changeZoom(-1);

			_miranda.setPosition(Common::Point(66, 209));
			g_globals->_sceneItems.push_back(&_miranda);
		}

		if (g_globals->getFlag(31)) {
			// Olo asleep
			_olloStand.setVisage(4051);
			_olloStand.setStrip(5);
			_olloStand.setPosition(Common::Point(173, 99));
			_olloStand._numFrames = 1;
			_olloStand.animate(ANIM_MODE_2, NULL);

			_olloFace.setPosition(Common::Point(177, 40));

			if (RING_INVENTORY._peg._sceneNumber == 4045) {
				_necklace.postInit();
				_necklace.setVisage(4045);
				_necklace.setStrip(2);
				_necklace.setPosition(Common::Point(108, 82));
				g_globals->_sceneItems.push_back(&_necklace);
			}
		} else {
			_olloStand.setPosition(Common::Point(186, 149));

			_hotspot4.postInit();
			_hotspot4.setVisage(4051);
			_hotspot4.setStrip(2);
			_hotspot4.fixPriority(152);
			_hotspot4.setPosition(Common::Point(202, 80));

			_olloFace.setPosition(Common::Point(192, 77));
			g_globals->setFlag(31);
			setAction(&_action1);

			g_globals->_player.disableControl();
		}
	}

	g_globals->_sceneItems.addItems(&_olloStand, &_hotspot7, &_hotspot8, &_hotspot9, &_hotspot10,
		&_hotspot13, &_hotspot11, &_hotspot12, &_hotspot14, NULL);
}

void Scene4045::stripCallback(int v) {
	switch (v) {
	case 1:
		_olloFace.animate(ANIM_MODE_7, 0, NULL);
		break;
	case 2:
		_olloFace.animate(ANIM_MODE_NONE, NULL);
		break;
	}
}

void Scene4045::signal() {
	switch (_sceneMode) {
	case 4046:
	case 4047:
		_olloFace.animate(ANIM_MODE_NONE, NULL);
		break;
	case 4050:
		g_globals->_sceneManager.changeScene(4000);
		break;
	case 4102:
		g_globals->_player.enableControl();
		break;
	}
}

void Scene4045::dispatch() {
	if (!_action) {
		if (g_globals->_player.getRegionIndex() == 8) {
			g_globals->_player.addMover(NULL);
			if (_olloStand._strip != 1) {
				g_globals->_player.disableControl();
				_sceneMode = 4046;
				_olloStand._numFrames = 10;
				setAction(&_sequenceManager, this, 4046, &g_globals->_player, &_olloStand, NULL);
			} else {
				setAction(&_action3);
			}
		}

		if (g_globals->_player.getRegionIndex() == 10)
			g_globals->_sceneManager.changeScene(4050);
		if (g_globals->_player._position.y >= 196) {
			_sceneMode = 4050;
			g_globals->_player.disableControl();
			setAction(&_sequenceManager, this, 4105, &g_globals->_player, NULL);
		}
	}

	Scene::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 4050 - Village - Outside
 *
 *--------------------------------------------------------------------------*/

void Scene4050::Action1::signal() {
	// "Map" on the wall
	Scene4050 *scene = (Scene4050 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		setDelay(3);
		break;
	case 1: {
		Common::Point pt(204, 152);
		PlayerMover *mover = new PlayerMover();
		g_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 2:
		g_globals->_player.checkAngle(&scene->_hotspot17);

		scene->_hotspot14.postInit();
		scene->_hotspot14.setVisage(4050);
		scene->_hotspot14.setStrip(2);
		scene->_hotspot14.setPosition(Common::Point(91, 154));
		scene->_hotspot14.fixPriority(200);
		setDelay(10);
		break;
	case 3:
		g_globals->_events.waitForPress();
		g_globals->setFlag(45);
		scene->_hotspot14.remove();

		g_globals->_player.enableControl();
		remove();
		break;
	}
}

void Scene4050::Action2::signal() {
	// Climb down the rope
	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		setDelay(120);
		break;
	case 1:
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 2:
		g_globals->_player.setVisage(4202);
		g_globals->_player.changeZoom(100);
		g_globals->_player.animate(ANIM_MODE_1, NULL);
		g_globals->_player.setObjectWrapper(new SceneObjectWrapper());
		g_globals->_player.setPosition(Common::Point(192, 130));

		ADD_MOVER(g_globals->_player, 215, 130);
		break;
	case 3:
		g_globals->_player.setVisage(4052);
		g_globals->_player.setStrip(5);
		g_globals->_player.setFrame(1);
		g_globals->_player.setPosition(Common::Point(236, 130));
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 4:
		g_globals->_player.setVisage(4202);
		g_globals->_player.animate(ANIM_MODE_1, NULL);
		g_globals->_player.setPosition(Common::Point(210, 185));
		g_globals->_player.fixPriority(-1);
		g_globals->_player.enableControl();
		remove();
		break;
	}
}

void Scene4050::Action3::signal() {
	// Climb up the rope
	switch (_actionIndex++) {
	case 0:
		ADD_PLAYER_MOVER(210, 185);
		break;
	case 1:
		g_globals->_player.fixPriority(200);
		g_globals->_player.setVisage(4052);
		g_globals->_player.setStrip(5);
		g_globals->_player.changeZoom(100);
		g_globals->_player.setFrame(g_globals->_player.getFrameCount());
		g_globals->_player.setPosition(Common::Point(236, 130));
		g_globals->_player.animate(ANIM_MODE_6, this);
		break;
	case 2:
		g_globals->_player.setVisage(4202);
		g_globals->_player.animate(ANIM_MODE_1, NULL);
		g_globals->_player.setObjectWrapper(new SceneObjectWrapper());
		g_globals->_player.setPosition(Common::Point(215, 130));
		ADD_MOVER(g_globals->_player, 212, 130);
		break;
	case 3:
		g_globals->_stripNum = 4050;
		g_globals->_sceneManager.changeScene(4000);
		break;
	}
}

void Scene4050::Action4::signal() {
	Scene4050 *scene = (Scene4050 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		ADD_MOVER(g_globals->_player, 189, 135);
		g_globals->_player.fixPriority(200);
		break;
	case 1:
		g_globals->_player._moveDiff.y = 3;
		g_globals->_player.setStrip2(3);
		g_globals->_player._frame = 1;
		g_globals->_player.setPosition(Common::Point(189, 129));
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 2:
		g_globals->_player.setVisage(4202);
		g_globals->_player.animate(ANIM_MODE_1, NULL);
		g_globals->_player.setObjectWrapper(new SceneObjectWrapper());
		g_globals->_player.setPosition(Common::Point(192, 130));
		g_globals->_player.changeZoom(100);
		ADD_MOVER(g_globals->_player, 215, 130);

		scene->_hotspot16.postInit();
		scene->_hotspot16.setVisage(4052);
		scene->_hotspot16.animate(ANIM_MODE_1, NULL);
		scene->_hotspot16.setStrip2(6);
		scene->_hotspot16.setPosition(Common::Point(160, 240));
		ADD_MOVER(scene->_hotspot16, 172, 188);
		break;
	case 3:
		g_globals->_player.setVisage(4052);
		g_globals->_player.setStrip(5);
		g_globals->_player.setFrame(1);
		g_globals->_player.setPosition(Common::Point(238, 130));
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 4:
		g_globals->_player.setVisage(4052);
		g_globals->_player.setStrip(2);
		g_globals->_player.setFrame(1);
		g_globals->_player.setPosition(Common::Point(216, 184));
		g_globals->_player.fixPriority(-1);
		break;
	case 5:
		scene->_hotspot16.setStrip2(4);
		scene->_hotspot16.setFrame(1);
		scene->_hotspot16.animate(ANIM_MODE_4, 4, 1, this);
		break;
	case 6:
		scene->_hotspot16.animate(ANIM_MODE_5, NULL);
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 7:
		g_globals->_player.setVisage(4202);
		g_globals->_player.setStrip(6);
		g_globals->_player.setFrame(1);

		scene->_stripManager.start(4051, this);
		break;
	case 8:
		setDelay(15);
		break;
	case 9:
		g_globals->_sceneManager.changeScene(4000);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene4050::Hotspot15::doAction(int action) {
	Scene4050 *scene = (Scene4050 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4050, 14);
		break;
	case CURSOR_USE:
		g_globals->_player.disableControl();
		setAction(&scene->_action3);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene4050::Hotspot17::doAction(int action) {
	Scene4050 *scene = (Scene4050 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		scene->setAction(&scene->_action1);
		break;
	case CURSOR_USE:
		SceneItem::display2(4050, 24);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene4050::Scene4050() :
		_hotspot1(0, CURSOR_LOOK, 4050, 0, LIST_END),
		_hotspot2(0, CURSOR_LOOK, 4050, 1, CURSOR_USE, 4050, 15, LIST_END),
		_hotspot3(0, CURSOR_LOOK, 4050, 2, CURSOR_USE, 4050, 16, LIST_END),
		_hotspot4(0, CURSOR_LOOK, 4050, 4, LIST_END),
		_hotspot5(0, CURSOR_LOOK, 4050, 5, LIST_END),
		_hotspot6(0, CURSOR_LOOK, 4050, 5, LIST_END),
		_hotspot7(0, CURSOR_LOOK, 4050, 6, CURSOR_USE, 4050, 18, LIST_END),
		_hotspot8(0, CURSOR_LOOK, 4050, 7, CURSOR_USE, 4050, 19, LIST_END),
		_hotspot9(0, CURSOR_LOOK, 4050, 8, CURSOR_USE, 4050, 20, LIST_END),
		_hotspot10(0, CURSOR_LOOK, 4050, 9, CURSOR_USE, 4050, 21, LIST_END),
		_hotspot11(0, CURSOR_LOOK, 4050, 10, CURSOR_USE, 4050, 22, LIST_END),
		_hotspot12(0, CURSOR_LOOK, 4050, 11, CURSOR_USE, 4050, 23, LIST_END),
		_hotspot13(0, CURSOR_LOOK, 4050, 3, CURSOR_USE, 4050, 17, LIST_END) {
}

void Scene4050::postInit(SceneObjectList *OwnerList) {
	loadScene(4050);
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);

	_stripManager.addSpeaker(&_speakerPText);
	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerGameText);

	g_globals->_player.postInit();

	switch (g_globals->_sceneManager._previousScene) {
	case 4000:
		if (g_globals->getFlag(41)) {
			// Using a rope
			_hotspot15.postInit();
			_hotspot15.setVisage(4054);
			_hotspot15.setPosition(Common::Point(206, 103));
			g_globals->_sceneItems.push_back(&_hotspot15);

			g_globals->_player.setVisage(4008);
			g_globals->_player.setPosition(Common::Point(206, 62));
			g_globals->_player.changeZoom(130);
			g_globals->_player.fixPriority(200);
			g_globals->_player.setStrip(2);

			setAction(&_action2);
			g_globals->_soundHandler.play(175);
		} else {
			// Without the rope
			g_globals->_player.setVisage(5315);
			g_globals->_player.setPosition(Common::Point(189, 83));
			g_globals->_player.changeZoom(130);
			g_globals->_player.setStrip2(2);
			g_globals->_player._moveDiff.y = 10;
			g_globals->_player.animate(ANIM_MODE_2, NULL);

			setAction(&_action4);
			g_globals->_soundHandler.play(176);
		}
		break;
	case 4045:
		_hotspot15.postInit();
		_hotspot15.setVisage(4054);
		_hotspot15.setPosition(Common::Point(206, 103));
		g_globals->_sceneItems.push_back(&_hotspot15);

		g_globals->_player.setVisage(4202);
		g_globals->_player.animate(ANIM_MODE_1, NULL);
		g_globals->_player.setObjectWrapper(new SceneObjectWrapper());
		g_globals->_player.setPosition(Common::Point(193, 193));

		g_globals->_soundHandler.play(175);
		break;
	default:
		break;
	}

	_hotspot13.postInit();
	_hotspot13.setVisage(4053);
	_hotspot13.animate(ANIM_MODE_2, NULL);
	_hotspot13.setPosition(Common::Point(190, 97));

	_hotspot17.postInit();
	_hotspot17.setVisage(4050);
	_hotspot17.setPosition(Common::Point(209, 119));
	_hotspot17.fixPriority(2);

	_hotspot1.setBounds(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
	_hotspot2.setBounds(Rect(150, 25, 198, 125));
	_hotspot3.setBounds(Rect(56, 94, 88, 153));
	_hotspot4.setBounds(Rect(170, 7, 209, 18));
	_hotspot5.setBounds(Rect(190, 72, 212, 92));
	_hotspot6.setBounds(Rect(122, 75, 145, 93));
	_hotspot7.setBounds(Rect(109, 103, 263, 158));
	_hotspot8.setBounds(Rect(157, 160, 213, 173));
	_hotspot9.setBounds(Rect(95, 157, 120, 178));
	_hotspot10.setBounds(Rect(278, 43, 310, 115));
	_hotspot11.setBounds(Rect(263, 105, 279, 147));
	_hotspot12.setBounds(Rect(258, 154, 307, 180));

	g_globals->_sceneItems.addItems(&_hotspot17, &_hotspot12, &_hotspot11, &_hotspot10, &_hotspot13,
		&_hotspot2, &_hotspot3, &_hotspot4, &_hotspot5, &_hotspot6, &_hotspot8, &_hotspot9,
		&_hotspot7, &_hotspot1, NULL);
}

void Scene4050::signal() {
	if (_sceneMode == 4050)
		g_globals->_sceneManager.changeScene(4045);
}

void Scene4050::dispatch() {
	if (!_action) {
		if ((g_globals->_player._canWalk) && (g_globals->_player._position.y > 196)) {
			_sceneMode = 4050;
			g_globals->_player.disableControl();

			Common::Point pt(160, 275);
			NpcMover *mover = new NpcMover();
			g_globals->_player.addMover(mover, &pt, this);
		}
	}
	Scene::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 4100 - Village - Hut
 *
 *--------------------------------------------------------------------------*/

void Scene4100::Action1::signal() {
	Scene4100 *scene = (Scene4100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		setDelay(10);
		break;
	case 1:
		scene->_stripManager.start(g_globals->_stripNum, this);
		break;
	case 2:
		setDelay(30);
		break;
	case 3:
		g_globals->_player.enableControl();
		remove();
		break;
	}
}

void Scene4100::Action2::signal() {
	Scene4100 *scene = (Scene4100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_hotspot2.postInit();
		scene->_hotspot2.changeZoom(95);
		scene->_hotspot2.setVisage(4120);
		scene->_hotspot2.animate(ANIM_MODE_1, NULL);
		scene->_hotspot2.setStrip2(4);
		scene->_hotspot2.fixPriority(100);
		scene->_hotspot2.setPosition(Common::Point(214, 119));

		setDelay(3);
		break;
	case 1:
		ADD_MOVER(scene->_hotspot2, 249, 131);
		break;
	case 2:
		scene->_hotspot2.setStrip2(3);
		scene->_hotspot2.setFrame2(5);
		scene->_hotspot2.animate(ANIM_MODE_NONE, NULL);
		setDelay(3);
		break;
	case 3:
		remove();
		break;
	}
}


void Scene4100::Action3::signal() {
	Scene4100 *scene = (Scene4100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->clearFlag(43);
		g_globals->setFlag(36);
		setDelay(15);
		break;
	case 1:
		scene->_stripManager.start(4505, this);
		break;
	case 2:
		setAction(&scene->_action2, this);
		break;
	case 3:
		scene->_stripManager.start(4510, this);
		break;
	case 4:
		setDelay(15);
		break;
	case 5:
		g_globals->_sceneManager.changeScene(4150);
		break;
	default:
		break;
	}
}

void Scene4100::Action4::signal() {
	// Rock getting drunk
	Scene4100 *scene = (Scene4100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(30);
		break;
	case 1:
		scene->_hotspot1.animate(ANIM_MODE_5, this);
		break;
	case 2:
		scene->_hotspot1.setVisage(4105);
		scene->_hotspot1.setFrame(1);
		scene->_hotspot1.animate(ANIM_MODE_5, this);
		break;
	case 3:
		g_globals->clearFlag(43);
		g_globals->setFlag(42);
		scene->_stripManager.start(4119, this);
		break;
	case 4:
		setDelay(15);
		break;
	case 5:
		g_globals->_player.enableControl();
		remove();
		break;
	}
}

void Scene4100::Action5::signal() {
	Scene4100 *scene = (Scene4100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		ADD_PLAYER_MOVER(58, 151);
		break;
	case 1:
		if (RING_INVENTORY._ladder._sceneNumber == 4100) {
			RING_INVENTORY._ladder._sceneNumber = 1;
			scene->_ladder.remove();
		} else {
			scene->_ladder.postInit();
			scene->_ladder.setVisage(4101);
			scene->_ladder.setPosition(Common::Point(49, 144));

			RING_INVENTORY._ladder._sceneNumber = 4100;
			g_globals->_sceneItems.push_front(&scene->_ladder);
		}

		g_globals->_player.enableControl();
		remove();
		break;
	}
}


void Scene4100::Action6::signal() {
	Scene4100 *scene = (Scene4100 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		scene->_stripManager.start(4103, this);
		ADD_PLAYER_MOVER(245, 167);
		break;
	case 1:
		g_globals->_player.enableControl();
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene4100::Hotspot1::doAction(int action)  {
	Scene4100 *scene = (Scene4100 *)g_globals->_sceneManager._scene;

	switch (action) {
	case OBJECT_STUNNER:
		SceneItem::display2(4100, 16);
		break;
	case OBJECT_ALE:
		g_globals->_player.disableControl();
		scene->setAction(&scene->_action3);
		break;
	case CURSOR_LOOK:
		SceneItem::display2(4100, g_globals->getFlag(42) ? 24 : 12);
		break;
	case CURSOR_USE:
		SceneItem::display2(4100, 22);
		break;
	case CURSOR_TALK:
		if (RING_INVENTORY._peg._sceneNumber == 1) {
			g_globals->_player.disableControl();
			scene->_sceneMode = 4109;
			scene->setAction(&scene->_sequenceManager, scene, 4109, NULL);
		} else if (g_globals->getFlag(42)) {
			scene->_sceneMode = 4102;
			scene->setAction(&scene->_sequenceManager, scene, 4102, NULL);
		} else {
			if (g_globals->getFlag(33))
				g_globals->_stripNum = 4077;
			else if (g_globals->getFlag(82)) {
				g_globals->clearFlag(82);
				g_globals->_stripNum = 4100;
			} else {
				g_globals->_stripNum = 4075;
				g_globals->setFlag(33);
			}

			scene->setAction(&scene->_action1);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene4100::Hotspot2::doAction(int action)  {
	switch (action) {
	case CURSOR_LOOK:
		error("*** The Chief's daughter... WOW!");
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene4100::Miranda::doAction(int action)  {
	Scene4100 *scene = (Scene4100 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4100, 14);
		break;
	case CURSOR_TALK:
		scene->_sceneMode = 4102;
		scene->setAction(&scene->_sequenceManager, scene, 4104, NULL);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene4100::Ladder::doAction(int action)  {
	Scene4100 *scene = (Scene4100 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4100, 2);
		break;
	case CURSOR_USE:
		if (g_globals->getFlag(42)) {
			g_globals->_player.disableControl();
			scene->setAction(&scene->_action5);
		} else {
			scene->_sceneMode = 4102;
			scene->setAction(&scene->_sequenceManager, scene, 4103, NULL);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

/**
 Exit hotspot, South
 */
void Scene4100::Hotspot14::doAction(int action)  {
	Scene4100 *scene = (Scene4100 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4100, 0);
		break;
	case OBJECT_LADDER:
		g_globals->_player.disableControl();
		scene->setAction(&scene->_action5);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}


/*--------------------------------------------------------------------------*/

Scene4100::Scene4100() :
		_hotspot3(0, CURSOR_LOOK, 4100, 10, CURSOR_USE, 4100, 21, LIST_END),
		_hotspot4(0, CURSOR_LOOK, 4100, 8, LIST_END),
		_hotspot7(0, CURSOR_USE, 4100, 3, CURSOR_USE, 4150, 29, LIST_END),
		_hotspot8(0, CURSOR_LOOK, 4100, 5, LIST_END),
		_hotspot9(0, CURSOR_LOOK, 4100, 6, CURSOR_USE, 4100, 20, LIST_END),
		_hotspot10(0, CURSOR_LOOK, 4100, 7, CURSOR_USE, 4100, 19, LIST_END),
		_hotspot11(0, CURSOR_LOOK, 4100, 8, CURSOR_USE, 4100, 18, LIST_END),
		_hotspot12(0, CURSOR_LOOK, 4100, 9, LIST_END),
		_hotspot13(0, CURSOR_LOOK, 4100, 11, CURSOR_USE, 4100, 17, LIST_END) {
}

void Scene4100::postInit(SceneObjectList *OwnerList) {
	loadScene(4100);
	Scene::postInit();
	setZoomPercents(135, 80, 190, 100);

	_stripManager.addSpeaker(&_speakerMText);
	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerCHFText);
	_stripManager.addSpeaker(&_speakerCDRText);
	_stripManager.addSpeaker(&_speakerCDR);
	_stripManager.addSpeaker(&_speakerML);
	_stripManager.addSpeaker(&_speakerQR);

	g_globals->_player.postInit();
	g_globals->_player.setVisage(4200);
	g_globals->_player.animate(ANIM_MODE_1, NULL);
	g_globals->_player.setStrip(4);
	g_globals->_player.setObjectWrapper(new SceneObjectWrapper());
	g_globals->_player.setPosition(Common::Point(88, 180));
	g_globals->_player._moveDiff = Common::Point(7, 4);

	_hotspot1.postInit();
	_hotspot1.setVisage(4102);
	_hotspot1.fixPriority(129);
	_hotspot1.setPosition(Common::Point(171, 120));

	_hotspot3.postInit();
	_hotspot3.setVisage(4130);
	_hotspot3.animate(ANIM_MODE_2, NULL);
	_hotspot3.fixPriority(200);
	_hotspot3.setPosition(Common::Point(272, 110));

	_hotspot4.postInit();
	_hotspot4.setVisage(4130);
	_hotspot4.animate(ANIM_MODE_2, NULL);
	_hotspot4.setStrip2(2);
	_hotspot4.setPosition(Common::Point(152, 167));

	if (g_globals->getFlag(36)) {
		_hotspot1.setVisage(4105);
		_hotspot1.setStrip(1);
		_hotspot1.setFrame(4);
	} else if (!g_globals->getFlag(43)) {
		_miranda.postInit();
		_miranda.setVisage(4102);
		_miranda.setStrip2(3);
		_miranda.setFrame(2);
		_miranda.setPosition(Common::Point(65, 188));

		g_globals->_sceneItems.push_back(&_miranda);
	}

	if (RING_INVENTORY._ladder._sceneNumber == 4100) {
		_ladder.postInit();
		_ladder.setVisage(4101);
		_ladder.setPosition(Common::Point(49, 144));

		g_globals->_sceneItems.push_back(&_ladder);
	}

	_hotspot14.setBounds(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
	_hotspot7._sceneRegionId = 16;
	_hotspot8.setBounds(Rect(90, 0, 211, 113));
	_hotspot9.setBounds(Rect(207, 96, 235, 115));
	_hotspot10.setBounds(Rect(87, 81, 212, 107));
	_hotspot11.setBounds(Rect(61, 148, 243, 168));
	_hotspot12.setBounds(Rect(229, 59, 256, 122));
	_hotspot13.setBounds(Rect(255, 152, 286, 183));

	g_globals->_sceneItems.addItems(&_hotspot3, &_hotspot1, &_hotspot13, &_hotspot12, &_hotspot4,
		&_hotspot11, &_hotspot9, &_hotspot7, &_hotspot10, &_hotspot8, &_hotspot14, NULL);

	if (g_globals->_sceneManager._previousScene == 4150) {
		g_globals->_soundHandler.play(155);

		if (!g_globals->getFlag(42)) {
			_hotspot1.setVisage(4104);
			_hotspot1.setPosition(Common::Point(152, 118));
			_hotspot1.setStrip2(-1);
			_hotspot1.setFrame2(-1);
			_hotspot1.setFrame(1);

			g_globals->_player.setStrip(2);
			g_globals->_player.disableControl();
			setAction(&_action4);

			g_globals->clearFlag(43);
		} else {
			// Workaround: In the original, the mouse is hidden when Quinn
			// goes back to scene 4150 then to scene 4100. This enables everything.
			g_globals->_player.enableControl();
		}

		g_globals->_player.setPosition(Common::Point(252, 139));
		g_globals->_player.setStrip(2);
	} else {
		if ((RING_INVENTORY._ale._sceneNumber == 4100) && !g_globals->getFlag(42)) {
			g_globals->_player.disableControl();
			setAction(&_action3);
		}
		if (g_globals->getFlag(35)) {
			g_globals->_player.disableControl();
			_sceneMode = 4101;
			setAction(&_sequenceManager, this, 4101, &g_globals->_player, NULL);
		}
	}
}

void Scene4100::signal() {
	switch (_sceneMode) {
	case 4101:
		g_globals->_sceneManager.changeScene(4000);
		break;
	case 4102:
	case 4103:
	case 4109:
		g_globals->_player.enableControl();
		break;
	}
}

void Scene4100::dispatch() {
	if (!_action) {
		if ((g_globals->_player.getRegionIndex() == 15) && !g_globals->getFlag(42))
			setAction(&_action6);

		if (g_globals->_player.getRegionIndex() == 8)
			g_globals->_sceneManager.changeScene(4150);

		if (g_globals->_player._position.y >= 196) {
			g_globals->_player.disableControl();
			_sceneMode = 4101;
			setAction(&_sequenceManager, this, 4105, &g_globals->_player, NULL);
		}
	}

	Scene::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 4150 - Village - Bedroom
 *
 *--------------------------------------------------------------------------*/

void Scene4150::Action1::signal() {
	Scene4150 *scene = (Scene4150 *)g_globals->_sceneManager._scene;
	byte adjustData[4] = {0, 0, 0, 0};

	switch (_actionIndex++) {
	case 0:
		setDelay(60);
		break;
	case 1:
		scene->_stripManager.start(4520, this);
		break;
	case 2:
		setDelay(15);
		break;
	case 3:
		setAction(&scene->_action2, this);
		break;
	case 4: {
		for (int idx = 100; idx >= 0; idx -= 5) {
			g_globals->_scenePalette.fade(adjustData, false, idx);
			GLOBALS._screenSurface.updateScreen();
			g_system->delayMillis(10);
		}

		scene->_hotspot4.postInit();
		scene->_hotspot4.setVisage(4150);
		scene->_hotspot4.setStrip(2);
		scene->_hotspot4.setPosition(Common::Point(166, 157));

		scene->_hotspot1.postInit();
		scene->_hotspot1.setVisage(4156);
		scene->_hotspot1.setStrip(1);
		scene->_hotspot1.setFrame(1);

		setDelay(120);
		break;
	}
	case 5:
		g_globals->_scenePalette.loadPalette(4150);
		g_globals->_scenePalette.refresh();
		setDelay(60);
		break;
	case 6:
		scene->_stripManager.start(4522, this);
		break;
	case 7:
		for (int idx = 100; idx >= 0; idx -= 5) {
			g_globals->_scenePalette.fade(adjustData, false, idx);
			GLOBALS._screenSurface.updateScreen();
			g_system->delayMillis(10);
		}

		scene->_hotspot4.remove();
		scene->_hotspot1.setVisage(4157);
		scene->_hotspot1.animate(ANIM_MODE_NONE, NULL);
		scene->_hotspot1.setStrip(1);
		scene->_hotspot1.setFrame(1);
		scene->_hotspot1.show();

		g_globals->_player.setVisage(4200);
		g_globals->_player.animate(ANIM_MODE_1, NULL);
		g_globals->_player.setStrip(3);
		g_globals->_player.setPosition(Common::Point(139, 166));
		g_globals->_player._moveDiff = Common::Point(7, 3);
		g_globals->_player.show();

		setDelay(120);
		break;
	case 8:
		g_globals->_scenePalette.loadPalette(4150);
		g_globals->_scenePalette.refresh();
		setDelay(30);
		break;
	case 9:
		scene->_stripManager.start(4523, this);
		break;
	case 10:
		g_globals->_player.setObjectWrapper(new SceneObjectWrapper());
		g_globals->_player._moveDiff = Common::Point(7, 4);
		g_globals->_player.setStrip(3);
		g_globals->_player.enableControl();

		remove();
		break;
	}
}

void Scene4150::Action2::signal() {
	Scene4150 *scene = (Scene4150 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(3);
		break;
	case 1:
		scene->_hotspot1.setVisage(4152);
		scene->_hotspot1.setStrip(1);
		scene->_hotspot1.setFrame(1);
		scene->_hotspot1.animate(ANIM_MODE_5, this);

		g_globals->_player.animate(ANIM_MODE_5, NULL);
		break;
	case 2:
		g_globals->_player.hide();
		scene->_hotspot1.setVisage(4153);
		scene->_hotspot1.setStrip(1);
		scene->_hotspot1.setFrame(1);
		scene->_hotspot1.animate(ANIM_MODE_5, this);
		break;
	case 3:
		remove();
		break;
	}
}

void Scene4150::Action3::signal() {
	Scene4150 *scene = (Scene4150 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		setDelay(15);
		break;
	case 1:
		g_globals->_player.checkAngle(&scene->_hotspot3);

		if (RING_INVENTORY._rope._sceneNumber == 1) {
			scene->_hotspot3.postInit();
			scene->_hotspot3.setVisage(4150);
			scene->_hotspot3.setPosition(Common::Point(175, 70));

			RING_INVENTORY._rope._sceneNumber = 4150;
			g_globals->_sceneItems.push_front(&scene->_hotspot3);
		} else {
			RING_INVENTORY._rope._sceneNumber = 1;
			scene->_hotspot3.remove();
		}

		g_globals->_player.enableControl();
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene4150::HotspotGroup1::doAction(int action) {
	Scene4150 *scene = (Scene4150 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4150, 0);
		break;
	case OBJECT_ROPE:
		scene->setAction(&scene->_action3);
		break;
	}
}

void Scene4150::HotspotGroup3::doAction(int action) {
	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4150, g_globals->getFlag(46) ? 9 : 8);
		g_globals->setFlag(46);
		break;
	case CURSOR_USE:
		SceneItem::display2(4150, 30);
		break;
	}
}

void Scene4150::HotspotGroup6::doAction(int action) {
	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4150, g_globals->getFlag(47) ? 16 : 15);
		g_globals->setFlag(47);
		break;
	case CURSOR_USE:
		SceneItem::display2(4150, 31);
		break;
	}
}

void Scene4150::Hotspot3::doAction(int action) {
	Scene4150 *scene = (Scene4150 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4150, 20);
		break;
	case CURSOR_USE:
		scene->setAction(&scene->_action3);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene4150::Scene4150() :
		_hotspot1(0, CURSOR_LOOK, 4150, 17, CURSOR_USE, 4150, 21, CURSOR_TALK, 4150, 21,
				OBJECT_SCANNER, 4150, 22, OBJECT_STUNNER, 4150, 23, LIST_END),
		_hotspot2(0, CURSOR_LOOK, 4150, 4, CURSOR_USE, 4150, 24, LIST_END),
		_hotspot7(0, CURSOR_LOOK, 4150, 1, CURSOR_USE, 4150, 25, OBJECT_ROPE, 4150, 26, LIST_END),
		_hotspot8(0, CURSOR_LOOK, 4150, 1, CURSOR_USE, 4150, 25, OBJECT_ROPE, 4150, 26, LIST_END),
		_hotspot9(0, CURSOR_LOOK, 4150, 2, CURSOR_USE, 4150, 27, OBJECT_SCANNER, 4150, 28,
				OBJECT_STUNNER, 4150, 32, LIST_END),
		_hotspot10(0, CURSOR_LOOK, 4150, 5, CURSOR_USE, 4150, 29, LIST_END),
		_hotspot11(0, CURSOR_LOOK, 4150, 6, CURSOR_USE, 4150, 29, LIST_END),
		_hotspot12(0, CURSOR_LOOK, 4150, 7, CURSOR_USE, 4150, 29, LIST_END),
		_hotspot17(0, CURSOR_LOOK, 4150, 10, CURSOR_USE, 4150, 27, OBJECT_STUNNER, 4150, 32, LIST_END),
		_hotspot18(0, CURSOR_LOOK, 4150, 11, CURSOR_USE, 4150, 27, OBJECT_STUNNER, 4150, 32, LIST_END),
		_hotspot19(0, CURSOR_LOOK, 4150, 12, CURSOR_USE, 4150, 29, LIST_END),
		_hotspot20(0, CURSOR_LOOK, 4150, 13, CURSOR_USE, 4150, 29, LIST_END),
		_hotspot21(0, CURSOR_LOOK, 4150, 13, CURSOR_USE, 4150, 29, LIST_END),
		_hotspot22(0, CURSOR_LOOK, 4150, 13, CURSOR_USE, 4150, 29, LIST_END),
		_hotspot23(0, CURSOR_LOOK, 4150, 14, CURSOR_USE, 4150, 27, OBJECT_STUNNER, 4150, 32, LIST_END),
		_hotspot24(0, CURSOR_LOOK, 4150, 14, CURSOR_USE, 4150, 27, OBJECT_STUNNER, 4150, 32, LIST_END)
		{
}

void Scene4150::postInit(SceneObjectList *OwnerList) {
	loadScene(4150);
	Scene::postInit();
	setZoomPercents(60, 85, 200, 100);

	_stripManager.addSpeaker(&_speakerQR);
	_stripManager.addSpeaker(&_speakerCDL);
	_stripManager.addSpeaker(&_speakerQText);

	_hotspot2.postInit();
	_hotspot2.setVisage(4171);
	_hotspot2.animate(ANIM_MODE_2, NULL);
	_hotspot2.fixPriority(100);
	_hotspot2.setPosition(Common::Point(76, 147));

	_hotspot1.postInit();
	_hotspot1.setPosition(Common::Point(177, 201));

	if (g_globals->getFlag(44)) {
		g_globals->_player.postInit();
		g_globals->_player.setVisage(4200);
		g_globals->_player.animate(ANIM_MODE_1, NULL);
		g_globals->_player.setObjectWrapper(new SceneObjectWrapper());
		g_globals->_player.setStrip(2);
		g_globals->_player.setPosition(Common::Point(328, 160));
		g_globals->_player._moveDiff = Common::Point(7, 4);
		g_globals->_player.disableControl();

		_sceneMode = 4151;
		setAction(&_sequenceManager, this, 4151, &g_globals->_player, NULL);

		_hotspot1.setVisage(4157);
		_hotspot1.setPosition(Common::Point(177, 201));
	} else {
		g_globals->_player.postInit();
		g_globals->_player.setVisage(4154);
		g_globals->_player._canWalk = false;
		g_globals->_player.setPosition(Common::Point(198, 202));
		g_globals->_player.disableControl();

		_hotspot1.setVisage(4152);
		setAction(&_action1);
		g_globals->setFlag(44);
	}

	if (RING_INVENTORY._rope._sceneNumber == 4150) {
		_hotspot3.postInit();
		_hotspot3.setVisage(4150);
		_hotspot3.setPosition(Common::Point(175, 70));

		g_globals->_sceneItems.push_back(&_hotspot3);
	}

	g_globals->_sceneItems.push_back(&_hotspot1);

	_hotspot5.setBounds(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
	_hotspot6.setBounds(Rect(28, 121, 80, 148));
	_hotspot7.setBounds(Rect(27, 17, 127, 100));
	_hotspot8.setBounds(Rect(62, 11, 117, 22));
	_hotspot9.setBounds(Rect(48, 78, 104, 94));
	_hotspot10.setBounds(Rect(32, 107, 58, 119));
	_hotspot11.setBounds(Rect(1, 130, 17, 163));
	_hotspot12.setBounds(Rect(1, 158, 78, 197));
	_hotspot13.setBounds(Rect(253, 11, 274, 188));
	_hotspot14.setBounds(Rect(4, 15, 29, 153));
	_hotspot15.setBounds(Rect(127, 22, 146, 133));
	_hotspot16.setBounds(Rect(165, 22, 181, 141));
	_hotspot17.setBounds(Rect(186, 31, 217, 52));
	_hotspot18.setBounds(Rect(200, 31, 251, 86));
	_hotspot19.setBounds(Rect(183, 81, 234, 111));
	_hotspot20.setBounds(Rect(188, 120, 219, 142));
	_hotspot21.setBounds(Rect(235, 122, 249, 145));
	_hotspot22.setBounds(Rect(283, 125, 320, 146));
	_hotspot23.setBounds(Rect(284, 27, 306, 160));
	_hotspot24.setBounds(Rect(257, 72, 284, 99));
	_hotspot25.setBounds(Rect(183, 155, 316, 190));
	_hotspot26.setBounds(Rect(98, 169, 238, 198));

	g_globals->_sceneItems.addItems(&_hotspot24, &_hotspot23, &_hotspot13, &_hotspot25,
		&_hotspot26, &_hotspot19, &_hotspot22, &_hotspot20, &_hotspot17, &_hotspot16,
		&_hotspot15, &_hotspot11, &_hotspot14, &_hotspot18, &_hotspot21, &_hotspot12,
		&_hotspot10, &_hotspot9, &_hotspot8, &_hotspot7, &_hotspot6, &_hotspot2,
		&_hotspot5, NULL);

	g_globals->_soundHandler.play(165);
	_soundHandler.play(311);
}

void Scene4150::signal() {
	if (_sceneMode == 4151)
		g_globals->_player.enableControl();
	else if (_sceneMode == 4152)
		g_globals->_sceneManager.changeScene(4100);
}

void Scene4150::dispatch() {
	Scene::dispatch();

	if (!_action && (g_globals->_player._position.x >= 316)) {
		g_globals->_soundHandler.fadeOut(NULL);
		_soundHandler.fadeOut(NULL);
		g_globals->_player.disableControl();
		_sceneMode = 4152;
		setAction(&_sequenceManager, this, 4152, &g_globals->_player, NULL);
	}
}

/*--------------------------------------------------------------------------
 * Scene 4000 - Village
 *
 *--------------------------------------------------------------------------*/

void Scene4250::Action1::signal() {
	Scene4250 *scene = (Scene4250 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		setDelay(3);
		break;
	case 1:
		ADD_MOVER_NULL(g_globals->_player, 6, 190);
		ADD_MOVER_NULL(scene->_hotspot3, 9, 195);
		ADD_MOVER(scene->_hotspot1, 12, 185);
		break;
	case 2:
		setDelay(30);
		break;
	case 3:
		scene->_stripManager.start(4450, this);
		break;
	case 4:
	case 7:
		setDelay(15);
		break;
	case 5:
		ADD_PLAYER_MOVER(220, 175);
		scene->_hotspot1.fixPriority(105);
		ADD_PLAYER_MOVER_NULL(scene->_hotspot1, 197, 173);
		break;
	case 6:
		scene->_stripManager.start(4470, this);
		break;
	case 8:
		g_globals->_player.enableControl();
		remove();
		break;
	}
}

void Scene4250::Action2::signal() {
	Scene4250 *scene = (Scene4250 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		ADD_PLAYER_MOVER(247, 182);
		break;
	case 1:
		g_globals->_player.setVisage(2670);
		g_globals->_player.changeZoom(50);
		g_globals->_player.setStrip(3);
		g_globals->_player.setFrame(1);
		g_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 2:
		if ((g_globals->_stripNum == 9000) || (g_globals->_stripNum == 4300)) {
			scene->_stripManager.start(4205, this);
		} else {
			scene->_stripManager.start(4490, this);
		}
		break;
	case 3:
		g_globals->_player.animate(ANIM_MODE_6, this);
		break;
	case 4:
		g_globals->_player.setVisage(2602);
		g_globals->_player.animate(ANIM_MODE_1, NULL);
		g_globals->_player.changeZoom(70);

		if ((g_globals->_stripNum == 9000) || (g_globals->_stripNum == 4300)) {
			g_globals->_player.enableControl();
			remove();
		} else {
			ADD_PLAYER_MOVER(6, 180);
			ADD_PLAYER_MOVER_NULL(scene->_hotspot1, 12, 185);
		}
		break;
	case 5:
		ADD_PLAYER_MOVER(-16, 180);
		ADD_PLAYER_MOVER_NULL(scene->_hotspot1, -12, 185);
		scene->_hotspot3.setStrip2(2);
		ADD_MOVER_NULL(scene->_hotspot5, -30, 195);
		break;
	case 6:
		g_globals->clearFlag(59);
		g_globals->clearFlag(70);
		g_globals->clearFlag(37);
		g_globals->clearFlag(114);
		g_globals->clearFlag(36);
		g_globals->clearFlag(43);
		g_globals->_sceneManager.changeScene(2100);
		break;
	}
}

void Scene4250::Action3::signal() {
	Scene4250 *scene = (Scene4250 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(3);
		break;
	case 1:
		scene->_stripManager.start(4480, this);
		break;
	case 2:
		g_globals->_player.enableControl();
		remove();
		break;
	}
}

void Scene4250::Action4::signal() {
	Scene4250 *scene = (Scene4250 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		setDelay(3);
		break;
	case 1:
		g_globals->_player.addMover(NULL);
		scene->_stripManager.start((RING_INVENTORY._helmet._sceneNumber == 4250) ? 4259 : 4256, this);
		break;
	case 2:
		ADD_PLAYER_MOVER(g_globals->_player._position.x + 5, g_globals->_player._position.y);
		break;
	case 3:
		g_globals->_player.enableControl();
		remove();
		break;
	}
}

void Scene4250::Action5::signal() {
	Scene4250 *scene = (Scene4250 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->_player.disableControl();
		setDelay(3);
		break;
	case 1:
		scene->_hotspot4.fixPriority(195);
		scene->_hotspot1.fixPriority(105);
		ADD_MOVER_NULL(g_globals->_player, 6, 185);
		ADD_MOVER_NULL(scene->_hotspot4, 9, 190);
		ADD_MOVER(scene->_hotspot1, 12, 180);
		break;
	case 2:
		ADD_PLAYER_MOVER(252, 176);
		ADD_PLAYER_MOVER_NULL(scene->_hotspot1, 197, 173);
		ADD_PLAYER_MOVER_NULL(scene->_hotspot4, 239, 195);
		break;
	case 3:
		scene->_hotspot4.fixPriority(-1);
		scene->_hotspot1.setStrip(5);
		scene->_hotspot4.setStrip(7);
		g_globals->_player.enableControl();
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene4250::Hotspot1::doAction(int action) {
	Scene4250 *scene = (Scene4250 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4250, (RING_INVENTORY._helmet._sceneNumber == 4250) ? 19 : 14);
		break;
	case CURSOR_TALK:
		g_globals->_player.disableControl();
		if (!g_globals->_sceneObjects->contains(&scene->_hotspot4)) {
			scene->setAction(&scene->_action3);
		} else {
			scene->_sceneMode = 4260;
			if (RING_INVENTORY._helmet._sceneNumber == 4250) {
				scene->_sceneMode = 4265;
				scene->setAction(&scene->_sequenceManager, scene, 4265, this, NULL);
			} else {
				scene->setAction(&scene->_sequenceManager, scene,
					g_globals->_sceneObjects->contains(&scene->_hotspot6) ? 4260 : 4262, this, NULL);
			}
		}
		break;
	case OBJECT_SCANNER:
		if (RING_INVENTORY._helmet._sceneNumber == 4250)
			SceneItem::display2(4250, 21);
		else
			SceneHotspot::doAction(action);
		break;
	case OBJECT_STUNNER:
		if (RING_INVENTORY._helmet._sceneNumber == 4250)
			SceneItem::display2(4250, 22);
		else
			SceneHotspot::doAction(action);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene4250::Hotspot2::doAction(int action) {
	//Ship with stasis field
	Scene4250 *scene = (Scene4250 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
	case CURSOR_USE:
		SceneItem::display2(4250, 16);
		break;
	case OBJECT_SCANNER:
		if ((g_globals->_stripNum == 9000) || (g_globals->_stripNum == 4300))
			scene->setAction(&scene->_action2);
		else if (g_globals->getFlag(55))
			SceneItem::display2(4250, 17);
		else {
			g_globals->setFlag(55);
			scene->setAction(&scene->_action2);
		}
		break;
	case OBJECT_STASIS_NEGATOR:
		g_globals->_player.disableControl();
		scene->_sceneMode = 4252;
		scene->setAction(&scene->_sequenceManager, scene, 4252, &g_globals->_player, this, NULL);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene4250::Hotspot4::doAction(int action) {
	Scene4250 *scene = (Scene4250 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4250, (RING_INVENTORY._helmet._sceneNumber == 4250) ? 18 : 5);
		break;
	case OBJECT_SCANNER:
		if (RING_INVENTORY._helmet._sceneNumber == 4250)
			SceneItem::display2(4250, 21);
		else
			SceneHotspot::doAction(action);
		break;
	case OBJECT_STUNNER:
		if (RING_INVENTORY._helmet._sceneNumber == 4250)
			SceneItem::display2(4250, 22);
		else
			SceneHotspot::doAction(action);
		break;
	case CURSOR_TALK:
		g_globals->_player.disableControl();
		if (!g_globals->_sceneObjects->contains(&scene->_hotspot6)) {
			scene->_sceneMode = 4254;
			scene->setAction(&scene->_sequenceManager, scene, 4263, NULL);
		} else {
			scene->_sceneMode = 4254;

			if (RING_INVENTORY._helmet._sceneNumber == 4250) {
				scene->_sceneMode = 4266;
				scene->setAction(&scene->_sequenceManager, scene, 4266, this, NULL);
			} else {
				scene->setAction(&scene->_sequenceManager, scene,
					(RING_INVENTORY._concentrator._sceneNumber == 1) ? 4255 : 4254, NULL);
			}
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene4250::Hotspot6::doAction(int action) {
	Scene4250 *scene = (Scene4250 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4250, (RING_INVENTORY._helmet._sceneNumber == 4250) ? 7 : 6);
		break;
	case OBJECT_SCANNER:
		SceneItem::display2(4250, (RING_INVENTORY._helmet._sceneNumber == 4250) ? 1 : 2);
		break;
	case OBJECT_STUNNER:
		SceneItem::display2(4250, (RING_INVENTORY._helmet._sceneNumber == 4250) ? 20 : 3);
		break;
	case OBJECT_HELMET:
		g_globals->_soundHandler.play(354);
		g_globals->_player.disableControl();
		RING_INVENTORY._helmet._sceneNumber = 4250;

		if (RING_INVENTORY._concentrator._sceneNumber == 1) {
			if (g_globals->getFlag(115)) {
				scene->_sceneMode = 4269;
				scene->setAction(&scene->_sequenceManager, scene, 4269, this, NULL);
			} else {
				g_globals->setFlag(115);
				g_globals->_events.setCursor(CURSOR_WALK);
				scene->_sceneMode = 4256;
				scene->setAction(&scene->_sequenceManager, scene, 4256, this, NULL);
			}
		} else if (RING_INVENTORY._keyDevice._sceneNumber == 1) {
			scene->_sceneMode = 4267;
			scene->setAction(&scene->_sequenceManager, scene, 4267, this, NULL);
		} else if (RING_INVENTORY._keyDevice._sceneNumber == 4300) {
			scene->_sceneMode = 4268;
			scene->setAction(&scene->_sequenceManager, scene, 4268, this, NULL);
		} else {
			g_globals->_events.setCursor(CURSOR_WALK);
			ADD_MOVER_NULL(scene->_hotspot1, 241, 169);
			scene->_sceneMode = 4261;
			scene->setAction(&scene->_sequenceManager, scene, 4261, &g_globals->_player, this, NULL);
		}
		break;
	case OBJECT_NULLIFIER:
		if (RING_INVENTORY._helmet._sceneNumber == 4250) {
			g_globals->_soundHandler.play(353);
			g_globals->_player.disableControl();
			RING_INVENTORY._helmet._sceneNumber = 1;

			scene->_sceneMode = 4257;
			scene->setAction(&scene->_sequenceManager, scene, 4257, &g_globals->_player, this, NULL);
		} else {
			SceneItem::display2(4250, 4);
		}
		break;
	case CURSOR_TALK:
		if (RING_INVENTORY._helmet._sceneNumber == 4250)
			doAction(OBJECT_HELMET);
		else {
			g_globals->_player.disableControl();
			scene->_sceneMode = 4264;
			scene->setAction(&scene->_sequenceManager, scene, 4264, this, NULL);
		}
		break;
	case CURSOR_USE:
		if (RING_INVENTORY._helmet._sceneNumber == 4250)
			doAction(OBJECT_HELMET);
		else {
			g_globals->_player.disableControl();
			if ((RING_INVENTORY._items._sceneNumber != 1) || (RING_INVENTORY._concentrator._sceneNumber != 1)) {
				scene->_sceneMode = 4258;
				scene->setAction(&scene->_sequenceManager, scene, 4258, this, NULL);
			} else {
				scene->_hotspot2.postInit();
				scene->_hotspot2.setVisage(4251);
				scene->_hotspot2.setFrame(scene->_hotspot2.getFrameCount());
				scene->_hotspot2.setPosition(Common::Point(267, 172));
				scene->_hotspot2.hide();

				scene->_sceneMode = 4259;
				scene->setAction(&scene->_sequenceManager, scene, 4259, &g_globals->_player, this, &scene->_hotspot2, NULL);
			}
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene4250::Hotspot8::doAction(int action) {
	Scene4250 *scene = (Scene4250 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
	case CURSOR_USE:
		g_globals->_sceneManager.changeScene(4300);
		break;
	case OBJECT_SCANNER:
		SceneItem::display2(4250, 24);
		break;
	case OBJECT_STUNNER:
		SceneItem::display2(4250, 25);
		break;
	case OBJECT_STASIS_NEGATOR:
		g_globals->_player.disableControl();
		scene->_sceneMode = 4270;
		scene->setAction(&scene->_sequenceManager, scene,
			(RING_INVENTORY._helmet._sceneNumber == 4250) ? 4270 : 4271, NULL);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene4250::Scene4250() :
		_hotspot7(0, CURSOR_LOOK, 4250, 0, LIST_END) {
}

void Scene4250::postInit(TsAGE::SceneObjectList *OwnerList) {
	loadScene(4250);
	Scene::postInit();
	setZoomPercents(160, 90, 185, 100);

	_stripManager.addSpeaker(&_speakerSR);
	_stripManager.addSpeaker(&_speakerSL);
	_stripManager.addSpeaker(&_speakerSText);
	_stripManager.addSpeaker(&_speakerGameText);
	_stripManager.addSpeaker(&_speakerQL);
	_stripManager.addSpeaker(&_speakerQR);
	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerPText);
	_stripManager.addSpeaker(&_speakerMText);
	_stripManager.addSpeaker(&_speakerFLText);

	_speakerSText.setTextPos(Common::Point(40, 40));
	_speakerPText.setTextPos(Common::Point(40, 100));
	_hotspot8._sceneRegionId = 16;

	g_globals->_player.postInit();
	g_globals->_player.setVisage(2602);
	g_globals->_player.animate(ANIM_MODE_1, NULL);
	g_globals->_player.setObjectWrapper(new SceneObjectWrapper());
	g_globals->_player.setPosition(Common::Point(-13, 190));
	g_globals->_player.changeZoom(-1);
	g_globals->_player._moveDiff = Common::Point(4, 1);

	_hotspot1.postInit();
	_hotspot1.setVisage(2801);
	_hotspot1.animate(ANIM_MODE_1, NULL);
	_hotspot1.setObjectWrapper(new SceneObjectWrapper());
	_hotspot1.setPosition(Common::Point(-18, 185));
	_hotspot1.changeZoom(-1);
	_hotspot1._moveDiff = Common::Point(4, 1);
	g_globals->_sceneItems.push_back(&_hotspot1);

	if (g_globals->_sceneManager._previousScene == 4300) {
		_hotspot5.postInit();
		_hotspot5.setVisage(4250);
		_hotspot5.setPosition(Common::Point(268, 168));
		_hotspot5.fixPriority(1);

		_hotspot4.postInit();
		_hotspot4.setVisage(2701);
		_hotspot4.animate(ANIM_MODE_1, NULL);
		_hotspot4.setObjectWrapper(new SceneObjectWrapper());
		_hotspot4.setPosition(Common::Point(272, 175));
		_hotspot4.setStrip(2);
		_hotspot4._moveDiff = Common::Point(4, 1);
		_hotspot4.changeZoom(70);
		g_globals->_sceneItems.push_back(&_hotspot4);

		_hotspot1.setPosition(Common::Point(197, 173));
		_hotspot1.changeZoom(70);

		g_globals->_player.setPosition(Common::Point(252, 176));
		g_globals->_player.changeZoom(70);

		_hotspot6.postInit();
		_hotspot6.setVisage(4302);
		_hotspot6.setStrip(4);
		_hotspot6.setFrame(3);
		_hotspot6.changeZoom(50);
		_hotspot6.fixPriority(70);
		_hotspot6.setPosition(Common::Point(261, 175));

		if (RING_INVENTORY._helmet._sceneNumber == 4250) {
			_hotspot6.setStrip(6);
			_hotspot6.setFrame(_hotspot6.getFrameCount());
		}

		if (g_globals->getFlag(98)) {
			g_globals->_sceneItems.push_front(&_hotspot6);
		} else {
			_hotspot6.hide();
			if ((g_globals->_stripNum == 4300) || (g_globals->_stripNum == 4301)) {
				g_globals->setFlag(98);
				g_globals->_player.setVisage(4302);
				g_globals->_player.setStrip(5);
				g_globals->_player.changeZoom(50);
				g_globals->_player.disableControl();

				_hotspot4.setPosition(Common::Point(239, 195));
				_sceneMode = 4253;
				g_globals->_sceneItems.push_front(&_hotspot6);

				setAction(&_sequenceManager, this, 4253, &g_globals->_player, &_hotspot6, &_hotspot4, NULL);
			}
		}
	} else if (g_globals->_stripNum == 9000) {
		_hotspot4.postInit();
		_hotspot4.setVisage(2701);
		_hotspot4.animate(ANIM_MODE_1, NULL);
		_hotspot4.setObjectWrapper(new SceneObjectWrapper());
		_hotspot4.setPosition(Common::Point(-15, 195));
		_hotspot4._moveDiff = Common::Point(4, 1);
		g_globals->_sceneItems.push_back(&_hotspot4);

		setAction(&_action5);
	} else {
		_hotspot3.postInit();
		_hotspot3.setVisage(4006);
		_hotspot3.animate(ANIM_MODE_1, NULL);
		_hotspot3.setObjectWrapper(new SceneObjectWrapper());
		_hotspot3.setPosition(Common::Point(-15, 195));
		_hotspot3.setStrip2(3);
		_hotspot3._moveDiff = Common::Point(4, 1);

		setAction(&_action1);
		g_globals->clearFlag(43);
		g_globals->clearFlag(114);
		g_globals->clearFlag(36);
	}

	if (g_globals->getFlag(17)) {
		g_globals->_sceneItems.push_back(&_hotspot8);
	} else {
		_hotspot2.postInit();
		_hotspot2.setVisage(4251);
		_hotspot2.setStrip2(1);
		_hotspot2.fixPriority(2);
		_hotspot2.setFrame(1);
		_hotspot2.setPosition(Common::Point(267, 172));

		g_globals->_sceneItems.push_back(&_hotspot2);
	}

	_hotspot7.setBounds(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
	g_globals->_sceneItems.push_back(&_hotspot7);
	g_globals->_soundHandler.play(185);
}

void Scene4250::signal() {
	switch (_sceneMode) {
	case 4252:
		g_globals->setFlag(17);
		g_globals->_sceneManager.changeScene(4301);
		break;
	case 4253:
		if (g_globals->_stripNum == 4301) {
			_sceneMode = 4261;
			ADD_MOVER_NULL(_hotspot1, 241, 169);
			setAction(&_sequenceManager, this, 4261, &g_globals->_player, &_hotspot6, NULL);
		} else {
			g_globals->_player.enableControl();
		}
		break;
	case 4254:
	case 4256:
	case 4257:
	case 4258:
	case 4260:
	case 4264:
	case 4265:
	case 4266:
	case 4267:
	case 4268:
	case 4269:
	case 4270:
		g_globals->_player.enableControl();
		break;
	case 4259:
		g_globals->_soundHandler.play(360);
		g_globals->_sceneManager.changeScene(9900);
		break;
	case 4261:
		RING_INVENTORY._keyDevice._sceneNumber = 1;
		g_globals->_player.enableControl();
		break;
	case 4255:
	case 4262:
	case 4263:
	default:
		break;
	}
}

void Scene4250::dispatch() {
	if (g_globals->_player.getRegionIndex() == 8)
		g_globals->_player.changeZoom(90 - (g_globals->_player._position.y - 153));
	if (g_globals->_player.getRegionIndex() == 12)
		g_globals->_player.changeZoom(70);
	if (g_globals->_player.getRegionIndex() == 15) {
		g_globals->_player.changeZoom(-1);
		g_globals->_player.fixPriority(-1);
	}

	if (_hotspot1.getRegionIndex() == 8)
		_hotspot1.changeZoom(90 - (_hotspot1._position.y - 153));
	if (_hotspot1.getRegionIndex() == 12)
		_hotspot1.changeZoom(70);
	if (_hotspot1.getRegionIndex() == 15) {
		_hotspot1.changeZoom(-1);
		_hotspot1.fixPriority(-1);
	}

	if (_hotspot4.getRegionIndex() == 8)
		_hotspot4.changeZoom(90 - (_hotspot4._position.y - 153));
	if (_hotspot4.getRegionIndex() == 12)
		_hotspot4.changeZoom(70);
	if (_hotspot4.getRegionIndex() == 15) {
		_hotspot4.changeZoom(-1);
		_hotspot4.fixPriority(-1);
	}

	Scene::dispatch();

	if (!_action) {
		if (!g_globals->getFlag(55) && (g_globals->_player.getRegionIndex() == 12)) {
			setAction(&_action4);
		}

		if (g_globals->_sceneObjects->contains(&_hotspot6) && (g_globals->_player.getRegionIndex() == 12))
			setAction(&_action4);
	}
}

/*--------------------------------------------------------------------------
 * Scene 4300 - Village - Slaver Ship
 *
 *--------------------------------------------------------------------------*/

void Scene4300::Action1::signal() {
	Scene4300 *scene = (Scene4300 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		g_globals->setFlag(56);
		g_globals->_scenePalette.addRotation(240, 254, -1);
		scene->_hotspot7.animate(ANIM_MODE_6, this);
		g_globals->_soundHandler.play(164);
		break;
	case 1:
		g_globals->_soundHandler.play(340);
		scene->_soundHandler1.play(341);
		scene->_hotspot1.remove();
		setDelay(3);
		break;
	case 2:
		scene->_soundHandler1.play(341);
		scene->_hotspot2.remove();
		setDelay(6);
		break;
	case 3:
		scene->_soundHandler1.play(341);
		scene->_hotspot3.remove();
		setDelay(6);
		break;
	case 4:
		scene->_soundHandler1.play(341);
		scene->_hotspot4.remove();
		setDelay(12);
		break;
	case 5:
		scene->_soundHandler1.play(341);
		scene->_hotspot5.remove();
		setDelay(12);
		break;
	case 6:
		scene->_soundHandler1.play(341);
		scene->_hotspot6.remove();
		setDelay(60);
		break;
	case 7:
		scene->_hotspot10.fixPriority(250);
		scene->_hotspot10.animate(ANIM_MODE_5, this);
		break;
	case 8:
		scene->_hotspot13.show();
		scene->_stripManager.start(8015, this, scene);
		break;
	case 9:
		g_globals->_soundHandler.play(350);
		g_globals->_sceneManager._fadeMode = FADEMODE_GRADUAL;
		g_globals->_events.setCursor(CURSOR_USE);
		g_globals->_player.enableControl();

		remove();
		break;
	}
}

void Scene4300::Action2::signal() {
	Scene4300 *scene = (Scene4300 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(3);
		break;
	case 1:
		scene->_stripManager.start(8016, this, scene);
		break;
	case 2:
		g_globals->_player.enableControl();
		remove();
		break;
	}
}


/*--------------------------------------------------------------------------*/

void Scene4300::Hotspot8::doAction(int action) {
	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4300, 15);
		break;
	case CURSOR_USE:
		SceneItem::display2(4300, 18);
		break;
	case OBJECT_SCANNER:
		SceneItem::display2(4300, 14);
		break;
	case OBJECT_STUNNER:
		SceneItem::display2(4300, 19);
		break;
	case OBJECT_KEY_DEVICE:
		RING_INVENTORY._keyDevice._sceneNumber = 4300;
		g_globals->_scenePalette.addRotation(240, 254, -1);
		animate(ANIM_MODE_5, NULL);

		g_globals->setFlag(99);
		g_globals->_sceneItems.push_back(this);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene4300::Hotspot9::doAction(int action) {
	switch (action) {
	case CURSOR_LOOK:
		if ((RING_INVENTORY._stasisBox2._sceneNumber == 4300) ||
			(RING_INVENTORY._concentrator._sceneNumber == 1))
			SceneItem::display2(4300, 7);
		else
			SceneItem::display2(4300, 1);
		break;
	case CURSOR_USE:
		if ((RING_INVENTORY._stasisBox2._sceneNumber == 4300) ||
			(RING_INVENTORY._concentrator._sceneNumber == 1))
			SceneItem::display2(4300, 7);
		else
			SceneItem::display2(4300, 3);
		break;
	case OBJECT_STUNNER:
		SceneItem::display2(4300, 24);
		break;
	case OBJECT_SCANNER:
		if ((RING_INVENTORY._stasisBox2._sceneNumber == 4300) ||
			(RING_INVENTORY._concentrator._sceneNumber != 1))
			SceneItem::display2(4300, 22);
		else
			SceneItem::display2(4300, 23);
		break;
	default:
		NamedHotspot::doAction(action);
		break;
	}
}

void Scene4300::Hotspot10::doAction(int action) {
	// Alien
	Scene4300 *scene = (Scene4300 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4300, 2);
		break;
	case CURSOR_TALK:
		g_globals->_player.disableControl();
		scene->setAction(&scene->_action2);
		break;
	case OBJECT_SCANNER:
		SceneItem::display2(4300, 20);
		break;
	case OBJECT_STUNNER:
		SceneItem::display2(4300, 21);
		break;
	case CURSOR_USE:
	case OBJECT_HELMET:
		g_globals->_stripNum = (action == CURSOR_USE) ? 4300 : 4301;
		g_globals->_events.setCursor(CURSOR_NONE);
		g_globals->_sceneManager.changeScene(4250);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene4300::Hotspot15::signal() {
	Scene4300 *scene = (Scene4300 *)g_globals->_sceneManager._scene;

	scene->_soundHandler2.play(345);

	_strip = (g_globals->_randomSource.getRandomNumber(6) < 2) ? 2 : 1;
	if ((RING_INVENTORY._stasisBox2._sceneNumber == 4300) ||
			(RING_INVENTORY._concentrator._sceneNumber == 1)) {
		setStrip(1);
		setFrame(1);
		animate(ANIM_MODE_NONE, NULL);
	} else {
		animate(ANIM_MODE_5, this);
	}
}

void Scene4300::Hotspot16::doAction(int action) {
	Scene4300 *scene = (Scene4300 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4300, 8);
		break;
	case CURSOR_USE:
		if ((RING_INVENTORY._stasisBox2._sceneNumber != 4300) &&
				(RING_INVENTORY._concentrator._sceneNumber != 4300)) {
			SceneItem::display2(4300, 16);
		} else {
			scene->_sceneMode = 4302;
			scene->setAction(&scene->_sequenceManager, scene, 4302, this, NULL);
		}
		break;
	case OBJECT_SCANNER:
		SceneItem::display2(4300, 27);
		break;
	case OBJECT_STUNNER:
		SceneItem::display2(4300, 24);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene4300::Hotspot17::doAction(int action) {
	Scene4300 *scene = (Scene4300 *)g_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4300, (RING_INVENTORY._stasisBox2._sceneNumber == 4300) ? 17 : 11);
		break;
	case CURSOR_USE:
		if (RING_INVENTORY._stasisBox2._sceneNumber != 4300)
			SceneItem::display2(4300, 13);
		else {
			g_globals->_scenePalette.clearListeners();
			remove();

			SceneItem::display2(4300, 12);
			RING_INVENTORY._concentrator._sceneNumber = 1;
		}
		break;
	case OBJECT_SCANNER:
		SceneItem::display2(4300, 25);
		break;
	case OBJECT_STUNNER:
		SceneItem::display2(4300, 26);
		break;
	case OBJECT_STASIS_BOX2:
		scene->_soundHandler1.play(352);
		g_globals->_events.setCursor(CURSOR_USE);
		scene->_sceneMode = 4303;

		scene->setAction(&scene->_sequenceManager, scene, 4303, this, NULL);
		break;
	}
}

void Scene4300::Hotspot19::doAction(int action) {
	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4300, 9);
		break;
	case OBJECT_SCANNER:
		SceneItem::display2(4300, 28);
		break;
	case OBJECT_STUNNER:
		SceneItem::display2(4300, 24);
		break;
	case CURSOR_USE:
		if ((RING_INVENTORY._stasisBox2._sceneNumber != 4300) &&
				(RING_INVENTORY._concentrator._sceneNumber != 4300))
			SceneItem::display2(4300, 10);
		else
			SceneItem::display2(4300, 29);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene4300::Scene4300() :
		_hotspot18(0, CURSOR_LOOK, 4300, 0, LIST_END) {
}

void Scene4300::postInit(SceneObjectList *OwnerList) {
	loadScene(4300);
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);

	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerSText);
	_stripManager.addSpeaker(&_speakerMText);
	_stripManager.addSpeaker(&_speakerFLText);

	_hotspot11.setDetails(76, 97, 102, 127, 4300, 5, 6);

	_hotspot7.postInit();
	_hotspot7.setPosition(Common::Point(90, 128));
	_hotspot7.setVisage(4303);
	_hotspot7.fixPriority(250);
	g_globals->_sceneItems.push_back(&_hotspot7);

	_hotspot9.setDetails(120, 49, 174, 91, 4300, -1, -1);

	_hotspot15.postInit();
	_hotspot15.setVisage(4300);
	_hotspot15.setPosition(Common::Point(149, 79));
	_hotspot15.signal();

	if (!g_globals->getFlag(99)) {
		_hotspot8.postInit();
		_hotspot8.setVisage(4300);
		_hotspot8.setStrip(3);
		_hotspot8.setPosition(Common::Point(196, 47));
		g_globals->_sceneItems.push_back(&_hotspot8);
	}

	if (RING_INVENTORY._concentrator._sceneNumber == 4300) {
		_hotspot17.postInit();
		_hotspot17.setVisage(4300);
		_hotspot17.setStrip(6);
		_hotspot17.fixPriority(1);
		_hotspot17.setPosition(Common::Point(200, 69));

		if (RING_INVENTORY._stasisBox2._sceneNumber == 4300)
			_hotspot17.setFrame(_hotspot17.getFrameCount());

		g_globals->_sceneItems.push_back(&_hotspot17);
	}

	if (!g_globals->getFlag(98)) {
		_hotspot10.postInit();
		_hotspot10.setVisage(4302);
		_hotspot10.setPosition(Common::Point(244, 179));
		_hotspot10.fixPriority(100);
		g_globals->_sceneItems.push_back(&_hotspot10);

		_hotspot12.postInit();
		_hotspot12.setVisage(4302);
		_hotspot12.setStrip2(3);
		_hotspot12.setPosition(Common::Point(231, 185));
		_hotspot12.fixPriority(251);
		_hotspot12.hide();

		_hotspot13.postInit();
		_hotspot13.hide();
		_hotspot13.setVisage(4302);
		_hotspot13.setStrip2(2);
		_hotspot13.setPosition(Common::Point(256, 168));
		_hotspot13.fixPriority(251);
		_hotspot13._numFrames = 1;
		_hotspot13.animate(ANIM_MODE_8, 0, NULL);
	}

	if (RING_INVENTORY._items._sceneNumber == 4300) {
		_hotspot16.postInit();
		_hotspot16.setVisage(4300);
		_hotspot16.setPosition(Common::Point(169, 141));
		_hotspot16.fixPriority(1);
		_hotspot16.setStrip(4);
		g_globals->_sceneItems.push_back(&_hotspot16);
	}

	if (g_globals->_sceneManager._previousScene == 4301) {
		g_globals->_player.disableControl();
		_hotspot7.setFrame(_hotspot7.getFrameCount());

		_hotspot1.postInit();
		_hotspot1.setVisage(4301);
		_hotspot1.fixPriority(145);
		_hotspot1.setPosition(Common::Point(160, 64));

		_hotspot2.postInit();
		_hotspot2.setVisage(4301);
		_hotspot2.setStrip2(2);
		_hotspot2.fixPriority(140);
		_hotspot2.setPosition(Common::Point(166, 90));

		_hotspot3.postInit();
		_hotspot3.setVisage(4301);
		_hotspot3.setStrip2(3);
		_hotspot3.fixPriority(135);
		_hotspot3.setPosition(Common::Point(173, 114));

		_hotspot4.postInit();
		_hotspot4.setVisage(4301);
		_hotspot4.setStrip2(4);
		_hotspot4.fixPriority(130);
		_hotspot4.setPosition(Common::Point(187, 141));

		_hotspot5.postInit();
		_hotspot5.setVisage(4301);
		_hotspot5.setStrip2(5);
		_hotspot5.fixPriority(125);
		_hotspot5.setPosition(Common::Point(201, 164));

		_hotspot6.postInit();
		_hotspot6.setVisage(4301);
		_hotspot6.setStrip2(6);
		_hotspot6.fixPriority(120);
		_hotspot6.setPosition(Common::Point(219, 186));

		setAction(&_action1);
	} else {
		g_globals->_player.disableControl();
		_sceneMode = 4304;
		setAction(&_sequenceManager, this, 4304, NULL);
	}

	_hotspot10.setBounds(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
	_hotspot19._sceneRegionId = 0;
	g_globals->_sceneItems.push_back(&_hotspot19);
	g_globals->_sceneItems.push_back(&_hotspot18);
}

void Scene4300::stripCallback(int v) {
	switch (v) {
	case 1:
		_hotspot12.show();
		_hotspot12.animate(ANIM_MODE_7, 0, NULL);
		break;
	case 2:
		_hotspot12.hide();
		break;
	}
}

void Scene4300::remove() {
	g_globals->_scenePalette.clearListeners();
	Scene::remove();
}

void Scene4300::signal() {
	switch (_sceneMode) {
	case 4302:
		RING_INVENTORY._items._sceneNumber = 1;
		_hotspot16.remove();
		g_globals->_player.enableControl();
		break;
	case 4303:
		RING_INVENTORY._stasisBox2._sceneNumber = 4300;
		_hotspot15.setStrip(1);
		_hotspot15.setFrame(1);
		_hotspot15.animate(ANIM_MODE_NONE, NULL);
		break;
	case 4304:
		_hotspot14.postInit();
		_hotspot14.setVisage(4300);
		_hotspot14.setStrip(7);
		_hotspot14.setPosition(Common::Point(60, 199));

		_gfxButton.setText(EXIT_MSG);
		_gfxButton._bounds.center(60, 193);
		_gfxButton.draw();
		_gfxButton._bounds.expandPanes();

		g_globals->_player.enableControl();
		g_globals->_scenePalette.addRotation(240, 254, -1);
		break;
	}
}

void Scene4300::dispatch() {
	if (_action)
		_action->dispatch();
}

void Scene4300::process(Event &event) {
	Scene::process(event);
	if (_gfxButton.process(event))
		g_globals->_sceneManager.changeScene(4250);
}

/*--------------------------------------------------------------------------
 * Scene 4301 - Village - Slaver Ship Keypad
 *
 *--------------------------------------------------------------------------*/

void Scene4301::Action1::synchronize(Serializer &s) {
	Action::synchronize(s);
	s.syncAsSint16LE(_field34E);
	for (int idx = 0; idx < 6; ++idx)
		s.syncAsSint16LE(_indexList[idx]);
}

void Scene4301::Action1::remove() {
	Scene4301 *scene = (Scene4301 *)g_globals->_sceneManager._scene;
	g_globals->_player.enableControl();

	for (_state = 0; _state < 6; ++_state)
		_buttonList[_state].remove();

	scene->_hotspot3.remove();
	scene->_hotspot2.remove();

	scene->_hotspot1.animate(ANIM_MODE_6, NULL);
	Action::remove();
}

void Scene4301::Action1::signal() {
	Scene4301 *scene = (Scene4301 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_soundHandler.play(164);
		scene->_hotspot1.animate(ANIM_MODE_5, this);
		break;
	case 1:
		g_globals->_soundHandler.play(335);
		g_globals->_events.setCursor(CURSOR_USE);

		scene->_hotspot2.postInit();
		scene->_hotspot2.setVisage(4303);
		scene->_hotspot2.setStrip(2);
		scene->_hotspot2.setFrame(1);
		scene->_hotspot2.setPosition(Common::Point(30, 15));
		scene->_hotspot2.fixPriority(255);

		scene->_hotspot3.postInit();
		scene->_hotspot3.setVisage(4303);
		scene->_hotspot3.setStrip(2);
		scene->_hotspot3.setFrame(2);
		scene->_hotspot3.setPosition(Common::Point(48, 29));
		scene->_hotspot3.fixPriority(255);
		scene->_hotspot3.hide();

		_field34E = 0;
		_state = 0;
		_actionIndex = 2;
		break;
	case 10:
		// Puzzle: Wrong code
		g_globals->_events.setCursor(CURSOR_NONE);
		scene->_soundHandler.play(337);
		if (scene->_hotspot3._flags & OBJFLAG_HIDE)
			scene->_hotspot3.show();
		else
			scene->_hotspot3.hide();
		setDelay(20);

		if (_state <= 8)
			_actionIndex = 10;
		++_state;
		break;
	case 11:
		for (_state = 0; _state < 6; ++_state)
			_buttonList[_state].remove();

		scene->_soundHandler.play(338);
		scene->_hotspot3.hide();

		_actionIndex = 2;
		_state = 0;
		g_globals->_events.setCursor(CURSOR_USE);
		break;
	case 20:
		// Puzzle: Correct code
		g_globals->_player.disableControl();
		scene->_soundHandler.play(339);
		scene->_hotspot3._frame = 3;
		if (scene->_hotspot3._flags & OBJFLAG_HIDE)
			scene->_hotspot3.show();
		else
			scene->_hotspot3.hide();

		if (_state <= 8)
			_actionIndex = 20;
		++_state;

		setDelay(20);
		break;
	case 21:
		scene->_puzzleDone = true;
		remove();
		break;
	}
}

void Scene4301::Action1::process(Event &event) {
	// Puzzle
	Scene4301 *scene = (Scene4301 *)g_globals->_sceneManager._scene;
	Rect buttonsRect;

	Action::process(event);
	if (event.handled || (_actionIndex != 2))
		return;

	buttonsRect = Rect(14, 35, 112, 100);
	buttonsRect.translate(30, 15);

	if ((event.eventType == EVENT_BUTTON_DOWN) && buttonsRect.contains(event.mousePos)) {
		event.handled = true;
		scene->_soundHandler.play(336);

		int buttonIndex = ((event.mousePos.y - buttonsRect.top) / 33) * 3 +
			((event.mousePos.x - buttonsRect.left) / 33);

		_buttonList[_state].postInit();
		_buttonList[_state].setVisage(4303);
		_buttonList[_state].setStrip(buttonIndex + 3);
		_buttonList[_state].setFrame(1);
		_buttonList[_state].setPosition(Common::Point((_state % 3) * 25 + 55, (_state / 3) * 25 + 121));
		_buttonList[_state].fixPriority(255);
		_buttonList[_state]._numFrames = 25;
		_buttonList[_state].animate(ANIM_MODE_5, NULL);

		_indexList[_state++] = buttonIndex;

		if (_state == 6) {
			// Six buttons pressed
			if ((_indexList[0] == 2) && (_indexList[1] == 3) && (_indexList[2] == 0) &&
				(_indexList[3] == 4) && (_indexList[4] == 1) && (_indexList[5] == 5)) {
				// Entered the correct full combination
				_state = 0;
				_actionIndex = 20;
			} else {
				// Incorrect combination entered
				_state = 0;
				_actionIndex = 10;
			}

			signal();
		}
	}

	if ((event.eventType == EVENT_KEYPRESS) && (event.kbd.keycode == Common::KEYCODE_ESCAPE)) {
		event.handled = true;
		remove();
	}
}

/*--------------------------------------------------------------------------*/

void Scene4301::Hotspot4::doAction(int action) {
	// Hatch near door
	Scene4301 *scene = (Scene4301 *)g_globals->_sceneManager._scene;

	if (action == CURSOR_USE) {
		g_globals->_player.disableControl();
		scene->setAction(&scene->_action1);
	} else {
		NamedHotspot::doAction(action);
	}
}

void Scene4301::Hotspot5::doAction(int action) {
	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4300, 0);
		break;
	case CURSOR_USE:
		SceneItem::display2(4300, 30);
		break;
	case OBJECT_SCANNER:
		SceneItem::display2(4300, 31);
		break;
	case OBJECT_STUNNER:
		SceneItem::display2(4300, 32);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene4301::postInit(SceneObjectList *OwnerList) {
	g_globals->setFlag(50);
	loadScene(4301);
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);

	_puzzleDone = false;
	RING_INVENTORY._stasisBox2._sceneNumber = 1;
	_hotspot4.setDetails(97, 76, 127, 102, 4300, 5, 6);

	_hotspot1.postInit();
	_hotspot1.setPosition(Common::Point(90, 128));
	_hotspot1.setVisage(4303);
	_hotspot1._strip = 1;
	_hotspot1._frame = 1;
	_hotspot1.fixPriority(250);

	_hotspot5.setBounds(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
	g_globals->_sceneItems.push_back(&_hotspot5);

	g_globals->_player.enableControl();
}

void Scene4301::dispatch() {
	if (_action) {
		_action->dispatch();
	} else if (_puzzleDone) {
		_puzzleDone = false;
		g_globals->clearFlag(50);
		g_globals->_sceneManager._fadeMode = FADEMODE_NONE;
		g_globals->_sceneManager.setNewScene(4300);
	}
}

} // End of namespace Ringworld

} // End of namespace TsAGE

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

#include "tsage/blue_force/blueforce_scenes6.h"
#include "tsage/blue_force/blueforce_dialogs.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace TsAGE {

namespace BlueForce {

/*--------------------------------------------------------------------------
 * Scene 600 - Crash cut-scene
 *
 *--------------------------------------------------------------------------*/

void Scene600::Action1::signal() {
	Scene600 *scene = (Scene600 *)BF_GLOBALS._sceneManager._scene;
	static byte red[3] = {220, 0, 0};

	switch (_actionIndex++) {
	case 0:
		setDelay(2);
		break;
	case 1:
		BF_GLOBALS._sound1.play(57);
		setDelay(120);
		break;
	case 2:
		scene->_sound1.play(59);
		setAction(&scene->_sequenceManager, this, 600, &scene->_object2, &scene->_ryan,
			&BF_GLOBALS._player, &scene->_skidMarks, NULL);
		break;
	case 3:
		BF_GLOBALS._sound1.play(61);
		setDelay(180);
		break;
	case 4:
		setDelay(180);
		break;
	case 5: {
		BF_GLOBALS._player.remove();
		scene->_ryan.remove();
		scene->_object2.remove();
		scene->_skidMarks.remove();

		for (int percent = 100; percent >= 0; percent -= 2) {
			BF_GLOBALS._scenePalette.fade((const byte *)&red, false, percent);
			g_system->delayMillis(5);
		}

		SynchronizedList<SceneObject *>::iterator i;
		for (i = BF_GLOBALS._sceneObjects->begin(); i != BF_GLOBALS._sceneObjects->end(); ++i) {
			SceneObject *pObj = *i;
			pObj->addMover(NULL);
			pObj->setObjectWrapper(NULL);
			pObj->animate(ANIM_MODE_NONE, NULL);
		}

		BF_GLOBALS._screenSurface.fillRect(BF_GLOBALS._screenSurface.getBounds(), 0);
		BF_GLOBALS._v51C44 = 1;
		scene->loadScene(999);
		setDelay(5);
		break;
	}
	case 6:
		setDelay(5);
		break;
	case 7:
		BF_GLOBALS._v51C44 = 0;
		remove();
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene600::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(600);
	setZoomPercents(0, 100, 200, 100);
	_sceneBounds.moveTo(320, 0);

	_sound1.play(58);
	_sound1.holdAt(1);

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.hide();
	BF_GLOBALS._player.setPosition(Common::Point(639, 0));
	BF_GLOBALS._player.disableControl();

	_skidMarks.postInit();
	_skidMarks.hide();
	_object2.postInit();

	_ryan.postInit();
	_ryan.setVisage(600);
	_ryan.setStrip(7);
	_ryan.setPosition(Common::Point(417, 82));

	BF_GLOBALS.clearFlag(onDuty);
	BF_INVENTORY.setObjectScene(INV_TICKET_BOOK, 60);
	BF_INVENTORY.setObjectScene(INV_MIRANDA_CARD, 60);
	_sceneMode = 600;

	setAction(&_action1, this);
}

void Scene600::signal() {
	BF_GLOBALS._sceneManager.changeScene(620);
}

// WORKAROUND: Fix for original game bug where the global scrolling object follower
// remains set to an object within the scene that is no longer active
void Scene600::remove() {
	BF_GLOBALS._scrollFollower = &BF_GLOBALS._player;

	SceneExt::remove();
}

/*--------------------------------------------------------------------------
 * Scene 620 - Hospital cut-scene
 *
 *--------------------------------------------------------------------------*/

void Scene620::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(999);

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.disableControl();
	BF_GLOBALS._player.setVisage(621);
	BF_GLOBALS._player.setPosition(Common::Point(47, 96));

	static const uint32 black = 0;
	add2Faders((const byte *)&black, 2, 621, this);
}

void Scene620::signal() {
	static const uint32 black = 0;

	switch (_sceneMode++) {
	case 0:
	case 3:
	case 9:
	case 12:
		_timer1.set(60, this);
		break;
	case 1:
	case 4:
	case 7:
	case 10:
	case 13:
	case 16:
	case 19:
		addFader((const byte *)&black, 2, this);
		break;
	case 2:
		BF_GLOBALS._player.remove();
		_object1.postInit();
		_object1.setVisage(622);
		_object1.setPosition(Common::Point(101, 41));
		add2Faders((const byte *)&black, 2, 622, this);
		break;
	case 5:
		_object1.remove();

		_object2.postInit();
		_object2.setVisage(623);
		_object2.setPosition(Common::Point(216, 4));
		add2Faders((const byte *)&black, 2, 623, this);
		break;
	case 6:
		_object2.animate(ANIM_MODE_5, this);
		break;
	case 8:
		_object2.remove();

		_object3.postInit();
		_object3.setVisage(624);
		_object3.setFrame(1);
		_object3.setPosition(Common::Point(28, 88));
		add2Faders((const byte *)&black, 2, 624, this);
		break;
	case 11:
		_object3.remove();

		_object4.postInit();
		_object4.setVisage(625);
		_object4.setPosition(Common::Point(168, 8));
		add2Faders((const byte *)&black, 2, 625, this);
		break;
	case 14:
		_object4.remove();

		_object5.postInit();
		_object5.setVisage(626);
		_object5.setPosition(Common::Point(249, 183));
		add2Faders((const byte *)&black, 2, 626, this);
		break;
	case 15:
		_object5.animate(ANIM_MODE_5, this);
		break;
	case 17:
		_object5.remove();

		_object6.postInit();
		_object6.setVisage(627);
		_object6.setPosition(Common::Point(65, 24));
		add2Faders((const byte *)&black, 2, 627, this);
		break;
	case 18:
		_object6.animate(ANIM_MODE_5, this);
		break;
	case 20:
		_object6.remove();
		BF_GLOBALS._bookmark = bArrestedDrunk;
		BF_GLOBALS._v4CEB6 = 0;
		BF_GLOBALS._dayNumber = 3;
		BF_GLOBALS._sceneManager.changeScene(271);
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 666 - Death Scene
 *
 *--------------------------------------------------------------------------*/

void Scene666::Action1::signal() {
	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.hide();
		setDelay(6);
		break;
	case 1:
		BF_GLOBALS._game->restartGame();
		break;
	}
}

/*--------------------------------------------------------------------------*/

bool Scene666::Item1::startAction(CursorType action, Event &event) {
	return true;
}

/*--------------------------------------------------------------------------*/

void Scene666::postInit(SceneObjectList *OwnerList) {
	BF_GLOBALS._sound1.play(27);
	SceneExt::postInit();
	BF_GLOBALS._interfaceY = 200;
	loadScene(999);
	BF_GLOBALS._screenSurface.fillRect(BF_GLOBALS._screenSurface.getBounds(), 0);

	if (BF_GLOBALS._dayNumber == 0) {
		BF_GLOBALS._dayNumber = 1;
		BF_GLOBALS._deathReason = BF_GLOBALS._randomSource.getRandomNumber(23);
	}

	BF_GLOBALS._scenePalette.loadPalette(BF_GLOBALS._sceneManager._previousScene);
	T2_GLOBALS._uiElements._active = false;

	_item1.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 666, -1, -1, -1, 1, NULL);
	BF_GLOBALS._player.postInit();
	BF_GLOBALS._events.setCursor(CURSOR_WALK);

	if (BF_GLOBALS._sceneManager._previousScene == 271) {
		setAction(&_action1);
	} else {
		switch (BF_GLOBALS._deathReason) {
		case 4:
		case 18:
		case 19:
		case 20:
			BF_GLOBALS._scenePalette.loadPalette(668);
			BF_GLOBALS._player.setVisage(668);
			BF_GLOBALS._player.setStrip2(1);
			BF_GLOBALS._player.setPosition(Common::Point(77, 155));
			BF_GLOBALS._player.animate(ANIM_MODE_5, this);
			break;
		case 5:
			BF_GLOBALS._scenePalette.loadPalette(900);
			BF_GLOBALS._scenePalette.refresh();
			BF_GLOBALS._player.setVisage(666);
			BF_GLOBALS._player.setPosition(Common::Point(60, 160));
			signal();
			break;
		case 7:
		case 11:
		case 12:
		case 22:
			BF_GLOBALS._scenePalette.loadPalette(667);
			BF_GLOBALS._scenePalette.refresh();

			_object1.postInit();
			_object2.postInit();
			_object3.postInit();
			setAction(&_sequenceManager, this, 6660, &BF_GLOBALS._player, &_object1, &_object2,
				&_object3, NULL);
			break;
		case 13:
		case 14:
			BF_GLOBALS._scenePalette.loadPalette(665);
			BF_GLOBALS._scenePalette.refresh();
			BF_GLOBALS._player.setVisage(665);
			BF_GLOBALS._player.setPosition(Common::Point(80, 140));
			signal();
			break;
		case 24:
			BF_GLOBALS._player.setVisage(664);
			BF_GLOBALS._player.setPosition(Common::Point(70, 160));
			signal();
			break;
		default:
			BF_GLOBALS._scenePalette.loadPalette(669);
			BF_GLOBALS._scenePalette.refresh();
			BF_GLOBALS._player.setVisage(669);
			BF_GLOBALS._player.setStrip(1);
			BF_GLOBALS._player.setPosition(Common::Point(27, 27));
			BF_GLOBALS._player.animate(ANIM_MODE_5, this);
			break;
		}
	}
}

void Scene666::remove() {
	BF_GLOBALS._sound1.fadeOut2(NULL);
	BF_GLOBALS._scrollFollower = &BF_GLOBALS._player;
	SceneExt::remove();
	T2_GLOBALS._uiElements._active = true;
}

void Scene666::signal() {
	BF_GLOBALS._player.enableControl();
	Rect textRect, sceneBounds;

	_text._color1 = 19;
	_text._color2 = 9;
	_text._color3 = 13;
	_text._fontNumber = 4;
	_text._width = 150;

	Common::String msg = g_resourceManager->getMessage(666, BF_GLOBALS._deathReason);
	sceneBounds = BF_GLOBALS._sceneManager._scene->_sceneBounds;
	sceneBounds.collapse(4, 2);
	BF_GLOBALS.gfxManager()._font.getStringBounds(msg.c_str(), textRect, _text._width);
	textRect.moveTo(160, 10);
	textRect.contain(sceneBounds);

	_text.setup(msg);
	_text.setPosition(Common::Point(textRect.left, textRect.top));
	_text.setPriority(255);
	_text.show();
}

/*--------------------------------------------------------------------------
 * Scene 690 - Decking
 *
 *--------------------------------------------------------------------------*/

bool Scene690::Object1::startAction(CursorType action, Event &event) {
	Scene690 *scene = (Scene690 *)BF_GLOBALS._sceneManager._scene;

	if ((action == CURSOR_USE) && (scene->_object2._strip == 1)) {
		BF_GLOBALS._player.disableControl();
		BF_GLOBALS._walkRegions.enableRegion(1);
		BF_GLOBALS._walkRegions.enableRegion(6);
		scene->_sceneMode = 6901;
		scene->setAction(&scene->_sequenceManager, scene, 6901, &BF_GLOBALS._player,
			&scene->_object2, &scene->_object1, &scene->_object4, &scene->_object5, NULL);
		return true;
	} else {
		return NamedObject::startAction(action, event);
	}
}

bool Scene690::Object2::startAction(CursorType action, Event &event) {
	Scene690 *scene = (Scene690 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (scene->_object2._strip == 3) {
			scene->_object6.postInit();
			scene->_object6.hide();
			scene->_object6.fixPriority(1);
			scene->_object6.setDetails(690, 21, 17, 23, 1, (SceneItem *)NULL);

			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 6902;
			scene->setAction(&scene->_sequenceManager, scene, 6902, &BF_GLOBALS._player,
				&scene->_object2, &scene->_object6, NULL);
			return true;
		}
		break;
	case CURSOR_TALK:
		scene->_stripManager.start(6900, &BF_GLOBALS._stripProxy);
		return true;
	default:
		break;
	}

	return NamedObject::startAction(action, event);
}

/*--------------------------------------------------------------------------*/

void Scene690::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(690);
	setZoomPercents(125, 80, 140, 100);
	BF_GLOBALS._sound1.fadeSound(48);

	if (BF_GLOBALS._dayNumber == 0)
		BF_GLOBALS._dayNumber = 1;

	_stripManager.addSpeaker(&_jakeSpeaker);

	_object1.postInit();
	_object1.setVisage(690);
	_object1.setStrip2(2);
	_object1.fixPriority(188);
	_object1.setPosition(Common::Point(50, 166));
	_object1.setDetails(690, 4, 17, 26, 1, (SceneItem *)NULL);

	_object3.postInit();
	_object3.setVisage(690);
	_object3.fixPriority(100);
	_object3.setPosition(Common::Point(238, 125));
	_object3.setDetails(690, 7, 17, 28, 1, (SceneItem *)NULL);

	_object2.postInit();
	_object2.setVisage(694);
	_object2.setStrip2(3);
	_object2.fixPriority(125);
	_object2.setPosition(Common::Point(100, 134));
	_object2.setDetails(690, 12, -1, 11, 1, (SceneItem *)NULL);

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.disableControl();
	BF_GLOBALS._player._moveDiff.x = 8;

	_object4.postInit();
	_object4.setDetails(690, 13, -1, -1, 1, (SceneItem *)NULL);

	_object5.postInit();
	_object5.setDetails(690, 14, -1, -1, 1, (SceneItem *)NULL);

	_sceneMode = 6903;
	setAction(&_sequenceManager, this, 6903, &BF_GLOBALS._player, &_object3, &_object4, &_object5, NULL);

	_item1.setDetails(Rect(45, 28, 74, 48), 690, 0, 15, 20, 1, NULL);
	_item2.setDetails(Rect(0, 0, 129, 78), 690, 1, 15, 20, 1, NULL);
	_item9.setDetails(Rect(4, 26, 35, 143), 690, 29, 17, 22, 1, NULL);
	_item10.setDetails(Rect(87, 29, 109, 112), 690, 29, 17, 22, 1, NULL);
	_item11.setDetails(Rect(135, 30, 160, 120), 690, 29, 17, 22, 1, NULL);
	_item3.setDetails(Rect(37, 89, 84, 117), 690, 2, 16, 21, 1, NULL);
	_item4.setDetails(Rect(123, 98, 201, 131), 690, 5, -1, -1, 1, NULL);
	_item7.setDetails(Rect(285, 95, 303, 133), 690, 8, -1, 25, 1, NULL);
	_item16.setDetails(Rect(166, 35, 210, 116), 690, 32, -1, -1, 1, NULL);
	_item6.setDetails(Rect(255, 44, 289, 130), 690, 7, 17, 24, 1, NULL);
	_item8.setDetails(Rect(299, 44, 314, 53), 690, 27, 15, -1, 1, NULL);
	_item5.setDetails(Rect(130, 20, 319, 135), 690, 6, 16, 28, 1, NULL);
	_item12.setDetails(1, 690, 10, -1, -1, 1);
	_item13.setDetails(2, 690, 30, -1, -1, 1);
	_item14.setDetails(3, 690, 31, -1, -1, 1);
	_item15.setDetails(4, 690, 9, -1, -1, 1);
	_item17.setDetails(Rect(143, 0, 182, 14), 690, 33, -1, -1, 1, NULL);
}

void Scene690::signal() {
	switch (_sceneMode) {
	case 6901:
		BF_GLOBALS._sound1.fadeOut2(NULL);
		BF_GLOBALS._sceneManager.changeScene(710);
		break;
	case 6902:
		BF_GLOBALS._player.enableControl();
		break;
	case 6903:
		BF_GLOBALS._walkRegions.disableRegion(1);
		BF_GLOBALS._walkRegions.disableRegion(6);
		BF_GLOBALS._player.enableControl();
		break;
	default:
		break;
	}
}


} // End of namespace BlueForce

} // End of namespace TsAGE

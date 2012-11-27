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
#include "tsage/ringworld/ringworld_scenes4.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace TsAGE {

namespace Ringworld {

/*--------------------------------------------------------------------------
 * Scene 3500 - Ringworld Scan
 *
 *--------------------------------------------------------------------------*/

void Scene3500::Action1::signal() {
	Scene3500 *scene = (Scene3500 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(10);
		break;
	case 1:
		scene->_stripManager.start(3500, this);
		break;
	case 2:
		setDelay(3);
		break;
	case 3:
		g_globals->_sceneManager.changeScene(9999);
		break;
	}
}

void Scene3500::Action2::signal() {
	Scene3500 *scene = (Scene3500 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(10);
		break;
	case 1:
		scene->_stripManager.start(3501, this);
		break;
	case 2:
		setDelay(3);
		break;
	case 3:
		g_globals->_sceneManager.changeScene(2012);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene3500::postInit(SceneObjectList *OwnerList) {
	loadScene((g_globals->_stripNum == 3600) ? 3600 : 3500);
	Scene::postInit();

	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerMText);
	_stripManager.addSpeaker(&_speakerSText);

	g_globals->_sceneManager._scene->_sceneBounds.contain(g_globals->_sceneManager._scene->_backgroundBounds);
	g_globals->_sceneOffset.x = (g_globals->_sceneManager._scene->_sceneBounds.top / 160) * 160;

	setAction((g_globals->_stripNum == 3600) ? (Action *)&_action2 : (Action *)&_action1);
}

/*--------------------------------------------------------------------------
 * Scene 3700 - Remote Viewer
 *
 *--------------------------------------------------------------------------*/

#define VIEW_FRAME_DELAY 10

Scene3700::Viewer::Viewer() {
	_images1.setVisage(3705, 1);
	_images2.setVisage(3705, 2);

	_frameList[0] = 1;
	for (int idx = 1; idx <= 3; ++idx)
		_frameList[idx] = g_globals->_randomSource.getRandomNumber(4) + 1;

	_active = true;
	_countdownCtr = 0;
	_percentList[0] = 120;
	_percentList[1] = 50;
	_percentList[2] = 75;
	_percentList[3] = 114;
}

void Scene3700::Viewer::synchronize(Serializer &s) {
	SceneObject::synchronize(s);
	s.syncAsByte(_active);
	s.syncAsSint16LE(_countdownCtr);
	for (int idx = 0; idx < 4; ++idx) {
		s.syncAsSint16LE(_frameList[idx]);
		s.syncAsSint16LE(_percentList[idx]);
	}
}

void Scene3700::Viewer::dispatch() {
	if (_active) {
		if (_countdownCtr-- <= 0) {
			_countdownCtr = VIEW_FRAME_DELAY;

			for (int idx = 3; idx > 1; --idx)
				_frameList[idx] = _frameList[idx - 1];

			int newFrame;
			do {
				newFrame = g_globals->_randomSource.getRandomNumber(4) + 1;
			} while (newFrame == _frameList[2]);

			_frameList[1] = newFrame;
			_flags |= OBJFLAG_PANES;
		}
	}
}

void Scene3700::Viewer::reposition() {
	_bounds = Rect(123, 40, 285, 123);
}

void Scene3700::Viewer::draw() {
	Region *priorityRegion = g_globals->_sceneManager._scene->_priorities.find(1);

	for (int idx = 0; idx < 4; ++idx) {
		Visage &v = (idx == 0) ? _images1 : _images2;

		GfxSurface img = v.getFrame(_frameList[idx]);
		Rect destRect = img.getBounds();
		destRect.resize(img, (_position.x - g_globals->_sceneOffset.x),
			(_position.y  - g_globals->_sceneOffset.y - _yDiff), _percentList[idx]);

		destRect.translate(-g_globals->_sceneManager._scene->_sceneBounds.left,
			-g_globals->_sceneManager._scene->_sceneBounds.top);

		g_globals->gfxManager().copyFrom(img, destRect, priorityRegion);
	}
}

/*--------------------------------------------------------------------------*/

void Scene3700::Action1::signal() {
	Scene3700 *scene = (Scene3700 *)g_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(10);
		break;
	case 1:
		scene->_stripManager.start(2162, this);
		break;
	case 2:
		scene->_viewer._active = false;
		setDelay(90);
		break;
	case 3:
		scene->_soundHandler.play(196);
		scene->_viewer.hide();

		scene->_hotspot1.postInit();
		scene->_hotspot1.setVisage(3710);
		scene->_hotspot1.setStrip(1);
		scene->_hotspot1.setFrame(1);
		scene->_hotspot1.setPosition(Common::Point(204, 120));

		setDelay(90);
		break;
	case 4:
		scene->_soundHandler.play(197);
		scene->_hotspot1.hide();

		scene->_hotspot2.postInit();
		scene->_hotspot2.setVisage(3710);
		scene->_hotspot2.setStrip(2);
		scene->_hotspot2.setFrame(1);
		scene->_hotspot2.setPosition(Common::Point(204, 120));

		setDelay(30);
		break;
	case 5:
		scene->_soundHandler.play(198);
		scene->_hotspot2.hide();
		scene->_hotspot1.show();
		setDelay(90);
		break;
	case 6:
		scene->_stripManager.start(2166, this);
		break;
	case 7:
		setDelay(60);
		break;
	case 8:
		scene->_hotspot1.remove();
		scene->_hotspot2.show();
		g_globals->setFlag(59);
		setDelay(30);
		break;
	case 9:
		g_globals->_sceneManager.changeScene(2100);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene3700::postInit(TsAGE::SceneObjectList *OwnerList) {
	Scene::postInit();
	loadScene(3700);

	_stripManager.addSpeaker(&_speakerSText);
	_stripManager.addSpeaker(&_speakerMText);
	_stripManager.addSpeaker(&_speakerMR);
	_speakerSText.setTextPos(Common::Point(20, 15));
	_speakerMText.setTextPos(Common::Point(20, 15));

	_viewer.postInit();
	_viewer.setVisage(3705);
	_viewer.setStrip(1);
	_viewer.setFrame(2);
	_viewer.setPosition(Common::Point(195, 83));

	setAction(&_action1);
	g_globals->_soundHandler.play(195);
}

} // End of namespace Ringworld

} // End of namespace TsAGE

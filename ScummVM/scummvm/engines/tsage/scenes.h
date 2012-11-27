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

#ifndef TSAGE_SCENES_H
#define TSAGE_SCENES_H

#include "common/scummsys.h"
#include "tsage/converse.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/saveload.h"

namespace TsAGE {

class Scene : public StripCallback {
private:
	void drawBackgroundObjects();
public:
	int _field12;
	int _screenNumber;
	int _activeScreenNumber;
	int _sceneMode;
	StripManager _stripManager;

	Rect _backgroundBounds;
	GfxSurface _backSurface;
	Rect _sceneBounds;
	Rect _oldSceneBounds;
	int _enabledSections[256];
	int _zoomPercents[256];
	ScenePriorities _priorities;
	SceneObjectList _bgSceneObjects;

	int _fieldA;
	int _fieldE;
public:
	Scene();
	virtual ~Scene();

	virtual Common::String getClassName() { return "Scene"; }
	virtual void synchronize(Serializer &s);
	virtual void stripCallback(int v) {}
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void process(Event &event);
	virtual void dispatch();
	virtual void loadScene(int sceneNum);
	virtual void refreshBackground(int xAmount, int yAmount);

	void setZoomPercents(int yStart, int minPercent, int yEnd, int maxPercent);
	void loadBackground(int xAmount, int yAmount);

	void loadSceneData(int sceneNum);
};

class SceneManager : public GameHandler, public SaveListener {
private:
	void disposeRegions() {
		// No need to do anything, since regions will be freed automatically when the scene is
	}
	Scene *getNewScene();
public:
	Scene *_scene;
	bool _hasPalette;
	int _loadMode;
	int _sceneNumber;
	int _previousScene;
	int _nextSceneNumber;
	FadeMode _fadeMode;
	Common::Point _sceneBgOffset;
	int _sceneLoadCount;
	Rect _scrollerRect;
	int _objectCount;
public:
	SceneManager();
	virtual ~SceneManager();

	virtual void listenerSynchronize(Serializer &s);
	void setNewScene(int sceneNumber);
	void checkScene();
	void sceneChange();
	void fadeInIfNecessary();
	void changeScene(int newSceneNumber);
	void setBgOffset(const Common::Point &pt, int loadCount);

	void removeAction(Action *action) {
		// Not currently implemented because addAction method doesn't seem to have any callers
	}

	static void setup();
	static void setBackSurface();
	static void saveListener(int saveMode);
	static void loadNotifier(bool postFlag);
};

class Game {
protected:
	SynchronizedList<GameHandler *> _handlers;

	static bool notLockedFn(GameHandler *g);
	virtual void handleSaveLoad(bool saveFlag, int &saveSlot, Common::String &saveName);
public:
	virtual ~Game() {}

	void addHandler(GameHandler *entry) { _handlers.push_back(entry); }
	void removeHandler(GameHandler *entry) { _handlers.remove(entry); }

	void execute();
	virtual void start() = 0;
	virtual void restart() {}
	virtual void restartGame();
	virtual void saveGame();
	virtual void restoreGame();
	virtual void quitGame();
	virtual void endGame(int resNum, int lineNum) {}
	virtual Scene *createScene(int sceneNumber) = 0;
	virtual void processEvent(Event &event) {}
	virtual void rightClick() {}
	virtual bool canSaveGameStateCurrently() = 0;
	virtual bool canLoadGameStateCurrently() = 0;
};

} // End of namespace TsAGE

#endif

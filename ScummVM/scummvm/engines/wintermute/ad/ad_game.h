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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */
#ifndef WINTERMUTE_ADGAME_H
#define WINTERMUTE_ADGAME_H

#include "engines/wintermute/ad/ad_types.h"
#include "engines/wintermute/base/base_game.h"

namespace Wintermute {
class AdItem;
class AdInventory;
class AdSceneState;
class AdScene;
class AdItem;
class AdObject;
class AdSentence;
class AdInventoryBox;
class AdResponseContext;
class AdResponseBox;
class AdGame : public BaseGame {
public:
	virtual bool onScriptShutdown(ScScript *script);

	virtual bool onMouseLeftDown();
	virtual bool onMouseLeftUp();
	virtual bool onMouseLeftDblClick();
	virtual bool onMouseRightDown();
	virtual bool onMouseRightUp();

	virtual bool displayDebugInfo();

	bool addSpeechDir(const char *dir);
	bool removeSpeechDir(const char *dir);
	char *findSpeechFile(char *StringID);

	bool deleteItem(AdItem *Item);
	char *_itemsFile;
	bool _tempDisableSaveState;
	virtual bool resetContent();
	bool addItem(AdItem *item);
	AdItem *getItemByName(const char *name);

	AdObject *_inventoryOwner;
	bool isItemTaken(char *itemName);
	bool registerInventory(AdInventory *inv);
	bool unregisterInventory(AdInventory *inv);
	virtual bool displayContent(bool update = true, bool displayAll = false);

	bool gameResponseUsed(int ID);
	bool addGameResponse(int ID);
	bool resetResponse(int ID);

	bool branchResponseUsed(int ID);
	bool addBranchResponse(int ID);
	bool clearBranchResponses(char *name);
	bool startDlgBranch(const char *branchName, const char *scriptName, const char *eventName);
	bool endDlgBranch(const char *branchName, const char *scriptName, const char *eventName);
	virtual bool windowLoadHook(UIWindow *win, char **buf, char **params);
	virtual bool windowScriptMethodHook(UIWindow *win, ScScript *script, ScStack *stack, const char *name);

	AdSceneState *getSceneState(const char *filename, bool saving);
	BaseViewport *_sceneViewport;

	int _texItemLifeTime;
	int _texWalkLifeTime;
	int _texStandLifeTime;
	int _texTalkLifeTime;

	TTalkSkipButton _talkSkipButton;

	virtual bool getVersion(byte *verMajor, byte *verMinor, byte *extMajor, byte *extMinor);
	bool scheduleChangeScene(const char *filename, bool fadeIn);
	void setPrevSceneName(const char *name);
	void setPrevSceneFilename(const char *name);

	AdItem *_selectedItem;
	bool cleanup();
	DECLARE_PERSISTENT(AdGame, BaseGame)

	void finishSentences();
	bool showCursor();

	TGameStateEx _stateEx;

	bool displaySentences(bool frozen);
	void addSentence(AdSentence *sentence);
	bool changeScene(const char *filename, bool fadeIn);
	bool removeObject(AdObject *object);
	bool addObject(AdObject *object);
	AdScene *_scene;
	bool initLoop();
	AdGame(const Common::String &gameId);
	virtual ~AdGame();

	BaseArray<AdObject *> _objects;

	virtual bool loadFile(const char *filename);
	virtual bool loadBuffer(byte *buffer, bool complete = true);

	bool loadItemsFile(const char *filename, bool merge = false);
	bool loadItemsBuffer(byte *buffer, bool merge = false);

	// scripting interface
	virtual ScValue *scGetProperty(const Common::String &name);
	virtual bool scSetProperty(const char *name, ScValue *value);
	virtual bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name);
	bool validMouse();
private:
	virtual bool externalCall(ScScript *script, ScStack *stack, ScStack *thisStack, char *name);

	AdObject *_invObject;
	BaseArray<AdInventory *> _inventories;
	char *_scheduledScene;
	bool _scheduledFadeIn;
	char *_prevSceneName;
	char *_prevSceneFilename;
	char *_debugStartupScene;
	char *_startupScene;
	bool _initialScene;
	bool _smartItemCursor;
	BaseArray<char *> _speechDirs;
	BaseArray<AdItem *> _items;

	BaseArray<AdSentence *> _sentences;

	BaseArray<AdSceneState *> _sceneStates;
	BaseArray<char *> _dlgPendingBranches;

	BaseArray<AdResponseContext *> _responsesBranch;
	BaseArray<AdResponseContext *> _responsesGame;

	AdResponseBox *_responseBox;
	AdInventoryBox *_inventoryBox;
};

} // end of namespace Wintermute

#endif

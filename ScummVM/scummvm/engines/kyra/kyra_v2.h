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

#ifndef KYRA_KYRA_V2_H
#define KYRA_KYRA_V2_H

#include "kyra/kyra_v1.h"
#include "kyra/gui_v1.h"
#include "kyra/wsamovie.h"
#include "kyra/item.h"

#include "common/list.h"
#include "common/hashmap.h"

namespace Kyra {

struct FrameControl {
	uint16 index;
	uint16 delay;
};

struct ItemAnimDefinition {
	Item itemIndex;
	uint8 numFrames;
	const FrameControl *frames;
};

struct ActiveItemAnim {
	uint16 currentFrame;
	uint32 nextFrameTime;
};

class Screen_v2;

class KyraEngine_v2 : public KyraEngine_v1 {
friend class Debugger_v2;
friend class GUI_v2;
public:
	struct EngineDesc {
		// Generic shape related
		int itemShapeStart;
		const uint8 *characterFrameTable;

		// Scene script
		int firstAnimSceneScript;

		// Animation script specific
		int animScriptFrameAdd;

		// Item specific
		Item maxItemId;
	};

	KyraEngine_v2(OSystem *system, const GameFlags &flags, const EngineDesc &desc);
	~KyraEngine_v2();

	virtual void pauseEngineIntern(bool pause);

	virtual Screen_v2 *screen_v2() const = 0;

	void delay(uint32 time, bool update = false, bool isMainLoop = false);

	const EngineDesc &engineDesc() const { return _desc; }
protected:
	EngineDesc _desc;

	// run
	uint32 _pauseStart;
	bool _runFlag;
	bool _showOutro;

	virtual void update() = 0;
	virtual void updateWithText() = 0;

	// detection
	int _lang;

	// Input
	virtual int inputSceneChange(int x, int y, int unk1, int unk2) = 0;

	// Animator
	struct AnimObj {
		uint16 index;
		uint16 type;
		bool enabled;
		uint16 needRefresh;
		uint16 specialRefresh;
		uint16 animFlags;
		uint16 flags;
		int16 xPos1, yPos1;
		uint8 *shapePtr;
		uint16 shapeIndex1;
		uint16 animNum;
		uint16 shapeIndex3;
		uint16 shapeIndex2;
		int16 xPos2, yPos2;
		int16 xPos3, yPos3;
		int16 width, height;
		int16 width2, height2;
		uint16 palette;
		AnimObj *nextObject;
	};

	void allocAnimObjects(int actors, int anims, int items);
	AnimObj *_animObjects;

	AnimObj *_animActor;
	AnimObj *_animAnims;
	AnimObj *_animItems;

	bool _drawNoShapeFlag;
	AnimObj *_animList;

	AnimObj *initAnimList(AnimObj *list, AnimObj *entry);
	AnimObj *addToAnimListSorted(AnimObj *list, AnimObj *entry);
	AnimObj *deleteAnimListEntry(AnimObj *list, AnimObj *entry);

	virtual void refreshAnimObjects(int force) = 0;
	void refreshAnimObjectsIfNeed();

	void flagAnimObjsSpecialRefresh();
	void flagAnimObjsForRefresh();

	virtual void clearAnimObjects() = 0;

	virtual void drawAnimObjects() = 0;
	virtual void drawSceneAnimObject(AnimObj *obj, int x, int y, int drawLayer) = 0;
	virtual void drawCharacterAnimObject(AnimObj *obj, int x, int y, int drawLayer) = 0;

	virtual void updateCharacterAnim(int) = 0;
	virtual void updateSceneAnim(int anim, int newFrame) = 0;

	void addItemToAnimList(int item);
	void deleteItemAnimEntry(int item);

	virtual void animSetupPaletteEntry(AnimObj *){}

	virtual void setCharacterAnimDim(int w, int h) = 0;
	virtual void resetCharacterAnimDim() = 0;

	virtual int getScale(int x, int y) = 0;

	uint8 *_screenBuffer;

	// Scene
	struct SceneDesc {
		char filename1[10];
		char filename2[10];

		uint16 exit1, exit2, exit3, exit4;
		uint8 flags;
		uint8 sound;
	};

	SceneDesc *_sceneList;
	int _sceneListSize;
	uint16 _currentScene;

	uint16 _sceneExit1, _sceneExit2, _sceneExit3, _sceneExit4;
	int _sceneEnterX1, _sceneEnterY1, _sceneEnterX2, _sceneEnterY2,
	    _sceneEnterX3, _sceneEnterY3, _sceneEnterX4, _sceneEnterY4;
	int _specialExitCount;
	uint16 _specialExitTable[25];
	bool checkSpecialSceneExit(int num, int x, int y);

	bool _overwriteSceneFacing;

	virtual void enterNewScene(uint16 newScene, int facing, int unk1, int unk2, int unk3) = 0;

	void runSceneScript6();

	EMCData _sceneScriptData;
	EMCState _sceneScriptState;

	virtual int trySceneChange(int *moveTable, int unk1, int unk2) = 0;

	// Animation
	virtual void restorePage3() = 0;

	struct SceneAnim {
		uint16 flags;
		int16 x, y;
		int16 x2, y2;
		int16 width, height;
		uint16 specialSize;
		int16 shapeIndex;
		uint16 wsaFlag;
		char filename[14];
	};

	SceneAnim _sceneAnims[16];
	WSAMovie_v2 *_sceneAnimMovie[16];

	void freeSceneAnims();

	bool _specialSceneScriptState[10];
	bool _specialSceneScriptStateBackup[10];
	EMCState _sceneSpecialScripts[10];
	uint32 _sceneSpecialScriptsTimer[10];
	int _lastProcessedSceneScript;
	bool _specialSceneScriptRunFlag;

	void updateSpecialSceneScripts();

	// Sequences
	EMCData _animationScriptData;
	EMCState _animationScriptState;
	Common::Array<const Opcode *> _opcodesAnimation;

	void runAnimationScript(const char *filename, int allowSkip, int resetChar, int newShapes, int shapeUnload);

	int o2a_setAnimationShapes(EMCState *script);
	int o2a_setResetFrame(EMCState *script);

	char _animShapeFilename[14];

	uint8 *_animShapeFiledata;
	int _animShapeCount;
	int _animShapeLastEntry;

	int _animNewFrame;
	int _animDelayTime;

	int _animResetFrame;

	int _animShapeWidth, _animShapeHeight;
	int _animShapeXAdd, _animShapeYAdd;

	bool _animNeedUpdate;

	virtual int initAnimationShapes(uint8 *filedata) = 0;
	void processAnimationScript(int allowSkip, int resetChar);
	virtual void uninitAnimationShapes(int count, uint8 *filedata) = 0;

	// Shapes
	typedef Common::HashMap<int, uint8 *> ShapeMap;
	ShapeMap _gameShapes;

	uint8 *getShapePtr(int index) const;
	void addShapeToPool(const uint8 *data, int realIndex, int shape);
	void addShapeToPool(uint8 *shpData, int index);
	void remShapeFromPool(int idx);

	int _characterShapeFile;
	virtual void loadCharacterShapes(int shapes) = 0;

	// pathfinder
	int _movFacingTable[600];
	int _pathfinderFlag;
	bool _smoothingPath;

	int findWay(int curX, int curY, int dstX, int dstY, int *moveTable, int moveTableSize);

	bool directLinePassable(int x, int y, int toX, int toY);

	int pathfinderInitPositionTable(int *moveTable);
	int pathfinderAddToPositionTable(int index, int v1, int v2);
	int pathfinderInitPositionIndexTable(int tableLen, int x, int y);
	int pathfinderAddToPositionIndexTable(int index, int v);
	void pathfinderFinializePath(int *moveTable, int unk1, int x, int y, int moveTableSize);

	int _pathfinderPositionTable[400];
	int _pathfinderPositionIndexTable[200];

	// items
	struct ItemDefinition {
		Item id;
		uint16 sceneId;
		int16 x;
		uint8 y;
	};

	void initItemList(int size);

	Item _hiddenItems[100];

	ItemDefinition *_itemList;
	int _itemListSize;

	int _itemInHand;
	int _savedMouseState;

	int findFreeItem();
	int countAllItems();

	int findItem(uint16 sceneId, Item id);
	int findItem(Item item);

	void resetItemList();
	void resetItem(int index);

	virtual void setMouseCursor(Item item) = 0;

	void setHandItem(Item item);
	void removeHandItem();

	// character
	struct Character {
		uint16 sceneId;
		int16 dlgIndex;
		uint8 height;
		uint8 facing;
		uint16 animFrame;
		byte walkspeed;
		Item inventory[20];
		int16 x1, y1;
		int16 x2, y2;
		int16 x3, y3;
	};

	Character _mainCharacter;
	int _mainCharX, _mainCharY;
	int _charScale;

	void moveCharacter(int facing, int x, int y);
	int updateCharPos(int *table, int force = 0);
	void updateCharPosWithUpdate();

	uint32 _updateCharPosNextUpdate;

	virtual int getCharacterWalkspeed() const = 0;
	virtual void updateCharAnimFrame(int *table) = 0;

	// chat
	int _vocHigh;

	const char *_chatText;
	int _chatObject;
	uint32 _chatEndTime;
	int _chatVocHigh, _chatVocLow;
	bool _chatTextEnabled;

	EMCData _chatScriptData;
	EMCState _chatScriptState;

	virtual void setDlgIndex(int dlgIndex) = 0;

	virtual void randomSceneChat() = 0;

	// unknown
	int _unk4, _unk5;
	bool _unkSceneScreenFlag1;
	bool _unkHandleSceneChangeFlag;

	// opcodes
	int o2_getCharacterX(EMCState *script);
	int o2_getCharacterY(EMCState *script);
	int o2_getCharacterFacing(EMCState *script);
	int o2_getCharacterScene(EMCState *script);
	int o2_setCharacterFacingOverwrite(EMCState *script);
	int o2_trySceneChange(EMCState *script);
	int o2_moveCharacter(EMCState *script);
	int o2_checkForItem(EMCState *script);
	int o2_defineItem(EMCState *script);
	int o2_addSpecialExit(EMCState *script);
	int o2_delay(EMCState *script);
	int o2_update(EMCState *script);
	int o2_getShapeFlag1(EMCState *script);
	int o2_waitForConfirmationClick(EMCState *script);
	int o2_randomSceneChat(EMCState *script);
	int o2_setDlgIndex(EMCState *script);
	int o2_getDlgIndex(EMCState *script);
	int o2_defineRoomEntrance(EMCState *script);
	int o2_runAnimationScript(EMCState *script);
	int o2_setSpecialSceneScriptRunTime(EMCState *script);
	int o2_defineScene(EMCState *script);
	int o2_setSpecialSceneScriptState(EMCState *script);
	int o2_clearSpecialSceneScriptState(EMCState *script);
	int o2_querySpecialSceneScriptState(EMCState *script);
	int o2_setHiddenItemsEntry(EMCState *script);
	int o2_getHiddenItemsEntry(EMCState *script);
	int o2_disableTimer(EMCState *script);
	int o2_enableTimer(EMCState *script);
	int o2_setTimerCountdown(EMCState *script);
	int o2_setVocHigh(EMCState *script);
	int o2_getVocHigh(EMCState *script);
};

} // End of namespace Kyra

#endif

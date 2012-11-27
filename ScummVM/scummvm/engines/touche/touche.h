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

#ifndef TOUCHE_ENGINE_H
#define TOUCHE_ENGINE_H

#include "common/array.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/random.h"
#include "common/rect.h"
#include "common/util.h"

#include "audio/mixer.h"

#include "engines/engine.h"

#include "touche/console.h"

/**
 * This is the namespace of the Touche engine.
 *
 * Status of this engine: ???
 *
 * Games using this engine:
 * - Touche: The Adventures of the Fifth Musketeer
 */
namespace Touche {

struct Area {
	Common::Rect r;
	int16 srcX, srcY;

	Area() {
		srcX = srcY = 0;
	}

	Area(int16 x, int16 y, int16 w, int16 h) {
		r = Common::Rect(x, y, x + w, y + h);
		srcX = srcY = 0;
	}

	bool clip(const Common::Rect &rect) {
		const int dx = r.left - rect.left;
		if (dx < 0) {
			srcX -= dx;
		}
		const int dy = r.top - rect.top;
		if (dy < 0) {
			srcY -= dy;
		}
		if (rect.left > r.left) {
			r.left = rect.left;
		}
		if (rect.top > r.top) {
			r.top = rect.top;
		}
		if (rect.right < r.right) {
			r.right = rect.right;
		}
		if (rect.bottom < r.bottom) {
			r.bottom = rect.bottom;
		}
		return (r.right > r.left && r.bottom > r.top);
	}
};

struct KeyChar {
	uint16 num;
	uint16 flags;
	int16 currentAnimCounter;
	int16 strNum;
	int16 walkDataNum;
	int16 spriteNum;
	Common::Rect prevBoundingRect;
	Common::Rect boundingRect;
	int16 xPos;
	int16 yPos;
	int16 zPos;
	int16 xPosPrev;
	int16 yPosPrev;
	int16 zPosPrev;
	int16 prevWalkDataNum;
	uint16 textColor;
	int16 inventoryItems[4];
	int16 money;
	int16 pointsDataNum;
	int16 currentWalkBox;
	uint16 prevPointsDataNum;
	int16 currentAnim;
	int16 facingDirection;
	int16 currentAnimSpeed;
	int16 framesList[16];
	int16 framesListCount;
	int16 currentFrame;
	int16 anim1Start;
	int16 anim1Count;
	int16 anim2Start;
	int16 anim2Count;
	int16 anim3Start;
	int16 anim3Count;
	int16 followingKeyCharNum;
	int16 followingKeyCharPos;
	uint16 sequenceDataIndex;
	uint16 sequenceDataOffset;
	int16 walkPointsListIndex;
	int16 walkPointsList[40];
	uint16 scriptDataStartOffset;
	uint16 scriptDataOffset;
	int16 *scriptStackPtr;
	int16 delay;
	int16 waitingKeyChar;
	int16 waitingKeyCharPosTable[3];
	int16 scriptStackTable[40];
};

struct Script {
	uint8 opcodeNum;
	uint32 dataOffset;
	int16 keyCharNum;
	uint8 *dataPtr;
	int16 *stackDataPtr;
	int16 *stackDataBasePtr;
	int16 quitFlag;
	int16 stackDataTable[500];

	void init(uint8 *data) {
		dataPtr = data;
		stackDataPtr = stackDataBasePtr = &stackDataTable[499];
		dataOffset = 0;
		quitFlag = 0;
	}

	uint8 readByte(uint32 offs) const {
		return *(dataPtr + offs);
	}

	int16 readWord(uint32 offs) const {
		return READ_LE_UINT16(dataPtr + offs);
	}

	uint8 readNextByte() {
		uint8 val = readByte(dataOffset);
		++dataOffset;
		return val;
	}

	int16 readNextWord() {
		int16 val = readWord(dataOffset);
		dataOffset += 2;
		return val;
	}
};

struct TalkEntry {
	int16 otherKeyChar;
	int16 talkingKeyChar;
	int16 num;
};

struct ConversationChoice {
	int16 num;
	int16 msg;
};

struct AnimationEntry {
	int16 num;
	int16 x;
	int16 y;
	int16 dx;
	int16 dy;
	int16 posNum;
	int16 delayCounter;
	int16 displayCounter;
	Common::Rect displayRect;
};

struct SequenceEntry {
	int16 sprNum;
	int16 seqNum;
};

struct SpriteData {
	uint32 size;
	uint8 *ptr;
	uint16 bitmapWidth;
	uint16 bitmapHeight;
	uint16 w;
	uint16 h;
};

struct InventoryState {
	int16 displayOffset;
	int16 lastItem;
	int16 itemsPerLine;
	int16 *itemsList;
};

struct ProgramPointData {
	int16 x, y, z;
	int16 order;
};

struct ProgramWalkData {
	int16 point1;
	int16 point2;
	int16 clippingRect;
	int16 area1;
	int16 area2;
};

struct ProgramAreaData {
	Area area;
	int16 id;
	int16 state;
	int16 animCount;
	int16 animNext;
};

struct ProgramBackgroundData {
	Area area;
	int16 type;
	int16 offset;
	int16 scaleMul;
	int16 scaleDiv;
};

struct ProgramHitBoxData {
	int16 item;
	int16 talk;
	uint16 state;
	int16 str;
	int16 defaultStr;
	int16 actions[8];
	Common::Rect hitBoxes[2];
};

struct ProgramActionScriptOffsetData {
	int16 object1;
	int16 action;
	int16 object2;
	uint16 offset;
};

struct ProgramKeyCharScriptOffsetData {
	int16 keyChar;
	uint16 offset;
};

struct ProgramConversationData {
	int16 num;
	uint16 offset;
	int16 msg;
};

enum {
	kDebugEngine   = 1 << 0,
	kDebugGraphics = 1 << 1,
	kDebugResource = 1 << 2,
	kDebugOpcodes  = 1 << 3,
	kDebugMenu     = 1 << 4,
	kDebugCharset  = 1 << 5
};

enum ResourceType {
	kResourceTypeRoomImage = 0,
	kResourceTypeSequence,
	kResourceTypeSpriteImage,
	kResourceTypeIconImage,
	kResourceTypeRoomInfo,
	kResourceTypeProgram,
	kResourceTypeMusic,
	kResourceTypeSound
};

enum TalkMode {
	kTalkModeTextOnly = 0,
	kTalkModeVoiceOnly,
	kTalkModeVoiceAndText,
	kTalkModeCount
};

enum ScriptFlag {
	kScriptStopped = 1 << 0,
	kScriptPaused  = 1 << 1
};

enum SaveLoadMode {
	kSaveGameState = 0,
	kLoadGameState
};

enum InventoryArea {
	kInventoryCharacter = 0,
	kInventoryMoneyDisplay,
	kInventoryGoldCoins,
	kInventorySilverCoins,
	kInventoryMoney,
	kInventoryScroller1,
	kInventoryObject1,
	kInventoryObject2,
	kInventoryObject3,
	kInventoryObject4,
	kInventoryObject5,
	kInventoryObject6,
	kInventoryScroller2
};

enum {
	kScreenWidth = 640,
	kScreenHeight = 400,
	kRoomHeight = 352,
	kStartupEpisode = 90,
	kCycleDelay = 1000 / (1193180 / 32768),
	kIconWidth = 58,
	kIconHeight = 42,
	kCursorWidth = 58,
	kCursorHeight = 42,
	kTextHeight = 16,
	kMaxProgramDataSize = 61440,
	kMaxSaveStates = 100
};

enum StringType {
	kStringTypeDefault,
	kStringTypeConversation
};

void readGameStateDescription(Common::ReadStream *f, char *description, int len);
Common::String generateGameStateFileName(const char *target, int slot, bool prefixOnly = false);
int getGameStateFileSlot(const char *filename);

enum GameState {
	kGameStateGameLoop,
	kGameStateOptionsDialog,
	kGameStateQuitDialog
};

class MidiPlayer;

class ToucheEngine: public Engine {
public:

	enum {
		NUM_FLAGS = 2000,
		NUM_KEYCHARS = 32,
		NUM_SPRITES = 7,
		NUM_SEQUENCES = 7,
		NUM_CONVERSATION_CHOICES = 40,
		NUM_TALK_ENTRIES = 16,
		NUM_ANIMATION_ENTRIES = 4,
		NUM_INVENTORY_ITEMS = 100,
		NUM_DIRTY_RECTS = 30,
		NUM_DIRECTIONS = 135
	};

	typedef void (ToucheEngine::*OpcodeProc)();

	ToucheEngine(OSystem *system, Common::Language language);
	virtual ~ToucheEngine();

	// Engine APIs
	virtual Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;
	virtual void syncSoundSettings();
	GUI::Debugger *getDebugger() { return _console; }

protected:

	void restart();
	void readConfigurationSettings();
	void writeConfigurationSettings();
	void mainLoop();
	void processEvents(bool handleKeyEvents = true);
	void runCycle();
	int16 getRandomNumber(int max);
	void changePaletteRange();
	void playSoundInRange();
	void resetSortedKeyCharsTable();
	void setupEpisode(int num);
	void setupNewEpisode();
	void drawKeyChar(KeyChar *key);
	void sortKeyChars();
	void runKeyCharScript(KeyChar *key);
	void runCurrentKeyCharScript(int mode);
	void executeScriptOpcode(int16 param);
	void initKeyChars(int keyChar);
	void setKeyCharTextColor(int keyChar, uint16 color);
	void waitForKeyCharPosition(int keyChar);
	void setKeyCharBox(int keyChar, int value);
	void setKeyCharFrame(int keyChar, int16 type, int16 value1, int16 value2);
	void setKeyCharFacingDirection(int keyChar, int16 dir);
	void initKeyCharScript(int keyChar, int16 spriteNum, int16 seqDataIndex, int16 seqDataOffs);
	uint16 findProgramKeyCharScriptOffset(int keyChar) const;
	bool scrollRoom(int keyChar);
	void drawIcon(int x, int y, int num);
	void centerScreenToKeyChar(int keyChar);
	void waitForKeyCharsSet();
	void redrawRoom();
	void fadePalette(int firstColor, int colorCount, int scale, int scaleInc, int fadingStepsCount);
	void fadePaletteFromFlags();
	void moveKeyChar(uint8 *dst, int dstPitch, KeyChar *key);
	void changeKeyCharFrame(KeyChar *key, int keyChar);
	void setKeyCharRandomFrame(KeyChar *key);
	void setKeyCharMoney();
	const char *getString(int num) const;
	int getStringWidth(int num) const;
	void drawString(uint16 color, int x, int y, int16 num, StringType strType = kStringTypeDefault);
	void drawGameString(uint16 color, int x1, int y, const char *str);
	int restartKeyCharScriptOnAction(int action, int obj1, int obj2);
	void buildSpriteScalingTable(int z1, int z2);
	void drawSpriteOnBackdrop(int num, int x, int y);
	void updateTalkFrames(int keyChar);
	void setKeyCharTalkingFrame(int keyChar);
	void lockUnlockHitBox(int num, int lock);
	void drawHitBoxes();
	void showCursor(bool show);
	void setCursor(int num);
	void setDefaultCursor(int num);
	void handleLeftMouseButtonClickOnInventory();
	void handleRightMouseButtonClickOnInventory();
	void handleMouseInput(int flag);
	void handleMouseClickOnRoom(int flag);
	void handleMouseClickOnInventory(int flag);
	void scrollScreenToPos(int num);
	void clearRoomArea();
	void startNewMusic();
	void startNewSound();
	void updateSpeech();
	int handleActionMenuUnderCursor(const int16 *actions, int offs, int y, int str);

	void redrawBackground();
	void addRoomArea(int num, int flag);
	void updateRoomAreas(int num, int flags);
	void setRoomAreaState(int num, uint16 state);
	void findAndRedrawRoomRegion(int num);
	void updateRoomRegions();
	void redrawRoomRegion(int num, bool markForRedraw);

	void initInventoryObjectsTable();
	void initInventoryLists();
	void setupInventoryAreas();
	void drawInventory(int index, int flag);
	void drawAmountOfMoneyInInventory();
	void packInventoryItems(int index);
	void appendItemToInventoryList(int index);
	void addItemToInventory(int inventory, int16 item);
	void removeItemFromInventory(int inventory, int16 item);

	void resetTalkingVars();
	int updateKeyCharTalk(int pauseFlag);
	const char *formatTalkText(int *y, int *h, const char *text);
	void addToTalkTable(int talkingKeyChar, int num, int otherKeyChar);
	void removeFromTalkTable(int keyChar);
	void addConversationChoice(int16 num);
	void removeConversationChoice(int16 num);
	void runConversationScript(uint16 offset);
	void findConversationByNum(int16 num);
	void clearConversationChoices();
	void scrollDownConversationChoice();
	void scrollUpConversationChoice();
	void drawCharacterConversation();
	void drawConversationString(int num, uint16 color);
	void clearConversationArea();
	void setupConversationScript(int num);
	void handleConversation();

	void buildWalkPointsList(int keyChar);
	int findWalkDataNum(int pointNum1, int pointNum2);
	void changeWalkPath(int num1, int num2, int16 val);
	void adjustKeyCharPosToWalkBox(KeyChar *key, int moveType);
	void lockWalkPath(int num1, int num2);
	void unlockWalkPath(int num1, int num2);
	void resetPointsData(int num);
	bool sortPointsData(int num1, int num2);
	void updateKeyCharWalkPath(KeyChar *key, int16 dx, int16 dy, int16 dz);
	void markWalkPoints(int keyChar);
	void buildWalkPath(int dstPosX, int dstPosY, int keyChar);

	void addToAnimationTable(int num, int posNum, int keyChar, int delayCounter);
	void copyAnimationImage(int dstX, int dstY, int w, int h, const uint8 *src, int srcX, int srcY, int fillColor);
	void drawAnimationImage(AnimationEntry *anim);
	void processAnimationTable();
	void clearAnimationTable();

	void addToDirtyRect(const Common::Rect &r);
	void clearDirtyRects();
	void setPalette(int firstColor, int colorCount, int redScale, int greenScale, int blueScale);
	void updateScreenArea(int x, int y, int w, int h);
	void updateEntireScreen();
	void updateDirtyScreenAreas();
	void updatePalette();

	void saveGameStateData(Common::WriteStream *stream);
	void loadGameStateData(Common::ReadStream *stream);
	virtual Common::Error saveGameState(int num, const Common::String &description);
	virtual Common::Error loadGameState(int num);
	virtual bool canLoadGameStateCurrently();
	virtual bool canSaveGameStateCurrently();

	ToucheConsole *_console;

	void setupOpcodes();
	void op_nop();
	void op_jnz();
	void op_jz();
	void op_jmp();
	void op_true();
	void op_false();
	void op_push();
	void op_not();
	void op_add();
	void op_sub();
	void op_mul();
	void op_div();
	void op_mod();
	void op_and();
	void op_or();
	void op_neg();
	void op_testGreater();
	void op_testEquals();
	void op_testLower();
	void op_fetchScriptWord();
	void op_testGreaterOrEquals();
	void op_testLowerOrEquals();
	void op_testNotEquals();
	void op_endConversation();
	void op_stopScript();
	void op_getFlag();
	void op_setFlag();
	void op_fetchScriptByte();
	void op_getKeyCharWalkBox();
	void op_startSound();
	void op_moveKeyCharToPos();
	void op_loadRoom();
	void op_updateRoom();
	void op_startTalk();
	void op_loadSprite();
	void op_loadSequence();
	void op_setKeyCharBox();
	void op_initKeyCharScript();
	void op_setKeyCharFrame();
	void op_setKeyCharDirection();
	void op_clearConversationChoices();
	void op_addConversationChoice();
	void op_removeConversationChoice();
	void op_getInventoryItem();
	void op_setInventoryItem();
	void op_startEpisode();
	void op_setConversationNum();
	void op_enableInput();
	void op_disableInput();
	void op_faceKeyChar();
	void op_getKeyCharCurrentAnim();
	void op_getCurrentKeyChar();
	void op_isKeyCharActive();
	void op_setPalette();
	void op_changeWalkPath();
	void op_lockWalkPath();
	void op_initializeKeyChar();
	void op_setupWaitingKeyChars();
	void op_updateRoomAreas();
	void op_unlockWalkPath();
	void op_addItemToInventoryAndRedraw();
	void op_giveItemTo();
	void op_setHitBoxText();
	void op_fadePalette();
	void op_getInventoryItemFlags();
	void op_drawInventory();
	void op_stopKeyCharScript();
	void op_restartKeyCharScript();
	void op_getKeyCharCurrentWalkBox();
	void op_getKeyCharPointsDataNum();
	void op_setupFollowingKeyChar();
	void op_startAnimation();
	void op_setKeyCharTextColor();
	void op_startMusic();
	void op_sleep();
	void op_setKeyCharDelay();
	void op_lockHitBox();
	void op_removeItemFromInventory();
	void op_unlockHitBox();
	void op_addRoomArea();
	void op_setKeyCharFlags();
	void op_unsetKeyCharFlags();
	void op_loadSpeechSegment();
	void op_drawSpriteOnBackdrop();
	void op_startPaletteFadeIn();
	void op_startPaletteFadeOut();
	void op_setRoomAreaState();

	void res_openDataFile();
	void res_closeDataFile();
	void res_allocateTables();
	void res_deallocateTables();
	uint32 res_getDataOffset(ResourceType type, int num, uint32 *size = NULL);
	void res_loadSpriteImage(int num, uint8 *dst);
	void res_loadProgram(int num);
	void res_decodeProgramData();
	void res_loadRoom(int num);
	void res_loadSprite(int num, int index);
	void res_loadSequence(int num, int index);
	void res_decodeScanLineImageRLE(uint8 *dst, int lineWidth);
	void res_loadBackdrop();
	void res_loadImage(int num, uint8 *dst);
	void res_loadImageHelper(uint8 *imgData, int imgWidth, int imgHeight);
	void res_loadSound(int flag, int num);
	void res_stopSound();
	void res_loadMusic(int num);
	void res_loadSpeech(int num);
	void res_loadSpeechSegment(int num);
	void res_stopSpeech();

	void drawButton(void *button);
	void redrawMenu(void *menu);
	void handleMenuAction(void *menu, int actionId);
	void handleOptions(int forceDisplay);
	void drawActionsPanel(int dstX, int dstY, int deltaX, int deltaY);
	void drawConversationPanelBorder(int dstY, int srcX, int srcY);
	void drawConversationPanel();
	void printStatusString(const char *str);
	void clearStatusString();
	int displayQuitDialog();
	void displayTextMode(int str);

	Common::Point getMousePos() const;

	MidiPlayer *_midiPlayer;

	Common::Language _language;
	Common::RandomSource _rnd;

	bool _inp_leftMouseButtonPressed;
	bool _inp_rightMouseButtonPressed;
	int _disabledInputCounter;
	bool _hideInventoryTexts;
	GameState _gameState;
	bool _displayQuitDialog;
	int _saveLoadCurrentPage;
	int _saveLoadCurrentSlot;

	int _newMusicNum;
	int _currentMusicNum;
	int _newSoundNum;
	int _newSoundDelay;
	int _newSoundPriority;
	int _playSoundCounter;
	bool _speechPlaying;
	Audio::SoundHandle _sfxHandle;
	Audio::SoundHandle _speechHandle;

	int16 _inventoryList1[101];
	int16 _inventoryList2[101];
	int16 _inventoryList3[7];
	InventoryState _inventoryStateTable[3];
	int16 _inventoryItemsInfoTable[NUM_INVENTORY_ITEMS];
	int16 *_inventoryVar1;
	int16 *_inventoryVar2;
	int _currentCursorObject;
	Common::Rect _inventoryAreasTable[13];

	int _talkTextMode;
	int _talkListEnd;
	int _talkListCurrent;
	bool _talkTextRectDefined;
	bool _talkTextDisplayed;
	bool _talkTextInitialized;
	bool _skipTalkText;
	int _talkTextSpeed;
	int _keyCharTalkCounter;
	int _talkTableLastTalkingKeyChar;
	int _talkTableLastOtherKeyChar;
	int _talkTableLastStringNum;
	int _objectDescriptionNum;
	TalkEntry _talkTable[NUM_TALK_ENTRIES];

	bool _conversationChoicesUpdated;
	int _conversationReplyNum;
	bool _conversationEnded;
	int _conversationNum;
	int _scrollConversationChoiceOffset;
	int _currentConversation;
	bool _disableConversationScript;
	bool _conversationAreaCleared;
	ConversationChoice _conversationChoicesTable[NUM_CONVERSATION_CHOICES];

	int16 _flagsTable[NUM_FLAGS];
	KeyChar _keyCharsTable[NUM_KEYCHARS];
	KeyChar *_sortedKeyCharsTable[NUM_KEYCHARS];
	int _currentKeyCharNum;

	int _newEpisodeNum;
	int _currentEpisodeNum;

	int _currentAmountOfMoney;
	int _giveItemToKeyCharNum;
	int _giveItemToObjectNum;
	int _giveItemToCounter;
	int _currentRoomNum;
	int _waitingSetKeyCharNum1;
	int _waitingSetKeyCharNum2;
	int _waitingSetKeyCharNum3;
	uint8 _updatedRoomAreasTable[200];
	Common::Rect _moveKeyCharRect;
	Common::Point _screenOffset;
	int _currentObjectNum;
	int _processRandomPaletteCounter;
	int16 _spriteScalingIndex[1000];
	int16 _spriteScalingTable[1000];

	bool _fastWalkMode;
	bool _fastMode;

	AnimationEntry _animationTable[NUM_ANIMATION_ENTRIES];

	Script _script;
	const OpcodeProc *_opcodesTable;
	int _numOpcodes;

	Common::File _fData;
	Common::File _fSpeech[2];
	int _compressedSpeechData;

	uint8 *_textData;
	uint8 *_backdropBuffer;
	uint8 *_menuKitData;
	uint8 *_convKitData;
	uint8 *_sequenceDataTable[NUM_SEQUENCES];
	uint8 *_programData;
	uint32 _programDataSize;
	uint8 *_mouseData;
	uint8 *_iconData;

	SpriteData _spritesTable[NUM_SPRITES];
	SequenceEntry _sequenceEntryTable[NUM_SEQUENCES];
	int _currentBitmapWidth;
	int _currentBitmapHeight;
	int _currentImageWidth;
	int _currentImageHeight;
	int _roomWidth;

	uint8 *_programTextDataPtr;
	Common::Array<Common::Rect> _programRectsTable;
	Common::Array<ProgramPointData> _programPointsTable;
	Common::Array<ProgramWalkData> _programWalkTable;
	Common::Array<ProgramAreaData> _programAreaTable;
	Common::Array<ProgramBackgroundData> _programBackgroundTable;
	Common::Array<ProgramHitBoxData> _programHitBoxTable;
	Common::Array<ProgramActionScriptOffsetData> _programActionScriptOffsetTable;
	Common::Array<ProgramKeyCharScriptOffsetData> _programKeyCharScriptOffsetTable;
	Common::Array<ProgramConversationData> _programConversationTable;
	Common::Rect _cursorObjectRect;
	Common::Rect _talkTextRect, _talkTextRect2;
	Common::Rect _screenRect;
	Common::Rect _roomAreaRect;

	bool _roomNeedRedraw;
	int _fullRedrawCounter;
	int _menuRedrawCounter;
	uint8 *_offscreenBuffer;
	uint8 _paletteBuffer[256 * 3];
	Common::Rect _dirtyRectsTable[NUM_DIRTY_RECTS];
	int _dirtyRectsTableCount;

	static const uint8 _directionsTable[NUM_DIRECTIONS];
};

/*
	FLAGS LIST

	115 : don't set backdrop palette on room loading
	118 : current amount of money
	119 : current cursor object
	176 : keychar max direction
	266 : keychar direction override
	267 : don't decode picture/sprite images (in load_image_helper)
	268 : don't decode picture/sprite images
	269 : disable room background animations
	270 : play random sound
	290 : process random palette
	295 : game cycle counter (incremented)
	296 : game cycle counter (incremented)
	297 : game cycle counter (incremented)
	298 : game cycle counter (decremented)
	299 : game cycle counter (decremented)
	600 : last ascii key press
	603 : fade palette "scale" increment (in vbl handler)
	605 : fade palette "scale"
	606 : inventory redraw disabled
	607 : first palette color to fade
	608 : last palette color to fade
	609 : max fade palette "scale"
	610 : min fade palette "scale"
	611 : quit game
	612 : random number modulo
	613 : last generated random number
	614 : room scroll x offset
	615 : room scroll y offset
	616 : disable room scrolling
	617 : current speech file number
	618 : hide mouse cursor
	621 : enable french version "features"
	902 : debug/draw walk boxes
	911 : load scripts/programs from external files
*/

} // namespace Touche

#endif

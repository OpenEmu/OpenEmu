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

#ifndef TUCKER_ENGINE_H
#define TUCKER_ENGINE_H

#include "common/file.h"
#include "common/util.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/random.h"
#include "common/stream.h"

#include "video/flic_decoder.h"

#include "audio/mixer.h"

#include "engines/engine.h"

#include "tucker/console.h"

namespace Audio {
class RewindableAudioStream;
} // End of namespace Audio

/**
 * This is the namespace of the Tucker engine.
 *
 * Status of this engine: Complete
 *
 * Games using this engine:
 * - Bud Tucker in Double Trouble
 */
namespace Tucker {

struct Action {
	int key;
	int testFlag1Num;
	int testFlag1Value;
	int testFlag2Num;
	int testFlag2Value;
	int speech;
	int flipX;
	int index;
	int delay;
	int setFlagNum;
	int setFlagValue;
	int fxNum;
	int fxDelay;
};

struct Sprite {
	int state;
	int gfxBackgroundOffset;
	int updateDelay;
	int backgroundOffset;
	int needUpdate;
	int stateIndex;
	int counter;
	int disabled;
	int colorType;
	int animationFrame;
	int firstFrame;
	uint8 *animationData;
	int prevState;
	int nextAnimationFrame;
	int prevAnimationFrame;
	int defaultUpdateDelay;
	int xSource;
	int yMaxBackground;
	int flipX;
};

struct CharPos {
	int xPos;
	int yPos;
	int xSize;
	int ySize;
	int xWalkTo;
	int yWalkTo;
	int flagNum;
	int flagValue;
	int direction;
	int name;
	int description;
};

struct SpriteFrame {
	int sourceOffset;
	int xOffset;
	int yOffset;
	int xSize;
	int ySize;
};

struct SpriteAnimation {
	int numParts;
	int rotateFlag;
	int firstFrameIndex;
};

struct Data {
	int sourceOffset;
	int xSize;
	int ySize;
	int xDest;
	int yDest;
	int index;
};

struct LocationAnimation {
	int graphicNum;
	int animInitCounter;
	int animCurrentCounter;
	int animLastCounter;
	int getFlag;
	int inventoryNum;
	int flagNum;
	int flagValue;
	int selectable;
	int standX;
	int standY;
	int drawFlag;
};

struct LocationObject {
	int xPos;
	int yPos;
	int xSize;
	int ySize;
	int textNum;
	int locationNum;
	int toX;
	int toY;
	int toX2;
	int toY2;
	int toWalkX2;
	int toWalkY2;
	int standX;
	int standY;
	int cursorNum;
};

struct LocationSound {
	int startFxSpriteState;
	int startFxSpriteNum;
	int updateType;
	int stopFxSpriteState;
	int stopFxSpriteNum;
	int offset;
	int type;
	int volume;
	int flagValueStartFx;
	int flagValueStopFx;
	int flagNum;
	int num;
};

struct LocationMusic {
	int flag;
	int offset;
	int volume;
	int num;
};

enum {
	kScreenWidth = 320,
	kScreenHeight = 200,
	kScreenPitch = 640,
	kFadePaletteStep = 5,
	kStartupLocationDemo = 9,
	kStartupLocationGame = 1,
	kDefaultCharSpeechSoundCounter = 1,
	kMaxSoundVolume = 127,
	kLastSaveSlot = 99
};

enum Verb {
	kVerbWalk  = 0,
	kVerbLook  = 1,
	kVerbTalk  = 2,
	kVerbOpen  = 3,
	kVerbClose = 4,
	kVerbGive  = 5,
	kVerbTake  = 6,
	kVerbMove  = 7,
	kVerbUse   = 8
};

enum InputKey {
	kInputKeyPause = 0,
	kInputKeyEscape,
	kInputKeyToggleInventory,
	kInputKeyToggleTextSpeech,
	kInputKeyHelp,
	kInputKeyCount
};

enum GameFlag {
	kGameFlagDemo = 1 << 0,
	kGameFlagEncodedData = 1 << 1,
	kGameFlagNoSubtitles = 1 << 2,
	kGameFlagIntroOnly = 1 << 3
};

enum CompressedSoundType {
	kSoundTypeFx,
	kSoundTypeMusic,
	kSoundTypeSpeech,
	kSoundTypeIntro
};

class CompressedSound {
public:

	CompressedSound() : _compressedSoundType(-1) {}

	void openFile();
	void closeFile();
	Audio::RewindableAudioStream *load(CompressedSoundType type, int num);

private:

	int _compressedSoundType;
	int _compressedSoundFlags;
	Common::File _fCompressedSound;
};

inline int scaleMixerVolume(int volume, int max = 100) {
	return volume * Audio::Mixer::kMaxChannelVolume / max;
}

Common::String generateGameStateFileName(const char *target, int slot, bool prefixOnly = false);

class AnimationSequencePlayer;

class TuckerEngine: public Engine {
public:

	enum {
		kLocationAnimationsTableSize = 20,
		kLocationObjectsTableSize = 10,
		kActionsTableSize = 100,
		kFlagsTableSize = 300,
		kLocationSoundsTableSize = 30,
		kLocationMusicsTableSize = 4,
		kSpriteFramesTableSize = 200,
		kSprA02TableSize = 20,
		kSprC02TableSize = 40,
		kDataTableSize = 500,
		kSpeechHistoryTableSize = 5,
		kMaxCharacters = 8,
		kMaxDirtyRects = 32
	};

	TuckerEngine(OSystem *system, Common::Language language, uint32 flags);
	virtual ~TuckerEngine();

	virtual Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;
	GUI::Debugger *getDebugger() { return _console; }

protected:

	int getRandomNumber();
	void allocateBuffers();
	void freeBuffers();
	void restart();
	void mainLoop();
	void waitForTimer(int ticksCount);
	void parseEvents();
	void updateCursorPos(int x, int y);
	void setCursorNum(int num);
	void setCursorType(int type);
	void setupNewLocation();
	void copyLocBitmap(const char *filename, int offset, bool isMask);
	void updateMouseState();
	void updateCharPositionHelper();
	void updateCharPosition();
	void updateFlagsForCharPosition();
	void fadeOutPalette(int colorsCount = 256);
	void fadeInPalette(int colorsCount = 256);
	void fadePaletteColor(int color, int step);
	void setBlackPalette();
	void updateCursor();
	void stopSounds();
	void playSounds();
	void updateCharactersPath();
	void setSoundVolumeDistance();
	void updateData3DrawFlag();
	void updateData3();
	void updateSfxData3_1();
	void updateSfxData3_2();
	void saveOrLoad();
	void handleMouseOnPanel();
	void switchPanelType();
	void redrawPanelOverBackground();
	void drawConversationTexts();
	void updateScreenScrolling();
	void updateGameHints();
	void startCharacterSounds();
	void updateSoundsTypes3_4();
	void drawData3();
	void execData3PreUpdate();
	void drawBackgroundSprites();
	void drawCurrentSprite();
	void setVolumeSound(int index, int volume);
	void setVolumeMusic(int index, int volume);
	void startSound(int offset, int index, int volume);
	void stopSound(int index);
	bool isSoundPlaying(int index);
	void startMusic(int offset, int index, int volume);
	void stopMusic(int index);
	void startSpeechSound(int num, int volume);
	void stopSpeechSound();
	bool isSpeechSoundPlaying();
	void rememberSpeechSound();
	void redrawPanelItems();
	void redrawPanelItemsHelper();
	void drawSprite(int i);
	void clearItemsGfx();
	void drawPausedInfoBar();
	const uint8 *getStringBuf(int type) const;
	void drawInfoString();
	void drawGameHintString();
	void updateCharacterAnimation();
	void execData3PostUpdate();
	void addObjectToInventory(int num);
	void removeObjectFromInventory(int num);
	void handleMap();
	void clearSprites();
	void updateSprites();
	void updateSprite(int i);
	void drawStringInteger(int num, int x, int y, int digits);
	void drawStringAlt(int x, int y, int color, const uint8 *str, int strLen = -1);
	void drawItemString(int x, int num, const uint8 *str);
	void drawCreditsString(int x, int y, int num);
	void updateCharSpeechSound(bool displayText);
	void updateItemsGfxColors(int bit0, int bit7);
	int testLocationMask(int x, int y);
	int getStringWidth(int num, const uint8 *ptr);
	int getPositionForLine(int num, const uint8 *ptr);
	void resetCharacterAnimationIndex(int count);
	int readTableInstructionCode(int *index);
	int readTableInstructionParam(int len);
	int executeTableInstruction();
	void moveUpInventoryObjects();
	void moveDownInventoryObjects();
	void setActionVerbUnderCursor();
	int getObjectUnderCursor();
	void setSelectedObjectKey();
	void setCharacterAnimation(int count, int spr);
	int testLocationMaskArea(int xBase, int yBase, int xPos, int yPos);
	void handleMouseClickOnInventoryObject();
	int setCharacterUnderCursor();
	int setLocationAnimationUnderCursor();
	void setActionForInventoryObject();
	void setActionState();
	void playSpeechForAction(int i);
	void drawSpeechText(int xStart, int y, const uint8 *dataPtr, int num, int color);
	int splitSpeechTextLines(const uint8 *dataPtr, int pos, int x, int &lineCharsCount, int &lineWidth);
	void drawSpeechTextLine(const uint8 *dataPtr, int pos, int count, int x, int y, uint8 color);
	void redrawScreen(int offset);
	void redrawScreenRect(const Common::Rect &clip, const Common::Rect &dirty);
	void addDirtyRect(int x, int y, int w, int h);

	void execData3PreUpdate_locationNum1();
	void execData3PreUpdate_locationNum1Helper1();
	void execData3PreUpdate_locationNum1Helper2();
	int execData3PreUpdate_locationNum1Helper3(int dx, int dy);
	void execData3PostUpdate_locationNum1();
	void updateSprite_locationNum2();
	void execData3PreUpdate_locationNum2();
	void execData3PreUpdate_locationNum2Helper();
	void updateSprite_locationNum3_0(int i);
	void updateSprite_locationNum3_1(int i);
	void updateSprite_locationNum3_2(int i);
	void execData3PreUpdate_locationNum3();
	void updateSprite_locationNum4(int i);
	void execData3PreUpdate_locationNum4();
	void updateSprite_locationNum5_0();
	void updateSprite_locationNum5_1(int i);
	void updateSprite_locationNum6_0(int i);
	void updateSprite_locationNum6_1(int i);
	void updateSprite_locationNum6_2(int i);
	void execData3PreUpdate_locationNum6();
	void execData3PreUpdate_locationNum6Helper1();
	void execData3PreUpdate_locationNum6Helper2(int dstOffset, const uint8 *src);
	void execData3PreUpdate_locationNum6Helper3(int dstOffset, const uint8 *src);
	void execData3PostUpdate_locationNum6();
	void updateSprite_locationNum7_0(int i);
	void updateSprite_locationNum7_1(int i);
	void updateSprite_locationNum8_0(int i);
	void updateSprite_locationNum8_1(int i);
	void execData3PostUpdate_locationNum8();
	void updateSprite_locationNum9_0(int i);
	void updateSprite_locationNum9_1(int i);
	void updateSprite_locationNum9_2(int i);
	void execData3PreUpdate_locationNum9();
	void execData3PostUpdate_locationNum9();
	void updateSprite_locationNum10();
	void execData3PreUpdate_locationNum10();
	void updateSprite_locationNum11_0(int i);
	void updateSprite_locationNum11_1(int i);
	void updateSprite_locationNum11_2(int i);
	void updateSprite_locationNum11_3(int i);
	void updateSprite_locationNum11_4(int i);
	void updateSprite_locationNum12_0(int i);
	void updateSprite_locationNum12_1(int i);
	void execData3PreUpdate_locationNum12();
	void updateSprite_locationNum13(int i);
	void execData3PreUpdate_locationNum13();
	void updateSprite_locationNum14(int i);
	void execData3PreUpdate_locationNum14();
	void execData3PreUpdate_locationNum14Helper1(int i);
	void execData3PreUpdate_locationNum14Helper2(int i);
	void execData3PostUpdate_locationNum14();
	void updateSprite_locationNum15_0(int i);
	void updateSprite_locationNum15_1(int i);
	void updateSprite_locationNum15_2(int i);
	void execData3PreUpdate_locationNum15();
	void updateSprite_locationNum16_0(int i);
	void updateSprite_locationNum16_1(int i);
	void updateSprite_locationNum16_2(int i);
	void execData3PreUpdate_locationNum16();
	void updateSprite_locationNum17();
	void updateSprite_locationNum18();
	void updateSprite_locationNum19_0(int i);
	void updateSprite_locationNum19_1(int i);
	void updateSprite_locationNum19_2(int i);
	void updateSprite_locationNum19_3(int i);
	void execData3PreUpdate_locationNum19();
	void updateSprite_locationNum21();
	void execData3PreUpdate_locationNum21();
	void execData3PostUpdate_locationNum21();
	void updateSprite_locationNum22();
	void execData3PreUpdate_locationNum22();
	void updateSprite_locationNum23_0(int i);
	void updateSprite_locationNum23_1(int i);
	void updateSprite_locationNum23_2(int i);
	void updateSprite_locationNum23_3(int i);
	void updateSprite_locationNum24_0(int i);
	void updateSprite_locationNum24_1(int i);
	void updateSprite_locationNum24_2(int i);
	void updateSprite_locationNum24_3(int i);
	void execData3PreUpdate_locationNum24();
	void execData3PostUpdate_locationNum24();
	void execData3PreUpdate_locationNum25();
	void updateSprite_locationNum26_0(int i);
	void updateSprite_locationNum26_1(int i);
	void execData3PreUpdate_locationNum26();
	void updateSprite_locationNum27(int i);
	void execData3PreUpdate_locationNum27();
	void execData3PostUpdate_locationNum27();
	void updateSprite_locationNum28_0(int i);
	void updateSprite_locationNum28_1(int i);
	void updateSprite_locationNum28_2(int i);
	void execData3PreUpdate_locationNum28();
	void execData3PostUpdate_locationNum28();
	void updateSprite_locationNum29_0(int i);
	void updateSprite_locationNum29_1(int i);
	void updateSprite_locationNum29_2(int i);
	void execData3PreUpdate_locationNum29();
	void updateSprite_locationNum30_34(int i);
	void execData3PreUpdate_locationNum30();
	void updateSprite_locationNum31_0(int i);
	void updateSprite_locationNum31_1(int i);
	void execData3PreUpdate_locationNum31();
	void updateSprite_locationNum32_0(int i);
	void execData3PreUpdate_locationNum32();
	void execData3PostUpdate_locationNum32();
	void updateSprite_locationNum33_0(int i);
	void updateSprite_locationNum33_1(int i);
	void updateSprite_locationNum33_2(int i);
	void execData3PreUpdate_locationNum33();
	void execData3PreUpdate_locationNum34();
	void execData3PreUpdate_locationNum35();
	void updateSprite_locationNum36(int i);
	void execData3PreUpdate_locationNum36();
	void updateSprite_locationNum37(int i);
	void execData3PreUpdate_locationNum38();
	void updateSprite_locationNum41(int i);
	void execData3PreUpdate_locationNum41();
	void updateSprite_locationNum42(int i);
	void execData3PreUpdate_locationNum42();
	void updateSprite_locationNum43_2(int i);
	void updateSprite_locationNum43_3(int i);
	void updateSprite_locationNum43_4(int i);
	void updateSprite_locationNum43_5(int i);
	void updateSprite_locationNum43_6(int i);
	void execData3PreUpdate_locationNum43();
	void execData3PreUpdate_locationNum44();
	void updateSprite_locationNum48(int i);
	void updateSprite_locationNum49(int i);
	void execData3PreUpdate_locationNum49();
	void updateSprite_locationNum50(int i);
	void updateSprite_locationNum51(int i);
	void execData3PreUpdate_locationNum52();
	void updateSprite_locationNum53_0(int i);
	void updateSprite_locationNum53_1(int i);
	void execData3PreUpdate_locationNum53();
	void updateSprite_locationNum54(int i);
	void updateSprite_locationNum55(int i);
	void updateSprite_locationNum56(int i);
	void updateSprite_locationNum57_0(int i);
	void updateSprite_locationNum57_1(int i);
	void execData3PreUpdate_locationNum57();
	void updateSprite_locationNum58(int i);
	void execData3PreUpdate_locationNum58();
	void updateSprite_locationNum59(int i);
	void updateSprite_locationNum60_0(int i);
	void updateSprite_locationNum60_1(int i);
	void execData3PostUpdate_locationNum60();
	void updateSprite_locationNum61_0(int i);
	void updateSprite_locationNum61_1(int i);
	void updateSprite_locationNum61_2(int i);
	void execData3PreUpdate_locationNum61();
	void updateSprite_locationNum63_0(int i);
	void updateSprite_locationNum63_1(int i);
	void updateSprite_locationNum63_2(int i);
	void updateSprite_locationNum63_3(int i);
	void updateSprite_locationNum63_4(int i);
	void execData3PreUpdate_locationNum63();
	void execData3PreUpdate_locationNum64();
	void updateSprite_locationNum65(int i);
	void execData3PreUpdate_locationNum65();
	void updateSprite_locationNum66_0(int i);
	void updateSprite_locationNum66_1(int i);
	void updateSprite_locationNum66_2(int i);
	void updateSprite_locationNum66_3(int i);
	void updateSprite_locationNum66_4(int i);
	void execData3PreUpdate_locationNum66();
	void execData3PostUpdate_locationNum66();
	void updateSprite_locationNum69_1(int i);
	void updateSprite_locationNum69_2(int i);
	void updateSprite_locationNum69_3(int i);
	void execData3PreUpdate_locationNum70();
	void execData3PreUpdate_locationNum70Helper();
	void updateSprite_locationNum71(int i);
	void updateSprite_locationNum72(int i);
	void updateSprite_locationNum74(int i);
	void updateSprite_locationNum79(int i);
	void updateSprite_locationNum81_0(int i);
	void updateSprite_locationNum81_1(int i);
	void updateSprite_locationNum82(int i);

	template<class S> void saveOrLoadGameStateData(S &s);
	virtual Common::Error loadGameState(int num);
	virtual Common::Error saveGameState(int num, const Common::String &description);
	virtual bool canLoadGameStateCurrently();
	virtual bool canSaveGameStateCurrently();

	TuckerConsole *_console;

	void handleIntroSequence();
	void handleCreditsSequence();
	void handleCongratulationsSequence();
	void handleNewPartSequence();
	void handleMeanwhileSequence();
	void handleMapSequence();
	void copyMapRect(int x, int y, int w, int h);
	int handleSpecialObjectSelectionSequence();

	uint8 *loadFile(const char *filename, uint8 *p);
	void loadImage(const char *filename, uint8 *dst, int a);
	void loadCursor();
	void loadCharset();
	void loadCharset2();
	void loadCharsetHelper();
	void loadCharSizeDta();
	void loadPanel();
	void loadBudSpr(int startOffset);
	int loadCTable01(int index, int firstSpriteNum, int *framesCount);
	void loadCTable02(int fl);
	void loadLoc();
	void loadObj();
	void loadData();
	int loadDataHelper(int offset, int index);
	void loadPanObj();
	void loadData3();
	void loadData4();
	void loadActionFile();
	void loadCharPos();
	void loadSprA02_01();
	void unloadSprA02_01();
	void loadSprC02_01();
	void unloadSprC02_01();
	void loadFx();
	void loadSound(Audio::Mixer::SoundType type, int num, int volume, bool loop, Audio::SoundHandle *handle);
	void loadActionsTable();

	Common::RandomSource _rnd;
	AnimationSequencePlayer *_player;
	CompressedSound _compressedSound;
	Common::Language _gameLang;
	uint32 _gameFlags;

	bool _quitGame;
	bool _fastMode;
	int _syncCounter;
	uint32 _lastFrameTime;
	int _mainLoopCounter1;
	int _mainLoopCounter2;
	int _timerCounter2;
	int _flagsTable[kFlagsTableSize];
	int _partNum;
	int _currentPartNum;
	int _locationNum;
	int _nextLocationNum;
	bool _gamePaused;
	bool _gameDebug;
	bool _displayGameHints;
	int _execData3Counter;
	bool _displaySpeechText;
	int _currentSaveLoadGameState;

	int _gameHintsIndex;
	int _gameHintsCounter;
	int _gameHintsDisplayText;
	int _gameHintsStringNum;

	int _fileLoadSize;
	uint8 *_loadTempBuf;
	uint8 *_cursorGfxBuf;
	uint8 *_charsetGfxBuf;
	uint8 *_panelGfxBuf;
	uint8 *_itemsGfxBuf;
	uint8 *_spritesGfxBuf;
	uint8 *_locationBackgroundGfxBuf;
	uint8 *_data5Buf;
	uint8 *_data3GfxBuf;
	uint8 *_quadBackgroundGfxBuf;
	uint8 *_objTxtBuf;
	uint8 *_panelObjectsGfxBuf;
	uint8 *_ptTextBuf;
	uint8 *_infoBarBuf;
	uint8 *_bgTextBuf;
	uint8 *_charNameBuf;
	uint8 *_locationBackgroundMaskBuf;
	uint8 *_csDataBuf;
	int _csDataSize;
	uint8 _charWidthTable[256];
	const uint8 *_locationWidthTable;
	uint8 *_locationHeightTable;

	int _mousePosX, _mousePosY;
	int _prevMousePosX, _prevMousePosY;
	int _mouseButtonsMask;
	int _mouseClick;
	int _saveOrLoadGamePanel;
	int _mouseIdleCounter;
	bool _leftMouseButtonPressed;
	bool _rightMouseButtonPressed;
	int _lastKeyPressed;
	bool _inputKeys[kInputKeyCount];
	int _cursorNum;
	int _cursorType;
	bool _updateCursorFlag;

	int _panelNum;
	int _panelState;
	bool _forceRedrawPanelItems;
	int _redrawPanelItemsCounter;
	int _switchPanelFlag;
	int _panelObjectsOffsetTable[50];
	int _switchPanelCounter;
	int _conversationOptionsCount;
	bool _fadedPanel;
	int _panelLockedFlag;
	int _conversationOptionLinesCount;
	int _inventoryItemsState[50];
	int _inventoryObjectsList[40];
	int _inventoryObjectsOffset;
	int _inventoryObjectsCount;
	int _lastInventoryObjectIndex;

	uint8 *_sprA02Table[kSprA02TableSize];
	uint8 *_sprC02Table[kSprC02TableSize];
	Action _actionsTable[kActionsTableSize];
	int _actionsCount;
	LocationObject _locationObjectsTable[kLocationObjectsTableSize];
	int _locationObjectsCount;
	Sprite _spritesTable[kMaxCharacters];
	int _spritesCount;
	LocationAnimation _locationAnimationsTable[kLocationAnimationsTableSize];
	int _locationAnimationsCount;
	Data _dataTable[kDataTableSize];
	int _dataCount;
	CharPos _charPosTable[kMaxCharacters];
	int _charPosCount;
	LocationSound _locationSoundsTable[kLocationSoundsTableSize];
	int _locationSoundsCount;
	LocationMusic _locationMusicsTable[kLocationMusicsTableSize];
	int _locationMusicsCount;

	int _currentFxSet;
	int _currentFxDist;
	int _currentFxScale;
	int _currentFxVolume;
	int _currentFxIndex;
	int _speechSoundNum;
	int _speechVolume;
	Audio::SoundHandle _sfxHandles[kLocationSoundsTableSize];
	Audio::SoundHandle _musicHandles[kLocationMusicsTableSize];
	Audio::SoundHandle _speechHandle;
	int _miscSoundFxNum[2];
	int _speechHistoryTable[kSpeechHistoryTableSize];
	int _charSpeechSoundCounter;
	int _miscSoundFxDelayCounter[2];
	int _characterSoundFxDelayCounter;
	int _characterSoundFxNum;
	int _speechSoundBaseNum;

	int _pendingActionIndex;
	int _pendingActionDelay;
	int _charPositionFlagNum;
	int _charPositionFlagValue;
	int _actionVerb;
	int _nextAction;
	int _selectedObjectNum;
	int _selectedObjectType;
	int _selectedCharacterNum;
	int _actionObj1Type, _actionObj2Type;
	int _actionObj1Num, _actionObj2Num;
	bool _actionRequiresTwoObjects;
	int _actionVerbLocked;
	int _actionPosX;
	int _actionPosY;
	int _selectedObjectLocationMask;
	struct {
		int xDefaultPos;
		int yDefaultPos;
		int xPos;
		int yPos;
		int locationObject_locationNum;
		int locationObject_toX;
		int locationObject_toY;
		int locationObject_toX2;
		int locationObject_toY2;
		int locationObject_toWalkX2;
		int locationObject_toWalkY2;
	} _selectedObject;
	int _selectedCharacterDirection;
	int _selectedCharacter2Num;
	int _currentActionObj1Num, _currentActionObj2Num;
	int _currentInfoString1SourceType, _currentInfoString2SourceType;
	int _speechActionCounterTable[9];
	int _actionCharacterNum;

	bool _csDataLoaded;
	bool _csDataHandled;
	bool _stopActionOnSoundFlag;
	bool _stopActionOnSpeechFlag;
	bool _stopActionOnPanelLock;
	int _csDataTableCount;
	int _stopActionCounter;
	int _actionTextColor;
	int _nextTableToLoadIndex;
	int _nextTableToLoadTable[6];
	int _soundInstructionIndex;
	const uint8 *_tableInstructionsPtr;
	int _tableInstructionObj1Table[6];
	int _tableInstructionObj2Table[6];
	int _tableInstructionFlag;
	int _tableInstructionItemNum1, _tableInstructionItemNum2;
	int _instructionsActionsTable[6];
	bool _validInstructionId;

	SpriteFrame _spriteFramesTable[kSpriteFramesTableSize];
	SpriteAnimation _spriteAnimationsTable[200];
	int _spriteAnimationFramesTable[500];
	int _spriteAnimationFrameIndex;
	int _backgroundSpriteCurrentFrame;
	int _backgroundSpriteLastFrame;
	int _backgroundSpriteCurrentAnimation;
	bool _disableCharactersPath;
	bool _skipCurrentCharacterDraw;
	int _xPosCurrent;
	int _yPosCurrent;
	const uint8 *_characterSpeechDataPtr;
	int _ptTextOffset;
	int _characterAnimationsTable[200];
	int _characterStateTable[200];
	int _backgroundSprOffset;
	int _currentActionVerb;
	int _previousActionVerb;
	int _mainSpritesBaseOffset;
	int _currentSpriteAnimationLength;
	int _currentSpriteAnimationFrame;
	int _currentSpriteAnimationFrame2;
	int _characterAnimationIndex;
	int _characterFacingDirection;
	int _characterPrevFacingDirection;
	int _characterBackFrontFacing;
	int _characterPrevBackFrontFacing;
	int _characterAnimationNum;
	int _noCharacterAnimationChange;
	int _changeBackgroundSprite;
	int _characterSpriteAnimationFrameCounter;
	int _locationMaskIgnore;
	int _locationMaskType;
	int _locationMaskCounter;
	int _updateSpriteFlag1;
	int _updateSpriteFlag2;
	int _handleMapCounter;
	bool _noPositionChangeAfterMap;

	int _mirroredDrawing;
	uint8 *_loadLocBufPtr;
	uint8 *_backgroundSpriteDataPtr;
	int _locationHeight;
	int _scrollOffset;
	int _currentGfxBackgroundCounter;
	uint8 *_currentGfxBackground;
	int _fadePaletteCounter;
	uint8 _currentPalette[768];
	bool _fullRedraw;
	int _dirtyRectsPrevCount, _dirtyRectsCount;
	Common::Rect _dirtyRectsTable[kMaxDirtyRects];

	int _updateLocationFadePaletteCounter;
	int _updateLocationCounter;
	int _updateLocationPos;
	int _updateLocationXPosTable[5], _updateLocationYPosTable[5];
	int _updateLocationFlagsTable[5];
	int _updateLocationXPosTable2[10], _updateLocationYPosTable2[10];
	int _updateLocationYMaxTable[3];
	int _updateLocation14Step[10];
	int _updateLocation14ObjNum[10];
	int _updateLocation14Delay[10];
	int _updateLocationCounter2;
	int _updateLocationFlag;
	int _updateLocation70StringLen;
	uint8 _updateLocation70String[20];

	static const uint8 _locationWidthTableGame[85];
	static const uint8 _locationWidthTableDemo[70];
	static const uint8 _sprA02LookupTable[88];
	static const uint8 _sprC02LookupTable[100];
	static const uint8 _sprC02LookupTable2[100];
	static const int _staticData3Table[1600];
	static uint8 _locationHeightTableGame[80];
	static uint8 _locationHeightTableDemo[70];
	static int _objectKeysPosXTable[80];
	static int _objectKeysPosYTable[80];
	static int _objectKeysLocationTable[80];
	static int _mapSequenceFlagsLocationTable[70];
	static const uint8 _charWidthCharset1[224];
	static const uint8 _charWidthCharset2[58];
};

enum {
	kFirstAnimationSequenceGame = 17,
	kFirstAnimationSequenceDemo = 13
};

enum AnimationSoundType {
	kAnimationSoundType8BitsRAW,
	kAnimationSoundType16BitsRAW,
	kAnimationSoundTypeWAV
};

enum {
	kSoundsList_Seq3_4,
	kSoundsList_Seq9_10,
	kSoundsList_Seq21_20,
	kSoundsList_Seq13_14,
	kSoundsList_Seq15_16,
	kSoundsList_Seq27_28,
	kSoundsList_Seq17_18,
	kSoundsList_Seq19_20
};

struct SoundSequenceData {
	int timestamp;
	int index;
	int num;
	int volume;
	int opcode;
};

struct SoundSequenceDataList {
	int musicIndex;
	int musicVolume;
	int soundList1Count;
	int soundList2Count;
	int soundSeqDataCount;
	const SoundSequenceData *soundSeqData;
};

class AnimationSequencePlayer {
public:

	enum {
		kSequenceFrameTime = 55
	};

	struct SequenceUpdateFunc {
		int num;
		int frameTime;
		void (AnimationSequencePlayer::*load)();
		void (AnimationSequencePlayer::*play)();
	};

	AnimationSequencePlayer(OSystem *system, Audio::Mixer *mixer, Common::EventManager *event, CompressedSound *sound, int num);
	~AnimationSequencePlayer();

	void mainLoop();

private:

	void syncTime();
	void loadSounds(int num);
	Audio::RewindableAudioStream *loadSound(int index, AnimationSoundType type);
	void updateSounds();
	void fadeInPalette();
	void fadeOutPalette();
	void unloadAnimation();
	uint8 *loadPicture(const char *fileName);
	void openAnimation(int index, const char *fileName);
	bool decodeNextAnimationFrame(int index, bool copyDirtyRects = true);
	void loadIntroSeq17_18();
	void playIntroSeq17_18();
	void loadIntroSeq19_20();
	void playIntroSeq19_20();
	void displayLoadingScreen();
	void initPicPart4();
	void drawPicPart4();
	void loadIntroSeq3_4();
	void playIntroSeq3_4();
	void drawPic2Part10();
	void drawPic1Part10();
	void loadIntroSeq9_10();
	void playIntroSeq9_10();
	void loadIntroSeq21_22();
	void playIntroSeq21_22();
	void loadIntroSeq13_14();
	void playIntroSeq13_14();
	void loadIntroSeq15_16();
	void playIntroSeq15_16();
	void loadIntroSeq27_28();
	void playIntroSeq27_28();
	void getRGBPalette(int index);

	OSystem *_system;
	Audio::Mixer *_mixer;
	Common::EventManager *_event;
	CompressedSound *_compressedSound;

	int _seqNum;
	bool _changeToNextSequence;
	const SequenceUpdateFunc *_updateFunc;
	int _updateFuncIndex;
	Video::FlicDecoder _flicPlayer[2];
	uint8 _animationPalette[256 * 3];
	int _soundSeqDataCount;
	int _soundSeqDataIndex;
	const SoundSequenceData *_soundSeqData;
	uint8 *_offscreenBuffer;
	int _updateScreenWidth;
	int _updateScreenPicture;
	int _updateScreenCounter;
	int _updateScreenIndex;
	int _frameCounter;
	int _frameTime;
	uint32 _lastFrameTime;
	uint8 *_picBufPtr, *_pic2BufPtr;
	Audio::SoundHandle _soundsHandle[15];
	Audio::SoundHandle _sfxHandle;
	Audio::SoundHandle _musicHandle;

	static const SoundSequenceDataList _soundSeqDataList[];
	static const char *const _audioFileNamesTable[];
};

} // namespace Tucker

#endif

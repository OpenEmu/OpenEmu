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

#ifndef KYRA_KYRA_LOK_H
#define KYRA_KYRA_LOK_H

#include "kyra/kyra_v1.h"
#include "kyra/script.h"
#include "kyra/screen_lok.h"
#include "kyra/gui_lok.h"
#include "kyra/item.h"

namespace Kyra {

class Movie;
class SoundDigital;
class SeqPlayer;
class Sprites;
class Animator_LoK;
class TextDisplayer;
class KyraEngine_LoK;

struct Character {
	uint16 sceneId;
	uint8 height;
	uint8 facing;
	uint16 currentAnimFrame;
	int8 inventoryItems[10];
	int16 x1, y1, x2, y2;
};

struct Shape {
	uint8 imageIndex;
	int8 xOffset, yOffset;
	uint8 x, y, w, h;
};

struct Room {
	uint8 nameIndex;
	uint16 northExit;
	uint16 eastExit;
	uint16 southExit;
	uint16 westExit;
	int8 itemsTable[12];
	uint16 itemsXPos[12];
	uint8 itemsYPos[12];
	uint8 needInit[12];
};

struct SeqLoop {
	const uint8 *ptr;
	uint16 count;
};

struct SceneExits {
	uint16 northXPos;
	uint8  northYPos;
	uint16 eastXPos;
	uint8  eastYPos;
	uint16 southXPos;
	uint8  southYPos;
	uint16 westXPos;
	uint8  westYPos;
};

struct BeadState {
	int16 x;
	int16 y;
	int16 width;
	int16 height;
	int16 dstX;
	int16 dstY;
	int16 width2;
	int16 unk8;
	int16 unk9;
	int16 tableIndex;
};

class KyraEngine_LoK : public KyraEngine_v1 {
	friend class MusicPlayer;
	friend class Debugger_LoK;
	friend class Animator_LoK;
	friend class GUI_LoK;
public:
	KyraEngine_LoK(OSystem *system, const GameFlags &flags);
	~KyraEngine_LoK();

	// _sprites and _seqplayer should be paused here too, to avoid some animation glitches,
	// also parts of the hardcoded Malcolm fight might need some special handling.

	Screen *screen() { return _screen; }
	Animator_LoK *animator() { return _animator; }
	GUI *gui() const { return _gui; }
	virtual Movie *createWSAMovie();

	uint8 **shapes() { return _shapes; }
	Character *currentCharacter() { return _currentCharacter; }
	Character *characterList() { return _characterList; }
	uint16 brandonStatus() { return _brandonStatusBit; }

	// TODO: remove me with workaround in animator.cpp l209
	uint16 getScene() { return _currentRoom; }

	int _paletteChanged;
	int16 _northExitHeight;

	typedef bool (KyraEngine_LoK::*IntroProc)();

	// static data access
	const char *const *seqWSATable() { return _seq_WSATable; }
	const char *const *seqCPSTable() { return _seq_CPSTable; }
	const char *const *seqCOLTable() { return _seq_COLTable; }
	const char *const *seqTextsTable() { return _seq_textsTable; }

	const uint8 *const *palTable1() { return &_specialPalettes[0]; }
	const uint8 *const *palTable2() { return &_specialPalettes[29]; }

protected:
	virtual Common::Error go();
	virtual Common::Error init();

public:
	// sequences
	// -> misc
	bool seq_skipSequence() const;
protected:
	// -> demo
	void seq_demo();

	// -> intro
	void seq_intro();
	bool seq_introPublisherLogos();
	bool seq_introLogos();
	bool seq_introStory();
	bool seq_introMalcolmTree();
	bool seq_introKallakWriting();
	bool seq_introKallakMalcolm();

	// -> ingame animations
	void seq_createAmuletJewel(int jewel, int page, int noSound, int drawOnly);
	void seq_brandonHealing();
	void seq_brandonHealing2();
	void seq_poisonDeathNow(int now);
	void seq_poisonDeathNowAnim();
	void seq_playFluteAnimation();
	void seq_winterScroll1();
	void seq_winterScroll2();
	void seq_makeBrandonInv();
	void seq_makeBrandonNormal();
	void seq_makeBrandonNormal2();
	void seq_makeBrandonWisp();
	void seq_dispelMagicAnimation();
	void seq_fillFlaskWithWater(int item, int type);
	void seq_playDrinkPotionAnim(int item, int unk2, int flags);
	void seq_brandonToStone();

	// -> end fight
	int seq_playEnd();
	void seq_playEnding();

	int handleMalcolmFlag();
	int handleBeadState();
	void initBeadState(int x, int y, int x2, int y2, int unk1, BeadState *ptr);
	int processBead(int x, int y, int &x2, int &y2, BeadState *ptr);

	// -> credits
	void seq_playCredits();
	void seq_playCreditsAmiga();

public:
	// delay
	void delayUntil(uint32 timestamp, bool updateGameTimers = false, bool update = false, bool isMainLoop = false);
	void delay(uint32 millis, bool update = false, bool isMainLoop = false);
	void delayWithTicks(int ticks);

	bool skipFlag() const;
	void resetSkipFlag(bool removeEvent = true);

	// TODO
	void registerDefaultSettings();
	void readSettings();
	void writeSettings();

	void snd_playSoundEffect(int track, int volume=0xFF);
	void snd_playWanderScoreViaMap(int command, int restart);
	virtual void snd_playVoiceFile(int id);
	void snd_voiceWaitForFinish(bool ingame = true);
	uint32 snd_getVoicePlayTime();

protected:
	int32 _speechPlayTime;

	Common::Error saveGameStateIntern(int slot, const char *saveName, const Graphics::Surface *thumbnail);
	Common::Error loadGameState(int slot);
protected:
	// input
	void processInput(int xpos, int ypos);
	int processInputHelper(int xpos, int ypos);
	int clickEventHandler(int xpos, int ypos);
	void clickEventHandler2();
	void updateMousePointer(bool forceUpdate = false);
	bool hasClickedOnExit(int xpos, int ypos);

	// scene
	// -> init
	void loadSceneMsc();
	void startSceneScript(int brandonAlive);
	void setupSceneItems();
	void initSceneData(int facing, int unk1, int brandonAlive);
	void initSceneObjectList(int brandonAlive);
	void initSceneScreen(int brandonAlive);
	void setupSceneResource(int sceneId);

	// -> process
	void enterNewScene(int sceneId, int facing, int unk1, int unk2, int brandonAlive);
	int handleSceneChange(int xpos, int ypos, int unk1, int frameReset);
	int processSceneChange(int *table, int unk1, int frameReset);
	int changeScene(int facing);

	// -> modification
	void transcendScenes(int roomIndex, int roomName);
	void setSceneFile(int roomIndex, int roomName);

	// -> pathfinder
	int findWay(int x, int y, int toX, int toY, int *moveTable, int moveTableSize);
	bool lineIsPassable(int x, int y);

	// -> item handling
	// --> misc
	void addItemToRoom(uint16 sceneId, uint8 item, int itemIndex, int x, int y);

	// --> drop handling
	void itemDropDown(int x, int y, int destX, int destY, byte freeItem, int item);
	int processItemDrop(uint16 sceneId, uint8 item, int x, int y, int unk1, int unk2);
	void dropItem(int unk1, int item, int x, int y, int unk2);

	// --> dropped item handling
	int countItemsInScene(uint16 sceneId);
	void exchangeItemWithMouseItem(uint16 sceneId, int itemIndex);
	byte findFreeItemInScene(int scene);
	byte findItemAtPos(int x, int y);

	// --> drop area handling
	void addToNoDropRects(int x, int y, int w, int h);
	void clearNoDropRects();
	int isDropable(int x, int y);
	int checkNoDropRects(int x, int y);

	// --> player items handling
	void updatePlayerItemsForScene();

	// --> item GFX handling
	void backUpItemRect0(int xpos, int ypos);
	void restoreItemRect0(int xpos, int ypos);
	void backUpItemRect1(int xpos, int ypos);
	void restoreItemRect1(int xpos, int ypos);

	// items
	// -> misc
	void placeItemInGenericMapScene(int item, int index);

	// -> mouse item
	void setHandItem(Item item);
	void removeHandItem();
	void setMouseItem(Item item);

	int getItemListIndex(Item item);

	// -> graphics effects
	void wipeDownMouseItem(int xpos, int ypos);
	void itemSpecialFX(int x, int y, int item);
	void itemSpecialFX1(int x, int y, int item);
	void itemSpecialFX2(int x, int y, int item);
	void magicOutMouseItem(int animIndex, int itemPos);
	void magicInMouseItem(int animIndex, int item, int itemPos);
	void specialMouseItemFX(int shape, int x, int y, int animIndex, int tableIndex, int loopStart, int maxLoops);
	void processSpecialMouseItemFX(int shape, int x, int y, int tableValue, int loopStart, int maxLoops);

	// character
	// -> movement
	void moveCharacterToPos(int character, int facing, int xpos, int ypos);
	void setCharacterPositionWithUpdate(int character);
	int setCharacterPosition(int character, int *facingTable);
	void setCharacterPositionHelper(int character, int *facingTable);
	void setCharactersPositions(int character);

	// -> brandon
	void setBrandonPoisonFlags(int reset);
	void resetBrandonPoisonFlags();

	// chat
	// -> process
	void characterSays(int vocFile, const char *chatStr, int8 charNum, int8 chatDuration);
	void waitForChatToFinish(int vocFile, int16 chatDuration, const char *str, uint8 charNum, const bool printText);

	// -> initialization
	int initCharacterChat(int8 charNum);
	void backupChatPartnerAnimFrame(int8 charNum);
	void restoreChatPartnerAnimFrame(int8 charNum);
	int8 getChatPartnerNum();

	// -> deinitialization
	void endCharacterChat(int8 charNum, int16 arg_4);

	// graphics
	// -> misc
	int findDuplicateItemShape(int shape);
	void updateKyragemFading();

	// -> interface
	void loadMainScreen(int page = 3);
	void redrawInventory(int page);
public:
	void drawSentenceCommand(const char *sentence, int unk1);
	void updateSentenceCommand(const char *str1, const char *str2, int unk1);
	void updateTextFade();

protected:
	// -> amulet
	void drawJewelPress(int jewel, int drawSpecial);
	void drawJewelsFadeOutStart();
	void drawJewelsFadeOutEnd(int jewel);

	// -> shape handling
	void setupShapes123(const Shape *shapeTable, int endShape, int flags);
	void freeShapes123();

	// misc (TODO)
	void startup();
	void mainLoop();

	int checkForNPCScriptRun(int xpos, int ypos);
	void runNpcScript(int func);

	void loadMouseShapes();
	void loadCharacterShapes();
	void loadSpecialEffectShapes();
	void loadItems();
	void loadButtonShapes();
	void initMainButtonList();
	void setCharactersInDefaultScene();
	void setupPanPages();
	void freePanPages();
	void closeFinalWsa();

	//void setTimer19();
	void setupTimers();
	void timerUpdateHeadAnims(int timerNum);
	void timerTulipCreator(int timerNum);
	void timerRubyCreator(int timerNum);
	void timerAsInvisibleTimeout(int timerNum);
	void timerAsWillowispTimeout(int timerNum);
	void checkAmuletAnimFlags();
	void timerRedrawAmulet(int timerNum);
	void timerLavenderRoseCreator(int timerNum);
	void timerAcornCreator(int timerNum);
	void timerBlueberryCreator(int timerNum);
	void timerFadeText(int timerNum);
	void timerWillowispFrameTimer(int timerNum);
	void timerInvisibleFrameTimer(int timerNum);
	void drawAmulet();
	void setTextFadeTimerCountdown(int16 countdown);
	void setWalkspeed(uint8 newSpeed);

	void setItemCreationFlags(int offset, int count);

	int buttonInventoryCallback(Button *caller);
	int buttonAmuletCallback(Button *caller);

	bool _seqPlayerFlag;
	bool _skipIntroFlag;
	bool _abortIntroFlag;

	bool _menuDirectlyToLoad;
	uint8 *_itemBkgBackUp[2];
	uint8 *_shapes[373];
	Item _itemInHand;
	bool _changedScene;
	int _unkScreenVar1, _unkScreenVar2, _unkScreenVar3;
	int _beadStateVar;
	int _unkAmuletVar;

	int _malcolmFlag;
	int _endSequenceSkipFlag;
	int _endSequenceNeedLoading;
	int _unkEndSeqVar2;
	uint8 *_endSequenceBackUpRect;
	int _unkEndSeqVar4;
	int _unkEndSeqVar5;
	int _lastDisplayedPanPage;
	uint8 *_panPagesTable[20];
	Movie *_finalA, *_finalB, *_finalC;

	Movie *_movieObjects[10];

	uint16 _entranceMouseCursorTracks[5];
	uint16 _walkBlockNorth;
	uint16 _walkBlockEast;
	uint16 _walkBlockSouth;
	uint16 _walkBlockWest;

	int32 _scaleMode;
	int16 _scaleTable[145];

	Common::Rect _noDropRects[11];

	int8 _birthstoneGemTable[4];
	int8 _idolGemsTable[3];

	int8 _marbleVaseItem;
	int8 _foyerItemTable[3];

	int8 _cauldronState;
	int8 _crystalState[2];

	uint16 _brandonStatusBit;
	uint8 _brandonStatusBit0x02Flag;
	uint8 _brandonStatusBit0x20Flag;
	uint8 _brandonPoisonFlagsGFX[256];
	int16 _brandonInvFlag;
	uint8 _poisonDeathCounter;
	int _brandonPosX;
	int _brandonPosY;

	uint16 _currentChatPartnerBackupFrame;
	uint16 _currentCharAnimFrame;

	int _characterFacingZeroCount[8];
	int _characterFacingFourCount[8];

	int8 *_sceneAnimTable[50];

	uint8 _itemHtDat[145];
	int _lastProcessedItem;
	int _lastProcessedItemHeight;

	int16 *_exitListPtr;
	int16 _exitList[11];
	SceneExits _sceneExits;
	uint16 _currentRoom;
	int _scenePhasingFlag;

	int _sceneChangeState;
	int _loopFlag2;

	int _pathfinderFlag;
	int _pathfinderFlag2;
	int _lastFindWayRet;
	int *_movFacingTable;

	int8 _talkingCharNum;
	int8 _charSayUnk2;
	int8 _charSayUnk3;
	int8 _currHeadShape;
	int _currentHeadFrameTableIndex;
	int8 _disabledTalkAnimObject;
	int8 _enabledTalkAnimObject;
	uint8 _currSentenceColor[3];
	int8 _startSentencePalIndex;
	bool _fadeText;

	uint8 _configTextspeed;

	Animator_LoK *_animator;
	SeqPlayer *_seq;
	Sprites *_sprites;
	Screen_LoK *_screen;

	EMCState _scriptMain;

	EMCState _npcScript;
	EMCData _npcScriptData;

	EMCState _scriptClick;
	EMCData _scriptClickData;

	Character *_characterList;
	Character *_currentCharacter;

	Button *_buttonList;
	GUI_LoK *_gui;

	uint16 _malcolmFrame;
	uint32 _malcolmTimer1;
	uint32 _malcolmTimer2;

	uint32 _beadStateTimer1;
	uint32 _beadStateTimer2;
	BeadState _beadState1;
	BeadState _beadState2;

	struct KyragemState {
		uint16 nextOperation;
		uint16 rOffset;
		uint16 gOffset;
		uint16 bOffset;
		uint32 timerCount;
	} _kyragemFadingState;

	static const int8 _dosTrackMap[];
	static const int _dosTrackMapSize;

	static const int8 _amigaTrackMap[];
	static const int _amigaTrackMapSize;

	// TODO: get rid of all variables having pointers to the static resources if possible
	// i.e. let them directly use the _staticres functions
	void initStaticResource();

	const uint8 *_seq_Forest;
	const uint8 *_seq_KallakWriting;
	const uint8 *_seq_KyrandiaLogo;
	const uint8 *_seq_KallakMalcolm;
	const uint8 *_seq_MalcolmTree;
	const uint8 *_seq_WestwoodLogo;
	const uint8 *_seq_Demo1;
	const uint8 *_seq_Demo2;
	const uint8 *_seq_Demo3;
	const uint8 *_seq_Demo4;
	const uint8 *_seq_Reunion;

	const char *const *_seq_WSATable;
	const char *const *_seq_CPSTable;
	const char *const *_seq_COLTable;
	const char *const *_seq_textsTable;

	const char *const *_storyStrings;

	int _seq_WSATable_Size;
	int _seq_CPSTable_Size;
	int _seq_COLTable_Size;
	int _seq_textsTable_Size;

	int _storyStringsSize;

	const char *const *_itemList;
	const char *const *_takenList;
	const char *const *_placedList;
	const char *const *_droppedList;
	const char *const *_noDropList;
	const char *const *_putDownFirst;
	const char *const *_waitForAmulet;
	const char *const *_blackJewel;
	const char *const *_poisonGone;
	const char *const *_healingTip;
	const char *const *_thePoison;
	const char *const *_fluteString;
	const char *const *_wispJewelStrings;
	const char *const *_magicJewelString;
	const char *const *_flaskFull;
	const char *const *_fullFlask;
	const char *const *_veryClever;
	const char *const *_homeString;
	const char *const *_newGameString;

	int _itemList_Size;
	int _takenList_Size;
	int _placedList_Size;
	int _droppedList_Size;
	int _noDropList_Size;
	int _putDownFirst_Size;
	int _waitForAmulet_Size;
	int _blackJewel_Size;
	int _poisonGone_Size;
	int _healingTip_Size;
	int _thePoison_Size;
	int _fluteString_Size;
	int _wispJewelStrings_Size;
	int _magicJewelString_Size;
	int _flaskFull_Size;
	int _fullFlask_Size;
	int _veryClever_Size;
	int _homeString_Size;
	int _newGameString_Size;

	const char *const *_characterImageTable;
	int _characterImageTableSize;

	const char *const *_guiStrings;
	int _guiStringsSize;

	const char *const *_configStrings;
	int _configStringsSize;

	Shape *_defaultShapeTable;
	int _defaultShapeTableSize;

	const Shape *_healingShapeTable;
	int  _healingShapeTableSize;
	const Shape *_healingShape2Table;
	int  _healingShape2TableSize;

	const Shape *_posionDeathShapeTable;
	int _posionDeathShapeTableSize;

	const Shape *_fluteAnimShapeTable;
	int _fluteAnimShapeTableSize;

	const Shape *_winterScrollTable;
	int _winterScrollTableSize;
	const Shape *_winterScroll1Table;
	int _winterScroll1TableSize;
	const Shape *_winterScroll2Table;
	int _winterScroll2TableSize;

	const Shape *_drinkAnimationTable;
	int _drinkAnimationTableSize;

	const Shape *_brandonToWispTable;
	int _brandonToWispTableSize;

	const Shape *_magicAnimationTable;
	int _magicAnimationTableSize;

	const Shape *_brandonStoneTable;
	int _brandonStoneTableSize;

	Room *_roomTable;
	int _roomTableSize;
	const char *const *_roomFilenameTable;
	int _roomFilenameTableSize;

	const uint8 *_amuleteAnim;

	const uint8 *const *_specialPalettes;

	// positions of the inventory
	static const uint16 _itemPosX[];
	static const uint8 _itemPosY[];

	void setupButtonData();
	Button *_buttonData;
	Button **_buttonDataListPtr;

	static const uint8 _magicMouseItemStartFrame[];
	static const uint8 _magicMouseItemEndFrame[];
	static const uint8 _magicMouseItemStartFrame2[];
	static const uint8 _magicMouseItemEndFrame2[];

	static const uint16 _amuletX[];
	static const uint16 _amuletY[];
	static const uint16 _amuletX2[];
	static const uint16 _amuletY2[];

	// special palette handling for AMIGA
	void setupZanthiaPalette(int pal);
protected:
	void setupOpcodeTable();

	// Opcodes
	int o1_magicInMouseItem(EMCState *script);
	int o1_characterSays(EMCState *script);
	int o1_delay(EMCState *script);
	int o1_drawSceneAnimShape(EMCState *script);
	int o1_runNPCScript(EMCState *script);
	int o1_setSpecialExitList(EMCState *script);
	int o1_walkPlayerToPoint(EMCState *script);
	int o1_dropItemInScene(EMCState *script);
	int o1_drawAnimShapeIntoScene(EMCState *script);
	int o1_savePageToDisk(EMCState *script);
	int o1_sceneAnimOn(EMCState *script);
	int o1_sceneAnimOff(EMCState *script);
	int o1_getElapsedSeconds(EMCState *script);
	int o1_mouseIsPointer(EMCState *script);
	int o1_runSceneAnimUntilDone(EMCState *script);
	int o1_fadeSpecialPalette(EMCState *script);
	int o1_phaseInSameScene(EMCState *script);
	int o1_setScenePhasingFlag(EMCState *script);
	int o1_resetScenePhasingFlag(EMCState *script);
	int o1_queryScenePhasingFlag(EMCState *script);
	int o1_sceneToDirection(EMCState *script);
	int o1_setBirthstoneGem(EMCState *script);
	int o1_placeItemInGenericMapScene(EMCState *script);
	int o1_setBrandonStatusBit(EMCState *script);
	int o1_delaySecs(EMCState *script);
	int o1_getCharacterScene(EMCState *script);
	int o1_runNPCSubscript(EMCState *script);
	int o1_magicOutMouseItem(EMCState *script);
	int o1_internalAnimOn(EMCState *script);
	int o1_forceBrandonToNormal(EMCState *script);
	int o1_poisonDeathNow(EMCState *script);
	int o1_setScaleMode(EMCState *script);
	int o1_openWSAFile(EMCState *script);
	int o1_closeWSAFile(EMCState *script);
	int o1_runWSAFromBeginningToEnd(EMCState *script);
	int o1_displayWSAFrame(EMCState *script);
	int o1_enterNewScene(EMCState *script);
	int o1_setSpecialEnterXAndY(EMCState *script);
	int o1_runWSAFrames(EMCState *script);
	int o1_popBrandonIntoScene(EMCState *script);
	int o1_restoreAllObjectBackgrounds(EMCState *script);
	int o1_setCustomPaletteRange(EMCState *script);
	int o1_loadPageFromDisk(EMCState *script);
	int o1_customPrintTalkString(EMCState *script);
	int o1_restoreCustomPrintBackground(EMCState *script);
	int o1_getCharacterX(EMCState *script);
	int o1_getCharacterY(EMCState *script);
	int o1_setCharacterFacing(EMCState *script);
	int o1_copyWSARegion(EMCState *script);
	int o1_printText(EMCState *script);
	int o1_loadSoundFile(EMCState *script);
	int o1_displayWSAFrameOnHidPage(EMCState *script);
	int o1_displayWSASequentialFrames(EMCState *script);
	int o1_refreshCharacter(EMCState *script);
	int o1_internalAnimOff(EMCState *script);
	int o1_changeCharactersXAndY(EMCState *script);
	int o1_clearSceneAnimatorBeacon(EMCState *script);
	int o1_querySceneAnimatorBeacon(EMCState *script);
	int o1_refreshSceneAnimator(EMCState *script);
	int o1_placeItemInOffScene(EMCState *script);
	int o1_wipeDownMouseItem(EMCState *script);
	int o1_placeCharacterInOtherScene(EMCState *script);
	int o1_getKey(EMCState *script);
	int o1_specificItemInInventory(EMCState *script);
	int o1_popMobileNPCIntoScene(EMCState *script);
	int o1_mobileCharacterInScene(EMCState *script);
	int o1_hideMobileCharacter(EMCState *script);
	int o1_unhideMobileCharacter(EMCState *script);
	int o1_setCharacterLocation(EMCState *script);
	int o1_walkCharacterToPoint(EMCState *script);
	int o1_specialEventDisplayBrynnsNote(EMCState *script);
	int o1_specialEventRemoveBrynnsNote(EMCState *script);
	int o1_setLogicPage(EMCState *script);
	int o1_fatPrint(EMCState *script);
	int o1_preserveAllObjectBackgrounds(EMCState *script);
	int o1_updateSceneAnimations(EMCState *script);
	int o1_sceneAnimationActive(EMCState *script);
	int o1_setCharacterMovementDelay(EMCState *script);
	int o1_getCharacterFacing(EMCState *script);
	int o1_bkgdScrollSceneAndMasksRight(EMCState *script);
	int o1_dispelMagicAnimation(EMCState *script);
	int o1_findBrightestFireberry(EMCState *script);
	int o1_setFireberryGlowPalette(EMCState *script);
	int o1_setDeathHandlerFlag(EMCState *script);
	int o1_drinkPotionAnimation(EMCState *script);
	int o1_makeAmuletAppear(EMCState *script);
	int o1_drawItemShapeIntoScene(EMCState *script);
	int o1_setCharacterCurrentFrame(EMCState *script);
	int o1_waitForConfirmationMouseClick(EMCState *script);
	int o1_pageFlip(EMCState *script);
	int o1_setSceneFile(EMCState *script);
	int o1_getItemInMarbleVase(EMCState *script);
	int o1_setItemInMarbleVase(EMCState *script);
	int o1_addItemToInventory(EMCState *script);
	int o1_intPrint(EMCState *script);
	int o1_shakeScreen(EMCState *script);
	int o1_createAmuletJewel(EMCState *script);
	int o1_setSceneAnimCurrXY(EMCState *script);
	int o1_poisonBrandonAndRemaps(EMCState *script);
	int o1_fillFlaskWithWater(EMCState *script);
	int o1_getCharacterMovementDelay(EMCState *script);
	int o1_getBirthstoneGem(EMCState *script);
	int o1_queryBrandonStatusBit(EMCState *script);
	int o1_playFluteAnimation(EMCState *script);
	int o1_playWinterScrollSequence(EMCState *script);
	int o1_getIdolGem(EMCState *script);
	int o1_setIdolGem(EMCState *script);
	int o1_totalItemsInScene(EMCState *script);
	int o1_restoreBrandonsMovementDelay(EMCState *script);
	int o1_setEntranceMouseCursorTrack(EMCState *script);
	int o1_itemAppearsOnGround(EMCState *script);
	int o1_setNoDrawShapesFlag(EMCState *script);
	int o1_fadeEntirePalette(EMCState *script);
	int o1_itemOnGroundHere(EMCState *script);
	int o1_queryCauldronState(EMCState *script);
	int o1_setCauldronState(EMCState *script);
	int o1_queryCrystalState(EMCState *script);
	int o1_setCrystalState(EMCState *script);
	int o1_setPaletteRange(EMCState *script);
	int o1_shrinkBrandonDown(EMCState *script);
	int o1_growBrandonUp(EMCState *script);
	int o1_setBrandonScaleXAndY(EMCState *script);
	int o1_resetScaleMode(EMCState *script);
	int o1_getScaleDepthTableValue(EMCState *script);
	int o1_setScaleDepthTableValue(EMCState *script);
	int o1_message(EMCState *script);
	int o1_checkClickOnNPC(EMCState *script);
	int o1_getFoyerItem(EMCState *script);
	int o1_setFoyerItem(EMCState *script);
	int o1_setNoItemDropRegion(EMCState *script);
	int o1_walkMalcolmOn(EMCState *script);
	int o1_passiveProtection(EMCState *script);
	int o1_setPlayingLoop(EMCState *script);
	int o1_brandonToStoneSequence(EMCState *script);
	int o1_brandonHealingSequence(EMCState *script);
	int o1_protectCommandLine(EMCState *script);
	int o1_pauseMusicSeconds(EMCState *script);
	int o1_resetMaskRegion(EMCState *script);
	int o1_setPaletteChangeFlag(EMCState *script);
	int o1_vocUnload(EMCState *script);
	int o1_vocLoad(EMCState *script);
	int o1_dummy(EMCState *script);
};

} // End of namespace Kyra

#endif

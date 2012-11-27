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

#ifndef KYRA_KYRA_HOF_H
#define KYRA_KYRA_HOF_H

#include "kyra/kyra_v2.h"
#include "kyra/script.h"
#include "kyra/script_tim.h"
#include "kyra/screen_hof.h"
#include "kyra/text_hof.h"
#include "kyra/gui_hof.h"

#include "common/list.h"
#include "common/func.h"

namespace Kyra {

//class WSAMovie_v2;
//class KyraEngine_HoF;
class TextDisplayer_HoF;
class SeqPlayer_HOF;

struct TIM;

class KyraEngine_HoF : public KyraEngine_v2 {
friend class Debugger_HoF;
friend class TextDisplayer_HoF;
friend class GUI_HoF;
public:
	KyraEngine_HoF(OSystem *system, const GameFlags &flags);
	~KyraEngine_HoF();

	void pauseEngineIntern(bool pause);

	Screen *screen() { return _screen; }
	Screen_v2 *screen_v2() const { return _screen; }
	GUI *gui() const { return _gui; }
	virtual TextDisplayer *text() { return _text; }
	int language() const { return _lang; }

protected:
	static const EngineDesc _hofEngineDesc;

	// intro/outro
	void seq_showStarcraftLogo();

	int seq_playIntro();
	int seq_playOutro();
	int seq_playDemo();

	void seq_pausePlayer(bool toggle);

	Common::Error init();
	Common::Error go();

	Screen_HoF *_screen;
	TextDisplayer_HoF *_text;
	TIMInterpreter *_tim;

	static const int8 _dosTrackMap[];
	static const int _dosTrackMapSize;
	static const int8 _mt32SfxMap[];
	static const int _mt32SfxMapSize;
	static const int8 _gmSfxMap[];
	static const int _gmSfxMapSize;
	static const int8 _pcSpkSfxMap[];
	static const int _pcSpkSfxMapSize;

protected:
	// game initialization
	void startup();
	void runLoop();
	void cleanup();

	void registerDefaultSettings();
	void writeSettings();
	void readSettings();
	uint8 _configTextspeed;

	// TODO: get rid of all variables having pointers to the static resources if possible
	// i.e. let them directly use the _staticres functions
	void initStaticResource();

	void setupTimers();
	void setupOpcodeTable();

	void loadMouseShapes();
	void loadItemShapes();

	// run
	void update();
	void updateWithText();

	Common::Functor0Mem<void, KyraEngine_HoF> _updateFunctor;

	void updateMouse();

	void dinoRide();

	void handleInput(int x, int y);
	bool handleInputUnkSub(int x, int y);

	int inputSceneChange(int x, int y, int unk1, int unk2);

	// gfx/animation specific
	bool _inventorySaved;
	void backUpPage0();
	void restorePage0();

	uint8 *_gfxBackUpRect;

	void backUpGfxRect24x24(int x, int y);
	void restoreGfxRect24x24(int x, int y);
	void backUpGfxRect32x32(int x, int y);
	void restoreGfxRect32x32(int x, int y);

	uint8 *_sceneShapeTable[50];

	WSAMovie_v2 *_wsaSlots[10];

	void freeSceneShapePtrs();

	struct ShapeDesc {
		uint8 unk0, unk1, unk2, unk3, unk4;
		uint16 width, height;
		int16 xAdd, yAdd;
	};

	ShapeDesc *_shapeDescTable;

	void loadCharacterShapes(int shapes);
	void loadInventoryShapes();

	void resetScaleTable();
	void setScaleTableItem(int item, int data);
	int getScale(int x, int y);
	uint16 _scaleTable[15];

	void setDrawLayerTableEntry(int entry, int data);
	int getDrawLayer(int x, int y);
	int _drawLayerTable[15];

	int _layerFlagTable[16]; // seems to indicate layers where items get destroyed when dropped to (TODO: check this!)

	int initAnimationShapes(uint8 *filedata);
	void uninitAnimationShapes(int count, uint8 *filedata);

	// animator
	uint8 *_gamePlayBuffer;
	void restorePage3();

	void clearAnimObjects();

	void refreshAnimObjects(int force);

	void drawAnimObjects();
	void drawSceneAnimObject(AnimObj *obj, int x, int y, int drawLayer);
	void drawCharacterAnimObject(AnimObj *obj, int x, int y, int drawLayer);

	void updateItemAnimations();

	void updateCharFacing();
	void updateCharacterAnim(int);
	void updateSceneAnim(int anim, int newFrame);

	int _animObj0Width, _animObj0Height;
	void setCharacterAnimDim(int w, int h);
	void resetCharacterAnimDim();

	// scene
	const char *_sceneCommentString;
	uint8 _scenePal[688];

	void enterNewScene(uint16 newScene, int facing, int unk1, int unk2, int unk3);
	void enterNewSceneUnk1(int facing, int unk1, int unk2);
	void enterNewSceneUnk2(int unk1);
	void unloadScene();

	void loadScenePal();
	void loadSceneMsc();

	void fadeScenePal(int srcIndex, int delay);

	void startSceneScript(int unk1);
	void runSceneScript2();
	void runSceneScript4(int unk1);
	void runSceneScript7();

	void initSceneAnims(int unk1);
	void initSceneScreen(int unk1);

	int trySceneChange(int *moveTable, int unk1, int updateChar);
	int checkSceneChange();

	// pathfinder
	bool lineIsPassable(int x, int y);

	// item
	void setMouseCursor(Item item);

	uint8 _itemHtDat[176];

	int checkItemCollision(int x, int y);
	void updateWaterFlasks();

	bool dropItem(int unk1, Item item, int x, int y, int unk2);
	bool processItemDrop(uint16 sceneId, Item item, int x, int y, int unk1, int unk2);
	void itemDropDown(int startX, int startY, int dstX, int dstY, int itemSlot, Item item);
	void exchangeMouseItem(int itemPos);
	bool pickUpItem(int x, int y);

	bool isDropable(int x, int y);

	static const byte _itemStringMap[];
	static const int _itemStringMapSize;

	static const Item _flaskTable[];
	bool itemIsFlask(Item item);

	// inventory
	static const int _inventoryX[];
	static const int _inventoryY[];
	static const uint16 _itemMagicTable[];

	int getInventoryItemSlot(Item item);
	void removeSlotFromInventory(int slot);
	bool checkInventoryItemExchange(Item item, int slot);
	void drawInventoryShape(int page, Item item, int slot);
	void clearInventorySlot(int slot, int page);
	void redrawInventory(int page);
	void scrollInventoryWheel();
	int findFreeVisibleInventorySlot();

	ActiveItemAnim _activeItemAnim[15];
	int _nextAnimItem;

	// gui
	bool _menuDirectlyToLoad;
	GUI_HoF *_gui;

	void loadButtonShapes();
	void setupLangButtonShapes();
	uint8 *_buttonShapes[19];

	void initInventoryButtonList();
	Button *_inventoryButtons;
	Button *_buttonList;

	int scrollInventory(Button *button);
	int buttonInventory(Button *button);
	int bookButton(Button *button);
	int cauldronButton(Button *button);
	int cauldronClearButton(Button *button);

	// book
	static const int _bookPageYOffset[];
	static const byte _bookTextColorMap[];

	int _bookMaxPage;
	int _bookNewPage;
	int _bookCurPage;
	int _bookBkgd;
	bool _bookShown;

	void loadBookBkgd();
	void showBookPage();
	void bookLoop();

	void bookDecodeText(uint8 *text);
	void bookPrintText(int dstPage, const uint8 *text, int x, int y, uint8 color);

	int bookPrevPage(Button *button);
	int bookNextPage(Button *button);
	int bookClose(Button *button);

	// cauldron
	uint8 _cauldronState;
	int16 _cauldronUseCount;
	int16 _cauldronTable[25];
	int16 _cauldronStateTables[23][7];

	static const int16 _cauldronProtectedItems[];
	static const int16 _cauldronBowlTable[];
	static const int16 _cauldronMagicTable[];
	static const int16 _cauldronMagicTableScene77[];
	static const uint8 _cauldronStateTable[];

	void resetCauldronStateTable(int idx);
	bool addToCauldronStateTable(int data, int idx);

	void setCauldronState(uint8 state, bool paletteFade);
	void clearCauldronTable();
	void addFrontCauldronTable(int item);
	void cauldronItemAnim(int item);
	void cauldronRndPaletteFade();
	bool updateCauldron();
	void listItemsInCauldron();

	// localization
	void loadCCodeBuffer(const char *file);
	void loadOptionsBuffer(const char *file);
	void loadChapterBuffer(int chapter);
	uint8 *_optionsBuffer;
	uint8 *_cCodeBuffer;

	uint8 *_chapterBuffer;
	int _currentChapter;
	int _newChapterFile;

	uint8 *getTableEntry(uint8 *buffer, int id);
	char *getTableString(int id, uint8 *buffer, int decode);
	const char *getChapterString(int id);

	void changeFileExtension(char *buffer);

	// - Just used in French version
	int getItemCommandStringDrop(Item item);
	int getItemCommandStringPickUp(Item item);
	int getItemCommandStringInv(Item item);
	// -

	char _internStringBuf[200];
	static const char *const _languageExtension[];
	static const char *const _scriptLangExt[];

	// character
	bool _useCharPal;
	bool _setCharPalFinal;
	int _charPalEntry;
	uint8 _charPalTable[16];
	void updateCharPal(int unk1);
	void setCharPalEntry(int entry, int value);

	int _characterFacingCountTable[2];

	int getCharacterWalkspeed() const;
	void updateCharAnimFrame(int *table);

	bool checkCharCollision(int x, int y);

	static const uint8 _characterFrameTable[];

	// text
	void showMessageFromCCode(int id, int16 palIndex, int);
	void showMessage(const char *string, int16 palIndex);
	void showChapterMessage(int id, int16 palIndex);

	void updateCommandLineEx(int str1, int str2, int16 palIndex);

	const char *_shownMessage;

	byte _messagePal[3];
	bool _fadeMessagePalette;
	void fadeMessagePalette();

	// chat
	bool _chatIsNote;

	int chatGetType(const char *text);
	int chatCalcDuration(const char *text);

	void objectChat(const char *text, int object, int vocHigh = -1, int vocLow = -1);
	void objectChatInit(const char *text, int object, int vocHigh = -1, int vocLow = -1);
	void objectChatPrintText(const char *text, int object);
	void objectChatProcess(const char *script);
	void objectChatWaitToFinish();

	void startDialogue(int dlgIndex);

	void zanthSceneStartupChat();
	void randomSceneChat();
	void updateDlgBuffer();
	void loadDlgHeader(int &csEntry, int &vocH, int &scIndex1, int &scIndex2);
	void processDialogue(int dlgOffset, int vocH = 0, int csEntry = 0);
	void npcChatSequence(const char *str, int objectId, int vocHigh = -1, int vocLow = -1);
	void setDlgIndex(int dlgIndex);

	int _npcTalkChpIndex;
	int _npcTalkDlgIndex;
	uint8 _newSceneDlgState[32];
	int8 **_conversationState;
	uint8 *_dlgBuffer;

	// Talk object handling
	void initTalkObject(int index);
	void deinitTalkObject(int index);

	struct TalkObject {
		char filename[13];
		int8 scriptId;
		int16 x, y;
		int8 color;
	};
	TalkObject *_talkObjectList;

	struct TalkSections {
		TIM *STATim;
		TIM *TLKTim;
		TIM *ENDTim;
	};
	TalkSections _currentTalkSections;

	char _TLKFilename[13];

	// tim
	void playTim(const char *filename);

	int t2_initChat(const TIM *tim, const uint16 *param);
	int t2_updateSceneAnim(const TIM *tim, const uint16 *param);
	int t2_resetChat(const TIM *tim, const uint16 *param);
	int t2_playSoundEffect(const TIM *tim, const uint16 *param);

	Common::Array<const TIMOpcode *> _timOpcodes;

	// sound
	int _oldTalkFile;
	int _currentTalkFile;
	void openTalkFile(int newFile);
	int _lastSfxTrack;

	virtual void snd_playVoiceFile(int id);
	void snd_loadSoundFile(int id);

	void playVoice(int high, int low);
	void snd_playSoundEffect(int track, int volume=0xFF);

	// timer
	void timerFadeOutMessage(int);
	void timerCauldronAnimation(int);
	void timerFunc4(int);
	void timerFunc5(int);
	void timerBurnZanthia(int);

	void setTimer1DelaySecs(int secs);

	uint32 _nextIdleAnim;
	int _lastIdleScript;
	bool _useSceneIdleAnim;

	void setNextIdleAnimTimer();
	void showIdleAnim();
	void runIdleScript(int script);

	void setWalkspeed(uint8 speed);

	// ingame static sequence handling
	void seq_makeBookOrCauldronAppear(int type);
	void seq_makeBookAppear();

	struct InventoryWsa {
		int x, y, x2, y2, w, h;
		int page;
		int curFrame, lastFrame, specialFrame;
		int sfx;
		int delay;
		bool running;
		uint32 timer;
		WSAMovie_v2 *wsa;
	} _invWsa;

	// TODO: move inside KyraEngine_HoF::InventoryWsa?
	void loadInvWsa(const char *filename, int run, int delay, int page, int sfx, int sFrame, int flags);
	void closeInvWsa();

	void updateInvWsa();
	void displayInvWsaLastFrame();

	// opcodes
	int o2_setCharacterFacingRefresh(EMCState *script);
	int o2_setCharacterPos(EMCState *script);
	int o2_defineObject(EMCState *script);
	int o2_refreshCharacter(EMCState *script);
	int o2_setSceneComment(EMCState *script);
	int o2_setCharacterAnimFrame(EMCState *script);
	int o2_setCharacterFacing(EMCState *script);
	int o2_customCharacterChat(EMCState *script);
	int o2_soundFadeOut(EMCState *script);
	int o2_showChapterMessage(EMCState *script);
	int o2_restoreTalkTextMessageBkgd(EMCState *script);
	int o2_wsaClose(EMCState *script);
	int o2_meanWhileScene(EMCState *script);
	int o2_backUpScreen(EMCState *script);
	int o2_restoreScreen(EMCState *script);
	int o2_displayWsaFrame(EMCState *script);
	int o2_displayWsaSequentialFramesLooping(EMCState *script);
	int o2_wsaOpen(EMCState *script);
	int o2_displayWsaSequentialFrames(EMCState *script);
	int o2_displayWsaSequence(EMCState *script);
	int o2_addItemToInventory(EMCState *script);
	int o2_drawShape(EMCState *script);
	int o2_addItemToCurScene(EMCState *script);
	int o2_loadSoundFile(EMCState *script);
	int o2_removeSlotFromInventory(EMCState *script);
	int o2_removeItemFromInventory(EMCState *script);
	int o2_countItemInInventory(EMCState *script);
	int o2_countItemsInScene(EMCState *script);
	int o2_wipeDownMouseItem(EMCState *script);
	int o2_getElapsedSecs(EMCState *script);
	int o2_getTimerDelay(EMCState *script);
	int o2_delaySecs(EMCState *script);
	int o2_setTimerDelay(EMCState *script);
	int o2_setScaleTableItem(EMCState *script);
	int o2_setDrawLayerTableItem(EMCState *script);
	int o2_setCharPalEntry(EMCState *script);
	int o2_loadZShapes(EMCState *script);
	int o2_drawSceneShape(EMCState *script);
	int o2_drawSceneShapeOnPage(EMCState *script);
	int o2_disableAnimObject(EMCState *script);
	int o2_enableAnimObject(EMCState *script);
	int o2_loadPalette384(EMCState *script);
	int o2_setPalette384(EMCState *script);
	int o2_restoreBackBuffer(EMCState *script);
	int o2_backUpInventoryGfx(EMCState *script);
	int o2_disableSceneAnim(EMCState *script);
	int o2_enableSceneAnim(EMCState *script);
	int o2_restoreInventoryGfx(EMCState *script);
	int o2_setSceneAnimPos2(EMCState *script);
	int o2_fadeScenePal(EMCState *script);
	int o2_enterNewScene(EMCState *script);
	int o2_switchScene(EMCState *script);
	int o2_setPathfinderFlag(EMCState *script);
	int o2_getSceneExitToFacing(EMCState *script);
	int o2_setLayerFlag(EMCState *script);
	int o2_setZanthiaPos(EMCState *script);
	int o2_loadMusicTrack(EMCState *script);
	int o2_setSceneAnimPos(EMCState *script);
	int o2_setCauldronState(EMCState *script);
	int o2_showItemString(EMCState *script);
	int o2_isAnySoundPlaying(EMCState *script);
	int o2_setDrawNoShapeFlag(EMCState *script);
	int o2_setRunFlag(EMCState *script);
	int o2_showLetter(EMCState *script);
	int o2_playFireflyScore(EMCState *script);
	int o2_encodeShape(EMCState *script);
	int o2_defineSceneAnim(EMCState *script);
	int o2_updateSceneAnim(EMCState *script);
	int o2_addToSceneAnimPosAndUpdate(EMCState *script);
	int o2_useItemOnMainChar(EMCState *script);
	int o2_startDialogue(EMCState *script);
	int o2_addCauldronStateTableEntry(EMCState *script);
	int o2_setCountDown(EMCState *script);
	int o2_getCountDown(EMCState *script);
	int o2_pressColorKey(EMCState *script);
	int o2_objectChat(EMCState *script);
	int o2_changeChapter(EMCState *script);
	int o2_getColorCodeFlag1(EMCState *script);
	int o2_setColorCodeFlag1(EMCState *script);
	int o2_getColorCodeFlag2(EMCState *script);
	int o2_setColorCodeFlag2(EMCState *script);
	int o2_getColorCodeValue(EMCState *script);
	int o2_setColorCodeValue(EMCState *script);
	int o2_countItemInstances(EMCState *script);
	int o2_removeItemFromScene(EMCState *script);
	int o2_initObject(EMCState *script);
	int o2_npcChat(EMCState *script);
	int o2_deinitObject(EMCState *script);
	int o2_playTimSequence(EMCState *script);
	int o2_makeBookOrCauldronAppear(EMCState *script);
	int o2_resetInputColorCode(EMCState *script);
	int o2_mushroomEffect(EMCState *script);
	int o2_customChat(EMCState *script);
	int o2_customChatFinish(EMCState *script);
	int o2_setupSceneAnimation(EMCState *script);
	int o2_stopSceneAnimation(EMCState *script);
	int o2_processPaletteIndex(EMCState *script);
	int o2_updateTwoSceneAnims(EMCState *script);
	int o2_getRainbowRoomData(EMCState *script);
	int o2_drawSceneShapeEx(EMCState *script);
	int o2_midiSoundFadeout(EMCState *script);
	int o2_getSfxDriver(EMCState *script);
	int o2_getVocSupport(EMCState *script);
	int o2_getMusicDriver(EMCState *script);
	int o2_zanthiaChat(EMCState *script);
	int o2_isVoiceEnabled(EMCState *script);
	int o2_isVoicePlaying(EMCState *script);
	int o2_stopVoicePlaying(EMCState *script);
	int o2_getGameLanguage(EMCState *script);
	int o2_demoFinale(EMCState *script);
	int o2_dummy(EMCState *script);

	// animation opcodes
	int o2a_setCharacterFrame(EMCState *script);

	// script
	void runStartScript(int script, int unk1);
	void loadNPCScript();

	bool _noScriptEnter;

	EMCData _npcScriptData;

	// pathfinder
	uint8 *_unkBuf500Bytes;
	uint8 *_unkBuf200kByte;
	bool _chatAltFlag;

	// sequence player
/*	ActiveWSA *_activeWSA;
	ActiveText *_activeText;
	*/
	/*const char *const *_sequencePakList;
	int _sequencePakListSize;*/
	const char *const *_ingamePakList;
	int _ingamePakListSize;

	const char *const *_musicFileListIntro;
	int _musicFileListIntroSize;
	const char *const *_musicFileListFinale;
	int _musicFileListFinaleSize;
	const char *const *_musicFileListIngame;
	int _musicFileListIngameSize;
	const uint8 *_cdaTrackTableIntro;
	int _cdaTrackTableIntroSize;
	const uint8 *_cdaTrackTableIngame;
	int _cdaTrackTableIngameSize;
	const uint8 *_cdaTrackTableFinale;
	int _cdaTrackTableFinaleSize;
	const char *const *_ingameSoundList;
	int _ingameSoundListSize;
	const uint16 *_ingameSoundIndex;
	int _ingameSoundIndexSize;
	const uint16 *_ingameTalkObjIndex;
	int _ingameTalkObjIndexSize;
	const char *const *_ingameTimJpStr;
	int _ingameTimJpStrSize;

	const ItemAnimDefinition *_itemAnimDefinition;
	int _itemAnimDefinitionSize;

	/*const HofSeqData *_sequences;

	const ItemAnimData_v1 *_demoAnimData;
	int _demoAnimSize;

	int _sequenceStringsDuration[33];*/

/*	static const uint8 _seqTextColorPresets[];
	char *_seqProcessedString;
	WSAMovie_v2 *_seqWsa;

	bool _abortIntroFlag;
	int _menuChoice;*/

	/*uint32 _seqFrameDelay;
	uint32 _seqStartTime;
	uint32 _seqSubFrameStartTime;
	uint32 _seqEndTime;
	uint32 _seqSubFrameEndTimeInternal;
	uint32 _seqWsaChatTimeout;
	uint32 _seqWsaChatFrameTimeout;

	int _seqFrameCounter;
	int _seqScrollTextCounter;
	int _seqWsaCurrentFrame;
	bool _seqSpecialFlag;
	bool _seqSubframePlaying;
	uint8 _seqTextColor[2];
	uint8 _seqTextColorMap[16];*/

	static const uint8 _rainbowRoomData[];

	// color code related vars
	int _colorCodeFlag1;
	int _colorCodeFlag2;
	uint8 _presetColorCode[7];
	uint8 _inputColorCode[7];
	uint32 _scriptCountDown;
	int _dbgPass;

	// save/load specific
	Common::Error saveGameStateIntern(int slot, const char *saveName, const Graphics::Surface *thumbnail);
	Common::Error loadGameState(int slot);
};

} // End of namespace Kyra

#endif

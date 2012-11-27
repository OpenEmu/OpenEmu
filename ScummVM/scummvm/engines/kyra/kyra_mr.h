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

#ifndef KYRA_KYRA_MR_H
#define KYRA_KYRA_MR_H

#include "kyra/kyra_v2.h"
#include "kyra/screen_mr.h"
#include "kyra/script.h"
#include "kyra/gui_mr.h"

#include "common/hashmap.h"
#include "common/list.h"

namespace Kyra {

class SoundDigital;
class Screen_MR;
class MainMenu;
class WSAMovie_v2;
class TextDisplayer_MR;
struct Button;

class KyraEngine_MR : public KyraEngine_v2 {
friend class TextDisplayer_MR;
friend class GUI_MR;
public:
	KyraEngine_MR(OSystem *system, const GameFlags &flags);
	~KyraEngine_MR();

	// Regarding pausing of the engine:
	// Idle animation time, item animations and album animations should be taken
	// care of, but since those would just produce minor glitches it's not that
	// important.

	Screen *screen() { return _screen; }
	Screen_v2 *screen_v2() const { return _screen; }
	GUI *gui() const { return _gui; }
	SoundDigital *soundDigital() { return _soundDigital; }
	int language() const { return _lang; }
	bool heliumMode() const { return _configHelium; }

	Common::Error go();

	void playVQA(const char *name);

private:
	static const EngineDesc _mrEngineDesc;

	// config
	bool _configStudio;
	bool _configSkip;
	bool _configHelium;
	int _configVQAQuality;

	void registerDefaultSettings();
	void writeSettings();
	void readSettings();

	void initStaticResource();

	// --
	Screen_MR *_screen;
	SoundDigital *_soundDigital;

	Common::Error init();

	void preinit();
	void startup();
	void runStartupScript(int script, int unk1);

	void setupOpcodeTable();

	// input
	bool skipFlag() const;
	void resetSkipFlag(bool removeEvent = true);

	// run
	bool _menuDirectlyToLoad;

	void runLoop();
	void handleInput(int x, int y);
	int inputSceneChange(int x, int y, int unk1, int unk2);

	void update();
	void updateWithText();
	void updateMouse();

	// sound specific
private:
	void playMenuAudioFile();

	int _musicSoundChannel;
	int _fadeOutMusicChannel;
	const char *_menuAudioFile;

	const char *const *_soundList;
	int _soundListSize;

	void snd_playWanderScoreViaMap(int track, int force);
	void stopMusicTrack();

	void fadeOutMusic(int ticks);

	void snd_playSoundEffect(int item, int volume);

	const uint8 *_sfxFileMap;
	int _sfxFileMapSize;
	const char *const *_sfxFileList;
	int _sfxFileListSize;

	int _voiceSoundChannel;

	void playVoice(int high, int low);
	void snd_playVoiceFile(int file);
	bool snd_voiceIsPlaying();
	void snd_stopVoice();

	int _curStudioSFX;
	void playStudioSFX(const char *str);

	// gui
	GUI_MR *_gui;

	Button *_mainButtonData;
	Button *_mainButtonList;
	bool _mainButtonListInitialized;
	void initMainButtonList(bool disable);

	bool _enableInventory;
	int buttonInventory(Button *button);
	int buttonMoodChange(Button *button);
	int buttonShowScore(Button *button);
	int buttonJesterStaff(Button *button);

	void loadButtonShapes();
	int callbackButton1(Button *button);
	int callbackButton2(Button *button);
	int callbackButton3(Button *button);

	// -> main menu
	void initMainMenu();
	void uninitMainMenu();

	MainMenu *_menu;
	WSAMovie_v2 *_menuAnim;

	// timer
	void setupTimers();

	void setWalkspeed(uint8);
	void setCommandLineRestoreTimer(int secs);

	void timerRestoreCommandLine(int arg);
	void timerRunSceneScript7(int arg);
	void timerFleaDeath(int arg);

	uint32 _nextIdleAnim;
	void setNextIdleAnimTimer();

	// pathfinder
	bool lineIsPassable(int x, int y);

private:
	// main menu
	const char *const *_mainMenuStrings;
	int _mainMenuStringsSize;

	static const char *const _mainMenuSpanishFan[];
	static const char *const _mainMenuItalianFan[];

	// animator
	uint8 *_gamePlayBuffer;
	void restorePage3();

	void clearAnimObjects();

	void animSetupPaletteEntry(AnimObj *anim);

	void drawAnimObjects();
	void drawSceneAnimObject(AnimObj *obj, int x, int y, int drawLayer);
	void drawCharacterAnimObject(AnimObj *obj, int x, int y, int drawLayer);

	void refreshAnimObjects(int force);

	bool _loadingState;
	void updateItemAnimations();
	void updateCharacterAnim(int charId);

	void updateSceneAnim(int anim, int newFrame);
	void setupSceneAnimObject(int anim, uint16 flags, int x, int y, int x2, int y2, int w, int h, int unk10, int specialSize, int unk14, int shape, const char *filename);
	void removeSceneAnimObject(int anim, int refresh);

	int _charBackUpWidth2, _charBackUpHeight2;
	int _charBackUpWidth, _charBackUpHeight;

	void setCharacterAnimDim(int w, int h);
	void resetCharacterAnimDim();

	bool _nextIdleType;
	void showIdleAnim();

	const ItemAnimDefinition *_itemAnimDefinition;
	ActiveItemAnim _activeItemAnim[10];
	int _nextAnimItem;

	// interface
	uint8 *_interface;
	uint8 *_interfaceCommandLine;

	void loadInterfaceShapes();
	void loadInterface();

	void showMessage(const char *string, uint8 c0, uint8 c1);
	void showMessageFromCCode(int string, uint8 c0, int);
	void updateItemCommand(Item item, int str, uint8 c0);

	void updateCommandLine();
	void restoreCommandLine();
	void updateCLState();

	int _commandLineY;
	const char *_shownMessage;
	bool _restoreCommandLine;
	bool _inventoryState;
	int _inventoryScrollSpeed;

	void showInventory();
	void hideInventory();

	void drawMalcolmsMoodText();
	void drawMalcolmsMoodPointer(int frame, int page);
	void drawJestersStaff(int type, int page);

	void drawScore(int page, int x, int y);
	void drawScoreCounting(int oldScore, int newScore, int drawOld, const int x);
	int getScoreX(const char *str);

	static const uint8 _inventoryX[];
	static const uint8 _inventoryY[];
	void redrawInventory(int page);
	void clearInventorySlot(int slot, int page);
	void drawInventorySlot(int page, Item item, int slot);

	WSAMovie_v2 *_invWsa;
	int _invWsaFrame;

	// localization
	uint8 *_scoreFile;
	uint8 *_cCodeFile;
	uint8 *_scenesFile;
	uint8 *_itemFile;
	uint8 *_optionsFile;
	uint8 *_actorFile;
	uint32 _actorFileSize;
	uint8 *_sceneStrings;

	uint8 *getTableEntry(uint8 *buffer, int id);
	void getTableEntry(Common::SeekableReadStream *stream, int id, char *dst);

	// items
	int8 *_itemBuffer1;
	int8 *_itemBuffer2;

	static const Item _trashItemList[];
	void removeTrashItems();

	void initItems();

	int checkItemCollision(int x, int y);

	bool dropItem(int unk1, Item item, int x, int y, int unk2);
	bool processItemDrop(uint16 sceneId, Item item, int x, int y, int unk1, int unk2);
	void itemDropDown(int startX, int startY, int dstX, int dstY, int itemSlot, Item item, int remove);
	void exchangeMouseItem(int itemPos, int runScript);
	bool pickUpItem(int x, int y, int runScript);

	bool isDropable(int x, int y);

	const uint8 *_itemMagicTable;
	bool itemListMagic(Item handItem, int itemSlot);
	bool itemInventoryMagic(Item handItem, int invSlot);

	const uint8 *_itemStringMap;
	int _itemStringMapSize;
	static const uint8 _itemStringPickUp[];
	static const uint8 _itemStringDrop[];
	static const uint8 _itemStringInv[];

	int getItemCommandStringPickUp(uint16 item);
	int getItemCommandStringDrop(uint16 item);
	int getItemCommandStringInv(uint16 item);

	// -> hand item
	void setItemMouseCursor();
	void setMouseCursor(Item item);

	// shapes
	void initMouseShapes();

	void loadCharacterShapes(int newShapes);
	void updateMalcolmShapes();

	int _malcolmShapeXOffset, _malcolmShapeYOffset;

	struct ShapeDesc {
		uint8 width, height;
		int8 xOffset, yOffset;
	};
	static const ShapeDesc _shapeDescs[];
	static const int _shapeDescsSize;

	// scene animation
	uint8 *_sceneShapes[20];

	void freeSceneShapes();

	// voice
	int _currentTalkFile;
	void openTalkFile(int file);

	// scene
	bool _noScriptEnter;
	void enterNewScene(uint16 scene, int facing, int unk1, int unk2, int unk3);
	void enterNewSceneUnk1(int facing, int unk1, int unk2);
	void enterNewSceneUnk2(int unk1);
	int _enterNewSceneLock;

	void unloadScene();

	void loadScenePal();
	void loadSceneMsc();
	void initSceneScript(int unk1);
	void initSceneAnims(int unk1);
	void initSceneScreen(int unk1);

	int runSceneScript1(int x, int y);
	int runSceneScript2();
	bool _noStartupChat;
	void runSceneScript4(int unk1);
	void runSceneScript8();

	int _sceneMinX, _sceneMaxX;
	int _maskPageMinY, _maskPageMaxY;

	int trySceneChange(int *moveTable, int unk1, int unk2);
	int checkSceneChange();

	int8 _sceneDatPalette[45];
	int8 _sceneDatLayerTable[15];
	struct SceneShapeDesc {
		// the original saves those variables, we don't, since
		// they are just needed on scene load
		/*int x, y;
		int w, h;*/
		int drawX, drawY;
	};
	SceneShapeDesc _sceneShapeDescs[20];

	int getDrawLayer(int x, int y);

	int getScale(int x, int y);
	int _scaleTable[15];

	// character
	int getCharacterWalkspeed() const;
	void updateCharAnimFrame(int *table);
	int8 _characterAnimTable[2];
	static const uint8 _characterFrameTable[];

	void updateCharPal(int unk1);
	int _lastCharPalLayer;
	bool _charPalUpdate;

	bool checkCharCollision(int x, int y);

	int _malcolmsMood;

	void makeCharFacingMouse();

	int findFreeInventorySlot();

	// talk object
	struct TalkObject {
		char filename[13];
		int8 sceneAnim;
		int8 sceneScript;
		int16 x, y;
		uint8 color;
		uint8 sceneId;
	};

	TalkObject *_talkObjectList;

	bool talkObjectsInCurScene();

	// chat
	int chatGetType(const char *text);
	int chatCalcDuration(const char *text);

	void objectChat(const char *text, int object, int vocHigh, int vocLow);
	void objectChatInit(const char *text, int object, int vocHigh, int vocLow);
	void objectChatPrintText(const char *text, int object);
	void objectChatProcess(const char *script);
	void objectChatWaitToFinish();

	void badConscienceChat(const char *str, int vocHigh, int vocLow);
	void badConscienceChatWaitToFinish();

	void goodConscienceChat(const char *str, int vocHigh, int vocLow);
	void goodConscienceChatWaitToFinish();

	bool _albumChatActive;
	void albumChat(const char *str, int vocHigh, int vocLow);
	void albumChatInit(const char *str, int object, int vocHigh, int vocLow);
	void albumChatWaitToFinish();

	void malcolmSceneStartupChat();

	byte _newSceneDlgState[40];
	int8 _conversationState[30][30];
	bool _chatAltFlag;
	void setDlgIndex(int index);
	void updateDlgIndex();

	Common::SeekableReadStream *_cnvFile;
	Common::SeekableReadStream *_dlgBuffer;
	int _curDlgChapter, _curDlgIndex, _curDlgLang;
	void updateDlgBuffer();
	void loadDlgHeader(int &vocHighBase, int &vocHighIndex, int &index1, int &index2);

	static const uint8 _vocHighTable[];
	bool _isStartupDialog;
	void processDialog(int vocHighIndex, int vocHighBase, int funcNum);

	EMCData _dialogScriptData;
	EMCState _dialogScriptState;
	int _dialogSceneAnim;
	int _dialogSceneScript;
	int _dialogScriptFuncStart, _dialogScriptFuncProc, _dialogScriptFuncEnd;

	void dialogStartScript(int object, int funcNum);
	void dialogEndScript(int object);

	void npcChatSequence(const char *str, int object, int vocHigh, int vocLow);

	Common::Array<const Opcode *> _opcodesDialog;

	int o3d_updateAnim(EMCState *script);
	int o3d_delay(EMCState *script);

	void randomSceneChat();
	void doDialog(int dlgIndex, int funcNum);

	// conscience
	bool _badConscienceShown;
	int _badConscienceAnim;
	bool _badConsciencePosition;

	static const uint8 _badConscienceFrameTable[];

	void showBadConscience();
	void hideBadConscience();

	bool _goodConscienceShown;
	int _goodConscienceAnim;
	bool _goodConsciencePosition;

	static const uint8 _goodConscienceFrameTable[];

	void showGoodConscience();
	void hideGoodConscience();

	// special script code
	bool _useFrameTable;

	int o3a_setCharacterFrame(EMCState *script);
	int o3a_playSoundEffect(EMCState *script);

	// special shape code
	int initAnimationShapes(uint8 *filedata);
	void uninitAnimationShapes(int count, uint8 *filedata);

	// unk
	uint8 *_costPalBuffer;
	uint8 *_paletteOverlay;
	bool _useActorBuffer;

	int _currentChapter;
	void changeChapter(int newChapter, int sceneId, int malcolmShapes, int facing);

	static const uint8 _chapterLowestScene[];

	void loadCostPal();
	void loadShadowShape();
	void loadExtrasShapes();

	uint8 *_gfxBackUpRect;
	void backUpGfxRect32x32(int x, int y);
	void restoreGfxRect32x32(int x, int y);

	char *_stringBuffer;

	int _score;
	int _scoreMax;

	const uint8 *_scoreTable;
	int _scoreTableSize;

	int8 _scoreFlagTable[26];
	bool updateScore(int scoreId, int strId);
	void scoreIncrease(int count, const char *str);

	void eelScript();

	// Album
	struct Album {
		uint8 *backUpPage;
		uint8 *file;
		WSAMovie_v2 *wsa;
		uint8 *backUpRect;

		struct PageMovie {
			WSAMovie_v2 *wsa;
			int curFrame;
			int maxFrame;
			uint32 timer;
		};

		PageMovie leftPage, rightPage;

		int curPage, nextPage;
		bool running;
		bool isPage14;
	} _album;

	static const int8 _albumWSAX[];
	static const int8 _albumWSAY[];

	void showAlbum();

	void loadAlbumPage();
	void loadAlbumPageWSA();

	void printAlbumPageText();
	void printAlbumText(int page, const char *str, int x, int y, uint8 c0);

	void processAlbum();

	void albumNewPage();
	void albumUpdateAnims();
	void albumAnim1();
	void albumAnim2();

	void albumBackUpRect();
	void albumRestoreRect();
	void albumUpdateRect();

	void albumSwitchPages(int oldPage, int newPage, int srcPage);

	int albumNextPage(Button *caller);
	int albumPrevPage(Button *caller);
	int albumClose(Button *caller);

	// save/load
	Common::Error saveGameStateIntern(int slot, const char *saveName, const Graphics::Surface *thumbnail);
	Common::Error loadGameState(int slot);

	// opcodes
	int o3_getMalcolmShapes(EMCState *script);
	int o3_setCharacterPos(EMCState *script);
	int o3_defineObject(EMCState *script);
	int o3_refreshCharacter(EMCState *script);
	int o3_getMalcolmsMood(EMCState *script);
	int o3_getCharacterFrameFromFacing(EMCState *script);
	int o3_setCharacterFacing(EMCState *script);
	int o3_showSceneFileMessage(EMCState *script);
	int o3_setCharacterAnimFrameFromFacing(EMCState *script);
	int o3_showBadConscience(EMCState *script);
	int o3_hideBadConscience(EMCState *script);
	int o3_showAlbum(EMCState *script);
	int o3_setInventorySlot(EMCState *script);
	int o3_getInventorySlot(EMCState *script);
	int o3_addItemToInventory(EMCState *script);
	int o3_addItemToCurScene(EMCState *script);
	int o3_objectChat(EMCState *script);
	int o3_resetInventory(EMCState *script);
	int o3_removeInventoryItemInstances(EMCState *script);
	int o3_countInventoryItemInstances(EMCState *script);
	int o3_npcChatSequence(EMCState *script);
	int o3_badConscienceChat(EMCState *script);
	int o3_wipeDownMouseItem(EMCState *script);
	int o3_setMalcolmsMood(EMCState *script);
	int o3_updateScore(EMCState *script);
	int o3_makeSecondChanceSave(EMCState *script);
	int o3_setSceneFilename(EMCState *script);
	int o3_removeItemsFromScene(EMCState *script);
	int o3_disguiseMalcolm(EMCState *script);
	int o3_drawSceneShape(EMCState *script);
	int o3_drawSceneShapeOnPage(EMCState *script);
	int o3_checkInRect(EMCState *script);
	int o3_updateConversations(EMCState *script);
	int o3_removeItemSlot(EMCState *script);
	int o3_setSceneDim(EMCState *script);
	int o3_setSceneAnimPosAndFrame(EMCState *script);
	int o3_removeItemInstances(EMCState *script);
	int o3_disableInventory(EMCState *script);
	int o3_enableInventory(EMCState *script);
	int o3_enterNewScene(EMCState *script);
	int o3_switchScene(EMCState *script);
	int o3_setMalcolmPos(EMCState *script);
	int o3_stopMusic(EMCState *script);
	int o3_playSoundEffect(EMCState *script);
	int o3_getScore(EMCState *script);
	int o3_daggerWarning(EMCState *script);
	int o3_blockOutWalkableRegion(EMCState *script);
	int o3_showSceneStringsMessage(EMCState *script);
	int o3_showGoodConscience(EMCState *script);
	int o3_goodConscienceChat(EMCState *script);
	int o3_hideGoodConscience(EMCState *script);
	int o3_defineSceneAnim(EMCState *script);
	int o3_updateSceneAnim(EMCState *script);
	int o3_runActorScript(EMCState *script);
	int o3_doDialog(EMCState *script);
	int o3_setConversationState(EMCState *script);
	int o3_getConversationState(EMCState *script);
	int o3_changeChapter(EMCState *script);
	int o3_countItemInstances(EMCState *script);
	int o3_dialogStartScript(EMCState *script);
	int o3_dialogEndScript(EMCState *script);
	int o3_customChat(EMCState *script);
	int o3_customChatFinish(EMCState *script);
	int o3_setupSceneAnimObject(EMCState *script);
	int o3_removeSceneAnimObject(EMCState *script);
	int o3_dummy(EMCState *script);

	// misc
	TextDisplayer_MR *_text;
	bool _wasPlayingVQA;

	// resource specific
private:
	static const char *const _languageExtension[];
	static const int _languageExtensionSize;

	int loadLanguageFile(const char *file, uint8 *&buffer);
};

} // End of namespace Kyra

#endif

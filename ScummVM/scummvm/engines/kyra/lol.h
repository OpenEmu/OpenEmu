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

#ifdef ENABLE_LOL

#ifndef KYRA_LOL_H
#define KYRA_LOL_H

#include "kyra/kyra_rpg.h"
#include "kyra/script_tim.h"
#include "kyra/script.h"
#include "kyra/gui_lol.h"
#include "kyra/text_lol.h"

#include "common/list.h"

namespace Audio {
class SeekableAudioStream;
} // End of namespace Audio

namespace Kyra {

class Screen_LoL;
class WSAMovie_v2;
struct Button;

struct LoLCharacter {
	uint16 flags;
	char name[11];
	uint8 raceClassSex;
	int16 id;
	uint8 curFaceFrame;
	uint8 tempFaceFrame;
	uint8 screamSfx;
	const uint16 *defaultModifiers;
	uint16 itemsMight[8];
	uint16 protectionAgainstItems[8];
	uint16 itemProtection;
	int16 hitPointsCur;
	uint16 hitPointsMax;
	int16 magicPointsCur;
	uint16 magicPointsMax;
	uint8 field_41;
	uint16 damageSuffered;
	uint16 weaponHit;
	uint16 totalMightModifier;
	uint16 totalProtectionModifier;
	uint16 might;
	uint16 protection;
	int16 nextAnimUpdateCountdown;
	uint16 items[11];
	uint8 skillLevels[3];
	int8 skillModifiers[3];
	int32 experiencePts[3];
	uint8 characterUpdateEvents[5];
	uint8 characterUpdateDelay[5];
};

struct SpellProperty {
	uint16 spellNameCode;
	uint16 mpRequired[4];
	uint16 field_a;
	uint16 field_c;
	uint16 hpRequired[4];
	uint16 field_16;
	uint16 field_18;
	uint16 flags;
};

struct LoLMonsterProperty {
	uint8 shapeIndex;
	uint8 maxWidth;
	uint16 fightingStats[9];
	uint16 itemsMight[8];
	uint16 protectionAgainstItems[8];
	uint16 itemProtection;
	uint16 hitPoints;
	uint8 speedTotalWaitTicks;
	uint8 skillLevel;
	uint16 flags;
	uint16 unk5;
	uint16 numDistAttacks;
	uint16 numDistWeapons;
	uint16 distWeapons[3];
	uint8 attackSkillChance;
	uint8 attackSkillType;
	uint8 defenseSkillChance;
	uint8 defenseSkillType;
	uint8 sounds[3];
};

struct LoLObject {
	uint16 nextAssignedObject;
	uint16 nextDrawObject;
	uint8 flyingHeight;
	uint16 block;
	uint16 x;
	uint16 y;
};

struct LoLMonster : public LoLObject {
	uint8 destDirection;
	int8 shiftStep;
	uint16 destX;
	uint16 destY;

	int8 hitOffsX;
	int8 hitOffsY;
	uint8 currentSubFrame;
	uint8 mode;
	int8 fightCurTick;
	uint8 id;
	uint8 direction;
	uint8 facing;
	uint16 flags;
	uint16 damageReceived;
	int16 hitPoints;
	uint8 speedTick;
	uint8 type;
	LoLMonsterProperty *properties;
	uint8 numDistAttacks;
	uint8 curDistWeapon;
	int8 distAttackTick;
	uint16 assignedItems;
	uint8 equipmentShapes[4];
};

struct LoLItem : public LoLObject {
	int8 level;
	uint16 itemPropertyIndex;
	uint16 shpCurFrame_flg;
};

struct ItemProperty {
	uint16 nameStringId;
	uint8 shpIndex;
	uint16 flags;
	uint16 type;
	uint8 itemScriptFunc;
	int8 might;
	uint8 skill;
	uint8 protection;
	uint16 unkB;
	uint8 unkD;
};

struct CompassDef {
	uint8 shapeIndex;
	int8 x;
	int8 y;
	uint8 flags;
};

struct LoLButtonDef {
	uint16 buttonflags;
	uint16 keyCode;
	uint16 keyCode2;
	int16 x;
	int16 y;
	uint16 w;
	uint16 h;
	uint16 index;
	uint16 screenDim;
};

struct ActiveSpell {
	uint8 spell;
	const SpellProperty *p;
	uint8 charNum;
	uint8 level;
	uint8 target;
};

struct FlyingObject {
	uint8 enable;
	uint8 objectType;
	uint16 attackerId;
	Item item;
	uint16 x;
	uint16 y;
	uint8 flyingHeight;
	uint8 direction;
	uint8 distance;
	int8 field_D;
	uint8 c;
	uint8 flags;
	uint8 wallFlags;
};

struct FlyingObjectShape {
	uint8 shapeFront;
	uint8 shapeBack;
	uint8 shapeLeft;
	uint8 drawFlags;
	uint8 flipFlags;
};

struct MapLegendData {
	uint8 shapeIndex;
	bool enable;
	int8 y;
	uint16 stringId;
};

struct LightningProperty {
	uint8 lastFrame;
	uint8 frameDiv;
	int16 sfxId;
};

struct FireballState {
	FireballState(int i) {
		active = true;
		destX = 200;
		destY = 60;
		tblIndex = ((i * 50) % 255) + 200;
		progress = 1000;
		step = 10;
		finalize = false;
		finProgress = 0;
	}

	bool active;
	int16 destX;
	int16 destY;
	uint16 tblIndex;
	int32 progress;
	uint8 step;
	bool finalize;
	uint8 finProgress;
};

struct MistOfDoomAnimData {
	uint8 part1First;
	uint8 part1Last;
	uint8 part2First;
	uint8 part2Last;
	uint8 sound;
};

class LoLEngine : public KyraRpgEngine {
friend class GUI_LoL;
friend class TextDisplayer_LoL;
friend class TIMInterpreter_LoL;
friend class TimAnimator;
friend class Debugger_LoL;
friend class HistoryPlayer;
public:
	LoLEngine(OSystem *system, const GameFlags &flags);
	virtual ~LoLEngine();

	virtual void initKeymap();

	void pauseEngineIntern(bool pause);

	Screen *screen();
	GUI *gui() const;

private:
	Screen_LoL *_screen;
	GUI_LoL *_gui;

	TIMInterpreter *_tim;

	Common::Error init();
	Common::Error go();

	// initialization
	void initStaticResource();
	void preInit();

	void loadItemIconShapes();
	int mainMenu();

	void startup();
	void startupNew();

	void registerDefaultSettings();
	void writeSettings();
	void readSettings();

	static const char *const kKeymapName;

	const char *const *_pakFileList;
	int _pakFileListSize;

	// options
	int _monsterDifficulty;
	bool _smoothScrollingEnabled;
	bool _floatingCursorsEnabled;

	// main loop
	void runLoop();
	void update();

	// mouse
	void setMouseCursorToIcon(int icon);
	void setMouseCursorToItemInHand();
	uint8 *getItemIconShapePtr(int index);

	void checkFloatingPointerRegions();
	int _floatingCursorControl;
	int _currentFloatingCursor;

	// intro + character selection
	int processPrologue();
	void setupPrologueData(bool load);

	void showIntro();

	struct CharacterPrev {
		int x, y;
		int attrib[3];
	};

	static const CharacterPrev _charPreviews[];
	static const char *const _charPreviewNamesDefault[];
	static const char *const _charPreviewNamesRussianFloppy[];

	// PC98 specific data
	static const uint16 _charPosXPC98[];
	static const uint8 _charNamesPC98[][11];

	WSAMovie_v2 *_chargenWSA;
	static const uint8 _chargenFrameTableTalkie[];
	static const uint8 _chargenFrameTableFloppy[];
	const uint8 *_chargenFrameTable;
	int chooseCharacter();

	void kingSelectionIntro();
	void kingSelectionReminder();
	void kingSelectionOutro();
	void processCharacterSelection();
	void updateSelectionAnims();
	int selectionCharInfo(int character);
	void selectionCharInfoIntro(char *file);

	int getCharSelection();
	int selectionCharAccept();

	void showStarcraftLogo();

	int _charSelection;
	int _charSelectionInfoResult;

	uint32 _selectionAnimTimers[4];
	uint8 _selectionAnimFrames[4];
	static const uint8 _selectionAnimIndexTable[];

	static const uint16 _selectionPosTable[];

	static const uint8 _selectionChar1IdxTable[];
	static const uint8 _selectionChar2IdxTable[];
	static const uint8 _selectionChar3IdxTable[];
	static const uint8 _selectionChar4IdxTable[];

	static const uint8 _reminderChar1IdxTable[];
	static const uint8 _reminderChar2IdxTable[];
	static const uint8 _reminderChar3IdxTable[];
	static const uint8 _reminderChar4IdxTable[];

	static const uint8 _charInfoFrameTable[];

	// outro
	void showOutro(int character, bool maxDifficulty);
	void setupEpilogueData(bool load);

	void showCredits();
	void processCredits(char *text, int dimState, int page, int delay);
	void loadOutroShapes(int file, uint8 **storage);

	uint8 _outroShapeTable[256];

	// TODO: Consider moving these tables to kyra.dat
	static const char *const _outroShapeFileTable[];
	static const uint8 _outroFrameTable[];

	static const int16 _outroRightMonsterPos[];
	static const int16 _outroLeftMonsterPos[];
	static const int16 _outroRightDoorPos[];
	static const int16 _outroLeftDoorPos[];

	static const int _outroMonsterScaleTableX[];
	static const int _outroMonsterScaleTableY[];

	// Non-interactive demo
	int playDemo();
	void pauseDemoPlayer(bool toggle);

	// timers
	void setupTimers();

	void timerProcessMonsters(int timerNum);
	void timerSpecialCharacterUpdate(int timerNum);
	void timerProcessFlyingObjects(int timerNum);
	void timerRunSceneAnimScript(int timerNum);
	void timerRegeneratePoints(int timerNum);
	void timerUpdatePortraitAnimations(int skipUpdate);
	void timerUpdateLampState(int timerNum);
	void timerFadeMessageText(int timerNum);

	uint8 getClock2Timer(int index) { return index < _numClock2Timers ? _clock2Timers[index] : 0; }
	uint8 getNumClock2Timers()  { return _numClock2Timers; }

	static const uint8 _clock2Timers[];
	static const uint8 _numClock2Timers;

	// sound
	int convertVolumeToMixer(int value);
	int convertVolumeFromMixer(int value);

	void loadTalkFile(int index);
	void snd_playVoiceFile(int track) {}
	bool snd_playCharacterSpeech(int id, int8 speaker, int);
	int snd_updateCharacterSpeech();
	void snd_stopSpeech(bool setFlag);
	void snd_playSoundEffect(int track, int volume);
	bool snd_processEnvironmentalSoundEffect(int soundId, int block);
	void snd_queueEnvironmentalSoundEffect(int soundId, int block);
	void snd_playQueuedEffects();
	void snd_loadSoundFile(int track);
	int snd_playTrack(int track);
	int snd_stopMusic();

	int _lastSpeechId;
	int _lastSpeaker;
	int _lastSfxTrack;
	int _lastMusicTrack;
	int _curMusicFileIndex;
	char _curMusicFileExt;
	bool _envSfxUseQueue;
	int _envSfxNumTracksInQueue;
	uint16 _envSfxQueuedTracks[10];
	uint16 _envSfxQueuedBlocks[10];
	int _nextSpeechId;
	int _nextSpeaker;
	typedef Common::List<Audio::SeekableAudioStream *> SpeechList;
	SpeechList _speechList;

	int _curTlkFile;

	char **_ingameSoundList;
	int _ingameSoundListSize;

	const uint8 *_musicTrackMap;
	const uint16 *_ingameSoundIndex;
	const uint8 *_ingameGMSoundIndex;
	int _ingameGMSoundIndexSize;
	const uint8 *_ingameMT32SoundIndex;
	int _ingameMT32SoundIndexSize;
	const uint8 *_ingamePCSpeakerSoundIndex;
	int _ingamePCSpeakerSoundIndexSize;

	// gui
	void gui_drawPlayField();
	void gui_drawScene(int pageNum);
	void gui_drawAllCharPortraitsWithStats();
	void gui_drawCharPortraitWithStats(int charNum);
	void gui_drawCharFaceShape(int charNum, int x, int y, int pageNum);
	void gui_highlightPortraitFrame(int charNum);
	void gui_drawLiveMagicBar(int x, int y, int curPoints, int unk, int maxPoints, int w, int h, int col1, int col2, int flag);
	void gui_drawMoneyBox(int pageNum);
	void gui_drawInventory();
	void gui_drawInventoryItem(int index);
	void gui_drawCompass();
	void gui_drawScroll();
	void gui_highlightSelectedSpell(bool mode);
	void gui_displayCharInventory(int charNum);
	void gui_printCharInventoryStats(int charNum);
	void gui_printCharacterStats(int index, int redraw, int value);
	void gui_changeCharacterStats(int charNum);
	void gui_drawCharInventoryItem(int itemIndex);

	int gui_enableControls();
	int gui_disableControls(int controlMode);
	void gui_toggleButtonDisplayMode(int shapeIndex, int mode);
	void gui_toggleFightButtons(bool disable);
	void gui_prepareForSequence(int x, int y, int w, int h, int buttonFlags);
	void gui_specialSceneSuspendControls(int controlMode);
	void gui_specialSceneRestoreControls(int restoreLamp);

	bool _weaponsDisabled;
	int _lastButtonShape;
	uint32 _buttonPressTimer;
	int _selectedCharacter;
	int _compassStep;
	int _compassDirectionIndex;
	uint32 _compassTimer;
	int _charInventoryUnk;

	const CompassDef *_compassDefs;

	void gui_updateInput();
	void gui_triggerEvent(int eventType);
	void gui_enableDefaultPlayfieldButtons();
	void gui_enableSequenceButtons(int x, int y, int w, int h, int enableFlags);
	void gui_specialSceneRestoreButtons();
	void gui_enableCharInventoryButtons(int charNum);

	void gui_setFaceFramesControlButtons(int index, int xOffs);
	void gui_initCharInventorySpecialButtons(int charNum);
	void gui_initMagicScrollButtons();
	void gui_initMagicSubmenu(int charNum);
	void gui_initButton(int index, int x = -1, int y = -1, int val = -1);

	LoLButtonDef _sceneWindowButton;

	int clickedUpArrow(Button *button);
	int clickedDownArrow(Button *button);
	int clickedLeftArrow(Button *button);
	int clickedRightArrow(Button *button);
	int clickedTurnLeftArrow(Button *button);
	int clickedTurnRightArrow(Button *button);
	int clickedAttackButton(Button *button);
	int clickedMagicButton(Button *button);
	int clickedMagicSubmenu(Button *button);
	int clickedScreen(Button *button);
	int clickedPortraitLeft(Button *button);
	int clickedLiveMagicBarsLeft(Button *button);
	int clickedPortraitEtcRight(Button *button);
	int clickedCharInventorySlot(Button *button);
	int clickedExitCharInventory(Button *button);
	int clickedSceneDropItem(Button *button);
	int clickedScenePickupItem(Button *button);
	int clickedInventorySlot(Button *button);
	int clickedInventoryScroll(Button *button);
	int clickedWall(Button *button);
	int clickedSequenceWindow(Button *button);
	int clickedScroll(Button *button);
	int clickedSpellTargetCharacter(Button *button);
	int clickedSpellTargetScene(Button *button);
	int clickedSceneThrowItem(Button *button);
	int clickedOptions(Button *button);
	int clickedRestParty(Button *button);
	int clickedMoneyBox(Button *button);
	int clickedCompass(Button *button);
	int clickedAutomap(Button *button);
	int clickedLamp(Button *button);
	int clickedStatusIcon(Button *button);

	const LoLButtonDef *_buttonData;
	const int16 *_buttonList1;
	const int16 *_buttonList2;
	const int16 *_buttonList3;
	const int16 *_buttonList4;
	const int16 *_buttonList5;
	const int16 *_buttonList6;
	const int16 *_buttonList7;
	const int16 *_buttonList8;

	// text
	int characterSays(int track, int charId, bool redraw);
	int playCharacterScriptChat(int charId, int mode, int restorePortrait, char *str, EMCState *script, const uint16 *paramList, int16 paramIndex);
	void setupDialogueButtons(int numStr, const char *s1, const char *s2, const char *s3);

	TextDisplayer_LoL *_txt;
	TextDisplayer_rpg *txt() { return _txt; }

	// emc scripts
	void runInitScript(const char *filename, int optionalFunc);
	void runInfScript(const char *filename);
	void runLevelScript(int block, int flags);
	void runLevelScriptCustom(int block, int flags, int charNum, int item, int reg3, int reg4);

	EMCData _scriptData;
	bool _suspendScript;
	uint16 _scriptDirection;
	int16 _globalScriptVars[24];

	// emc opcode
	int olol_setWallType(EMCState *script);
	int olol_getWallType(EMCState *script);
	int olol_drawScene(EMCState *script);
	int olol_rollDice(EMCState *script);
	int olol_moveParty(EMCState *script);
	int olol_delay(EMCState *script);
	int olol_setGameFlag(EMCState *script);
	int olol_testGameFlag(EMCState *script);
	int olol_loadLevelGraphics(EMCState *script);
	int olol_loadBlockProperties(EMCState *script);
	int olol_loadMonsterShapes(EMCState *script);
	int olol_deleteHandItem(EMCState *script);
	int olol_allocItemPropertiesBuffer(EMCState *script);
	int olol_setItemProperty(EMCState *script);
	int olol_makeItem(EMCState *script);
	int olol_placeMoveLevelItem(EMCState *script);
	int olol_createLevelItem(EMCState *script);
	int olol_getItemPara(EMCState *script);
	int olol_getCharacterStat(EMCState *script);
	int olol_setCharacterStat(EMCState *script);
	int olol_loadLevelShapes(EMCState *script);
	int olol_closeLevelShapeFile(EMCState *script);
	int olol_loadDoorShapes(EMCState *script);
	int olol_initAnimStruct(EMCState *script);
	int olol_playAnimationPart(EMCState *script);
	int olol_freeAnimStruct(EMCState *script);
	int olol_getDirection(EMCState *script);
	int olol_characterSurpriseFeedback(EMCState *script);
	int olol_setMusicTrack(EMCState *script);
	int olol_setSequenceButtons(EMCState *script);
	int olol_setDefaultButtonState(EMCState *script);
	int olol_checkRectForMousePointer(EMCState *script);
	int olol_clearDialogueField(EMCState *script);
	int olol_setupBackgroundAnimationPart(EMCState *script);
	int olol_startBackgroundAnimation(EMCState *script);
	int olol_fadeToBlack(EMCState *script);
	int olol_fadePalette(EMCState *script);
	int olol_loadBitmap(EMCState *script);
	int olol_stopBackgroundAnimation(EMCState *script);
	int olol_getGlobalScriptVar(EMCState *script);
	int olol_setGlobalScriptVar(EMCState *script);
	int olol_getGlobalVar(EMCState *script);
	int olol_setGlobalVar(EMCState *script);
	int olol_triggerDoorSwitch(EMCState *script);
	int olol_checkEquippedItemScriptFlags(EMCState *script);
	int olol_setDoorState(EMCState *script);
	int olol_updateBlockAnimations(EMCState *script);
	int olol_assignLevelDecorationShape(EMCState *script);
	int olol_resetBlockShapeAssignment(EMCState *script);
	int olol_copyRegion(EMCState *script);
	int olol_initMonster(EMCState *script);
	int olol_fadeClearSceneWindow(EMCState *script);
	int olol_fadeSequencePalette(EMCState *script);
	int olol_redrawPlayfield(EMCState *script);
	int olol_loadNewLevel(EMCState *script);
	int olol_getNearestMonsterFromCharacter(EMCState *script);
	int olol_dummy0(EMCState *script);
	int olol_loadMonsterProperties(EMCState *script);
	int olol_battleHitSkillTest(EMCState *script);
	int olol_inflictDamage(EMCState *script);
	int olol_moveMonster(EMCState *script);
	int olol_setupDialogueButtons(EMCState *script);
	int olol_giveTakeMoney(EMCState *script);
	int olol_checkMoney(EMCState *script);
	int olol_setScriptTimer(EMCState *script);
	int olol_createHandItem(EMCState *script);
	int olol_playAttackSound(EMCState *script);
	int olol_addRemoveCharacter(EMCState *script);
	int olol_giveItem(EMCState *script);
	int olol_loadTimScript(EMCState *script);
	int olol_runTimScript(EMCState *script);
	int olol_releaseTimScript(EMCState *script);
	int olol_initSceneWindowDialogue(EMCState *script);
	int olol_restoreAfterSceneWindowDialogue(EMCState *script);
	int olol_getItemInHand(EMCState *script);
	int olol_checkMagic(EMCState *script);
	int olol_giveItemToMonster(EMCState *script);
	int olol_loadLangFile(EMCState *script);
	int olol_playSoundEffect(EMCState *script);
	int olol_processDialogue(EMCState *script);
	int olol_stopTimScript(EMCState *script);
	int olol_getWallFlags(EMCState *script);
	int olol_changeMonsterStat(EMCState *script);
	int olol_getMonsterStat(EMCState *script);
	int olol_releaseMonsterShapes(EMCState *script);
	int olol_playCharacterScriptChat(EMCState *script);
	int olol_playEnvironmentalSfx(EMCState *script);
	int olol_update(EMCState *script);
	int olol_healCharacter(EMCState *script);
	int olol_drawExitButton(EMCState *script);
	int olol_loadSoundFile(EMCState *script);
	int olol_playMusicTrack(EMCState *script);
	int olol_deleteMonstersFromBlock(EMCState *script);
	int olol_countBlockItems(EMCState *script);
	int olol_characterSkillTest(EMCState *script);
	int olol_countAllMonsters(EMCState *script);
	int olol_playEndSequence(EMCState *script);
	int olol_stopPortraitSpeechAnim(EMCState *script);
	int olol_setPaletteBrightness(EMCState *script);
	int olol_calcInflictableDamage(EMCState *script);
	int olol_getInflictedDamage(EMCState *script);
	int olol_checkForCertainPartyMember(EMCState *script);
	int olol_printMessage(EMCState *script);
	int olol_deleteLevelItem(EMCState *script);
	int olol_calcInflictableDamagePerItem(EMCState *script);
	int olol_distanceAttack(EMCState *script);
	int olol_removeCharacterEffects(EMCState *script);
	int olol_checkInventoryFull(EMCState *script);
	int olol_moveBlockObjects(EMCState *script);
	int olol_addSpellToScroll(EMCState *script);
	int olol_playDialogueText(EMCState *script);
	int olol_playDialogueTalkText(EMCState *script);
	int olol_checkMonsterTypeHostility(EMCState *script);
	int olol_setNextFunc(EMCState *script);
	int olol_dummy1(EMCState *script);
	int olol_suspendMonster(EMCState *script);
	int olol_setScriptTextParameter(EMCState *script);
	int olol_triggerEventOnMouseButtonClick(EMCState *script);
	int olol_printWindowText(EMCState *script);
	int olol_countSpecificMonsters(EMCState *script);
	int olol_updateBlockAnimations2(EMCState *script);
	int olol_checkPartyForItemType(EMCState *script);
	int olol_blockDoor(EMCState *script);
	int olol_resetTimDialogueState(EMCState *script);
	int olol_getItemOnPos(EMCState *script);
	int olol_removeLevelItem(EMCState *script);
	int olol_savePage5(EMCState *script);
	int olol_restorePage5(EMCState *script);
	int olol_initDialogueSequence(EMCState *script);
	int olol_restoreAfterDialogueSequence(EMCState *script);
	int olol_setSpecialSceneButtons(EMCState *script);
	int olol_restoreButtonsAfterSpecialScene(EMCState *script);
	int olol_prepareSpecialScene(EMCState *script);
	int olol_restoreAfterSpecialScene(EMCState *script);
	int olol_assignCustomSfx(EMCState *script);
	int olol_findAssignedMonster(EMCState *script);
	int olol_checkBlockForMonster(EMCState *script);
	int olol_crossFadeRegion(EMCState *script);
	int olol_calcCoordinatesAddDirectionOffset(EMCState *script);
	int olol_resetPortraitsAndDisableSysTimer(EMCState *script);
	int olol_enableSysTimer(EMCState *script);
	int olol_checkNeedSceneRestore(EMCState *script);
	int olol_getNextActiveCharacter(EMCState *script);
	int olol_paralyzePoisonCharacter(EMCState *script);
	int olol_drawCharPortrait(EMCState *script);
	int olol_removeInventoryItem(EMCState *script);
	int olol_getAnimationLastPart(EMCState *script);
	int olol_assignSpecialGuiShape(EMCState *script);
	int olol_findInventoryItem(EMCState *script);
	int olol_restoreFadePalette(EMCState *script);
	int olol_getSelectedCharacter(EMCState *script);
	int olol_setHandItem(EMCState *script);
	int olol_drinkBezelCup(EMCState *script);
	int olol_changeItemTypeOrFlag(EMCState *script);
	int olol_placeInventoryItemInHand(EMCState *script);
	int olol_castSpell(EMCState *script);
	int olol_pitDrop(EMCState *script);
	int olol_increaseSkill(EMCState *script);
	int olol_paletteFlash(EMCState *script);
	int olol_restoreMagicShroud(EMCState *script);
	int olol_disableControls(EMCState *script);
	int olol_enableControls(EMCState *script);
	int olol_shakeScene(EMCState *script);
	int olol_gasExplosion(EMCState *script);
	int olol_calcNewBlockPosition(EMCState *script);
	int olol_crossFadeScene(EMCState *script);
	int olol_updateDrawPage2(EMCState *script);
	int olol_setMouseCursor(EMCState *script);
	int olol_characterSays(EMCState *script);
	int olol_queueSpeech(EMCState *script);
	int olol_getItemPrice(EMCState *script);
	int olol_getLanguage(EMCState *script);

	// tim scripts
	TIM *_activeTim[10];

	// tim opcode
	void setupOpcodeTable();

	Common::Array<const TIMOpcode *> _timIntroOpcodes;
	int tlol_setupPaletteFade(const TIM *tim, const uint16 *param);
	int tlol_loadPalette(const TIM *tim, const uint16 *param);
	int tlol_setupPaletteFadeEx(const TIM *tim, const uint16 *param);
	int tlol_processWsaFrame(const TIM *tim, const uint16 *param);
	int tlol_displayText(const TIM *tim, const uint16 *param);

	Common::Array<const TIMOpcode *> _timOutroOpcodes;
	int tlol_fadeInScene(const TIM *tim, const uint16 *param);
	int tlol_unusedResourceFunc(const TIM *tim, const uint16 *param);
	int tlol_fadeInPalette(const TIM *tim, const uint16 *param);
	int tlol_fadeSoundOut(const TIM *tim, const uint16 *param);
	int tlol_displayAnimFrame(const TIM *tim, const uint16 *param);
	int tlol_delayForChat(const TIM *tim, const uint16 *param);
	int tlol_fadeOutSound(const TIM *tim, const uint16 *param);

	Common::Array<const TIMOpcode *> _timIngameOpcodes;
	int tlol_initSceneWindowDialogue(const TIM *tim, const uint16 *param);
	int tlol_restoreAfterSceneWindowDialogue(const TIM *tim, const uint16 *param);
	int tlol_giveItem(const TIM *tim, const uint16 *param);
	int tlol_setPartyPosition(const TIM *tim, const uint16 *param);
	int tlol_fadeClearWindow(const TIM *tim, const uint16 *param);
	int tlol_copyRegion(const TIM *tim, const uint16 *param);
	int tlol_characterChat(const TIM *tim, const uint16 *param);
	int tlol_drawScene(const TIM *tim, const uint16 *param);
	int tlol_update(const TIM *tim, const uint16 *param);
	int tlol_clearTextField(const TIM *tim, const uint16 *param);
	int tlol_loadSoundFile(const TIM *tim, const uint16 *param);
	int tlol_playMusicTrack(const TIM *tim, const uint16 *param);
	int tlol_playDialogueTalkText(const TIM *tim, const uint16 *param);
	int tlol_playSoundEffect(const TIM *tim, const uint16 *param);
	int tlol_startBackgroundAnimation(const TIM *tim, const uint16 *param);
	int tlol_stopBackgroundAnimation(const TIM *tim, const uint16 *param);

	// translation
	int _lang;

	uint8 *_landsFile;
	uint8 *_levelLangFile;

	int _lastUsedStringBuffer;
	char _stringBuffer[5][512]; // TODO: The original used a size of 512, it looks a bit large.
	                            // Maybe we can someday reduce the size.
	char *getLangString(uint16 id);
	uint8 *getTableEntry(uint8 *buffer, uint16 id);
	void decodeSjis(const char *src, char *dst);
	int decodeCyrillic(const char *src, char *dst);

	static const char *const _languageExt[];

	// graphics
	void setupScreenDims();
	void initSceneWindowDialogue(int controlMode);
	void restoreAfterSceneWindowDialogue(int redraw);
	void initDialogueSequence(int controlMode, int pageNum);
	void restoreAfterDialogueSequence(int controlMode);
	void resetPortraitsAndDisableSysTimer();
	void toggleSelectedCharacterFrame(bool mode);
	void fadeText();
	void transformRegion(int x1, int y1, int x2, int y2, int w, int h, int srcPage, int dstPage);
	void setPaletteBrightness(const Palette &srcPal, int brightness, int modifier);
	void generateBrightnessPalette(const Palette &src, Palette &dst, int brightness, int16 modifier);
	void generateFlashPalette(const Palette &src, Palette &dst, int colorFlags);
	void createTransparencyTables();
	void updateSequenceBackgroundAnimations();

	uint8 **_itemIconShapes;
	int _numItemIconShapes;
	uint8 **_itemShapes;
	int _numItemShapes;
	uint8 **_gameShapes;
	int _numGameShapes;
	uint8 **_thrownShapes;
	int _numThrownShapes;
	uint8 **_effectShapes;
	int _numEffectShapes;

	const int8 *_gameShapeMap;

	uint8 *_characterFaceShapes[40][3];

	// characters
	bool addCharacter(int id);
	void setTemporaryFaceFrame(int charNum, int frame, int updateDelay, int redraw);
	void setTemporaryFaceFrameForAllCharacters(int frame, int updateDelay, int redraw);
	void setCharacterUpdateEvent(int charNum, int updateType, int updateDelay, int overwrite);
	int countActiveCharacters();
	void loadCharFaceShapes(int charNum, int id);
	void calcCharPortraitXpos();

	void updatePortraitSpeechAnim();
	void stopPortraitSpeechAnim();
	void initTextFading(int textType, int clearField);
	void setCharFaceFrame(int charNum, int frameNum);
	void faceFrameRefresh(int charNum);

	void recalcCharacterStats(int charNum);
	int calculateCharacterStats(int charNum, int index);
	int calculateProtection(int index);

	void setCharacterMagicOrHitPoints(int charNum, int type, int points, int mode);
	void increaseExperience(int charNum, int skill, uint32 points);
	void increaseCharacterHitpoints(int charNum, int points, bool ignoreDeath);

	LoLCharacter *_characters;
	uint16 _activeCharsXpos[3];

	int _portraitSpeechAnimMode;
	int _textColorFlag;
	uint32 _palUpdateTimer;
	uint32 _updatePortraitNext;

	int _loadLevelFlag;
	int _activeMagicMenu;
	uint16 _scriptCharacterCycle;
	int _charStatsTemp[5];

	const LoLCharacter *_charDefaults;
	int _charDefaultsSize;

	const uint16 *_charDefsMan;
	const uint16 *_charDefsWoman;
	const uint16 *_charDefsKieran;
	const uint16 *_charDefsAkshel;
	const int32 *_expRequirements;

	// lamp
	void resetLampStatus();
	void setLampMode(bool lampOn);
	void updateLampStatus();

	int8 _lampEffect;
	int _brightness;
	int _lampOilStatus;
	uint32 _lampStatusTimer;
	bool _lampStatusSuspended;

	// level
	void loadLevel(int index);
	void addLevelItems();
	void loadLevelWallData(int fileIndex, bool mapShapes);
	void assignBlockItem(LevelBlockProperty *l, uint16 item);
	int assignLevelDecorationShapes(int index);
	uint8 *getLevelDecorationShapes(int index);
	void releaseDecorations(int first = 0, int num = 400);
	void restoreTempDataAdjustMonsterStrength(int index);
	void loadBlockProperties(const char *cmzFile);
	const uint8 *getBlockFileData(int levelIndex);
	void loadLevelShpDat(const char *shpFile, const char *datFile, bool flag);
	void loadLevelGraphics(const char *file, int specialColor, int weight, int vcnLen, int vmpLen, const char *palFile);

	void resetItems(int flag);
	void disableMonsters();
	void resetBlockProperties();
	bool testWallFlag(int block, int direction, int flag);
	bool testWallInvisibility(int block, int direction);

	void drawScene(int pageNum);

	void drawSceneShapes(int start = 0);
	void drawDecorations(int index);
	void drawBlockEffects(int index, int type);
	void drawSpecialGuiShape(int pageNum);
	void setWallType(int block, int wall, int val);
	void updateDrawPage2();

	void prepareSpecialScene(int fieldType, int hasDialogue, int suspendGui, int allowSceneUpdate, int controlMode, int fadeFlag);
	int restoreAfterSpecialScene(int fadeFlag, int redrawPlayField, int releaseTimScripts, int sceneUpdateMode);

	void setSequenceButtons(int x, int y, int w, int h, int enableFlags);
	void setSpecialSceneButtons(int x, int y, int w, int h, int enableFlags);
	void setDefaultButtonState();

	void updateCompass();

	void moveParty(uint16 direction, int unk1, int unk2, int buttonShape);
	void notifyBlockNotPassable(int scrollFlag);
	virtual bool checkBlockPassability(uint16 block, uint16 direction);

	uint16 calcBlockIndex(uint16 x, uint16 y);
	void calcCoordinates(uint16 &x, uint16 &y, int block, uint16 xOffs, uint16 yOffs);
	void calcCoordinatesForSingleCharacter(int charNum, uint16 &x, uint16 &y);
	void calcCoordinatesAddDirectionOffset(uint16 &x, uint16 &y, int direction);

	int clickedDoorSwitch(uint16 block, uint16 direction);
	int clickedNiche(uint16 block, uint16 direction);

	void movePartySmoothScrollBlocked(int speed);
	void movePartySmoothScrollUp(int speed);
	void movePartySmoothScrollDown(int speed);
	void movePartySmoothScrollLeft(int speed);
	void movePartySmoothScrollRight(int speed);
	void movePartySmoothScrollTurnLeft(int speed);
	void movePartySmoothScrollTurnRight(int speed);

	void pitDropScroll(int numSteps);

	void shakeScene(int duration, int width, int height, int restore);
	void processGasExplosion(int soundId);

	int smoothScrollDrawSpecialGuiShape(int pageNum);

	int _blockDoor;

	int _smoothScrollModeNormal;

	const uint8 *_scrollXTop;
	const uint8 *_scrollYTop;
	const uint8 *_scrollXBottom;
	const uint8 *_scrollYBottom;

	int _nextScriptFunc;
	int _lvlShapeIndex;
	bool _partyAwake;

	uint8 *_specialGuiShape;
	uint16 _specialGuiShapeX;
	uint16 _specialGuiShapeY;
	uint16 _specialGuiShapeMirrorFlag;

	char _lastOverridePalFile[12];
	char *_lastOverridePalFilePtr;
	int _lastSpecialColor;
	int _lastSpecialColorWeight;

	uint8 *_transparencyTable2;
	uint8 *_transparencyTable1;

	int _loadSuppFilesFlag;
	uint8 *_wllAutomapData;

	uint16 _partyPosX;
	uint16 _partyPosY;

	Common::SeekableReadStream *_lvlShpFileHandle;

	int _shpDmX;
	int _shpDmY;
	uint16 _dmScaleW;
	uint16 _dmScaleH;

	int _lastMouseRegion;
	int _seqWindowX1, _seqWindowY1, _seqWindowX2, _seqWindowY2, _seqTrigger;
	int _spsWindowX, _spsWindowY, _spsWindowW, _spsWindowH;

	uint8 *_tempBuffer5120;

	const char *const *_levelDatList;
	const char *const *_levelShpList;

	const int8 *_dscWalls;

	const uint8 *_dscOvlMap;
	const uint8 *_dscShapeOvlIndex;
	const uint16 *_dscShapeScaleW;
	const uint16 *_dscShapeScaleH;
	const int8 *_dscShapeY;

	const uint16 *_dscDoorMonsterScaleTable;
	const uint16 *_dscDoor4;
	const int16 *_dscDoorMonsterX;
	const int16 *_dscDoorMonsterY;

	// objects (item/monster common)
	LoLObject *findObject(uint16 index);
	int calcObjectPosition(LoLObject *obj, uint16 direction);
	void removeAssignedObjectFromBlock(LevelBlockProperty *l, uint16 id);
	void removeDrawObjectFromBlock(LevelBlockProperty *l, uint16 id);
	void assignObjectToBlock(uint16 *assignedBlockObjects, uint16 id);

	// items
	void giveCredits(int credits, int redraw);
	void takeCredits(int credits, int redraw);
	Item makeItem(int itemType, int curFrame, int flags);
	void placeMoveLevelItem(Item itemIndex, int level, int block, int xOffs, int yOffs, int flyingHeight);
	bool addItemToInventory(Item itemIndex);
	bool isItemMoveable(Item itemIndex);
	void deleteItem(Item itemIndex);
	void runItemScript(int charNum, Item item, int flags, int next, int reg4);
	void setHandItem(Item itemIndex);
	bool itemEquipped(int charNum, uint16 itemType);

	void setItemPosition(Item item, uint16 x, uint16 y, int flyingHeight, int moveable);
	void removeLevelItem(Item item, int block);
	bool launchObject(int objectType, Item item, int startX, int startY, int flyingHeight, int direction, int, int attackerId, int c);
	void endObjectFlight(FlyingObject *t, int x, int y, int collisionType);
	void processObjectFlight(FlyingObject *t, int x, int y);
	void updateObjectFlightPosition(FlyingObject *t);
	void objectFlightProcessHits(FlyingObject *t, int x, int y, int collisionType);
	void updateFlyingObject(FlyingObject *t);

	void assignItemToBlock(uint16 *assignedBlockObjects, int id);
	int checkDrawObjectSpace(int x1, int y1, int x2, int y2);
	int checkSceneForItems(uint16 *blockDrawObjects, int color);

	uint8 _moneyColumnHeight[5];
	uint16 _credits;

	LoLItem *_itemsInPlay;
	ItemProperty *_itemProperties;

	Item _itemInHand;
	Item _inventory[48];
	Item _inventoryCurItem;

	int _lastCharInventory;
	uint16 _charStatusFlags[3];
	int _emcLastItem;

	FlyingObject *_flyingObjects;

	EMCData _itemScript;

	const uint8 *_charInvIndex;
	const uint8 *_charInvDefs;
	const uint16 *_inventorySlotDesc;
	const uint16 *_itemCost;
	const uint8 *_stashSetupData;
	const int8 *_sceneItemOffs;
	const FlyingObjectShape *_flyingItemShapes;

	// monsters
	void loadMonsterShapes(const char *file, int monsterIndex, int b);
	void releaseMonsterShapes(int monsterIndex);
	int deleteMonstersFromBlock(int block);
	void setMonsterMode(LoLMonster *monster, int mode);
	bool updateMonsterAdjustBlocks(LoLMonster *monster);
	void placeMonster(LoLMonster *monster, uint16 x, uint16 y);
	int calcMonsterDirection(uint16 x1, uint16 y1, uint16 x2, uint16 y2);
	void setMonsterDirection(LoLMonster *monster, int dir);
	void monsterDropItems(LoLMonster *monster);
	void giveItemToMonster(LoLMonster *monster, Item item);
	int checkBlockBeforeObjectPlacement(uint16 x, uint16 y, uint16 objectWidth, uint16 testFlag, uint16 wallFlag);
	int testBlockPassability(int block, int x, int y, int objectWidth, int testFlag, int wallFlag);
	int calcMonsterSkillLevel(int id, int a);
	int checkBlockOccupiedByParty(int x, int y, int testFlag);
	const uint16 *getCharacterOrMonsterStats(int id);
	uint16 *getCharacterOrMonsterItemsMight(int id);
	uint16 *getCharacterOrMonsterProtectionAgainstItems(int id);

	void drawBlockObjects(int blockArrayIndex);
	void drawMonster(uint16 id);
	int getMonsterCurFrame(LoLMonster *m, uint16 dirFlags);
	void reassignDrawObjects(uint16 direction, uint16 itemIndex, LevelBlockProperty *l, bool flag);
	void redrawSceneItem();
	void calcSpriteRelPosition(uint16 x1, uint16 y1, int &x2, int &y2, uint16 direction);
	void drawDoor(uint8 *shape, uint8 *doorPalette, int index, int unk2, int w, int h, int flags);
	void drawDoorOrMonsterEquipment(uint8 *shape, uint8 *objectPalette, int x, int y, int flags, const uint8 *brightnessOverlay);
	uint8 *drawItemOrMonster(uint8 *shape, uint8 *monsterPalette, int x, int y, int fineX, int fineY, int flags, int tblValue, bool vflip);
	int calcDrawingLayerParameters(int srcX, int srcY, int &x2, int &y2, uint16 &w, uint16 &h, uint8 *shape, int vflip);

	void updateMonster(LoLMonster *monster);
	void moveMonster(LoLMonster *monster);
	void walkMonster(LoLMonster *monster);
	bool chasePartyWithDistanceAttacks(LoLMonster *monster);
	void chasePartyWithCloseAttacks(LoLMonster *monster);
	int walkMonsterCalcNextStep(LoLMonster *monster);
	int checkForPossibleDistanceAttack(uint16 monsterBlock, int direction, int distance, uint16 curBlock);
	int walkMonsterCheckDest(int x, int y, LoLMonster *monster, int unk);
	void getNextStepCoords(int16 monsterX, int16 monsterY, int &newX, int &newY, uint16 direction);
	void alignMonsterToParty(LoLMonster *monster);
	void moveStrayingMonster(LoLMonster *monster);
	void killMonster(LoLMonster *monster);

	LoLMonster *_monsters;
	LoLMonsterProperty *_monsterProperties;
	uint8 **_monsterDecorationShapes;
	uint8 _monsterAnimType[3];
	uint16 _monsterCurBlock;
	int _objectLastDirection;

	const uint16 *_monsterModifiers;
	const int8 *_monsterShiftOffs;
	const uint8 *_monsterDirFlags;
	const uint8 *_monsterScaleX;
	const uint8 *_monsterScaleY;
	const uint16 *_monsterScaleWH;

	// misc
	void delay(uint32 millis, bool doUpdate = false, bool isMainLoop = false);

	const KyraRpgGUISettings *guiSettings();

	uint8 _compassBroken;
	uint8 _drainMagic;
	uint16 _globalScriptVars2[8];

	uint8 *_pageBuffer1;
	uint8 *_pageBuffer2;

	static const KyraRpgGUISettings _guiSettings;

	// spells
	typedef Common::Functor1Mem<ActiveSpell *, int, LoLEngine> SpellProc;
	Common::Array<const SpellProc *> _spellProcs;
	typedef void (LoLEngine::*SpellProcCallback)(WSAMovie_v2 *, int, int);

	int castSpell(int charNum, int spellType, int spellLevel);

	int castSpark(ActiveSpell *a);
	int castHeal(ActiveSpell *a);
	int castIce(ActiveSpell *a);
	int castFireball(ActiveSpell *a);
	int castHandOfFate(ActiveSpell *a);
	int castMistOfDoom(ActiveSpell *a);
	int castLightning(ActiveSpell *a);
	int castFog(ActiveSpell *a);
	int castSwarm(ActiveSpell *a);
	int castVaelansCube(ActiveSpell *a);
	int castGuardian(ActiveSpell *a);
	int castHealOnSingleCharacter(ActiveSpell *a);

	int processMagicSpark(int charNum, int spellLevel);
	int processMagicHealSelectTarget();
	int processMagicHeal(int charNum, int spellLevel);
	int processMagicIce(int charNum, int spellLevel);
	int processMagicFireball(int charNum, int spellLevel);
	int processMagicHandOfFate(int spellLevel);
	int processMagicMistOfDoom(int charNum, int spellLevel);
	int processMagicLightning(int charNum, int spellLevel);
	int processMagicFog();
	int processMagicSwarm(int charNum, int damage);
	int processMagicVaelansCube();
	int processMagicGuardian(int charNum);

	void callbackProcessMagicSwarm(WSAMovie_v2 *mov, int x, int y);
	void callbackProcessMagicLightning(WSAMovie_v2 *mov, int x, int y);

	void drinkBezelCup(int a, int charNum);

	void addSpellToScroll(int spell, int charNum);
	void transferSpellToScollAnimation(int charNum, int spell, int slot);

	void playSpellAnimation(WSAMovie_v2 *mov, int firstFrame, int lastFrame, int frameDelay, int x, int y, SpellProcCallback callback, uint8 *pal1, uint8 *pal2, int fadeDelay, bool restoreScreen);
	int checkMagic(int charNum, int spellNum, int spellLevel);
	int getSpellTargetBlock(int currentBlock, int direction, int maxDistance, uint16 &targetBlock);
	void inflictMagicalDamage(int target, int attacker, int damage, int index, int hitType);
	void inflictMagicalDamageForBlock(int block, int attacker, int damage, int index);

	ActiveSpell _activeSpell;
	int8 _availableSpells[8];
	int _selectedSpell;
	const SpellProperty *_spellProperties;
	//int _spellPropertiesSize;
	int _subMenuIndex;

	LightningProperty *_lightningProps;
	int16 _lightningCurSfx;
	int16 _lightningDiv;
	int16 _lightningFirstSfx;
	int16 _lightningSfxFrame;

	uint8 *_healOverlay;
	uint8 _swarmSpellStatus;

	uint8 **_fireballShapes;
	int _numFireballShapes;
	uint8 **_healShapes;
	int _numHealShapes;
	uint8 **_healiShapes;
	int _numHealiShapes;

	static const MistOfDoomAnimData _mistAnimData[];

	const uint8 *_updateSpellBookCoords;
	const uint8 *_updateSpellBookAnimData;
	const uint8 *_healShapeFrames;
	const int16 *_fireBallCoords;

	// fight
	int battleHitSkillTest(int16 attacker, int16 target, int skill);
	int calcInflictableDamage(int16 attacker, int16 target, int hitType);
	int inflictDamage(uint16 target, int damage, uint16 attacker, int skill, int flags);
	void characterHitpointsZero(int16 charNum, int a);
	void removeCharacterEffects(LoLCharacter *c, int first, int last);
	int calcInflictableDamagePerItem(int16 attacker, int16 target, uint16 itemMight, int index, int hitType);
	void checkForPartyDeath();

	void applyMonsterAttackSkill(LoLMonster *monster, int16 target, int16 damage);
	void applyMonsterDefenseSkill(LoLMonster *monster, int16 attacker, int flags, int skill, int damage);
	int removeCharacterItem(int charNum, int itemFlags);
	int paralyzePoisonCharacter(int charNum, int typeFlag, int immunityFlags, int hitChance, int redraw);
	void paralyzePoisonAllCharacters(int typeFlag, int immunityFlags, int hitChance);
	void stunCharacter(int charNum);
	void restoreSwampPalette();

	void launchMagicViper();

	void breakIceWall(uint8 *pal1, uint8 *pal2);

	uint16 getNearestMonsterFromCharacter(int charNum);
	uint16 getNearestMonsterFromCharacterForBlock(uint16 block, int charNum);
	uint16 getNearestMonsterFromPos(int x, int y);
	uint16 getNearestPartyMemberFromPos(int x, int y);

	int _partyDamageFlags;

	// magic atlas
	void displayAutomap();
	void updateAutoMap(uint16 block);
	bool updateAutoMapIntern(uint16 block, uint16 x, uint16 y, int16 xOffs, int16 yOffs);
	void loadMapLegendData(int level);
	void drawMapPage(int pageNum);
	bool automapProcessButtons(int inputFlag);
	void automapBackButton();
	void automapForwardButton();
	void redrawMapCursor();
	void drawMapBlockWall(uint16 block, uint8 wall, int x, int y, int direction);
	void drawMapShape(uint8 wall, int x, int y, int direction);
	int mapGetStartPosX();
	int mapGetStartPosY();
	void mapIncludeLegendData(int type);
	void printMapText(uint16 stringId, int x, int y);
	void printMapExitButtonText();

	uint8 _currentMapLevel;
	uint8 *_mapOverlay;
	const uint8 **_automapShapes;
	const uint16 *_autoMapStrings;
	MapLegendData *_defaultLegendData;
	uint8 *_mapCursorOverlay;
	uint8 _automapTopLeftX;
	uint8 _automapTopLeftY;
	static const int8 _mapCoords[12][4];
	bool _mapUpdateNeeded;

	// unneeded
	void setWalkspeed(uint8) {}
	void removeHandItem() {}
	bool lineIsPassable(int, int) { return false; }

	// save
	Common::Error loadGameState(int slot);
	Common::Error saveGameStateIntern(int slot, const char *saveName, const Graphics::Surface *thumbnail);

	void *generateMonsterTempData(LevelTempData *tmp);
	void restoreBlockTempData(int levelIndex);
	void restoreMonsterTempData(LevelTempData *tmp);
	void releaseMonsterTempData(LevelTempData *tmp);

	Graphics::Surface *generateSaveThumbnail() const;
};

class HistoryPlayer {
public:
	HistoryPlayer(LoLEngine *vm);
	~HistoryPlayer();

	void play();
private:
	OSystem *_system;
	LoLEngine *_vm;
	Screen *_screen;

	int _x, _y, _width, _height;
	int _frame;
	Movie *_wsa;

	void loadWsa(const char *filename);
	void playWsa(bool direction);
	void restoreWsaBkgd();

	Movie *_fireWsa;
	int _fireFrame;
	uint32 _nextFireTime;
	void updateFire();
};

} // End of namespace Kyra

#endif

#endif // ENABLE_LOL

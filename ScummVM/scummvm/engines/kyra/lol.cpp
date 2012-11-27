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

#include "kyra/lol.h"
#include "kyra/screen_lol.h"
#include "kyra/resource.h"
#include "kyra/timer.h"
#include "kyra/util.h"
#include "kyra/debugger.h"
#include "kyra/sound.h"

#include "audio/audiostream.h"

#include "common/config-manager.h"
#include "common/system.h"
#include "common/translation.h"

#include "backends/keymapper/keymapper.h"

namespace Kyra {

const char *const LoLEngine::kKeymapName = "lol";

LoLEngine::LoLEngine(OSystem *system, const GameFlags &flags) : KyraRpgEngine(system, flags) {
	_screen = 0;
	_gui = 0;
	_tim = 0;

	_lang = 0;
	Common::Language lang = Common::parseLanguage(ConfMan.get("language"));
	if (lang == _flags.fanLang && _flags.replacedLang != Common::UNK_LANG)
		lang = _flags.replacedLang;

	switch (lang) {
	case Common::EN_ANY:
	case Common::EN_USA:
	case Common::EN_GRB:
		_lang = 0;
		break;

	case Common::FR_FRA:
		_lang = 1;
		break;

	case Common::DE_DEU:
		_lang = 2;
		break;

	case Common::JA_JPN:
		_lang = 0;
		break;

	default:
		warning("unsupported language, switching back to English");
		_lang = 0;
	}

	_chargenFrameTable = _flags.isTalkie ? _chargenFrameTableTalkie : _chargenFrameTableFloppy;
	_chargenWSA = 0;
	_lastUsedStringBuffer = 0;
	_landsFile = 0;
	_levelLangFile = 0;

	_lastMusicTrack = -1;
	_lastSfxTrack = -1;
	_curTlkFile = -1;
	_lastSpeaker = _lastSpeechId = _nextSpeechId = _nextSpeaker = -1;

	memset(_moneyColumnHeight, 0, sizeof(_moneyColumnHeight));
	_credits = 0;

	_itemsInPlay = 0;
	_itemProperties = 0;
	_itemInHand = 0;
	memset(_inventory, 0, sizeof(_inventory));
	memset(_charStatusFlags, 0, sizeof(_charStatusFlags));
	_inventoryCurItem = 0;
	_lastCharInventory = -1;
	_emcLastItem = -1;

	_itemIconShapes = _itemShapes = _gameShapes = _thrownShapes = _effectShapes = _fireballShapes = _healShapes = _healiShapes = 0;
	_levelShpList = _levelDatList = 0;
	_gameShapeMap = 0;
	memset(_monsterAnimType, 0, 3);
	_healOverlay = 0;
	_swarmSpellStatus = 0;

	_ingameMT32SoundIndex = _ingameGMSoundIndex = _ingamePCSpeakerSoundIndex = 0;

	_charSelection = -1;
	_characters = 0;
	_spellProperties = 0;
	_selectedSpell = 0;
	_updateCharNum = _portraitSpeechAnimMode = _textColorFlag = 0;
	_palUpdateTimer = _updatePortraitNext = 0;
	_lampStatusTimer = 0xffffffff;

	_weaponsDisabled = false;
	_charInventoryUnk = 0;
	_lastButtonShape = 0;
	_buttonPressTimer = 0;
	_selectedCharacter = 0;
	_suspendScript = false;
	_scriptDirection = 0;
	_compassDirectionIndex = -1;
	_compassStep = 0;

	_smoothScrollModeNormal = 1;
	_wllAutomapData = 0;
	_sceneXoffset = 112;
	_sceneShpDim = 13;
	_monsters = 0;
	_monsterProperties = 0;
	_lvlShapeIndex = 0;
	_partyAwake = true;
	_transparencyTable2 = 0;
	_transparencyTable1 = 0;
	_specialGuiShape = 0;
	_specialGuiShapeX = _specialGuiShapeY = _specialGuiShapeMirrorFlag = 0;
	memset(_characterFaceShapes, 0, sizeof(_characterFaceShapes));

	_lampEffect = _brightness = _lampOilStatus = 0;
	_lampStatusSuspended = false;
	_tempBuffer5120 = 0;
	_flyingObjects = 0;
	_monsters = 0;
	_lastMouseRegion = 0;
	_objectLastDirection = 0;
	_monsterCurBlock = 0;
	_seqWindowX1 = _seqWindowY1 = _seqWindowX2 = _seqWindowY2 = _seqTrigger = 0;
	_spsWindowX = _spsWindowY = _spsWindowW = _spsWindowH = 0;

	_dscWalls = 0;
	_dscOvlMap = 0;
	_dscShapeScaleW = 0;
	_dscShapeScaleH = 0;
	_dscShapeY = 0;
	_dscShapeOvlIndex = 0;
	_dscDoorMonsterX = _dscDoorMonsterY = 0;
	_dscDoor4 = 0;

	_ingameSoundList = 0;
	_ingameSoundIndex = 0;
	_ingameSoundListSize = 0;
	_musicTrackMap = 0;
	_curMusicTheme = -1;
	_curMusicFileExt = 0;
	_curMusicFileIndex = -1;
	_envSfxUseQueue = false;
	_envSfxNumTracksInQueue = 0;
	memset(_envSfxQueuedTracks, 0, sizeof(_envSfxQueuedTracks));
	memset(_envSfxQueuedBlocks, 0, sizeof(_envSfxQueuedBlocks));

	_partyPosX = _partyPosY = 0;
	_shpDmX = _shpDmY = _dmScaleW = _dmScaleH = 0;

	_floatingCursorControl = _currentFloatingCursor = 0;

	memset(_activeTim, 0, sizeof(_activeTim));
	memset(&_activeSpell, 0, sizeof(_activeSpell));

	_pageBuffer1 = _pageBuffer2 = 0;

	memset(_charStatsTemp, 0, sizeof(_charStatsTemp));

	_compassBroken = _drainMagic = 0;

	_buttonData = 0;
	_preserveEvents = false;
	_buttonList1 = _buttonList2 = _buttonList3 = _buttonList4 = _buttonList5 = _buttonList6 = _buttonList7 = _buttonList8 = 0;

	_mapOverlay = 0;
	_automapShapes = 0;
	_defaultLegendData = 0;
	_mapCursorOverlay = 0;

	_lightningProps = 0;
	_lightningCurSfx = -1;
	_lightningDiv = 0;
	_lightningFirstSfx = 0;
	_lightningSfxFrame = 0;

	_compassTimer = 0;
	_scriptCharacterCycle = 0;
	_partyDamageFlags = -1;

	memset(&_itemScript, 0, sizeof(_itemScript));
}

LoLEngine::~LoLEngine() {
	setupPrologueData(false);
	releaseTempData();

	delete[] _landsFile;
	delete[] _levelLangFile;

	delete _screen;
	_screen = 0;
	delete _gui;
	_gui = 0;
	delete _tim;
	_tim = 0;
	delete _txt;
	_txt = 0;

	delete[] _itemsInPlay;
	delete[] _itemProperties;
	delete[] _characters;

	delete[] _pageBuffer1;
	delete[] _pageBuffer2;

	if (_itemIconShapes) {
		for (int i = 0; i < _numItemIconShapes; i++)
			delete[]  _itemIconShapes[i];
		delete[] _itemIconShapes;
	}

	if (_itemShapes) {
		for (int i = 0; i < _numItemShapes; i++)
			delete[]  _itemShapes[i];
		delete[] _itemShapes;
	}

	if (_gameShapes) {
		for (int i = 0; i < _numGameShapes; i++)
			delete[]  _gameShapes[i];
		delete[] _gameShapes;
	}

	if (_thrownShapes) {
		for (int i = 0; i < _numThrownShapes; i++)
			delete[]  _thrownShapes[i];
		delete[] _thrownShapes;
	}

	if (_effectShapes) {
		for (int i = 0; i < _numEffectShapes; i++)
			delete[]  _effectShapes[i];
		delete[] _effectShapes;
	}

	if (_fireballShapes) {
		for (int i = 0; i < _numFireballShapes; i++)
			delete[]  _fireballShapes[i];
		delete[] _fireballShapes;
	}

	if (_healShapes) {
		for (int i = 0; i < _numHealShapes; i++)
			delete[]  _healShapes[i];
		delete[] _healShapes;
	}

	if (_healiShapes) {
		for (int i = 0; i < _numHealiShapes; i++)
			delete[]  _healiShapes[i];
		delete[] _healiShapes;
	}

	if (_monsterDecorationShapes) {
		for (int i = 0; i < 3; i++)
			releaseMonsterShapes(i);

		delete[] _monsterShapes;
		_monsterShapes = 0;
		delete[] _monsterPalettes;
		_monsterPalettes = 0;
		delete[] _monsterDecorationShapes;
		_monsterDecorationShapes = 0;
	}

	for (int i = 0; i < 6; i++) {
		delete[] _doorShapes[i];
		_doorShapes[i] = 0;
	}

	releaseDecorations();

	delete[] _automapShapes;

	for (Common::Array<const TIMOpcode *>::iterator i = _timIntroOpcodes.begin(); i != _timIntroOpcodes.end(); ++i)
		delete *i;
	_timIntroOpcodes.clear();

	for (Common::Array<const TIMOpcode *>::iterator i = _timOutroOpcodes.begin(); i != _timOutroOpcodes.end(); ++i)
		delete *i;
	_timOutroOpcodes.clear();

	for (Common::Array<const TIMOpcode *>::iterator i = _timIngameOpcodes.begin(); i != _timIngameOpcodes.end(); ++i)
		delete *i;
	_timIngameOpcodes.clear();

	delete[] _wllAutomapData;
	delete[] _tempBuffer5120;
	delete[] _flyingObjects;
	delete[] _monsters;
	delete[] _monsterProperties;

	delete[] _transparencyTable2;
	delete[] _transparencyTable1;
	delete[] _lightningProps;

	delete _lvlShpFileHandle;

	if (_ingameSoundList) {
		for (int i = 0; i < _ingameSoundListSize; i++)
			delete[] _ingameSoundList[i];
		delete[] _ingameSoundList;
	}

	for (int i = 0; i < 3; i++) {
		for (int ii = 0; ii < 40; ii++)
			delete[] _characterFaceShapes[ii][i];
	}

	delete[] _healOverlay;

	delete[] _defaultLegendData;
	delete[] _mapCursorOverlay;
	delete[] _mapOverlay;

	for (Common::Array<const SpellProc *>::iterator i = _spellProcs.begin(); i != _spellProcs.end(); ++i)
		delete *i;
	_spellProcs.clear();

	for (SpeechList::iterator i = _speechList.begin(); i != _speechList.end(); ++i)
		delete *i;
	_speechList.clear();

	_emc->unload(&_itemScript);
	_emc->unload(&_scriptData);
}

Screen *LoLEngine::screen() {
	return _screen;
}

GUI *LoLEngine::gui() const {
	return _gui;
}

Common::Error LoLEngine::init() {
	_screen = new Screen_LoL(this, _system);
	assert(_screen);
	_screen->setResolution();

	_debugger = new Debugger_LoL(this);
	assert(_debugger);

	KyraEngine_v1::init();
	initStaticResource();

	_gui = new GUI_LoL(this);
	assert(_gui);
	_gui->initStaticData();

	_txt = new TextDisplayer_LoL(this, _screen);

	_screen->setAnimBlockPtr(10000);
	_screen->setScreenDim(0);

	_pageBuffer1 = new uint8[0xfa00];
	memset(_pageBuffer1, 0, 0xfa00);
	_pageBuffer2 = new uint8[0xfa00];
	memset(_pageBuffer2, 0, 0xfa00);

	_itemsInPlay = new LoLItem[400];
	memset(_itemsInPlay, 0, sizeof(LoLItem) * 400);

	_characters = new LoLCharacter[4];
	memset(_characters, 0, sizeof(LoLCharacter) * 4);

	if (!_sound->init())
		error("Couldn't init sound");

	KyraRpgEngine::init();

	_wllAutomapData = new uint8[80];
	memset(_wllAutomapData, 0, 80);

	_monsters = new LoLMonster[30];
	memset(_monsters, 0, 30 * sizeof(LoLMonster));
	_monsterProperties = new LoLMonsterProperty[5];
	memset(_monsterProperties, 0, 5 * sizeof(LoLMonsterProperty));

	_tempBuffer5120 = new uint8[5120];
	memset(_tempBuffer5120, 0, 5120);

	_flyingObjects = new FlyingObject[_numFlyingObjects];
	_flyingObjectsPtr = _flyingObjects;
	_flyingObjectStructSize = sizeof(FlyingObject);
	memset(_flyingObjects, 0, _numFlyingObjects * sizeof(FlyingObject));

	memset(_globalScriptVars, 0, sizeof(_globalScriptVars));

	_lvlShpFileHandle = 0;

	_sceneDrawPage1 = 2;
	_sceneDrawPage2 = 6;

	_clickedShapeXOffs = 136;
	_clickedShapeYOffs = 8;
	_clickedSpecialFlag = 0x40;

	_monsterShapes = new uint8*[48];
	memset(_monsterShapes, 0, 48 * sizeof(uint8 *));
	_monsterPalettes = new uint8*[48];
	memset(_monsterPalettes, 0, 48 * sizeof(uint8 *));
	_monsterDecorationShapes = new uint8*[576];
	memset(_monsterDecorationShapes, 0, 576 * sizeof(uint8 *));
	memset(&_scriptData, 0, sizeof(EMCData));

	_activeMagicMenu = -1;

	_automapShapes = new const uint8*[109];
	_mapOverlay = new uint8[256];

	memset(_availableSpells, -1, 8);

	_spellProcs.push_back(new SpellProc(this, &LoLEngine::castSpark));
	_spellProcs.push_back(new SpellProc(this, &LoLEngine::castHeal));
	_spellProcs.push_back(new SpellProc(this, &LoLEngine::castIce));
	_spellProcs.push_back(new SpellProc(this, &LoLEngine::castFireball));
	_spellProcs.push_back(new SpellProc(this, &LoLEngine::castHandOfFate));
	_spellProcs.push_back(new SpellProc(this, &LoLEngine::castMistOfDoom));
	_spellProcs.push_back(new SpellProc(this, &LoLEngine::castLightning));
	_spellProcs.push_back(new SpellProc(this, 0));
	_spellProcs.push_back(new SpellProc(this, &LoLEngine::castFog));
	_spellProcs.push_back(new SpellProc(this, &LoLEngine::castSwarm));
	_spellProcs.push_back(new SpellProc(this, 0));
	_spellProcs.push_back(new SpellProc(this, 0));
	_spellProcs.push_back(new SpellProc(this, &LoLEngine::castVaelansCube));
	_spellProcs.push_back(new SpellProc(this, 0));
	_spellProcs.push_back(new SpellProc(this, 0));
	_spellProcs.push_back(new SpellProc(this, 0));
	_spellProcs.push_back(new SpellProc(this, &LoLEngine::castGuardian));

#ifdef ENABLE_KEYMAPPER
	_eventMan->getKeymapper()->pushKeymap(kKeymapName, true);
#endif

	return Common::kNoError;
}

void LoLEngine::initKeymap() {
#ifdef ENABLE_KEYMAPPER
	Common::Keymapper *const mapper = _eventMan->getKeymapper();

	// Do not try to recreate same keymap over again
	if (mapper->getKeymap(kKeymapName) != 0)
		return;

	Common::Keymap *const engineKeyMap = new Common::Keymap(kKeymapName);

	const Common::KeyActionEntry keyActionEntries[] = {
		{Common::KeyState(Common::KEYCODE_F1, Common::ASCII_F1), "AT1", _("Attack 1")},
		{Common::KeyState(Common::KEYCODE_F2, Common::ASCII_F2), "AT2", _("Attack 2")},
		{Common::KeyState(Common::KEYCODE_F3, Common::ASCII_F3), "AT3", _("Attack 3")},
		{Common::KeyState(Common::KEYCODE_UP), "MVF", _("Move Forward")},
		{Common::KeyState(Common::KEYCODE_DOWN), "MVB", _("Move Back")},
		{Common::KeyState(Common::KEYCODE_LEFT), "SLL", _("Slide Left")},
		{Common::KeyState(Common::KEYCODE_RIGHT), "SLR", _("Slide Right")},
		{Common::KeyState(Common::KEYCODE_HOME), "TL", _("Turn Left")},
		{Common::KeyState(Common::KEYCODE_PAGEUP), "TR", _("Turn Right")},
		{Common::KeyState(Common::KEYCODE_r), "RST", _("Rest")},
		{Common::KeyState(Common::KEYCODE_o), "OPT", _("Options")},
		{Common::KeyState(Common::KEYCODE_SLASH), "SPL", _("Choose Spell")},
		{Common::KeyState(), 0, 0}
	};

	for (const Common::KeyActionEntry *entry = keyActionEntries; entry->id; ++entry) {
		Common::Action *const act = new Common::Action(engineKeyMap, entry->id, entry->description);
		act->addKeyEvent(entry->ks);
	}

	mapper->addGameKeymap(engineKeyMap);
#endif
}

void LoLEngine::pauseEngineIntern(bool pause) {
	KyraEngine_v1::pauseEngineIntern(pause);
	pauseDemoPlayer(pause);
}

Common::Error LoLEngine::go() {
	int action = -1;

	if (_gameToLoad == -1) {
		action = processPrologue();
		if (action == -1)
			return Common::kNoError;
	}

	if (_flags.isTalkie && !_flags.isDemo) {
		if (!_res->loadFileList("FILEDATA.FDT"))
			error("Couldn't load file list: 'FILEDATA.FDT'");
	} else if (_pakFileList) {
		_res->loadFileList(_pakFileList, _pakFileListSize);
	}

	// Usually fonts etc. would be setup by the prologue code, if we skip
	// the prologue code we need to setup them manually here.
	if (_gameToLoad != -1 && action != 3) {
		preInit();
		_screen->setFont(_flags.use16ColorMode ? Screen::FID_SJIS_FNT : Screen::FID_9_FNT);
	}

	// We have three sound.dat files, one for the intro, one for the
	// end sequence and one for ingame, each contained in a different
	// PAK file. Therefore a new call to loadSoundFile() is required
	// whenever the PAK file configuration changes.
	if (_flags.platform == Common::kPlatformPC98)
		_sound->loadSoundFile("sound.dat");

	_sound->selectAudioResourceSet(kMusicIngame);
	if (_flags.platform != Common::kPlatformPC)
		_sound->loadSoundFile(0);

	_tim = new TIMInterpreter_LoL(this, _screen, _system);
	assert(_tim);

	if (shouldQuit())
		return Common::kNoError;

	startup();

	if (action == 0) {
		startupNew();
	} else if (_gameToLoad != -1) {
		// FIXME: Instead of throwing away the error returned by
		// loadGameState, we should use it / augment it.
		if (loadGameState(_gameToLoad).getCode() != Common::kNoError)
			error("Couldn't load game slot %d on startup", _gameToLoad);
		_gameToLoad = -1;
	}

	_screen->_fadeFlag = 3;
	_sceneUpdateRequired = true;
	enableSysTimer(1);
	runLoop();

	return Common::kNoError;
}

#pragma mark - Initialization

void LoLEngine::preInit() {
	_res->loadPakFile("GENERAL.PAK");
	if (_flags.isTalkie)
		_res->loadPakFile("STARTUP.PAK");

	_screen->loadFont(Screen::FID_9_FNT, "FONT9P.FNT");
	_screen->loadFont(Screen::FID_6_FNT, "FONT6P.FNT");

	loadTalkFile(0);

	Common::String filename;
	filename = Common::String::format("LANDS.%s", _languageExt[_lang]);
	_res->exists(filename.c_str(), true);
	delete[] _landsFile;
	_landsFile = _res->fileData(filename.c_str(), 0);
	loadItemIconShapes();
}

void LoLEngine::loadItemIconShapes() {
	if (_itemIconShapes) {
		for (int i = 0; i < _numItemIconShapes; i++)
			delete[]  _itemIconShapes[i];
		delete[] _itemIconShapes;
	}

	_screen->loadBitmap("ITEMICN.SHP", 3, 3, 0);
	const uint8 *shp = _screen->getCPagePtr(3);
	_numItemIconShapes = READ_LE_UINT16(shp);
	_itemIconShapes = new uint8*[_numItemIconShapes];
	for (int i = 0; i < _numItemIconShapes; i++)
		_itemIconShapes[i] = _screen->makeShapeCopy(shp, i);

	_screen->setMouseCursor(0, 0, _itemIconShapes[0]);

	if (!_gameShapes) {
		_screen->loadBitmap("GAMESHP.SHP", 3, 3, 0);
		shp = _screen->getCPagePtr(3);
		_numGameShapes = READ_LE_UINT16(shp);
		_gameShapes = new uint8*[_numGameShapes];
		for (int i = 0; i < _numGameShapes; i++)
			_gameShapes[i] = _screen->makeShapeCopy(shp, i);
	}
}

void LoLEngine::setMouseCursorToIcon(int icon) {
	_flagsTable[31] |= 0x02;
	int i = _itemProperties[_itemsInPlay[_itemInHand].itemPropertyIndex].shpIndex;
	if (i == icon)
		return;
	_screen->setMouseCursor(0, 0, _itemIconShapes[icon]);
}

void LoLEngine::setMouseCursorToItemInHand() {
	_flagsTable[31] &= 0xFD;
	int o = (_itemInHand == 0) ? 0 : 10;
	_screen->setMouseCursor(o, o, getItemIconShapePtr(_itemInHand));
}

void LoLEngine::checkFloatingPointerRegions() {
	if (!_floatingCursorsEnabled)
		return;

	int t = -1;

	Common::Point p = getMousePos();

	if (!(_updateFlags & 4) & !_floatingCursorControl) {
		if (posWithinRect(p.x, p.y, 96, 0, 303, 136)) {
			if (!posWithinRect(p.x, p.y, 128, 16, 271, 119)) {
				if (posWithinRect(p.x, p.y, 112, 0, 287, 15))
					t = 0;
				if (posWithinRect(p.x, p.y, 272, 88, 303, 319))
					t = 1;
				if (posWithinRect(p.x, p.y, 112, 110, 287, 135))
					t = 2;
				if (posWithinRect(p.x, p.y, 96, 88, 127, 119))
					t = 3;
				if (posWithinRect(p.x, p.y, 96, 16, 127, 87))
					t = 4;
				if (posWithinRect(p.x, p.y, 272, 16, 303, 87))
					t = 5;

				if (t < 4) {
					int d = (_currentDirection + t) & 3;
					if (!checkBlockPassability(calcNewBlockPosition(_currentBlock, d), d))
						t = 6;
				}
			}
		}
	}

	if (t == _currentFloatingCursor)
		return;

	if (t == -1) {
		setMouseCursorToItemInHand();
	} else {
		static const uint8 floatingPtrX[] = { 7, 13, 7, 0, 0, 15, 7 };
		static const uint8 floatingPtrY[] = { 0, 7, 12, 7, 6, 6, 7 };
		_screen->setMouseCursor(floatingPtrX[t], floatingPtrY[t], _gameShapes[10 + t]);
	}

	_currentFloatingCursor = t;
}

uint8 *LoLEngine::getItemIconShapePtr(int index) {
	int ix = _itemProperties[_itemsInPlay[index].itemPropertyIndex].shpIndex;
	if (_itemProperties[_itemsInPlay[index].itemPropertyIndex].flags & 0x200)
		ix += (_itemsInPlay[index].shpCurFrame_flg & 0x1fff) - 1;

	return _itemIconShapes[ix];
}

int LoLEngine::mainMenu() {
	bool hasSave = saveFileLoadable(0);

	MainMenu::StaticData data[] = {
		// 256 color mode
		{
			{ 0, 0, 0, 0, 0 },
			{ 0x01, 0x04, 0x0C, 0x04, 0x00, 0x3D, 0x9F },
			{ 0x2C, 0x19, 0x48, 0x2C },
			Screen::FID_9_FNT, 1
		},
		// 16 color mode
		{
			{ 0, 0, 0, 0, 0 },
			{ 0x01, 0x04, 0x0C, 0x04, 0x00, 0xC1, 0xE1 },
			{ 0xCC, 0xDD, 0xDD, 0xDD },
			Screen::FID_SJIS_FNT, 1
		}
	};

	int dataIndex = _flags.use16ColorMode ? 1 : 0;

	if (!_flags.isTalkie)
		--data[dataIndex].menuTable[3];

	if (hasSave)
		++data[dataIndex].menuTable[3];

	static const uint16 mainMenuStrings[4][5] = {
		{ 0x4248, 0x4249, 0x42DD, 0x424A, 0x0000 },
		{ 0x4248, 0x4249, 0x42DD, 0x4001, 0x424A },
		{ 0x4248, 0x4249, 0x424A, 0x0000, 0x0000 },
		{ 0x4248, 0x4249, 0x4001, 0x424A, 0x0000 }
	};

	int tableOffs = _flags.isTalkie ? 0 : 2;

	for (int i = 0; i < 5; ++i) {
		if (hasSave)
			data[dataIndex].strings[i] = getLangString(mainMenuStrings[1 + tableOffs][i]);
		else
			data[dataIndex].strings[i] = getLangString(mainMenuStrings[tableOffs][i]);
	}

	MainMenu *menu = new MainMenu(this);
	assert(menu);
	menu->init(data[dataIndex], MainMenu::Animation());

	int selection = menu->handle(_flags.isTalkie ? (hasSave ? 19 : 6) : (hasSave ? 6 : 20));
	delete menu;
	_screen->setScreenDim(0);

	if (!_flags.isTalkie && selection >= 2)
		selection++;

	if (!hasSave && selection == 3)
		selection = 4;

	return selection;
}

void LoLEngine::startup() {
	_screen->clearPage(0);

	Palette &pal = _screen->getPalette(0);
	_screen->loadBitmap("PLAYFLD.CPS", 3, 3, &pal);

	if (_flags.use16ColorMode) {
		memset(_screen->_paletteOverlay1, 0, 256);
		memset(_screen->_paletteOverlay2, 0, 256);

		static const uint8 colTable1[] = { 0x00, 0xEE, 0xCC, 0xFF, 0x44, 0x66, 0x44, 0x88, 0xEE, 0xAA, 0x11, 0xCC, 0xDD, 0xEE, 0x44, 0xCC };
		static const uint8 colTable2[] = { 0x00, 0xCC, 0xFF, 0xBB, 0xEE, 0xBB, 0x55, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xFF, 0xCC, 0xDD, 0xBB };
		static const uint8 colTable3[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };

		for (int i = 0; i < 16; i++) {
			_screen->_paletteOverlay1[colTable3[i]] = colTable1[i];
			_screen->_paletteOverlay2[colTable3[i]] = colTable2[i];
		}

	} else {
		_screen->copyPalette(1, 0);
		pal.fill(0, 1, 0x3F);
		pal.fill(2, 126, 0x3F);
		pal.fill(192, 4, 0x3F);
		_screen->generateOverlay(pal, _screen->_paletteOverlay1, 1, 96, 254);
		_screen->generateOverlay(pal, _screen->_paletteOverlay2, 144, 65, 254);
		_screen->copyPalette(0, 1);
	}

	_screen->getPalette(1).clear();
	_screen->getPalette(2).clear();

	loadItemIconShapes();
	_screen->setMouseCursor(0, 0, _itemIconShapes[0x85]);

	_screen->loadBitmap("ITEMSHP.SHP", 3, 3, 0);
	const uint8 *shp = _screen->getCPagePtr(3);
	_numItemShapes = READ_LE_UINT16(shp);
	_itemShapes = new uint8*[_numItemShapes];
	for (int i = 0; i < _numItemShapes; i++)
		_itemShapes[i] = _screen->makeShapeCopy(shp, i);

	_screen->loadBitmap("THROWN.SHP", 3, 3, 0);
	shp = _screen->getCPagePtr(3);
	_numThrownShapes = READ_LE_UINT16(shp);
	_thrownShapes = new uint8*[_numThrownShapes];
	for (int i = 0; i < _numThrownShapes; i++)
		_thrownShapes[i] = _screen->makeShapeCopy(shp, i);

	_screen->loadBitmap("ICE.SHP", 3, 3, 0);
	shp = _screen->getCPagePtr(3);
	_numEffectShapes = READ_LE_UINT16(shp);
	_effectShapes = new uint8*[_numEffectShapes];
	for (int i = 0; i < _numEffectShapes; i++)
		_effectShapes[i] = _screen->makeShapeCopy(shp, i);

	_screen->loadBitmap("FIREBALL.SHP", 3, 3, 0);
	shp = _screen->getCPagePtr(3);
	_numFireballShapes = READ_LE_UINT16(shp);
	_fireballShapes = new uint8*[_numFireballShapes];
	for (int i = 0; i < _numFireballShapes; i++)
		_fireballShapes[i] = _screen->makeShapeCopy(shp, i);

	_screen->loadBitmap("HEAL.SHP", 3, 3, 0);
	shp = _screen->getCPagePtr(3);
	_numHealShapes = READ_LE_UINT16(shp);
	_healShapes = new uint8*[_numHealShapes];
	for (int i = 0; i < _numHealShapes; i++)
		_healShapes[i] = _screen->makeShapeCopy(shp, i);

	_screen->loadBitmap("HEALI.SHP", 3, 3, 0);
	shp = _screen->getCPagePtr(3);
	_numHealiShapes = READ_LE_UINT16(shp);
	_healiShapes = new uint8*[_numHealiShapes];
	for (int i = 0; i < _numHealiShapes; i++)
		_healiShapes[i] = _screen->makeShapeCopy(shp, i);

	memset(_itemsInPlay, 0, 400 * sizeof(LoLItem));
	for (int i = 0; i < 400; i++)
		_itemsInPlay[i].shpCurFrame_flg |= 0x8000;

	runInitScript("ONETIME.INF", 0);
	_emc->load("ITEM.INF", &_itemScript, &_opcodes);

	_transparencyTable1 = new uint8[256];
	_transparencyTable2 = new uint8[5120];

	_loadSuppFilesFlag = 1;

	_sound->loadSfxFile("LORESFX");

	setMouseCursorToItemInHand();
}

void LoLEngine::startupNew() {
	_selectedSpell = 0;
	_compassStep = 0;
	_compassDirection = _compassDirectionIndex = -1;

	_lastMouseRegion = -1;
	_currentLevel = 1;

	giveCredits(41, 0);
	_inventory[0] = makeItem(216, 0, 0);
	_inventory[1] = makeItem(217, 0, 0);
	_inventory[2] = makeItem(218, 0, 0);

	_availableSpells[0] = 0;
	setupScreenDims();

	Common::fill(_globalScriptVars2, ARRAYEND(_globalScriptVars2), 0x100);

	static const int selectIds[] = { -9, -1, -8, -5 };
	assert(_charSelection >= 0);
	assert(_charSelection < ARRAYSIZE(selectIds));
	addCharacter(selectIds[_charSelection]);

	gui_enableDefaultPlayfieldButtons();

	loadLevel(_currentLevel);

	_screen->showMouse();
}

void LoLEngine::runLoop() {
	// Initialize debugger since how it should be fully usable
	_debugger->initialize();

	enableSysTimer(2);

	_flagsTable[73] |= 0x08;

	while (!shouldQuit()) {
		if (_gameToLoad != -1) {
			// FIXME: Instead of throwing away the error returned by
			// loadGameState, we should use it / augment it.
			if (loadGameState(_gameToLoad).getCode() != Common::kNoError)
				error("Couldn't load game slot %d", _gameToLoad);
			_gameToLoad = -1;
		}

		if (_nextScriptFunc) {
			runLevelScript(_nextScriptFunc, 2);
			_nextScriptFunc = 0;
		}

		_timer->update();

		checkFloatingPointerRegions();
		gui_updateInput();

		update();

		if (_sceneUpdateRequired)
			gui_drawScene(0);
		else
			updateEnvironmentalSfx(0);

		if (_partyDamageFlags != -1) {
			checkForPartyDeath();
			_partyDamageFlags = -1;
		}

		delay(_tickLength);
	}
}

void LoLEngine::registerDefaultSettings() {
	KyraEngine_v1::registerDefaultSettings();

	// Most settings already have sensible defaults. This one, however, is
	// specific to the LoL engine.
	ConfMan.registerDefault("floating_cursors", false);
	ConfMan.registerDefault("smooth_scrolling", true);
	ConfMan.registerDefault("monster_difficulty", 1);
}

void LoLEngine::writeSettings() {
	ConfMan.setInt("monster_difficulty", _monsterDifficulty);
	ConfMan.setBool("floating_cursors", _floatingCursorsEnabled);
	ConfMan.setBool("smooth_scrolling", _smoothScrollingEnabled);

	switch (_lang) {
	case 1:
		_flags.lang = Common::FR_FRA;
		break;

	case 2:
		_flags.lang = Common::DE_DEU;
		break;

	case 0:
	default:
		if (_flags.platform == Common::kPlatformPC98)
			_flags.lang = Common::JA_JPN;
		else
			_flags.lang = Common::EN_ANY;
	}

	if (_flags.lang == _flags.replacedLang && _flags.fanLang != Common::UNK_LANG)
		_flags.lang = _flags.fanLang;

	ConfMan.set("language", Common::getLanguageCode(_flags.lang));

	KyraEngine_v1::writeSettings();
}

void LoLEngine::readSettings() {
	_monsterDifficulty = ConfMan.getInt("monster_difficulty");
	if (_monsterDifficulty < 0 || _monsterDifficulty > 2) {
		_monsterDifficulty = CLIP(_monsterDifficulty, 0, 2);
		warning("LoLEngine: Config file contains invalid difficulty setting.");
	}
	_smoothScrollingEnabled = ConfMan.getBool("smooth_scrolling");
	_floatingCursorsEnabled = ConfMan.getBool("floating_cursors");

	KyraEngine_v1::readSettings();
}

void LoLEngine::update() {
	updateSequenceBackgroundAnimations();

	if (_updateCharNum != -1 && _system->getMillis() > _updatePortraitNext)
		updatePortraitSpeechAnim();

	if (_flagsTable[31] & 0x08 || !(_updateFlags & 4))
		updateLampStatus();

	if (_flagsTable[31] & 0x40 && !(_updateFlags & 4) && (_compassDirection == -1 || (_currentDirection << 6) != _compassDirection || _compassStep))
		updateCompass();

	snd_updateCharacterSpeech();
	fadeText();

	updateInput();
	_screen->updateScreen();
}

#pragma mark - Localization

char *LoLEngine::getLangString(uint16 id) {
	if (id == 0xFFFF)
		return 0;

	uint16 realId = id & 0x3FFF;
	uint8 *buffer = 0;

	if (id & 0x4000)
		buffer = _landsFile;
	else
		buffer = _levelLangFile;

	if (!buffer)
		return 0;

	char *string = (char *)getTableEntry(buffer, realId);

	char *srcBuffer = _stringBuffer[_lastUsedStringBuffer];
	if (_flags.lang == Common::JA_JPN) {
		decodeSjis(string, srcBuffer);
	} else if (_flags.lang == Common::RU_RUS && !_flags.isTalkie) {
		decodeCyrillic(string, srcBuffer);
		Util::decodeString2(srcBuffer, srcBuffer);
	} else {
		Util::decodeString1(string, srcBuffer);
		Util::decodeString2(srcBuffer, srcBuffer);
	}

	++_lastUsedStringBuffer;
	_lastUsedStringBuffer %= ARRAYSIZE(_stringBuffer);

	return srcBuffer;
}

uint8 *LoLEngine::getTableEntry(uint8 *buffer, uint16 id) {
	if (!buffer)
		return 0;

	return buffer + READ_LE_UINT16(buffer + (id << 1));
}

void LoLEngine::decodeSjis(const char *src, char *dst) {
	char s[2];
	char d[3];
	s[1] = 0;

	uint8 cmd = 0;
	while ((cmd = *src++) != 0) {
		if (cmd == 27) {
			cmd = *src++ & 0x7f;
			memcpy(dst, src, cmd * 2);
			dst += cmd * 2;
			src += cmd * 2;
		} else {
			s[0] = cmd;
			int size = Util::decodeString1(s, d);
			memcpy(dst, d, size);
			dst += size;
		}
	}

	*dst = 0;
}

int LoLEngine::decodeCyrillic(const char *src, char *dst) {
	static const uint8 decodeTable1[] = {
		0x20, 0xAE, 0xA5, 0xA0, 0xE2, 0xAD, 0xA8, 0xE0, 0xE1, 0xAB, 0xA2,
		0xA4, 0xAC, 0xAA, 0xE3, 0x2E
	};

	static const uint8 decodeTable2[] = {
		0xAD, 0xAF, 0xA2, 0xE1, 0xAC, 0xAA, 0x20, 0xA4, 0xAB, 0x20,
		0xE0, 0xE2, 0xA4, 0xA2, 0xA6, 0xAA, 0x20, 0xAD, 0xE2, 0xE0,
		0xAB, 0xAC, 0xE1, 0xA1, 0x20, 0xAC, 0xE1, 0xAA, 0xAB, 0xE0,
		0xE2, 0xAD, 0xAE, 0xEC, 0xA8, 0xA5, 0xA0, 0x20, 0xE0, 0xEB,
		0xAE, 0xA0, 0xA8, 0xA5, 0xEB, 0xEF, 0x20, 0xE3, 0xE2, 0x20,
		0xAD, 0xE7, 0xAB, 0xAC, 0xA5, 0xE0, 0xAE, 0xA0, 0xA5, 0xA8,
		0xE3, 0xEB, 0xEF, 0xAA, 0xE2, 0xEF, 0xA5, 0xEC, 0xAB, 0xAE,
		0xAA, 0xAF, 0xA8, 0xA0, 0xA5, 0xEF, 0xAE, 0xEE, 0xEC, 0xE3,
		0xA0, 0xAE, 0xA5, 0xA8, 0xEB, 0x20, 0xE0, 0xE3, 0xA0, 0xA5,
		0xAE, 0xA8, 0xE3, 0xE1, 0xAD, 0xAB, 0x20, 0xAE, 0xA5, 0xA0,
		0xA8, 0xAD, 0x2E, 0xE3, 0xAE, 0xA0, 0xA8, 0x20, 0xE0, 0xE3,
		0xAB, 0xE1, 0x20, 0xA4, 0xAD, 0xE2, 0xA1, 0xA6, 0xAC, 0xE1,
		0x0D, 0x20, 0x2E, 0x09, 0xA0, 0xA1, 0x9D, 0xA5
	};

	int size = 0;
	uint cChar = 0;
	while ((cChar = *src++) != 0) {
		if (cChar & 0x80) {
			cChar &= 0x7F;
			int index = (cChar & 0x78) >> 3;
			*dst++ = decodeTable1[index];
			++size;
			assert(cChar < sizeof(decodeTable2));
			cChar = decodeTable2[cChar];
		} else if (cChar >= 0x70) {
			cChar = *src++;
		} else if (cChar >= 0x30) {
			if (cChar < 0x60)
				cChar -= 0x30;
			cChar |= 0x80;
		}

		*dst++ = cChar;
		++size;
	}

	*dst++ = 0;
	return size;
}

bool LoLEngine::addCharacter(int id) {
	const uint16 *cdf[] = {
		_charDefsMan, _charDefsMan, _charDefsMan, _charDefsWoman,
		_charDefsMan, _charDefsMan, _charDefsWoman, _charDefsKieran, _charDefsAkshel
	};

	int numChars = countActiveCharacters();
	if (numChars == 4)
		return false;

	int i = 0;
	for (; i < _charDefaultsSize; i++) {
		if (_charDefaults[i].id == id) {
			memcpy(&_characters[numChars], &_charDefaults[i], sizeof(LoLCharacter));
			_characters[numChars].defaultModifiers = cdf[i];
			break;
		}
	}
	if (i == _charDefaultsSize)
		return false;

	loadCharFaceShapes(numChars, id);

	_characters[numChars].nextAnimUpdateCountdown = rollDice(1, 12) + 6;

	for (i = 0; i < 11; i++) {
		if (_characters[numChars].items[i]) {
			_characters[numChars].items[i] = makeItem(_characters[numChars].items[i], 0, 0);
			runItemScript(numChars, _characters[numChars].items[i], 0x80, 0, 0);
		}
	}

	calcCharPortraitXpos();
	if (numChars > 0)
		setTemporaryFaceFrame(numChars, 2, 6, 0);

	return true;
}

void LoLEngine::setTemporaryFaceFrame(int charNum, int frame, int updateDelay, int redraw) {
	_characters[charNum].tempFaceFrame = frame;
	if (frame || updateDelay)
		setCharacterUpdateEvent(charNum, 6, updateDelay, 1);
	if (redraw)
		gui_drawCharPortraitWithStats(charNum);
}

void LoLEngine::setTemporaryFaceFrameForAllCharacters(int frame, int updateDelay, int redraw) {
	for (int i = 0; i < 4; i++)
		setTemporaryFaceFrame(i, frame, updateDelay, 0);
	if (redraw)
		gui_drawAllCharPortraitsWithStats();
}

void LoLEngine::setCharacterUpdateEvent(int charNum, int updateType, int updateDelay, int overwrite) {
	LoLCharacter *l = &_characters[charNum];
	for (int i = 0; i < 5; i++) {
		if (l->characterUpdateEvents[i] && (!overwrite || l->characterUpdateEvents[i] != updateType))
			continue;

		l->characterUpdateEvents[i] = updateType;
		l->characterUpdateDelay[i] = updateDelay;
		_timer->setNextRun(3, _system->getMillis());
		_timer->resetNextRun();
		_timer->enable(3);
		break;
	}
}

int LoLEngine::countActiveCharacters() {
	int i = 0;
	while (_characters[i].flags & 1)
		i++;
	return i;
}

void LoLEngine::loadCharFaceShapes(int charNum, int id) {
	if (id < 0)
		id = -id;

	Common::String file = Common::String::format("FACE%02d.SHP", id);
	_screen->loadBitmap(file.c_str(), 3, 3, 0);

	const uint8 *p = _screen->getCPagePtr(3);
	for (int i = 0; i < 40; i++) {
		delete[] _characterFaceShapes[i][charNum];
		_characterFaceShapes[i][charNum] = _screen->makeShapeCopy(p, i);
	}
}

void LoLEngine::updatePortraitSpeechAnim() {
	int x = 0;
	int y = 0;
	bool redraw = false;

	if (_portraitSpeechAnimMode == 0) {
		x = _activeCharsXpos[_updateCharNum];
		y = 144;
		redraw = true;
	} else if (_portraitSpeechAnimMode == 1) {
		if (textEnabled()) {
			x = 90;
			y = 130;
		} else {
			x = _activeCharsXpos[_updateCharNum];
			y = 144;
		}
	} else if (_portraitSpeechAnimMode == 2) {
		if (textEnabled()) {
			x = 16;
			y = 134;
		} else {
			x = _activeCharsXpos[_updateCharNum] + 10;
			y = 145;
		}
	}

	int f = rollDice(1, 6) - 1;
	if (f == _characters[_updateCharNum].curFaceFrame)
		f++;
	if (f > 5)
		f -= 5;
	f += 7;

	if (speechEnabled()) {
		if (snd_updateCharacterSpeech() == 2)
			// WORKAROUND for portrait speech animations which would "freeze" in some situations
			if (_resetPortraitAfterSpeechAnim == 2)
				_resetPortraitAfterSpeechAnim = 1;
			else
				_updatePortraitSpeechAnimDuration = 2;
		else
			_updatePortraitSpeechAnimDuration = 1;
	} else if (_resetPortraitAfterSpeechAnim == 2) {
		_resetPortraitAfterSpeechAnim = 1;
	}

	_updatePortraitSpeechAnimDuration--;

	if (_updatePortraitSpeechAnimDuration) {
		setCharFaceFrame(_updateCharNum, f);
		if (redraw)
			gui_drawCharPortraitWithStats(_updateCharNum);
		else
			gui_drawCharFaceShape(_updateCharNum, x, y, 0);
		_updatePortraitNext = _system->getMillis() + 10 * _tickLength;
	} else if (_resetPortraitAfterSpeechAnim != 0) {
		faceFrameRefresh(_updateCharNum);
		if (redraw) {
			gui_drawCharPortraitWithStats(_updateCharNum);
			initTextFading(0, 0);
		} else {
			gui_drawCharFaceShape(_updateCharNum, x, y, 0);
		}
		_updateCharNum = -1;
	}
}

void LoLEngine::stopPortraitSpeechAnim() {
	if (_updateCharNum == -1)
		return;

	_updatePortraitSpeechAnimDuration = 1;
	// WORKAROUND for portrait speech animations which would "freeze" in some situations
	_resetPortraitAfterSpeechAnim = 2;
	updatePortraitSpeechAnim();
	_updatePortraitSpeechAnimDuration = 1;
	_updateCharNum = -1;

	if (!_portraitSpeechAnimMode)
		initTextFading(0, 0);
}

void LoLEngine::initTextFading(int textType, int clearField) {
	if (_textColorFlag == textType || !textType) {
		_fadeText = true;
		_palUpdateTimer = _system->getMillis();
	}

	if (!clearField)
		return;

	stopPortraitSpeechAnim();
	if (_needSceneRestore)
		_screen->setScreenDim(_txt->clearDim(3));

	_fadeText = false;
	_timer->disable(11);
}

void LoLEngine::setCharFaceFrame(int charNum, int frameNum) {
	_characters[charNum].curFaceFrame = frameNum;
}

void LoLEngine::faceFrameRefresh(int charNum) {
	if (_characters[charNum].curFaceFrame == 1)
		setTemporaryFaceFrame(charNum, 0, 0, 0);
	else if (_characters[charNum].curFaceFrame == 6)
		if (_characters[charNum].tempFaceFrame != 5)
			setTemporaryFaceFrame(charNum, 0, 0, 0);
		else
			_characters[charNum].curFaceFrame = 5;
	else
		_characters[charNum].curFaceFrame = 0;
}

void LoLEngine::recalcCharacterStats(int charNum) {
	for (int i = 0; i < 5; i++)
		_charStatsTemp[i] = calculateCharacterStats(charNum, i);
}

int LoLEngine::calculateCharacterStats(int charNum, int index) {
	if (index == 0) {
		// Might
		int c = 0;
		for (int i = 0; i < 8; i++)
			c += _characters[charNum].itemsMight[i];
		if (c)
			c += _characters[charNum].might;
		else
			c = _characters[charNum].defaultModifiers[8];

		c = (c * _characters[charNum].defaultModifiers[1]) >> 8;
		c = (c * _characters[charNum].totalMightModifier) >> 8;

		return c;

	} else if (index == 1) {
		// Protection
		return calculateProtection(charNum);

	} else if (index > 4) {
		return -1;

	} else {
		// Fighter
		// Rogue
		// Mage
		index -= 2;
		return _characters[charNum].skillLevels[index] + _characters[charNum].skillModifiers[index];
	}

	//return 1;
}

int LoLEngine::calculateProtection(int index) {
	int c = 0;
	if (index & 0x8000) {
		// Monster
		index &= 0x7fff;
		c = (_monsters[index].properties->itemProtection * _monsters[index].properties->fightingStats[2]) >> 8;
	} else {
		// Character
		c = _characters[index].itemProtection + _characters[index].protection;
		c = (c * _characters[index].defaultModifiers[2]) >> 8;
		c = (c * _characters[index].totalProtectionModifier) >> 8;
	}

	return c;
}

void LoLEngine::setCharacterMagicOrHitPoints(int charNum, int type, int points, int mode) {
	static const uint16 barData[4][5] = {
		// xPos, bar color, text color, flag, string id
		{ 0x27, 0x9A, 0x98, 0x01, 0x4254 },
		{ 0x21, 0xA2, 0xA0, 0x00, 0x4253 },
		// 16 color mode
		{ 0x27, 0x66, 0x55, 0x01, 0x4254 },
		{ 0x21, 0xAA, 0x99, 0x00, 0x4253 }
	};

	if (charNum > 3)
		return;

	LoLCharacter *c = &_characters[charNum];
	if (!(c->flags & 1))
		return;

	int pointsMax = type ? c->magicPointsMax : c->hitPointsMax;
	int pointsCur = type ? c->magicPointsCur : c->hitPointsCur;

	int newVal = (mode == 2) ? (pointsMax + points) : (mode ? (pointsCur + points) : points);
	newVal = CLIP(newVal, 0, pointsMax);

	if (type) {
		c->magicPointsCur = newVal;
	} else {
		c->hitPointsCur = newVal;
		if (c->hitPointsCur < 1)
			c->flags |= 8;
	}

	if (_updateFlags & 2)
		return;

	Screen::FontId cf = _screen->setFont(Screen::FID_6_FNT);
	int cp = _screen->setCurPage(0);

	int s = 8192 / pointsMax;
	pointsMax = (s * pointsMax) >> 8;
	pointsCur = (s * pointsCur) >> 8;
	newVal = (s * newVal) >> 8;
	int newValScl = CLIP(newVal, 0, pointsMax);

	int step = (newVal > pointsCur) ? 2 : -2;
	newVal = CLIP(newVal + step, 0, pointsMax);

	if (_flags.use16ColorMode)
		type += 2;

	if (newVal != pointsCur) {
		step = (newVal >= pointsCur) ? 2 : -2;

		for (int i = pointsCur; i != newVal || newVal != newValScl;) {
			if (ABS(i - newVal) < ABS(step))
				step >>= 1;

			i += step;

			uint32 delayTimer = _system->getMillis() + _tickLength;

			gui_drawLiveMagicBar(barData[type][0] + _activeCharsXpos[charNum], 175, i, 0, pointsMax, 5, 32, barData[type][1], _flags.use16ColorMode ? 0x44 : 1, barData[type][3]);
			_screen->printText(getLangString(barData[type][4]), barData[type][0] + _activeCharsXpos[charNum], 144, barData[type][2], 0);
			_screen->updateScreen();

			if (i == newVal) {
				newVal = newValScl;
				step = -step;
			}

			delayUntil(delayTimer);
		}
	}

	_screen->setFont(cf);
	_screen->setCurPage(cp);
}

void LoLEngine::increaseExperience(int charNum, int skill, uint32 points) {
	if (charNum & 0x8000)
		return;

	if (_characters[charNum].flags & 8)
		return;

	_characters[charNum].experiencePts[skill] += points;

	bool loop = true;
	while (loop) {
		if (_characters[charNum].experiencePts[skill] < _expRequirements[_characters[charNum].skillLevels[skill]])
			break;

		_characters[charNum].skillLevels[skill]++;
		_characters[charNum].flags |= (0x200 << skill);
		int inc = 0;

		switch (skill) {
		case 0:
			_txt->printMessage(0x8003, getLangString(0x4023), _characters[charNum].name);
			inc = rollDice(4, 6);
			_characters[charNum].hitPointsCur += inc;
			_characters[charNum].hitPointsMax += inc;
			break;

		case 1:
			_txt->printMessage(0x8003, getLangString(0x4025), _characters[charNum].name);
			inc = rollDice(2, 6);
			_characters[charNum].hitPointsCur += inc;
			_characters[charNum].hitPointsMax += inc;
			break;

		case 2:
			_txt->printMessage(0x8003, getLangString(0x4024), _characters[charNum].name);
			inc = (_characters[charNum].defaultModifiers[6] * (rollDice(1, 8) + 17)) >> 8;
			_characters[charNum].magicPointsCur += inc;
			_characters[charNum].magicPointsMax += inc;
			inc = rollDice(1, 6);
			_characters[charNum].hitPointsCur += inc;
			_characters[charNum].hitPointsMax += inc;
			break;

		default:
			break;
		}

		snd_playSoundEffect(118, -1);
		gui_drawCharPortraitWithStats(charNum);
	}
}

void LoLEngine::increaseCharacterHitpoints(int charNum, int points, bool ignoreDeath) {
	if (_characters[charNum].hitPointsCur <= 0 && !ignoreDeath)
		return;

	if (points <= 1)
		points = 1;

	_characters[charNum].hitPointsCur = CLIP<int16>(_characters[charNum].hitPointsCur + points, 1, _characters[charNum].hitPointsMax);
	_characters[charNum].flags &= 0xfff7;
}

void LoLEngine::setupScreenDims() {
	if (textEnabled()) {
		_screen->modifyScreenDim(4, 11, 124, 28, 45);
		_screen->modifyScreenDim(5, 85, 123, 233, 54);
	} else {
		_screen->modifyScreenDim(4, 11, 124, 28, 9);
		_screen->modifyScreenDim(5, 85, 123, 233, 18);
	}
}

void LoLEngine::initSceneWindowDialogue(int controlMode) {
	resetPortraitsAndDisableSysTimer();
	gui_prepareForSequence(112, 0, 176, 120, controlMode);

	_updateFlags |= 3;

	_txt->setupField(true);
	_txt->expandField();
	setupScreenDims();
	gui_disableControls(controlMode);
}

void LoLEngine::toggleSelectedCharacterFrame(bool mode) {
	if (countActiveCharacters() == 1)
		return;

	int col = mode ? 212 : 1;

	int cp = _screen->setCurPage(0);
	int x = _activeCharsXpos[_selectedCharacter];

	_screen->drawBox(x, 143, x + 65, 176, col);
	_screen->setCurPage(cp);
}

void LoLEngine::gui_prepareForSequence(int x, int y, int w, int h, int buttonFlags) {
	setSequenceButtons(x, y, w, h, buttonFlags);

	_seqWindowX1 = x;
	_seqWindowY1 = y;
	_seqWindowX2 = x + w;
	_seqWindowY2 = y + h;

	int mouseOffs = _itemInHand ? 10 : 0;
	_screen->setMouseCursor(mouseOffs, mouseOffs, getItemIconShapePtr(_itemInHand));

	_lastMouseRegion = -1;

	if (w == 320) {
		setLampMode(false);
		_lampStatusSuspended = true;
	}
}

void LoLEngine::gui_specialSceneSuspendControls(int controlMode) {
	if (controlMode) {
		_updateFlags |= 4;
		setLampMode(false);
	}
	_updateFlags |= 1;
	_specialSceneFlag = 1;
	_currentControlMode = controlMode;
	calcCharPortraitXpos();
	checkFloatingPointerRegions();
}

void LoLEngine::gui_specialSceneRestoreControls(int restoreLamp) {
	if (restoreLamp) {
		_updateFlags &= 0xfffa;
		resetLampStatus();
	}
	_updateFlags &= 0xfffe;
	_specialSceneFlag = 0;
	checkFloatingPointerRegions();
}

void LoLEngine::restoreAfterSceneWindowDialogue(int redraw) {
	gui_enableControls();
	_txt->setupField(false);
	_updateFlags &= 0xffdf;

	setDefaultButtonState();

	for (int i = 0; i < 6; i++)
		_tim->freeAnimStruct(i);

	_updateFlags = 0;

	if (redraw) {
		if (_screen->_fadeFlag != 2)
			_screen->fadeClearSceneWindow(10);
		gui_drawPlayField();
		setPaletteBrightness(_screen->getPalette(0), _brightness, _lampEffect);
		_screen->_fadeFlag = 0;
	}

	_needSceneRestore = 0;
	enableSysTimer(2);
}

void LoLEngine::initDialogueSequence(int controlMode, int pageNum) {
	if (controlMode) {
		_timer->disable(11);
		_fadeText = false;
		int cp = _screen->setCurPage(pageNum);

		if (_flags.use16ColorMode) {
			_screen->fillRect(0, 128, 319, 199, 0x44);
			gui_drawBox(0, 129, 320, 71, 0xee, 0xcc, -1);
			gui_drawBox(1, 130, 318, 69, 0xee, 0xcc, 0x11);
		} else {
			_screen->fillRect(0, 128, 319, 199, 1);
			gui_drawBox(0, 129, 320, 71, 136, 251, -1);
			gui_drawBox(1, 130, 318, 69, 136, 251, 252);
		}

		_screen->modifyScreenDim(5, 8, 131, 306, 66);
		_screen->modifyScreenDim(4, 1, 133, 38, 60);
		_txt->clearDim(4);

		_updateFlags |= 2;
		_currentControlMode = controlMode;
		calcCharPortraitXpos();

		if (!textEnabled() && (!(controlMode & 2))) {
			int nc = countActiveCharacters();
			for (int i = 0; i < nc; i++) {
				_portraitSpeechAnimMode = 2;
				_updateCharNum = i;
				_screen->drawShape(0, _gameShapes[88], _activeCharsXpos[_updateCharNum] + 8, 142, 0, 0);
				stopPortraitSpeechAnim();
			}
		}

		_screen->setCurPage(cp);

	} else {
		_txt->setupField(true);
		_txt->expandField();
		setupScreenDims();
		_txt->clearDim(4);
	}

	_currentControlMode = controlMode;
	_dialogueField = true;
}

void LoLEngine::restoreAfterDialogueSequence(int controlMode) {
	if (!_dialogueField)
		return;

	stopPortraitSpeechAnim();
	_currentControlMode = controlMode;
	calcCharPortraitXpos();

	if (_currentControlMode) {
		_screen->modifyScreenDim(4, 11, 124, 28, 45);
		_screen->modifyScreenDim(5, 85, 123, 233, 54);
		_updateFlags &= 0xfffd;
	} else {
		const ScreenDim *d = _screen->getScreenDim(5);
		_screen->fillRect(d->sx, d->sy, d->sx + d->w - (_flags.use16ColorMode ? 3 : 2), d->sy + d->h - 2, d->unkA);
		_txt->clearDim(4);
		_txt->setupField(false);
	}

	_dialogueField = false;
}

void LoLEngine::resetPortraitsAndDisableSysTimer() {
	_needSceneRestore = 1;
	if (!textEnabled() || (!(_currentControlMode & 2)))
		timerUpdatePortraitAnimations(1);

	disableSysTimer(2);
}

void LoLEngine::fadeText() {
	if (!_fadeText)
		return;

	if (_screen->fadeColor(192, 252, (_system->getMillis() - _palUpdateTimer) / _tickLength, 60))
		return;

	if (_needSceneRestore)
		return;

	_screen->setScreenDim(_txt->clearDim(3));

	_timer->disable(11);

	_fadeText = false;
}

void LoLEngine::setPaletteBrightness(const Palette &srcPal, int brightness, int modifier) {
	generateBrightnessPalette(srcPal, _screen->getPalette(1), brightness, modifier);
	_screen->fadePalette(_screen->getPalette(1), 5, 0);
	_screen->_fadeFlag = 0;
}

void LoLEngine::generateBrightnessPalette(const Palette &src, Palette &dst, int brightness, int16 modifier) {
	dst.copy(src);
	if (_flags.use16ColorMode) {
		if (!brightness)
			modifier = 0;
		else if (modifier < 0 || modifier > 7 || !(_flagsTable[31] & 0x08))
			modifier = 8;

		modifier >>= 1;
		if (modifier)
			modifier--;
		if (modifier > 3)
			modifier = 3;
		_blockBrightness = modifier << 4;
		_sceneUpdateRequired = true;

	} else {
		_screen->loadSpecialColors(dst);

		brightness = (8 - brightness) << 5;
		if (modifier >= 0 && modifier < 8 && (_flagsTable[31] & 0x08)) {
			brightness = 256 - ((((modifier & 0xfffe) << 5) * (256 - brightness)) >> 8);
			if (brightness < 0)
				brightness = 0;
		}

		for (int i = 0; i < 384; i++) {
			uint16 c = (dst[i] * brightness) >> 8;
			dst[i] = c & 0xff;
		}
	}
}

void LoLEngine::generateFlashPalette(const Palette &src, Palette &dst, int colorFlags) {
	dst.copy(src, 0, 2);

	for (int i = 2; i < 128; i++) {
		for (int ii = 0; ii < 3; ii++) {
			uint8 t = src[i * 3 + ii] & 0x3f;
			if (colorFlags & (1 << ii))
				t += ((0x3f - t) >> 1);
			else
				t -= (t >> 1);
			dst[i * 3 + ii] = t;
		}
	}

	dst.copy(src, 128);
}

void LoLEngine::createTransparencyTables() {
	if (_flags.isTalkie || _loadSuppFilesFlag)
		return;

	uint8 *tpal = new uint8[768];

	if (_flags.use16ColorMode) {
		static const uint8 colTbl[] = {
			0x00, 0x00, 0x11, 0x00, 0x22, 0x00, 0x33, 0x00, 0x44, 0x00, 0x55, 0x00, 0x66, 0x00, 0x77, 0x00,
			0x88, 0x00, 0x99, 0x00, 0xAA, 0x00, 0xBB, 0x00, 0xCC, 0x00, 0xDD, 0x00, 0xEE, 0x00, 0xFF, 0x00
		};

		memset(tpal, 0xff, 768);
		_res->loadFileToBuf("LOL.NOL", tpal, 48);

		for (int i = 15; i > -1; i--) {
			int s = colTbl[i << 1] * 3;
			tpal[s] = tpal[i * 3];
			tpal[s + 1] = tpal[i * 3 + 1];
			tpal[s + 2] = tpal[i * 3 + 2];
			tpal[i * 3 + 2] = tpal[i * 3 + 1] = tpal[i * 3] = 0xff;
		}

		_screen->createTransparencyTablesIntern(colTbl, 16, tpal, tpal,  _transparencyTable1, _transparencyTable2, 80);

	} else {
		_res->loadFileToBuf("fxpal.col", tpal, 768);
		_screen->loadBitmap("fxpal.shp", 3, 3, 0);
		const uint8 *shpPal = _screen->getPtrToShape(_screen->getCPagePtr(2), 0) + 11;

		_screen->createTransparencyTablesIntern(shpPal, 20, tpal, _screen->getPalette(1).getData(), _transparencyTable1, _transparencyTable2, 70);
	}

	delete[] tpal;
	_loadSuppFilesFlag = 1;
}

void LoLEngine::updateSequenceBackgroundAnimations() {
	if (_updateFlags & 8 || !_tim)
		return;
	if (!_tim->animator())
		return;

	for (int i = 0; i < 6; i++)
		_tim->animator()->update(i);
}

void LoLEngine::loadTalkFile(int index) {
	if (index == _curTlkFile)
		return;

	if (_curTlkFile > 0 && index > 0)
		_res->unloadPakFile(Common::String::format("%02d.TLK", _curTlkFile));

	if (index > 0)
		_curTlkFile = index;

	_res->loadPakFile(Common::String::format("%02d.TLK", index));
}

int LoLEngine::characterSays(int track, int charId, bool redraw) {
	if (charId == 1) {
		charId = _selectedCharacter;
	} if (charId <= 0) {
		charId = 0;
	} else {
		int i = 0;
		for (; i < 4; i++) {
			if (charId != _characters[i].id || !(_characters[i].flags & 1))
				continue;
			charId = i;
			break;
		}

		if (i == 4)
			return 0;
	}

	bool r = snd_playCharacterSpeech(track, charId, 0);

	if (r && redraw) {
		stopPortraitSpeechAnim();
		_updateCharNum = charId;
		_portraitSpeechAnimMode = 0;
		_resetPortraitAfterSpeechAnim = 1;
		_fadeText = false;
		updatePortraitSpeechAnim();
	}

	return r ? (textEnabled() ? 1 : 0) : 1;
}

int LoLEngine::playCharacterScriptChat(int charId, int mode, int restorePortrait, char *str, EMCState *script, const uint16 *paramList, int16 paramIndex) {
	int ch = 0;
	bool skipAnim = false;

	if ((charId == -1) || (!(charId & 0x70)))
		charId = ch = (charId == 1) ? (_selectedCharacter ? _characters[_selectedCharacter].id : 0) : charId;
	else
		charId ^= 0x70;

	stopPortraitSpeechAnim();

	if (charId < 0) {
		charId = ch = _rnd.getRandomNumber(countActiveCharacters() - 1);
	} else if (charId > 0) {
		int i = 0;

		for (; i < 4; i++) {
			if (_characters[i].id != charId || !(_characters[i].flags & 1))
				continue;
			if (charId == ch)
				ch = i;
			charId = i;
			break;
		}

		if (i == 4) {
			if (charId == 8)
				skipAnim = true;
			else
				return 0;
		}
	}

	if (!skipAnim) {
		_updateCharNum = charId;
		_portraitSpeechAnimMode = mode;
		_updatePortraitSpeechAnimDuration = strlen(str) >> 1;
		_resetPortraitAfterSpeechAnim = restorePortrait;
	}

	if (script)
		snd_playCharacterSpeech(script->stack[script->sp + 2], ch, 0);
	else if (paramList)
		snd_playCharacterSpeech(paramList[2], ch, 0);

	if (textEnabled()) {
		if (mode == 0) {
			_txt->printDialogueText(3, str, script, paramList, paramIndex);

		} else if (mode == 1) {
			_txt->clearDim(4);
			_screen->modifyScreenDim(4, 16, 123, 23, 47);
			_txt->printDialogueText(4, str, script, paramList, paramIndex);
			_screen->modifyScreenDim(4, 11, 123, 28, 47);

		} else if (mode == 2) {
			_txt->clearDim(4);
			_screen->modifyScreenDim(4, 9, 133, 30, 60);
			_txt->printDialogueText(4, str, script, paramList, 3);
			_screen->modifyScreenDim(4, 1, 133, 37, 60);
		}
	}

	_fadeText = false;
	if (!skipAnim)
		updatePortraitSpeechAnim();

	return 1;
}

void LoLEngine::setupDialogueButtons(int numStr, const char *s1, const char *s2, const char *s3) {
	screen()->setScreenDim(5);

	if (numStr == 1 && speechEnabled()) {
		_dialogueNumButtons = 0;
		_dialogueButtonString[0] = _dialogueButtonString[1] = _dialogueButtonString[2] = 0;
	} else {
		_dialogueNumButtons = numStr;
		_dialogueButtonString[0] = s1;
		_dialogueButtonString[1] = s2;
		_dialogueButtonString[2] = s3;
		_dialogueHighlightedButton = 0;

		const ScreenDim *d = screen()->getScreenDim(5);

		static uint16 posX[3];
		static uint8 posY[3];

		memset(posY, d->sy + d->h - 9, 3);

		_dialogueButtonPosX = posX;
		_dialogueButtonPosY = posY;

		if (numStr == 1) {
			posX[0] = posX[1] = posX[2] = d->sx + d->w - (_dialogueButtonWidth + 3);
		} else {
			int xOffs = d->w / numStr;
			posX[0] = d->sx + (xOffs >> 1) - 37;
			posX[1] = posX[0] + xOffs;
			posX[2] = posX[1] + xOffs;
		}

		drawDialogueButtons();
	}

	if (!shouldQuit())
		removeInputTop();
}

void LoLEngine::giveItemToMonster(LoLMonster *monster, Item item) {
	uint16 *c = &monster->assignedItems;
	while (*c)
		c = &_itemsInPlay[*c].nextAssignedObject;
	*c = (uint16)item;
	_itemsInPlay[item].nextAssignedObject = 0;
}

const uint16 *LoLEngine::getCharacterOrMonsterStats(int id) {
	return (id & 0x8000) ? (const uint16 *)_monsters[id & 0x7fff].properties->fightingStats : _characters[id].defaultModifiers;
}

uint16 *LoLEngine::getCharacterOrMonsterItemsMight(int id) {
	return (id & 0x8000) ? _monsters[id & 0x7fff].properties->itemsMight : _characters[id].itemsMight;
}

uint16 *LoLEngine::getCharacterOrMonsterProtectionAgainstItems(int id) {
	return (id & 0x8000) ? _monsters[id & 0x7fff].properties->protectionAgainstItems : _characters[id].protectionAgainstItems;
}

void LoLEngine::delay(uint32 millis, bool doUpdate, bool) {
	while (millis && !shouldQuit()) {
		if (doUpdate)
			update();
		else
			updateInput();

		uint32 step = MIN<uint32>(millis, _tickLength);
		_system->delayMillis(step);
		millis -= step;
	}
}

const KyraRpgGUISettings *LoLEngine::guiSettings() {
	return &_guiSettings;
}

// spells

int LoLEngine::castSpell(int charNum, int spellType, int spellLevel) {
	_activeSpell.charNum = charNum;
	_activeSpell.spell = spellType;
	_activeSpell.p = &_spellProperties[spellType];

	_activeSpell.level = ABS(spellLevel);

	if ((_spellProperties[spellType].flags & 0x100) && testWallFlag(calcNewBlockPosition(_currentBlock, _currentDirection), _currentDirection, 1)) {
		_txt->printMessage(2, "%s", getLangString(0x4257));
		return 0;
	}

	if (charNum < 0) {
		_activeSpell.charNum = (charNum * -1) - 1;
		if (_spellProcs[spellType]->isValid())
			return (*_spellProcs[spellType])(&_activeSpell);
	} else {
		if (_activeSpell.p->mpRequired[spellLevel] > _characters[charNum].magicPointsCur)
			return 0;

		if (_activeSpell.p->hpRequired[spellLevel] >= _characters[charNum].hitPointsCur)
			return 0;

		setCharacterMagicOrHitPoints(charNum, 1, -_activeSpell.p->mpRequired[spellLevel], 1);
		setCharacterMagicOrHitPoints(charNum, 0, -_activeSpell.p->hpRequired[spellLevel], 1);
		gui_drawCharPortraitWithStats(charNum);

		if (_spellProcs[spellType]->isValid())
			(*_spellProcs[spellType])(&_activeSpell);
	}

	return 1;
}

int LoLEngine::castSpark(ActiveSpell *a) {
	processMagicSpark(a->charNum, a->level);
	return 1;
}

int LoLEngine::castHeal(ActiveSpell *a) {
	if (a->level < 3)
		processMagicHealSelectTarget();
	else
		processMagicHeal(-1, a->level);

	return 1;
}

int LoLEngine::castIce(ActiveSpell *a) {
	processMagicIce(a->charNum, a->level);
	return 1;
}

int LoLEngine::castFireball(ActiveSpell *a) {
	processMagicFireball(a->charNum, a->level);
	return 1;
}

int LoLEngine::castHandOfFate(ActiveSpell *a) {
	processMagicHandOfFate(a->level);
	return 1;
}

int LoLEngine::castMistOfDoom(ActiveSpell *a) {
	processMagicMistOfDoom(a->charNum, a->level);
	return 1;
}

int LoLEngine::castLightning(ActiveSpell *a) {
	processMagicLightning(a->charNum, a->level);
	return 1;
}

int LoLEngine::castFog(ActiveSpell *a) {
	processMagicFog();
	return 1;
}

int LoLEngine::castSwarm(ActiveSpell *a) {
	processMagicSwarm(a->charNum, 10);
	return 1;
}

int LoLEngine::castVaelansCube(ActiveSpell *a) {
	return processMagicVaelansCube();
}

int LoLEngine::castGuardian(ActiveSpell *a) {
	return processMagicGuardian(a->charNum);
}

int LoLEngine::castHealOnSingleCharacter(ActiveSpell *a) {
	processMagicHeal(a->target, a->level);
	return 1;
}

int LoLEngine::processMagicSpark(int charNum, int spellLevel) {
	WSAMovie_v2 *mov = new WSAMovie_v2(this);
	_screen->copyPage(0, 12);

	mov->open("spark1.wsa", 0, 0);
	if (!mov->opened())
		error("SPARK: Unable to load SPARK1.WSA");
	snd_playSoundEffect(72, -1);
	playSpellAnimation(mov, 0, 7, 4, _activeCharsXpos[charNum] - 2, 138, 0, 0, 0, 0, false);
	mov->close();

	_screen->copyPage(12, 0);
	_screen->updateScreen();

	uint16 targetBlock = 0;
	int dist = getSpellTargetBlock(_currentBlock, _currentDirection, 4, targetBlock);
	uint16 target = getNearestMonsterFromCharacterForBlock(targetBlock, charNum);

	static const uint8 dmg[] = { 7, 15, 25, 60 };
	if (target != 0xffff) {
		inflictMagicalDamage(target, charNum, dmg[spellLevel], 5, 0);
		updateDrawPage2();
		gui_drawScene(0);
		_screen->copyPage(0, 12);
	}

	int numFrames = mov->open("spark2.wsa", 0, 0);
	if (!mov->opened())
		error("SPARK: Unable to load SPARK2.WSA");

	uint16 wX[6];
	uint16 wY[6];
	uint16 wFrames[6];
	const uint16 width = mov->width();
	const uint16 height = mov->height();

	for (int i = 0; i < 6; i++) {
		wX[i] = (_rnd.getRandomNumber(0x7fff) % 64) + ((176 - width) >> 1) + 80;
		wY[i] = (_rnd.getRandomNumber(0x7fff) % 32) + ((120 - height) >> 1) - 16;
		wFrames[i] = i << 1;
	}

	for (int i = 0, d = ((spellLevel << 1) + 12); i < d; i++) {
		uint32 delayTimer = _system->getMillis() + 4 * _tickLength;
		_screen->copyPage(12, 2);

		for (int ii = 0; ii <= spellLevel; ii++) {
			if (wFrames[ii] >= i || wFrames[ii] + 13 <= i)
				continue;

			if ((i - wFrames[ii]) == 1)
				snd_playSoundEffect(162, -1);

			mov->displayFrame(((i - wFrames[ii]) + (dist << 4)) % numFrames, 2, wX[ii], wY[ii], 0x5000, _transparencyTable1, _transparencyTable2);
			_screen->copyRegion(wX[ii], wY[ii], wX[ii], wY[ii], width, height, 2, 0, Screen::CR_NO_P_CHECK);
			_screen->updateScreen();
		}

		if (i < d - 1)
			delayUntil(delayTimer);
	}

	mov->close();

	_screen->copyPage(12, 2);
	updateDrawPage2();

	_sceneUpdateRequired = true;

	delete mov;
	return 1;
}

int LoLEngine::processMagicHealSelectTarget() {
	_txt->printMessage(0, "%s", getLangString(0x4040));
	gui_resetButtonList();
	gui_setFaceFramesControlButtons(81, 0);
	gui_initButtonsFromList(_buttonList8);
	return 1;
}

int LoLEngine::processMagicHeal(int charNum, int spellLevel) {
	if (!_healOverlay) {
		_healOverlay = new uint8[256];
		Palette tpal(256);
		tpal.copy(_screen->getPalette(1));

		if (_flags.use16ColorMode) {
			tpal.fill(16, 240, 0xff);
			uint8 *dst = tpal.getData();
			for (int i = 1; i < 16; i++) {
				int s = ((i << 4) | i) * 3;
				SWAP(dst[s], dst[i]);
				SWAP(dst[s + 1], dst[i + 1]);
				SWAP(dst[s + 2], dst[i + 2]);
			}
		}

		_screen->generateGrayOverlay(tpal, _healOverlay, 52, 22, 20, 0, 256, true);
	}

	const uint8 *healShpFrames = 0;
	const uint8 *healiShpFrames = 0;
	bool curePoison = false;
	int points = 0;

	if (spellLevel == 0) {
		points = 25;
		healShpFrames = _healShapeFrames;
		healiShpFrames = _healShapeFrames + 32;

	} else if (spellLevel == 1) {
		points = 45;
		healShpFrames = _healShapeFrames + 16;
		healiShpFrames = _healShapeFrames + 48;

	} else if (spellLevel > 3) {
		curePoison = true;
		points = spellLevel;
		healShpFrames = _healShapeFrames + 16;
		healiShpFrames = _healShapeFrames + 64;

	} else {
		curePoison = true;
		points = 10000;
		healShpFrames = _healShapeFrames + 16;
		healiShpFrames = _healShapeFrames + 64;
	}

	int ch = 0;
	int n = 4;

	if (charNum != -1) {
		ch = charNum;
		n = charNum + 1;
	}

	charNum = ch;

	uint16 pX[4];
	uint16 pY = 138;
	uint16 diff[4];
	uint16 pts[4];
	memset(pts, 0, sizeof(pts));

	while (charNum < n) {
		if (!(_characters[charNum].flags & 1)) {
			charNum++;
			continue;
		}

		pX[charNum] = _activeCharsXpos[charNum] - 6;
		_characters[charNum].damageSuffered = 0;
		int dmg = _characters[charNum].hitPointsMax - _characters[charNum].hitPointsCur;
		diff[charNum] = (dmg < points) ? dmg : points;
		_screen->copyRegion(pX[charNum], pY, charNum * 77, 32, 77, 44, 0, 2, Screen::CR_NO_P_CHECK);
		charNum++;
	}

	int cp = _screen->setCurPage(2);
	snd_playSoundEffect(68, -1);

	for (int i = 0; i < 16; i++) {
		uint32 delayTimer = _system->getMillis() + 4 * _tickLength;

		for (charNum = ch; charNum < n; charNum++) {
			if (!(_characters[charNum].flags & 1))
				continue;

			_screen->copyRegion(charNum * 77, 32, pX[charNum], pY, 77, 44, 2, 2, Screen::CR_NO_P_CHECK);

			pts[charNum] &= 0xff;
			pts[charNum] += ((diff[charNum] << 8) / 16);
			increaseCharacterHitpoints(charNum, pts[charNum] / 256, true);
			gui_drawCharPortraitWithStats(charNum);

			_screen->drawShape(2, _healShapes[healShpFrames[i]], pX[charNum], pY, 0, 0x1000, _transparencyTable1, _transparencyTable2);
			_screen->fillRect(0, 0, 31, 31, 0);

			_screen->drawShape(_screen->_curPage, _healiShapes[healiShpFrames[i]], 0, 0, 0, 0);
			_screen->applyOverlaySpecial(_screen->_curPage, 0, 0, 2, pX[charNum] + 7, pY + 6, 32, 32, 0, 0, _healOverlay);

			_screen->copyRegion(pX[charNum], pY, pX[charNum], pY, 77, 44, 2, 0, Screen::CR_NO_P_CHECK);
			_screen->updateScreen();
		}

		delayUntil(delayTimer);
	}

	for (charNum = ch; charNum < n; charNum++) {
		if (!(_characters[charNum].flags & 1))
			continue;

		_screen->copyRegion(charNum * 77, 32, pX[charNum], pY, 77, 44, 2, 2, Screen::CR_NO_P_CHECK);

		if (curePoison)
			removeCharacterEffects(&_characters[charNum], 4, 4);

		gui_drawCharPortraitWithStats(charNum);
		_screen->copyRegion(pX[charNum], pY, pX[charNum], pY, 77, 44, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
	}

	_screen->setCurPage(cp);
	updateDrawPage2();
	return 1;
}

int LoLEngine::processMagicIce(int charNum, int spellLevel) {
	int cp = _screen->setCurPage(2);

	disableSysTimer(2);

	gui_drawScene(0);
	_screen->copyPage(0, 12);

	Palette tpal(256), swampCol(256);

	if (_currentLevel == 11 && !(_flagsTable[52] & 0x04)) {
		uint8 *sc = _screen->getPalette(0).getData();
		uint8 *dc = _screen->getPalette(2).getData();
		for (int i = 1; i < (_screen->getPalette(0).getNumColors() * 3); i++)
			SWAP(sc[i], dc[i]);

		_flagsTable[52] |= 0x04;
		static const uint8 freezeTimes[] =  { 20, 28, 40, 60 };
		setCharacterUpdateEvent(charNum, 8, freezeTimes[spellLevel], 1);
	}

	Palette s(256);
	s.copy(_screen->getPalette(1));
	if (_flags.use16ColorMode) {
		_screen->loadPalette("LOLICE.NOL", swampCol);
		for (int i = 1; i < 16; i++) {
			uint16 v = (s[i * 3] + s[i * 3 + 1] + s[i * 3 + 2]) / 3;
			tpal[i * 3] = 0;
			tpal[i * 3 + 1] = v;
			tpal[i * 3 + 2] = v << 1;

			if (tpal[i * 3 + 2] > 29)
				tpal[i * 3 + 2] = 29;
		}

	} else {
		_screen->loadPalette("SWAMPICE.COL", swampCol);
		tpal.copy(s, 128);
		swampCol.copy(s, 128);

		for (int i = 1; i < 128; i++) {
			tpal[i * 3] = 0;
			uint16 v = (s[i * 3] + s[i * 3 + 1] + s[i * 3 + 2]) / 3;
			tpal[i * 3 + 1] = v;
			tpal[i * 3 + 2] = v << 1;

			if (tpal[i * 3 + 2] > 0x3f)
				tpal[i * 3 + 2] = 0x3f;
		}
	}

	generateBrightnessPalette(tpal, tpal, _brightness, _lampEffect);
	generateBrightnessPalette(swampCol, swampCol, _brightness, _lampEffect);
	swampCol[0] = swampCol[1] = swampCol[2] = tpal[0] = tpal[1] = tpal[2] = 0;

	generateBrightnessPalette(_screen->getPalette(0), s, _brightness, _lampEffect);

	int sX = 112;
	int sY = 0;
	WSAMovie_v2 *mov = new WSAMovie_v2(this);

	if (spellLevel == 0) {
		sX = 0;
	} if (spellLevel == 1 || spellLevel == 2) {
		mov->open("SNOW.WSA", 1, 0);
		if (!mov->opened())
			error("Ice: Unable to load snow.wsa");
	} if (spellLevel == 3) {
		mov->open("ICE.WSA", 1, 0);
		if (!mov->opened())
			error("Ice: Unable to load ice.wsa");
		sX = 136;
		sY = 12;
	}

	snd_playSoundEffect(71, -1);

	playSpellAnimation(0, 0, 0, 2, 0, 0, 0, s.getData(), tpal.getData(), 40, false);

	_screen->timedPaletteFadeStep(s.getData(), tpal.getData(), _system->getMillis(), _tickLength);
	if (mov->opened()) {
		int r = true;
		if (spellLevel > 2) {
			_levelBlockProperties[calcNewBlockPosition(_currentBlock, _currentDirection)].flags |= 0x10;
			snd_playSoundEffect(165, -1);
			r = false;
		};

		playSpellAnimation(mov, 0, mov->frames(), 2, sX, sY, 0, 0, 0, 0, r);
		mov->close();
	}

	delete mov;
	static const uint8 snowDamage[] = { 10, 20, 30, 55 };
	static const uint8 iceDamageMax[] = {1, 2, 15, 20, 35};
	static const uint8 iceDamageMin[] = {10, 10, 3, 4, 4};
	static const uint8 iceDamageAdd[] = {5, 10, 30, 10, 10};

	bool breakWall = false;

	if (spellLevel < 3) {
		inflictMagicalDamageForBlock(calcNewBlockPosition(_currentBlock, _currentDirection), charNum, snowDamage[spellLevel], 3);
	} else {
		uint16 o = _levelBlockProperties[calcNewBlockPosition(_currentBlock, _currentDirection)].assignedObjects;
		while (o & 0x8000) {
			int might = rollDice(iceDamageMin[spellLevel], iceDamageMax[spellLevel]) + iceDamageAdd[spellLevel];
			int dmg = calcInflictableDamagePerItem(charNum, 0, might, 3, 2);

			LoLMonster *m = &_monsters[o & 0x7fff];
			if (m->hitPoints <= dmg) {
				increaseExperience(charNum, 2, m->hitPoints);
				o = m->nextAssignedObject;

				if (m->flags & 0x20) {
					m->mode = 0;
					monsterDropItems(m);
					if (_currentLevel != 29)
						setMonsterMode(m, 14);
					runLevelScriptCustom(0x404, -1, o, o, 0, 0);
					checkSceneUpdateNeed(m->block);
					if (m->mode != 14)
						placeMonster(m, 0, 0);

				} else {
					killMonster(m);
				}

			} else {
				breakWall = true;
				inflictDamage(o, dmg, charNum, 2, 3);
				m->damageReceived = 0;
				o = m->nextAssignedObject;
			}

			if (m->flags & 0x20)
				break;
		}
	}

	updateDrawPage2();
	gui_drawScene(0);
	enableSysTimer(2);

	if (_currentLevel != 11)
		generateBrightnessPalette(_screen->getPalette(0), swampCol, _brightness, _lampEffect);

	playSpellAnimation(0, 0, 0, 2, 0, 0, 0, tpal.getData(), swampCol.getData(), 40, 0);

	_screen->timedPaletteFadeStep(tpal.getData(), swampCol.getData(), _system->getMillis(), _tickLength);

	if (breakWall)
		breakIceWall(tpal.getData(), swampCol.getData());

	_screen->setCurPage(cp);
	return 1;
}

int LoLEngine::processMagicFireball(int charNum, int spellLevel) {
	int fbCnt = 0;
	int d = 1;

	if (spellLevel == 0) {
		fbCnt = 4;
	} else if (spellLevel == 1) {
		fbCnt = 5;
	} else if (spellLevel == 2) {
		fbCnt = 6;
	} else if (spellLevel == 3) {
		d = 0;
		fbCnt = 5;
	}

	int drawPage1 = 2;
	int drawPage2 = 4;

	int bl = _currentBlock;
	int fireballItem = makeItem(9, 0, 0);

	int i = 0;
	for (; i < 3; i++) {
		runLevelScriptCustom(bl, 0x200, -1, fireballItem, 0, 0);
		uint16 o = _levelBlockProperties[bl].assignedObjects;

		if ((o & 0x8000) || (_wllWallFlags[_levelBlockProperties[bl].walls[_currentDirection ^ 2]] & 7)) {
			while (o & 0x8000) {
				static const uint8 fireballDamage[] = { 20, 40, 80, 100 };
				int dmg = calcInflictableDamagePerItem(charNum, o, fireballDamage[spellLevel], 4, 1);
				LoLMonster *m = &_monsters[o & 0x7fff];
				o = m->nextAssignedObject;
				_envSfxUseQueue = true;
				inflictDamage(m->id | 0x8000, dmg, charNum, 2, 4);
				_envSfxUseQueue = false;
			}
			break;
		}

		bl = calcNewBlockPosition(bl, _currentDirection);
	}

	d += i;
	if (d > 3)
		d = 3;

	deleteItem(fireballItem);

	snd_playSoundEffect(69, -1);

	int cp = _screen->setCurPage(2);
	_screen->copyPage(0, 12);

	int fireBallWH = (d << 4) * -1;
	int numFireballs = 1;
	if (fbCnt > 3)
		numFireballs = fbCnt - 3;

	FireballState *fireballState[3];
	memset(&fireballState, 0, sizeof(fireballState));
	for (i = 0; i < numFireballs; i++)
		fireballState[i] = new FireballState(i);

	_screen->copyPage(12, drawPage1);

	for (i = 0; i < numFireballs;) {
		_screen->setCurPage(drawPage1);
		uint32 ctime = _system->getMillis();

		for (int ii = 0; ii < MIN(fbCnt, 3); ii++) {
			FireballState *fb = fireballState[ii];
			if (!fb)
				continue;
			if (!fb->active)
				continue;

			static const int8 finShpIndex1[] = { 5, 6, 7, 7, 6, 5 };
			static const int8 finShpIndex2[] = { -1, 1, 2, 3, 4, -1 };
			uint8 *shp = fb->finalize ? _fireballShapes[finShpIndex1[fb->finProgress]] : _fireballShapes[0];

			int fX = (((fb->progress * _fireBallCoords[fb->tblIndex & 0xff]) >> 16) + fb->destX) - ((fb->progress / 8 + shp[3] + fireBallWH) >> 1);
			int fY = (((fb->progress * _fireBallCoords[(fb->tblIndex + 64) & 0xff]) >> 16) + fb->destY) - ((fb->progress / 8 + shp[2] + fireBallWH) >> 1);
			int sW = ((fb->progress / 8 + shp[3] + fireBallWH) << 8) / shp[3];
			int sH = ((fb->progress / 8 + shp[2] + fireBallWH) << 8) / shp[2];

			if (fb->finalize) {
				if (_flags.use16ColorMode)
					_screen->drawShape(_screen->_curPage, shp, fX, fY, 0, 4, sW, sH);
				else
					_screen->drawShape(_screen->_curPage, shp, fX, fY, 0, 0x1004, _transparencyTable1, _transparencyTable2, sW, sH);

				if (finShpIndex2[fb->finProgress] != -1) {
					shp = _fireballShapes[finShpIndex2[fb->finProgress]];
					fX = (((fb->progress * _fireBallCoords[fb->tblIndex & 0xff]) >> 16) + fb->destX) - ((fb->progress / 8 + shp[3] + fireBallWH) >> 1);
					fY = (((fb->progress * _fireBallCoords[(fb->tblIndex + 64) & 0xff]) >> 16) + fb->destY) - ((fb->progress / 8 + shp[2] + fireBallWH) >> 1);
					sW = ((fb->progress / 8 + shp[3] + fireBallWH) << 8) / shp[3];
					sH = ((fb->progress / 8 + shp[2] + fireBallWH) << 8) / shp[2];
					_screen->drawShape(_screen->_curPage, shp, fX, fY, 0, 4, sW, sH);
				}

			} else {
				if (_flags.use16ColorMode)
					_screen->drawShape(_screen->_curPage, shp, fX, fY, 0, 4, sW, sH);
				else
					_screen->drawShape(_screen->_curPage, shp, fX, fY, 0, 0x1004, _transparencyTable1, _transparencyTable2, sW, sH);
			}

			if (fb->finalize) {
				if (++fb->finProgress >= 6) {
					fb->active = false;
					i++;
				}
			} else {
				if (fb->step < 40)
					fb->step += 2;
				else
					fb->step = 40;

				if (fb->progress < fb->step) {
					if (ii < 1) {
						fb->progress = fb->step = fb->finProgress = 0;
						fb->finalize = true;
					} else {
						fb->active = false;
						i++;
					}

					static const uint8 fireballSfx[] = { 98, 167, 167, 168 };
					snd_playSoundEffect(fireballSfx[d], -1);

				} else {
					fb->progress -= fb->step;
				}
			}
		}

		int del = _tickLength - (_system->getMillis() - ctime);
		if (del > 0)
			delay(del);

		_screen->checkedPageUpdate(drawPage1, drawPage2);
		_screen->updateScreen();
		SWAP(drawPage1, drawPage2);
		_screen->copyPage(12, drawPage1);
	}

	for (i = 0; i < numFireballs; i++)
		delete fireballState[i];

	_screen->setCurPage(cp);
	_screen->copyPage(12, 0);
	_screen->updateScreen();
	updateDrawPage2();
	snd_playQueuedEffects();
	runLevelScriptCustom(bl, 0x20, charNum, 3, 0, 0);
	return 1;
}

int LoLEngine::processMagicHandOfFate(int spellLevel) {
	int cp = _screen->setCurPage(2);
	_screen->copyPage(0, 12);

	WSAMovie_v2 *mov = new WSAMovie_v2(this);
	mov->open("hand.wsa", 1, 0);
	if (!mov->opened())
		error("Hand: Unable to load HAND.WSA");

	static const uint8 frames[] = { 17, 26, 11, 16, 27, 35, 27, 35, 0, 75 };

	snd_playSoundEffect(173, -1);
	playSpellAnimation(mov, 0, 10, 3, 112, 0, 0, 0, 0, 0, false);
	snd_playSoundEffect(151, -1);
	playSpellAnimation(mov, frames[spellLevel * 2] , frames[spellLevel * 2 + 1], 3, 112, 0, 0, 0, 0, 0, false);
	snd_playSoundEffect(18, -1);
	playSpellAnimation(mov, 10, 0, 3, 112, 0, 0, 0, 0, 0, false);

	mov->close();
	delete mov;

	_screen->setCurPage(cp);
	_screen->copyPage(12, 2);
	gui_drawScene(2);

	if (spellLevel < 2) {
		uint16 b1 = calcNewBlockPosition(_currentBlock, _currentDirection);
		uint16 b2 = calcNewBlockPosition(b1, _currentDirection);

		if (!testWallFlag(b2, 0, 4)) {
			if (!(_levelBlockProperties[b2].assignedObjects & 0x8000)) {
				checkSceneUpdateNeed(b1);

				uint16 dir = (_currentDirection << 1);
				uint16 o = _levelBlockProperties[b1].assignedObjects;
				while (o & 0x8000) {
					uint16 o2 = o;
					LoLMonster *m = &_monsters[o & 0x7fff];
					o = findObject(o)->nextAssignedObject;
					int nX = 0;
					int nY = 0;

					getNextStepCoords(m->x, m->y, nX, nY, dir);
					for (int i = 0; i < 7; i++)
						getNextStepCoords(nX, nY, nX, nY, dir);

					placeMonster(m, nX, nY);
					runLevelScriptCustom(b2, 0x800, -1, o2, 0, 0);
				}
			}
		}

	} else {
		uint16 b1 = calcNewBlockPosition(_currentBlock, _currentDirection);
		checkSceneUpdateNeed(b1);

		static const uint16 damage[] = { 75, 125, 175 };
		uint16 o = _levelBlockProperties[b1].assignedObjects;

		while (o & 0x8000) {
			uint16 t = o;
			o = findObject(o)->nextAssignedObject;
			// This might be a bug in the original code, but using
			// the hand of fate spell won't give any experience points
			int dmg = calcInflictableDamagePerItem(-1, t, damage[spellLevel - 2], 0x80, 1);
			inflictDamage(t, dmg, 0xffff, 3, 0x80);
		}
	}

	if (_currentLevel == 29)
		_screen->copyPage(12, 2);

	_screen->copyPage(2, 0);
	_screen->updateScreen();

	gui_drawScene(2);
	updateDrawPage2();
	return 1;
}

int LoLEngine::processMagicMistOfDoom(int charNum, int spellLevel) {
	static const uint8 mistDamage[] = { 30, 70, 110, 200 };

	_envSfxUseQueue = true;
	inflictMagicalDamageForBlock(calcNewBlockPosition(_currentBlock, _currentDirection), charNum, mistDamage[spellLevel], 0x80);
	_envSfxUseQueue = false;

	int cp = _screen->setCurPage(2);
	_screen->copyPage(0, 2);
	gui_drawScene(2);
	_screen->copyPage(2, 12);

	snd_playSoundEffect(155, -1);

	Common::String wsafile = Common::String::format("mists%0d.wsa", spellLevel + 1);
	WSAMovie_v2 *mov = new WSAMovie_v2(this);
	mov->open(wsafile.c_str(), 1, 0);
	if (!mov->opened())
		error("Mist: Unable to load %s", wsafile.c_str());

	snd_playSoundEffect(_mistAnimData[spellLevel].sound, -1);
	playSpellAnimation(mov, _mistAnimData[spellLevel].part1First, _mistAnimData[spellLevel].part1Last, 7, 112, 0, 0, 0, 0, 0, false);
	playSpellAnimation(mov, _mistAnimData[spellLevel].part2First, _mistAnimData[spellLevel].part2Last, 14, 112, 0, 0, 0, 0, 0, false);

	mov->close();
	delete mov;

	_screen->setCurPage(cp);
	_screen->copyPage(12, 0);

	updateDrawPage2();
	snd_playQueuedEffects();
	return 1;
}

int LoLEngine::processMagicLightning(int charNum, int spellLevel) {
	_screen->hideMouse();
	_screen->copyPage(0, 2);
	gui_drawScene(2);
	_screen->copyPage(2, 12);

	_lightningCurSfx = _lightningProps[spellLevel].sfxId;
	_lightningDiv = _lightningProps[spellLevel].frameDiv;
	_lightningFirstSfx = 0;

	Common::String wsafile = Common::String::format("litning%d.wsa", spellLevel + 1);
	WSAMovie_v2 *mov = new WSAMovie_v2(this);
	mov->open(wsafile.c_str(), 1, 0);
	if (!mov->opened())
		error("Litning: Unable to load %s", wsafile.c_str());

	for (int i = 0; i < 4; i++)
		playSpellAnimation(mov, 0, _lightningProps[spellLevel].lastFrame, 3, 93, 0, &LoLEngine::callbackProcessMagicLightning, 0, 0, 0, false);

	mov->close();
	delete mov;

	_screen->setScreenPalette(_screen->getPalette(1));
	_screen->copyPage(12, 2);
	_screen->copyPage(12, 0);
	updateDrawPage2();

	static const uint8 lightningDamage[] = { 18, 35, 50, 72 };
	inflictMagicalDamageForBlock(calcNewBlockPosition(_currentBlock, _currentDirection), charNum, lightningDamage[spellLevel], 5);

	_sceneUpdateRequired = true;
	gui_drawScene(0);
	_screen->showMouse();
	return 1;
}

int LoLEngine::processMagicFog() {
	int cp = _screen->setCurPage(2);
	_screen->copyPage(0, 12);

	WSAMovie_v2 *mov = new WSAMovie_v2(this);
	int numFrames = mov->open("fog.wsa", 0, 0);
	if (!mov->opened())
		error("Fog: Unable to load fog.wsa");

	snd_playSoundEffect(145, -1);

	for (int curFrame = 0; curFrame < numFrames; curFrame++) {
		uint32 delayTimer = _system->getMillis() + 3 * _tickLength;
		_screen->copyPage(12, 2);
		mov->displayFrame(curFrame % numFrames, 2, 112, 0, 0x5000, _transparencyTable1, _transparencyTable2);
		_screen->copyRegion(112, 0, 112, 0, 176, 120, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		delayUntil(delayTimer);
	}

	mov->close();
	delete mov;

	_screen->copyPage(12, 2);
	_screen->setCurPage(cp);
	updateDrawPage2();

	uint16 o = _levelBlockProperties[calcNewBlockPosition(_currentBlock, _currentDirection)].assignedObjects;
	while (o & 0x8000) {
		inflictMagicalDamage(o, -1, 15, 6, 0);
		o = _monsters[o & 0x7fff].nextAssignedObject;
	}

	gui_drawScene(0);
	return 1;
}

int LoLEngine::processMagicSwarm(int charNum, int damage) {
	createTransparencyTables();

	int cp = _screen->setCurPage(2);
	_screen->copyPage(0, 12);
	snd_playSoundEffect(74, -1);

	uint16 destIds[6];
	uint8 destModes[6];
	int8 destTicks[6];

	memset(destIds, 0, sizeof(destIds));
	memset(destModes, 8, sizeof(destModes));
	memset(destTicks, 0, sizeof(destTicks));

	int t = 0;
	uint16 o = _levelBlockProperties[calcNewBlockPosition(_currentBlock, _currentDirection)].assignedObjects;
	while (o & 0x8000) {
		o &= 0x7fff;
		if (_monsters[o].mode != 13) {
			destIds[t++] = o;

			if (!(_monsters[o].flags & 0x2000)) {
				_envSfxUseQueue = true;
				inflictMagicalDamage(o | 0x8000, charNum, damage, 0, 0);
				_envSfxUseQueue = false;
				_monsters[o].flags &= 0xffef;
			}
		}
		o = _monsters[o].nextAssignedObject;
	}

	for (int i = 0; i < t; i++) {
		SWAP(destModes[i], _monsters[destIds[i]].mode);
		SWAP(destTicks[i], _monsters[destIds[i]].fightCurTick);
	}

	gui_drawScene(_screen->_curPage);
	_screen->copyRegion(112, 0, 112, 0, 176, 120, _screen->_curPage, 7);

	for (int i = 0; i < t; i++) {
		_monsters[destIds[i]].mode = destModes[i];
		_monsters[destIds[i]].fightCurTick = destTicks[i];
	}

	WSAMovie_v2 *mov = new WSAMovie_v2(this);

	mov->open("swarm.wsa", 0, 0);
	if (!mov->opened())
		error("Swarm: Unable to load SWARM.WSA");
	_screen->hideMouse();
	playSpellAnimation(mov, 0, 37, 2, 0, 0, 0, 0, 0, 0, false);
	playSpellAnimation(mov, 38, 41, 8, 0, 0, &LoLEngine::callbackProcessMagicSwarm, 0, 0, 0, false);
	_screen->showMouse();
	mov->close();

	_screen->copyPage(12, 0);
	_screen->updateScreen();
	updateDrawPage2();

	snd_playQueuedEffects();

	_screen->setCurPage(cp);
	delete mov;
	return 1;
}

int LoLEngine::processMagicVaelansCube() {
	uint8 *sp1 = _screen->getPalette(1).getData();
	int len = _screen->getPalette(1).getNumColors() * 3;

	uint8 *tmpPal1 = new uint8[len];
	uint8 *tmpPal2 = new uint8[len];

	memcpy(tmpPal1, sp1, len);
	memcpy(tmpPal2, sp1, len);

	if (_flags.use16ColorMode) {
		for (int i = 0; i < 16; i++) {
			uint16 a = sp1[i * 3 + 1] + 16;
			tmpPal2[i * 3 + 1] = (a > 58) ? 58 : a;
			tmpPal2[i * 3] = sp1[i * 3];
			a = sp1[i * 3 + 2] + 16;
			tmpPal2[i * 3 + 2] = (a > 63) ? 63 : a;
		}
	} else {
		for (int i = 0; i < 128; i++) {
			uint16 a = sp1[i * 3] + 16;
			tmpPal2[i * 3] = (a > 60) ? 60 : a;
			tmpPal2[i * 3 + 1] = sp1[i * 3 + 1];
			a = sp1[i * 3 + 2] + 19;
			tmpPal2[i * 3 + 2] = (a > 60) ? 60 : a;
		}
	}

	snd_playSoundEffect(146, -1);

	uint32 ctime = _system->getMillis();
	uint32 endTime = _system->getMillis() + 70 * _tickLength;

	while (_system->getMillis() < endTime) {
		_screen->timedPaletteFadeStep(tmpPal1, tmpPal2, _system->getMillis() - ctime, 70 * _tickLength);
		updateInput();
	}

	uint16 bl = calcNewBlockPosition(_currentBlock, _currentDirection);
	uint8 s = _levelBlockProperties[bl].walls[_currentDirection ^ 2];
	uint8 flg = _wllWallFlags[s];

	int res = (s == 47 && (_currentLevel == 17 || _currentLevel == 24)) ? 1 : 0;
	if ((_wllVmpMap[s] == 1 || _wllVmpMap[s] == 2) && (!(flg & 1)) && (_currentLevel != 22)) {
		memset(_levelBlockProperties[bl].walls, 0, 4);
		gui_drawScene(0);
		res = 1;
	}

	uint16 o = _levelBlockProperties[bl].assignedObjects;
	while (o & 0x8000) {
		LoLMonster *m = &_monsters[o & 0x7fff];
		if (m->properties->flags & 0x1000) {
			inflictDamage(o, 100, 0xffff, 0, 0x80);
			res = 1;
		}
		o = m->nextAssignedObject;
	}

	ctime = _system->getMillis();
	endTime = _system->getMillis() + 70 * _tickLength;

	while (_system->getMillis() < endTime) {
		_screen->timedPaletteFadeStep(tmpPal2, tmpPal1, _system->getMillis() - ctime, 70 * _tickLength);
		updateInput();
	}

	delete[] tmpPal1;
	delete[] tmpPal2;

	return res;
}

int LoLEngine::processMagicGuardian(int charNum) {
	int cp = _screen->setCurPage(2);
	_screen->copyPage(0, 2);
	_screen->copyPage(2, 12);

	WSAMovie_v2 *mov = new WSAMovie_v2(this);
	mov->open("guardian.wsa", 0, 0);
	if (!mov->opened())
		error("Guardian: Unable to load guardian.wsa");
	snd_playSoundEffect(156, -1);
	playSpellAnimation(mov, 0, 37, 2, 112, 0, 0, 0, 0, 0, false);

	_screen->copyPage(2, 12);

	uint16 bl = calcNewBlockPosition(_currentBlock, _currentDirection);
	int res = (_levelBlockProperties[bl].assignedObjects & 0x8000) ? 1 : 0;
	inflictMagicalDamageForBlock(bl, charNum, 200, 0x80);

	_screen->copyPage(12, 2);
	updateDrawPage2();
	gui_drawScene(2);

	_screen->copyPage(2, 12);
	snd_playSoundEffect(176, -1);
	playSpellAnimation(mov, 38, 48, 8, 112, 0, 0, 0, 0, 0, false);

	mov->close();
	delete mov;

	_screen->setCurPage(cp);
	gui_drawPlayField();
	updateDrawPage2();
	return res;
}

void LoLEngine::callbackProcessMagicSwarm(WSAMovie_v2 *mov, int x, int y) {
	if (_swarmSpellStatus)
		_screen->copyRegion(112, 0, 112, 0, 176, 120, 6, _screen->_curPage);
	_swarmSpellStatus ^= 1;
}

void LoLEngine::callbackProcessMagicLightning(WSAMovie_v2 *mov, int x, int y) {
	if (_lightningDiv == 2)
		shakeScene(1, 2, 3, 0);

	const Palette &p1 = _screen->getPalette(1);

	if (_lightningSfxFrame % _lightningDiv) {
		_screen->setScreenPalette(p1);
	} else {
		Palette tpal(p1.getNumColors());
		tpal.copy(p1);

		int start = 6;
		int end = 384;

		if (_flags.use16ColorMode) {
			start = 3;
			end = 48;
		}

		for (int i = start; i < end; i++) {
			uint16 v = (tpal[i] * 120) / 64;
			tpal[i] = (v < 64) ? v : 63;
		}

		_screen->setScreenPalette(tpal);
	}

	if (_lightningDiv == 2) {
		if (!_lightningFirstSfx) {
			snd_playSoundEffect(_lightningCurSfx, -1);
			_lightningFirstSfx = 1;
		}
	} else {
		if (!(_lightningSfxFrame & 7))
			snd_playSoundEffect(_lightningCurSfx, -1);
	}

	_lightningSfxFrame++;
}

void LoLEngine::drinkBezelCup(int numUses, int charNum) {
	createTransparencyTables();

	int cp = _screen->setCurPage(2);
	snd_playSoundEffect(73, -1);

	WSAMovie_v2 *mov = new WSAMovie_v2(this);
	mov->open("bezel.wsa", 0, 0);
	if (!mov->opened())
		error("Bezel: Unable to load bezel.wsa");

	int x = _activeCharsXpos[charNum] - 11;
	int y = 124;
	int w = mov->width();
	int h = mov->height();

	_screen->copyRegion(x, y, 0, 0, w, h, 0, 2, Screen::CR_NO_P_CHECK);

	static const uint8 bezelAnimData[] = { 0, 26, 20, 27, 61, 55, 62, 92, 86, 93, 131, 125 };
	int frm = bezelAnimData[numUses * 3];
	int hpDiff = _characters[charNum].hitPointsMax - _characters[charNum].hitPointsCur;
	uint16 step = 0;

	do {
		step = (step & 0xff) + (hpDiff * 256) / (bezelAnimData[numUses * 3 + 1]);
		increaseCharacterHitpoints(charNum, step / 256, true);
		gui_drawCharPortraitWithStats(charNum);

		uint32 etime = _system->getMillis() + 4 * _tickLength;

		_screen->copyRegion(0, 0, x, y, w, h, 2, 2, Screen::CR_NO_P_CHECK);
		mov->displayFrame(frm, 2, x, y, _flags.use16ColorMode ? 0x4000 : 0x5000, _transparencyTable1, _transparencyTable2);
		_screen->copyRegion(x, y, x, y, w, h, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();

		delayUntil(etime);
	} while (++frm < bezelAnimData[numUses * 3 + 1]);

	_characters[charNum].hitPointsCur = _characters[charNum].hitPointsMax;
	_screen->copyRegion(0, 0, x, y, w, h, 2, 2, Screen::CR_NO_P_CHECK);
	removeCharacterEffects(&_characters[charNum], 4, 4);
	gui_drawCharPortraitWithStats(charNum);
	_screen->copyRegion(x, y, x, y, w, h, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();

	mov->close();
	delete mov;

	_screen->setCurPage(cp);
}

void LoLEngine::addSpellToScroll(int spell, int charNum) {
	bool assigned = false;
	int slot = 0;
	for (int i = 0; i < 7; i++) {
		if (!assigned && _availableSpells[i] == -1) {
			assigned = true;
			slot = i;
		}

		if (_availableSpells[i] == spell) {
			_txt->printMessage(2, "%s", getLangString(0x42d0));
			return;
		}
	}

	if (spell > 1)
		transferSpellToScollAnimation(charNum, spell, slot - 1);

	_availableSpells[slot] = spell;
	gui_enableDefaultPlayfieldButtons();
}

void LoLEngine::transferSpellToScollAnimation(int charNum, int spell, int slot)  {
	int cX = 16 + _activeCharsXpos[charNum];

	if (slot != 1) {
		_screen->loadBitmap("playfld.cps", 3, 3, 0);
		_screen->copyRegion(8, 0, 216, 0, 96, 120, 3, 3, Screen::CR_NO_P_CHECK);
		_screen->copyPage(3, 10);
		for (int i = 0; i < 9; i++) {
			int h = (slot + 1) * 9 + i + 1;
			uint32 delayTimer = _system->getMillis() + _tickLength;
			_screen->copyPage(10, 3);
			_screen->copyRegion(216, 0, 8, 0, 96, 120, 3, 3, Screen::CR_NO_P_CHECK);
			_screen->copyRegion(112, 0, 12, 0, 87, 15, 2, 2, Screen::CR_NO_P_CHECK);
			_screen->copyRegion(201, 1, 17, 15, 6, h, 2, 2, Screen::CR_NO_P_CHECK);
			_screen->copyRegion(208, 1, 89, 15, 6, h, 2, 2, Screen::CR_NO_P_CHECK);
			int cp = _screen->setCurPage(2);
			_screen->fillRect(21, 15, 89, h + 15, _flags.use16ColorMode ? 0xbb : 206);
			_screen->copyRegion(112, 16, 12, h + 15, 87, 14, 2, 2, Screen::CR_NO_P_CHECK);

			int y = 15;
			Screen::FontId of = _screen->setFont(Screen::FID_9_FNT);
			for (int ii = 0; ii < 7; ii++) {
				if (_availableSpells[ii] == -1)
					continue;
				uint8 col = (ii == _selectedSpell) ? 132 : 1;
				if (_flags.use16ColorMode)
					col = (ii == _selectedSpell) ? 0x88 : 0x44;
				_screen->fprintString("%s", 24, y, col, 0, 0, getLangString(_spellProperties[_availableSpells[ii]].spellNameCode));
				y += 9;
			}
			_screen->setFont(of);

			_screen->setCurPage(cp);
			_screen->copyRegion(8, 0, 8, 0, 96, 120, 3, 0, Screen::CR_NO_P_CHECK);
			_screen->updateScreen();

			delayUntil(delayTimer);
		}
	}

	_screen->hideMouse();

	_screen->copyPage(0, 12);
	int vX = _updateSpellBookCoords[slot << 1] + 32;
	int vY = _updateSpellBookCoords[(slot << 1) + 1] + 5;

	Common::String wsaFile = Common::String::format("write%0d", spell);
	if (_flags.isTalkie)
		wsaFile += (_lang == 1) ? 'f' : (_lang == 0 ? 'e' : 'g');
	wsaFile += ".wsa";
	snd_playSoundEffect(_updateSpellBookAnimData[(spell << 2) + 3], -1);
	snd_playSoundEffect(95, -1);

	WSAMovie_v2 *mov = new WSAMovie_v2(this);

	mov->open("getspell.wsa", 0, 0);
	if (!mov->opened())
		error("SpellBook: Unable to load getspell anim");
	snd_playSoundEffect(128, -1);
	playSpellAnimation(mov, 0, 25, 5, _activeCharsXpos[charNum], 148, 0, 0, 0, 0, true);
	snd_playSoundEffect(128, -1);
	playSpellAnimation(mov, 26, 52, 5, _activeCharsXpos[charNum], 148, 0, 0, 0, 0, true);

	for (int i = 16; i > 0; i--) {
		uint32 delayTimer = _system->getMillis() + _tickLength;
		_screen->copyPage(12, 2);

		int wsaX = vX + (((((cX - vX) << 8) / 16) * i) >> 8) - 16;
		int wsaY = vY + (((((160 - vY) << 8) / 16) * i) >> 8) - 16;

		mov->displayFrame(51, 2, wsaX, wsaY, 0x5000, _transparencyTable1, _transparencyTable2);

		_screen->copyRegion(wsaX, wsaY, wsaX, wsaY, mov->width() + 48, mov->height() + 48, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();

		delayUntil(delayTimer);
	}

	mov->close();

	mov->open("spellexp.wsa", 0, 0);
	if (!mov->opened())
		error("SpellBook: Unable to load spellexp anim");
	snd_playSoundEffect(168, -1);
	playSpellAnimation(mov, 0, 8, 3, vX - 44, vY - 38, 0, 0, 0, 0, true);
	mov->close();

	mov->open("writing.wsa", 0, 0);
	if (!mov->opened())
		error("SpellBook: Unable to load writing anim");
	playSpellAnimation(mov, 0, 6, 5, _updateSpellBookCoords[slot << 1], _updateSpellBookCoords[(slot << 1) + 1], 0, 0, 0, 0, false);
	mov->close();

	mov->open(wsaFile.c_str(), 0, 0);
	if (!mov->opened())
		error("SpellBook: Unable to load spellbook anim");
	snd_playSoundEffect(_updateSpellBookAnimData[(spell << 2) + 3], -1);
	playSpellAnimation(mov, _updateSpellBookAnimData[(spell << 2) + 1], _updateSpellBookAnimData[(spell << 2) + 2], _updateSpellBookAnimData[spell << 2], _updateSpellBookCoords[slot << 1], _updateSpellBookCoords[(slot << 1) + 1], 0, 0, 0, 0, false);
	mov->close();

	gui_drawScene(2);
	updateDrawPage2();

	_screen->showMouse();

	delete mov;
}

void LoLEngine::playSpellAnimation(WSAMovie_v2 *mov, int firstFrame, int lastFrame, int frameDelay, int x, int y, SpellProcCallback callback, uint8 *pal1, uint8 *pal2, int fadeDelay, bool restoreScreen) {
	int w = 0;
	int h = 0;

	if (mov) {
		w = mov->width();
		h = mov->height();
	}

	int w2 = w;
	int h2 = h;
	uint32 startTime = _system->getMillis();

	if (x < 0)
		w2 += x;
	if (y < 0)
		h2 += y;

	int dir = lastFrame >= firstFrame ? 1 : -1;
	int curFrame = firstFrame;

	bool fin = false;

	while (!fin) {
		uint32 delayTimer = _system->getMillis() + _tickLength * frameDelay;

		if (mov || callback)
			_screen->copyPage(12, 2);

		if (callback)
			(this->*callback)(mov, x, y);

		if (mov)
			mov->displayFrame(curFrame % mov->frames(), 2, x, y, _flags.use16ColorMode ? 0x4000 : 0x5000, _transparencyTable1, _transparencyTable2);

		if (mov || callback) {
			_screen->copyRegion(x, y, x, y, w2, h2, 2, 0, Screen::CR_NO_P_CHECK);
			_screen->updateScreen();
		}

		uint32 tm = _system->getMillis();
		uint32 del = (delayTimer > tm) ? (delayTimer - tm) : 0;

		do {
			uint32 step = del > _tickLength ? _tickLength : del;

			if (!pal1 || !pal2) {
				if (del) {
					delay(step);
					del -= step;
				} else {
					updateInput();
				}
				continue;
			}

			if (!_screen->timedPaletteFadeStep(pal1, pal2, _system->getMillis() - startTime, _tickLength * fadeDelay) && !mov)
				return;

			if (del) {
				delay(step);
				del -= step;
			} else {
				updateInput();
			}
		} while (del);

		if (!mov)
			continue;

		curFrame += dir;
		if ((dir > 0 && curFrame >= lastFrame) || (dir < 0 && curFrame < lastFrame))
			fin = true;
	}

	if (restoreScreen && (mov || callback)) {
		_screen->copyPage(12, 2);
		_screen->copyRegion(x, y, x, y, w2, h2, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
	}
}

int LoLEngine::checkMagic(int charNum, int spellNum, int spellLevel) {
	if (_spellProperties[spellNum].mpRequired[spellLevel] > _characters[charNum].magicPointsCur) {
		if (characterSays(0x4043, _characters[charNum].id, true))
			_txt->printMessage(6, getLangString(0x4043), _characters[charNum].name);
		return 1;
	} else if (_spellProperties[spellNum].hpRequired[spellLevel] >= _characters[charNum].hitPointsCur) {
		_txt->printMessage(2, getLangString(0x4179), _characters[charNum].name);
		return 1;
	}

	return 0;
}

int LoLEngine::getSpellTargetBlock(int currentBlock, int direction, int maxDistance, uint16 &targetBlock) {
	targetBlock = 0xffff;
	uint16 c = calcNewBlockPosition(currentBlock, direction);

	int i = 0;
	for (; i < maxDistance; i++) {
		if (_levelBlockProperties[currentBlock].assignedObjects & 0x8000) {
			targetBlock = currentBlock;
			return i;
		}

		if (_wllWallFlags[_levelBlockProperties[c].walls[direction ^ 2]] & 7) {
			targetBlock = c;
			return i;
		}

		currentBlock = c;
		c = calcNewBlockPosition(currentBlock, direction);
	}

	return i;
}

void LoLEngine::inflictMagicalDamage(int target, int attacker, int damage, int index, int hitType) {
	hitType = hitType ? 1 : 2;
	damage = calcInflictableDamagePerItem(attacker, target, damage, index, hitType);
	inflictDamage(target, damage, attacker, 2, index);
}

void LoLEngine::inflictMagicalDamageForBlock(int block, int attacker, int damage, int index) {
	uint16 o = _levelBlockProperties[block].assignedObjects;
	while (o & 0x8000) {
		inflictDamage(o, calcInflictableDamagePerItem(attacker, o, damage, index, 2), attacker, 2, index);
		if ((_monsters[o & 0x7fff].flags & 0x20) && (_currentLevel != 22))
			break;
		o = _monsters[o & 0x7fff].nextAssignedObject;
	}
}

// fight

int LoLEngine::battleHitSkillTest(int16 attacker, int16 target, int skill) {
	if (target == -1)
		return 0;
	if (attacker == -1)
		return 1;

	if (target & 0x8000) {
		if (_monsters[target & 0x7fff].mode >= 13)
			return 0;
	}

	uint16 hitChanceModifier = 0;
	uint16 evadeChanceModifier = 0;
	int sk = 0;

	if (attacker & 0x8000) {
		hitChanceModifier = _monsters[target & 0x7fff].properties->fightingStats[0];
		sk = 100 - _monsters[target & 0x7fff].properties->skillLevel;
	} else {
		hitChanceModifier = _characters[attacker].defaultModifiers[0];
		int8 m = _characters[attacker].skillModifiers[skill];
		if (skill == 1)
			m *= 3;
		sk = 100 - (_characters[attacker].skillLevels[skill] + m);
	}

	if (target & 0x8000) {
		evadeChanceModifier = (_monsterModifiers[9 + _monsterDifficulty] * _monsters[target & 0x7fff].properties->fightingStats[3]) >> 8;
		_monsters[target & 0x7fff].flags |= 0x10;
	} else {
		evadeChanceModifier = _characters[target].defaultModifiers[3];
	}

	int r = rollDice(1, 100);
	if (r >= sk)
		return 2;

	uint16 v = (evadeChanceModifier << 8) / hitChanceModifier;

	if (r < v)
		return 0;

	return 1;
}

int LoLEngine::calcInflictableDamage(int16 attacker, int16 target, int hitType) {
	const uint16 *s = getCharacterOrMonsterItemsMight(attacker);

	// The original code looks somewhat like the commented out part of the next line.
	// In the end the value is always set to zero. I do not know whether this is done on purpose or not.
	// It might be a bug in the original code.
	int res = 0/*attacker & 0x8000 ? 0 : _characters[attacker].might*/;
	for (int i = 0; i < 8; i++)
		res += calcInflictableDamagePerItem(attacker, target, s[i], i, hitType);

	return res;
}

int LoLEngine::inflictDamage(uint16 target, int damage, uint16 attacker, int skill, int flags) {
	LoLMonster *m = 0;
	LoLCharacter *c = 0;

	if (target & 0x8000) {
		m = &_monsters[target & 0x7fff];
		if (m->mode >= 13)
			return 0;

		if (damage > 0) {
			m->hitPoints -= damage;
			m->damageReceived = 0x8000 | damage;
			m->flags |= 0x10;
			m->hitOffsX = rollDice(1, 24);
			m->hitOffsX -= 12;
			m->hitOffsY = rollDice(1, 24);
			m->hitOffsY -= 12;
			m->hitPoints = CLIP<int16>(m->hitPoints, 0, m->properties->hitPoints);

			if (!(attacker & 0x8000))
				applyMonsterDefenseSkill(m, attacker, flags, skill, damage);

			snd_queueEnvironmentalSoundEffect(m->properties->sounds[2], m->block);
			checkSceneUpdateNeed(m->block);

			if (m->hitPoints <= 0) {
				m->hitPoints = 0;
				if (!(attacker & 0x8000))
					increaseExperience(attacker, skill, m->properties->hitPoints);
				setMonsterMode(m, 13);
			}
		} else {
			m->hitPoints -= damage;
			m->hitPoints = CLIP<int16>(m->hitPoints, 1, m->properties->hitPoints);
		}

	} else {
		if (target > 3) {
			// WORKAROUND for script bug
			int i = 0;
			for (; i < 4; i++) {
				if (_characters[i].id == target) {
					target = i;
					break;
				}
			}
			if (i == 4)
				return 0;
		}

		c = &_characters[target];
		if (!(c->flags & 1) || (c->flags & 8))
			return 0;

		if (!(c->flags & 0x1000))
			snd_playSoundEffect(c->screamSfx, -1);

		setTemporaryFaceFrame(target, 6, 4, 0);

		// check for equipped cloud ring
		if (flags == 4 && itemEquipped(target, 229))
			damage >>= 2;

		setCharacterMagicOrHitPoints(target, 0, -damage, 1);

		if (c->hitPointsCur <= 0) {
			characterHitpointsZero(target, flags);
		} else {
			_characters[target].damageSuffered = damage;
			setCharacterUpdateEvent(target, 2, 4, 1);
		}
		gui_drawCharPortraitWithStats(target);
	}

	if (!(attacker & 0x8000)) {
		if (!skill)
			_characters[attacker].weaponHit = damage;
		increaseExperience(attacker, skill, damage);
	}

	return damage;
}

void LoLEngine::characterHitpointsZero(int16 charNum, int flags) {
	LoLCharacter *c = &_characters[charNum];
	c->hitPointsCur = 0;
	c->flags |= 8;
	removeCharacterEffects(c, 1, 5);
	_partyDamageFlags = flags;
}

void LoLEngine::removeCharacterEffects(LoLCharacter *c, int first, int last) {
	for (int i = first; i <= last; i++) {
		switch (i - 1) {
		case 0:
			c->flags &= 0xfffb;
			c->weaponHit = 0;
			break;

		case 1:
			c->damageSuffered = 0;
			break;

		case 2:
			c->flags &= 0xffbf;
			break;

		case 3:
			c->flags &= 0xff7f;
			break;

		case 4:
			c->flags &= 0xfeff;
			break;

		case 6:
			c->flags &= 0xefff;
			break;

		default:
			break;
		}

		for (int ii = 0; ii < 5; ii++) {
			if (i != c->characterUpdateEvents[ii])
				continue;

			c->characterUpdateEvents[ii] = 0;
			c->characterUpdateDelay[ii] = 0;
		}
	}

	_timer->enable(3);
}

int LoLEngine::calcInflictableDamagePerItem(int16 attacker, int16 target, uint16 itemMight, int index, int hitType) {
	int dmg = (attacker == -1) ? 0x100 : getCharacterOrMonsterStats(attacker)[1];
	const uint16 *st_t = getCharacterOrMonsterProtectionAgainstItems(target);

	dmg = (dmg * itemMight) >> 8;
	if (!dmg)
		return 0;

	if (!(attacker & 0x8000)) {
		dmg = (dmg * _characters[attacker].totalMightModifier) >> 8;
		if (!dmg)
			return 0;
	}

	int d = (index & 0x80) ? st_t[7] : st_t[index];
	int r = (dmg * ABS(d)) >> 8;
	dmg = d < 0 ? -r : r;

	if (hitType == 2 || !dmg)
		return (dmg == 1) ? 2 : dmg;


	int p = (calculateProtection(target) << 7) / dmg;
	if (p > 217)
		p = 217;

	d = 256 - p;
	r = (dmg * ABS(d)) >> 8;
	dmg = d < 0 ? -r : r;

	return (dmg < 2) ? 2 : dmg;
}

void LoLEngine::checkForPartyDeath() {
	Button b;
	b.data0Val2 = b.data1Val2 = b.data2Val2 = 0xfe;
	b.data0Val3 = b.data1Val3 = b.data2Val3 = 0x01;

	for (int i = 0; i < 4; i++) {
		if (!(_characters[i].flags & 1) || _characters[i].hitPointsCur <= 0)
			continue;
		return;
	}

	if (_weaponsDisabled)
		clickedExitCharInventory(&b);

	gui_drawAllCharPortraitsWithStats();

	if (_partyDamageFlags & 0x40) {
		_screen->fadeToBlack(40);
		for (int i = 0; i < 4; i++) {
			if (_characters[i].flags & 1)
				increaseCharacterHitpoints(i, 1, true);
		}
		gui_drawAllCharPortraitsWithStats();
		_screen->fadeToPalette1(40);

	} else {
		if (!_flags.use16ColorMode)
			_screen->fadeClearSceneWindow(10);
		restoreAfterSpecialScene(0, 1, 1, 0);

		snd_playTrack(325);
		stopPortraitSpeechAnim();
		initTextFading(0, 1);
		setMouseCursorToIcon(0);
		_updateFlags |= 4;
		setLampMode(true);
		disableSysTimer(2);

		_gui->runMenu(_gui->_deathMenu);

		setMouseCursorToItemInHand();
		_updateFlags &= 0xfffb;
		resetLampStatus();

		gui_enableDefaultPlayfieldButtons();
		enableSysTimer(2);
		updateDrawPage2();
	}
}

void LoLEngine::applyMonsterAttackSkill(LoLMonster *monster, int16 target, int16 damage) {
	if (rollDice(1, 100) > monster->properties->attackSkillChance)
		return;

	int t = 0;

	switch (monster->properties->attackSkillType - 1) {
	case 0:
		t = removeCharacterItem(target, 0x7ff);
		if (t) {
			giveItemToMonster(monster, t);
			if (characterSays(0x4019, _characters[target].id, true))
				_txt->printMessage(6, "%s", getLangString(0x4019));
		}
		break;

	case 1:
		// poison character
		paralyzePoisonCharacter(target, 0x80, 0x88, 100, 1);
		break;

	case 2:
		t = removeCharacterItem(target, 0x20);
		if (t) {
			deleteItem(t);
			if (characterSays(0x401b, _characters[target].id, true))
				_txt->printMessage(6, "%s", getLangString(0x401b));
		}
		break;

	case 3:
		t = removeCharacterItem(target, 0x0f);
		if (t) {
			if (characterSays(0x401e, _characters[target].id, true))
				_txt->printMessage(6, getLangString(0x401e), _characters[target].name);
			setItemPosition(t, monster->x, monster->y, 0, 1);
		}
		break;

	case 5:
		if (_characters[target].magicPointsCur <= 0)
			return;

		monster->hitPoints += _characters[target].magicPointsCur;
		_characters[target].magicPointsCur = 0;
		gui_drawCharPortraitWithStats(target);
		if (characterSays(0x4020, _characters[target].id, true))
			_txt->printMessage(6, getLangString(0x4020), _characters[target].name);
		break;

	case 7:
		stunCharacter(target);
		break;

	case 8:
		monster->hitPoints += damage;
		if (monster->hitPoints > monster->properties->hitPoints)
			monster->hitPoints = monster->properties->hitPoints;
		break;

	case 9:
		// paralyze party (spider web)
		paralyzePoisonAllCharacters(0x40, 0x48, 100);
		break;

	default:
		break;
	}
}

void LoLEngine::applyMonsterDefenseSkill(LoLMonster *monster, int16 attacker, int flags, int skill, int damage) {
	if (rollDice(1, 100) > monster->properties->defenseSkillChance)
		return;

	int itm = 0;

	switch (monster->properties->defenseSkillType - 1) {
	case 0:
	case 1:
		if ((flags & 0x3f) == 2 || skill)
			return;

		for (int i = 0; i < 3; i++) {
			itm = _characters[attacker].items[i];
			if (!itm)
				continue;
			if ((_itemProperties[_itemsInPlay[itm].itemPropertyIndex].protection & 0x3f) != flags)
				continue;

			removeCharacterItem(attacker, 0x7fff);

			if (monster->properties->defenseSkillType == 1) {
				giveItemToMonster(monster, itm);
				if (characterSays(0x401c, _characters[attacker].id, true))
					_txt->printMessage(6, "%s", getLangString(0x401c));

			} else {
				deleteItem(itm);
				if (characterSays(0x401d, _characters[attacker].id, true))
					_txt->printMessage(6, "%s", getLangString(0x401d));
			}
		}
		break;

	case 2:
		if (!(flags & 0x80))
			return;
		monster->flags |= 8;
		monster->direction = calcMonsterDirection(monster->x, monster->y, _partyPosX, _partyPosY) ^ 4;
		setMonsterMode(monster, 9);
		monster->fightCurTick = 30;
		break;

	case 3:
		if (flags != 3)
			return;
		monster->hitPoints += damage;
		if (monster->hitPoints > monster->properties->hitPoints)
			monster->hitPoints = monster->properties->hitPoints;
		break;

	case 4:
		if (!(flags & 0x80))
			return;
		monster->hitPoints += damage;
		if (monster->hitPoints > monster->properties->hitPoints)
			monster->hitPoints = monster->properties->hitPoints;
		break;

	case 5:
		if ((flags & 0x84) == 0x84)
			monster->numDistAttacks++;
		break;

	default:
		break;
	}
}

int LoLEngine::removeCharacterItem(int charNum, int itemFlags) {
	for (int i = 0; i < 11; i++) {
		int s = _characters[charNum].items[i];
		if (!((1 << i) & itemFlags) || !s)
			continue;

		_characters[charNum].items[i] = 0;
		runItemScript(charNum, s, 0x100, 0, 0);

		return s;
	}

	return 0;
}

int LoLEngine::paralyzePoisonCharacter(int charNum, int typeFlag, int immunityFlags, int hitChance, int redraw) {
	if (!(_characters[charNum].flags & 1) || (_characters[charNum].flags & immunityFlags))
		return 0;

	if (rollDice(1, 100) > hitChance)
		return 0;

	int r = 0;

	if (typeFlag == 0x40) {
		_characters[charNum].flags |= 0x40;
		setCharacterUpdateEvent(charNum, 3, 3600, 1);
		r = 1;

	// check for bezel ring
	} else if (typeFlag == 0x80 && !itemEquipped(charNum, 225)) {
		_characters[charNum].flags |= 0x80;
		setCharacterUpdateEvent(charNum, 4, 10, 1);
		if (characterSays(0x4021, _characters[charNum].id, true))
			_txt->printMessage(6, getLangString(0x4021), _characters[charNum].name);
		r = 1;

	} else if (typeFlag == 0x1000) {
		_characters[charNum].flags |= 0x1000;
		setCharacterUpdateEvent(charNum, 7, 120, 1);
		r = 1;
	}

	if (r && redraw)
		gui_drawCharPortraitWithStats(charNum);

	return r;
}

void LoLEngine::paralyzePoisonAllCharacters(int typeFlag, int immunityFlags, int hitChance) {
	bool r = false;
	for (int i = 0; i < 4; i++) {
		if (paralyzePoisonCharacter(i, typeFlag, immunityFlags, hitChance, 0))
			r = true;
	}
	if (r)
		gui_drawAllCharPortraitsWithStats();
}

void LoLEngine::stunCharacter(int charNum) {
	if (!(_characters[charNum].flags & 1) || (_characters[charNum].flags & 0x108))
		return;

	_characters[charNum].flags |= 0x100;

	setCharacterUpdateEvent(charNum, 5, 20, 1);
	gui_drawCharPortraitWithStats(charNum);

	_txt->printMessage(6, getLangString(0x4026), _characters[charNum].name);
}

void LoLEngine::restoreSwampPalette() {
	_flagsTable[52] &= 0xFB;
	if (_currentLevel != 11)
		return;

	uint8 *s = _screen->getPalette(2).getData();
	uint8 *d = _screen->getPalette(0).getData();
	uint8 *d2 = _screen->getPalette(1).getData();

	for (int i = 1; i < (_screen->getPalette(0).getNumColors() * 3); i++)
		SWAP(s[i], d[i]);

	generateBrightnessPalette(_screen->getPalette(0), _screen->getPalette(1), _brightness, _lampEffect);
	_screen->loadSpecialColors(_screen->getPalette(2));
	_screen->loadSpecialColors(_screen->getPalette(1));

	playSpellAnimation(0, 0, 0, 2, 0, 0, 0, s, d2, 40, 0);
}

void LoLEngine::launchMagicViper() {
	_partyAwake = true;

	int d = 0;
	for (uint16 b = _currentBlock; d < 3; d++) {
		uint16 o = _levelBlockProperties[b].assignedObjects;
		if (o & 0x8000)
			break;
		b = calcNewBlockPosition(b, _currentDirection);
		if (_wllWallFlags[_levelBlockProperties[b].walls[_currentDirection ^ 2]] & 7)
			break;
	}

	_screen->copyPage(0, 12);
	snd_playSoundEffect(148, -1);

	WSAMovie_v2 *mov = new WSAMovie_v2(this);
	int numFrames = mov->open("viper.wsa", 1, 0);
	if (!mov->opened())
		error("Viper: Unable to load viper.wsa");

	static const uint8 viperAnimData[] = { 15, 25, 20, 10, 25, 20, 5, 25, 20, 0, 25, 20 };
	const uint8 *v = &viperAnimData[d * 3];
	int frm = v[0];

	for (bool running = true; running;) {
		uint32 etime = _system->getMillis() + 5 * _tickLength;
		_screen->copyPage(12, 2);

		if (frm == v[2])
			snd_playSoundEffect(172, -1);

		mov->displayFrame(frm++ % numFrames, 2, 112, 0, 0x5000, _transparencyTable1, _transparencyTable2);
		_screen->copyRegion(112, 0, 112, 0, 176, 120, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		delayUntil(etime);

		if (frm > v[1])
			running = false;
	}

	mov->close();
	delete mov;

	_screen->copyPage(12, 0);
	_screen->copyPage(12, 2);

	int t = rollDice(1, 4);

	for (int i = 0; i < 4; i++) {
		if (!(_characters[i].flags & 1)) {
			t = t % 4;
			continue;
		}
		inflictDamage(t, _currentLevel + 10, 0x8000, 2, 0x86);
	}
}

void LoLEngine::breakIceWall(uint8 *pal1, uint8 *pal2) {
	_screen->hideMouse();
	uint16 bl = calcNewBlockPosition(_currentBlock, _currentDirection);
	_levelBlockProperties[bl].flags &= 0xef;
	_screen->copyPage(0, 2);
	gui_drawScene(2);
	_screen->copyPage(2, 10);

	WSAMovie_v2 *mov = new WSAMovie_v2(this);
	int numFrames = mov->open("shatter.wsa", 1, 0);
	if (!mov->opened())
		error("Shatter: Unable to load shatter.wsa");
	snd_playSoundEffect(166, -1);
	playSpellAnimation(mov, 0, numFrames, 1, 58, 0, 0, pal1, pal2, 20, true);
	mov->close();
	delete mov;

	_screen->copyPage(10, 0);
	updateDrawPage2();
	gui_drawScene(0);
	_screen->showMouse();
}

uint16 LoLEngine::getNearestMonsterFromCharacter(int charNum) {
	return getNearestMonsterFromCharacterForBlock(calcNewBlockPosition(_currentBlock, _currentDirection), charNum);
}

uint16 LoLEngine::getNearestMonsterFromCharacterForBlock(uint16 block, int charNum) {
	uint16 cX = 0;
	uint16 cY = 0;

	uint16 id = 0xffff;
	int minDist = 0x7fff;

	if (block == 0xffff)
		return id;

	calcCoordinatesForSingleCharacter(charNum, cX, cY);

	int o = _levelBlockProperties[block].assignedObjects;

	while (o & 0x8000) {
		LoLMonster *m = &_monsters[o & 0x7fff];
		if (m->mode >= 13) {
			o = m->nextAssignedObject;
			continue;
		}

		int d = ABS(cX - m->x) + ABS(cY - m->y);
		if (d < minDist) {
			minDist = d;
			id = o;
		}

		o = m->nextAssignedObject;
	}

	return id;
}

uint16 LoLEngine::getNearestMonsterFromPos(int x, int y) {
	uint16 id = 0xffff;
	int minDist = 0x7fff;

	for (int i = 0; i < 30; i++) {
		if (_monsters[i].mode > 13)
			continue;

		int d = ABS(x - _monsters[i].x) + ABS(y - _monsters[i].y);
		if (d < minDist) {
			minDist = d;
			id = 0x8000 | i;
		}
	}

	return id;
}

uint16 LoLEngine::getNearestPartyMemberFromPos(int x, int y) {
	uint16 id = 0xffff;
	int minDist = 0x7fff;

	for (int i = 0; i < 4; i++) {
		if (!(_characters[i].flags & 1) || _characters[i].hitPointsCur <= 0)
			continue;

		uint16 charX = 0;
		uint16 charY = 0;
		calcCoordinatesForSingleCharacter(i, charX, charY);

		int d = ABS(x - charX) + ABS(y - charY);
		if (d < minDist) {
			minDist = d;
			id = i;
		}
	}

	return id;
}

// magic atlas

void LoLEngine::displayAutomap() {
	snd_playSoundEffect(105, -1);
	gui_toggleButtonDisplayMode(_flags.isTalkie ? 78 : 76, 1);

	_currentMapLevel = _currentLevel;
	uint8 *tmpWll = new uint8[80];
	memcpy(tmpWll, _wllAutomapData, 80);

	_screen->loadBitmap("parch.cps", 2, 2, &_screen->getPalette(3));
	_screen->loadBitmap("autobut.shp", 3, 5, 0);
	const uint8 *shp = _screen->getCPagePtr(5);

	for (int i = 0; i < 109; i++)
		_automapShapes[i] = _screen->getPtrToShape(shp, i + 11);

	if (_flags.use16ColorMode) {
		static const uint8 ovlSrc[] = { 0x00, 0xEE, 0xCC, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0x22, 0x11, 0xDD, 0xEE, 0xCC };
		memset(_mapOverlay, 0, 256);
		for (int i = 0; i < 16; i++)
			_mapOverlay[(i << 4) | i] = ovlSrc[i];
	} else
		_screen->generateGrayOverlay(_screen->getPalette(3), _mapOverlay, 52, 0, 0, 0, 256, false);

	_screen->loadFont(Screen::FID_9_FNT, "FONT9PN.FNT");
	_screen->loadFont(Screen::FID_6_FNT, "FONT6PN.FNT");

	for (int i = 0; i < 11; i++)
		_defaultLegendData[i].enable = false;

	disableSysTimer(2);
	generateTempData();
	resetItems(1);
	disableMonsters();

	bool exitAutomap = false;
	_mapUpdateNeeded = false;

	restoreBlockTempData(_currentMapLevel);
	loadMapLegendData(_currentMapLevel);
	_screen->fadeToBlack(10);
	drawMapPage(2);

	_screen->copyPage(2, 0);
	_screen->updateScreen();
	_screen->fadePalette(_screen->getPalette(3), 10);
	uint32 delayTimer = _system->getMillis() + 8 * _tickLength;

	while (!exitAutomap && !shouldQuit()) {
		if (_mapUpdateNeeded) {
			drawMapPage(2);
			_screen->copyPage(2, 0);
			_screen->updateScreen();
			_mapUpdateNeeded = false;
		}

		if (_system->getMillis() >= delayTimer) {
			redrawMapCursor();
			delayTimer = _system->getMillis() + 8 * _tickLength;
		}

		int f = checkInput(0) & 0xff;
		removeInputTop();

		if (f) {
			exitAutomap = automapProcessButtons(f);
			gui_notifyButtonListChanged();
		}

		if (f == 0x30) {
			for (int i = 0; i < 1024; i++)
				_levelBlockProperties[i].flags |= 7;
			_mapUpdateNeeded = true;
		} else if (f == _keyMap[Common::KEYCODE_ESCAPE]) {
			exitAutomap = true;
		}

		delay(_tickLength);
	}

	_screen->loadFont(Screen::FID_9_FNT, "FONT9P.FNT");
	_screen->loadFont(Screen::FID_6_FNT, "FONT6P.FNT");

	if (_flags.use16ColorMode)
		_screen->clearPage(2);

	_screen->fadeToBlack(10);
	loadLevelWallData(_currentLevel, false);
	memcpy(_wllAutomapData, tmpWll, 80);
	delete[] tmpWll;
	restoreBlockTempData(_currentLevel);
	addLevelItems();
	gui_notifyButtonListChanged();
	enableSysTimer(2);
}

void LoLEngine::updateAutoMap(uint16 block) {
	if (!(_flagsTable[31] & 0x10))
		return;
	_levelBlockProperties[block].flags |= 7;

	uint16 x = block & 0x1f;
	uint16 y = block >> 5;

	updateAutoMapIntern(block, x, y, -1, -1);
	updateAutoMapIntern(block, x, y, 1, -1);
	updateAutoMapIntern(block, x, y, -1, 1);
	updateAutoMapIntern(block, x, y, 1, 1);
	updateAutoMapIntern(block, x, y, 0, -1);
	updateAutoMapIntern(block, x, y, 0, 1);
	updateAutoMapIntern(block, x, y, -1, 0);
	updateAutoMapIntern(block, x, y, 1, 0);
}

bool LoLEngine::updateAutoMapIntern(uint16 block, uint16 x, uint16 y, int16 xOffs, int16 yOffs) {
	static const int16 blockPosTable[] = { 1, -1, 3, 2, -1, 0, -1, 0, 1, -32, 0, 32 };
	x += xOffs;
	y += yOffs;

	if ((x & 0xffe0) || (y & 0xffe0))
		return false;

	xOffs++;
	yOffs++;

	int16 fx = blockPosTable[xOffs];
	uint16 b = block + blockPosTable[6 + xOffs];

	if (fx != -1) {
		if (_wllAutomapData[_levelBlockProperties[b].walls[fx]] & 0xc0)
			return false;
	}

	int16 fy = blockPosTable[3 + yOffs];
	b = block + blockPosTable[9 + yOffs];

	if (fy != -1) {
		if (_wllAutomapData[_levelBlockProperties[b].walls[fy]] & 0xc0)
			return false;
	}

	b = block + blockPosTable[6 + xOffs] + blockPosTable[9 + yOffs];

	if ((fx != -1) && (fy != -1) && (_wllAutomapData[_levelBlockProperties[b].walls[fx]] & 0xc0) && (_wllAutomapData[_levelBlockProperties[b].walls[fy]] & 0xc0))
		return false;

	_levelBlockProperties[b].flags |= 7;

	return true;
}

void LoLEngine::loadMapLegendData(int level) {
	uint16 *legendData = (uint16 *)_tempBuffer5120;
	for (int i = 0; i < 32; i++) {
		legendData[i * 6] = 0xffff;
		legendData[i * 6 + 5] = 0xffff;
	}

	Common::String file = Common::String::format("level%d.xxx", level);
	uint32 size = 0;
	uint8 *data = _res->fileData(file.c_str(), &size);
	uint8 *pos = data;
	size = MIN<uint32>(size / 12, 32);

	for (uint32 i = 0; i < size; i++) {
		uint16 *l = &legendData[i * 6];
		l[3] = READ_LE_UINT16(pos);
		pos += 2;
		l[4] = READ_LE_UINT16(pos);
		pos += 2;
		l[5] = READ_LE_UINT16(pos);
		pos += 2;
		l[0] = READ_LE_UINT16(pos);
		pos += 2;
		l[1] = READ_LE_UINT16(pos);
		pos += 2;
		l[2] = READ_LE_UINT16(pos);
		pos += 2;
	}

	delete[] data;
}

void LoLEngine::drawMapPage(int pageNum) {
	// WORKAROUND for French version. The text does not always properly fit the screen there.
	const int8 xOffset = (_lang == 1) ? -2 : 0;

	if (_flags.use16ColorMode)
		_screen->clearPage(pageNum);

	for (int i = 0; i < 2; i++) {
		_screen->loadBitmap("parch.cps", pageNum, pageNum, &_screen->getPalette(3));
		if (_lang == 1)
			_screen->copyRegion(236, 16, 236 + xOffset, 16, -xOffset, 1, pageNum, pageNum, Screen::CR_NO_P_CHECK);

		int cp = _screen->setCurPage(pageNum);
		Screen::FontId of = _screen->setFont(_flags.use16ColorMode ? Screen::FID_SJIS_FNT : Screen::FID_9_FNT);
		_screen->printText(getLangString(_autoMapStrings[_currentMapLevel]), 236 + xOffset, 8, 1, 0);
		uint16 blX = mapGetStartPosX();
		uint16 bl = (mapGetStartPosY() << 5) + blX;

		int sx = _automapTopLeftX;
		int sy = _automapTopLeftY;

		for (; bl < 1024; bl++) {
			uint8 *w = _levelBlockProperties[bl].walls;
			if ((_levelBlockProperties[bl].flags & 7) == 7 && (!(_wllAutomapData[w[0]] & 0xc0)) && (!(_wllAutomapData[w[2]] & 0xc0)) && (!(_wllAutomapData[w[1]] & 0xc0)) && (!(_wllAutomapData[w[3]] & 0xc0))) {
				uint16 b0 = calcNewBlockPosition(bl, 0);
				uint16 b2 = calcNewBlockPosition(bl, 2);
				uint16 b1 = calcNewBlockPosition(bl, 1);
				uint16 b3 = calcNewBlockPosition(bl, 3);

				uint8 w02 = _levelBlockProperties[b0].walls[2];
				uint8 w20 = _levelBlockProperties[b2].walls[0];
				uint8 w13 = _levelBlockProperties[b1].walls[3];
				uint8 w31 = _levelBlockProperties[b3].walls[1];

				// draw block
				_screen->copyBlockAndApplyOverlay(_screen->_curPage, sx, sy, _screen->_curPage, sx, sy, 7, 6, 0, _mapOverlay);

				// draw north wall
				drawMapBlockWall(b3, w31, sx, sy, 3);
				drawMapShape(w31, sx, sy, 3);
				if (_wllAutomapData[w31] & 0xc0)
					_screen->copyBlockAndApplyOverlay(_screen->_curPage, sx, sy, _screen->_curPage, sx, sy, 1, 6, 0, _mapOverlay);

				// draw west wall
				drawMapBlockWall(b1, w13, sx, sy, 1);
				drawMapShape(w13, sx, sy, 1);
				if (_wllAutomapData[w13] & 0xc0)
					_screen->copyBlockAndApplyOverlay(_screen->_curPage, sx + 6, sy, _screen->_curPage, sx + 6, sy, 1, 6, 0, _mapOverlay);

				// draw east wall
				drawMapBlockWall(b0, w02, sx, sy, 0);
				drawMapShape(w02, sx, sy, 0);
				if (_wllAutomapData[w02] & 0xc0)
					_screen->copyBlockAndApplyOverlay(_screen->_curPage, sx, sy, _screen->_curPage, sx, sy, 7, 1, 0, _mapOverlay);

				//draw south wall
				drawMapBlockWall(b2, w20, sx, sy, 2);
				drawMapShape(w20, sx, sy, 2);
				if (_wllAutomapData[w20] & 0xc0)
					_screen->copyBlockAndApplyOverlay(_screen->_curPage, sx, sy + 5, _screen->_curPage, sx, sy + 5, 7, 1, 0, _mapOverlay);
			}

			sx += 7;
			if (bl % 32 == 31) {
				sx = _automapTopLeftX;
				sy += 6;
				bl += blX;
			}
		}

		_screen->setFont(of);
		_screen->setCurPage(cp);

		of = _screen->setFont(_flags.use16ColorMode ? Screen::FID_SJIS_FNT : Screen::FID_6_FNT);

		int tY = 0;
		sx = mapGetStartPosX();
		sy = mapGetStartPosY();

		uint16 *legendData = (uint16 *)_tempBuffer5120;
		uint8 yOffset = _flags.use16ColorMode ? 4 : 0;

		for (int ii = 0; ii < 32; ii++)  {
			uint16 *l = &legendData[ii * 6];
			if (l[0] == 0xffff)
				break;

			uint16 cbl = l[0] + (l[1] << 5);
			if ((_levelBlockProperties[cbl].flags & 7) != 7)
				continue;

			if (l[2] == 0xffff)
				continue;

			printMapText(l[2], 244 + xOffset, (tY << 3) + 22 + yOffset);

			if (l[5] == 0xffff) {
				tY++;
				continue;
			}

			uint16 cbl2 = l[3] + (l[4] << 5);
			_levelBlockProperties[cbl2].flags |= 7;
			_screen->drawShape(2, _automapShapes[l[5] << 2], (l[3] - sx) * 7 + _automapTopLeftX - 3, (l[4] - sy) * 6 + _automapTopLeftY - 3, 0, 0);
			_screen->drawShape(2, _automapShapes[l[5] << 2], 231 + xOffset, (tY << 3) + 19 + yOffset, 0, 0);
			tY++;
		}

		cp = _screen->setCurPage(pageNum);

		for (int ii = 0; ii < 11; ii++) {
			if (!_defaultLegendData[ii].enable)
				continue;
			_screen->copyBlockAndApplyOverlay(_screen->_curPage, 235, (tY << 3) + 21 + yOffset, _screen->_curPage, 235 + xOffset, (tY << 3) + 21 + yOffset, 7, 6, 0, _mapOverlay);
			_screen->drawShape(_screen->_curPage, _automapShapes[_defaultLegendData[ii].shapeIndex << 2], 232 + xOffset, (tY << 3) + 18 + yOffset + _defaultLegendData[ii].y, 0, 0);
			printMapText(_defaultLegendData[ii].stringId, 244 + xOffset, (tY << 3) + 22 + yOffset);
			tY++;
		}

		_screen->setFont(of);
		_screen->setCurPage(cp);
	}

	printMapExitButtonText();
}

bool LoLEngine::automapProcessButtons(int inputFlag) {
	int r = -1;
	if (inputFlag == _keyMap[Common::KEYCODE_RIGHT] || inputFlag == _keyMap[Common::KEYCODE_KP6]) {
		r = 0;
	} else if (inputFlag == _keyMap[Common::KEYCODE_LEFT] || inputFlag == _keyMap[Common::KEYCODE_KP4]) {
		r = 1;
	} else if (inputFlag == 199) {
		if (posWithinRect(_mouseX, _mouseY, 252, 175, 273, 200))
			r = 0;
		else if (posWithinRect(_mouseX, _mouseY, 231, 175, 252, 200))
			r = 1;
		else if (posWithinRect(_mouseX, _mouseY, 275, 175, 315, 197))
			r = 2;

		printMapExitButtonText();

		while (inputFlag == 199 || inputFlag == 200) {
			inputFlag = checkInput(0, false);
			removeInputTop();
			delay(_tickLength);
		}
	} else {
		return false;
	}

	if (r == 0) {
		automapForwardButton();
		printMapExitButtonText();
	} else if (r == 1) {
		automapBackButton();
		printMapExitButtonText();
	} if (r == 2) {
		return true;
	}

	return false;
}

void LoLEngine::automapForwardButton() {
	int i = _currentMapLevel + 1;
	while (!(_hasTempDataFlags & (1 << (i - 1))))
		i = (i + 1) & 0x1f;
	if (i == _currentMapLevel)
		return;

	for (int l = 0; l < 11; l++)
		_defaultLegendData[l].enable = false;

	_currentMapLevel = i;
	loadLevelWallData(i, false);
	restoreBlockTempData(i);
	loadMapLegendData(i);
	_mapUpdateNeeded = true;
}

void LoLEngine::automapBackButton() {
	int i = _currentMapLevel - 1;
	while (!(_hasTempDataFlags & (1 << (i - 1))))
		i = (i - 1) & 0x1f;
	if (i == _currentMapLevel)
		return;

	for (int l = 0; l < 11; l++)
		_defaultLegendData[l].enable = false;

	_currentMapLevel = i;
	loadLevelWallData(i, false);
	restoreBlockTempData(i);
	loadMapLegendData(i);
	_mapUpdateNeeded = true;
}

void LoLEngine::redrawMapCursor() {
	int sx = mapGetStartPosX();
	int sy = mapGetStartPosY();

	if (_currentLevel != _currentMapLevel)
		return;

	int cx = _automapTopLeftX + (((_currentBlock - sx) % 32) * 7);
	int cy = _automapTopLeftY + (((_currentBlock - (sy << 5)) / 32) * 6);

	if (_flags.use16ColorMode) {
		_screen->drawShape(0, _automapShapes[48 + _currentDirection], cx - 3, cy - 2, 0, 0);
	} else {
		_screen->fillRect(0, 0, 16, 16, 0, 2);
		_screen->drawShape(2, _automapShapes[48 + _currentDirection], 0, 0, 0, 0);
		_screen->copyRegion(cx, cy, cx, cy, 16, 16, 2, 0);
		_screen->copyBlockAndApplyOverlay(2, 0, 0, 0, cx - 3, cy - 2, 16, 16, 0, _mapCursorOverlay);

		_mapCursorOverlay[24] = _mapCursorOverlay[1];
		for (int i = 1; i < 24; i++)
			_mapCursorOverlay[i] = _mapCursorOverlay[i + 1];
	}

	_screen->updateScreen();
}

void LoLEngine::drawMapBlockWall(uint16 block, uint8 wall, int x, int y, int direction) {
	if (((1 << direction) & _levelBlockProperties[block].flags) || ((_wllAutomapData[wall] & 0x1f) != 13))
		return;

	int cp = _screen->_curPage;
	_screen->copyBlockAndApplyOverlay(cp, x + _mapCoords[0][direction], y + _mapCoords[1][direction], cp, x + _mapCoords[0][direction], y + _mapCoords[1][direction], _mapCoords[2][direction], _mapCoords[3][direction], 0, _mapOverlay);
	_screen->copyBlockAndApplyOverlay(cp, x + _mapCoords[4][direction], y + _mapCoords[5][direction], cp, x + _mapCoords[4][direction], y + _mapCoords[5][direction], _mapCoords[8][direction], _mapCoords[9][direction], 0, _mapOverlay);
	_screen->copyBlockAndApplyOverlay(cp, x + _mapCoords[6][direction], y + _mapCoords[7][direction], cp, x + _mapCoords[6][direction], y + _mapCoords[7][direction], _mapCoords[8][direction], _mapCoords[9][direction], 0, _mapOverlay);
}

void LoLEngine::drawMapShape(uint8 wall, int x, int y, int direction) {
	int l = _wllAutomapData[wall] & 0x1f;
	if (l == 0x1f)
		return;

	_screen->drawShape(_screen->_curPage, _automapShapes[(l << 2) + direction], x + _mapCoords[10][direction] - 2, y + _mapCoords[11][direction] - 2, 0, 0);
	mapIncludeLegendData(l);
}

int LoLEngine::mapGetStartPosX() {
	int c = 0;
	int a = 32;

	do {
		for (a = 0; a < 32; a++) {
			if (_levelBlockProperties[(a << 5) + c].flags)
				break;
		}
		if (a == 32)
			c++;
	} while (c < 32 && a == 32);

	int d = 31;
	a = 32;

	do {
		for (a = 0; a < 32; a++) {
			if (_levelBlockProperties[(a << 5) + d].flags)
				break;
		}
		if (a == 32)
			d--;
	} while (d > 0 && a == 32);

	_automapTopLeftX = (d > c) ? ((32 - (d - c)) >> 1) * 7 + 5 : 5;
	return (d > c) ? c : 0;
}

int LoLEngine::mapGetStartPosY() {
	int c = 0;
	int a = 32;

	do {
		for (a = 0; a < 32; a++) {
			if (_levelBlockProperties[(c << 5) + a].flags)
				break;
		}
		if (a == 32)
			c++;
	} while (c < 32 && a == 32);

	int d = 31;
	a = 32;

	do {
		for (a = 0; a < 32; a++) {
			if (_levelBlockProperties[(d << 5) + a].flags)
				break;
		}
		if (a == 32)
			d--;
	} while (d > 0 && a == 32);

	_automapTopLeftY = (d > c) ? ((32 - (d - c)) >> 1) * 6 + 4 : 4;
	return (d > c) ? c : 0;
}

void LoLEngine::mapIncludeLegendData(int type) {
	type &= 0x7f;
	for (int i = 0; i < 11; i++) {
		if (_defaultLegendData[i].shapeIndex != type)
			continue;
		_defaultLegendData[i].enable = true;
		return;
	}
}

void LoLEngine::printMapText(uint16 stringId, int x, int y) {
	int cp = _screen->setCurPage(2);
	if (_flags.use16ColorMode)
		_screen->printText(getLangString(stringId), x & ~3, y & ~7, 1, 0);
	else
		_screen->printText(getLangString(stringId), x, y, 239, 0);
	_screen->setCurPage(cp);
}

void LoLEngine::printMapExitButtonText() {
	int cp = _screen->setCurPage(2);
	Screen::FontId of = _screen->setFont(Screen::FID_9_FNT);
	_screen->fprintString("%s", 295, 182, _flags.use16ColorMode ? 0xbb : 172, 0, 5, getLangString(0x4033));
	_screen->setFont(of);
	_screen->setCurPage(cp);
}



} // End of namespace Kyra

#endif // ENABLE_LOL

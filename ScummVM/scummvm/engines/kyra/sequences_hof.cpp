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

#include "kyra/kyra_hof.h"
#include "kyra/screen_hof.h"
#include "kyra/screen_lol.h"
#include "kyra/resource.h"
#include "kyra/sound.h"
#include "kyra/sequences_hof.h"
#include "kyra/timer.h"

#include "common/system.h"

namespace Kyra {

enum SequenceID {
	kSequenceNoLooping = -1,
	kSequenceVirgin = 0,
	kSequenceWestwood,
	kSequenceTitle,
	kSequenceOverview,
	kSequenceLibrary,
	kSequenceHand,
	kSequencePoint,
	kSequenceZanfaun,

	kSequenceFunters,
	kSequenceFerb,
	kSequenceFish,
	kSequenceFheep,
	kSequenceFarmer,
	kSequenceFuards,
	kSequenceFirates,
	kSequenceFrash,

	kSequenceHoFDemoVirgin,
	kSequenceHoFDemoWestwood,
	kSequenceHoFDemoTitle,
	kSequenceHoFDemoHill,
	kSequenceHoFDemoOuthome,
	kSequenceHoFDemoWharf,
	kSequenceHoFDemoDinob,
	kSequenceHoFDemoFisher,

// The following enums remain active even if LoL is disabled
	kSequenceLoLDemoScene1,
	kSequenceLoLDemoText1,
	kSequenceLoLDemoScene2,
	kSequenceLoLDemoText2,
	kSequenceLoLDemoScene3,
	kSequenceLoLDemoText3,
	kSequenceLoLDemoScene4,
	kSequenceLoLDemoText4,
	kSequenceLoLDemoScene5,
	kSequenceLoLDemoText5,
	kSequenceLoLDemoScene6,

	kSequenceArraySize
};

enum NestedSequenceID {
	kNestedSequenceFiggle = 0,

	kNestedSequenceOver1,
	kNestedSequenceOver2,
	kNestedSequenceForest,
	kNestedSequenceDragon,
	kNestedSequenceDarm,
	kNestedSequenceLibrary2,
	kNestedSequenceLibrary3,
	kNestedSequenceMarco,
	kNestedSequenceHand1a,
	kNestedSequenceHand1b,
	kNestedSequenceHand1c,
	kNestedSequenceHand2,
	kNestedSequenceHand3,
	kNestedSequenceHand4,

	kNestedSequenceHoFDemoWharf2,
	kNestedSequenceHoFDemoDinob2,
	kNestedSequenceHoFDemoWater,
	kNestedSequenceHoFDemoBail,
	kNestedSequenceHoFDemoDig,

	kNestedSequenceArraySize
};

typedef int (SeqPlayer_HOF::*SeqProc)(WSAMovie_v2 *, int, int, int);

struct SeqPlayerConfig {
	SeqPlayerConfig(const HoFSeqData *data, const SeqProc *callbacks, const SeqProc *nestedCallbacks) : seq(data->seq), seqProc(callbacks), numSeq(data->numSeq), nestedSeq(data->nestedSeq), nestedSeqProc(nestedCallbacks), numNestedSeq(data->numNestedSeq) {}
	const HoFSequence *seq;
	const SeqProc *seqProc;
	int numSeq;
	const HoFNestedSequence *nestedSeq;
	const SeqProc *nestedSeqProc;
	int numNestedSeq;
};

class SeqPlayer_HOF {
public:
	SeqPlayer_HOF(KyraEngine_v1 *vm, Screen_v2 *screen, OSystem *system, bool startupSaveLoadable = false);
	~SeqPlayer_HOF();

	int play(SequenceID firstScene, SequenceID loopStartScene);
	void pause(bool toggle);

	static SeqPlayer_HOF *instance() { return _instance; }

private:
	// Init
	void setupCallbacks();

	// Playback loop
	void runLoop();
	void playScenes();

	bool checkAbortPlayback();
	bool checkPlaybackStatus();

	bool _abortRequested;
	uint32 _pauseStart;

	// Sequence transitions
	void doTransition(int type);
	void nestedFrameAnimTransition(int srcPage, int dstPage, int delaytime, int steps, int x, int y, int w, int h, int openClose, int directionFlags);
	void nestedFrameFadeTransition(const char *cmpFile);

	// Animations
	void playAnimation(WSAMovie_v2 *wsaObj, int startFrame, int numFrames, int frameRate, int x, int y, const SeqProc callback, Palette *fadePal1, Palette *fadePal2, int fadeRate, bool restoreScreen);
	void playDialogueAnimation(uint16 strID, uint16 soundID, int textColor, int textPosX, int textPosY, int textWidth, WSAMovie_v2 *wsaObj, int animStartFrame, int animLastFrame, int animPosX, int animPosY);

	void startNestedAnimation(int animSlot, int sequenceID);
	void closeNestedAnimation(int animSlot);
	void unloadNestedAnimation(int animSlot);
	void doNestedFrameTransition(int transitionType, int animSlot);
	void updateAllNestedAnimations();
	bool updateNestedAnimation(int animSlot);

	struct AnimSlot {
		SeqProc callback;
		WSAMovie_v2 *movie;
		const FrameControl *control;
		int16 flags;
		uint16 startFrame;
		uint16 endFrame;
		uint16 frameDelay;
		uint32 nextFrame;
		uint16 currentFrame;
		uint16 lastFrame;
		uint16 x;
		uint16 y;
		uint16 fadeInTransitionType;
		uint16 fadeOutTransitionType;
	};

	AnimSlot _animSlots[8];

	bool _updateAnimations;
	uint32 _animDuration;
	int _animCurrentFrame;
	int _callbackCurrentFrame;

	// The only reason to declare these here (instead of just locally) is being able to increase them after pausing the Engine
	uint32 _specialAnimTimeOutTotal;
	uint32 _specialAnimFrameTimeOut;

	// Subtitles/Dialogue/Sound
	void playSoundEffect(uint16 id, int16 vol);
	void playSoundAndDisplaySubTitle(uint16 id);
	void printFadingText(uint16 strID, int x, int y, const uint8 *colorMap, uint8 textcolor);

	int displaySubTitle(uint16 strID, uint16 posX, uint16 posY, int duration, uint16 width);
	void updateSubTitles();
	char *preprocessString(const char *str, int width);
	void waitForSubTitlesTimeout();
	uint32 ticksTillSubTitlesTimeout();
	void resetAllTextSlots();

	void fadeOutMusic();

	struct TextSlot {
		uint16 strIndex;
		uint16 x;
		uint16 y;
		uint16 width;
		int32 duration;
		uint32 startTime;
		int16 textcolor;
	};

	TextSlot _textSlots[10];

	char *_tempString;

	uint8 _textColor[2];
	uint8 _textColorMap[16];
	int _textDuration[33];

	const char *const *_sequenceStrings;
	const char *const *_sequenceSoundList;
	int _sequenceSoundListSize;

	static const uint8 _textColorPresets[];

	// HOF credits
	void playHoFTalkieCredits();
	void displayHoFTalkieScrollText(uint8 *data, const ScreenDim *d, int tempPage1, int tempPage2, int speed, int step, Screen::FontId fid1, Screen::FontId fid2, const uint8 *shapeData = 0, const char *const *specialData = 0);

	bool _talkieFinaleExtraFlag;

	// HOF+LOL demo specific
	void updateDemoAdText(int bottom, int top);

	ActiveItemAnim _hofDemoActiveItemAnim[5];
	const HoFSeqItemAnimData *_hofDemoAnimData;

	uint32 _fisherAnimCurTime;
	int _scrollProgressCounter;

	uint8 *_hofDemoShapeData;
	uint8 *_hofDemoItemShapes[20];

	// Misc
	void delayTicks(uint32 ticks);
	void delayUntil(uint32 dest);
	void setCountDown(uint32 ticks);
	bool countDownRunning();

	uint32 _countDownRemainder;
	uint32 _countDownLastUpdate;

	enum SeqPlayerTargetInfo {
		kHoF = 0,
		kHoFDemo,
		kLoLDemo
	};

	SeqPlayerTargetInfo _target;
	int _firstScene, _loopStartScene, _curScene, _preventSkipBeforeScene, _lastScene;
	bool _startupSaveLoadable, _isFinale, _preventLooping;

	SeqPlayerConfig *_config;

	MainMenu *_menu;
	int _result;

	bool _abortPlayback;

	KyraEngine_v1 *_vm;
	Screen_v2 *_screen;
	// We might consider getting rid of Screen_HoF, since there are only 2 methods left in that class anyway
	Screen_HoF *_screenHoF;
	OSystem *_system;

	static SeqPlayer_HOF *_instance;

private:
	// Sequence specific callback functions
	int cbHOF_westwood(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOF_title(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOF_overview(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOF_library(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOF_hand(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOF_point(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOF_zanfaun(WSAMovie_v2 *wsaObj, int x, int y, int frm);

	int cbHOF_over1(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOF_over2(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOF_forest(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOF_dragon(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOF_darm(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOF_library2(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOF_marco(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOF_hand1a(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOF_hand1b(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOF_hand1c(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOF_hand2(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOF_hand3(WSAMovie_v2 *wsaObj, int x, int y, int frm);

	int cbHOF_funters(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOF_ferb(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOF_fish(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOF_fheep(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOF_farmer(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOF_fuards(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOF_firates(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOF_frash(WSAMovie_v2 *wsaObj, int x, int y, int frm);

	int cbHOF_figgle(WSAMovie_v2 *wsaObj, int x, int y, int frm);

	int cbHOFDEMO_virgin(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOFDEMO_westwood(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOFDEMO_title(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOFDEMO_hill(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOFDEMO_outhome(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOFDEMO_wharf(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOFDEMO_dinob(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOFDEMO_fisher(WSAMovie_v2 *wsaObj, int x, int y, int frm);

	int cbHOFDEMO_wharf2(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOFDEMO_dinob2(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOFDEMO_water(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOFDEMO_bail(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbHOFDEMO_dig(WSAMovie_v2 *wsaObj, int x, int y, int frm);

#ifdef ENABLE_LOL
	int cbLOLDEMO_scene1(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbLOLDEMO_scene2(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbLOLDEMO_scene3(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbLOLDEMO_scene4(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbLOLDEMO_scene5(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbLOLDEMO_text5(WSAMovie_v2 *wsaObj, int x, int y, int frm);
	int cbLOLDEMO_scene6(WSAMovie_v2 *wsaObj, int x, int y, int frm);
#endif // ENABLE_LOL
};

SeqPlayer_HOF *SeqPlayer_HOF::_instance = 0;

SeqPlayer_HOF::SeqPlayer_HOF(KyraEngine_v1 *vm, Screen_v2 *screen, OSystem *system, bool startupSaveLoadable) : _vm(vm), _screen(screen), _system(system), _startupSaveLoadable(startupSaveLoadable) {
	// We use a static pointer for pauseEngine functionality. Since we don't
	// ever need more than one SeqPlayer_HOF object at the same time we keep
	// this simple and just add an assert to detect typos, regressions, etc.
	assert(_instance == 0);

	memset(_animSlots, 0, sizeof(_animSlots));
	memset(_textSlots, 0, sizeof(_textSlots));
	memset(_hofDemoActiveItemAnim, 0, sizeof(_hofDemoActiveItemAnim));

	_screenHoF = _vm->game() == GI_KYRA2 ? (Screen_HoF*)screen : 0;
	_config = 0;
	_result = 0;
	_sequenceSoundList = 0;
	_hofDemoAnimData = 0;
	_hofDemoShapeData = 0;
	_isFinale = false;
	_preventLooping = false;
	_menu = 0;
	_abortRequested = false;
	_pauseStart = 0;

	_updateAnimations = false;
	_animDuration = 0;
	_animCurrentFrame = 0;
	_callbackCurrentFrame = 0;

	_abortPlayback = false;
	_curScene = 0;
	_preventSkipBeforeScene = -1;
	_lastScene = 0;

	_scrollProgressCounter = 0;
	_fisherAnimCurTime = 0;

	_tempString = new char[200];

	_countDownRemainder = 0;
	_countDownLastUpdate = 0;

	int tempSize = 0;
	_vm->resource()->unloadAllPakFiles();
	_vm->resource()->loadPakFile(StaticResource::staticDataFilename());
	const char *const *files = _vm->staticres()->loadStrings(k2SeqplayPakFiles, tempSize);
	_vm->resource()->loadFileList(files, tempSize);

	_sequenceStrings = _vm->staticres()->loadStrings(k2SeqplayStrings, tempSize);
	uint8 multiplier = (_vm->gameFlags().platform == Common::kPlatformFMTowns || _vm->gameFlags().platform == Common::kPlatformPC98) ? 12 : 8;
	for (int i = 0; i < MIN(33, tempSize); i++)
		_textDuration[i] = (int) strlen(_sequenceStrings[i]) * multiplier;

	if (_sequenceSoundList) {
		for (int i = 0; i < _sequenceSoundListSize; i++) {
			if (_sequenceSoundList[i])
				delete[] _sequenceSoundList[i];
		}
		delete[] _sequenceSoundList;
		_sequenceSoundList = 0;
	}

	const char *const *seqSoundList = _vm->staticres()->loadStrings(k2SeqplaySfxFiles, _sequenceSoundListSize);

	// replace sequence talkie files with localized versions
	const char *const *tlkfiles = _vm->staticres()->loadStrings(k2SeqplayTlkFiles, tempSize);
	char **tmpSndLst = new char *[_sequenceSoundListSize];

	for (int i = 0; i < _sequenceSoundListSize; i++) {
		const int len = strlen(seqSoundList[i]);

		tmpSndLst[i] = new char[len + 1];
		tmpSndLst[i][0] = 0;

		if (tlkfiles && len > 1) {
			for (int ii = 0; ii < tempSize; ii++) {
				if (strlen(tlkfiles[ii]) > 1 && !scumm_stricmp(&seqSoundList[i][1], &tlkfiles[ii][1]))
					strcpy(tmpSndLst[i], tlkfiles[ii]);
			}
		}

		if (tmpSndLst[i][0] == 0)
			strcpy(tmpSndLst[i], seqSoundList[i]);
	}

	tlkfiles = seqSoundList = 0;
	_vm->staticres()->unloadId(k2SeqplayTlkFiles);
	_vm->staticres()->unloadId(k2SeqplaySfxFiles);
	_sequenceSoundList = tmpSndLst;

	if (_vm->gameFlags().platform == Common::kPlatformPC98)
		_vm->sound()->loadSoundFile("SOUND.DAT");

	_screen->loadFont(_screen->FID_GOLDFONT_FNT, "GOLDFONT.FNT");
	_screen->setFont(_vm->gameFlags().lang == Common::JA_JPN ? Screen::FID_SJIS_FNT : Screen::FID_GOLDFONT_FNT);

	if (_vm->gameFlags().isDemo && !_vm->gameFlags().isTalkie) {
		if (_vm->game() == GI_KYRA2) {
			_hofDemoAnimData = _vm->staticres()->loadHoFSeqItemAnimData(k2SeqplayShapeAnimData, tempSize);
			uint8 *shp = _vm->resource()->fileData("ICONS.SHP", 0);
			uint32 outsize = READ_LE_UINT16(shp + 4);
			_hofDemoShapeData = new uint8[outsize];
			Screen::decodeFrame4(shp + 10, _hofDemoShapeData, outsize);
			for (int i = 0; i < 20; i++)
				_hofDemoItemShapes[i] = _screen->getPtrToShape(_hofDemoShapeData, i);
			delete[] shp;
		}
	} else {
		const MainMenu::StaticData data = {
			{ _sequenceStrings[97], _sequenceStrings[96], _sequenceStrings[95], _sequenceStrings[98], 0 },
			{ 0x01, 0x04, 0x0C, 0x04, 0x00, 0xd7, 0xd6 },
			{ 0xd8, 0xda, 0xd9, 0xd8 },
			(_vm->gameFlags().lang == Common::JA_JPN) ? Screen::FID_SJIS_FNT : Screen::FID_8_FNT, 240
		};

		_menu = new MainMenu(_vm);
		_menu->init(data, MainMenu::Animation());
	}

	_instance = this;
}

SeqPlayer_HOF::~SeqPlayer_HOF() {
	_instance = 0;

	if (_sequenceSoundList) {
		for (int i = 0; i < _sequenceSoundListSize; i++) {
			if (_sequenceSoundList[i])
				delete[] _sequenceSoundList[i];
		}
		delete[] _sequenceSoundList;
		_sequenceSoundList = NULL;
	}

	delete[] _tempString;
	delete[] _hofDemoShapeData;
	delete _menu;

	if (_vm->game() != GI_LOL)
		_screen->setFont(_vm->gameFlags().lang == Common::JA_JPN ? Screen::FID_SJIS_FNT : Screen::FID_8_FNT);
}

int SeqPlayer_HOF::play(SequenceID firstScene, SequenceID loopStartScene) {
	bool incompatibleData = false;
	AudioResourceSet soundSet = kMusicIntro;
	_firstScene = firstScene;
	_loopStartScene = loopStartScene;
	_preventLooping = false;
	_result = 0;

	if (firstScene >= kSequenceArraySize || firstScene < kSequenceVirgin || loopStartScene >= kSequenceArraySize || loopStartScene < kSequenceNoLooping) {
		return 0;
	} else if (firstScene >= kSequenceLoLDemoScene1) {
#ifndef ENABLE_LOL
		error("SeqPlayer_HOF::play(): The Lands of Lore sub engine (including this non-interactive demo) has been disabled in this build");
#endif
		incompatibleData = (_vm->game() != GI_LOL);
		_firstScene -= kSequenceLoLDemoScene1;
		if (loopStartScene != kSequenceNoLooping)
			_loopStartScene -= kSequenceLoLDemoScene1;
		_lastScene = kSequenceLoLDemoScene6 - kSequenceLoLDemoScene1;
		_target = kLoLDemo;
		_screen->_charWidth = 0;
	} else if (firstScene >= kSequenceHoFDemoVirgin) {
		incompatibleData = (_vm->game() != GI_KYRA2 || !_vm->gameFlags().isDemo || _vm->gameFlags().isTalkie);
		_firstScene -= kSequenceHoFDemoVirgin;
		if (loopStartScene != kSequenceNoLooping)
			_loopStartScene -= kSequenceHoFDemoVirgin;
		_lastScene = kSequenceHoFDemoFisher - kSequenceHoFDemoVirgin;
		_target = kHoFDemo;
		_screen->_charWidth = -2;
	} else {
		_isFinale = _preventLooping = firstScene > kSequenceZanfaun;
		incompatibleData = (_vm->game() != GI_KYRA2 || (_vm->gameFlags().isDemo && (!_vm->gameFlags().isTalkie || _isFinale)));
		_target = kHoF;
		_screen->_charWidth = -2;
		if (_isFinale) {
			soundSet = kMusicFinale;
			_lastScene = kSequenceFrash;
		} else {
			_lastScene = kSequenceZanfaun;
		}
	}

	if (incompatibleData)
		error("SeqPlayer_HOF::play(): Specified sequences do not match the available sequence data for this target");

	_vm->sound()->selectAudioResourceSet(soundSet);
	_vm->sound()->loadSoundFile(0);

	setupCallbacks();
	runLoop();

	return _result;
}

void SeqPlayer_HOF::pause(bool toggle) {
	if (toggle) {
		_pauseStart = _system->getMillis();
	} else {
		uint32 pausedTime = _system->getMillis() - _pauseStart;
		_pauseStart = 0;

		_countDownLastUpdate += pausedTime;
		_fisherAnimCurTime += pausedTime;
		_specialAnimTimeOutTotal += pausedTime;
		_specialAnimFrameTimeOut += pausedTime;

		for (int i = 0; i < 10; i++) {
			if (_textSlots[i].duration != -1)
				_textSlots[i].startTime += pausedTime;
		}

		for (int i = 0; i < 8; i++) {
			if (_animSlots[i].flags != -1)
				_animSlots[i].nextFrame += pausedTime;
		}
	}
}

void SeqPlayer_HOF::setupCallbacks() {
#define SCB(x) &SeqPlayer_HOF::cbHOF_##x
	static const SeqProc seqCallbacksHoF[] = { 0, SCB(westwood), SCB(title), SCB(overview), SCB(library), SCB(hand), SCB(point), SCB(zanfaun), SCB(funters), SCB(ferb), SCB(fish), SCB(fheep), SCB(farmer), SCB(fuards), SCB(firates), SCB(frash) };
	static const SeqProc nestedSeqCallbacksHoF[] = { SCB(figgle), SCB(over1), SCB(over2), SCB(forest), SCB(dragon), SCB(darm), SCB(library2), SCB(library2), SCB(marco), SCB(hand1a), SCB(hand1b), SCB(hand1c), SCB(hand2), SCB(hand3), 0 };
#undef SCB
#define SCB(x) &SeqPlayer_HOF::cbHOFDEMO_##x
	static const SeqProc seqCallbacksHoFDemo[] = {	SCB(virgin), SCB(westwood), SCB(title), SCB(hill), SCB(outhome), SCB(wharf), SCB(dinob), SCB(fisher) };
	static const SeqProc nestedSeqCallbacksHoFDemo[] = { SCB(wharf2), SCB(dinob2), SCB(water), SCB(bail), SCB(dig), 0 };
#undef SCB
#ifdef ENABLE_LOL
#define SCB(x) &SeqPlayer_HOF::cbLOLDEMO_##x
	static const SeqProc seqCallbacksLoLDemo[] = { SCB(scene1), 0, SCB(scene2), 0, SCB(scene3), 0, SCB(scene4), 0, SCB(scene5), SCB(text5), SCB(scene6), 0 };
#undef SCB
#else
	static const SeqProc seqCallbacksLoLDemo[] = { 0 };
#endif
	static const SeqProc nestedSeqCallbacksLoLDemo[] = { 0 };

	static const SeqProc *const seqCallbacks[] = { seqCallbacksHoF, seqCallbacksHoFDemo, seqCallbacksLoLDemo};
	static const SeqProc *const nestedSeqCallbacks[] = { nestedSeqCallbacksHoF, nestedSeqCallbacksHoFDemo, nestedSeqCallbacksLoLDemo};

	int tmpSize = 0;
	delete _config;
	_config = new SeqPlayerConfig(_vm->staticres()->loadHoFSequenceData(k2SeqplaySeqData, tmpSize), seqCallbacks[_target], nestedSeqCallbacks[_target]);
}

void SeqPlayer_HOF::runLoop() {
	memset(_animSlots, 0, sizeof(_animSlots));
	memset(_textSlots, 0, sizeof(_textSlots));
	memset(_hofDemoActiveItemAnim, 0, sizeof(_hofDemoActiveItemAnim));
	for (int i = 0; i < 8; ++i)
		_animSlots[i].flags = -1;

	_screen->clearPage(10);
	_screen->clearPage(12);
	_screen->hideMouse();
	int oldPage = _screen->setCurPage(2);

	for (int i = 0; i < 4; ++i)
		_screen->getPalette(i).clear();

	_updateAnimations = false;
	_animCurrentFrame = 0;
	_textColor[0] = _textColor[1] = 0;
	_curScene = _firstScene;

	do {
		playScenes();
		doTransition(0);
		resetAllTextSlots();
		fadeOutMusic();
		_firstScene = ((!_startupSaveLoadable || _preventLooping) && _curScene >= _loopStartScene) ? kSequenceNoLooping : _loopStartScene;
	} while (!_vm->shouldQuit() && _firstScene != kSequenceNoLooping);

	checkPlaybackStatus();

	for (int i = 0; i < 8; i++)
		unloadNestedAnimation(i);

	if (_vm->gameFlags().isDemo && !_vm->gameFlags().isTalkie)
		_screen->fadeToBlack();

	if (!_result)
		delayTicks(75);

	_screen->setCurPage(oldPage);
	_screen->_charWidth = 0;
	_screen->showMouse();
}

void SeqPlayer_HOF::playScenes() {
	_vm->sound()->stopAllSoundEffects();
	_curScene = _firstScene;

	_screen->copyPalette(1, 0);
	WSAMovie_v2 anim(_vm);
	_abortRequested = false;

	_scrollProgressCounter = 0;

	while (!_vm->shouldQuit()) {
		if (checkAbortPlayback())
			if (checkPlaybackStatus())
				break;

		_callbackCurrentFrame = 0;

		if (_curScene > _lastScene)
			break;

		const Kyra::HoFSequence &sq = _config->seq[_curScene];

		if (sq.flags & 2) {
			_screen->loadBitmap(sq.cpsFile, 2, 2, &_screen->getPalette(0));
			_screen->setScreenPalette(_screen->getPalette(0));
		} else {
			_screen->setCurPage(2);
			_screen->clearPage(2);
			_screen->loadPalette("GOLDFONT.COL", _screen->getPalette(0));
		}

		if (_config->seqProc[_curScene] && !(_vm->gameFlags().isDemo && !_vm->gameFlags().isTalkie))
			(this->*_config->seqProc[_curScene])(0, 0, 0, -1);

		if (sq.flags & 1) {
			anim.open(sq.wsaFile, 0, &_screen->getPalette(0));
			if (!(sq.flags & 2))
				anim.displayFrame(0, 2, sq.xPos, sq.yPos, 0x4000, 0, 0);
		}

		if (sq.flags & 4) {
			int cp = _screen->setCurPage(2);
			Screen::FontId cf =	_screen->setFont(_vm->gameFlags().lang == Common::JA_JPN ? Screen::FID_SJIS_FNT : Screen::FID_GOLDFONT_FNT);

			if (sq.stringIndex1 != -1)
				_screen->printText(_sequenceStrings[sq.stringIndex1], (320 - _screen->getTextWidth(_sequenceStrings[sq.stringIndex1])) / 2, 100 - _screen->getFontHeight(), 1, 0);

			if (sq.stringIndex2 != -1)
				_screen->printText(_sequenceStrings[sq.stringIndex2], (320 - _screen->getTextWidth(_sequenceStrings[sq.stringIndex2])) / 2, 100, 1, 0);

			_screen->setFont(cf);
			_screen->setCurPage(cp);
		}

		_screen->copyPage(2, 12);
		_screen->copyPage(0, 2);
		_screen->copyPage(2, 10);
		_screen->copyPage(12, 2);

		doTransition(sq.fadeInTransitionType);

		if (!(checkAbortPlayback() || _vm->shouldQuit() || _result)) {
			_screen->copyPage(2, 0);
			_screen->updateScreen();
		}

		//_screen->copyPage(2, 6);

		if (sq.flags & 1) {
			playAnimation(&anim, sq.startFrame, sq.numFrames, sq.duration, sq.xPos, sq.yPos, _config->seqProc[_curScene], &_screen->getPalette(1), &_screen->getPalette(0), 30, 0);
			anim.close();
		} else {
			_animDuration = sq.duration;
			setCountDown(_animDuration);

			while (!checkAbortPlayback() && !_vm->shouldQuit() && (countDownRunning() || _updateAnimations)) {
				uint32 endFrame = (_system->getMillis() + _vm->tickLength()) & ~(_vm->tickLength() - 1);
				updateAllNestedAnimations();

				if (_config->seqProc[_curScene])
					(this->*_config->seqProc[_curScene])(0, 0, 0, 0);

				updateSubTitles();

				_screen->copyPage(2, 0);
				_screen->updateScreen();
				_screen->copyPage(12, 2);

				do {
					if (checkAbortPlayback())
						if (checkPlaybackStatus())
							break;
				} while (_system->getMillis() < endFrame);
			}
		}

		if (_config->seqProc[_curScene] && !(_vm->gameFlags().isDemo && !_vm->gameFlags().isTalkie))
			(this->*_config->seqProc[_curScene])(0, 0, 0, -2);

		uint32 textTimeOut = ticksTillSubTitlesTimeout();
		setCountDown(sq.timeout < textTimeOut ? textTimeOut : sq.timeout);

		while (!checkAbortPlayback() && !_vm->shouldQuit() && (countDownRunning() || _updateAnimations)) {
			updateAllNestedAnimations();
			_screen->copyPage(2, 0);
			_screen->updateScreen();
			_screen->copyPage(12, 2);
		}

		doTransition(sq.fadeOutTransitionType);
		_curScene++;
	}

	resetAllTextSlots();
	_vm->sound()->haltTrack();
	_vm->sound()->voiceStop();

	if ((!checkAbortPlayback() || _vm->shouldQuit()) && _vm->gameFlags().isDemo)
		_curScene = -1;
}

bool SeqPlayer_HOF::checkAbortPlayback() {
	Common::Event event;

	if (_vm->skipFlag()) {
		_abortRequested = true;
		_vm->resetSkipFlag();
	}

	if (_abortRequested)
		return true;

	while (_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == Common::KEYCODE_q && event.kbd.hasFlags(Common::KBD_CTRL)) {
				_abortRequested = true;
				_vm->quitGame();
				return true;
			} else if (event.kbd.keycode != Common::KEYCODE_ESCAPE && event.kbd.keycode != Common::KEYCODE_RETURN && event.kbd.keycode != Common::KEYCODE_SPACE) {
				continue;
			}
			// fall through
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_RBUTTONDOWN:
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONUP:
			_abortRequested = true;
			return true;
		default:
			break;
		}
	}

	return false;
}

bool SeqPlayer_HOF::checkPlaybackStatus() {
	_updateAnimations = false;

	if (_curScene <= _preventSkipBeforeScene || (_curScene == _loopStartScene && !_isFinale)) {
		_abortRequested = false;
		return false;
	}

	if (_loopStartScene == kSequenceNoLooping) {
		doTransition(0);
		fadeOutMusic();
		_abortPlayback = true;
	} else {
		return true;
	}

	return false;
}

void SeqPlayer_HOF::doTransition(int type) {
	for (int i = 0; i < 8; i++)
		closeNestedAnimation(i);

	switch (type) {
	case 0:
		_screen->fadeToBlack(36);
		_screen->getPalette(0).clear();
		_screen->getPalette(1).clear();
		break;

	case 1:
		playSoundAndDisplaySubTitle(_vm->_rnd.getRandomBit());

		_screen->getPalette(0).fill(0, 256, 0x3F);
		_screen->fadePalette(_screen->getPalette(0), 16);

		_screen->copyPalette(1, 0);
		break;

	case 3:
		_screen->copyPage(2, 0);
		_screen->fadePalette(_screen->getPalette(0), 16);
		_screen->copyPalette(1, 0);
		break;

	case 4:
		_screen->copyPage(2, 0);
		_screen->fadePalette(_screen->getPalette(0), 36);
		_screen->copyPalette(1, 0);
		break;

	case 5:
		_screen->copyPage(2, 0);
		break;

	case 6:
		// UNUSED
		// seq_loadBLD("library.bld");
		break;

	case 7:
		// UNUSED
		// seq_loadBLD("marco.bld");
		break;

	case 8:
		_screen->fadeToBlack(16);
		_screen->getPalette(0).clear();
		_screen->getPalette(1).clear();

		delayTicks(120);
		break;

	case 9: {
		Palette &pal = _screen->getPalette(0);
		for (int i = 0; i < 255; i++)
			pal.fill(i, 1, (pal[3 * i] + pal[3 * i + 1] + pal[3 * i + 2]) / 3);
		pal.fill(255, 1, 0x3F);

		_screen->fadePalette(pal, 64);
		_screen->copyPalette(1, 0);
		} break;

	default:
		break;
	}
}

void SeqPlayer_HOF::nestedFrameAnimTransition(int srcPage, int dstPage, int delaytime, int steps, int x, int y, int w, int h, int openClose, int directionFlags) {
	if (openClose) {
		for (int i = 1; i < steps; i++) {
			uint32 endtime = _system->getMillis() + delaytime * _vm->tickLength();

			int w2 = (((w * 256) / steps) * i) / 256;
			int h2 = (((h * 256) / steps) * i) / 256;

			int ym = (directionFlags & 2) ? (h - h2) : 0;
			int xm = (directionFlags & 1) ? (w - w2) : 0;

			_screen->wsaFrameAnimationStep(0, 0, x + xm, y + ym, w, h, w2, h2, srcPage, dstPage, 0);

			_screen->copyPage(dstPage, 6);
			_screen->copyPage(dstPage, 0);
			_screen->updateScreen();

			_screen->copyPage(12, dstPage);
			delayUntil(endtime);
		}

		_screen->wsaFrameAnimationStep(0, 0, x, y, w, h, w, h, srcPage, dstPage, 0);
		_screen->copyPage(dstPage, 6);
		_screen->copyPage(dstPage, 0);
		_screen->updateScreen();
	} else {
		_screen->copyPage(12, dstPage);
		for (int i = steps; i; i--) {
			uint32 endtime = _system->getMillis() + delaytime * _vm->tickLength();

			int w2 = (((w * 256) / steps) * i) / 256;
			int h2 = (((h * 256) / steps) * i) / 256;

			int ym = (directionFlags & 2) ? (h - h2) : 0;
			int xm = (directionFlags & 1) ? (w - w2) : 0;

			_screen->wsaFrameAnimationStep(0, 0, x + xm, y + ym, w, h, w2, h2, srcPage, dstPage, 0);

			_screen->copyPage(dstPage, 6);
			_screen->copyPage(dstPage, 0);
			_screen->updateScreen();

			_screen->copyPage(12, dstPage);
			delayUntil(endtime);
		}
	}
}

void SeqPlayer_HOF::nestedFrameFadeTransition(const char *cmpFile) {
	_screen->copyPage(10, 2);
	_screen->copyPage(4, 10);
	_screen->clearPage(6);
	_screen->loadBitmap(cmpFile, 6, 6, 0);
	_screen->copyPage(12, 4);

	for (int i = 0; i < 3; i++) {
		uint32 endtime = _system->getMillis() + 4 * _vm->tickLength();
		assert(_screenHoF);
		_screenHoF->cmpFadeFrameStep(4, 320, 200, 0, 0, 2, 320, 200, 0, 0, 320, 200, 6);
		_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0);
		_screen->updateScreen();
		delayUntil(endtime);
	}

	_screen->copyPage(4, 0);
	_screen->updateScreen();
	_screen->copyPage(4, 2);
	_screen->copyPage(4, 6);
	_screen->copyPage(10, 4);
}

void SeqPlayer_HOF::playAnimation(WSAMovie_v2 *wsaObj, int startFrame, int lastFrame, int frameRate, int x, int y, const SeqProc callback, Palette *fadePal1, Palette *fadePal2, int fadeRate, bool restoreScreen) {
	bool finished = false;
	uint32 startTime = _system->getMillis();

	int origW = wsaObj->width();
	int origH = wsaObj->width();
	int drwX = x;
	int drwY = y;
	int drwW = origW;
	int drwH = origH;

	_animDuration = frameRate;

	if (wsaObj) {
		if (x < 0) {
			drwW += x;
			drwX = 0;
		}

		if (y < 0) {
			drwH += y;
			drwY = 0;
		}

		if (x + origW > 319)
			origW = 320 - x;

		if (y + origH > 199)
			origW = 200 - y;
	}

	int8 frameStep = (startFrame > lastFrame) ? -1 : 1;
	_animCurrentFrame = startFrame;

	while (!_vm->shouldQuit() && !finished) {
		if (checkAbortPlayback())
			if (checkPlaybackStatus())
				break;

		setCountDown(_animDuration);

		if (wsaObj || callback)
			_screen->copyPage(12, 2);

		int frameIndex = _animCurrentFrame;
		if (wsaObj)
			frameIndex %= wsaObj->frames();

		if (callback)
			(this->*callback)(wsaObj, x, y, frameIndex);

		if (wsaObj)
			wsaObj->displayFrame(frameIndex, 2, x, y, 0, 0, 0);

		_screen->copyPage(2, 12);

		updateAllNestedAnimations();
		updateSubTitles();

		if ((wsaObj || callback) && (!(checkAbortPlayback() || _vm->shouldQuit() || _result))) {
			_screen->copyPage(2, 0);
			_screen->updateScreen();
		}

		while (!_vm->shouldQuit()) {
			if (checkAbortPlayback())
				if (checkPlaybackStatus())
					break;

			if (fadePal1 && fadePal2) {
				if (!_screen->timedPaletteFadeStep(fadePal1->getData(), fadePal2->getData(), _system->getMillis() - startTime, fadeRate * _vm->tickLength()) && !wsaObj)
					break;
			}

			if ((wsaObj || callback) && (!(checkAbortPlayback() || _vm->shouldQuit() || _result))) {
				_screen->copyPage(2, 0);
				_screen->updateScreen();
			}

			updateSubTitles();

			if (!countDownRunning())
				break;
		}

		if (wsaObj) {
			_animCurrentFrame += frameStep;
			if ((frameStep > 0 && _animCurrentFrame >= lastFrame) || (frameStep < 0 && _animCurrentFrame < lastFrame))
				finished = true;
		}

		if (restoreScreen && (wsaObj || callback)) {
			_screen->copyPage(12, 2);
			_screen->copyRegion(drwX, drwY, drwX, drwY, drwW, drwH, 2, 0, Screen::CR_NO_P_CHECK);
			_screen->updateScreen();
		}
	}
}

void SeqPlayer_HOF::playDialogueAnimation(uint16 strID, uint16 soundID, int textColor, int textPosX, int textPosY, int textWidth, WSAMovie_v2 *wsaObj, int animStartFrame, int animLastFrame, int animPosX, int animPosY) {
	int dur = int(strlen(_sequenceStrings[strID])) * (_vm->gameFlags().isTalkie ? 7 : 15);
	if (_vm->textEnabled()) {
		int slot = displaySubTitle(strID, textPosX, textPosY, dur, textWidth);
		_textSlots[slot].textcolor = textColor;
	}
	_specialAnimTimeOutTotal = _system->getMillis() + dur * _vm->tickLength();
	int curframe = animStartFrame;

	if (soundID && _vm->speechEnabled()) {
		while (_vm->sound()->voiceIsPlaying() && !_abortPlayback)
			delayTicks(1);
		playSoundAndDisplaySubTitle(soundID);
	}

	while (_system->getMillis() < _specialAnimTimeOutTotal && !_abortPlayback) {
		if (animLastFrame < 0) {
			int t = ABS(animLastFrame);
			if (t < curframe)
				curframe = t;
		}

		if (ABS(animLastFrame) < curframe)
			curframe = animStartFrame;

		_specialAnimFrameTimeOut = _system->getMillis() + _animDuration * _vm->tickLength();
		setCountDown(_animDuration);

		if (wsaObj)
			wsaObj->displayFrame(curframe % wsaObj->frames(), 2, animPosX, animPosY, 0, 0, 0);

		_screen->copyPage(2, 12);
		updateSubTitles();
		delayUntil(MIN(_specialAnimFrameTimeOut, _specialAnimTimeOutTotal));

		if (_vm->speechEnabled() && !_vm->textEnabled() && !_vm->snd_voiceIsPlaying())
			break;

		if (checkAbortPlayback())
			if (checkPlaybackStatus())
				break;

		_screen->copyPage(2, 0);
		_screen->updateScreen();
		curframe++;
	}

	if (_abortPlayback)
		_vm->sound()->voiceStop();

	if (ABS(animLastFrame) < curframe)
		curframe = ABS(animLastFrame);

	if (curframe == animStartFrame)
		curframe++;

	_animCurrentFrame = curframe;
}

void SeqPlayer_HOF::startNestedAnimation(int animSlot, int sequenceID) {
	if (_animSlots[animSlot].flags != -1)
		return;

	if (_target == kLoLDemo) {
		 return;
	} else if (_target == kHoFDemo) {
		assert(sequenceID >= kNestedSequenceHoFDemoWharf2);
		sequenceID -= kNestedSequenceHoFDemoWharf2;
	}

	HoFNestedSequence s = _config->nestedSeq[sequenceID];

	if (!_animSlots[animSlot].movie) {
		_animSlots[animSlot].movie = new WSAMovie_v2(_vm);
		assert(_animSlots[animSlot].movie);
	}

	_animSlots[animSlot].movie->close();

	_animSlots[animSlot].movie->open(s.wsaFile, 0, 0);

	if (!_animSlots[animSlot].movie->opened()) {
		delete _animSlots[animSlot].movie;
		_animSlots[animSlot].movie = 0;
		return;
	}

	_animSlots[animSlot].endFrame = s.endFrame;
	_animSlots[animSlot].startFrame = _animSlots[animSlot].currentFrame = s.startframe;
	_animSlots[animSlot].frameDelay = s.frameDelay;
	_animSlots[animSlot].callback = _config->nestedSeqProc[sequenceID];
	_animSlots[animSlot].control = s.wsaControl;

	_animSlots[animSlot].flags = s.flags | 1;
	_animSlots[animSlot].x = s.x;
	_animSlots[animSlot].y = s.y;
	_animSlots[animSlot].fadeInTransitionType = s.fadeInTransitionType;
	_animSlots[animSlot].fadeOutTransitionType = s.fadeOutTransitionType;
	_animSlots[animSlot].lastFrame = 0xffff;

	doNestedFrameTransition(s.fadeInTransitionType, animSlot);

	if (!s.fadeInTransitionType)
		updateNestedAnimation(animSlot);

	_animSlots[animSlot].nextFrame = _system->getMillis() & ~(_vm->tickLength() - 1);
}

void SeqPlayer_HOF::closeNestedAnimation(int animSlot) {
	if (_animSlots[animSlot].flags == -1)
		return;

	_animSlots[animSlot].flags = -1;
	doNestedFrameTransition(_animSlots[animSlot].fadeOutTransitionType, animSlot);
	_animSlots[animSlot].movie->close();
}

void SeqPlayer_HOF::unloadNestedAnimation(int animSlot) {
	if (_animSlots[animSlot].movie) {
		_animSlots[animSlot].movie->close();
		delete _animSlots[animSlot].movie;
		_animSlots[animSlot].movie = 0;
	}
}

void SeqPlayer_HOF::doNestedFrameTransition(int transitionType, int animSlot) {
	int xa = 0, ya = 0;
	transitionType--;
	if (!_animSlots[animSlot].movie || _abortPlayback || _vm->shouldQuit())
		return;

	switch (transitionType) {
	case 0:
		xa = -_animSlots[animSlot].movie->xAdd();
		ya = -_animSlots[animSlot].movie->yAdd();
		_animSlots[animSlot].movie->displayFrame(0, 8, xa, ya, 0, 0, 0);
		nestedFrameAnimTransition(8, 2, 7, 8, _animSlots[animSlot].movie->xAdd(), _animSlots[animSlot].movie->yAdd(),
							_animSlots[animSlot].movie->width(), _animSlots[animSlot].movie->height(), 1, 2);
		break;

	case 1:
		xa = -_animSlots[animSlot].movie->xAdd();
		ya = -_animSlots[animSlot].movie->yAdd();
		_animSlots[animSlot].movie->displayFrame(0, 8, xa, ya, 0, 0, 0);
		nestedFrameAnimTransition(8, 2, 7, 8, _animSlots[animSlot].movie->xAdd(), _animSlots[animSlot].movie->yAdd(),
							_animSlots[animSlot].movie->width(), _animSlots[animSlot].movie->height(), 1, 1);
		break;

	case 2:
		waitForSubTitlesTimeout();
		xa = -_animSlots[animSlot].movie->xAdd();
		ya = -_animSlots[animSlot].movie->yAdd();
		_animSlots[animSlot].movie->displayFrame(21, 8, xa, ya, 0, 0, 0);
		nestedFrameAnimTransition(8, 2, 7, 8, _animSlots[animSlot].movie->xAdd(), _animSlots[animSlot].movie->yAdd(),
							_animSlots[animSlot].movie->width(), _animSlots[animSlot].movie->height(), 0, 2);
		break;

	case 3:
		_screen->copyPage(2, 10);
		_animSlots[animSlot].movie->displayFrame(0, 2, 0, 0, 0, 0, 0);
		_screen->copyPage(2, 12);
		nestedFrameFadeTransition("scene2.cmp");
		break;

	case 4:
		_screen->copyPage(2, 10);
		_animSlots[animSlot].movie->displayFrame(0, 2, 0, 0, 0, 0, 0);
		_screen->copyPage(2, 12);
		nestedFrameFadeTransition("scene3.cmp");
		break;

	default:
		break;
	}
}

void SeqPlayer_HOF::updateAllNestedAnimations() {
	for (int i = 0; i < 8; i++) {
		if (_animSlots[i].flags != -1) {
			if (updateNestedAnimation(i))
				closeNestedAnimation(i);
		}
	}
}

bool SeqPlayer_HOF::updateNestedAnimation(int animSlot) {
	uint16 currentFrame = _animSlots[animSlot].currentFrame;
	uint32 curTick = _system->getMillis() & ~(_vm->tickLength() - 1);

	if (_animSlots[animSlot].callback && currentFrame != _animSlots[animSlot].lastFrame) {
		_animSlots[animSlot].lastFrame = currentFrame;
		currentFrame = (this->*_animSlots[animSlot].callback)(_animSlots[animSlot].movie, _animSlots[animSlot].x, _animSlots[animSlot].y, currentFrame);
	}

	if (_animSlots[animSlot].movie) {
		if (_animSlots[animSlot].flags & 0x20) {
			_animSlots[animSlot].movie->displayFrame(_animSlots[animSlot].control[currentFrame].index, 2, _animSlots[animSlot].x, _animSlots[animSlot].y, 0x4000, 0, 0);
			_animSlots[animSlot].frameDelay = _animSlots[animSlot].control[currentFrame].delay;
		} else {
			_animSlots[animSlot].movie->displayFrame(currentFrame % _animSlots[animSlot].movie->frames(), 2, _animSlots[animSlot].x, _animSlots[animSlot].y, 0x4000, 0, 0);
		}
	}

	if (_animSlots[animSlot].flags & 0x10) {
		currentFrame = (curTick - _animSlots[animSlot].nextFrame) / (_animSlots[animSlot].frameDelay * _vm->tickLength());
	} else {
		int diff = (curTick - _animSlots[animSlot].nextFrame) / (_animSlots[animSlot].frameDelay * _vm->tickLength());
		if (diff > 0) {
			currentFrame++;
			if (_vm->gameFlags().platform == Common::kPlatformFMTowns || _vm->gameFlags().platform == Common::kPlatformPC98)
				_animSlots[animSlot].nextFrame += ((curTick - _animSlots[animSlot].nextFrame) * 2 / 3);
			else
				_animSlots[animSlot].nextFrame = curTick;
		}
	}

	bool res = false;

	if (currentFrame >= _animSlots[animSlot].endFrame) {
		int sw = ((_animSlots[animSlot].flags & 0x1e) - 2);
		switch (sw) {
		case 0:
			res = true;
			currentFrame = _animSlots[animSlot].endFrame;
			_screen->copyPage(2, 12);
			break;

		case 6:
		case 8:
			currentFrame = _animSlots[animSlot].endFrame - 1;
			break;

		case 2:
		case 10:
			currentFrame = _animSlots[animSlot].startFrame;
			break;

		default:
			currentFrame = _animSlots[animSlot].endFrame - 1;
			res = true;
		}
	}

	_animSlots[animSlot].currentFrame = currentFrame;
	return res;
}

void SeqPlayer_HOF::playSoundEffect(uint16 id, int16 vol) {
	assert(id < _sequenceSoundListSize);
	_vm->sound()->voicePlay(_sequenceSoundList[id], 0, vol);
}

void SeqPlayer_HOF::playSoundAndDisplaySubTitle(uint16 id) {
	assert(id < _sequenceSoundListSize);

	if (id < 12 && !_vm->gameFlags().isDemo && _vm->textEnabled())
		displaySubTitle(id, 160, 168, _textDuration[id], 160);

	_vm->sound()->voicePlay(_sequenceSoundList[id], 0);
}

void SeqPlayer_HOF::printFadingText(uint16 strID, int x, int y, const uint8 *colorMap, uint8 textcolor) {
	uint8 cmap[16];

	if (checkAbortPlayback())
		checkPlaybackStatus();

	if (_abortPlayback || _abortRequested || _vm->shouldQuit() || _result)
		return;

	Screen::FontId of = _screen->setFont(Screen::FID_8_FNT);
	_screen->getPalette(0).fill(254, 2, 63);
	_screen->setPaletteIndex(252, 63, 32, 48);
	cmap[0] = colorMap[0];
	cmap[1] = 253;
	memcpy(&cmap[2], &colorMap[2], 14);
	uint8 col0 = _textColor[0];

	_textColor[0] = 253;
	_screen->setTextColorMap(cmap);
	resetAllTextSlots();
	displaySubTitle(strID, x, y, 128, 120);
	updateSubTitles();
	_screen->copyPage(2, 0);
	_screen->updateScreen();
	_screen->getPalette(0).copy(_screen->getPalette(0), textcolor, 1, 253);
	_screen->fadePalette(_screen->getPalette(0), 24);

	_textColor[0] = textcolor;
	_screen->setTextColorMap(colorMap);
	resetAllTextSlots();
	displaySubTitle(strID, x, y, 128, 120);
	updateSubTitles();
	_screen->copyPage(2, 0);
	_screen->updateScreen();
	_screen->getPalette(0).fill(253, 1, 0);
	_screen->fadePalette(_screen->getPalette(0), 1);

	_screen->copyPage(2, 12);
	resetAllTextSlots();

	_textColor[0] = col0;

	_screen->setFont(of);
}

int SeqPlayer_HOF::displaySubTitle(uint16 strIndex, uint16 posX, uint16 posY, int duration, uint16 width) {
	for (int i = 0; i < 10; i++) {
		if (_textSlots[i].duration != -1) {
			if (i < 9)
				continue;
			else
				return -1;
		}

		_textSlots[i].strIndex = strIndex;
		_textSlots[i].x = posX;
		_textSlots[i].y = posY;
		_textSlots[i].duration = duration * _vm->tickLength();
		_textSlots[i].width = width;
		_textSlots[i].startTime = _system->getMillis();
		_textSlots[i].textcolor = -1;

		return i;
	}
	return -1;
}

void SeqPlayer_HOF::updateSubTitles() {
	int curPage = _screen->setCurPage(2);
	char outputStr[70];

	for (int i = 0; i < 10; i++) {
		if (_textSlots[i].startTime + _textSlots[i].duration > _system->getMillis() && _textSlots[i].duration != -1) {

			char *srcStr = preprocessString(_sequenceStrings[_textSlots[i].strIndex], _textSlots[i].width);
			int yPos = _textSlots[i].y;

			while (*srcStr) {
				uint32 linePos = 0;
				for (; *srcStr; linePos++) {
					if (*srcStr == '\r')
						break;
					outputStr[linePos] = *srcStr;
					srcStr++;
				}
				outputStr[linePos] = 0;
				if (*srcStr == '\r')
					srcStr++;

				uint8 textColor = (_textSlots[i].textcolor >= 0) ? _textSlots[i].textcolor : _textColor[0];
				_screen->printText(outputStr, _textSlots[i].x - (_screen->getTextWidth(outputStr) / 2), yPos, textColor, 0);
				yPos += 10;
			}
		} else {
			_textSlots[i].duration = -1;
		}
	}

	_screen->setCurPage(curPage);
}

char *SeqPlayer_HOF::preprocessString(const char *srcStr, int width) {
	char *dstStr = _tempString;
	int lineStart = 0;
	int linePos = 0;

	while (*srcStr) {
		while (*srcStr && *srcStr != ' ')
			dstStr[lineStart + linePos++] = *srcStr++;
		dstStr[lineStart + linePos] = 0;

		int len = _screen->getTextWidth(&dstStr[lineStart]);
		if (width >= len && *srcStr) {
			dstStr[lineStart + linePos++] = *srcStr++;
		} else {
			dstStr[lineStart + linePos] = '\r';
			lineStart += linePos + 1;
			linePos = 0;
			if (*srcStr)
				srcStr++;
		}
	}
	dstStr[lineStart + linePos] = 0;

	return strlen(_tempString) ? dstStr : 0;
}

void SeqPlayer_HOF::waitForSubTitlesTimeout() {
	uint32 timeOut = _system->getMillis() + ticksTillSubTitlesTimeout() * _vm->tickLength();

	if (_vm->textEnabled()) {
		delayUntil(timeOut);
	} else if (_vm->speechEnabled()) {
		while (_vm->snd_voiceIsPlaying())
			delayTicks(1);
	}

	resetAllTextSlots();
}

uint32 SeqPlayer_HOF::ticksTillSubTitlesTimeout() {
	uint32 longest = 0;

	for (int i = 0; i < 10; i++) {
		uint32 timeOut = (_textSlots[i].duration + _textSlots[i].startTime);
		uint32 curtime = _system->getMillis();
		if (_textSlots[i].duration != -1 && timeOut > curtime) {
			timeOut -= curtime;
			if (longest < timeOut)
				longest = timeOut;
		}
	}

	uint32 tl = _vm->tickLength();
	return (longest + (tl - 1)) / tl;
}

void SeqPlayer_HOF::resetAllTextSlots() {
	for (int i = 0; i < 10; i++)
		_textSlots[i].duration = -1;
}

void SeqPlayer_HOF::fadeOutMusic() {
	_vm->sound()->beginFadeOut();
	delayTicks(80);
}

void SeqPlayer_HOF::playHoFTalkieCredits() {
	static const uint8 colormap[] = {0, 0, 102, 102, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static const ScreenDim d = { 0x00, 0x0C, 0x28, 0xB4, 0xFF, 0x00, 0x00, 0x00 };

	_screen->loadBitmap("finale.cps", 3, 3, &_screen->getPalette(0));
	_screen->setFont(Screen::FID_GOLDFONT_FNT);

	int talkieCreditsSize, talkieCreditsSpecialSize;
	const uint8 *talkieCredits = _vm->staticres()->loadRawData(k2SeqplayCredits, talkieCreditsSize);
	const char *const *talkieCreditsSpecial = _vm->staticres()->loadStrings(k2SeqplayCreditsSpecial, talkieCreditsSpecialSize);

	_vm->sound()->selectAudioResourceSet(kMusicIngame);
	_vm->sound()->loadSoundFile(3);
	_vm->sound()->playTrack(3);

	_screen->setTextColorMap(colormap);
	_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0);
	_screen->updateScreen();
	_screen->fadeFromBlack();

	_screen->_charWidth = -2;
	uint8 *dataPtr = new uint8[0xafd];
	memcpy(dataPtr, talkieCredits, talkieCreditsSize);
	_vm->staticres()->unloadId(k2SeqplayCredits);

	displayHoFTalkieScrollText(dataPtr, &d, 2, 6, 5, 1, Screen::FID_GOLDFONT_FNT, Screen::FID_GOLDFONT_FNT, 0, talkieCreditsSpecial);
	delayTicks(8);

	delete[] dataPtr;
	_vm->staticres()->unloadId(k2SeqplayCreditsSpecial);
	_vm->sound()->selectAudioResourceSet(kMusicFinale);
	_vm->sound()->loadSoundFile(0);
}

void SeqPlayer_HOF::displayHoFTalkieScrollText(uint8 *data, const ScreenDim *d, int tempPage1, int tempPage2, int speed,
	int step, Screen::FontId fid1, Screen::FontId fid2, const uint8 *shapeData, const char *const *specialData) {
	if (!data)
		return;

	static const char mark[] = { 5, 13, 0 };

	_screen->clearPage(tempPage1);
	_screen->clearPage(tempPage2);
	_screen->copyRegion(d->sx << 3, d->sy, d->sx << 3, d->sy, d->w << 3, d->h, 0, tempPage1);

	struct ScrollTextData {
		int16	x;
		int16	y;
		uint8	*text;
		byte	unk1;
		byte	height;
		byte	adjust;

		ScrollTextData() {
			x = 0;      // 0  11
			y = 0;		// 2  13
			text = 0;	// 4  15
			unk1 = 0;   // 8  19
			height = 0; // 9  20
			adjust = 0; // 10 21
		}
	};

	ScrollTextData *textData = new ScrollTextData[36];
	uint8 *ptr = data;

	bool loop = true;
	int cnt = 0;

	while (loop) {
		uint32 loopEnd = _system->getMillis() + speed * _vm->tickLength();

		while (cnt < 35 && *ptr) {
			uint16 cH;

			if (cnt)
				cH = textData[cnt].y + textData[cnt].height + (textData[cnt].height >> 3);
			else
				cH = d->h;

			char *str = (char *)ptr;

			ptr = (uint8 *)strpbrk(str, mark);
			if (!ptr)
				ptr = (uint8 *)strchr(str, 0);

			textData[cnt + 1].unk1 = *ptr;
			*ptr = 0;
			if (textData[cnt + 1].unk1)
				ptr++;

			if (*str == 3 || *str == 4)
				textData[cnt + 1].adjust = *str++;
			else
				textData[cnt + 1].adjust = 0;

			_screen->setFont(fid1);

			if (*str == 1) {
				_screen->setFont(fid2);
				str++;
			} else if (*str == 2) {
				str++;
			}

			textData[cnt + 1].height = _screen->getFontHeight();

			switch (textData[cnt + 1].adjust) {
			case 3:
				textData[cnt + 1].x = 157 - _screen->getTextWidth(str);
				break;
			case 4:
				textData[cnt + 1].x = 161;
				break;
			default:
				textData[cnt + 1].x = (((d->w << 3) - _screen->getTextWidth(str)) >> 1) + 1;
			}

			if (textData[cnt].unk1 == 5)
				cH -= (textData[cnt].height + (textData[cnt].height >> 3));

			textData[cnt + 1].y = cH;
			textData[cnt + 1].text = (uint8 *)str;
			cnt++;
		}

		_screen->copyRegion(d->sx << 3, d->sy, d->sx << 3, d->sy, d->w << 3, d->h, tempPage1, tempPage2);

		int cnt2 = 0;
		bool palCycle = 0;

		while (cnt2 < cnt) {
			const char *str = (const char *)textData[cnt2 + 1].text;
			const char *str2 = str;
			int16 cW = textData[cnt2 + 1].x - 10;
			int16 cH = textData[cnt2 + 1].y;
			int x = (d->sx << 3) + cW;
			int y = d->sy + cH;
			int col1 = 255;

			if (cH < d->h) {
				_screen->setCurPage(tempPage2);
				_screen->setFont(fid1);
				if (textData[cnt2 + 1].height != _screen->getFontHeight())
					_screen->setFont(fid2);

				if (specialData) {
					if (!strcmp(str, specialData[0])) {
						col1 = 112;
						char cChar[2] = " ";
						while (*str2) {
							cChar[0] = *str2;
							_screen->printText(cChar, x, y, col1++, 0);
							x += _screen->getCharWidth((uint8)*str2++);
						}
						palCycle = true;
					} else if (!strcmp(str, specialData[1])) {
						col1 = 133;
						char cChar[2] = " ";
						while (*str2) {
							cChar[0] = *str2;
							_screen->printText(cChar, x, y, col1--, 0);
							x += _screen->getCharWidth((uint8)*str2++);
						}
						palCycle = true;
					} else {
						_screen->printText(str, x, y, col1, 0);
					}
				} else {
					_screen->printText(str, x, y, col1, 0);
				}
				_screen->setCurPage(0);
			}

			textData[cnt2 + 1].y -= step;
			cnt2++;
		}

		_screen->copyRegion(d->sx << 3, d->sy, d->sx << 3, d->sy, d->w << 3, d->h, tempPage2, 0);
		_screen->updateScreen();

		if (textData[1].y < -10) {
			textData[1].text += strlen((char *)textData[1].text);
			textData[1].text[0] = textData[1].unk1;
			cnt--;
			memcpy(&textData[1], &textData[2], cnt * sizeof(ScrollTextData));
		}

		if (palCycle) {
			for (int col = 133; col > 112; col--)
				_screen->getPalette(0).copy(_screen->getPalette(0), col - 1, 1, col);
			_screen->getPalette(0).copy(_screen->getPalette(0), 133, 1, 112);
			_screen->setScreenPalette(_screen->getPalette(0));
		}

		delayUntil(loopEnd);

		if ((cnt < 36) && ((d->sy + d->h) > (textData[cnt].y + textData[cnt].height)) && !_abortPlayback) {
			delayTicks(500);
			cnt = 0;
		}

		if (checkAbortPlayback())
			if (checkPlaybackStatus())
				loop = false;

		if (!cnt || _abortPlayback)
			loop = false;
	}

	_vm->sound()->beginFadeOut();
	_screen->fadeToBlack();

	_abortPlayback = _abortRequested = false;

	delete[] textData;
}

void SeqPlayer_HOF::updateDemoAdText(int bottom, int top) {
	int dstY, dstH, srcH;

	static const ScreenDim d = { 0x00, 0x00, 0x28, 0x320, 0xFF, 0xFE, 0x00, 0x00 };

	if (_scrollProgressCounter - (top - 1) < 0) {
		dstY = top - _scrollProgressCounter;
		dstH = _scrollProgressCounter;
		srcH = 0;
	} else {
		dstY = 0;
		srcH = _scrollProgressCounter - top;
		dstH = (400 - srcH <= top) ? 400 - srcH : top;
	}

	if (dstH > 0) {
		if (_hofDemoAnimData) {
			for (int i = 0; i < 4; i++) {
				const HoFSeqItemAnimData *def = &_hofDemoAnimData[i];
				ActiveItemAnim *a = &_hofDemoActiveItemAnim[i];

				_screen->fillRect(12, def->y - 8, 28, def->y + 8, 0, 4);
				_screen->drawShape(4, _hofDemoItemShapes[def->itemIndex + def->frames[a->currentFrame]], 12, def->y - 8, 0, 0);
				if (_callbackCurrentFrame % 2 == 0)
					a->currentFrame = (a->currentFrame + 1) % 20;
			}
		}
		_screen->copyRegionEx(4, 0, srcH, 2, 2, dstY + bottom, 320, dstH, &d);
	}
}

void SeqPlayer_HOF::delayTicks(uint32 ticks) {
	uint32 len = ticks * _vm->tickLength();
	while (len && !_vm->shouldQuit() && !checkAbortPlayback()) {
		uint32 step = (len >= 10) ? 10 : len;
		_system->delayMillis(step);
		len -= step;
	}
}

void SeqPlayer_HOF::delayUntil(uint32 dest) {
	for (uint32 ct = _system->getMillis(); ct < dest && !_vm->shouldQuit() && !checkAbortPlayback(); ) {
		uint32 step = (dest - ct >= 10) ? 10 : (dest - ct);
		_system->delayMillis(step);
		ct = _system->getMillis();
	}
}

void SeqPlayer_HOF::setCountDown(uint32 ticks) {
	_countDownRemainder = ticks * _vm->tickLength();
	if (_vm->gameFlags().platform == Common::kPlatformFMTowns || _vm->gameFlags().platform == Common::kPlatformPC98)
		_countDownRemainder = _countDownRemainder * 2 / 3;
	_countDownLastUpdate = _system->getMillis() & ~(_vm->tickLength() - 1);
}

bool SeqPlayer_HOF::countDownRunning() {
	uint32 cur = _system->getMillis();
	uint32 step = cur - _countDownLastUpdate;
	_countDownLastUpdate = cur;
	_countDownRemainder = (step <= _countDownRemainder) ? _countDownRemainder - step : 0;
	return _countDownRemainder;
}

#define CASE_ALT(dosCase, towns98Case)\
	case dosCase:\
	case towns98Case:\
		if (!((_callbackCurrentFrame == towns98Case && (_vm->gameFlags().platform == Common::kPlatformFMTowns || _vm->gameFlags().platform == Common::kPlatformPC98)) || (_callbackCurrentFrame == dosCase && _vm->gameFlags().platform == Common::kPlatformPC)))\
			break;

int SeqPlayer_HOF::cbHOF_westwood(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (frm == -2) {
		if (_vm->gameFlags().platform == Common::kPlatformFMTowns || _vm->gameFlags().platform == Common::kPlatformPC98)
			delayTicks(300);
	} else if (!frm) {
		_vm->sound()->playTrack(2);
	}

	return 0;
}

int SeqPlayer_HOF::cbHOF_title(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (frm == 1) {
		_vm->sound()->playTrack(3);
	} else if (frm == 25 && _startupSaveLoadable) {
		int cp = _screen->setCurPage(0);
		_screen->showMouse();
		_system->updateScreen();
		_result = _menu->handle(11) + 1;
		_updateAnimations = false;

		if (_result == 1) {
			_curScene = _lastScene;
			_preventLooping = true;
		} else {
			setCountDown(200);
		}

		if (_result == 4)
			_vm->quitGame();

		_screen->hideMouse();
		_screen->setCurPage(cp);
	}

	return 0;
}

int SeqPlayer_HOF::cbHOF_overview(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	uint8 *tmpPal = _screen->getPalette(3).getData() + 0x101;
	memset(tmpPal, 0, 256);
	uint32 frameEnd = 0;

	switch (_callbackCurrentFrame) {
	case 0:
		_updateAnimations = true;
		fadeOutMusic();
		_vm->sound()->playTrack(4);
		frameEnd = _system->getMillis() + 60 * _vm->tickLength();

		_textColor[1] = _screen->findLeastDifferentColor(_textColorPresets, _screen->getPalette(0), 1, 255) & 0xff;
		memset(_textColorMap, _textColor[1], 16);
		_textColorMap[1] = _textColor[0] = _screen->findLeastDifferentColor(_textColorPresets + 3, _screen->getPalette(0), 1, 255) & 0xff;
		_screen->setTextColorMap(_textColorMap);

		delayUntil(frameEnd);
		break;

	case 1:
		assert(_screenHoF);
		_screenHoF->generateGrayOverlay(_screen->getPalette(0), _screen->getPalette(3).getData(), 0x40, 0, 0, 0, 0x100, true);
		for (int i = 0; i < 256; i++)
			tmpPal[_screen->getPalette(3)[i]] = 1;

		for (int i = 0; i < 256; i++) {
			int v = (tmpPal[i] == 1) ? i : _screen->getPalette(3)[i];
			v *= 3;
			_screen->getPalette(2)[3 * i] = _screen->getPalette(0)[v];
			_screen->getPalette(2)[3 * i + 1] = _screen->getPalette(0)[v + 1];
			_screen->getPalette(2)[3 * i + 2] = _screen->getPalette(0)[v + 2];
		}
		break;

	case 40:
		startNestedAnimation(0, kNestedSequenceOver1);
		break;

	case 60:
		startNestedAnimation(1, kNestedSequenceOver2);
		break;

	case 120:
		playSoundAndDisplaySubTitle(0);
		break;

	case 200:
		waitForSubTitlesTimeout();
		_screen->fadePalette(_screen->getPalette(2), 64);
		break;

	case 201:
		_screen->setScreenPalette(_screen->getPalette(2));
		_screen->updateScreen();
		_screen->applyOverlay(0, 0, 320, 200, 2, _screen->getPalette(3).getData());
		_screen->copyPage(2, 12);
		_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0);
		_screen->setScreenPalette(_screen->getPalette(0));
		_screen->updateScreen();
		closeNestedAnimation(0);
		closeNestedAnimation(1);
		break;

	case 282:
		startNestedAnimation(0, kNestedSequenceForest);
		playSoundAndDisplaySubTitle(1);
		break;

	CASE_ALT(434, 354)
		closeNestedAnimation(0);
		startNestedAnimation(0, kNestedSequenceDragon);
		break;

	CASE_ALT(540, 400)
		waitForSubTitlesTimeout();
		closeNestedAnimation(0);
		setCountDown(0);
		_updateAnimations = false;
		break;

	default:
		break;
	}

	_callbackCurrentFrame++;
	return 0;
}

int SeqPlayer_HOF::cbHOF_library(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	switch (_callbackCurrentFrame) {
	case 0:
		_updateAnimations = true;
		_vm->sound()->playTrack(5);

		assert(_screenHoF);
		_screenHoF->generateGrayOverlay(_screen->getPalette(0), _screen->getPalette(3).getData(), 0x24, 0, 0, 0, 0x100, false);
		_textColor[1] = _screen->findLeastDifferentColor(_textColorPresets, _screen->getPalette(0), 1, 255) & 0xff;
		memset(_textColorMap, _textColor[1], 16);
		_textColorMap[1] = _textColor[0] = _screen->findLeastDifferentColor(_textColorPresets + 3, _screen->getPalette(0), 1, 255) & 0xff;

		_screen->setTextColorMap(_textColorMap);
		break;

	case 1:
		startNestedAnimation(0, kNestedSequenceLibrary3);
		playSoundAndDisplaySubTitle(4);
		break;

	case 100:
		waitForSubTitlesTimeout();

		_screen->copyPage(12, 2);
		_screen->applyOverlay(0, 0, 320, 200, 2, _screen->getPalette(3).getData());
		_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0);
		_screen->updateScreen();
		_screen->copyPage(2, 12);

		closeNestedAnimation(0);
		startNestedAnimation(0, kNestedSequenceDarm);
		break;

	case 104:
		playSoundAndDisplaySubTitle(5);
		break;

	case 240:
		waitForSubTitlesTimeout();
		closeNestedAnimation(0);
		startNestedAnimation(0, kNestedSequenceLibrary2);
		break;

	case 340:
		closeNestedAnimation(0);
		_screen->applyOverlay(0, 0, 320, 200, 2, _screen->getPalette(3).getData());
		_screen->copyPage(2, 12);
		_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0);
		_screen->updateScreen();

		startNestedAnimation(0, kNestedSequenceMarco);
		playSoundAndDisplaySubTitle(6);
		break;

	CASE_ALT(660, 480)
		_screen->copyPage(2, 12);
		waitForSubTitlesTimeout();
		closeNestedAnimation(0);
		setCountDown(0);
		_updateAnimations = false;
		break;

	default:
		break;
	}

	_callbackCurrentFrame++;
	return 0;
}

int SeqPlayer_HOF::cbHOF_hand(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	switch (_callbackCurrentFrame) {
	case 0:
		_updateAnimations = true;
		_vm->sound()->playTrack(6);

		assert(_screenHoF);
		_screenHoF->generateGrayOverlay(_screen->getPalette(0), _screen->getPalette(3).getData(), 0x24, 0, 0, 0, 0x100, false);
		_textColor[1] = _screen->findLeastDifferentColor(_textColorPresets, _screen->getPalette(0), 1, 255) & 0xff;
		memset(_textColorMap, _textColor[1], 16);
		_textColorMap[1] = _textColor[0] = _screen->findLeastDifferentColor(_textColorPresets + 3, _screen->getPalette(0), 1, 255) & 0xff;

		_screen->setTextColorMap(_textColorMap);
		break;

	case 1:
		startNestedAnimation(0, kNestedSequenceHand1a);
		startNestedAnimation(1, kNestedSequenceHand1b);
		startNestedAnimation(2, kNestedSequenceHand1c);
		playSoundAndDisplaySubTitle(7);
		break;

	case 201:
		waitForSubTitlesTimeout();
		_screen->applyOverlay(0, 0, 320, 200, 2, _screen->getPalette(3).getData());
		_screen->copyPage(2, 12);
		_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0);
		_screen->updateScreen();
		closeNestedAnimation(0);
		closeNestedAnimation(1);
		closeNestedAnimation(2);
		startNestedAnimation(0, kNestedSequenceHand2);
		playSoundAndDisplaySubTitle(8);
		break;

	CASE_ALT(395, 260)
		waitForSubTitlesTimeout();
		closeNestedAnimation(0);
		startNestedAnimation(1, kNestedSequenceHand3);
		playSoundAndDisplaySubTitle(9);
		break;

	CASE_ALT(500, 365)
		waitForSubTitlesTimeout();
		closeNestedAnimation(1);
		startNestedAnimation(0, kNestedSequenceHand4);
		break;

	CASE_ALT(540, 405)
		playSoundAndDisplaySubTitle(10);
		break;

	CASE_ALT(630, 484)
		waitForSubTitlesTimeout();
		closeNestedAnimation(0);
		setCountDown(0);
		_updateAnimations = false;
		break;

	default:
		break;
	}

	_callbackCurrentFrame++;
	return 0;
}

int SeqPlayer_HOF::cbHOF_point(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (frm == -2) {
		waitForSubTitlesTimeout();
		setCountDown(0);
	}

	switch (_callbackCurrentFrame) {
	case -2:
		waitForSubTitlesTimeout();
		break;

	case 0:
		_vm->sound()->playTrack(7);

		_textColor[1] = 0xf7;
		memset(_textColorMap, _textColor[1], 16);
		_textColorMap[1] = _textColor[0] = _screen->findLeastDifferentColor(_textColorPresets + 3, _screen->getPalette(0), 1, 255) & 0xff;
		_screen->setTextColorMap(_textColorMap);
		assert(_screenHoF);
		_screenHoF->generateGrayOverlay(_screen->getPalette(0), _screen->getPalette(3).getData(), 0x24, 0, 0, 0, 0x100, false);
		break;

	case 1:
		playSoundAndDisplaySubTitle(11);
		break;

	default:
		break;
	}

	_callbackCurrentFrame++;
	return 0;
}

int SeqPlayer_HOF::cbHOF_zanfaun(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (frm == -2) {
		waitForSubTitlesTimeout();
		setCountDown(0);
		return 0;
	}

	switch (_callbackCurrentFrame) {
	case 0:
		_vm->sound()->playTrack(8);

		_textColor[1] = 0xfd;
		memset(_textColorMap, _textColor[1], 16);
		_textColorMap[1] = _textColor[0] = _screen->findLeastDifferentColor(_textColorPresets + 3, _screen->getPalette(0), 1, 255) & 0xff;
		_screen->setTextColorMap(_textColorMap);
		break;

	case 1:
		if (_vm->gameFlags().isTalkie) {
			playDialogueAnimation(21, 13, -1, 140, 70, 160, wsaObj, 0, 8, x, y);
		} else {
			displaySubTitle(21, 140, 70, 200, 160);
			_animDuration = 200;
		}
		break;

	case 2:
	case 11:
	case 21:
		if (!_vm->gameFlags().isTalkie)
			_animDuration = 12;
		break;

	case 9:
		if (_vm->gameFlags().isTalkie)
			playDialogueAnimation(13, 14, -1, 140, (_vm->gameFlags().lang == Common::FR_FRA
				|| _vm->gameFlags().lang == Common::DE_DEU) ? 50 : 70, 160, wsaObj, 9, 15, x, y);
		break;

	case 10:
		if (!_vm->gameFlags().isTalkie) {
			waitForSubTitlesTimeout();
			displaySubTitle(13, 140, 50, _textDuration[13], 160);
			_animDuration = 300;
		}
		break;

	case 16:
		if (_vm->gameFlags().isTalkie)
			playDialogueAnimation(18, 15, -1, 140, (_vm->gameFlags().lang == Common::FR_FRA) ? 50 :
				(_vm->gameFlags().lang == Common::DE_DEU ? 40 : 70), 160, wsaObj, 10, 16, x, y);
		break;

	case 17:
		if (_vm->gameFlags().isTalkie)
			_animDuration = 12;
		break;

	case 20:
		if (!_vm->gameFlags().isTalkie) {
			waitForSubTitlesTimeout();
			displaySubTitle(18, 160, 50, _textDuration[18], 160);
			_animDuration = 200;
		}
		break;

	case 26:
		waitForSubTitlesTimeout();
		break;

	case 46:
		if (_vm->gameFlags().isTalkie) {
			playDialogueAnimation(16, 16, -1, 200, 50, 120, wsaObj, 46, 46, x, y);
		} else {
			waitForSubTitlesTimeout();
			displaySubTitle(16, 200, 50, _textDuration[16], 120);
		}

		setCountDown(120);
		break;

	default:
		break;
	}

	_callbackCurrentFrame++;
	return 0;
}

int SeqPlayer_HOF::cbHOF_over1(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (frm == 2)
		waitForSubTitlesTimeout();
	else if (frm == 3)
		playSoundAndDisplaySubTitle(12);
	return frm;
}

int SeqPlayer_HOF::cbHOF_over2(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (frm == 1)
		playSoundAndDisplaySubTitle(12);
	return frm;
}

int SeqPlayer_HOF::cbHOF_forest(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (frm == 11)
		waitForSubTitlesTimeout();
	else if (frm == 12)
		playSoundAndDisplaySubTitle(2);

	return frm;
}

int SeqPlayer_HOF::cbHOF_dragon(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (frm == 11)
		waitForSubTitlesTimeout();
	else if (frm == 3)
		playSoundAndDisplaySubTitle(3);
	return frm;
}

int SeqPlayer_HOF::cbHOF_darm(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	return frm;
}

int SeqPlayer_HOF::cbHOF_library2(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	return frm;
}

int SeqPlayer_HOF::cbHOF_marco(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (frm == 36) {
		waitForSubTitlesTimeout();
		setCountDown(0);
	}
	return frm;
}

int SeqPlayer_HOF::cbHOF_hand1a(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	return frm;
}

int SeqPlayer_HOF::cbHOF_hand1b(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (frm == 15)
		frm = 12;
	return frm;
}

int SeqPlayer_HOF::cbHOF_hand1c(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (frm == 8)
		frm = 4;
	return frm;
}

int SeqPlayer_HOF::cbHOF_hand2(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	return frm;
}

int SeqPlayer_HOF::cbHOF_hand3(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	return frm;
}

int SeqPlayer_HOF::cbHOF_funters(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	uint32 frameEnd = 0;
	int subTitleX = 0;
	int subTitleY = 0;
	int subTitleW = 0;
	int subTitleFirstFrame = 0;
	int subTitleLastFrame = 0;
	uint16 voiceIndex = 0;

	switch (frm) {
	case -2:
		doTransition(9);
		break;

	case 0:
		_vm->sound()->playTrack(3);

		_textColor[1] = _screen->findLeastDifferentColor(_textColorPresets, _screen->getPalette(0), 1, 255) & 0xff;
		memset(_textColorMap, _textColor[1], 16);
		_textColor[0] = _textColorMap[1] = 0xff;
		_screen->setTextColorMap(_textColorMap);

		frameEnd = _system->getMillis() + 480 * _vm->tickLength();
		printFadingText(81, 240, 70, _textColorMap, 252);
		printFadingText(82, 240, 90, _textColorMap, _textColor[0]);
		_screen->copyPage(2, 12);
		playSoundAndDisplaySubTitle(_vm->gameFlags().isTalkie ? 28 : 24);
		delayUntil(frameEnd);
		_textColor[0] = 1;

		if (_vm->gameFlags().isTalkie) {
			subTitleY = (_vm->gameFlags().lang == Common::FR_FRA) ? 70 : 78;
			subTitleFirstFrame = 9;
			subTitleLastFrame = 15;
			voiceIndex = 34;
		} else {
			subTitleY = (_vm->gameFlags().lang == Common::FR_FRA) ? 78 : 70;
			subTitleFirstFrame = 0;
			subTitleLastFrame = 8;
		}
		subTitleX = (_vm->gameFlags().lang == Common::FR_FRA) ? 84 : 88;
		subTitleW = 100;

		playDialogueAnimation(22, voiceIndex, 187, subTitleX, subTitleY, subTitleW, wsaObj, subTitleFirstFrame, subTitleLastFrame, x, y);
		break;

	case 9:
	case 16:
		if (!((frm == 9 && !_vm->gameFlags().isTalkie) || (frm == 16 && _vm->gameFlags().isTalkie)))
			break;

		_animDuration = 12;

		if (_vm->gameFlags().lang == Common::FR_FRA) {
			subTitleX = 80;
			subTitleW = 112;
		} else {
			subTitleX = (_vm->gameFlags().lang == Common::DE_DEU) ? 84 : 96;
			subTitleW = 100;
		}

		if (_vm->gameFlags().isTalkie) {
			subTitleFirstFrame = 0;
			subTitleLastFrame = 8;
			voiceIndex = 35;
		} else {
			subTitleFirstFrame = 9;
			subTitleLastFrame = 15;
		}
		subTitleY = 70;

		playDialogueAnimation(23, voiceIndex, 137, subTitleX, subTitleY, subTitleW, wsaObj, subTitleFirstFrame, subTitleLastFrame, x, y);
		if (_vm->gameFlags().isTalkie)
			_animCurrentFrame = 17;
		break;

	default:
		break;
	}

	_callbackCurrentFrame++;
	return 0;
}

int SeqPlayer_HOF::cbHOF_ferb(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	uint32 frameEnd = 0;
	int subTitleX = 0;
	int subTitleY = 0;
	int subTitleW = 0;
	int subTitleFirstFrame = 0;
	int subTitleLastFrame = 0;
	uint16 voiceIndex = 0;

	switch (frm) {
	case -2:
		doTransition(9);
		frameEnd = _system->getMillis() + 480 * _vm->tickLength();
		printFadingText(34, 240, _vm->gameFlags().isTalkie ? 60 : 40, _textColorMap, 252);
		printFadingText(35, 240, _vm->gameFlags().isTalkie ? 70 : 50, _textColorMap, _textColor[0]);
		printFadingText(36, 240, _vm->gameFlags().isTalkie ? 90 : 70, _textColorMap, 252);
		printFadingText(37, 240, _vm->gameFlags().isTalkie ? 100 : 90, _textColorMap, _textColor[0]);
		printFadingText(38, 240, _vm->gameFlags().isTalkie ? 120 : 110, _textColorMap, 252);
		printFadingText(39, 240, _vm->gameFlags().isTalkie ? 130 : 120, _textColorMap, _textColor[0]);
		if (_vm->gameFlags().platform == Common::kPlatformFMTowns || _vm->gameFlags().platform == Common::kPlatformPC98)
			printFadingText(103, 240, 130, _textColorMap, _textColor[0]);
		delayUntil(frameEnd);
		setCountDown(0);
		break;

	case 0:
		_textColor[1] = _screen->findLeastDifferentColor(_textColorPresets, _screen->getPalette(0), 1, 255) & 0xff;
		memset(_textColorMap, _textColor[1], 16);
		_textColor[0] = _textColorMap[1] = 255;
		_screen->setTextColorMap(_textColorMap);
		break;

	case 5:
		if (!_vm->gameFlags().isTalkie)
			playSoundAndDisplaySubTitle(18);
		_animDuration = 16;

		if (_vm->gameFlags().isTalkie) {
			subTitleFirstFrame = 5;
			subTitleLastFrame = 8;
			voiceIndex = 22;
		} else {
			subTitleLastFrame = 14;
		}
		subTitleX = 116;
		subTitleY = 90;
		subTitleW = 60;

		playDialogueAnimation(24, voiceIndex, 149, subTitleX, subTitleY, subTitleW, wsaObj, subTitleFirstFrame, subTitleLastFrame, x, y);
		break;

	case 11:
		if (_vm->gameFlags().isTalkie)
			playDialogueAnimation(24, 22, 149, 116, 90, 60, wsaObj, 11, 14, x, y);
		break;

	case 16:
		playSoundAndDisplaySubTitle(_vm->gameFlags().isTalkie ? 23 : 19);
		_animDuration = _vm->gameFlags().isTalkie ? 20 : 16;

		if (_vm->gameFlags().lang == Common::FR_FRA) {
			subTitleY = 48;
			subTitleW = 88;
		} else {
			subTitleY = 60;
			subTitleW = 100;
		}
		subTitleX = 60;

		if (_vm->gameFlags().isTalkie)
			voiceIndex = 36;

		playDialogueAnimation(25, voiceIndex, 143, subTitleX, subTitleY, subTitleW, wsaObj, 16, 25, x, y);
		_animDuration = 16;
		break;

	default:
		break;
	}

	_callbackCurrentFrame++;
	return 0;
}

int SeqPlayer_HOF::cbHOF_fish(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	uint32 frameEnd = 0;
	int subTitleX = 0;
	int subTitleY = 0;
	int subTitleW = 0;
	uint16 voiceIndex = 0;

	switch (frm) {
	case -2:
		doTransition(9);
		frameEnd = _system->getMillis() + 480 * _vm->tickLength();

		printFadingText(40, 240, _vm->gameFlags().isTalkie ? 55 : 40, _textColorMap, 252);
		printFadingText(41, 240, _vm->gameFlags().isTalkie ? 65 : 50, _textColorMap, _textColor[0]);
		printFadingText(42, 240, _vm->gameFlags().isTalkie ? 75 : 60, _textColorMap, _textColor[0]);
		printFadingText(43, 240, _vm->gameFlags().isTalkie ? 95 : 80, _textColorMap, 252);
		printFadingText(44, 240, _vm->gameFlags().isTalkie ? 105 : 90, _textColorMap, _textColor[0]);
		printFadingText(93, 240, _vm->gameFlags().isTalkie ? 125 : 110, _textColorMap, 252);
		printFadingText(94, 240, _vm->gameFlags().isTalkie ? 135 : 120, _textColorMap, _textColor[0]);
		delayUntil(frameEnd);
		setCountDown(0);
		break;

	case 0:
		_textColor[1] = _screen->findLeastDifferentColor(_textColorPresets, _screen->getPalette(0), 1, 255) & 0xff;
		memset(_textColorMap, _textColor[1], 16);
		_textColor[0] = _textColorMap[1] = 0xff;
		_screen->setTextColorMap(_textColorMap);
		break;

	case 4:
		subTitleX = 94;
		subTitleY = 42;
		subTitleW = 100;
		if (_vm->gameFlags().isTalkie)
			voiceIndex = 37;
		playDialogueAnimation(26, voiceIndex, 149, subTitleX, subTitleY, subTitleW, wsaObj, 3, 12, x, y);
		break;

	case 14:
		playSoundAndDisplaySubTitle(_vm->gameFlags().isTalkie ? 19 : 15);
		break;

	case 23:
		playSoundAndDisplaySubTitle(_vm->gameFlags().isTalkie ? 20 : 16);
		break;

	case 29:
		subTitleX = (_vm->gameFlags().lang == Common::DE_DEU) ? 82 : ((_vm->gameFlags().lang == Common::FR_FRA) ? 92 : 88);
		subTitleY = 40;
		subTitleW = 100;

		if (_vm->gameFlags().isTalkie) {
			if (_vm->gameFlags().lang == Common::DE_DEU)
				subTitleY = 35;
			voiceIndex = 38;
		}

		playDialogueAnimation(27, voiceIndex, 187, subTitleX, subTitleY, subTitleW, wsaObj, 28, 34, x, y);
		break;

	case 45:
		playSoundAndDisplaySubTitle(_vm->gameFlags().isTalkie ? 21 : 17);
		break;

	case 50:
		playSoundAndDisplaySubTitle(_vm->gameFlags().isTalkie ? 29 : 25);
		break;

	default:
		break;
	}

	_callbackCurrentFrame++;
	return 0;
}

int SeqPlayer_HOF::cbHOF_fheep(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	uint32 frameEnd = 0;
	int subTitleX = 0;
	int subTitleY = 0;
	int subTitleW = 0;
	int subTitleFirstFrame = 0;
	int subTitleLastFrame = 0;
	uint16 voiceIndex = 0;

	switch (frm) {
	case -2:
		_screen->copyPage(12, 2);
		_screen->copyPage(2, 0);
		_screen->updateScreen();
		doTransition(9);
		frameEnd = _system->getMillis() + 480 * _vm->tickLength();
		printFadingText(49, 240, 20, _textColorMap, 252);
		printFadingText(50, 240, 30, _textColorMap, _textColor[0]);
		printFadingText(51, 240, 40, _textColorMap, _textColor[0]);
		printFadingText(52, 240, 50, _textColorMap, _textColor[0]);
		printFadingText(53, 240, 60, _textColorMap, _textColor[0]);
		printFadingText(54, 240, 70, _textColorMap, _textColor[0]);
		printFadingText(55, 240, 80, _textColorMap, _textColor[0]);
		printFadingText(56, 240, 90, _textColorMap, _textColor[0]);
		printFadingText(57, 240, 100, _textColorMap, _textColor[0]);
		printFadingText(58, 240, 110, _textColorMap, _textColor[0]);
		printFadingText(60, 240, 120, _textColorMap, _textColor[0]);
		printFadingText(61, 240, 130, _textColorMap, _textColor[0]);
		printFadingText(62, 240, 140, _textColorMap, _textColor[0]);
		printFadingText(63, 240, 150, _textColorMap, _textColor[0]);
		printFadingText(64, 240, 160, _textColorMap, _textColor[0]);

		delayUntil(frameEnd);
		setCountDown(0);
		break;

	case 0:
		_textColor[1] = _screen->findLeastDifferentColor(_textColorPresets, _screen->getPalette(0), 1, 255) & 0xff;
		memset(_textColorMap, _textColor[1], 16);
		_textColor[0] = _textColorMap[1] = 0xff;
		_screen->setTextColorMap(_textColorMap);
		break;

	case 2:
		playSoundAndDisplaySubTitle(_vm->gameFlags().isTalkie ? 25 : 21);

		if (_vm->gameFlags().lang == Common::FR_FRA) {
			subTitleX = 92;
			subTitleY = 72;
		} else {
			subTitleX = (_vm->gameFlags().lang == Common::DE_DEU) ? 90 : 98;
			subTitleY = 84;
		}

		if (_vm->gameFlags().isTalkie) {
			subTitleFirstFrame = 8;
			subTitleLastFrame = 9;
			voiceIndex = 39;
		} else {
			subTitleFirstFrame = 2;
			subTitleLastFrame = -8;
		}
		subTitleW = 100;

		playDialogueAnimation(28, voiceIndex, -1, subTitleX, subTitleY, subTitleW, wsaObj, subTitleFirstFrame, subTitleLastFrame, x, y);
		if (_vm->gameFlags().isTalkie)
			_animCurrentFrame = 4;
		break;

	case 9:
		playSoundAndDisplaySubTitle(_vm->gameFlags().isTalkie ? 24 : 20);
		_animDuration = 100;
		break;

	default:
		break;
	}

	_callbackCurrentFrame++;
	return 0;
}

int SeqPlayer_HOF::cbHOF_farmer(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	uint32 frameEnd = 0;
	int subTitleX = 0;
	int subTitleY = 0;
	int subTitleW = 0;
	uint16 voiceIndex = 0;

	switch (frm) {
	case -2:
		_screen->copyPage(12, 2);
		_screen->copyPage(2, 0);
		_screen->updateScreen();
		doTransition(9);
		frameEnd = _system->getMillis() + 480 * _vm->tickLength();
		printFadingText(45, 240, 40, _textColorMap, 252);
		printFadingText(46, 240, 50, _textColorMap, _textColor[0]);
		printFadingText(47, 240, 60, _textColorMap, _textColor[0]);
		printFadingText(83, 240, 80, _textColorMap, 252);
		printFadingText(48, 240, 90, _textColorMap, _textColor[0]);
		printFadingText(65, 240, 110, _textColorMap, 252);
		printFadingText(66, 240, 120, _textColorMap, _textColor[0]);
		printFadingText(67, 240, 130, _textColorMap, _textColor[0]);
		printFadingText(68, 240, 140, _textColorMap, _textColor[0]);
		printFadingText(69, 240, 150, _textColorMap, _textColor[0]);
		if (_vm->gameFlags().platform == Common::kPlatformFMTowns || _vm->gameFlags().platform == Common::kPlatformPC98)
			printFadingText(104, 240, 160, _textColorMap, _textColor[0]);
		delayUntil(frameEnd);
		setCountDown(0);
		break;

	case 0:
		_textColor[1] = 1 + (_screen->findLeastDifferentColor(_textColorPresets, _screen->getPalette(0), 1, 254) & 0xff);
		memset(_textColorMap, _textColor[1], 16);
		_textColorMap[1] = _textColor[0] = 1 + (_screen->findLeastDifferentColor(_textColorPresets + 3, _screen->getPalette(0), 1, 254) & 0xff);
		_screen->setTextColorMap(_textColorMap);
		playSoundAndDisplaySubTitle(_vm->gameFlags().isTalkie ? 30 : 26);
		break;

	case 6:
		if (_vm->gameFlags().isTalkie)
			playSoundAndDisplaySubTitle(18);
		break;

	case 12:
		if (!_vm->gameFlags().isTalkie)
			playSoundAndDisplaySubTitle(14);

		subTitleX = 90;
		subTitleY = 30;
		subTitleW = 100;

		if (_vm->gameFlags().isTalkie) {
			if (_vm->gameFlags().lang == Common::FR_FRA || _vm->gameFlags().lang == Common::DE_DEU) {
				subTitleX = 75;
				subTitleY = 25;
			}
			voiceIndex = 40;
		}

		playDialogueAnimation(29, voiceIndex, 150, subTitleX, subTitleY, subTitleW, wsaObj, 12, -21, x, y);
		break;

	default:
		break;
	}

	_callbackCurrentFrame++;
	return 0;
}

int SeqPlayer_HOF::cbHOF_fuards(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	uint32 frameEnd = 0;
	int subTitleX = 0;
	int subTitleY = 0;
	int subTitleW = 0;
	int subTitleFirstFrame = 0;
	int subTitleLastFrame = 0;

	uint16 voiceIndex = 0;

	switch (frm) {
	case -2:
		doTransition(9);
		frameEnd = _system->getMillis() + 480 * _vm->tickLength();
		printFadingText(70, 240, 20, _textColorMap, 252);
		printFadingText(71, 240, 30, _textColorMap, _textColor[0]);
		printFadingText(72, 240, 40, _textColorMap, _textColor[0]);
		printFadingText(73, 240, 50, _textColorMap, _textColor[0]);
		printFadingText(74, 240, 60, _textColorMap, _textColor[0]);
		printFadingText(75, 240, 70, _textColorMap, _textColor[0]);
		printFadingText(101, 240, 80, _textColorMap, _textColor[0]);
		printFadingText(102, 240, 90, _textColorMap, _textColor[0]);
		printFadingText(87, 240, 100, _textColorMap, _textColor[0]);
		printFadingText(88, 240, 110, _textColorMap, _textColor[0]);
		printFadingText(89, 240, 120, _textColorMap, _textColor[0]);
		printFadingText(90, 240, 130, _textColorMap, _textColor[0]);
		printFadingText(91, 240, 140, _textColorMap, _textColor[0]);
		printFadingText(92, 240, 150, _textColorMap, _textColor[0]);
		delayUntil(frameEnd);
		setCountDown(0);
		break;

	case 0:
		for (int i = 0; i < 0x300; i++)
			_screen->getPalette(0)[i] &= 0x3f;
		_textColor[1] = 0xCf;
		memset(_textColorMap, _textColor[1], 16);
		_textColor[0] = _textColorMap[1] = 0xfe;

		_screen->setTextColorMap(_textColorMap);
		break;

	case 6:
		_animDuration = 20;

		if (_vm->gameFlags().isTalkie) {
			subTitleX = 82;
			subTitleFirstFrame = 16;
			subTitleLastFrame = 21;
			voiceIndex = 41;
		} else {
			subTitleX = 62;
			subTitleFirstFrame = 9;
			subTitleLastFrame = 13;
		}
		subTitleY = (_vm->gameFlags().lang == Common::FR_FRA || _vm->gameFlags().lang == Common::DE_DEU) ? 88 :100;
		subTitleW = 80;

		playDialogueAnimation(30, voiceIndex, 137, subTitleX, subTitleY, subTitleW, wsaObj, subTitleFirstFrame, subTitleLastFrame, x, y);
		if (_vm->gameFlags().isTalkie)
			_animCurrentFrame = 8;
		break;

	case 9:
	case 16:
		if (_vm->gameFlags().isTalkie) {
			if (frm == 16)
				break;
			subTitleX = 64;
			subTitleFirstFrame = 9;
			subTitleLastFrame = 13;
			voiceIndex = 42;
		} else {
			if (frm == 9)
				break;
			subTitleX = 80;
			subTitleFirstFrame = 16;
			subTitleLastFrame = 21;
		}
		subTitleY = 100;
		subTitleW = 100;

		playDialogueAnimation(31, voiceIndex, 143, subTitleX, subTitleY, subTitleW, wsaObj, subTitleFirstFrame, subTitleLastFrame, x, y);
		if (_vm->gameFlags().isTalkie)
			_animCurrentFrame = 21;
		break;

	default:
		break;
	}

	_callbackCurrentFrame++;
	return 0;
}

int SeqPlayer_HOF::cbHOF_firates(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	uint32 frameEnd = 0;
	int subTitleX = 0;
	int subTitleY = 0;
	int subTitleW = 0;
	uint16 voiceIndex = 0;

	switch (frm) {
	case -2:
		_screen->copyPage(12, 2);
		_screen->copyPage(2, 0);
		_screen->updateScreen();
		doTransition(9);
		frameEnd = _system->getMillis() + 480 * _vm->tickLength();
		printFadingText(76, 240, 40, _textColorMap, 252);
		printFadingText(77, 240, 50, _textColorMap, 252);
		printFadingText(78, 240, 60, _textColorMap, _textColor[0]);
		printFadingText(79, 240, 70, _textColorMap, _textColor[0]);
		printFadingText(80, 240, 80, _textColorMap, _textColor[0]);
		printFadingText(84, 240, 100, _textColorMap, 252);
		printFadingText(85, 240, 110, _textColorMap, _textColor[0]);
		printFadingText(99, 240, 130, _textColorMap, 252);
		printFadingText(100, 240, 140, _textColorMap, _textColor[0]);
		delayUntil(frameEnd);
		setCountDown(0);
		break;

	case 0:
		_textColor[1] = _screen->findLeastDifferentColor(_textColorPresets, _screen->getPalette(0), 1, 255) & 0xff;
		memset(_textColorMap, _textColor[1], 16);
		_textColor[0] = _textColorMap[1] = 0xff;
		_screen->setTextColorMap(_textColorMap);
		break;

	case 6:
		playSoundAndDisplaySubTitle(_vm->gameFlags().isTalkie ? 31 : 27);
		break;

	case 14:
	case 15:
		if (!((frm == 15 && !_vm->gameFlags().isTalkie) || (frm == 14 && _vm->gameFlags().isTalkie)))
			break;

		playSoundAndDisplaySubTitle(_vm->gameFlags().isTalkie ? 31 : 27);

		if (_vm->gameFlags().lang == Common::DE_DEU) {
			subTitleX = 82;
			subTitleY = 84;
			subTitleW = 140;
		} else {
			subTitleX = 74;
			subTitleY = (_vm->gameFlags().lang == Common::FR_FRA) ? 96: 108;
			subTitleW = 80;
		}

		if (_vm->gameFlags().isTalkie)
			voiceIndex = 43;

		playDialogueAnimation(32, voiceIndex, 137, subTitleX, subTitleY, subTitleW, wsaObj, 14, 16, x, y);
		break;

	case 28:
		playSoundAndDisplaySubTitle(_vm->gameFlags().isTalkie ? 32 : 28);
		break;

	case 29:
		playSoundAndDisplaySubTitle(_vm->gameFlags().isTalkie ? 33 : 29);
		break;

	case 31:
		if (_vm->gameFlags().isTalkie)
			voiceIndex = 44;

		subTitleX = 90;
		subTitleY = (_vm->gameFlags().lang == Common::DE_DEU) ? 60 : 76;
		subTitleW = 80;

		playDialogueAnimation(33, voiceIndex, 143, subTitleX, subTitleY, subTitleW, wsaObj, 31, 34, x, y);
		break;

	case 35:
		_animDuration = 300;
		break;

	default:
		break;
	}

	_callbackCurrentFrame++;
	return 0;
}

int SeqPlayer_HOF::cbHOF_frash(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	int tmp = 0;

	switch (frm) {
	case -2:
		_screen->setCurPage(2);
		_screen->clearCurPage();
		_screen->copyPage(2, 12);
		_screen->copyPage(2, 0);
		_screen->updateScreen();
		_callbackCurrentFrame = 0;
		startNestedAnimation(0, kNestedSequenceFiggle);
		break;

	case -1:
		if (_vm->gameFlags().isTalkie)
			 playHoFTalkieCredits();
		_talkieFinaleExtraFlag = _vm->gameFlags().isTalkie;
		break;

	case 0:
		if (_callbackCurrentFrame == 1) {
			_vm->sound()->playTrack(4);
			_textColor[1] = _screen->findLeastDifferentColor(_textColorPresets, _screen->getPalette(0), 1, 255) & 0xff;
			memset(_textColorMap, _textColor[1], 16);
			_textColor[0] = _textColorMap[1] = 0xff;
			_screen->setTextColorMap(_textColorMap);
		}
		_animDuration = 10;
		break;

	case 1:
		if (_callbackCurrentFrame < 20 && _talkieFinaleExtraFlag) {
			_animCurrentFrame = 0;
		} else {
			_animDuration = _vm->gameFlags().isTalkie ? 500 : (300 + _vm->_rnd.getRandomNumberRng(1, 300));
			playSoundAndDisplaySubTitle(_vm->gameFlags().isTalkie ? 26 : 22);
			if (_talkieFinaleExtraFlag) {
				_callbackCurrentFrame = 3;
				_talkieFinaleExtraFlag = false;
			}
		}
		break;

	case 2:
		_animDuration = 20;
		break;

	case 3:
		playSoundAndDisplaySubTitle(_vm->gameFlags().isTalkie ? 27 : 23);
		_animDuration = _vm->gameFlags().isTalkie ? 500 : (300 + _vm->_rnd.getRandomNumberRng(1, 300));
		break;

	case 4:
		_animDuration = 10;
		break;

	case 5:
		playSoundAndDisplaySubTitle(_vm->gameFlags().isTalkie ? 27 : 23);
		tmp = _callbackCurrentFrame / 6;
		if (tmp == 2)
			_animDuration = _vm->gameFlags().isTalkie ? 7 : (1 + _vm->_rnd.getRandomNumberRng(1, 10));
		else if (tmp < 2)
			_animDuration = _vm->gameFlags().isTalkie ? 500 : (300 + _vm->_rnd.getRandomNumberRng(1, 300));
		break;

	case 6:
		_animDuration = 10;
		tmp = _callbackCurrentFrame / 6;
		if (tmp == 2)
			_animCurrentFrame = 4;
		else if (tmp < 2)
			_animCurrentFrame = 0;
		break;

	case 7:
		_callbackCurrentFrame = 0;
		_animDuration = 5;
		playSoundAndDisplaySubTitle(_vm->gameFlags().isTalkie ? 26 : 22);
		break;

	case 11:
		if (_callbackCurrentFrame < 8)
			_animCurrentFrame = 8;
		break;

	default:
		break;
	}

	_callbackCurrentFrame++;
	return 0;
}

int SeqPlayer_HOF::cbHOF_figgle(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (_callbackCurrentFrame == 10)
		setCountDown(0);
	if (_callbackCurrentFrame == 10 || _callbackCurrentFrame == 5 || _callbackCurrentFrame == 7)
		playSoundAndDisplaySubTitle(_vm->gameFlags().isTalkie ? 45 : 30);

	_callbackCurrentFrame++;
	return frm;
}

int SeqPlayer_HOF::cbHOFDEMO_virgin(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (!frm)
		delayTicks(50);
	return 0;
}

int SeqPlayer_HOF::cbHOFDEMO_westwood(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (!frm)
		_vm->sound()->playTrack(2);
	return 0;
}

int SeqPlayer_HOF::cbHOFDEMO_title(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (!frm) {
		_vm->sound()->playTrack(3);
	} else if (frm == 25) {
		delayTicks(60);
		setCountDown(0);
		doTransition(0);
	}
	return 0;
}

int SeqPlayer_HOF::cbHOFDEMO_hill(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (!frm) {
		_vm->sound()->playTrack(4);
	} else if (frm == 25) {
		startNestedAnimation(0, kNestedSequenceHoFDemoWater);
		_animDuration--;
	} else if (frm > 25 && frm < 50) {
		if (_animDuration > 3)
			_animDuration--;
	} else if (frm == 95) {
		_animDuration = 70;
	} else if (frm == 96) {
		_animDuration = 7;
	} else if (frm == 129) {
		closeNestedAnimation(0);
	}

	return 0;
}

int SeqPlayer_HOF::cbHOFDEMO_outhome(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	switch (frm) {
	case 12:
		playSoundAndDisplaySubTitle(4);
		break;

	case 32:
		playSoundAndDisplaySubTitle(7);
		break;

	case 36:
		playSoundAndDisplaySubTitle(10);
		break;

	case 57:
		playSoundAndDisplaySubTitle(9);
		break;

	case 80:
	case 96:
	case 149:
		_animDuration = 70;
		break;

	case 81:
	case 97:
		_animDuration = 5;
		break;

	case 110:
		playSoundAndDisplaySubTitle(5);
		break;

	case 137:
		playSoundAndDisplaySubTitle(6);
		break;
	}

	return 0;
}

int SeqPlayer_HOF::cbHOFDEMO_wharf(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (!_callbackCurrentFrame)
		startNestedAnimation(0, kNestedSequenceHoFDemoWharf2);

	switch (frm) {
	case 0:
		playSoundAndDisplaySubTitle(11);
		break;

	case 5:
		if ((_callbackCurrentFrame / 8) <= 2 || _animSlots[0].flags != -1)
			_animCurrentFrame = 0;
		else
			closeNestedAnimation(0);
		break;

	case 6:
		closeNestedAnimation(0);
		break;

	case 8:
	case 10:
		playSoundAndDisplaySubTitle(2);
		break;

	case 13:
		playSoundAndDisplaySubTitle(7);
		break;

	case 16:
		playSoundAndDisplaySubTitle(12);
		break;

	default:
		break;
	}

	_callbackCurrentFrame++;
	return 0;
}

int SeqPlayer_HOF::cbHOFDEMO_dinob(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (frm == 0) {
		if (!(_callbackCurrentFrame/8)) {
			startNestedAnimation(0, kNestedSequenceHoFDemoDinob2);
			_animCurrentFrame = 0;
		}
	} else if (frm == 3) {
		if (_animSlots[0].flags != -1) {
			_animCurrentFrame = 0;
		} else {
			closeNestedAnimation(0);
			_screen->copyPage(2, 12);
		}
	} else if (frm == 4) {
		closeNestedAnimation(0);
	}

	_callbackCurrentFrame++;
	return 0;
}

int SeqPlayer_HOF::cbHOFDEMO_fisher(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (((_system->getMillis() - _fisherAnimCurTime) / (5 * _vm->tickLength())) > 0) {
		_fisherAnimCurTime = _system->getMillis();
		if (!_callbackCurrentFrame) {
			startNestedAnimation(0, kNestedSequenceHoFDemoBail);
			startNestedAnimation(1, kNestedSequenceHoFDemoDig);
		}

		if (_scrollProgressCounter >= 0x18f && !_callbackCurrentFrame)
			return 0;

		if (!_callbackCurrentFrame) {
			_screen->loadBitmap("adtext.cps", 4, 4, 0);
			_screen->loadBitmap("adtext2.cps", 6, 6, 0);
			_screen->copyPageMemory(6, 0, 4, 64000, 1024);
			_screen->copyPageMemory(6, 1023, 6, 0, 64000);
			_scrollProgressCounter = 0;
		}

		updateDemoAdText(24, 144);
		_callbackCurrentFrame++;
		if (_callbackCurrentFrame < 0x256 || _callbackCurrentFrame > 0x31c) {
			if (_callbackCurrentFrame < 0x174 || _callbackCurrentFrame > 0x1d7) {
				if (_callbackCurrentFrame < 0x84 || _callbackCurrentFrame > 0xe7) {
					_scrollProgressCounter++;
				}
			}
		}

		if (_callbackCurrentFrame > 0x31e) {
			closeNestedAnimation(0);
			closeNestedAnimation(1);
			setCountDown(0);
			_screen->copyPage(2, 12);
		}

	} else {
		updateDemoAdText(24, 144);
	}
	return 0;
}

int SeqPlayer_HOF::cbHOFDEMO_wharf2(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (frm == 69)
		_animCurrentFrame = 8;

	return frm;
}

int SeqPlayer_HOF::cbHOFDEMO_dinob2(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	switch (frm) {
	case 19:
		playSoundAndDisplaySubTitle(13);
		break;

	case 54:
		playSoundAndDisplaySubTitle(15);
		break;

	case 61:
		playSoundAndDisplaySubTitle(16);
		break;

	case 69:
		playSoundAndDisplaySubTitle(14);
		break;

	case 77:
		playSoundAndDisplaySubTitle(13);
		break;

	case 79:
		_animCurrentFrame = 4;
		break;
	}

	return frm;
}

int SeqPlayer_HOF::cbHOFDEMO_water(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (frm == 1)
		playSoundAndDisplaySubTitle(11);
	return frm;
}

int SeqPlayer_HOF::cbHOFDEMO_bail(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	return frm;
}

int SeqPlayer_HOF::cbHOFDEMO_dig(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	return frm;
}

#ifdef ENABLE_LOL
int SeqPlayer_HOF::cbLOLDEMO_scene1(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	Palette &tmpPal = _screen->getPalette(2);

	if (!(_callbackCurrentFrame % 100)) {
		if (_callbackCurrentFrame == 0) {
			_vm->sound()->haltTrack();
			_vm->sound()->playTrack(6);
		}
		tmpPal.copy(_screen->getPalette(0));

		for (int i = 3; i < 768; i++) {
			tmpPal[i] = ((int)tmpPal[i] * 120) / 64;
			if (tmpPal[i] > 0x3f)
				tmpPal[i] = 0x3f;
		}

		playSoundAndDisplaySubTitle(_vm->_rnd.getRandomBit());
		_screen->setScreenPalette(tmpPal);
		_screen->updateScreen();
		_vm->delay(8);
	} else {
		_screen->setScreenPalette(_screen->getPalette(0));
		_screen->updateScreen();
		if (_callbackCurrentFrame == 40)
			playSoundAndDisplaySubTitle(3);
	}

	_callbackCurrentFrame++;
	return frm;
}

int SeqPlayer_HOF::cbLOLDEMO_scene2(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	switch (frm - 17) {
	case 0:
		_animDuration = 8;
		break;
	case 3:
	case 6:
	case 9:
		playSoundEffect(8, 255 - ((26 - frm) << 3));
		break;
	case 15:
		playSoundAndDisplaySubTitle(9);
		break;
	case 18:
		playSoundAndDisplaySubTitle(2);
		break;
	default:
		break;
	}
	_callbackCurrentFrame++;
	return frm;
}

int SeqPlayer_HOF::cbLOLDEMO_scene3(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (frm == 1)
		playSoundAndDisplaySubTitle(6);
	else if (frm == 24)
		playSoundAndDisplaySubTitle(7);

	_callbackCurrentFrame++;
	return frm;
}

int SeqPlayer_HOF::cbLOLDEMO_scene4(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	switch (frm) {
	case 11:
	case 14:
	case 17:
	case 20:
		playSoundEffect(8, 255 - ((22 - frm) << 3));
		break;
	case 22:
		playSoundAndDisplaySubTitle(11);
		break;
	case 24:
		playSoundAndDisplaySubTitle(8);
		break;
	case 30:
		playSoundAndDisplaySubTitle(15);
		break;
	case 34:
		playSoundAndDisplaySubTitle(14);
		break;
	case 38:
		playSoundAndDisplaySubTitle(13);
		break;
	case 42:
		playSoundAndDisplaySubTitle(12);
		break;
	default:
		break;
	}

	_callbackCurrentFrame++;
	return frm;
}

int SeqPlayer_HOF::cbLOLDEMO_scene5(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	switch (_callbackCurrentFrame++) {
	case 0:
	case 4:
	case 6:
	case 8:
	case 10:
	case 14:
	case 16:
	case 18:
	case 20:
	case 22:
	case 24:
	case 26:
	case 28:
	case 30:
		playSoundEffect(15, 255 - ((31 - frm) << 3));
		break;
	case 32:
		playSoundAndDisplaySubTitle(16);
		break;
	case 42:
		playSoundAndDisplaySubTitle(6);
		break;
	default:
		break;
	}
	return frm;
}

int SeqPlayer_HOF::cbLOLDEMO_text5(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (_callbackCurrentFrame++ == 100)
		playSoundAndDisplaySubTitle(5);
	return frm;
}

int SeqPlayer_HOF::cbLOLDEMO_scene6(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	while (_scrollProgressCounter < 290) {
		setCountDown(6);
		if (!_callbackCurrentFrame) {
			_screen->loadBitmap("adtext.cps", 4, 4, 0);
			_screen->loadBitmap("adtext2.cps", 6, 6, 0);
			_screen->copyPageMemory(6, 0, 4, 64000, 1024);
			_screen->copyPageMemory(6, 1023, 6, 0, 64000);
			_scrollProgressCounter = 0;
		}

		if (_callbackCurrentFrame % 175) {
			_screen->setScreenPalette(_screen->getPalette(0));
		} else {
			Palette &tmpPal = _screen->getPalette(2);
			tmpPal.copy(_screen->getPalette(0));

			for (int i = 3; i < 0x300; i++) {
				tmpPal[i] = ((int)tmpPal[i] * 120) / 64;
				if (tmpPal[i] > 0x3f)
					tmpPal[i] = 0x3f;
			}

			playSoundAndDisplaySubTitle(_vm->_rnd.getRandomBit());
			_screen->setScreenPalette(tmpPal);
			_screen->updateScreen();
			_vm->delay(8);
		}

		if (_callbackCurrentFrame == 40 || _callbackCurrentFrame == 80 || _callbackCurrentFrame == 150 || _callbackCurrentFrame == 300)
			playSoundAndDisplaySubTitle(3);

		_screen->copyPage(12, 2);
		updateDemoAdText(70, 130);
		_screen->copyPage(2, 0);
		_screen->updateScreen();
		_callbackCurrentFrame++;
		if (_callbackCurrentFrame < 128 || _callbackCurrentFrame > 207)
			_scrollProgressCounter++;

		while (countDownRunning())
			delayTicks(1);
	}
	_screen->copyPage(2, 12);

	return 0;
}
#endif // ENABLE_LOL

#undef CASE_ALT

const uint8 SeqPlayer_HOF::_textColorPresets[] = { 0x01, 0x01, 0x00, 0x3f, 0x3f, 0x3f };

void KyraEngine_HoF::seq_showStarcraftLogo() {
	WSAMovie_v2 *ci = new WSAMovie_v2(this);
	assert(ci);
	_screen->clearPage(2);
	_res->loadPakFile("INTROGEN.PAK");
	int endframe = ci->open("CI.WSA", 0, &_screen->getPalette(0));
	_res->unloadPakFile("INTROGEN.PAK");
	if (!ci->opened()) {
		delete ci;
		return;
	}
	_screen->hideMouse();
	ci->displayFrame(0, 2, 0, 0, 0, 0, 0);
	_screen->copyPage(2, 0);
	_screen->fadeFromBlack();
	for (int i = 1; i < endframe; i++) {
		uint32 end = _system->getMillis() + 50;
		if (skipFlag())
			break;
		ci->displayFrame(i, 2, 0, 0, 0, 0, 0);
		_screen->copyPage(2, 0);
		_screen->updateScreen();
		uint32 cur = _system->getMillis();
		if (end > cur)
			delay(end - cur);
		else
			updateInput();
	}
	if (!skipFlag()) {
		uint32 end = _system->getMillis() + 50;
		ci->displayFrame(0, 2, 0, 0, 0, 0, 0);
		_screen->copyPage(2, 0);
		_screen->updateScreen();
		uint32 cur = _system->getMillis();
		if (end > cur)
			delay(end - cur);
		else
			updateInput();
	}
	_screen->fadeToBlack();
	_screen->showMouse();

	_eventList.clear();
	delete ci;
}

int KyraEngine_HoF::seq_playIntro() {
	bool startupSaveLoadable = saveFileLoadable(0);
	return SeqPlayer_HOF(this, _screen, _system, startupSaveLoadable).play(kSequenceVirgin, startupSaveLoadable? kSequenceTitle : kSequenceNoLooping);
}

int KyraEngine_HoF::seq_playOutro() {
	return SeqPlayer_HOF(this, _screen, _system).play(kSequenceFunters, kSequenceFrash);
}

int KyraEngine_HoF::seq_playDemo() {
	SeqPlayer_HOF(this, _screen, _system).play(kSequenceHoFDemoVirgin, kSequenceHoFDemoVirgin);
	return 4;
}

void KyraEngine_HoF::seq_pausePlayer(bool toggle) {
	SeqPlayer_HOF *activePlayer = SeqPlayer_HOF::instance();
	if (activePlayer)
		activePlayer->pause(toggle);
}

#ifdef ENABLE_LOL
int LoLEngine::playDemo() {
	SeqPlayer_HOF(this, _screen, _system).play(kSequenceLoLDemoScene1, kSequenceLoLDemoScene1);
	return -1;
}

void LoLEngine::pauseDemoPlayer(bool toggle) {
	SeqPlayer_HOF *activePlayer = SeqPlayer_HOF::instance();
	if (activePlayer)
		activePlayer->pause(toggle);
}
#endif // ENABLE_LOL

#pragma mark -
#pragma mark - Ingame sequences
#pragma mark -

void KyraEngine_HoF::seq_makeBookOrCauldronAppear(int type) {
	_screen->hideMouse();
	showMessage(0, 0xCF);

	if (type == 1)
		seq_makeBookAppear();
	else if (type == 2)
		loadInvWsa("CAULDRON.WSA", 1, 6, 0, -2, -2, 1);

	_screen->copyRegionToBuffer(2, 0, 0, 320, 200, _screenBuffer);
	_screen->loadBitmap("_PLAYALL.CPS", 3, 3, 0);

	static const uint8 bookCauldronRects[] = {
		0x46, 0x90, 0x7F, 0x2B,	// unknown rect (maybe unused?)
		0xCE, 0x90, 0x2C, 0x2C,	// book rect
		0xFA, 0x90, 0x46, 0x2C	// cauldron rect
	};

	int x = bookCauldronRects[type*4+0];
	int y = bookCauldronRects[type*4+1];
	int w = bookCauldronRects[type*4+2];
	int h = bookCauldronRects[type*4+3];
	_screen->copyRegion(x, y, x, y, w, h, 2, 0, Screen::CR_NO_P_CHECK);

	_screen->copyBlockToPage(2, 0, 0, 320, 200, _screenBuffer);

	if (type == 2) {
		int32 countdown = _rnd.getRandomNumberRng(45, 80);
		_timer->setCountdown(2, countdown * 60);
	}

	_screen->showMouse();
}

void KyraEngine_HoF::seq_makeBookAppear() {
	_screen->hideMouse();

	displayInvWsaLastFrame();

	showMessage(0, 0xCF);

	loadInvWsa("BOOK2.WSA", 0, 4, 2, -1, -1, 0);

	uint8 *rect = new uint8[_screen->getRectSize(_invWsa.w, _invWsa.h)];
	assert(rect);

	_screen->copyRegionToBuffer(_invWsa.page, _invWsa.x, _invWsa.y, _invWsa.w, _invWsa.h, rect);

	_invWsa.running = false;
	snd_playSoundEffect(0xAF);

	while (true) {
		_invWsa.timer = _system->getMillis() + _invWsa.delay * _tickLength;

		_screen->copyBlockToPage(_invWsa.page, _invWsa.x, _invWsa.y, _invWsa.w, _invWsa.h, rect);

		_invWsa.wsa->displayFrame(_invWsa.curFrame, _invWsa.page, 0, 0, 0x4000, 0, 0);

		if (_invWsa.page)
			_screen->copyRegion(_invWsa.x, _invWsa.y, _invWsa.x, _invWsa.y, _invWsa.w, _invWsa.h, _invWsa.page, 0, Screen::CR_NO_P_CHECK);

		++_invWsa.curFrame;

		if (_invWsa.curFrame >= _invWsa.lastFrame && !shouldQuit())
			break;

		switch (_invWsa.curFrame) {
		case 39:
			snd_playSoundEffect(0xCA);
			break;

		case 50:
			snd_playSoundEffect(0x6A);
			break;

		case 72:
			snd_playSoundEffect(0xCB);
			break;

		case 85:
			snd_playSoundEffect(0x38);
			break;

		default:
			break;
		}

		do {
			update();
		} while (_invWsa.timer > _system->getMillis() && !skipFlag());
	}

	closeInvWsa();
	delete[] rect;
	_invWsa.running = false;

	_screen->showMouse();
}

} // End of namespace Kyra

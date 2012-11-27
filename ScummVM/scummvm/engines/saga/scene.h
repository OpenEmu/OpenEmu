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

// Scene management module private header file

#ifndef SAGA_SCENE_H
#define SAGA_SCENE_H

#include "saga/font.h"
#include "saga/actor.h"
#include "saga/interface.h"
#include "saga/puzzle.h"
#include "saga/events.h"

namespace Saga {

//#define SCENE_DEBUG // for scene debugging

#define SCENE_DOORS_MAX 16
#define NO_CHAPTER_CHANGE -2

// Scenes
#define ITE_SCENE_INV -1
#define ITE_SCENE_PUZZLE 26
#define ITE_SCENE_LODGE 21
#define ITE_SCENE_ENDCREDIT1 295
#define ITE_SCENE_OVERMAP 226

// Default scenes
#define ITE_DEFAULT_SCENE 32
#define IHNM_DEFAULT_SCENE 151
#define ITEDEMO_DEFAULT_SCENE 68
#define IHNMDEMO_DEFAULT_SCENE 144

class ObjectMap;

enum SceneFlags {
	kSceneFlagISO        = 1,
	kSceneFlagShowCursor = 2
};

// FTA2 possible endings
enum FTA2Endings {
	kFta2BadEndingLaw = 0,
	kFta2BadEndingChaos = 1,
	kFta2GoodEnding1 = 2,
	kFta2GoodEnding2 = 3,
	kFta2BadEndingDeath = 4
};

struct BGInfo {
	Rect bounds;
	byte *buffer;
};

typedef int (SceneProc) (int, void *);


enum SCENE_PROC_PARAMS {
	SCENE_BEGIN = 0,
	SCENE_END
};

// Resource type numbers
enum SAGAResourceTypes {
	SAGA_UNKNOWN,
	SAGA_ACTOR,
	SAGA_OBJECT,
	SAGA_BG_IMAGE,
	SAGA_BG_MASK,
	SAGA_STRINGS,
	SAGA_OBJECT_MAP,
	SAGA_ACTION_MAP,
	SAGA_ISO_IMAGES,
	SAGA_ISO_MAP,
	SAGA_ISO_PLATFORMS,
	SAGA_ISO_METATILES,
	SAGA_ENTRY,
	SAGA_ANIM,
	SAGA_ISO_MULTI,
	SAGA_PAL_ANIM,
	SAGA_FACES,
	SAGA_PALETTE
};

#define SAGA_RESLIST_ENTRY_LEN 4

struct SceneResourceData {
	uint32 resourceId;
	int resourceType;
	bool invalid;

	SceneResourceData() : resourceId(0), resourceType(0),  invalid(false) {
	}
};

typedef Common::Array<SceneResourceData> SceneResourceDataArray;

#define SAGA_SCENE_DESC_LEN 16

struct SceneDescription {
	int16 flags;
	int16 resourceListResourceId;
	int16 endSlope;
	int16 beginSlope;
	uint16 scriptModuleNumber;
	uint16 sceneScriptEntrypointNumber;
	uint16 startScriptEntrypointNumber;
	int16 musicResourceId;

	void reset()  {
		flags = resourceListResourceId = endSlope = beginSlope = scriptModuleNumber = sceneScriptEntrypointNumber = startScriptEntrypointNumber = musicResourceId = 0;
	}
};

struct SceneEntry {
	Location location;
	uint16 facing;
};

typedef Common::Array<SceneEntry> SceneEntryList;

struct SceneImage {
	bool loaded;
	int w;
	int h;
	int p;
	ByteArray buffer;
	PalEntry pal[256];

	SceneImage() : loaded(false), w(0), h(0), p(0) {
		memset(pal, 0, sizeof(pal));
	}
};


enum SceneTransitionType {
	kTransitionNoFade,
	kTransitionFade
};

enum SceneLoadFlags {
	kLoadByResourceId,
	kLoadBySceneNumber
};

struct LoadSceneParams {
	int32 sceneDescriptor;
	SceneLoadFlags loadFlag;
	SceneProc *sceneProc;
	bool sceneSkipTarget;
	SceneTransitionType transitionType;
	int actorsEntrance;
	int chapter;
};

typedef Common::List<LoadSceneParams> SceneQueueList;

///// IHNM-specific stuff
#define IHNM_PALFADE_TIME    1000
#define IHNM_INTRO_FRAMETIME 80
#define IHNM_DGLOGO_TIME     8000
#define IHNM_TITLE_TIME_GM   28750
#define IHNM_TITLE_TIME_FM   19500

///// ITE-specific stuff
#define ITE_INTRO_FRAMETIME 90

#define INTRO_CAPTION_Y 170
#define INTRO_DE_CAPTION_Y 160
#define INTRO_IT_CAPTION_Y 160
#define VOICE_PAD 50
#define VOICE_LETTERLEN 90

#define PALETTE_FADE_DURATION 1000
#define DISSOLVE_DURATION 3000
#define LOGO_DISSOLVE_DURATION 1000

#define CREDIT_DURATION1 4000

struct IntroDialogue {
	uint32 i_voice_rn;
	const char *i_str;
};

struct IntroCredit {
	Common::Language lang;
	int game;
	int type;
	const char *string;
};


class Scene {
 public:
	Scene(SagaEngine *vm);
	~Scene();

// Console functions
	void cmdActionMapInfo();
	void cmdObjectMapInfo();

	void cmdSceneChange(int argc, const char **argv);

	void startScene();
	void creditsScene();
	void nextScene();
	void skipScene();
	void endScene();
	void restoreScene();
	void queueScene(const LoadSceneParams &sceneQueue) {
		_sceneQueue.push_back(sceneQueue);
	}

	void draw();
	int getFlags() const { return _sceneDescription.flags; }
	int getScriptModuleNumber() const { return _sceneDescription.scriptModuleNumber; }
	bool isInIntro() { return !_inGame; }
	const Rect& getSceneClip() const { return _sceneClip; }

	void getBGMaskInfo(int &width, int &height, byte *&buffer);
	int isBGMaskPresent() { return _bgMask.loaded; }

	int getBGMaskType(const Point &testPoint) {
		uint offset;
		if (!_bgMask.loaded) {
			return 0;
		}
		offset = testPoint.x + testPoint.y * _bgMask.w;

		#ifdef SCENE_DEBUG
		if (offset >= _bgMask.buf_len) {
			error("Scene::getBGMaskType offset 0x%X exceed bufferLength 0x%X", offset, (int)_bgMask.buf_len);
		}
		#endif

		return (_bgMask.buffer[offset] >> 4) & 0x0f;
	}

	bool validBGMaskPoint(const Point &testPoint) {
		#ifdef SCENE_DEBUG
		if (!_bgMask.loaded) {
			error("Scene::validBGMaskPoint _bgMask not loaded");
		}
		#endif

		return !((testPoint.x < 0) || (testPoint.x >= _bgMask.w) ||
			(testPoint.y < 0) || (testPoint.y >= _bgMask.h));
	}

	bool canWalk(const Point &testPoint);
	bool offscreenPath(Point &testPoint);

	void setDoorState(int doorNumber, int doorState) {
		#ifdef SCENE_DEBUG
		if ((doorNumber < 0) || (doorNumber >= SCENE_DOORS_MAX))
			error("Scene::setDoorState wrong doorNumber");
		#endif

		_sceneDoors[doorNumber] = doorState;
	}

	int getDoorState(int doorNumber) {
		#ifdef SCENE_DEBUG
		if ((doorNumber < 0) || (doorNumber >= SCENE_DOORS_MAX))
			error("Scene::getDoorState wrong doorNumber");
		#endif

		return _sceneDoors[doorNumber];
	}

	void initDoorsState();

	void getBGInfo(BGInfo &bgInfo);
	void getBGPal(PalEntry *&pal) {
		pal = (PalEntry *)_bg.pal;
	}

	void getSlopes(int &beginSlope, int &endSlope);

	void clearSceneQueue() {
		_sceneQueue.clear();
	}
	void changeScene(int16 sceneNumber, int actorsEntrance, SceneTransitionType transitionType, int chapter = NO_CHAPTER_CHANGE);

	bool isSceneLoaded() const { return _sceneLoaded; }

	uint16 getSceneResourceId(int sceneNumber) {
	#ifdef SCENE_DEBUG
		if ((sceneNumber < 0) || (sceneNumber >= _sceneCount)) {
			error("getSceneResourceId: wrong sceneNumber %i", sceneNumber);
		}
	#endif
		return _sceneLUT[sceneNumber];
	}
	int currentProtag() const { return _currentProtag; }
	void setProtag(int pr) { _currentProtag = pr; }
	int currentSceneNumber() const { return _sceneNumber; }
	int currentChapterNumber() const { return _chapterNumber; }
	void setChapterNumber(int ch) { _chapterNumber = ch; }
	int getOutsetSceneNumber() const { return _outsetSceneNumber; }
	int currentSceneResourceId() const { return _sceneResourceId; }
	int getCurrentMusicTrack() const { return _currentMusicTrack; }
	void setCurrentMusicTrack(int tr) { _currentMusicTrack = tr; }
	int getCurrentMusicRepeat() const { return _currentMusicRepeat; }
	void setCurrentMusicRepeat(int rp) { _currentMusicRepeat = rp; }
	bool haveChapterPointsChanged() const { return _chapterPointsChanged; }
	void setChapterPointsChanged(bool cp) { _chapterPointsChanged = cp; }

	void cutawaySkip() {
		_vm->_framesEsc = _vm->_scene->isInIntro() ? 2 : 1;
	}

	void drawTextList();

	int getHeight(bool speech = false) const {
		if (_vm->getGameId() == GID_IHNM && _vm->_scene->currentChapterNumber() == 8 && !speech)
			return _vm->getDisplayInfo().height;
		else
			return _vm->getDisplayInfo().sceneHeight;
	}

	void clearPlacard();
	void showPsychicProfile(const char *text);
	void clearPsychicProfile();
	void showIHNMDemoSpecialScreen();

	bool isNonInteractiveIHNMDemoPart() {
		return _vm->isIHNMDemo() && (_sceneNumber >= 144 && _sceneNumber <= 149);
	}

	bool isITEPuzzleScene() {
		return _vm->getGameId() == GID_ITE && _vm->_puzzle->isActive();
	}

 private:
	void loadScene(LoadSceneParams &loadSceneParams);
	void loadSceneDescriptor(uint32 resourceId);
	void loadSceneResourceList(uint32 resourceId, SceneResourceDataArray &resourceList);
	void loadSceneEntryList(const ByteArray &resourceData);
	void processSceneResources(SceneResourceDataArray &resourceList);
	void getResourceTypes(SAGAResourceTypes *&types, int &typesCount);


	SagaEngine *_vm;

	ResourceContext *_sceneContext;
	Common::Array<uint16> _sceneLUT;
	SceneQueueList _sceneQueue;
	bool _sceneLoaded;
	int _currentProtag;
	int _sceneNumber;
	int _chapterNumber;
	int _outsetSceneNumber;
	int _sceneResourceId;
	int _currentMusicTrack;
	int _currentMusicRepeat;
	bool _chapterPointsChanged;
	bool _inGame;
	SceneDescription _sceneDescription;
	SceneProc *_sceneProc;
	SceneImage _bg;
	SceneImage _bgMask;
	Common::Rect _sceneClip;

	int _sceneDoors[SCENE_DOORS_MAX];


 public:
	ObjectMap *_actionMap;
	ObjectMap *_objectMap;
	SceneEntryList _entryList;
	StringsTable _sceneStrings;
	TextList _textList;

 private:
	int ITEStartProc();
	int IHNMStartProc();
	int IHNMCreditsProc();
	int DinoStartProc();
	int FTA2StartProc();
	int FTA2EndProc(FTA2Endings whichEnding);
	void playMovie(const char *filename);

	void IHNMLoadCutaways();
	bool checkKey();

	bool playTitle(int title, int time, int mode = kPanelVideo);
	bool playLoopingTitle(int title, int seconds);

 public:
	static int SC_IHNMIntroMovieProc1(int param, void *refCon);
	static int SC_IHNMIntroMovieProc2(int param, void *refCon);
	static int SC_IHNMIntroMovieProc3(int param, void *refCon);
	static int SC_IHNMCreditsMovieProc(int param, void *refCon);

 private:
	int IHNMIntroMovieProc1(int param);
	int IHNMIntroMovieProc2(int param);
	int IHNMIntroMovieProc3(int param);
	int IHNMCreditsMovieProc(int param);

 public:
	static int SC_ITEIntroAnimProc(int param, void *refCon);
	static int SC_ITEIntroCave1Proc(int param, void *refCon);
	static int SC_ITEIntroCave2Proc(int param, void *refCon);
	static int SC_ITEIntroCave3Proc(int param, void *refCon);
	static int SC_ITEIntroCave4Proc(int param, void *refCon);
	static int SC_ITEIntroValleyProc(int param, void *refCon);
	static int SC_ITEIntroTreeHouseProc(int param, void *refCon);
	static int SC_ITEIntroFairePathProc(int param, void *refCon);
	static int SC_ITEIntroFaireTentProc(int param, void *refCon);

 private:
	EventColumns *ITEQueueDialogue(EventColumns *eventColumns, int n_dialogues, const IntroDialogue dialogue[]);
	EventColumns *ITEQueueCredits(int delta_time, int duration, int n_credits, const IntroCredit credits[]);
	int ITEIntroAnimProc(int param);
	int ITEIntroCave1Proc(int param);
	int ITEIntroCave2Proc(int param);
	int ITEIntroCave3Proc(int param);
	int ITEIntroCave4Proc(int param);
	int ITEIntroValleyProc(int param);
	int ITEIntroTreeHouseProc(int param);
	int ITEIntroFairePathProc(int param);
	int ITEIntroFaireTentProc(int param);

};

} // End of namespace Saga

#endif

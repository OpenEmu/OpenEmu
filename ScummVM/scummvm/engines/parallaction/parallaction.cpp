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

#include "common/debug-channels.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "parallaction/exec.h"
#include "parallaction/input.h"
#include "parallaction/parallaction.h"
#include "parallaction/debug.h"
#include "parallaction/saveload.h"
#include "parallaction/sound.h"
#include "parallaction/walk.h"

namespace Parallaction {
Parallaction *g_vm = NULL;
// public stuff

char		g_saveData1[30] = { '\0' };
uint32		g_engineFlags = 0;
uint32		g_globalFlags = 0;

// private stuff

Parallaction::Parallaction(OSystem *syst, const PARALLACTIONGameDescription *gameDesc) :
	Engine(syst), _gameDescription(gameDesc), _location(getGameType()),
	_dialogueMan(0), _rnd("parallaction") {
	// Setup mixer
	syncSoundSettings();

	g_vm = this;
	DebugMan.addDebugChannel(kDebugDialogue, "dialogue", "Dialogues debug level");
	DebugMan.addDebugChannel(kDebugParser, "parser", "Parser debug level");
	DebugMan.addDebugChannel(kDebugDisk, "disk", "Disk debug level");
	DebugMan.addDebugChannel(kDebugWalk, "walk", "Walk debug level");
	DebugMan.addDebugChannel(kDebugGraphics, "gfx", "Gfx debug level");
	DebugMan.addDebugChannel(kDebugExec, "exec", "Execution debug level");
	DebugMan.addDebugChannel(kDebugInput, "input", "Input debug level");
	DebugMan.addDebugChannel(kDebugAudio, "audio", "Audio debug level");
	DebugMan.addDebugChannel(kDebugMenu, "menu", "Menu debug level");
	DebugMan.addDebugChannel(kDebugInventory, "inventory", "Inventory debug level");
}

Parallaction::~Parallaction() {
	delete _debugger;
	delete _globalFlagsNames;
	delete _callableNames;
	delete _cmdExec;
	delete _programExec;
	destroyDialogueManager();
	delete _saveLoad;

	cleanupGui();

	_gfx->freeCharacterObjects();
	_gfx->freeLocationObjects();
	delete _balloonMan;
	_balloonMan = 0;

	delete _localFlagNames;
	_char._ani.reset();
	delete _gfx;
	delete _soundMan;
	delete _disk;
	delete _input;
}

Common::Error Parallaction::init() {
	_gameType = getGameType();
	g_engineFlags = 0;
	_objectsNames = NULL;
	_globalFlagsNames = NULL;
	_location._hasSound = false;
	_numLocations = 0;
	_location._startPosition.x = -1000;
	_location._startPosition.y = -1000;
	_location._startFrame = 0;
	_location._followerStartPosition.x = -1000;
	_location._followerStartPosition.y = -1000;
	_location._followerStartFrame = 0;
	_objects = 0;

	_screenSize = _screenWidth * _screenHeight;

	strcpy(_characterName1, "null");

	memset(_localFlags, 0, sizeof(_localFlags));
	memset(_locationNames, 0, NUM_LOCATIONS * 32);

	// this needs _disk to be already setup
	_input = new Input(this);

	_gfx = new Gfx(this);

	_debugger = new Debugger(this);

	_menuHelper = 0;

	return Common::kNoError;
}

void Parallaction::pauseEngineIntern(bool pause) {
	if (_soundMan) {
		_soundMan->execute(SC_PAUSE, (int)pause);
	}
}

GUI::Debugger *Parallaction::getDebugger() {
	return _debugger;
}

void Parallaction::updateView() {

	if ((g_engineFlags & kEnginePauseJobs) && (_input->_inputMode != Input::kInputModeInventory)) {
		return;
	}

	_gfx->animatePalette();
	_gfx->updateScreen();
	_system->delayMillis(30);
}

void Parallaction::pauseJobs() {
	debugC(9, kDebugExec, "pausing jobs execution");

	g_engineFlags |= kEnginePauseJobs;
	return;
}

void Parallaction::resumeJobs() {
	debugC(9, kDebugExec, "resuming jobs execution");

	g_engineFlags &= ~kEnginePauseJobs;
	return;
}

AnimationPtr Location::findAnimation(const char *name) {

	for (AnimationList::iterator it = _animations.begin(); it != _animations.end(); ++it)
		if (!scumm_stricmp((*it)->_name, name)) return *it;

	return AnimationPtr();
}

void Parallaction::allocateLocationSlot(const char *name) {
	// WORKAROUND: the original code erroneously incremented
	// _currentLocationIndex, thus producing inconsistent
	// savegames. This workaround modified the following loop
	// and if-statement, so the code exactly matches the one
	// in Big Red Adventure.
	_currentLocationIndex = -1;
	uint16 _di = 0;
	while (_locationNames[_di][0] != '\0') {
		if (!scumm_stricmp(_locationNames[_di], name)) {
			_currentLocationIndex = _di;
		}
		_di++;
	}

	if (_di == 120)
		error("No more location slots available. Please report this immediately to ScummVM team");

	if (_currentLocationIndex  == -1) {
		strcpy(_locationNames[_numLocations], name);
		_currentLocationIndex = _numLocations;

		_numLocations++;
		_locationNames[_numLocations][0] = '\0';
		_localFlags[_numLocations] = 0;
	} else {
		setLocationFlags(kFlagsVisited);	// 'visited'
	}
}

Location::Location(int gameType) : _gameType(gameType) {
	cleanup(true);
}

Location::~Location() {
	cleanup(true);
}

void Location::cleanup(bool removeAll) {
	_comment.clear();
	_endComment.clear();

	freeZones(removeAll);

	_programs.clear();
	_commands.clear();
	_aCommands.clear();

	_hasSound = false;

	// NS specific
	_walkPoints.clear();

	// BRA specific
	_zeta0 = _zeta1 = _zeta2 = 0;
	_escapeCommands.clear();
}

int Location::getScale(int z) const {
	int scale = 100;
	if (z <= _zeta0) {
		scale = _zeta2;
		if (z >= _zeta1) {
			scale += ((z - _zeta1) * (100 - _zeta2)) / (_zeta0 - _zeta1);
		}
	}
	return scale;
}

void Parallaction::showSlide(const char *name, int x, int y) {
	BackgroundInfo *info = new BackgroundInfo;
	_disk->loadSlide(*info, name);

	info->_x = (x == CENTER_LABEL_HORIZONTAL) ? ((_screenWidth - info->width) >> 1) : x;
	info->_y = (y == CENTER_LABEL_VERTICAL) ? ((_screenHeight - info->height) >> 1) : y;

	_gfx->setBackground(kBackgroundSlide, info);
}

void Parallaction::showLocationComment(const Common::String &text, bool end) {
	_balloonMan->setLocationBalloon(text, end);
}

void Parallaction::runGameFrame(int event) {
	if (_input->_inputMode != Input::kInputModeGame) {
		return;
	}

	if (!processGameEvent(event)) {
		return;
	}

	_gfx->beginFrame();

	runPendingZones();

	if (shouldQuit())
		return;

	if (g_engineFlags & kEngineChangeLocation) {
		changeLocation();
	}

	_programExec->runScripts(_location._programs.begin(), _location._programs.end());
	_char._ani->resetZ();
	updateWalkers();
	updateZones();
}

void Parallaction::runGame() {
	int event = _input->updateInput();
	if (shouldQuit())
		return;

	switch (_input->_inputMode) {
	case Input::kInputModeMenu:
		runGuiFrame();
		break;

	case Input::kInputModeDialogue:
		runDialogueFrame();
		break;

	case Input::kInputModeComment:
		runCommentFrame();
		break;

	case Input::kInputModeGame:
		runGameFrame(event);
		break;
	}

	if (shouldQuit())
		return;

	// change this to endFrame?
	updateView();
}

//	displays transition before a new location
//
//	clears screen (in white??)
//	shows location comment (if any)
//	waits for mouse click
//	fades towards game palette
//
void Parallaction::doLocationEnterTransition() {
	debugC(2, kDebugExec, "doLocationEnterTransition");

	if (_location._comment.empty()) {
		return;
	}

	if (getLocationFlags() & kFlagsVisited) {
		debugC(2, kDebugExec, "skipping location transition");
		return; // visited
	}

	Palette pal(_gfx->_palette);
	pal.makeGrayscale();
	_gfx->setPalette(pal);

	_programExec->runScripts(_location._programs.begin(), _location._programs.end());
	updateZones();
	showLocationComment(_location._comment, false);
	_gfx->updateScreen();

	_input->waitForButtonEvent(kMouseLeftUp);
	_gfx->freeDialogueObjects();

	// fades maximum intensity palette towards approximation of main palette
	for (uint16 _si = 0; _si<6; _si++) {
		pal.fadeTo(_gfx->_palette, 4);
		_gfx->setPalette(pal);
		_gfx->updateScreen();
		_system->delayMillis(20);
	}

	_gfx->setPalette(_gfx->_palette);

	debugC(2, kDebugExec, "doLocationEnterTransition completed");
}

void Parallaction::setLocationFlags(uint32 flags) {
	_localFlags[_currentLocationIndex] |= flags;
}

void Parallaction::clearLocationFlags(uint32 flags) {
	_localFlags[_currentLocationIndex] &= ~flags;
}

void Parallaction::toggleLocationFlags(uint32 flags) {
	_localFlags[_currentLocationIndex] ^= flags;
}

uint32 Parallaction::getLocationFlags() {
	return _localFlags[_currentLocationIndex];
}

void Parallaction::drawAnimation(AnimationPtr anim) {
	if ((anim->_flags & kFlagsActive) == 0)   {
		return;
	}

	GfxObj *obj = anim->gfxobj;
	if (!obj) {
		return;
	}

	// animation display defaults to topmost and no scaling
	uint16 layer = LAYER_FOREGROUND;
	uint16 scale = 100;

	switch (_gameType) {
	case GType_Nippon:
		if ((anim->_flags & kFlagsNoMasked) == 0) {
			// Layer in NS depends on where the animation is on the screen, for each animation.
			layer = _gfx->_backgroundInfo->getMaskLayer(anim->getBottom());
		}
		break;

	case GType_BRA:
		if ((anim->_flags & kFlagsNoMasked) == 0) {
			// Layer in BRA is calculated from Z value. For characters it is the same as NS,
			// but other animations can have Z set from scripts independently from their
			// position on the screen.
			layer = _gfx->_backgroundInfo->getMaskLayer(anim->getZ());
		}
		if (anim->_flags & (kFlagsScaled | kFlagsCharacter)) {
			scale = _location.getScale(anim->getZ());
		}
		break;
	}

	// updates the data for display
	_gfx->showGfxObj(obj, true);
	obj->frame = anim->getF();
	obj->x = anim->getX();
	obj->y = anim->getY();
	obj->z = anim->getZ();
	obj->layer = layer;
	obj->scale = scale;
	_gfx->addObjectToScene(obj);
}

void Parallaction::drawZone(ZonePtr zone) {
	if (!zone) {
		return;
	}

	GfxObj *obj = 0;
	if (ACTIONTYPE(zone) == kZoneGet) {
		obj = zone->u._gfxobj;
	} else
	if (ACTIONTYPE(zone) == kZoneDoor) {
		obj = zone->u._gfxobj;
	}

	if (!obj) {
		return;
	}

	obj->x = zone->getX();
	obj->y = zone->getY();
	_gfx->addObjectToScene(obj);
}

void Parallaction::updateZones() {
	debugC(9, kDebugExec, "Parallaction::updateZones()\n");

	// go through all animations and mark/unmark each of them for display
	for (AnimationList::iterator ait = _location._animations.begin(); ait != _location._animations.end(); ++ait) {
		AnimationPtr anim = *ait;
		if ((anim->_flags & kFlagsRemove) != 0)	{
			// marks the animation as invisible for this frame
			_gfx->showGfxObj(anim->gfxobj, false);
			anim->_flags &= ~(kFlagsActive | kFlagsRemove);
		} else {
			// updates animation parameters
			drawAnimation(anim);
		}
	}

	// go through all zones and mark/unmark each of them for display
	for (ZoneList::iterator zit = _location._zones.begin(); zit != _location._zones.end(); ++zit) {
		drawZone(*zit);
	}

	debugC(9, kDebugExec, "Parallaction::updateZones done()\n");
}

void Parallaction::showZone(ZonePtr z, bool visible) {
	if (!z) {
		return;
	}

	if (visible) {
		z->_flags &= ~kFlagsRemove;
		z->_flags |= kFlagsActive;
	} else {
		z->_flags |= kFlagsRemove;
	}

	if (ACTIONTYPE(z) == kZoneGet) {
		_gfx->showGfxObj(z->u._gfxobj, visible);
	}
}

//	ZONE TYPE: EXAMINE

void Parallaction::enterCommentMode(ZonePtr z) {
	if (!z) {
		return;
	}

	_commentZone = z;

	TypeData *data = &_commentZone->u;

	if (data->_examineText.empty()) {
		exitCommentMode();
		return;
	}

	// TODO: move this balloons stuff into DialogueManager and BalloonManager
	if (_gameType == GType_Nippon) {
		if (!data->_filename.empty()) {
			if (data->_gfxobj == 0) {
				data->_gfxobj = _disk->loadStatic(data->_filename.c_str());
			}

			_gfx->setHalfbriteMode(true);
			_balloonMan->setSingleBalloon(data->_examineText, 0, 90, 0, BalloonManager::kNormalColor);
			Common::Rect r;
			data->_gfxobj->getRect(0, r);
			_gfx->setItem(data->_gfxobj, 140, (_screenHeight - r.height())/2);
			_gfx->setItem(_char._head, 100, 152);
		} else {
			_balloonMan->setSingleBalloon(data->_examineText, 140, 10, 0, BalloonManager::kNormalColor);
			_gfx->setItem(_char._talk, 190, 80);
		}
	} else
	if (_gameType == GType_BRA) {
		_balloonMan->setSingleBalloon(data->_examineText, 0, 0, 1, BalloonManager::kNormalColor);
		_gfx->setItem(_char._talk, 10, 80);
	}

	_input->_inputMode = Input::kInputModeComment;
}

void Parallaction::exitCommentMode() {
	_input->_inputMode = Input::kInputModeGame;

	_gfx->freeDialogueObjects();
	_gfx->setHalfbriteMode(false);

	_cmdExec->run(_commentZone->_commands, _commentZone);
	_commentZone.reset();
}

void Parallaction::runCommentFrame() {
	if (_input->_inputMode != Input::kInputModeComment) {
		return;
	}

	if (_input->getLastButtonEvent() == kMouseLeftUp) {
		exitCommentMode();
	}
}

void Parallaction::runZone(ZonePtr z) {
	debugC(3, kDebugExec, "runZone (%s)", z->_name);

	uint16 actionType = ACTIONTYPE(z);

	debugC(3, kDebugExec, "actionType = %x, itemType = %x", actionType, ITEMTYPE(z));
	switch (actionType) {

	case kZoneExamine:
		enterCommentMode(z);
		return;

	case kZoneGet:
		pickupItem(z);
		break;

	case kZoneDoor:
		if (z->_flags & kFlagsLocked) break;
		updateDoor(z, !(z->_flags & kFlagsClosed));
		break;

	case kZoneHear:
		if (z->u._hearChannel == MUSIC_HEAR_CHANNEL) {
			_soundMan->execute(SC_SETMUSICFILE, z->u._filename.c_str());
			_soundMan->execute(SC_PLAYMUSIC);
		} else {
			_soundMan->execute(SC_SETSFXCHANNEL, z->u._hearChannel);
			_soundMan->execute(SC_SETSFXLOOPING, (int)((z->_flags & kFlagsLooping) == kFlagsLooping));
			_soundMan->execute(SC_SETSFXVOLUME, 60);
			_soundMan->execute(SC_PLAYSFX, z->u._filename.c_str());
		}
		break;

	case kZoneSpeak:
		if (z->u._speakDialogue) {
			enterDialogueMode(z);
			return;
		}
		break;
	}

	debugC(3, kDebugExec, "runZone completed");

	_cmdExec->run(z->_commands, z);

	return;
}

//	ZONE TYPE: DOOR

void Parallaction::updateDoor(ZonePtr z, bool close) {
	z->_flags = close ? (z->_flags |= kFlagsClosed) : (z->_flags &= ~kFlagsClosed);

	if (z->u._gfxobj) {
		uint frame = (close ? 0 : 1);
//		z->u._gfxobj->setFrame(frame);
		z->u._gfxobj->frame = frame;
	}

	return;
}

//	ZONE TYPE: GET

bool Parallaction::pickupItem(ZonePtr z) {
	if (z->_flags & kFlagsFixed) {
		return false;
	}

	int slot = addInventoryItem(z->u._getIcon);
	if (slot != -1) {
		showZone(z, false);
	}

	return (slot != -1);
}

bool Parallaction::checkSpecialZoneBox(ZonePtr z, uint32 type, uint x, uint y) {
	// check if really a special zone
	if (_gameType == GType_Nippon) {
		// so-called special zones in NS have special x coordinates
		if ((z->getX() != -2) && (z->getX() != -3)) {
			return false;
		}
	}
	if (_gameType == GType_BRA) {
		// so far, special zones in BRA are only merge zones
		if (ACTIONTYPE(z) != kZoneMerge) {
			return false;
		}
	}

	// WORKAROUND: this huge condition is needed because we made TypeData a collection of structs
	// instead of an union. So, merge->_obj1 and get->_icon were just aliases in the original engine,
	// but we need to check it separately here. The same workaround is applied in freeZones.
	if (((ACTIONTYPE(z) == kZoneMerge) && (((x == z->u._mergeObj1) && (y == z->u._mergeObj2)) || ((x == z->u._mergeObj2) && (y == z->u._mergeObj1)))) ||
		((ACTIONTYPE(z) == kZoneGet) && ((x == z->u._getIcon) || (y == z->u._getIcon)))) {

		// WORKAROUND for bug 2070751: special zones are only used in NS, to allow the
		// the EXAMINE/USE action to be applied on some particular item in the inventory.
		// The usage a verb requires at least an item match, so type can't be 0, as it
		// was in the original code. This bug has been here since the beginning, and was
		// hidden by label code, which filtered the bogus matches produced here.

		// look for action + item match
		if (z->_type == type)
			return true;
		// look for item match, but don't accept 0 types
		if ((ITEMTYPE(z) == type) && (type))
			return true;
	}

	return false;
}

bool Parallaction::checkZoneType(ZonePtr z, uint32 type) {
	if (_gameType == GType_Nippon) {
		if ((type == 0) && (ITEMTYPE(z) == 0))
			return true;
	}

	if (_gameType == GType_BRA) {
		if (type == 0) {
			if (ITEMTYPE(z) == 0) {
				if (ACTIONTYPE(z) != kZonePath) {
					return true;
				}
			}
			if (ACTIONTYPE(z) == kZoneDoor) {
				return true;
			}
		}
	}

	if (z->_type == type)
		return true;
	if (ITEMTYPE(z) == type)
		return true;

	return false;
}

bool Parallaction::checkZoneBox(ZonePtr z, uint32 type, uint x, uint y) {
	if (z->_flags & kFlagsRemove)
		return false;

	debugC(5, kDebugExec, "checkZoneBox for %s (type = %x, x = %i, y = %i)", z->_name, type, x, y);

	if (!z->hitRect(x, y)) {
		// check for special zones (items defined in common.loc)
		if (checkSpecialZoneBox(z, type, x, y))
			return true;

		// check if self-use zone (nothing to do with kFlagsSelfuse)
		if (_gameType == GType_Nippon) {
			if (z->getX() != -1) {	// no explicit self-use flag in NS
				return false;
			}
		}
		if (_gameType == GType_BRA) {
			if (!(z->_flags & kFlagsYourself)) {
				return false;
			}
		}
		if (!_char._ani->hitFrameRect(x, y)) {
			return false;
		}
		// we get here only if (x,y) hits the character and the zone is marked as self-use
	}

	return checkZoneType(z, type);
}

bool Parallaction::checkLinkedAnimBox(ZonePtr z, uint32 type, uint x, uint y) {
	if (z->_flags & kFlagsRemove)
		return false;

	// flag kFlagsAnimLinked may be on, but the animation may not be loaded, so
	// we must use the animation reference to check here
	if (!z->_linkedAnim)
		return false;

	debugC(5, kDebugExec, "checkLinkedAnimBox for %s (type = %x, x = %i, y = %i)", z->_name, type, x, y);

	if (!z->_linkedAnim->hitFrameRect(x, y)) {
		return false;
	}

	return checkZoneType(z, type);
}

// NOTE: hitZone needs to be passed absolute game coordinates to work.
//
// When type is kZoneMerge, then x and y are the identifiers of the objects to merge,
// and the above requirement does not apply.
ZonePtr Parallaction::hitZone(uint32 type, uint16 x, uint16 y) {
	uint16 _di = y;
	uint16 _si = x;

	for (ZoneList::iterator it = _location._zones.begin(); it != _location._zones.end(); ++it) {
		if (checkLinkedAnimBox(*it, type, x, y)) {
			return *it;
		}
		if (checkZoneBox(*it, type, x, y)) {
			return *it;
		}
	}

	int16 _a, _b, _c, _d;
	bool _ef;
	for (AnimationList::iterator ait = _location._animations.begin(); ait != _location._animations.end(); ++ait) {

		AnimationPtr a = *ait;

		_a = (a->_flags & kFlagsActive) ? 1 : 0;	// _a: active Animation

		if (!_a) {
			if (_gameType == GType_BRA && ACTIONTYPE(a) != kZoneTrap) {
				continue;
			}
		}

		_ef = a->hitFrameRect(_si, _di);

		_b = ((type != 0) || (a->_type == kZoneYou)) ? 0 : 1;										 // _b: (no type specified) AND (Animation is not the character)
		_c = ITEMTYPE(a) ? 0 : 1;															// _c: Animation is not an object
		_d = (ITEMTYPE(a) != type) ? 0 : 1;													// _d: Animation is an object of the same type

		if ((_a != 0 && _ef) && ((_b != 0 && _c != 0) || (a->_type == type) || (_d != 0))) {
			return a;
		}
	}

	return ZonePtr();
}

ZonePtr Location::findZone(const char *name) {
	for (ZoneList::iterator it = _zones.begin(); it != _zones.end(); ++it) {
		if (!scumm_stricmp((*it)->_name, name)) return *it;
	}
	return findAnimation(name);
}

bool Location::keepZone_ns(ZonePtr z) {
	return (z->getY() == -1) || (z->getX() == -2);
}

bool Location::keepAnimation_ns(AnimationPtr a) {
	return false;
}

bool Location::keepZone_br(ZonePtr z) {
	return (z->_flags & kFlagsSelfuse) || (ACTIONTYPE(z) == kZoneMerge);
}

bool Location::keepAnimation_br(AnimationPtr a) {
	return keepZone_br(a);
}

template<class T>
void Location::freeList(Common::List<T> &list, bool removeAll, Common::MemFunc1<bool, T, Location> filter) {
	typedef typename Common::List<T>::iterator iterator;
	iterator it = list.begin();
	while (it != list.end()) {
		T z = *it;
		if (!removeAll && filter(this, z)) {
			++it;
		} else {
			z->_commands.clear();
			it = list.erase(it);
		}
	}
}

void Location::freeZones(bool removeAll) {
	debugC(2, kDebugExec, "freeZones: removeAll = %i", removeAll);

	switch (_gameType) {
	case GType_Nippon:
		freeList(_zones, removeAll, Common::mem_fun(&Location::keepZone_ns));
		freeList(_animations, removeAll, Common::mem_fun(&Location::keepAnimation_ns));
		break;

	case GType_BRA:
		freeList(_zones, removeAll, Common::mem_fun(&Location::keepZone_br));
		freeList(_animations, removeAll, Common::mem_fun(&Location::keepAnimation_br));
		break;
	}
}

Character::Character() : _ani(new Animation) {
	_talk = NULL;
	_head = NULL;

	_ani->setX(150);
	_ani->setY(100);
	_ani->setZ(10);
	_ani->setF(0);
	_ani->_flags = kFlagsActive | kFlagsNoName | kFlagsCharacter;
	_ani->_type = kZoneYou;
	strncpy(_ani->_name, "yourself", ZONENAME_LENGTH);
}

void Character::setName(const char *name) {
	_name.bind(name);
}

const char *Character::getName() const {
	return _name.getName();
}

const char *Character::getBaseName() const {
	return _name.getBaseName();
}

const char *Character::getFullName() const {
	return _name.getFullName();
}

bool Character::dummy() const {
	return _name.dummy();
}

// Various ways of detecting character modes used to exist
// inside the engine, so they have been unified in the two
// following macros.
// Mini characters are those used in far away shots, like
// the highway scenery, while Dummy characters are a mere
// workaround to keep the engine happy when showing slides.
// As a sidenote, standard sized characters' names start
// with a lowercase 'd'.
#define IS_MINI_CHARACTER(s) (((s)[0] == 'm'))
#define IS_DUMMY_CHARACTER(s) (((s)[0] == 'D'))

const char CharacterName::_prefixMini[] = "mini";
const char CharacterName::_suffixTras[] = "tras";
const char CharacterName::_empty[] = "\0";

void CharacterName::dummify() {
	_dummy = true;
	_baseName[0] = '\0';
	_name[0] = '\0';
	_fullName[0] = '\0';
}

CharacterName::CharacterName() {
	dummify();
}

CharacterName::CharacterName(const char *name) {
	bind(name);
}

void CharacterName::bind(const char *name) {
	const char *begin = name;
	const char *end = begin + strlen(name);

	_prefix = _empty;
	_suffix = _empty;

	_dummy = IS_DUMMY_CHARACTER(name);

	if (!_dummy) {
		if (!strcmp(name, "donna")) {
			g_engineFlags &= ~kEngineTransformedDonna;
		} else {
			if (g_engineFlags & kEngineTransformedDonna) {
				_suffix = _suffixTras;
			} else {
				const char *s = strstr(name, "tras");
				if (s) {
					g_engineFlags |= kEngineTransformedDonna;
					_suffix = _suffixTras;
					end = s;
				}
			}
			if (IS_MINI_CHARACTER(name)) {
				_prefix = _prefixMini;
				begin = name + 4;
			}
		}
	}

	memset(_baseName, 0, 30);
	strncpy(_baseName, begin, end - begin);
	sprintf(_name, "%s%s", _prefix, _baseName);
	sprintf(_fullName, "%s%s%s", _prefix, _baseName, _suffix);
}

const char *CharacterName::getName() const {
	return _name;
}

const char *CharacterName::getBaseName() const {
	return _baseName;
}

const char *CharacterName::getFullName() const {
	return _fullName;
}

bool CharacterName::dummy() const {
	return _dummy;
}

void Parallaction::beep() {
	if (_gameType == GType_Nippon) {
		_soundMan->execute(SC_SETSFXCHANNEL, 3);
		_soundMan->execute(SC_SETSFXVOLUME, 127);
		_soundMan->execute(SC_SETSFXLOOPING, (int32)0);
		_soundMan->execute(SC_PLAYSFX, "beep");
	}
}

void Parallaction::scheduleLocationSwitch(const char *location) {
	debugC(9, kDebugExec, "scheduleLocationSwitch(%s)\n", location);
	_newLocationName = location;
	g_engineFlags |= kEngineChangeLocation;
}

} // End of namespace Parallaction

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

#ifndef PARALLACTION_H
#define PARALLACTION_H

#include "common/str.h"
#include "common/stack.h"
#include "common/array.h"
#include "common/func.h"
#include "common/random.h"
#include "common/savefile.h"
#include "common/textconsole.h"

#include "engines/engine.h"

#include "parallaction/input.h"
#include "parallaction/inventory.h"
#include "parallaction/objects.h"
#include "parallaction/disk.h"

#define PATH_LEN	200


/**
 * This is the namespace of the Parallaction engine.
 *
 * Status of this engine: ???
 *
 * Games using this engine:
 * - Nippon Safes Inc. (complete)
 * - The Big Red Adventure (work in progress)
 */
namespace Parallaction {

enum {
	kDebugDisk = 1 << 0,
	kDebugWalk = 1 << 1,
	kDebugParser = 1 << 2,
	kDebugDialogue = 1 << 3,
	kDebugGraphics = 1 << 4,
	kDebugExec = 1 << 5,
	kDebugInput = 1 << 6,
	kDebugAudio = 1 << 7,
	kDebugMenu = 1 << 8,
	kDebugInventory = 1 << 9
};

enum {
	GF_DEMO = 1 << 0,
	GF_LANG_EN = 1 << 1,
	GF_LANG_FR = 1 << 2,
	GF_LANG_DE = 1 << 3,
	GF_LANG_IT = 1 << 4,
	GF_LANG_MULT = 1 << 5
};


enum EngineFlags {
	kEnginePauseJobs	= (1 << 1),
	kEngineWalking		= (1 << 3),
	kEngineChangeLocation	= (1 << 4),
	kEngineBlockInput	= (1 << 5),
	kEngineDragging		= (1 << 6),
	kEngineTransformedDonna	= (1 << 7),

	// BRA specific
	kEngineReturn		= (1 << 10)
};

enum {
	kEvNone			= 0,
	kEvSaveGame		= 2000,
	kEvLoadGame		= 4000,
	kEvIngameMenu   = 8000
};

enum ParallactionGameType {
	GType_Nippon = 1,
	GType_BRA
};

struct PARALLACTIONGameDescription;




extern uint32		g_engineFlags;
extern char			g_saveData1[];
extern uint32		g_globalFlags;
extern const char	*g_dinoName;
extern const char	*g_donnaName;
extern const char	*g_doughName;
extern const char	*g_drkiName;
extern const char	*g_minidinoName;
extern const char	*g_minidonnaName;
extern const char	*g_minidoughName;
extern const char	*g_minidrkiName;






class Debugger;
class Gfx;
class Input;
class DialogueManager;
class MenuInputHelper;
class PathWalker_NS;
class PathWalker_BR;
class CommandExec;
class ProgramExec;
class SoundMan;
class SoundMan_ns;
class SoundMan_br;
class LocationParser_ns;
class LocationParser_br;
class ProgramParser_ns;
class ProgramParser_br;
class BalloonManager;

struct Location {

	Common::Point	_startPosition;
	uint16			_startFrame;
	char			_name[100];

	CommandList		_aCommands;
	CommandList		_commands;
	Common::String	_comment;
	Common::String	_endComment;

	ZoneList		_zones;
	AnimationList	_animations;
	ProgramList		_programs;

	bool		_hasSound;
	char		_soundFile[50];

	// NS specific
	PointList	_walkPoints;
	Common::String _slideText[2];

	// BRA specific
	int			_zeta0;
	int			_zeta1;
	int			_zeta2;
	CommandList		_escapeCommands;
	Common::Point	_followerStartPosition;
	uint16			_followerStartFrame;


protected:
	int			_gameType;

	bool keepZone_br(ZonePtr z);
	bool keepZone_ns(ZonePtr z);
	bool keepAnimation_ns(AnimationPtr a);
	bool keepAnimation_br(AnimationPtr a);

	template<class T>
	void freeList(Common::List<T> &list, bool removeAll, Common::MemFunc1<bool, T, Location> filter);

public:
	Location(int gameType);
	~Location();

	AnimationPtr findAnimation(const char *name);
	ZonePtr findZone(const char *name);

	void cleanup(bool removeAll);
	void freeZones(bool removeAll);

	int getScale(int z) const;
};


class CharacterName {
	const char *_prefix;
	const char *_suffix;
	bool _dummy;
	char _name[30];
	char _baseName[30];
	char _fullName[30];
	static const char _prefixMini[];
	static const char _suffixTras[];
	static const char _empty[];
	void dummify();
public:
	CharacterName();
	CharacterName(const char *name);
	void bind(const char *name);
	const char *getName() const;
	const char *getBaseName() const;
	const char *getFullName() const;
	bool dummy() const;
};


struct Character {
	AnimationPtr	_ani;
	GfxObj			*_head;
	GfxObj			*_talk;

	Character();

protected:
	CharacterName	_name;

public:
	void setName(const char *name);
	const char *getName() const;
	const char *getBaseName() const;
	const char *getFullName() const;
	bool dummy() const;
};


class SaveLoad;

#define NUM_LOCATIONS 120

class Parallaction : public Engine {
	friend class Debugger;

public:
	int getGameType() const;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;

protected:		// members
	bool detectGame();

private:
	const PARALLACTIONGameDescription *_gameDescription;
	uint16	_language;

public:
	Parallaction(OSystem *syst, const PARALLACTIONGameDescription *gameDesc);
	~Parallaction();

	// Engine APIs
	virtual Common::Error init();
	virtual Common::Error go() = 0;
	virtual Common::Error run() {
		Common::Error err;
		err = init();
		if (err.getCode() != Common::kNoError)
			return err;
		return go();
	}

	virtual bool hasFeature(EngineFeature f) const;
	virtual void pauseEngineIntern(bool pause);
	virtual GUI::Debugger *getDebugger();

	// info
	int32			_screenWidth;
	int32			_screenHeight;
	int32			_screenSize;
	int				_gameType;

	// subsystems
	Gfx				*_gfx;
	Disk			*_disk;
	Input			*_input;
	Debugger		*_debugger;
	SaveLoad		*_saveLoad;
	MenuInputHelper *_menuHelper;
	Common::RandomSource _rnd;
	SoundMan		*_soundMan;

	// fonts
	Font		*_labelFont;
	Font		*_menuFont;
	Font		*_introFont;
	Font		*_dialogueFont;

	// game utilities
	Table				*_globalFlagsNames;
	Table				*_objectsNames;
	GfxObj				*_objects;
	Table				*_callableNames;
	Table				*_localFlagNames;
	CommandExec			*_cmdExec;
	ProgramExec			*_programExec;
	BalloonManager		*_balloonMan;
	DialogueManager		*_dialogueMan;
	InventoryRenderer	*_inventoryRenderer;
	Inventory			*_inventory;			// inventory for the current character

	// game data
	Character		_char;
	uint32			_localFlags[NUM_LOCATIONS];
	char			_locationNames[NUM_LOCATIONS][32];
	int16			_currentLocationIndex;
	uint16			_numLocations;
	Location		_location;
	ZonePtr			_activeZone;
	char			_characterName1[50];	// only used in changeCharacter
	ZonePtr			_zoneTrap;
	ZonePtr			_commentZone;
	Common::String  _newLocationName;

protected:
	void	runGame();
	void	runGameFrame(int event);
	void	runGuiFrame();
	void	cleanupGui();
	void	runDialogueFrame();
	void	exitDialogueMode();
	void	runCommentFrame();
	void	enterCommentMode(ZonePtr z);
	void	exitCommentMode();
	void	updateView();
	void	drawAnimation(AnimationPtr anim);
	void	drawZone(ZonePtr zone);
	void	updateZones();
	void	doLocationEnterTransition();
	void	allocateLocationSlot(const char *name);
	void	finalizeLocationParsing();
	void	showLocationComment(const Common::String &text, bool end);
	void	destroyDialogueManager();

public:
	void	beep();
	void	pauseJobs();
	void	resumeJobs();
	uint	getInternLanguage();
	void	setInternLanguage(uint id);
	void	enterDialogueMode(ZonePtr z);
	void	scheduleLocationSwitch(const char *location);
	void	showSlide(const char *name, int x = 0, int y = 0);

public:
	void		setLocationFlags(uint32 flags);
	void		clearLocationFlags(uint32 flags);
	void		toggleLocationFlags(uint32 flags);
	uint32		getLocationFlags();
	bool		checkSpecialZoneBox(ZonePtr z, uint32 type, uint x, uint y);
	bool		checkZoneBox(ZonePtr z, uint32 type, uint x, uint y);
	bool 		checkZoneType(ZonePtr z, uint32 type);
	bool		checkLinkedAnimBox(ZonePtr z, uint32 type, uint x, uint y);
	ZonePtr		hitZone(uint32 type, uint16 x, uint16 y);
	void		runZone(ZonePtr z);
	bool		pickupItem(ZonePtr z);
	void		updateDoor(ZonePtr z, bool close);
	void		showZone(ZonePtr z, bool visible);
	void		highlightInventoryItem(ItemPosition pos);
	int16		getHoverInventoryItem(int16 x, int16 y);
	int		addInventoryItem(ItemName item);
	int		addInventoryItem(ItemName item, uint32 value);
	void		dropItem(uint16 v);
	bool		isItemInInventory(int32 v);
	const		InventoryItem* getInventoryItem(int16 pos);
	int16		getInventoryItemIndex(int16 pos);
	void		cleanInventory(bool keepVerbs = true);
	void		openInventory();
	void		closeInventory();

	virtual void parseLocation(const char* name) = 0;
	virtual void changeLocation() = 0;
	virtual void changeCharacter(const char *name) = 0;
	virtual	void callFunction(uint index, void* parm) = 0;
	virtual void runPendingZones() = 0;
	virtual void cleanupGame() = 0;
	virtual void updateWalkers() = 0;
	virtual void scheduleWalk(int16 x, int16 y, bool fromUser) = 0;
	virtual DialogueManager *createDialogueManager(ZonePtr z) = 0;
	virtual bool processGameEvent(int event) = 0;
};



class Parallaction_ns : public Parallaction {

public:
	Parallaction_ns(OSystem* syst, const PARALLACTIONGameDescription *gameDesc);
	~Parallaction_ns();

	// Engine APIs
	virtual Common::Error init();
	virtual Common::Error go();

	SoundMan_ns*	_soundManI;

	uint16			_score;
	Common::String	_password;


public:
	virtual void parseLocation(const char *filename);
	virtual void changeLocation();
	virtual void changeCharacter(const char *name);
	virtual void callFunction(uint index, void* parm);
	virtual void runPendingZones();
	virtual void cleanupGame();
	virtual void updateWalkers();
	virtual void scheduleWalk(int16 x, int16 y, bool fromUser);
	virtual DialogueManager *createDialogueManager(ZonePtr z);
	virtual bool processGameEvent(int event);

	void	changeBackground(const char *background, const char *mask = 0, const char *path = 0);

private:
	bool				_inTestResult;
	LocationParser_ns	*_locationParser;
	ProgramParser_ns	*_programParser;

private:
	void	initFonts();
	void	freeFonts();
	void	initResources();
	void	initInventory();
	void	destroyInventory();
	void	setupBalloonManager();
	void	startGui();
	void	startCreditSequence();
	void	startEndPartSequence();
	void	loadProgram(AnimationPtr a, const char *filename);
	void	freeLocation(bool removeAll);
	void	freeCharacter();
	void	destroyTestResultLabels();
	void	startMovingSarcophagus(ZonePtr sarc);
	void	stopMovingSarcophagus();


	//  callables data
	typedef void (Parallaction_ns::*Callable)(void *);
	const Callable *_callables;
	ZonePtr _moveSarcGetZone;
	ZonePtr _moveSarcExaZone;
	ZonePtr _moveSarcGetZones[5];
	ZonePtr _moveSarcExaZones[5];
	uint16 num_foglie;

	int16 _sarcophagusDeltaX;
	bool	_movingSarcophagus;		 // sarcophagus stuff to be saved
	uint16	_freeSarcophagusSlotX;		 // sarcophagus stuff to be saved
	AnimationPtr _rightHandAnim;
	bool _intro;
	static const Callable _dosCallables[25];
	static const Callable _amigaCallables[25];

	GfxObj *_testResultLabels[2];

	PathWalker_NS		*_walker;

	// common callables
	void _c_play_boogie(void *);
	void _c_startIntro(void *);
	void _c_endIntro(void *);
	void _c_moveSheet(void *);
	void _c_sketch(void *);
	void _c_shade(void *);
	void _c_score(void *);
	void _c_fade(void *);
	void _c_moveSarc(void *);
	void _c_contaFoglie(void *);
	void _c_zeroFoglie(void *);
	void _c_trasformata(void *);
	void _c_offMouse(void *);
	void _c_onMouse(void *);
	void _c_setMask(void *);
	void _c_endComment(void *);
	void _c_frankenstein(void *);
	void _c_finito(void *);
	void _c_ridux(void *);
	void _c_testResult(void *);

	// dos specific callables
	void _c_null(void *);

	// amiga specific callables
	void _c_projector(void *);
	void _c_HBOff(void *);
	void _c_offSound(void *);
	void _c_startMusic(void *);
	void _c_closeMusic(void *);
	void _c_HBOn(void *);
};



#define NUM_ZONES	100

class Parallaction_br : public Parallaction {

public:
	Parallaction_br(OSystem* syst, const PARALLACTIONGameDescription *gameDesc);
	~Parallaction_br();

	virtual Common::Error init();
	virtual Common::Error go();

public:
	virtual void parseLocation(const char* name);
	virtual void changeLocation();
	virtual void changeCharacter(const char *name);
	virtual	void callFunction(uint index, void* parm);
	virtual void runPendingZones();
	virtual void cleanupGame();
	virtual void updateWalkers();
	virtual void scheduleWalk(int16 x, int16 y, bool fromUser);
	virtual DialogueManager *createDialogueManager(ZonePtr z);
	virtual bool processGameEvent(int event);

	void setupSubtitles(char *s, char *s2, int y);
	void clearSubtitles();

	void testCounterCondition(const Common::String &name, int op, int value);
	void restoreOrSaveZoneFlags(ZonePtr z, bool restore);

public:
	bool	counterExists(const Common::String &name);
	int		getCounterValue(const Common::String &name);
	void	setCounterValue(const Common::String &name, int value);

	void	setFollower(const Common::String &name);

	int		getSfxStatus();
	int		getMusicStatus();
	void	enableSfx(bool enable);
	void	enableMusic(bool enable);

	const char **_audioCommandsNamesRes;
	static const char *_partNames[];
	int			_part;
	int			_nextPart;


#if 0	// disabled since I couldn't find any references to lip sync in the scripts
	int16		_lipSyncVal;
	uint		_subtitleLipSync;
#endif
	int			_subtitleY;
	GfxObj		*_subtitle[2];
	ZonePtr		_activeZone2;
	uint32		_zoneFlags[NUM_LOCATIONS][NUM_ZONES];


private:
	LocationParser_br		*_locationParser;
	ProgramParser_br		*_programParser;
	SoundMan_br				*_soundManI;
	Inventory				*_charInventories[3];	// all the inventories

	int32		_counters[32];
	Table		*_countersNames;

private:
	void	initResources();
	void	initInventory();
	void	destroyInventory();
	void	setupBalloonManager();
	void	initFonts();
	void	freeFonts();
	void	freeLocation(bool removeAll);
	void	loadProgram(AnimationPtr a, const char *filename);
	void	startGui(bool showSplash);
	void 	startIngameMenu();
	void	freeCharacter();

	typedef void (Parallaction_br::*Callable)(void *);
	const Callable *_callables;
	static const Callable _dosCallables[6];
	static const Callable _amigaCallables[6];

	Common::String		_followerName;
	AnimationPtr		_follower;
	PathWalker_BR		*_walker;

	// dos callables
	void _c_null(void *);
	void _c_blufade(void *);
	void _c_resetpalette(void *);
	void _c_ferrcycle(void *);
	void _c_lipsinc(void *);
	void _c_albcycle(void *);
	void _c_password(void *);
};

extern Parallaction *g_vm;


} // End of namespace Parallaction


#endif

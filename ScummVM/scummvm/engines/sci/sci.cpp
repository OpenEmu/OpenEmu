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

#include "common/system.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"

#include "engines/advancedDetector.h"
#include "engines/util.h"

#include "sci/sci.h"
#include "sci/debug.h"
#include "sci/console.h"
#include "sci/event.h"

#include "sci/engine/features.h"
#include "sci/engine/message.h"
#include "sci/engine/object.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/engine/script.h"	// for script_adjust_opcode_formats
#include "sci/engine/selector.h"	// for SELECTOR

#include "sci/sound/audio.h"
#include "sci/sound/music.h"
#include "sci/sound/soundcmd.h"
#include "sci/graphics/animate.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/compare.h"
#include "sci/graphics/controls16.h"
#include "sci/graphics/controls32.h"
#include "sci/graphics/coordadjuster.h"
#include "sci/graphics/cursor.h"
#include "sci/graphics/maciconbar.h"
#include "sci/graphics/menu.h"
#include "sci/graphics/paint16.h"
#include "sci/graphics/paint32.h"
#include "sci/graphics/picture.h"
#include "sci/graphics/ports.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/text16.h"
#include "sci/graphics/transitions.h"

#ifdef ENABLE_SCI32
#include "sci/graphics/text32.h"
#include "sci/graphics/frameout.h"
#include "sci/video/robot_decoder.h"
#endif

namespace Sci {

SciEngine *g_sci = 0;


class GfxDriver;

SciEngine::SciEngine(OSystem *syst, const ADGameDescription *desc, SciGameId gameId)
		: Engine(syst), _gameDescription(desc), _gameId(gameId), _rng("sci") {

	assert(g_sci == 0);
	g_sci = this;

	_gfxMacIconBar = 0;

	_audio = 0;
	_features = 0;
	_resMan = 0;
	_gamestate = 0;
	_kernel = 0;
	_vocabulary = 0;
	_vocabularyLanguage = 1; // we load english vocabulary on startup
	_eventMan = 0;
	_console = 0;
	_opcode_formats = 0;

	// Set up the engine specific debug levels
	DebugMan.addDebugChannel(kDebugLevelError, "Error", "Script error debugging");
	DebugMan.addDebugChannel(kDebugLevelNodes, "Lists", "Lists and nodes debugging");
	DebugMan.addDebugChannel(kDebugLevelGraphics, "Graphics", "Graphics debugging");
	DebugMan.addDebugChannel(kDebugLevelStrings, "Strings", "Strings debugging");
	DebugMan.addDebugChannel(kDebugLevelMemory, "Memory", "Memory debugging");
	DebugMan.addDebugChannel(kDebugLevelFuncCheck, "Func", "Function parameter debugging");
	DebugMan.addDebugChannel(kDebugLevelBresen, "Bresenham", "Bresenham algorithms debugging");
	DebugMan.addDebugChannel(kDebugLevelSound, "Sound", "Sound debugging");
	DebugMan.addDebugChannel(kDebugLevelBaseSetter, "Base", "Base Setter debugging");
	DebugMan.addDebugChannel(kDebugLevelParser, "Parser", "Parser debugging");
	DebugMan.addDebugChannel(kDebugLevelSaid, "Said", "Said specs debugging");
	DebugMan.addDebugChannel(kDebugLevelFile, "File", "File I/O debugging");
	DebugMan.addDebugChannel(kDebugLevelTime, "Time", "Time debugging");
	DebugMan.addDebugChannel(kDebugLevelRoom, "Room", "Room number debugging");
	DebugMan.addDebugChannel(kDebugLevelAvoidPath, "Pathfinding", "Pathfinding debugging");
	DebugMan.addDebugChannel(kDebugLevelDclInflate, "DCL", "DCL inflate debugging");
	DebugMan.addDebugChannel(kDebugLevelVM, "VM", "VM debugging");
	DebugMan.addDebugChannel(kDebugLevelScripts, "Scripts", "Notifies when scripts are unloaded");
	DebugMan.addDebugChannel(kDebugLevelGC, "GC", "Garbage Collector debugging");
	DebugMan.addDebugChannel(kDebugLevelResMan, "ResMan", "Resource manager debugging");
	DebugMan.addDebugChannel(kDebugLevelOnStartup, "OnStartup", "Enter debugger at start of game");
	DebugMan.addDebugChannel(kDebugLevelDebugMode, "DebugMode", "Enable game debug mode at start of game");

	const Common::FSNode gameDataDir(ConfMan.get("path"));

	SearchMan.addSubDirectoryMatching(gameDataDir, "actors");	// KQ6 hi-res portraits
	SearchMan.addSubDirectoryMatching(gameDataDir, "aud");	// resource.aud and audio files
	SearchMan.addSubDirectoryMatching(gameDataDir, "audio");// resource.aud and audio files
	SearchMan.addSubDirectoryMatching(gameDataDir, "audiosfx");// resource.aud and audio files
	SearchMan.addSubDirectoryMatching(gameDataDir, "wav");	// speech files in WAV format
	SearchMan.addSubDirectoryMatching(gameDataDir, "sfx");	// music/sound files in WAV format
	SearchMan.addSubDirectoryMatching(gameDataDir, "avi");	// AVI movie files for Windows versions
	SearchMan.addSubDirectoryMatching(gameDataDir, "seq");	// SEQ movie files for DOS versions
	SearchMan.addSubDirectoryMatching(gameDataDir, "robot");	// robot movie files
	SearchMan.addSubDirectoryMatching(gameDataDir, "robots");	// robot movie files
	SearchMan.addSubDirectoryMatching(gameDataDir, "movie");	// VMD movie files
	SearchMan.addSubDirectoryMatching(gameDataDir, "movies");	// VMD movie files
	SearchMan.addSubDirectoryMatching(gameDataDir, "vmd");	// VMD movie files
	SearchMan.addSubDirectoryMatching(gameDataDir, "duk");	// Duck movie files in Phantasmagoria 2
	SearchMan.addSubDirectoryMatching(gameDataDir, "Robot Folder"); // Mac robot files
	SearchMan.addSubDirectoryMatching(gameDataDir, "Sound Folder"); // Mac audio files
	SearchMan.addSubDirectoryMatching(gameDataDir, "Voices Folder"); // Mac audio36 files
	SearchMan.addSubDirectoryMatching(gameDataDir, "Voices"); // Mac audio36 files
	SearchMan.addSubDirectoryMatching(gameDataDir, "VMD Folder"); // Mac VMD files

	// Add the patches directory, except for KQ6CD; The patches folder in some versions of KQ6CD
	// is for the demo of Phantasmagoria, included in the disk
	if (_gameId != GID_KQ6)
		SearchMan.addSubDirectoryMatching(gameDataDir, "patches");	// resource patches
}

SciEngine::~SciEngine() {
	// Remove all of our debug levels here
	DebugMan.clearAllDebugChannels();

#ifdef ENABLE_SCI32
	delete _gfxControls32;
	delete _gfxText32;
	delete _robotDecoder;
	delete _gfxFrameout;
#endif
	delete _gfxMenu;
	delete _gfxControls16;
	delete _gfxText16;
	delete _gfxAnimate;
	delete _gfxPaint;
	delete _gfxTransitions;
	delete _gfxCompare;
	delete _gfxCoordAdjuster;
	delete _gfxPorts;
	delete _gfxCache;
	delete _gfxPalette;
	delete _gfxCursor;
	delete _gfxScreen;

	delete _audio;
	delete _soundCmd;
	delete _kernel;
	delete _vocabulary;
	delete _console;
	delete _features;
	delete _gfxMacIconBar;

	delete _eventMan;
	delete _gamestate->_segMan;
	delete _gamestate;

	delete[] _opcode_formats;

	delete _resMan;	// should be deleted last
	g_sci = 0;
}

extern void showScummVMDialog(const Common::String &message);

Common::Error SciEngine::run() {
	// Assign default values to the config manager, in case settings are missing
	ConfMan.registerDefault("originalsaveload", "false");
	ConfMan.registerDefault("native_fb01", "false");
	ConfMan.registerDefault("windows_cursors", "false");	// Windows cursors for KQ6 Windows
	ConfMan.registerDefault("silver_cursors", "false");	// Silver cursors for SQ4 CD

	_resMan = new ResourceManager();
	assert(_resMan);
	_resMan->addAppropriateSources();
	_resMan->init();

	// TODO: Add error handling. Check return values of addAppropriateSources
	// and init. We first have to *add* sensible return values, though ;).
/*
	if (!_resMan) {
		warning("No resources found, aborting");
		return Common::kNoGameDataFoundError;
	}
*/

	// Reset, so that error()s before SoundCommandParser is initialized wont cause a crash
	_soundCmd = NULL;

	// Add the after market GM patches for the specified game, if they exist
	_resMan->addNewGMPatch(_gameId);
	_gameObjectAddress = _resMan->findGameObject();

	SegManager *segMan = new SegManager(_resMan);

	// Initialize the game screen
	_gfxScreen = new GfxScreen(_resMan);
	_gfxScreen->enableUndithering(ConfMan.getBool("disable_dithering"));

	_kernel = new Kernel(_resMan, segMan);

	_features = new GameFeatures(segMan, _kernel);
	// Only SCI0, SCI01 and SCI1 EGA games used a parser
	_vocabulary = (getSciVersion() <= SCI_VERSION_1_EGA_ONLY) ? new Vocabulary(_resMan, false) : NULL;
	// Also, XMAS1990 apparently had a parser too. Refer to http://forums.scummvm.org/viewtopic.php?t=9135
	if (getGameId() == GID_CHRISTMAS1990)
		_vocabulary = new Vocabulary(_resMan, false);
	_audio = new AudioPlayer(_resMan);
	_gamestate = new EngineState(segMan);
	_eventMan = new EventManager(_resMan->detectFontExtended());

	// Create debugger console. It requires GFX and _gamestate to be initialized
	_console = new Console(this);

	// The game needs to be initialized before the graphics system is initialized, as
	// the graphics code checks parts of the seg manager upon initialization (e.g. for
	// the presence of the fastCast object)
	if (!initGame()) { /* Initialize */
		warning("Game initialization failed: Aborting...");
		// TODO: Add an "init failed" error?
		return Common::kUnknownError;
	}

	// we try to find the super class address of the game object, we can't do that earlier
	const Object *gameObject = segMan->getObject(_gameObjectAddress);
	if (!gameObject) {
		warning("Could not get game object, aborting...");
		return Common::kUnknownError;
	}

	script_adjust_opcode_formats();

	// Must be called after game_init(), as they use _features
	_kernel->loadKernelNames(_features);
	_soundCmd = new SoundCommandParser(_resMan, segMan, _kernel, _audio, _features->detectDoSoundType());

	syncSoundSettings();
	syncIngameAudioOptions();

	// Load our Mac executable here for icon bar palettes and high-res fonts
	loadMacExecutable();

	// Initialize all graphics related subsystems
	initGraphics();

	// Patch in our save/restore code, so that dialogs are replaced
	patchGameSaveRestore();
	setLauncherLanguage();

	// Check whether loading a savestate was requested
	int directSaveSlotLoading = ConfMan.getInt("save_slot");
	if (directSaveSlotLoading >= 0) {
		// call GameObject::play (like normally)
		initStackBaseWithSelector(SELECTOR(play));
		// We set this, so that the game automatically quit right after init
		_gamestate->variables[VAR_GLOBAL][4] = TRUE_REG;

		_gamestate->_executionStackPosChanged = false;
		run_vm(_gamestate);

		// As soon as we get control again, actually restore the game
		reg_t restoreArgv[2] = { NULL_REG, make_reg(0, directSaveSlotLoading) };	// special call (argv[0] is NULL)
		kRestoreGame(_gamestate, 2, restoreArgv);

		// this indirectly calls GameObject::init, which will setup menu, text font/color codes etc.
		//  without this games would be pretty badly broken
	}

	// Show any special warnings for buggy scripts with severe game bugs,
	// which have been patched by Sierra
	if (getGameId() == GID_LONGBOW) {
		// Longbow 1.0 has a buggy script which prevents the game
		// from progressing during the Green Man riddle sequence.
		// A patch for this buggy script has been released by Sierra,
		// and is necessary to complete the game without issues.
		// The patched script is included in Longbow 1.1.
		// Refer to bug #3036609.
		Resource *buggyScript = _resMan->findResource(ResourceId(kResourceTypeScript, 180), 0);

		if (buggyScript && (buggyScript->size == 12354 || buggyScript->size == 12362)) {
			showScummVMDialog("A known buggy game script has been detected, which could "
			                  "prevent you from progressing later on in the game, during "
			                  "the sequence with the Green Man's riddles. Please, apply "
			                  "the latest patch for this game by Sierra to avoid possible "
			                  "problems");
		}
	}

	// Show a warning if the user has selected a General MIDI device, no GM patch exists
	// (i.e. patch 4) and the game is one of the known 8 SCI1 games that Sierra has provided
	// after market patches for in their "General MIDI Utility".
	if (_soundCmd->getMusicType() == MT_GM && !ConfMan.getBool("native_mt32")) {
		if (!_resMan->findResource(ResourceId(kResourceTypePatch, 4), 0)) {
			switch (getGameId()) {
			case GID_ECOQUEST:
			case GID_HOYLE3:
			case GID_LSL1:
			case GID_LSL5:
			case GID_LONGBOW:
			case GID_SQ1:
			case GID_SQ4:
			case GID_FAIRYTALES:
				showScummVMDialog("You have selected General MIDI as a sound device. Sierra "
				                  "has provided after-market support for General MIDI for this "
				                  "game in their \"General MIDI Utility\". Please, apply this "
				                  "patch in order to enjoy MIDI music with this game. Once you "
				                  "have obtained it, you can unpack all of the included *.PAT "
				                  "files in your ScummVM extras folder and ScummVM will add the "
				                  "appropriate patch automatically. Alternatively, you can follow "
				                  "the instructions in the READ.ME file included in the patch and "
				                  "rename the associated *.PAT file to 4.PAT and place it in the "
				                  "game folder. Without this patch, General MIDI music for this "
				                  "game will sound badly distorted.");
				break;
			default:
				break;
			}
		}
	}

	if (gameHasFanMadePatch()) {
		showScummVMDialog("Your game is patched with a fan made script patch. Such patches have "
		                  "been reported to cause issues, as they modify game scripts extensively. "
		                  "The issues that these patches fix do not occur in ScummVM, so you are "
		                  "advised to remove this patch from your game folder in order to avoid "
		                  "having unexpected errors and/or issues later on.");
	}

	runGame();

	ConfMan.flushToDisk();

	return Common::kNoError;
}

bool SciEngine::gameHasFanMadePatch() {
	struct FanMadePatchInfo {
		SciGameId gameID;
		uint16 targetScript;
		uint16 targetSize;
		uint16 patchedByteOffset;
		byte patchedByte;
	};

	const FanMadePatchInfo patchInfo[] = {
		// game        script    size  offset   byte
		// ** NRS Patches **************************
		{ GID_HOYLE3,     994,   2580,    656,  0x78 },
		{ GID_KQ1,         85,   5156,    631,  0x02 },
		{ GID_LAURABOW2,  994,   4382,      0,  0x00 },
		{ GID_LONGBOW,    994,   4950,   1455,  0x78 },	// English
		{ GID_LONGBOW,    994,   5020,   1469,  0x78 },	// German
		{ GID_LSL1,       803,    592,    342,  0x01 },
		{ GID_LSL3,       380,   6148,    195,  0x35 },
		{ GID_LSL5,       994,   4810,   1342,  0x78 },	// English
		{ GID_LSL5,       994,   4942,   1392,  0x76 },	// German
		{ GID_PQ1,        994,   4332,   1473,  0x78 },
		{ GID_PQ2,        200,  10614,      0,  0x00 },
		{ GID_PQ3,        994,   4686,   1291,  0x78 },	// English
		{ GID_PQ3,        994,   4734,   1283,  0x78 },	// German
		{ GID_QFG1VGA,    994,   4388,      0,  0x00 },
		{ GID_QFG3,        33,    260,      0,  0x00 },
		// TODO: Disabled, as it fixes a whole lot of bugs which can't be tested till SCI2.1 support is finished
		//{ GID_QFG4,       710,  11477,      0,  0x00 },
		{ GID_SQ1,        994,   4740,      0,  0x00 },
		{ GID_SQ5,        994,   4142,   1496,  0x78 },	// English/German/French
		// TODO: Disabled, till we can test the Italian version
		//{ GID_SQ5,        994,   4148,      0,  0x00 },	// Italian - patched file is the same size as the original
		// TODO: The bugs in SQ6 can't be tested till SCI2.1 support is finished
		//{ GID_SQ6,        380,  16308,  15042,  0x0C },	// English
		//{ GID_SQ6,        380,  11652,      0,  0x00 },	// German - patched file is the same size as the original
		// ** End marker ***************************
		{ GID_FANMADE,      0,      0,      0,  0x00 }
	};

	int curEntry = 0;

	while (true) {
		if (patchInfo[curEntry].targetSize == 0)
			break;

		if (patchInfo[curEntry].gameID == getGameId()) {
			Resource *targetScript = _resMan->findResource(ResourceId(kResourceTypeScript, patchInfo[curEntry].targetScript), 0);

			if (targetScript && targetScript->size + 2 == patchInfo[curEntry].targetSize) {
				if (patchInfo[curEntry].patchedByteOffset == 0)
					return true;
				else if (targetScript->data[patchInfo[curEntry].patchedByteOffset - 2] == patchInfo[curEntry].patchedByte)
					return true;
			}
		}

		curEntry++;
	}

	return false;
}

static byte patchGameRestoreSave[] = {
	0x39, 0x03,        // pushi 03
	0x76,              // push0
	0x38, 0xff, 0xff,  // pushi -1
	0x76,              // push0
	0x43, 0xff, 0x06,  // callk kRestoreGame/kSaveGame (will get changed afterwards)
	0x48,              // ret
};

// SCI2 version: Same as above, but the second parameter to callk is a word
static byte patchGameRestoreSaveSci2[] = {
	0x39, 0x03,        // pushi 03
	0x76,              // push0
	0x38, 0xff, 0xff,  // pushi -1
	0x76,              // push0
	0x43, 0xff, 0x06, 0x00, // callk kRestoreGame/kSaveGame (will get changed afterwards)
	0x48,              // ret
};

// SCI21 version: Same as above, but the second parameter to callk is a word
static byte patchGameRestoreSaveSci21[] = {
	0x39, 0x04,        // pushi 04
	0x76,              // push0	// 0: save, 1: restore (will get changed afterwards)
	0x76,              // push0
	0x38, 0xff, 0xff,  // pushi -1
	0x76,              // push0
	0x43, 0xff, 0x08, 0x00, // callk kSave (will get changed afterwards)
	0x48,              // ret
};

static void patchGameSaveRestoreCode(SegManager *segMan, reg_t methodAddress, byte id) {
	Script *script = segMan->getScript(methodAddress.getSegment());
	byte *patchPtr = const_cast<byte *>(script->getBuf(methodAddress.getOffset()));
	if (getSciVersion() <= SCI_VERSION_1_1)
		memcpy(patchPtr, patchGameRestoreSave, sizeof(patchGameRestoreSave));
	else	// SCI2+
		memcpy(patchPtr, patchGameRestoreSaveSci2, sizeof(patchGameRestoreSaveSci2));
	patchPtr[8] = id;
}

static void patchGameSaveRestoreCodeSci21(SegManager *segMan, reg_t methodAddress, byte id, bool doRestore) {
	Script *script = segMan->getScript(methodAddress.getSegment());
	byte *patchPtr = const_cast<byte *>(script->getBuf(methodAddress.getOffset()));
	memcpy(patchPtr, patchGameRestoreSaveSci21, sizeof(patchGameRestoreSaveSci21));
	if (doRestore)
		patchPtr[2] = 0x78;	// push1
	patchPtr[9] = id;
}

void SciEngine::patchGameSaveRestore() {
	SegManager *segMan = _gamestate->_segMan;
	const Object *gameObject = segMan->getObject(_gameObjectAddress);
	const Object *gameSuperObject = segMan->getObject(gameObject->getSuperClassSelector());
	if (!gameSuperObject)
		gameSuperObject = gameObject;	// happens in KQ5CD, when loading saved games before r54510
	byte kernelIdRestore = 0;
	byte kernelIdSave = 0;

	switch (_gameId) {
	case GID_HOYLE1: // gets confused, although the game doesnt support saving/restoring at all
	case GID_HOYLE2: // gets confused, see hoyle1
	case GID_JONES: // gets confused, when we patch us in, the game is only able to save to 1 slot, so hooking is not required
	case GID_MOTHERGOOSE256: // mother goose saves/restores directly and has no save/restore dialogs
	case GID_PHANTASMAGORIA: // has custom save/load code
	case GID_SHIVERS: // has custom save/load code
		return;
	default:
		break;
	}

	if (ConfMan.getBool("originalsaveload"))
		return;

	uint16 kernelNamesSize = _kernel->getKernelNamesSize();
	for (uint16 kernelNr = 0; kernelNr < kernelNamesSize; kernelNr++) {
		Common::String kernelName = _kernel->getKernelName(kernelNr);
		if (kernelName == "RestoreGame")
			kernelIdRestore = kernelNr;
		if (kernelName == "SaveGame")
			kernelIdSave = kernelNr;
		if (kernelName == "Save")
			kernelIdSave = kernelIdRestore = kernelNr;
	}

	// Search for gameobject superclass ::restore
	uint16 gameSuperObjectMethodCount = gameSuperObject->getMethodCount();
	for (uint16 methodNr = 0; methodNr < gameSuperObjectMethodCount; methodNr++) {
		uint16 selectorId = gameSuperObject->getFuncSelector(methodNr);
		Common::String methodName = _kernel->getSelectorName(selectorId);
		if (methodName == "restore") {
			if (kernelIdSave != kernelIdRestore)
				patchGameSaveRestoreCode(segMan, gameSuperObject->getFunction(methodNr), kernelIdRestore);
			else
				patchGameSaveRestoreCodeSci21(segMan, gameSuperObject->getFunction(methodNr), kernelIdRestore, true);
		}
		else if (methodName == "save") {
			if (_gameId != GID_FAIRYTALES) {	// Fairy Tales saves automatically without a dialog
				if (kernelIdSave != kernelIdRestore)
					patchGameSaveRestoreCode(segMan, gameSuperObject->getFunction(methodNr), kernelIdSave);
				else
					patchGameSaveRestoreCodeSci21(segMan, gameSuperObject->getFunction(methodNr), kernelIdSave, false);
			}
		}
	}

	// Search for gameobject ::save, if there is one patch that one too
	uint16 gameObjectMethodCount = gameObject->getMethodCount();
	for (uint16 methodNr = 0; methodNr < gameObjectMethodCount; methodNr++) {
		uint16 selectorId = gameObject->getFuncSelector(methodNr);
		Common::String methodName = _kernel->getSelectorName(selectorId);
		if (methodName == "save") {
			if (_gameId != GID_FAIRYTALES) {	// Fairy Tales saves automatically without a dialog
				if (kernelIdSave != kernelIdRestore)
					patchGameSaveRestoreCode(segMan, gameObject->getFunction(methodNr), kernelIdSave);
				else
					patchGameSaveRestoreCodeSci21(segMan, gameObject->getFunction(methodNr), kernelIdSave, false);
			}
			break;
		}
	}
}

bool SciEngine::initGame() {
	// Script 0 needs to be allocated here before anything else!
	int script0Segment = _gamestate->_segMan->getScriptSegment(0, SCRIPT_GET_LOCK);
	DataStack *stack = _gamestate->_segMan->allocateStack(VM_STACK_SIZE, NULL);

	_gamestate->_msgState = new MessageState(_gamestate->_segMan);
	_gamestate->gcCountDown = GC_INTERVAL - 1;

	// Script 0 should always be at segment 1
	if (script0Segment != 1) {
		debug(2, "Failed to instantiate script.000");
		return false;
	}

	_gamestate->initGlobals();
	_gamestate->_segMan->initSysStrings();

	_gamestate->r_acc = _gamestate->r_prev = NULL_REG;

	_gamestate->_executionStack.clear();    // Start without any execution stack
	_gamestate->executionStackBase = -1; // No vm is running yet
	_gamestate->_executionStackPosChanged = false;

	_gamestate->abortScriptProcessing = kAbortNone;
	_gamestate->gameIsRestarting = GAMEISRESTARTING_NONE;

	_gamestate->stack_base = stack->_entries;
	_gamestate->stack_top = stack->_entries + stack->_capacity;

	if (!_gamestate->_segMan->instantiateScript(0)) {
		error("initGame(): Could not instantiate script 0");
		return false;
	}

	// Reset parser
	if (_vocabulary)
		_vocabulary->reset();

	_gamestate->lastWaitTime = _gamestate->_screenUpdateTime = g_system->getMillis();

	// Load game language into printLang property of game object
	setSciLanguage();

	return true;
}

void SciEngine::initGraphics() {

	// Reset all graphics objects
	_gfxAnimate = 0;
	_gfxCache = 0;
	_gfxCompare = 0;
	_gfxControls16 = 0;
	_gfxCoordAdjuster = 0;
	_gfxCursor = 0;
	_gfxMacIconBar = 0;
	_gfxMenu = 0;
	_gfxPaint = 0;
	_gfxPaint16 = 0;
	_gfxPalette = 0;
	_gfxPorts = 0;
	_gfxText16 = 0;
	_gfxTransitions = 0;
#ifdef ENABLE_SCI32
	_gfxControls32 = 0;
	_gfxText32 = 0;
	_robotDecoder = 0;
	_gfxFrameout = 0;
	_gfxPaint32 = 0;
#endif

	if (hasMacIconBar())
		_gfxMacIconBar = new GfxMacIconBar();

	_gfxPalette = new GfxPalette(_resMan, _gfxScreen);
	_gfxCache = new GfxCache(_resMan, _gfxScreen, _gfxPalette);
	_gfxCursor = new GfxCursor(_resMan, _gfxPalette, _gfxScreen);

#ifdef ENABLE_SCI32
	if (getSciVersion() >= SCI_VERSION_2) {
		// SCI32 graphic objects creation
		_gfxCoordAdjuster = new GfxCoordAdjuster32(_gamestate->_segMan);
		_gfxCursor->init(_gfxCoordAdjuster, _eventMan);
		_gfxCompare = new GfxCompare(_gamestate->_segMan, _kernel, _gfxCache, _gfxScreen, _gfxCoordAdjuster);
		_gfxPaint32 = new GfxPaint32(_resMan, _gamestate->_segMan, _kernel, _gfxCoordAdjuster, _gfxCache, _gfxScreen, _gfxPalette);
		_gfxPaint = _gfxPaint32;
		_gfxText32 = new GfxText32(_gamestate->_segMan, _gfxCache, _gfxScreen);
		_gfxControls32 = new GfxControls32(_gamestate->_segMan, _gfxCache, _gfxScreen, _gfxText32);
		_robotDecoder = new RobotDecoder(getPlatform() == Common::kPlatformMacintosh);
		_gfxFrameout = new GfxFrameout(_gamestate->_segMan, _resMan, _gfxCoordAdjuster, _gfxCache, _gfxScreen, _gfxPalette, _gfxPaint32);
	} else {
#endif
		// SCI0-SCI1.1 graphic objects creation
		_gfxPorts = new GfxPorts(_gamestate->_segMan, _gfxScreen);
		_gfxCoordAdjuster = new GfxCoordAdjuster16(_gfxPorts);
		_gfxCursor->init(_gfxCoordAdjuster, _eventMan);
		_gfxCompare = new GfxCompare(_gamestate->_segMan, _kernel, _gfxCache, _gfxScreen, _gfxCoordAdjuster);
		_gfxTransitions = new GfxTransitions(_gfxScreen, _gfxPalette);
		_gfxPaint16 = new GfxPaint16(_resMan, _gamestate->_segMan, _kernel, _gfxCache, _gfxPorts, _gfxCoordAdjuster, _gfxScreen, _gfxPalette, _gfxTransitions, _audio);
		_gfxPaint = _gfxPaint16;
		_gfxAnimate = new GfxAnimate(_gamestate, _gfxCache, _gfxPorts, _gfxPaint16, _gfxScreen, _gfxPalette, _gfxCursor, _gfxTransitions);
		_gfxText16 = new GfxText16(_resMan, _gfxCache, _gfxPorts, _gfxPaint16, _gfxScreen);
		_gfxControls16 = new GfxControls16(_gamestate->_segMan, _gfxPorts, _gfxPaint16, _gfxText16, _gfxScreen);
		_gfxMenu = new GfxMenu(_eventMan, _gamestate->_segMan, _gfxPorts, _gfxPaint16, _gfxText16, _gfxScreen, _gfxCursor);

		_gfxMenu->reset();
#ifdef ENABLE_SCI32
	}
#endif

	if (_gfxPorts) {
		_gfxPorts->init(_features->usesOldGfxFunctions(), _gfxPaint16, _gfxText16);
		_gfxPaint16->init(_gfxAnimate, _gfxText16);
	}
	// Set default (EGA, amiga or resource 999) palette
	_gfxPalette->setDefault();
}

void SciEngine::initStackBaseWithSelector(Selector selector) {
	_gamestate->stack_base[0] = make_reg(0, (uint16)selector);
	_gamestate->stack_base[1] = NULL_REG;

	// Register the first element on the execution stack
	if (!send_selector(_gamestate, _gameObjectAddress, _gameObjectAddress, _gamestate->stack_base, 2, _gamestate->stack_base)) {
		_console->printObject(_gameObjectAddress);
		error("initStackBaseWithSelector: error while registering the first selector in the call stack");
	}

}

void SciEngine::runGame() {
	setTotalPlayTime(0);

	initStackBaseWithSelector(SELECTOR(play)); // Call the play selector

	// Attach the debug console on game startup, if requested
	if (DebugMan.isDebugChannelEnabled(kDebugLevelOnStartup))
		_console->attach();

	_gamestate->_syncedAudioOptions = false;

	do {
		_gamestate->_executionStackPosChanged = false;
		run_vm(_gamestate);
		exitGame();

		_gamestate->_syncedAudioOptions = true;

		if (_gamestate->abortScriptProcessing == kAbortRestartGame) {
			_gamestate->_segMan->resetSegMan();
			initGame();
			initStackBaseWithSelector(SELECTOR(play));
			patchGameSaveRestore();
			setLauncherLanguage();
			_gamestate->gameIsRestarting = GAMEISRESTARTING_RESTART;
			_gamestate->_throttleLastTime = 0;
			if (_gfxMenu)
				_gfxMenu->reset();
			_gamestate->abortScriptProcessing = kAbortNone;
		} else if (_gamestate->abortScriptProcessing == kAbortLoadGame) {
			_gamestate->abortScriptProcessing = kAbortNone;
			_gamestate->_executionStack.clear();
			initStackBaseWithSelector(SELECTOR(replay));
			patchGameSaveRestore();
			setLauncherLanguage();
			_gamestate->shrinkStackToBase();
			_gamestate->abortScriptProcessing = kAbortNone;

			syncSoundSettings();
			syncIngameAudioOptions();
		} else {
			break;	// exit loop
		}
	} while (true);
}

void SciEngine::exitGame() {
	if (_gamestate->abortScriptProcessing != kAbortLoadGame) {
		_gamestate->_executionStack.clear();
		_audio->stopAllAudio();
		_soundCmd->clearPlayList();
	}

	// TODO Free parser segment here

	// TODO Free scripts here

	// Close all opened file handles
	_gamestate->_fileHandles.clear();
	_gamestate->_fileHandles.resize(5);
}

// Invoked by error() when a severe error occurs
GUI::Debugger *SciEngine::getDebugger() {
	if (_gamestate) {
		ExecStack *xs = &(_gamestate->_executionStack.back());
		if (xs) {
			xs->addr.pc.setOffset(_debugState.old_pc_offset);
			xs->sp = _debugState.old_sp;
		}
	}

	_debugState.runningStep = 0; // Stop multiple execution
	_debugState.seeking = kDebugSeekNothing; // Stop special seeks

	return _console;
}

// Used to obtain the engine's console in order to print messages to it
Console *SciEngine::getSciDebugger() {
	return _console;
}

const char *SciEngine::getGameIdStr() const {
	return _gameDescription->gameid;
}

Common::Language SciEngine::getLanguage() const {
	return _gameDescription->language;
}

Common::Platform SciEngine::getPlatform() const {
	return _gameDescription->platform;
}

bool SciEngine::isDemo() const {
	return _gameDescription->flags & ADGF_DEMO;
}

bool SciEngine::isCD() const {
	return _gameDescription->flags & ADGF_CD;
}

bool SciEngine::isBE() const{
	switch(_gameDescription->platform) {
	case Common::kPlatformAmiga:
	case Common::kPlatformMacintosh:
		return true;
	default:
		return false;
	}
}

bool SciEngine::hasMacIconBar() const {
	return _resMan->isSci11Mac() && getSciVersion() == SCI_VERSION_1_1 &&
			(getGameId() == GID_KQ6 || getGameId() == GID_FREDDYPHARKAS);
}

Common::String SciEngine::getSavegameName(int nr) const {
	return _targetName + Common::String::format(".%03d", nr);
}

Common::String SciEngine::getSavegamePattern() const {
	return _targetName + ".???";
}

Common::String SciEngine::getFilePrefix() const {
	return _targetName;
}

Common::String SciEngine::wrapFilename(const Common::String &name) const {
	return getFilePrefix() + "-" + name;
}

Common::String SciEngine::unwrapFilename(const Common::String &name) const {
	Common::String prefix = getFilePrefix() + "-";
	if (name.hasPrefix(prefix.c_str()))
		return Common::String(name.c_str() + prefix.size());
	return name;
}

int SciEngine::inQfGImportRoom() const {
	if (_gameId == GID_QFG2 && _gamestate->currentRoomNumber() == 805) {
		// QFG2 character import screen
		return 2;
	} else if (_gameId == GID_QFG3 && _gamestate->currentRoomNumber() == 54) {
		// QFG3 character import screen
		return 3;
	} else if (_gameId == GID_QFG4 && _gamestate->currentRoomNumber() == 54) {
		return 4;
	}
	return 0;
}

void SciEngine::setLauncherLanguage() {
	if (_gameDescription->flags & ADGF_ADDENGLISH) {
		// If game is multilingual
		if (Common::parseLanguage(ConfMan.get("language")) == Common::EN_ANY) {
			// and English was selected as language
			if (SELECTOR(printLang) != -1) // set text language to English
				writeSelectorValue(_gamestate->_segMan, _gameObjectAddress, SELECTOR(printLang), K_LANG_ENGLISH);
			if (SELECTOR(parseLang) != -1) // and set parser language to English as well
				writeSelectorValue(_gamestate->_segMan, _gameObjectAddress, SELECTOR(parseLang), K_LANG_ENGLISH);
		}
	}
}

void SciEngine::pauseEngineIntern(bool pause) {
	_mixer->pauseAll(pause);
	if (_soundCmd)
		_soundCmd->pauseAll(pause);
}

void SciEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	int soundVolumeMusic = (mute ? 0 : ConfMan.getInt("music_volume"));

	if (_gamestate && _soundCmd) {
		int vol =  (soundVolumeMusic + 1) * MUSIC_MASTERVOLUME_MAX / Audio::Mixer::kMaxMixerVolume;
		_soundCmd->setMasterVolume(vol);
	}
}

void SciEngine::syncIngameAudioOptions() {
	// Now, sync the in-game speech/subtitles settings for SCI1.1 CD games
	if (isCD() && getSciVersion() == SCI_VERSION_1_1) {
		bool subtitlesOn = ConfMan.getBool("subtitles");
		bool speechOn = !ConfMan.getBool("speech_mute");

		if (subtitlesOn && !speechOn) {
			_gamestate->variables[VAR_GLOBAL][90] = make_reg(0, 1);	// subtitles
		} else if (!subtitlesOn && speechOn) {
			_gamestate->variables[VAR_GLOBAL][90] = make_reg(0, 2);	// speech
		} else if (subtitlesOn && speechOn) {
			// Is it a game that supports simultaneous speech and subtitles?
			if (getGameId() == GID_SQ4
				|| getGameId() == GID_FREDDYPHARKAS
				|| getGameId() == GID_ECOQUEST
				|| getGameId() == GID_LSL6
				// TODO: The following need script patches for simultaneous speech and subtitles
				//|| getGameId() == GID_KQ6
				//|| getGameId() == GID_LAURABOW2
				) {
				_gamestate->variables[VAR_GLOBAL][90] = make_reg(0, 3);	// speech + subtitles
			} else {
				// Game does not support speech and subtitles, set it to speech
				_gamestate->variables[VAR_GLOBAL][90] = make_reg(0, 2);	// speech
			}
		}
	}
}

void SciEngine::loadMacExecutable() {
	if (getPlatform() != Common::kPlatformMacintosh || getSciVersion() < SCI_VERSION_1_EARLY || getSciVersion() > SCI_VERSION_1_1)
		return;

	Common::String filename;

	switch (getGameId()) {
	case GID_KQ6:
		filename = "King's Quest VI";
		break;
	case GID_FREDDYPHARKAS:
		filename = "Freddy Pharkas";
		break;
	default:
		break;
	}

	if (filename.empty())
		return;

	if (!_macExecutable.open(filename) || !_macExecutable.hasResFork()) {
		// KQ6/Freddy require the executable to load their icon bar palettes
		if (hasMacIconBar())
			error("Could not load Mac resource fork '%s'", filename.c_str());

		// TODO: Show some sort of warning dialog saying they can't get any
		// high-res Mac fonts, when we get to that point ;)
	}
}

} // End of namespace Sci

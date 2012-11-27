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

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/system.h"
#include "common/translation.h"
#include "common/textconsole.h"

#include "mohawk/cursors.h"
#include "mohawk/myst.h"
#include "mohawk/myst_areas.h"
#include "mohawk/myst_graphics.h"
#include "mohawk/myst_scripts.h"
#include "mohawk/myst_state.h"
#include "mohawk/dialogs.h"
#include "mohawk/resource.h"
#include "mohawk/resource_cache.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"

// The stacks
#include "mohawk/myst_stacks/channelwood.h"
#include "mohawk/myst_stacks/credits.h"
#include "mohawk/myst_stacks/demo.h"
#include "mohawk/myst_stacks/dni.h"
#include "mohawk/myst_stacks/intro.h"
#include "mohawk/myst_stacks/makingof.h"
#include "mohawk/myst_stacks/mechanical.h"
#include "mohawk/myst_stacks/myst.h"
#include "mohawk/myst_stacks/preview.h"
#include "mohawk/myst_stacks/selenitic.h"
#include "mohawk/myst_stacks/slides.h"
#include "mohawk/myst_stacks/stoneship.h"

namespace Mohawk {

MohawkEngine_Myst::MohawkEngine_Myst(OSystem *syst, const MohawkGameDescription *gamedesc) : MohawkEngine(syst, gamedesc) {
	DebugMan.addDebugChannel(kDebugVariable, "Variable", "Track Variable Accesses");
	DebugMan.addDebugChannel(kDebugSaveLoad, "SaveLoad", "Track Save/Load Function");
	DebugMan.addDebugChannel(kDebugView, "View", "Track Card File (VIEW) Parsing");
	DebugMan.addDebugChannel(kDebugHint, "Hint", "Track Cursor Hints (HINT) Parsing");
	DebugMan.addDebugChannel(kDebugResource, "Resource", "Track Resource (RLST) Parsing");
	DebugMan.addDebugChannel(kDebugINIT, "Init", "Track Card Init Script (INIT) Parsing");
	DebugMan.addDebugChannel(kDebugEXIT, "Exit", "Track Card Exit Script (EXIT) Parsing");
	DebugMan.addDebugChannel(kDebugScript, "Script", "Track Script Execution");
	DebugMan.addDebugChannel(kDebugHelp, "Help", "Track Help File (HELP) Parsing");
	DebugMan.addDebugChannel(kDebugCache, "Cache", "Track Resource Cache Accesses");

	// Engine tweaks
	// Disabling this makes engine behavior as per
	// original, including bugs, missing bits etc. :)
	_tweaksEnabled = true;

	_currentCursor = 0;
	_mainCursor = kDefaultMystCursor;
	_showResourceRects = false;
	_curCard = 0;
	_needsUpdate = false;
	_curResource = -1;
	_hoverResource = 0;
	_dragResource = 0;

	_gfx = NULL;
	_console = NULL;
	_scriptParser = NULL;
	_gameState = NULL;
	_loadDialog = NULL;
	_optionsDialog = NULL;

	_cursorHintCount = 0;
	_cursorHints = NULL;

	_prevStack = NULL;

	_view.conditionalImageCount = 0;
	_view.conditionalImages = NULL;
	_view.soundList = NULL;
	_view.soundListVolume = NULL;
	_view.scriptResCount = 0;
	_view.scriptResources = NULL;
}

MohawkEngine_Myst::~MohawkEngine_Myst() {
	DebugMan.clearAllDebugChannels();

	delete _gfx;
	delete _console;
	delete _scriptParser;
	delete _gameState;
	delete _loadDialog;
	delete _optionsDialog;
	delete _prevStack;
	delete _rnd;

	delete[] _cursorHints;

	delete[] _view.conditionalImages;
	delete[] _view.scriptResources;

	for (uint32 i = 0; i < _resources.size(); i++)
		delete _resources[i];

	_resources.clear();
}

// Uses cached data objects in preference to disk access
Common::SeekableReadStream *MohawkEngine_Myst::getResource(uint32 tag, uint16 id) {
	Common::SeekableReadStream *ret = _cache.search(tag, id);

	if (ret)
		return ret;

	for (uint32 i = 0; i < _mhk.size(); i++)
		if (_mhk[i]->hasResource(tag, id)) {
			ret = _mhk[i]->getResource(tag, id);
			_cache.add(tag, id, ret);
			return ret;
		}

	error("Could not find a \'%s\' resource with ID %04x", tag2str(tag), id);
	return NULL;
}

void MohawkEngine_Myst::cachePreload(uint32 tag, uint16 id) {
	if (!_cache.enabled)
		return;

	for (uint32 i = 0; i < _mhk.size(); i++) {
		// Check for MJMP in Myst ME
		if ((getFeatures() & GF_ME) && tag == ID_MSND && _mhk[i]->hasResource(ID_MJMP, id)) {
			Common::SeekableReadStream *tempData = _mhk[i]->getResource(ID_MJMP, id);
			uint16 msndId = tempData->readUint16LE();
			delete tempData;

			// We've found where the real MSND data is, so go get that
			tempData = _mhk[i]->getResource(tag, msndId);
			_cache.add(tag, id, tempData);
			delete tempData;
			return;
		}

		if (_mhk[i]->hasResource(tag, id)) {
			Common::SeekableReadStream *tempData = _mhk[i]->getResource(tag, id);
			_cache.add(tag, id, tempData);
			delete tempData;
			return;
		}
	}

	warning("cachePreload: Could not find a \'%s\' resource with ID %04x", tag2str(tag), id);
}

static const char *mystFiles[] = {
	"channel.dat",
	"credits.dat",
	"demo.dat",
	"dunny.dat",
	"intro.dat",
	"making.dat",
	"mechan.dat",
	"myst.dat",
	"selen.dat",
	"slides.dat",
	"sneak.dat",
	"stone.dat"
};

// Myst Hardcoded Movie Paths
// Mechanical Stack Movie "sstairs" referenced in executable, but not used?

// NOTE: cl1wg1.mov etc. found in the root directory in versions of Myst
// Original are duplicates of those in qtw/myst directory and thus not necessary.
// However, this *is* a problem for Myst ME Mac. Right now it will use the qtw/myst
// video, but this is most likely going to fail for the standalone Mac version.

// The following movies are not referenced in RLST or hardcoded into the executables.
// It is likely they are unused:
// qtw/mech/lwrgear2.mov + lwrgears.mov:	I have no idea what these are; perhaps replaced by an animated image in-game?
// qtw/myst/gar4wbf1.mov:	gar4wbf2.mov has two butterflies instead of one
// qtw/myst/libelev.mov:	libup.mov is basically the same with sound

Common::String MohawkEngine_Myst::wrapMovieFilename(const Common::String &movieName, uint16 stack) {
	Common::String prefix;

	switch (stack) {
	case kIntroStack:
		prefix = "intro/";
		break;
	case kChannelwoodStack:
		// The Windmill videos like to hide in a different folder
		if (movieName.contains("wmill"))
			prefix = "channel2/";
		else
			prefix = "channel/";
		break;
	case kDniStack:
		prefix = "dunny/";
		break;
	case kMechanicalStack:
		prefix = "mech/";
		break;
	case kMystStack:
		prefix = "myst/";
		break;
	case kSeleniticStack:
		prefix = "selen/";
		break;
	case kStoneshipStack:
		prefix = "stone/";
		break;
	default:
		// Masterpiece Edition Only Movies
		break;
	}

	return Common::String("qtw/") + prefix + movieName + ".mov";
}

Common::Error MohawkEngine_Myst::run() {
	MohawkEngine::run();

	_gfx = new MystGraphics(this);
	_console = new MystConsole(this);
	_gameState = new MystGameState(this, _saveFileMan);
	_loadDialog = new GUI::SaveLoadChooser(_("Load game:"), _("Load"), false);
	_optionsDialog = new MystOptionsDialog(this);
	_cursor = new MystCursorManager(this);
	_rnd = new Common::RandomSource("myst");

	// Cursor is visible by default
	_cursor->showCursor();

	// Load game from launcher/command line if requested
	if (ConfMan.hasKey("save_slot") && canLoadGameStateCurrently()) {
		uint32 gameToLoad = ConfMan.getInt("save_slot");
		Common::StringArray savedGamesList = _gameState->generateSaveGameList();
		if (gameToLoad > savedGamesList.size())
			error ("Could not find saved game");
		_gameState->load(savedGamesList[gameToLoad]);
	} else {
		// Start us on the first stack.
		if (getGameType() == GType_MAKINGOF)
			changeToStack(kMakingOfStack, 1, 0, 0);
		else if (getFeatures() & GF_DEMO)
			changeToStack(kDemoStack, 2000, 0, 0);
		else
			changeToStack(kIntroStack, 1, 0, 0);
	}

	// Load Help System (Masterpiece Edition Only)
	if (getFeatures() & GF_ME) {
		MohawkArchive *mhk = new MohawkArchive();
		if (!mhk->openFile("help.dat"))
			error("Could not load help.dat");
		_mhk.push_back(mhk);
	}

	// Test Load Function...
	loadHelp(10000);

	Common::Event event;
	while (!shouldQuit()) {
		// Update any background videos
		_needsUpdate = _video->updateMovies();
		_scriptParser->runPersistentScripts();

		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_MOUSEMOVE: {
				_needsUpdate = true;
				bool mouseClicked = _system->getEventManager()->getButtonState() & 1;

				// Keep the same resource when dragging
				if (!mouseClicked) {
					checkCurrentResource();
				}
				if (_curResource >= 0 && _resources[_curResource]->isEnabled() && mouseClicked) {
					debug(2, "Sending mouse move event to resource %d", _curResource);
					_resources[_curResource]->handleMouseDrag();
				}
				break;
			}
			case Common::EVENT_LBUTTONUP:
				if (_curResource >= 0 && _resources[_curResource]->isEnabled()) {
					debug(2, "Sending mouse up event to resource %d", _curResource);
					_resources[_curResource]->handleMouseUp();
				}
				checkCurrentResource();
				break;
			case Common::EVENT_LBUTTONDOWN:
				if (_curResource >= 0 && _resources[_curResource]->isEnabled()) {
					debug(2, "Sending mouse up event to resource %d", _curResource);
					_resources[_curResource]->handleMouseDown();
				}
				break;
			case Common::EVENT_KEYDOWN:
				switch (event.kbd.keycode) {
				case Common::KEYCODE_d:
					if (event.kbd.flags & Common::KBD_CTRL) {
						_console->attach();
						_console->onFrame();
					}
					break;
				case Common::KEYCODE_SPACE:
					pauseGame();
					break;
				case Common::KEYCODE_F4:
					_showResourceRects = !_showResourceRects;
					if (_showResourceRects)
						drawResourceRects();
					break;
				case Common::KEYCODE_F5:
					_needsPageDrop = false;
					_needsShowMap = false;
					_needsShowDemoMenu = false;

					runDialog(*_optionsDialog);

					if (_needsPageDrop) {
						dropPage();
						_needsPageDrop = false;
					}

					if (_needsShowMap) {
						_scriptParser->showMap();
						_needsShowMap = false;
					}

					if (_needsShowDemoMenu) {
						changeToStack(kDemoStack, 2002, 0, 0);
						_needsShowDemoMenu = false;
					}
					break;
				default:
					break;
				}
				break;
			default:
				break;
			}
		}

		if (_needsUpdate) {
			_system->updateScreen();
			_needsUpdate = false;
		}

		// Cut down on CPU usage
		_system->delayMillis(10);
	}

	return Common::kNoError;
}

bool MohawkEngine_Myst::skippableWait(uint32 duration) {
	uint32 end = _system->getMillis() + duration;
	bool skipped = false;

	while (_system->getMillis() < end && !skipped) {
		Common::Event event;
		while (_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_LBUTTONUP:
				skipped = true;
				break;
			case Common::EVENT_KEYDOWN:
				switch (event.kbd.keycode) {
				case Common::KEYCODE_SPACE:
					pauseGame();
					break;
				case Common::KEYCODE_ESCAPE:
					skipped = true;
					break;
				default:
					break;
			}
			default:
				break;
			}
		}

		// Cut down on CPU usage
		_system->delayMillis(10);
	}

	return skipped;
}

void MohawkEngine_Myst::changeToStack(uint16 stack, uint16 card, uint16 linkSrcSound, uint16 linkDstSound) {
	debug(2, "changeToStack(%d)", stack);

	_curStack = stack;

	// Fill screen with black and empty cursor
	_cursor->setCursor(0);
	_system->fillScreen(_system->getScreenFormat().RGBToColor(0, 0, 0));
	_system->updateScreen();

	_sound->stopSound();
	_sound->stopBackgroundMyst();
	if (linkSrcSound)
		_sound->playSoundBlocking(linkSrcSound);

	// Delete the previous stack and move the current stack to the previous one
	// There's probably a better way to do this, but the script classes shouldn't
	// take up much memory.
	delete _prevStack;
	_prevStack = _scriptParser;

	switch (_curStack) {
	case kChannelwoodStack:
		_gameState->_globals.currentAge = 4;
		_scriptParser = new MystStacks::Channelwood(this);
		break;
	case kCreditsStack:
		_scriptParser = new MystStacks::Credits(this);
		break;
	case kDemoStack:
		_gameState->_globals.currentAge = 0;
		_scriptParser = new MystStacks::Demo(this);
		break;
	case kDniStack:
		_gameState->_globals.currentAge = 6;
		_scriptParser = new MystStacks::Dni(this);
		break;
	case kIntroStack:
		_scriptParser = new MystStacks::Intro(this);
		break;
	case kMakingOfStack:
		_scriptParser = new MystStacks::MakingOf(this);
		break;
	case kMechanicalStack:
		_gameState->_globals.currentAge = 3;
		_scriptParser = new MystStacks::Mechanical(this);
		break;
	case kMystStack:
		_gameState->_globals.currentAge = 2;
		_scriptParser = new MystStacks::Myst(this);
		break;
	case kDemoPreviewStack:
		_scriptParser = new MystStacks::Preview(this);
		break;
	case kSeleniticStack:
		_gameState->_globals.currentAge = 0;
		_scriptParser = new MystStacks::Selenitic(this);
		break;
	case kDemoSlidesStack:
		_gameState->_globals.currentAge = 1;
		_scriptParser = new MystStacks::Slides(this);
		break;
	case kStoneshipStack:
		_gameState->_globals.currentAge = 1;
		_scriptParser = new MystStacks::Stoneship(this);
		break;
	default:
		error("Unknown Myst stack");
	}

	// If the array is empty, add a new one. Otherwise, delete the first
	// entry which is the stack file (the second, if there, is the help file).
	if (_mhk.empty())
		_mhk.push_back(new MohawkArchive());
	else {
		delete _mhk[0];
		_mhk[0] = new MohawkArchive();
	}

	if (!_mhk[0]->openFile(mystFiles[_curStack]))
		error("Could not open %s", mystFiles[_curStack]);

	_runExitScript = false;

	// Clear the resource cache and the image cache
	_cache.clear();
	_gfx->clearCache();

	// Play Flyby Entry Movie on Masterpiece Edition.
	const char *flyby = 0;
	if (getFeatures() & GF_ME) {
		switch (_curStack) {
		case kSeleniticStack:
			flyby = "selenitic flyby";
			break;
		case kStoneshipStack:
			flyby = "stoneship flyby";
			break;
		// Myst Flyby Movie not used in Original Masterpiece Edition Engine
		case kMystStack:
			if (_tweaksEnabled)
				flyby = "myst flyby";
			break;
		case kMechanicalStack:
			flyby = "mech age flyby";
			break;
		case kChannelwoodStack:
			flyby = "channelwood flyby";
			break;
		default:
			break;
		}

		if (flyby)
			_video->playMovieBlockingCentered(wrapMovieFilename(flyby, kMasterpieceOnly));
	}

	changeToCard(card, true);

	if (linkDstSound)
		_sound->playSoundBlocking(linkDstSound);
}

uint16 MohawkEngine_Myst::getCardBackgroundId() {
	uint16 imageToDraw = 0;

	if (_view.conditionalImageCount == 0)
		imageToDraw = _view.mainImage;
	else {
		for (uint16 i = 0; i < _view.conditionalImageCount; i++) {
			uint16 varValue = _scriptParser->getVar(_view.conditionalImages[i].var);
			if (varValue < _view.conditionalImages[i].numStates)
				imageToDraw = _view.conditionalImages[i].values[varValue];
		}
	}

	return imageToDraw;
}

void MohawkEngine_Myst::drawCardBackground() {
	_gfx->copyImageToBackBuffer(getCardBackgroundId(), Common::Rect(0, 0, 544, 332));
}

void MohawkEngine_Myst::changeToCard(uint16 card, bool updateScreen) {
	debug(2, "changeToCard(%d)", card);

	_scriptParser->disablePersistentScripts();

	_video->stopVideos();

	// Run exit script from last card (if present)
	if (_runExitScript)
		runExitScript();

	_runExitScript = true;

	unloadCard();

	// Clear the resource cache and image cache
	_cache.clear();
	_gfx->clearCache();

	_curCard = card;

	// Load a bunch of stuff
	loadCard();
	loadResources();
	loadCursorHints();

	// Handle images
	drawCardBackground();

	// Handle sound
	int16 soundAction = 0;
	uint16 soundActionVolume = 0;

	if (_view.sound == kMystSoundActionConditional) {
		uint16 soundVarValue = _scriptParser->getVar(_view.soundVar);
		if (soundVarValue >= _view.soundCount)
			warning("Conditional sound variable outside range");
		else {
			soundAction = _view.soundList[soundVarValue];
			soundActionVolume = _view.soundListVolume[soundVarValue];
		}
	} else {
		soundAction = _view.sound;
		soundActionVolume = _view.soundVolume;
	}

	if (soundAction == kMystSoundActionContinue)
		debug(2, "Continuing with current sound");
	else if (soundAction == kMystSoundActionChangeVolume) {
		debug(2, "Continuing with current sound, changing volume");
		_sound->changeBackgroundVolumeMyst(soundActionVolume);
	} else if (soundAction == kMystSoundActionStop) {
		debug(2, "Stopping sound");
		_sound->stopBackgroundMyst();
	} else if (soundAction > 0) {
		debug(2, "Playing new sound %d", soundAction);
		_sound->replaceBackgroundMyst(soundAction, soundActionVolume);
	} else {
		error("Unknown sound action %d", soundAction);
	}

	if (_view.flags & kMystZipDestination)
		_gameState->addZipDest(_curStack, card);

	// Run the entrance script (if present)
	runInitScript();

	// Update the images of each area too
	drawResourceImages();

	for (uint16 i = 0; i < _resources.size(); i++)
		_resources[i]->handleCardChange();

	// The demo resets the cursor at each card change except when in the library
	if (getFeatures() & GF_DEMO
			&& _gameState->_globals.currentAge != 2) {
		_cursor->setDefaultCursor();
	}

	// Make sure the screen is updated
	if (updateScreen) {
		_gfx->copyBackBufferToScreen(Common::Rect(544, 333));
		_system->updateScreen();
	}

	// Make sure we have the right cursor showing
	_dragResource = 0;
	_hoverResource = 0;
	_curResource = -1;
	checkCurrentResource();

	// Debug: Show resource rects
	if (_showResourceRects)
		drawResourceRects();
}

void MohawkEngine_Myst::drawResourceRects() {
	for (uint16 i = 0; i < _resources.size(); i++) {
		_resources[i]->getRect().debugPrint(0);
		_resources[i]->drawBoundingRect();
	}

	_system->updateScreen();
}

void MohawkEngine_Myst::checkCurrentResource() {
	// See what resource we're over
	bool foundResource = false;
	const Common::Point &mouse = _system->getEventManager()->getMousePos();

	// Tell previous resource the mouse is no longer hovering it
	if (_hoverResource && !_hoverResource->contains(mouse)) {
		_hoverResource->handleMouseLeave();
		_hoverResource = 0;
	}

	for (uint16 i = 0; i < _resources.size(); i++)
		if (_resources[i]->contains(mouse)) {
			if (_hoverResource != _resources[i] && _resources[i]->type == kMystHoverArea) {
				_hoverResource = static_cast<MystResourceType13 *>(_resources[i]);
				_hoverResource->handleMouseEnter();
			}

			if (!foundResource && _resources[i]->canBecomeActive()) {
				_curResource = i;
				foundResource = true;
			}
		}

	// Set the resource to none if we're not over any
	if (!foundResource)
		_curResource = -1;

	checkCursorHints();
}

MystResource *MohawkEngine_Myst::updateCurrentResource() {
	checkCurrentResource();

	if (_curResource >= 0)
		return _resources[_curResource];
	else
		return 0;
}

void MohawkEngine_Myst::loadCard() {
	debugC(kDebugView, "Loading Card View:");

	Common::SeekableReadStream *viewStream = getResource(ID_VIEW, _curCard);

	// Card Flags
	_view.flags = viewStream->readUint16LE();
	debugC(kDebugView, "Flags: 0x%04X", _view.flags);

	// The Image Block (Reminiscent of Riven PLST resources)
	_view.conditionalImageCount = viewStream->readUint16LE();
	debugC(kDebugView, "Conditional Image Count: %d", _view.conditionalImageCount);
	if (_view.conditionalImageCount != 0) {
		_view.conditionalImages = new MystCondition[_view.conditionalImageCount];
		for (uint16 i = 0; i < _view.conditionalImageCount; i++) {
			debugC(kDebugView, "\tImage %d:", i);
			_view.conditionalImages[i].var = viewStream->readUint16LE();
			debugC(kDebugView, "\t\tVar: %d", _view.conditionalImages[i].var);
			_view.conditionalImages[i].numStates = viewStream->readUint16LE();
			debugC(kDebugView, "\t\tNumber of States: %d", _view.conditionalImages[i].numStates);
			_view.conditionalImages[i].values = new uint16[_view.conditionalImages[i].numStates];
			for (uint16 j = 0; j < _view.conditionalImages[i].numStates; j++) {
				_view.conditionalImages[i].values[j] = viewStream->readUint16LE();
				debugC(kDebugView, "\t\tState %d -> Value %d", j, _view.conditionalImages[i].values[j]);
			}
		}
		_view.mainImage = 0;
	} else {
		_view.mainImage = viewStream->readUint16LE();
		debugC(kDebugView, "Main Image: %d", _view.mainImage);
	}

	// The Sound Block (Reminiscent of Riven SLST resources)
	_view.sound = viewStream->readSint16LE();
	debugCN(kDebugView, "Sound Control: %d = ", _view.sound);
	if (_view.sound > 0) {
		debugC(kDebugView, "Play new Sound, change volume");
		debugC(kDebugView, "\tSound: %d", _view.sound);
		_view.soundVolume = viewStream->readUint16LE();
		debugC(kDebugView, "\tVolume: %d", _view.soundVolume);
	} else if (_view.sound == kMystSoundActionContinue)
		debugC(kDebugView, "Continue current sound");
	else if (_view.sound == kMystSoundActionChangeVolume) {
		debugC(kDebugView, "Continue current sound, change volume");
		_view.soundVolume = viewStream->readUint16LE();
		debugC(kDebugView, "\tVolume: %d", _view.soundVolume);
	} else if (_view.sound == kMystSoundActionStop) {
		debugC(kDebugView, "Stop sound");
	} else if (_view.sound == kMystSoundActionConditional) {
		debugC(kDebugView, "Conditional sound list");
		_view.soundVar = viewStream->readUint16LE();
		debugC(kDebugView, "\tVar: %d", _view.soundVar);
		_view.soundCount = viewStream->readUint16LE();
		debugC(kDebugView, "\tCount: %d", _view.soundCount);
		_view.soundList = new int16[_view.soundCount];
		_view.soundListVolume = new uint16[_view.soundCount];

		for (uint16 i = 0; i < _view.soundCount; i++) {
			_view.soundList[i] = viewStream->readSint16LE();
			debugC(kDebugView, "\t\tCondition %d: Action %d", i, _view.soundList[i]);
			if (_view.soundList[i] == kMystSoundActionChangeVolume || _view.soundList[i] >= 0) {
				_view.soundListVolume[i] = viewStream->readUint16LE();
				debugC(kDebugView, "\t\tCondition %d: Volume %d", i, _view.soundListVolume[i]);
			}
		}
	} else {
		debugC(kDebugView, "Unknown");
		warning("Unknown sound control value in card");
	}

	// Resources that scripts can call upon
	_view.scriptResCount = viewStream->readUint16LE();
	debugC(kDebugView, "Script Resource Count: %d", _view.scriptResCount);
	if (_view.scriptResCount != 0) {
		_view.scriptResources = new MystView::ScriptResource[_view.scriptResCount];
		for (uint16 i = 0; i < _view.scriptResCount; i++) {
			debugC(kDebugView, "\tResource %d:", i);
			_view.scriptResources[i].type = viewStream->readUint16LE();
			debugC(kDebugView, "\t\t Type: %d", _view.scriptResources[i].type);

			switch (_view.scriptResources[i].type) {
			case 1:
				debugC(kDebugView, "\t\t\t\t= Image");
				break;
			case 2:
				debugC(kDebugView, "\t\t\t\t= Sound");
				break;
			case 3:
				debugC(kDebugView, "\t\t\t\t= Resource List");
				break;
			default:
				debugC(kDebugView, "\t\t\t\t= Unknown");
				break;
			}

			if (_view.scriptResources[i].type == 3) {
				_view.scriptResources[i].var = viewStream->readUint16LE();
				debugC(kDebugView, "\t\t Var: %d", _view.scriptResources[i].var);
				_view.scriptResources[i].count = viewStream->readUint16LE();
				debugC(kDebugView, "\t\t Resource List Count: %d", _view.scriptResources[i].count);
				_view.scriptResources[i].u0 = viewStream->readUint16LE();
				debugC(kDebugView, "\t\t u0: %d", _view.scriptResources[i].u0);
				_view.scriptResources[i].resource_list = new int16[_view.scriptResources[i].count];

				for (uint16 j = 0; j < _view.scriptResources[i].count; j++) {
					_view.scriptResources[i].resource_list[j] = viewStream->readSint16LE();
					debugC(kDebugView, "\t\t Resource List %d: %d", j, _view.scriptResources[i].resource_list[j]);
				}
			} else {
				_view.scriptResources[i].resource_list = NULL;
				_view.scriptResources[i].id = viewStream->readUint16LE();
				debugC(kDebugView, "\t\t Id: %d", _view.scriptResources[i].id);
			}
		}
	}

	// Identifiers for other resources. 0 if non existent. There is always an RLST.
	_view.rlst = viewStream->readUint16LE();
	if (!_view.rlst)
		error("RLST Index missing");

	_view.hint = viewStream->readUint16LE();
	_view.init = viewStream->readUint16LE();
	_view.exit = viewStream->readUint16LE();

	delete viewStream;

	// Precache Card Resources
	// TODO: Deal with Mac ME External Picture File
	uint32 cacheImageType;
	if (getFeatures() & GF_ME)
		cacheImageType = ID_PICT;
	else
		cacheImageType = ID_WDIB;

	// Precache Image Block data
	if (_view.conditionalImageCount != 0) {
		for (uint16 i = 0; i < _view.conditionalImageCount; i++)
			for (uint16 j = 0; j < _view.conditionalImages[i].numStates; j++)
				cachePreload(cacheImageType, _view.conditionalImages[i].values[j]);
	} else
		cachePreload(cacheImageType, _view.mainImage);

	// Precache Sound Block data
	if (_view.sound > 0)
		cachePreload(ID_MSND, _view.sound);
	else if (_view.sound == kMystSoundActionConditional) {
		for (uint16 i = 0; i < _view.soundCount; i++) {
			if (_view.soundList[i] > 0)
				cachePreload(ID_MSND, _view.soundList[i]);
		}
	}

	// Precache Script Resources
	if (_view.scriptResCount != 0) {
		for (uint16 i = 0; i < _view.scriptResCount; i++) {
			switch (_view.scriptResources[i].type) {
			case 1:
				cachePreload(cacheImageType, _view.scriptResources[i].id);
				break;
			case 2:
				cachePreload(ID_MSND, _view.scriptResources[i].id);
				break;
			case 3:
				warning("TODO: Precaching of Script Resource List not supported");
				break;
			default:
				warning("Unknown Resource in Script Resource List Precaching");
				break;
			}
		}
	}
}

void MohawkEngine_Myst::unloadCard() {
	for (uint16 i = 0; i < _view.conditionalImageCount; i++)
		delete[] _view.conditionalImages[i].values;

	delete[] _view.conditionalImages;
	_view.conditionalImageCount = 0;
	_view.conditionalImages = NULL;

	delete[] _view.soundList;
	_view.soundList = NULL;
	delete[] _view.soundListVolume;
	_view.soundListVolume = NULL;

	for (uint16 i = 0; i < _view.scriptResCount; i++)
		delete[] _view.scriptResources[i].resource_list;

	delete[] _view.scriptResources;
	_view.scriptResources = NULL;
	_view.scriptResCount = 0;
}

void MohawkEngine_Myst::runInitScript() {
	if (!_view.init) {
		debugC(kDebugINIT, "No INIT Present");
		return;
	}

	debugC(kDebugINIT, "Running INIT script");

	Common::SeekableReadStream *initStream = getResource(ID_INIT, _view.init);
	MystScript script = _scriptParser->readScript(initStream, kMystScriptInit);
	delete initStream;

	_scriptParser->runScript(script);
}

void MohawkEngine_Myst::runExitScript() {
	if (!_view.exit) {
		debugC(kDebugEXIT, "No EXIT Present");
		return;
	}

	debugC(kDebugEXIT, "Running EXIT script");

	Common::SeekableReadStream *exitStream = getResource(ID_EXIT, _view.exit);
	MystScript script = _scriptParser->readScript(exitStream, kMystScriptExit);
	delete exitStream;

	_scriptParser->runScript(script);
}

void MohawkEngine_Myst::loadHelp(uint16 id) {
	// The original version did not have the help system
	if (!(getFeatures() & GF_ME))
		return;

	// TODO: Help File contains 5 cards i.e. VIEW, RLST, etc.
	//       in addition to HELP resources.
	//       These are Ids 9930 to 9934
	//       Need to deal with loading and displaying these..
	//       Current engine structure only supports display of
	//       card from primary stack MHK

	debugC(kDebugHelp, "Loading Help System Data");

	Common::SeekableReadStream *helpStream = getResource(ID_HELP, id);

	uint16 count = helpStream->readUint16LE();
	uint16 *u0 = new uint16[count];
	Common::String helpText;

	debugC(kDebugHelp, "\tcount: %d", count);

	for (uint16 i = 0; i < count; i++) {
		u0[i] = helpStream->readUint16LE();
		debugC(kDebugHelp, "\tu0[%d]: %d", i, u0[i]);
	}

	// TODO: Previous values i.e. u0[0] to u0[count - 2]
	// appear to be resource ids in the help.dat file..
	if (u0[count - 1] != count)
		warning("loadHelp(): last u0 value is not equal to count");

	do {
		helpText += helpStream->readByte();
	} while (helpText.lastChar() != 0);
	helpText.deleteLastChar();

	debugC(kDebugHelp, "\thelpText: \"%s\"", helpText.c_str());

	delete[] u0;
}

void MohawkEngine_Myst::loadCursorHints() {
	for (uint16 i = 0; i < _cursorHintCount; i++)
		delete[] _cursorHints[i].variableHint.values;
	_cursorHintCount = 0;
	delete[] _cursorHints;
	_cursorHints = NULL;

	if (!_view.hint) {
		debugC(kDebugHint, "No HINT Present");
		return;
	}

	debugC(kDebugHint, "Loading Cursor Hints:");

	Common::SeekableReadStream *hintStream = getResource(ID_HINT, _curCard);
	_cursorHintCount = hintStream->readUint16LE();
	debugC(kDebugHint, "Cursor Hint Count: %d", _cursorHintCount);
	_cursorHints = new MystCursorHint[_cursorHintCount];

	for (uint16 i = 0; i < _cursorHintCount; i++) {
		debugC(kDebugHint, "Cursor Hint %d:", i);
		_cursorHints[i].id = hintStream->readUint16LE();
		debugC(kDebugHint, "\tId: %d", _cursorHints[i].id);
		_cursorHints[i].cursor = hintStream->readSint16LE();
		debugC(kDebugHint, "\tCursor: %d", _cursorHints[i].cursor);

		if (_cursorHints[i].cursor == -1) {
			debugC(kDebugHint, "\tConditional Cursor Hints:");
			_cursorHints[i].variableHint.var = hintStream->readUint16LE();
			debugC(kDebugHint, "\tVar: %d", _cursorHints[i].variableHint.var);
			_cursorHints[i].variableHint.numStates = hintStream->readUint16LE();
			debugC(kDebugHint, "\tNumber of States: %d", _cursorHints[i].variableHint.numStates);
			_cursorHints[i].variableHint.values = new uint16[_cursorHints[i].variableHint.numStates];
			for (uint16 j = 0; j < _cursorHints[i].variableHint.numStates; j++) {
				_cursorHints[i].variableHint.values[j] = hintStream->readUint16LE();
				debugC(kDebugHint, "\t\t State %d: Cursor %d", j, _cursorHints[i].variableHint.values[j]);
			}
		} else {
			_cursorHints[i].variableHint.var = 0;
			_cursorHints[i].variableHint.numStates = 0;
			_cursorHints[i].variableHint.values = NULL;
		}
	}

	delete hintStream;
}

void MohawkEngine_Myst::setMainCursor(uint16 cursor) {
	_currentCursor = _mainCursor = cursor;
	_cursor->setCursor(_currentCursor);
}

void MohawkEngine_Myst::checkCursorHints() {
	if (!_view.hint) {
		// Default to the main cursor when no hints are present
		if (_currentCursor != _mainCursor) {
			_currentCursor = _mainCursor;
			_cursor->setCursor(_currentCursor);
		}
		return;
	}

	// Check all the cursor hints to see if we're in a hotspot that contains a hint.
	for (uint16 i = 0; i < _cursorHintCount; i++)
		if (_cursorHints[i].id == _curResource && _resources[_cursorHints[i].id]->isEnabled()) {
			if (_cursorHints[i].cursor == -1) {
				uint16 var_value = _scriptParser->getVar(_cursorHints[i].variableHint.var);

				if (var_value >= _cursorHints[i].variableHint.numStates)
					warning("Variable %d Out of Range in variable HINT Resource %d", _cursorHints[i].variableHint.var, i);
				else {
					_currentCursor = _cursorHints[i].variableHint.values[var_value];
					if (_currentCursor == 0)
						_currentCursor = _mainCursor;
					_cursor->setCursor(_currentCursor);
				}
			} else if (_currentCursor != _cursorHints[i].cursor) {
				if (_cursorHints[i].cursor == 0)
					_currentCursor = _mainCursor;
				else
					_currentCursor = _cursorHints[i].cursor;

				_cursor->setCursor(_currentCursor);
			}
			return;
		}

	if (_currentCursor != _mainCursor) {
		_currentCursor = _mainCursor;
		_cursor->setCursor(_currentCursor);
	}
}

void MohawkEngine_Myst::setResourceEnabled(uint16 resourceId, bool enable) {
	if (resourceId < _resources.size()) {
		_resources[resourceId]->setEnabled(enable);
	} else
		warning("Attempt to change unknown resource enable state");
}

void MohawkEngine_Myst::drawResourceImages() {
	for (uint16 i = 0; i < _resources.size(); i++)
		if (_resources[i]->isDrawSubimages())
			_resources[i]->drawDataToScreen();
}

void MohawkEngine_Myst::redrawResource(MystResourceType8 *resource, bool update) {
	resource->drawConditionalDataToScreen(_scriptParser->getVar(resource->getType8Var()), update);
}

void MohawkEngine_Myst::redrawArea(uint16 var, bool update) {
	for (uint16 i = 0; i < _resources.size(); i++)
		if (_resources[i]->type == kMystConditionalImage && _resources[i]->getType8Var() == var)
			redrawResource(static_cast<MystResourceType8 *>(_resources[i]), update);
}

MystResource *MohawkEngine_Myst::loadResource(Common::SeekableReadStream *rlstStream, MystResource *parent) {
	MystResource *resource = 0;
	ResourceType type = static_cast<ResourceType>(rlstStream->readUint16LE());

	debugC(kDebugResource, "\tType: %d", type);
	debugC(kDebugResource, "\tSub_Record: %d", (parent == NULL) ? 0 : 1);

	switch (type) {
	case kMystAction:
		resource =  new MystResourceType5(this, rlstStream, parent);
		break;
	case kMystVideo:
		resource =  new MystResourceType6(this, rlstStream, parent);
		break;
	case kMystSwitch:
		resource =  new MystResourceType7(this, rlstStream, parent);
		break;
	case kMystConditionalImage:
		resource =  new MystResourceType8(this, rlstStream, parent);
		break;
	case kMystSlider:
		resource =  new MystResourceType10(this, rlstStream, parent);
		break;
	case kMystDragArea:
		resource =  new MystResourceType11(this, rlstStream, parent);
		break;
	case kMystVideoInfo:
		resource =  new MystResourceType12(this, rlstStream, parent);
		break;
	case kMystHoverArea:
		resource =  new MystResourceType13(this, rlstStream, parent);
		break;
	default:
		resource = new MystResource(this, rlstStream, parent);
		break;
	}

	resource->type = type;

	return resource;
}

void MohawkEngine_Myst::loadResources() {
	for (uint32 i = 0; i < _resources.size(); i++)
		delete _resources[i];

	_resources.clear();

	if (!_view.rlst) {
		debugC(kDebugResource, "No RLST present");
		return;
	}

	Common::SeekableReadStream *rlstStream = getResource(ID_RLST, _view.rlst);
	uint16 resourceCount = rlstStream->readUint16LE();
	debugC(kDebugResource, "RLST Resource Count: %d", resourceCount);

	for (uint16 i = 0; i < resourceCount; i++) {
		debugC(kDebugResource, "Resource #%d:", i);
		_resources.push_back(loadResource(rlstStream, NULL));
	}

	delete rlstStream;
}

Common::Error MohawkEngine_Myst::loadGameState(int slot) {
	if (_gameState->load(_gameState->generateSaveGameList()[slot]))
		return Common::kNoError;

	return Common::kUnknownError;
}

Common::Error MohawkEngine_Myst::saveGameState(int slot, const Common::String &desc) {
	Common::StringArray saveList = _gameState->generateSaveGameList();

	if ((uint)slot < saveList.size())
		_gameState->deleteSave(saveList[slot]);

	return _gameState->save(Common::String(desc)) ? Common::kNoError : Common::kUnknownError;
}

bool MohawkEngine_Myst::canLoadGameStateCurrently() {
	// No loading in the demo/makingof
	return !(getFeatures() & GF_DEMO) && getGameType() != GType_MAKINGOF;
}

bool MohawkEngine_Myst::canSaveGameStateCurrently() {
	// There's a limited number of stacks the game can save in
	switch (_curStack) {
	case kChannelwoodStack:
	case kDniStack:
	case kMechanicalStack:
	case kMystStack:
	case kSeleniticStack:
	case kStoneshipStack:
		return true;
	}

	return false;
}

void MohawkEngine_Myst::dropPage() {
    uint16 page = _gameState->_globals.heldPage;
	bool whitePage = page == 13;
	bool bluePage = page - 1 < 6;
    bool redPage = page - 7 < 6;

    // Play drop page sound
    _sound->replaceSoundMyst(800);

    // Drop page
    _gameState->_globals.heldPage = 0;

    // Redraw page area
    if (whitePage && _gameState->_globals.currentAge == 2) {
    	redrawArea(41);
    } else if (bluePage) {
    	if (page == 6) {
    		if (_gameState->_globals.currentAge == 2)
    			redrawArea(24);
    	} else {
    		redrawArea(103);
    	}
    } else if (redPage) {
    	if (page == 12) {
    		if (_gameState->_globals.currentAge == 2)
    			redrawArea(25);
    	} else if (page == 10) {
    		if (_gameState->_globals.currentAge == 1)
    			redrawArea(35);
    	} else {
    		redrawArea(102);
    	}
    }

    setMainCursor(kDefaultMystCursor);
    checkCursorHints();
}

} // End of namespace Mohawk

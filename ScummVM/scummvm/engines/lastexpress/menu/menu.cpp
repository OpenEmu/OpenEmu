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

#include "lastexpress/menu/menu.h"

// Data
#include "lastexpress/data/animation.h"
#include "lastexpress/data/cursor.h"
#include "lastexpress/data/snd.h"
#include "lastexpress/data/scene.h"

#include "lastexpress/fight/fight.h"

#include "lastexpress/game/entities.h"
#include "lastexpress/game/inventory.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/savegame.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/scenes.h"
#include "lastexpress/game/state.h"

#include "lastexpress/menu/clock.h"
#include "lastexpress/menu/trainline.h"

#include "lastexpress/sound/queue.h"

#include "lastexpress/graphics.h"
#include "lastexpress/lastexpress.h"
#include "lastexpress/resource.h"

#include "common/rational.h"

#define getNextGameId() (GameId)((_gameId + 1) % 6)

namespace LastExpress {

// Bottom-left buttons (quit.seq)
enum StartMenuButtons {
	kButtonVolumeDownPushed,
	kButtonVolumeDown,
	kButtonVolume,
	kButtonVolumeUp,
	kButtonVolumeUpPushed,
	kButtonBrightnessDownPushed,    // 5
	kButtonBrightnessDown,
	kButtonBrightness,
	kButtonBrightnessUp,
	kButtonBrightnessUpPushed,
	kButtonQuit,                    // 10
	kButtonQuitPushed
};

// Egg buttons (buttns.seq)
enum StartMenuEggButtons {
	kButtonShield,
	kButtonRewind,
	kButtonRewindPushed,
	kButtonForward,
	kButtonForwardPushed,
	kButtonCredits,                // 5
	kButtonCreditsPushed,
	kButtonContinue
};

// Tooltips sequence (helpnewr.seq)
enum StartMenuTooltips {
	kTooltipInsertCd1,
	kTooltipInsertCd2,
	kTooltipInsertCd3,
	kTooltipContinueGame,
	kTooltipReplayGame,
	kTooltipContinueRewoundGame,    // 5
	kTooltipViewGameEnding,
	kTooltipStartAnotherGame,
	kTooltipVolumeUp,
	kTooltipVolumeDown,
	kTooltipBrightnessUp,           // 10
	kTooltipBrightnessDown,
	kTooltipQuit,
	kTooltipRewindParis,
	kTooltipForwardStrasbourg,
	kTooltipRewindStrasbourg,      // 15
	kTooltipRewindMunich,
	kTooltipForwardMunich,
	kTooltipForwardVienna,
	kTooltipRewindVienna,
	kTooltipRewindBudapest,        // 20
	kTooltipForwardBudapest,
	kTooltipForwardBelgrade,
	kTooltipRewindBelgrade,
	kTooltipForwardConstantinople,
	kTooltipSwitchBlueGame,        // 25
	kTooltipSwitchRedGame,
	kTooltipSwitchGoldGame,
	kTooltipSwitchGreenGame,
	kTooltipSwitchTealGame,
	kTooltipSwitchPurpleGame,      // 30
	kTooltipPlayNewGame,
	kTooltipCredits,
	kTooltipFastForward,
	kTooltipRewind
};

//////////////////////////////////////////////////////////////////////////
// DATA
//////////////////////////////////////////////////////////////////////////
static const struct {
	TimeValue time;
	uint index;
	StartMenuTooltips rewind;
	StartMenuTooltips forward;
} _cityButtonsInfo[7] = {
	{kTimeCityParis, 64, kTooltipRewindParis, kTooltipRewindParis},
	{kTimeCityStrasbourg, 128, kTooltipRewindStrasbourg, kTooltipForwardStrasbourg},
	{kTimeCityMunich, 129, kTooltipRewindMunich, kTooltipForwardMunich},
	{kTimeCityVienna, 130, kTooltipRewindVienna, kTooltipForwardVienna},
	{kTimeCityBudapest, 131, kTooltipRewindBudapest, kTooltipForwardBudapest},
	{kTimeCityBelgrade, 132, kTooltipRewindBelgrade, kTooltipForwardBelgrade},
	{kTimeCityConstantinople, 192, kTooltipForwardConstantinople, kTooltipForwardConstantinople}
};

//////////////////////////////////////////////////////////////////////////
// Menu
//////////////////////////////////////////////////////////////////////////
Menu::Menu(LastExpressEngine *engine) : _engine(engine),
	_seqTooltips(NULL), _seqEggButtons(NULL), _seqButtons(NULL), _seqAcorn(NULL), _seqCity1(NULL), _seqCity2(NULL), _seqCity3(NULL), _seqCredits(NULL),
	_gameId(kGameBlue), _hasShownStartScreen(false), _hasShownIntro(false),
	_isShowingCredits(false), _isGameStarted(false), _isShowingMenu(false),
	_creditsSequenceIndex(0), _checkHotspotsTicks(15),  _mouseFlags(Common::EVENT_INVALID), _lastHotspot(NULL),
	_currentTime(kTimeNone), _lowerTime(kTimeNone), _time(kTimeNone), _currentIndex(0), _index(0), _lastIndex(0), _delta(0), _handleTimeDelta(false) {

	_clock = new Clock(_engine);
	_trainLine = new TrainLine(_engine);
}

Menu::~Menu() {
	SAFE_DELETE(_clock);
	SAFE_DELETE(_trainLine);

	SAFE_DELETE(_seqTooltips);
	SAFE_DELETE(_seqEggButtons);
	SAFE_DELETE(_seqButtons);
	SAFE_DELETE(_seqAcorn);
	SAFE_DELETE(_seqCity1);
	SAFE_DELETE(_seqCity2);
	SAFE_DELETE(_seqCity3);
	SAFE_DELETE(_seqCredits);

	_lastHotspot = NULL;

	// Cleanup frames
	for (MenuFrames::iterator it = _frames.begin(); it != _frames.end(); it++)
		SAFE_DELETE(it->_value);

	_frames.clear();

	// Zero passed pointers
	_engine = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Setup
void Menu::setup() {

	// Clear drawing queue
	getScenes()->removeAndRedraw(&_frames[kOverlayAcorn], false);
	SAFE_DELETE(_seqAcorn);

	// Load Menu scene
	// + 1 = normal menu with open egg / clock
	// + 2 = shield menu, when no savegame exists (no game has been started)
	_isGameStarted = _lowerTime >= kTimeStartGame;
	getScenes()->loadScene((SceneIndex)(_isGameStarted ? _gameId * 5 + 1 : _gameId * 5 + 2));
	getFlags()->shouldRedraw = true;
	getLogic()->updateCursor();

	//////////////////////////////////////////////////////////////////////////
	// Load Acorn sequence
	_seqAcorn = loadSequence(getAcornSequenceName(_isGameStarted ? getNextGameId() : kGameBlue));

	//////////////////////////////////////////////////////////////////////////
	// Check if we loaded sequences before
	if (_seqTooltips && _seqTooltips->count() > 0)
		return;

	// Load all static data
	_seqTooltips = loadSequence("helpnewr.seq");
	_seqEggButtons = loadSequence("buttns.seq");
	_seqButtons = loadSequence("quit.seq");
	_seqCity1 = loadSequence("jlinetl.seq");
	_seqCity2 = loadSequence("jlinecen.seq");
	_seqCity3 = loadSequence("jlinebr.seq");
	_seqCredits = loadSequence("credits.seq");

	_frames[kOverlayTooltip] = new SequenceFrame(_seqTooltips);
	_frames[kOverlayEggButtons] = new SequenceFrame(_seqEggButtons);
	_frames[kOverlayButtons] = new SequenceFrame(_seqButtons);
	_frames[kOverlayAcorn] = new SequenceFrame(_seqAcorn);
	_frames[kOverlayCity1] = new SequenceFrame(_seqCity1);
	_frames[kOverlayCity2] = new SequenceFrame(_seqCity2);
	_frames[kOverlayCity3] = new SequenceFrame(_seqCity3);
	_frames[kOverlayCredits] = new SequenceFrame(_seqCredits);
}

//////////////////////////////////////////////////////////////////////////
// Handle events
void Menu::eventMouse(const Common::Event &ev) {
	if (!getFlags()->shouldRedraw)
		return;

	bool redraw = true;
	getFlags()->shouldRedraw = false;

	// Update coordinates
	setCoords(ev.mouse);
	//_mouseFlags = (Common::EventType)(ev.type & Common::EVENT_LBUTTONUP);

	if (_isShowingCredits) {
		if (ev.type == Common::EVENT_RBUTTONUP) {
			showFrame(kOverlayCredits, -1, true);
			_isShowingCredits = false;
		}

		if (ev.type == Common::EVENT_LBUTTONUP) {
			// Last frame of the credits
			if (_seqCredits && _creditsSequenceIndex == _seqCredits->count() - 1) {
				showFrame(kOverlayCredits, -1, true);
				_isShowingCredits = false;
			} else {
				++_creditsSequenceIndex;
				showFrame(kOverlayCredits, _creditsSequenceIndex, true);
			}
		}
	} else {
		// Check for hotspots
		SceneHotspot *hotspot = NULL;
		getScenes()->get(getState()->scene)->checkHotSpot(ev.mouse, &hotspot);

		if (_lastHotspot != hotspot || ev.type == Common::EVENT_LBUTTONUP) {
			_lastHotspot = hotspot;

			if (ev.type == Common::EVENT_MOUSEMOVE) { /* todo check event type */
				if (!_handleTimeDelta && hasTimeDelta())
					setTime();
			}

			if (hotspot) {
				redraw = handleEvent((StartMenuAction)hotspot->action, ev.type);
				getFlags()->mouseRightClick = false;
				getFlags()->mouseLeftClick = false;
			} else {
				hideOverlays();
			}
		}
	}

	if (redraw) {
		getFlags()->shouldRedraw = true;
		askForRedraw();
	}
}

void Menu::eventTick(const Common::Event&) {
	if (hasTimeDelta())
		adjustTime();
	else if (_handleTimeDelta)
		_handleTimeDelta = false;

	// Check hotspots
	if (!--_checkHotspotsTicks) {
		checkHotspots();
		_checkHotspotsTicks = 15;
	}
}

//////////////////////////////////////////////////////////////////////////
// Show the intro and load the main menu scene
void Menu::show(bool doSavegame, SavegameType type, uint32 value) {

	if (_isShowingMenu)
		return;

	_isShowingMenu = true;
	getEntities()->reset();

	// If no blue savegame exists, this might be the first time we start the game, so we show the full intro
	if (!getFlags()->mouseRightClick) {
		if (!SaveLoad::isSavegameValid(kGameBlue) && _engine->getResourceManager()->loadArchive(kArchiveCd1)) {

			if (!_hasShownIntro) {
				// Show Broderbrund logo
				Animation animation;
				if (animation.load(getArchive("1930.nis")))
					animation.play();

				getFlags()->mouseRightClick = false;

				// Play intro music
				getSound()->playSoundWithSubtitles("MUS001.SND", kFlagMusic, kEntityPlayer);

				// Show The Smoking Car logo
				if (animation.load(getArchive("1931.nis")))
					animation.play();

				_hasShownIntro = true;
			}
		} else {
			// Only show the quick intro
			if (!_hasShownStartScreen) {
				getSound()->playSoundWithSubtitles("MUS018.SND", kFlagMusic, kEntityPlayer);
				getScenes()->loadScene(kSceneStartScreen);

				// Original game waits 60 frames and loops Sound::unknownFunction1 unless the right button is pressed
				uint32 nextFrameCount = getFrameCount() + 60;
				while (getFrameCount() < nextFrameCount) {
					_engine->pollEvents();

					if (getFlags()->mouseRightClick)
						break;

					getSoundQueue()->updateQueue();
				}
			}
		}
	}

	_hasShownStartScreen = true;

	// Init Menu
	init(doSavegame, type, value);

	// Setup sound
	getSoundQueue()->resetQueue();
	getSoundQueue()->resetQueue(kSoundType11, kSoundType13);
	if (getSoundQueue()->isBuffered("TIMER"))
		getSoundQueue()->removeFromQueue("TIMER");

	// Init flags & misc
	_isShowingCredits = false;
	_handleTimeDelta = hasTimeDelta();
	getInventory()->unselectItem();

	// Set Cursor type
	_engine->getCursor()->setStyle(kCursorNormal);
	_engine->getCursor()->show(true);

	setup();
	checkHotspots();

	// Set event handlers
	SET_EVENT_HANDLERS(Menu, this);
}

bool Menu::handleEvent(StartMenuAction action, Common::EventType type) {
	bool clicked = (type == Common::EVENT_LBUTTONUP);

	switch(action) {
	default:
		hideOverlays();
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuCredits:
		if (hasTimeDelta()) {
			hideOverlays();
			break;
		}

		if (clicked) {
			showFrame(kOverlayEggButtons, kButtonCreditsPushed, true);
			showFrame(kOverlayTooltip, -1, true);

			getSound()->playSound(kEntityPlayer, "LIB046");

			hideOverlays();

			_isShowingCredits = true;
			_creditsSequenceIndex = 0;

			showFrame(kOverlayCredits, 0, true);
		} else {
			// TODO check flags ?

			showFrame(kOverlayEggButtons, kButtonCredits, true);
			showFrame(kOverlayTooltip, kTooltipCredits, true);
		}
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuQuitGame:
		showFrame(kOverlayTooltip, kTooltipQuit, true);

		if (clicked) {
			showFrame(kOverlayButtons, kButtonQuitPushed, true);

			getSoundQueue()->clearStatus();
			getSoundQueue()->updateQueue();
			getSound()->playSound(kEntityPlayer, "LIB046");

			// FIXME uncomment when sound queue is properly implemented
			/*while (getSoundQueue()->isBuffered("LIB046"))
				getSoundQueue()->updateQueue();*/

			getFlags()->shouldRedraw = false;

			Engine::quitGame();

			return false;
		} else {
			showFrame(kOverlayButtons, kButtonQuit, true);
		}
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuCase4:
		if (clicked)
			_index = 0;
		// fall down to kMenuContinue

	//////////////////////////////////////////////////////////////////////////
	case kMenuContinue: {
		if (hasTimeDelta()) {
			hideOverlays();
			break;
		}

		// Determine the proper CD archive
		ArchiveIndex cd = kArchiveCd1;
		if (getProgress().chapter > kChapter1)
			cd = (getProgress().chapter > kChapter3) ? kArchiveCd3 : kArchiveCd2;

		// Show tooltips & buttons to start a game, continue a game or load the proper cd
		if (ResourceManager::isArchivePresent(cd)) {
			if (_isGameStarted) {
				showFrame(kOverlayEggButtons, kButtonContinue, true);

				if (_lastIndex == _index) {
					showFrame(kOverlayTooltip, getSaveLoad()->isGameFinished(_index, _lastIndex) ? kTooltipViewGameEnding : kTooltipContinueGame, true);
				} else {
					showFrame(kOverlayTooltip, kTooltipContinueRewoundGame, true);
				}

			} else {
				showFrame(kOverlayEggButtons, kButtonShield, true);
				showFrame(kOverlayTooltip, kTooltipPlayNewGame, true);
			}
		} else {
			showFrame(kOverlayEggButtons, -1, true);
			showFrame(kOverlayTooltip, cd - 1, true);
		}

		if (!clicked)
			break;

		// Try loading the archive file
		if (!_engine->getResourceManager()->loadArchive(cd))
			break;

		// Load the train data file and setup game
		getScenes()->loadSceneDataFile(cd);
		showFrame(kOverlayTooltip, -1, true);
		getSound()->playSound(kEntityPlayer, "LIB046");

		// Setup new game
		getSavePoints()->reset();
		setLogicEventHandlers();

		if (_index) {
			getSoundQueue()->processEntry(kSoundType11);
		} else {
			if (!getFlags()->mouseRightClick) {
				getScenes()->loadScene((SceneIndex)(5 * _gameId + 3));

				if (!getFlags()->mouseRightClick) {
					getScenes()->loadScene((SceneIndex)(5 * _gameId + 4));

					if (!getFlags()->mouseRightClick) {
						getScenes()->loadScene((SceneIndex)(5 * _gameId + 5));

						if (!getFlags()->mouseRightClick) {
							getSoundQueue()->processEntry(kSoundType11);

							// Show intro
							Animation animation;
							if (animation.load(getArchive("1601.nis")))
								animation.play();

							getEvent(kEventIntro) = 1;
						}
					}
				}
			}

			if (!getEvent(kEventIntro))	{
				getEvent(kEventIntro) = 1;

				getSoundQueue()->processEntry(kSoundType11);
			}
		}

		// Setup game
		getFlags()->isGameRunning = true;
		startGame();

		if (!_isShowingMenu)
			getInventory()->show();

		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	case kMenuSwitchSaveGame:
		if (hasTimeDelta()) {
			hideOverlays();
			break;
		}

		if (clicked) {
			showFrame(kOverlayAcorn, 1, true);
			showFrame(kOverlayTooltip, -1, true);
			getSound()->playSound(kEntityPlayer, "LIB047");

			// Setup new menu screen
			switchGame();
			setup();

			// Set fight state to 0
			getFight()->resetState();

			return true;
		}

		// TODO Check for flag

		showFrame(kOverlayAcorn, 0, true);

		if (_isGameStarted) {
			showFrame(kOverlayTooltip, kTooltipSwitchBlueGame, true);
			break;
		}

		if (_gameId == kGameGold) {
			showFrame(kOverlayTooltip, kTooltipSwitchBlueGame, true);
			break;
		}

		if (!SaveLoad::isSavegameValid(getNextGameId())) {
			showFrame(kOverlayTooltip, kTooltipStartAnotherGame, true);
			break;
		}

		// Stupid tooltips ids are not in order, so we can't just increment them...
		switch(_gameId) {
		default:
			break;

		case kGameBlue:
			showFrame(kOverlayTooltip, kTooltipSwitchRedGame, true);
			break;

		case kGameRed:
			showFrame(kOverlayTooltip, kTooltipSwitchGreenGame, true);
			break;

		case kGameGreen:
			showFrame(kOverlayTooltip, kTooltipSwitchPurpleGame, true);
			break;

		case kGamePurple:
			showFrame(kOverlayTooltip, kTooltipSwitchTealGame, true);
			break;

		case kGameTeal:
			showFrame(kOverlayTooltip, kTooltipSwitchGoldGame, true);
			break;
		}
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuRewindGame:
		if (!_index || _currentTime < _time) {
			hideOverlays();
			break;
		}

		if (clicked) {
			if (hasTimeDelta())
				_handleTimeDelta = false;

			showFrame(kOverlayEggButtons, kButtonRewindPushed, true);
			showFrame(kOverlayTooltip, -1, true);

			getSound()->playSound(kEntityPlayer, "LIB046");

			rewindTime();

			_handleTimeDelta = false;
		} else {
			showFrame(kOverlayEggButtons, kButtonRewind, true);
			showFrame(kOverlayTooltip, kTooltipRewind, true);
		}
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuForwardGame:
		if (_lastIndex <= _index || _currentTime > _time) {
			hideOverlays();
			break;
		}

		if (clicked) {
			if (hasTimeDelta())
				_handleTimeDelta = false;

			showFrame(kOverlayEggButtons, kButtonForwardPushed, true);
			showFrame(kOverlayTooltip, -1, true);

			getSound()->playSound(kEntityPlayer, "LIB046");

			forwardTime();

			_handleTimeDelta = false;
		} else {
			showFrame(kOverlayEggButtons, kButtonForward, true);
			showFrame(kOverlayTooltip, kTooltipFastForward, true);
		}
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuParis:
		moveToCity(kParis, clicked);
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuStrasBourg:
		moveToCity(kStrasbourg, clicked);
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuMunich:
		moveToCity(kMunich, clicked);
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuVienna:
		moveToCity(kVienna, clicked);
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuBudapest:
		moveToCity(kBudapest, clicked);
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuBelgrade:
		moveToCity(kBelgrade, clicked);
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuConstantinople:
		moveToCity(kConstantinople, clicked);
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuDecreaseVolume:
		if (hasTimeDelta()) {
			hideOverlays();
			break;
		}

		// Cannot decrease volume further
		if (getVolume() == 0) {
			showFrame(kOverlayButtons, kButtonVolume, true);
			showFrame(kOverlayTooltip, -1, true);
			break;
		}

		showFrame(kOverlayTooltip, kTooltipVolumeDown, true);

		// Show highlight on button & adjust volume if needed
		if (clicked) {
			showFrame(kOverlayButtons, kButtonVolumeDownPushed, true);
			getSound()->playSound(kEntityPlayer, "LIB046");
			setVolume(getVolume() - 1);

			getSaveLoad()->saveVolumeBrightness();

			uint32 nextFrameCount = getFrameCount() + 15;
			while (nextFrameCount > getFrameCount()) {
				_engine->pollEvents();

				getSoundQueue()->updateQueue();
			}
		} else {
			showFrame(kOverlayButtons, kButtonVolumeDown, true);
		}
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuIncreaseVolume:
		if (hasTimeDelta()) {
			hideOverlays();
			break;
		}

		// Cannot increase volume further
		if (getVolume() >= 7) {
			showFrame(kOverlayButtons, kButtonVolume, true);
			showFrame(kOverlayTooltip, -1, true);
			break;
		}

		showFrame(kOverlayTooltip, kTooltipVolumeUp, true);

		// Show highlight on button & adjust volume if needed
		if (clicked) {
			showFrame(kOverlayButtons, kButtonVolumeUpPushed, true);
			getSound()->playSound(kEntityPlayer, "LIB046");
			setVolume(getVolume() + 1);

			getSaveLoad()->saveVolumeBrightness();

			uint32 nextFrameCount = getFrameCount() + 15;
			while (nextFrameCount > getFrameCount()) {
				_engine->pollEvents();

				getSoundQueue()->updateQueue();
			}
		} else {
			showFrame(kOverlayButtons, kButtonVolumeUp, true);
		}
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuDecreaseBrightness:
		if (hasTimeDelta()) {
			hideOverlays();
			break;
		}

		// Cannot increase brightness further
		if (getBrightness() == 0) {
			showFrame(kOverlayButtons, kButtonBrightness, true);
			showFrame(kOverlayTooltip, -1, true);
			break;
		}

		showFrame(kOverlayTooltip, kTooltipBrightnessDown, true);

		// Show highlight on button & adjust brightness if needed
		if (clicked) {
			showFrame(kOverlayButtons, kButtonBrightnessDownPushed, true);
			getSound()->playSound(kEntityPlayer, "LIB046");
			setBrightness(getBrightness() - 1);

			getSaveLoad()->saveVolumeBrightness();

			// Reshow the background and frames (they will pick up the new brightness through the GraphicsManager)
			_engine->getGraphicsManager()->draw(getScenes()->get((SceneIndex)(_isGameStarted ? _gameId * 5 + 1 : _gameId * 5 + 2)), GraphicsManager::kBackgroundC, true);
			showFrame(kOverlayTooltip, kTooltipBrightnessDown, false);
			showFrame(kOverlayButtons, kButtonBrightnessDownPushed, false);
		} else {
			showFrame(kOverlayButtons, kButtonBrightnessDown, true);
		}
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuIncreaseBrightness:
		if (hasTimeDelta()) {
			hideOverlays();
			break;
		}

		// Cannot increase brightness further
		if (getBrightness() >= 6) {
			showFrame(kOverlayButtons, kButtonBrightness, true);
			showFrame(kOverlayTooltip, -1, true);
			break;
		}

		showFrame(kOverlayTooltip, kTooltipBrightnessUp, true);

		// Show highlight on button & adjust brightness if needed
		if (clicked) {
			showFrame(kOverlayButtons, kButtonBrightnessUpPushed, true);
			getSound()->playSound(kEntityPlayer, "LIB046");
			setBrightness(getBrightness() + 1);

			getSaveLoad()->saveVolumeBrightness();

			// Reshow the background and frames (they will pick up the new brightness through the GraphicsManager)
			_engine->getGraphicsManager()->draw(getScenes()->get((SceneIndex)(_isGameStarted ? _gameId * 5 + 1 : _gameId * 5 + 2)), GraphicsManager::kBackgroundC, true);
			showFrame(kOverlayTooltip, kTooltipBrightnessUp, false);
			showFrame(kOverlayButtons, kButtonBrightnessUpPushed, false);
		} else {
			showFrame(kOverlayButtons, kButtonBrightnessUp, true);
		}
		break;
	}

	return true;
}

void Menu::setLogicEventHandlers() {
	SET_EVENT_HANDLERS(Logic, getLogic());
	clear();
	_isShowingMenu = false;
}

//////////////////////////////////////////////////////////////////////////
// Game-related
//////////////////////////////////////////////////////////////////////////
void Menu::init(bool doSavegame, SavegameType type, uint32 value) {

	bool useSameIndex = true;

	if (getGlobalTimer()) {
		value = 0;

		// Check if the CD file is present
		ArchiveIndex index = kArchiveCd1;
		switch (getProgress().chapter) {
		default:
		case kChapter1:
			break;

		case kChapter2:
		case kChapter3:
			index = kArchiveCd2;
			break;

		case kChapter4:
		case kChapter5:
			index = kArchiveCd3;
			break;
		}

		if (ResourceManager::isArchivePresent(index)) {
			setGlobalTimer(0);
			useSameIndex = false;

			// TODO remove existing savegame and reset index & savegame name
			warning("[Menu::initGame] Not implemented");
		}

		doSavegame = false;
	} else {
		warning("[Menu::initGame] Renaming saves not implemented");
	}

	// Create a new savegame if needed
	if (!SaveLoad::isSavegamePresent(_gameId))
		getSaveLoad()->create(_gameId);

	if (doSavegame)
		getSaveLoad()->saveGame(kSavegameTypeEvent2, kEntityPlayer, kEventNone);

	if (!getGlobalTimer()) {
		warning("[Menu::initGame] Removing temporary saves not implemented");
	}

	// Init savegame & menu values
	_lastIndex = getSaveLoad()->init(_gameId, true);
	_lowerTime = getSaveLoad()->getTime(_lastIndex);

	if (useSameIndex)
		_index = _lastIndex;

	//if (!getGlobalTimer())
	//	_index3 = 0;

	if (!getProgress().chapter)
		getProgress().chapter = kChapter1;

	getState()->time = (TimeValue)getSaveLoad()->getTime(_index);
	getProgress().chapter = getSaveLoad()->getChapter(_index);

	if (_lowerTime >= kTimeStartGame) {
		_currentTime = (uint32)getState()->time;
		_time = (uint32)getState()->time;
		_clock->draw(_time);
		_trainLine->draw(_time);

		initTime(type, value);
	}
}

// Start a game (or load an existing savegame)
void Menu::startGame() {
	// Clear savegame headers
	getSaveLoad()->clear();

	// Hide menu elements
	_clock->clear();
	_trainLine->clear();

	if (_lastIndex == _index) {
		setGlobalTimer(0);
		if (_index) {
			getSaveLoad()->loadLastGame();
		} else {
			getLogic()->resetState();
			getEntities()->setup(true, kEntityPlayer);
		}
	} else {
		getSaveLoad()->loadGame(_index);
	}
}

// Switch to the next savegame
void Menu::switchGame() {

	// Switch back to blue game is the current game is not started
	_gameId = SaveLoad::isSavegameValid(_gameId) ? getNextGameId() : kGameBlue;

	// Initialize savegame if needed
	if (!SaveLoad::isSavegamePresent(_gameId))
		getSaveLoad()->create(_gameId);

	getState()->time = kTimeNone;

	// Clear menu elements
	_clock->clear();
	_trainLine->clear();

	// Clear loaded savegame data
	getSaveLoad()->clear(true);

	init(false, kSavegameTypeIndex, 0);
}

//////////////////////////////////////////////////////////////////////////
// Overlays & elements
//////////////////////////////////////////////////////////////////////////
void Menu::checkHotspots() {
	if (!_isShowingMenu)
		return;

	if (!getFlags()->shouldRedraw)
		return;

	if (_isShowingCredits)
		return;

	SceneHotspot *hotspot = NULL;
	getScenes()->get(getState()->scene)->checkHotSpot(getCoords(), &hotspot);

	if (hotspot)
		handleEvent((StartMenuAction)hotspot->action, _mouseFlags);
	else
		hideOverlays();
}

void Menu::hideOverlays() {
	_lastHotspot = NULL;

	// Hide all menu overlays
	for (MenuFrames::iterator it = _frames.begin(); it != _frames.end(); it++)
		showFrame(it->_key, -1, false);

	getScenes()->drawFrames(true);
}

void Menu::showFrame(StartMenuOverlay overlayType, int index, bool redraw) {
	if (index == -1) {
		getScenes()->removeFromQueue(_frames[overlayType]);
	} else {
		// Check that the overlay is valid
		if (!_frames[overlayType])
			return;

		// Remove the frame and add a new one with the proper index
		getScenes()->removeFromQueue(_frames[overlayType]);
		_frames[overlayType]->setFrame((uint16)index);
		getScenes()->addToQueue(_frames[overlayType]);
	}

	if (redraw)
		getScenes()->drawFrames(true);
}

// Remove all frames from the queue
void Menu::clear() {
	for (MenuFrames::iterator it = _frames.begin(); it != _frames.end(); it++)
		getScenes()->removeAndRedraw(&it->_value, false);

	clearBg(GraphicsManager::kBackgroundOverlay);
}

// Get the sequence name to use for the acorn highlight, depending of the currently loaded savegame
Common::String Menu::getAcornSequenceName(GameId id) const {
	Common::String name = "";
	switch (id) {
	default:
	case kGameBlue:
		name = "aconblu3.seq";
		break;

	case kGameRed:
		name = "aconred.seq";
		break;

	case kGameGreen:
		name = "acongren.seq";
		break;

	case kGamePurple:
		name = "aconpurp.seq";
		break;

	case kGameTeal:
		name = "aconteal.seq";
		break;

	case kGameGold:
		name = "acongold.seq";
		break;
	}

	return name;
}

//////////////////////////////////////////////////////////////////////////
// Time
//////////////////////////////////////////////////////////////////////////
void Menu::initTime(SavegameType type, uint32 value) {
	if (!value)
		return;

	// The savegame entry index
	uint32 entryIndex = 0;

	switch (type) {
	default:
		break;

	case kSavegameTypeIndex:
		entryIndex = (_index <= value) ? 1 : _index - value;
		break;

	case kSavegameTypeTime:
		if (value < kTimeStartGame)
			break;

		entryIndex = _index;
		if (!entryIndex)
			break;

		// Iterate through existing entries
		do {
			if (getSaveLoad()->getTime(entryIndex) <= value)
				break;

			entryIndex--;
		} while (entryIndex);
		break;

	case kSavegameTypeEvent:
		entryIndex = _index;
		if (!entryIndex)
			break;

		do {
			if (getSaveLoad()->getValue(entryIndex) == value)
				break;

			entryIndex--;
		} while (entryIndex);
		break;

	case kSavegameTypeEvent2:
		// TODO rewrite in a more legible way
		if (_index > 1) {
			uint32 index = _index;
			do {
				if (getSaveLoad()->getValue(index) == value)
					break;

				index--;
			} while (index > 1);

			entryIndex = index - 1;
		} else {
			entryIndex = _index - 1;
		}
		break;
	}

	if (entryIndex) {
		_currentIndex = entryIndex;
		updateTime(getSaveLoad()->getTime(entryIndex));
	}
}

void Menu::updateTime(uint32 time) {
	if (_currentTime == _time)
		_delta = 0;

	_currentTime = time;

	if (_time != time) {
		if (getSoundQueue()->isBuffered(kEntityChapters))
			getSoundQueue()->removeFromQueue(kEntityChapters);

		getSound()->playSoundWithSubtitles((_currentTime >= _time) ? "LIB042" : "LIB041", kFlagMenuClock, kEntityChapters);
		adjustIndex(_currentTime, _time, false);
	}
}

void Menu::adjustIndex(uint32 time1, uint32 time2, bool searchEntry) {
	uint32 index = 0;
	int32 timeDelta = -1;

	if (time1 != time2) {

		index = _index;

		if (time2 >= time1) {
			if (searchEntry) {
				uint32 currentIndex = _index;

				if ((int32)_index >= 0) {
					do {
						// Calculate new delta
						int32 newDelta = time1 - (uint32)getSaveLoad()->getTime(currentIndex);

						if (newDelta >= 0 && timeDelta >= newDelta) {
							timeDelta = newDelta;
							index = currentIndex;
						}

						--currentIndex;
					} while ((int32)currentIndex >= 0);
				}
			} else {
				index = _index - 1;
			}
		} else {
			if (searchEntry) {
				uint32 currentIndex = _index;

				if (_lastIndex >= _index) {
					do {
						// Calculate new delta
						int32 newDelta = (uint32)getSaveLoad()->getTime(currentIndex) - time1;

						if (newDelta >= 0 && timeDelta > newDelta) {
							timeDelta = newDelta;
							index = currentIndex;
						}

						++currentIndex;
					} while (currentIndex <= _lastIndex);
				}
			} else {
				index = _index + 1;
			}
		}

		_index = index;
		checkHotspots();
	}

	if (_index == _currentIndex) {
		if (getProgress().chapter != getSaveLoad()->getChapter(index))
			getProgress().chapter = getSaveLoad()->getChapter(_index);
	}
}

void Menu::goToTime(uint32 time) {

	uint32 entryIndex = 0;
	uint32 deltaTime = (uint32)ABS((int32)(getSaveLoad()->getTime(0) - time));
	uint32 index = 0;

	do {
		uint32 deltaTime2 = (uint32)ABS((int32)(getSaveLoad()->getTime(index) - time));
		if (deltaTime2 < deltaTime) {
			deltaTime = deltaTime2;
			entryIndex = index;
		}

		++index;
	} while (_lastIndex >= index);

	_currentIndex = entryIndex;
	updateTime(getSaveLoad()->getTime(entryIndex));
}

void Menu::setTime() {
	_currentIndex = _index;
	_currentTime = getSaveLoad()->getTime(_currentIndex);

	if (_time == _currentTime)
		adjustTime();
}

void Menu::forwardTime() {
	if (_lastIndex <= _index)
		return;

	_currentIndex = _lastIndex;
	updateTime(getSaveLoad()->getTime(_currentIndex));
}

void Menu::rewindTime() {
	if (!_index)
		return;

	_currentIndex = 0;
	updateTime(getSaveLoad()->getTime(_currentIndex));
}

void Menu::adjustTime() {
	uint32 originalTime = _time;

	// Adjust time delta
	Common::Rational timeDelta(_delta >= 90 ? 9 : (9 * _delta + 89), _delta >= 90 ? 1 : 90);

	if (_currentTime < _time) {
		timeDelta *= 900;
		_time -= (uint)timeDelta.toInt();

		if (_currentTime > _time)
			_time = _currentTime;
	} else {
		timeDelta *= 900;
		_time += (uint)timeDelta.toInt();

		if (_currentTime < _time)
			_time = _currentTime;
	}

	if (_currentTime == _time && getSoundQueue()->isBuffered(kEntityChapters))
		getSoundQueue()->removeFromQueue(kEntityChapters);

	_clock->draw(_time);
	_trainLine->draw(_time);
	getScenes()->drawFrames(true);

	adjustIndex(_time, originalTime, true);

	++_delta;
}

void Menu::moveToCity(CityButton city, bool clicked) {
	uint32 time = (uint32)_cityButtonsInfo[city].time;

	// TODO Check if we have access (there seems to be more checks on some internal times) - probably : current_time (menu only) / game time / some other?
	if (_lowerTime < time || _time == time || _currentTime == time) {
		hideOverlays();
		return;
	}

	// Show city overlay
	showFrame((StartMenuOverlay)((_cityButtonsInfo[city].index >> 6) + 3), _cityButtonsInfo[city].index & 63, true);

	if (clicked) {
		showFrame(kOverlayTooltip, -1, true);
		getSound()->playSound(kEntityPlayer, "LIB046");
		goToTime(time);

		_handleTimeDelta = true;

		return;
	}

	// Special case of first and last cities
	if (city == kParis || city == kConstantinople) {
		showFrame(kOverlayTooltip, (city == kParis) ? kTooltipRewindParis : kTooltipForwardConstantinople, true);
		return;
	}

	showFrame(kOverlayTooltip, (_time <= time) ? _cityButtonsInfo[city].forward : _cityButtonsInfo[city].rewind, true);
}

//////////////////////////////////////////////////////////////////////////
// Sound / Brightness
//////////////////////////////////////////////////////////////////////////

// Get current volume (converted internal ScummVM value)
uint32 Menu::getVolume() const {
	return getState()->volume;
}

// Set the volume (converts to ScummVM values)
void Menu::setVolume(uint32 volume) const {
	getState()->volume = volume;

	// Clamp volume
	uint32 value = volume * Audio::Mixer::kMaxMixerVolume / 7;

	if (value > Audio::Mixer::kMaxMixerVolume)
		value = Audio::Mixer::kMaxMixerVolume;

	_engine->_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, (int32)value);
}

uint32 Menu::getBrightness() const {
	return getState()->brightness;
}

void Menu::setBrightness(uint32 brightness) const {
	getState()->brightness = brightness;

	// TODO reload cursor & font with adjusted brightness
}

} // End of namespace LastExpress

/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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
#include "common/error.h"
#include "common/events.h"
#include "common/fs.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "common/random.h"
#include "backends/keymapper/keymapper.h"
#include "base/plugins.h"
#include "base/version.h"
#include "gui/saveload.h"
#include "video/qt_decoder.h"

#include "pegasus/console.h"
#include "pegasus/cursor.h"
#include "pegasus/energymonitor.h"
#include "pegasus/gamestate.h"
#include "pegasus/interface.h"
#include "pegasus/menu.h"
#include "pegasus/movie.h"
#include "pegasus/pegasus.h"
#include "pegasus/timers.h"
#include "pegasus/ai/ai_area.h"
#include "pegasus/items/itemlist.h"
#include "pegasus/items/biochips/aichip.h"
#include "pegasus/items/biochips/biochipitem.h"
#include "pegasus/items/biochips/mapchip.h"
#include "pegasus/items/biochips/opticalchip.h"
#include "pegasus/items/biochips/pegasuschip.h"
#include "pegasus/items/biochips/retscanchip.h"
#include "pegasus/items/biochips/shieldchip.h"
#include "pegasus/items/inventory/airmask.h"
#include "pegasus/items/inventory/gascanister.h"
#include "pegasus/items/inventory/inventoryitem.h"
#include "pegasus/items/inventory/keycard.h"
#include "pegasus/neighborhood/neighborhood.h"
#include "pegasus/neighborhood/caldoria/caldoria.h"
#include "pegasus/neighborhood/mars/mars.h"
#include "pegasus/neighborhood/norad/constants.h"
#include "pegasus/neighborhood/norad/alpha/noradalpha.h"
#include "pegasus/neighborhood/norad/delta/noraddelta.h"
#include "pegasus/neighborhood/prehistoric/prehistoric.h"
#include "pegasus/neighborhood/tsa/fulltsa.h"
#include "pegasus/neighborhood/tsa/tinytsa.h"
#include "pegasus/neighborhood/wsc/wsc.h"

namespace Pegasus {

PegasusEngine::PegasusEngine(OSystem *syst, const PegasusGameDescription *gamedesc) : Engine(syst), InputHandler(0), _gameDescription(gamedesc),
		_shellNotification(kJMPDCShellNotificationID, this), _returnHotspot(kInfoReturnSpotID), _itemDragger(this), _bigInfoMovie(kNoDisplayElement),
		_smallInfoMovie(kNoDisplayElement) {
	_continuePoint = 0;
	_saveAllowed = _loadAllowed = true;
	_saveRequested = _loadRequested = false;
	_gameMenu = 0;
	_deathReason = kDeathStranded;
	_neighborhood = 0;
	_FXLevel = 0x80;
	_ambientLevel = 0x80;
	_gameMode = kNoMode;
	_switchModesSync = false;
	_draggingItem = 0;
	_dragType = kDragNoDrag;
	_idlerHead = 0;
	_currentCD = 1;
	_introTimer = 0;
	_aiSaveStream = 0;
}

PegasusEngine::~PegasusEngine() {
	delete _resFork;
	delete _console;
	delete _cursor;
	delete _continuePoint;
	delete _gameMenu;
	delete _neighborhood;
	delete _rnd;
	delete _introTimer;
	delete _aiSaveStream;

	for (ItemIterator it = _allItems.begin(); it != _allItems.end(); it++)
		delete *it;

	InputDeviceManager::destroy();
	GameStateManager::destroy();

	// NOTE: This must be deleted last!
	delete _gfx;
}

Common::Error PegasusEngine::run() {
	_console = new PegasusConsole(this);
	_gfx = new GraphicsManager(this);
	_resFork = new Common::MacResManager();
	_cursor = new Cursor();
	_rnd = new Common::RandomSource("Pegasus");

	if (!_resFork->open("JMP PP Resources") || !_resFork->hasResFork())
		error("Could not load JMP PP Resources");

	// Initialize items
	createItems();

	// Initialize cursors
	_cursor->addCursorFrames(0x80); // Main
	_cursor->addCursorFrames(900);  // Mars Shuttle

	// Initialize the item dragger bounds
	_itemDragger.setHighlightBounds();

	if (!isDemo() && !detectOpeningClosingDirectory()) {
		Common::String message = "Missing intro directory. ";

		// Give Mac OS X a more specific message because we can
#ifdef MACOSX
		message += "Make sure \"Opening/Closing\" is present.";
#else
		message += "Be sure to rename \"Opening/Closing\" to \"Opening_Closing\".";
#endif

		GUIErrorMessage(message);
		warning("%s", message.c_str());
		return Common::kNoGameDataFoundError;
	}

	// Set up input
	initKeymap();
	InputHandler::setInputHandler(this);
	allowInput(true);

	// Set up inventories
	_items.setWeightLimit(9);
	_items.setOwnerID(kPlayerID);
	_biochips.setWeightLimit(8);
	_biochips.setOwnerID(kPlayerID);

	_returnHotspot.setArea(Common::Rect(kNavAreaLeft, kNavAreaTop, 512 + kNavAreaLeft, 256 + kNavAreaTop));
	_returnHotspot.setHotspotFlags(kInfoReturnSpotFlag);
	_allHotspots.push_back(&_returnHotspot);

	_screenDimmer.setBounds(Common::Rect(0, 0, 640, 480));
	_screenDimmer.setDisplayOrder(kScreenDimmerOrder);

	// Load from the launcher/cli if requested (and don't show the intro in those cases)
	bool doIntro = true;
	if (ConfMan.hasKey("save_slot")) {
		uint32 gameToLoad = ConfMan.getInt("save_slot");
		doIntro = (loadGameState(gameToLoad).getCode() != Common::kNoError);
	}

	_shellNotification.notifyMe(this, kJMPShellNotificationFlags, kJMPShellNotificationFlags);

	if (doIntro)
		// Start up the first notification
		_shellNotification.setNotificationFlags(kGameStartingFlag, kGameStartingFlag);

	if (!isDemo()) {
		_introTimer = new FuseFunction();
		_introTimer->setFunctor(new Common::Functor0Mem<void, PegasusEngine>(this, &PegasusEngine::introTimerExpired));
	}

	while (!shouldQuit()) {
		processShell();
		_system->delayMillis(10); // Ease off the CPU
	}

	return Common::kNoError;
}

bool PegasusEngine::canLoadGameStateCurrently() {
	return _loadAllowed && !isDemo();
}

bool PegasusEngine::canSaveGameStateCurrently() {
	return _saveAllowed && !isDemo() && g_neighborhood;
}

bool PegasusEngine::detectOpeningClosingDirectory() {
	// We need to detect what our Opening/Closing directory is listed as
	// On the original disc, it was 'Opening/Closing' but only HFS(+) supports the slash
	// Mac OS X will display this as 'Opening:Closing' and we can use that directly
	// On other systems, users will need to rename to "Opening_Closing"

	Common::FSNode gameDataDir(ConfMan.get("path"));
	gameDataDir = gameDataDir.getChild("Images");

	if (!gameDataDir.exists())
		return false;

	Common::FSList fsList;
	if (!gameDataDir.getChildren(fsList, Common::FSNode::kListDirectoriesOnly, true))
		return false;

	for (uint i = 0; i < fsList.size() && _introDirectory.empty(); i++) {
		Common::String name = fsList[i].getName();

		if (name.equalsIgnoreCase("Opening:Closing"))
			_introDirectory = name;
		else if (name.equalsIgnoreCase("Opening_Closing"))
			_introDirectory = name;
	}

	if (_introDirectory.empty())
		return false;

	debug(0, "Detected intro location as '%s'", _introDirectory.c_str());
	_introDirectory = Common::String("Images/") + _introDirectory;
	return true;
}

void PegasusEngine::createItems() {
	Common::SeekableReadStream *res = _resFork->getResource(MKTAG('N', 'I', 't', 'm'), 0x80);

	uint16 entryCount = res->readUint16BE();

	for (uint16 i = 0; i < entryCount; i++) {
		ItemID itemID = res->readUint16BE();
		NeighborhoodID neighborhoodID = res->readUint16BE();
		RoomID roomID = res->readUint16BE();
		DirectionConstant direction = res->readByte();
		res->readByte(); // alignment

		createItem(itemID, neighborhoodID, roomID, direction);
	}

	delete res;
}

void PegasusEngine::createItem(ItemID itemID, NeighborhoodID neighborhoodID, RoomID roomID, DirectionConstant direction) {
	switch (itemID) {
	case kInterfaceBiochip:
		// Unused in game, but still in the data and we need to create
		// it because it's saved/loaded from save files.
		new BiochipItem(itemID, neighborhoodID, roomID, direction);
		break;
	case kAIBiochip:
		new AIChip(itemID, neighborhoodID, roomID, direction);
		break;
	case kPegasusBiochip:
		new PegasusChip(itemID, neighborhoodID, roomID, direction);
		break;
	case kOpticalBiochip:
		new OpticalChip(itemID, neighborhoodID, roomID, direction);
		break;
	case kMapBiochip:
		new MapChip(itemID, neighborhoodID, roomID, direction);
		break;
	case kRetinalScanBiochip:
		new RetScanChip(itemID, neighborhoodID, roomID, direction);
		break;
	case kShieldBiochip:
		new ShieldChip(itemID, neighborhoodID, roomID, direction);
		break;
	case kAirMask:
		new AirMask(itemID, neighborhoodID, roomID, direction);
		break;
	case kKeyCard:
		new KeyCard(itemID, neighborhoodID, roomID, direction);
		break;
	case kGasCanister:
		new GasCanister(itemID, neighborhoodID, roomID, direction);
		break;
	default:
		// Everything else is a normal inventory item
		new InventoryItem(itemID, neighborhoodID, roomID, direction);
		break;
	}
}

void PegasusEngine::runIntro() {
	stopIntroTimer();

	bool skipped = false;

	Video::VideoDecoder *video = new Video::QuickTimeDecoder();
	if (video->loadFile(_introDirectory + "/BandaiLogo.movie")) {
		video->start();

		while (!shouldQuit() && !video->endOfVideo() && !skipped) {
			if (video->needsUpdate()) {
				const Graphics::Surface *frame = video->decodeNextFrame();

				if (frame) {
					_system->copyRectToScreen((byte *)frame->pixels, frame->pitch, 0, 0, frame->w, frame->h);
					_system->updateScreen();
				}
			}

			Input input;
			InputDevice.getInput(input, kFilterAllInput);
			if (input.anyInput())
				skipped = true;

			_system->delayMillis(10);
		}
	}

	delete video;

	if (shouldQuit() || skipped)
		return;

	video = new Video::QuickTimeDecoder();

	if (!video->loadFile(_introDirectory + "/Big Movie.movie"))
		error("Could not load intro movie");

	video->seek(Audio::Timestamp(0, 10 * 600, 600));
	video->start();

	playMovieScaled(video, 0, 0);

	delete video;
}

Common::Error PegasusEngine::showLoadDialog() {
	GUI::SaveLoadChooser slc(_("Load game:"), _("Load"), false);

	Common::String gameId = ConfMan.get("gameid");

	const EnginePlugin *plugin = 0;
	EngineMan.findGame(gameId, &plugin);

	int slot = slc.runModalWithPluginAndTarget(plugin, ConfMan.getActiveDomainName());

	Common::Error result;

	if (slot >= 0) {
		if (loadGameState(slot).getCode() == Common::kNoError)
			result = Common::kNoError;
		else
			result = Common::kUnknownError;
	} else {
		result = Common::kUserCanceled;
	}

	return result;
}

Common::Error PegasusEngine::showSaveDialog() {
	GUI::SaveLoadChooser slc(_("Save game:"), _("Save"), true);

	Common::String gameId = ConfMan.get("gameid");

	const EnginePlugin *plugin = 0;
	EngineMan.findGame(gameId, &plugin);

	int slot = slc.runModalWithPluginAndTarget(plugin, ConfMan.getActiveDomainName());

	Common::Error result;

	if (slot >= 0) {
		if (saveGameState(slot, slc.getResultString()).getCode() == Common::kNoError)
			result = Common::kNoError;
		else
			result = Common::kUnknownError;
	} else {
		result = Common::kUserCanceled;
	}

	return result;
}

GUI::Debugger *PegasusEngine::getDebugger() {
	return _console;
}

void PegasusEngine::addIdler(Idler *idler) {
	idler->_nextIdler = _idlerHead;
	if (_idlerHead)
		_idlerHead->_prevIdler = idler;
	idler->_prevIdler = 0;
	_idlerHead = idler;
}

void PegasusEngine::removeIdler(Idler *idler) {
	if (idler->_prevIdler)
		idler->_prevIdler->_nextIdler = idler->_nextIdler;
	if (idler->_nextIdler)
		idler->_nextIdler->_prevIdler = idler->_prevIdler;
	if (idler == _idlerHead)
		_idlerHead = idler->_nextIdler;
	idler->_nextIdler = 0;
	idler->_prevIdler = 0;
}

void PegasusEngine::giveIdleTime() {
	for (Idler *idler = _idlerHead; idler != 0; idler = idler->_nextIdler)
		idler->useIdleTime();
}

void PegasusEngine::addTimeBase(TimeBase *timeBase) {
	_timeBases.push_back(timeBase);
}

void PegasusEngine::removeTimeBase(TimeBase *timeBase) {
	_timeBases.remove(timeBase);
}

bool PegasusEngine::loadFromStream(Common::ReadStream *stream) {
	// Dispose currently running stuff
	useMenu(0);
	useNeighborhood(0);
	removeAllItemsFromInventory();
	removeAllItemsFromBiochips();
	_currentItemID = kNoItemID;
	_currentBiochipID = kNoItemID;

	if (!g_interface)
		createInterface();

	// Signature
	uint32 creator = stream->readUint32BE();
	if (creator != kPegasusPrimeCreator) {
		warning("Bad save creator '%s'", tag2str(creator));
		return false;
	}

	uint32 gameType = stream->readUint32BE();
	int saveType;

	switch (gameType) {
	case kPegasusPrimeDisk1GameType:
	case kPegasusPrimeDisk2GameType:
	case kPegasusPrimeDisk3GameType:
	case kPegasusPrimeDisk4GameType:
		_currentCD = gameType - kPegasusPrimeDisk1GameType + 1;
		saveType = kNormalSave;
		break;
	case kPegasusPrimeContinueType:
		saveType = kContinueSave;
		break;
	default:
		// There are five other possible game types on the Pippin
		// version, but hopefully we don't see any of those here
		warning("Unhandled pegasus game type '%s'", tag2str(gameType));
		return false;
	}

	uint32 version = stream->readUint32BE();
	if (version != kPegasusPrimeVersion) {
		warning("Where did you get this save? It's a beta (v%04x)!", version & 0x7fff);
		return false;
	}

	// Game State
	GameState.readGameState(stream);

	// Energy
	setLastEnergyValue(stream->readUint32BE());

	// Death reason
	setEnergyDeathReason(stream->readByte());

	// Items
	_allItems.readFromStream(stream);

	// Inventory
	byte itemCount = stream->readByte();

	if (itemCount > 0) {
		for (byte i = 0; i < itemCount; i++) {
			InventoryItem *inv = (InventoryItem *)_allItems.findItemByID((ItemID)stream->readUint16BE());
			addItemToInventory(inv);
		}

		g_interface->setCurrentInventoryItemID((ItemID)stream->readUint16BE());
	}

	// Biochips
	byte biochipCount = stream->readByte();

	if (biochipCount > 0) {
		for (byte i = 0; i < biochipCount; i++) {
			BiochipItem *biochip = (BiochipItem *)_allItems.findItemByID((ItemID)stream->readUint16BE());
			addItemToBiochips(biochip);
		}

		g_interface->setCurrentBiochipID((ItemID)stream->readUint16BE());
	}


	// TODO: Disc check

	// Jump to environment
	jumpToNewEnvironment(GameState.getCurrentNeighborhood(), GameState.getCurrentRoom(), GameState.getCurrentDirection());
	_shellNotification.setNotificationFlags(0, kNeedNewJumpFlag);
	performJump(GameState.getCurrentNeighborhood());

	// AI rules
	if (g_AIArea)
		g_AIArea->readAIRules(stream);

	startNeighborhood();

	// Make a new continue point if this isn't already one
	if (saveType == kNormalSave)
		makeContinuePoint();

	return true;
}

bool PegasusEngine::writeToStream(Common::WriteStream *stream, int saveType) {
	// WORKAROUND: If we don't have the interface, we can't actually save.
	// However, we should still have a continue point, so we will just dump that
	// out. This is needed for saving a game while in the space chase.
	if (!g_interface) {
		// Saving a continue stream from a continue stream should
		// never happen. In addition, we do need to have a continue
		// stream for this to work.
		if (saveType != kNormalSave || !_continuePoint)
			return false;

		writeContinueStream(stream);
		return true;
	}

	if (g_neighborhood)
		g_neighborhood->flushGameState();

	// Signature
	stream->writeUint32BE(kPegasusPrimeCreator);

	if (saveType == kNormalSave)
		stream->writeUint32BE(kPegasusPrimeDisk1GameType + _currentCD - 1);
	else // Continue
		stream->writeUint32BE(kPegasusPrimeContinueType);

	stream->writeUint32BE(kPegasusPrimeVersion);

	// Game State
	GameState.writeGameState(stream);

	// Energy
	stream->writeUint32BE(getSavedEnergyValue());

	// Death reason
	stream->writeByte(getEnergyDeathReason());

	// Items
	_allItems.writeToStream(stream);

	// Inventory
	byte itemCount = _items.getNumItems();
	stream->writeByte(itemCount);

	if (itemCount > 0) {
		for (uint32 i = 0; i < itemCount; i++)
			stream->writeUint16BE(_items.getItemIDAt(i));

		stream->writeUint16BE(g_interface->getCurrentInventoryItem()->getObjectID());
	}

	// Biochips
	byte biochipCount = _biochips.getNumItems();
	stream->writeByte(biochipCount);

	if (biochipCount > 0) {
		for (uint32 i = 0; i < biochipCount; i++)
			stream->writeUint16BE(_biochips.getItemIDAt(i));

		stream->writeUint16BE(g_interface->getCurrentBiochip()->getObjectID());
	}

	// AI rules
	if (g_AIArea)
		g_AIArea->writeAIRules(stream);

	return true;
}

void PegasusEngine::makeContinuePoint() {
	// WORKAROUND: Do not attempt to make a continue point if the interface is not set
	// up. The original did *not* do this and attempting to restore the game using the pause
	// menu during the canyon/space chase sequence would segfault the game and crash the
	// system. Nice!
	if (!g_interface)
		return;

	delete _continuePoint;

	Common::MemoryWriteStreamDynamic newPoint(DisposeAfterUse::NO);
	writeToStream(&newPoint, kContinueSave);
	_continuePoint = new Common::MemoryReadStream(newPoint.getData(), newPoint.size(), DisposeAfterUse::YES);
}

void PegasusEngine::loadFromContinuePoint() {
	// Failure to load a continue point is fatal

	if (!_continuePoint)
		error("Attempting to load from non-existant continue point");

	_continuePoint->seek(0);

	if (!loadFromStream(_continuePoint))
		error("Failed loading continue point");
}

void PegasusEngine::writeContinueStream(Common::WriteStream *stream) {
	// We're going to pretty much copy the stream, except for the save type
	_continuePoint->seek(0);
	stream->writeUint32BE(_continuePoint->readUint32BE());
	_continuePoint->readUint32BE(); // skip the continue type
	stream->writeUint32BE(kPegasusPrimeDisk1GameType + _currentCD - 1);

	// Now just copy over the rest
	uint32 size = _continuePoint->size() - _continuePoint->pos();
	byte *data = new byte[size];
	_continuePoint->read(data, size);
	stream->write(data, size);
	delete[] data;
}

Common::Error PegasusEngine::loadGameState(int slot) {
	Common::StringArray filenames = _saveFileMan->listSavefiles("pegasus-*.sav");
	Common::InSaveFile *loadFile = _saveFileMan->openForLoading(filenames[slot]);
	if (!loadFile)
		return Common::kUnknownError;

	bool valid = loadFromStream(loadFile);
	delete loadFile;

	return valid ? Common::kNoError : Common::kUnknownError;
}

Common::Error PegasusEngine::saveGameState(int slot, const Common::String &desc) {
	Common::String output = Common::String::format("pegasus-%s.sav", desc.c_str());
	Common::OutSaveFile *saveFile = _saveFileMan->openForSaving(output, false);
	if (!saveFile)
		return Common::kUnknownError;

	bool valid = writeToStream(saveFile, kNormalSave);
	delete saveFile;

	return valid ? Common::kNoError : Common::kUnknownError;
}

void PegasusEngine::receiveNotification(Notification *notification, const NotificationFlags flags) {
	if (&_shellNotification == notification) {
		switch (flags) {
		case kGameStartingFlag: {
			useMenu(new MainMenu());

			if (!isDemo()) {
				runIntro();
				resetIntroTimer();
			} else {
				showTempScreen("Images/Demo/NGsplashScrn.pict");
			}

			if (shouldQuit())
				return;

			_gfx->invalRect(Common::Rect(0, 0, 640, 480));
			_gfx->updateDisplay();
			((MainMenu *)_gameMenu)->startMainMenuLoop();
			break;
		}
		case kPlayerDiedFlag:
			doDeath();
			break;
		case kNeedNewJumpFlag:
			performJump(GameState.getNextNeighborhood());
			startNeighborhood();
			break;
		default:
			break;
		}
	}
}

void PegasusEngine::checkCallBacks() {
	for (Common::List<TimeBase *>::iterator it = _timeBases.begin(); it != _timeBases.end(); it++)
		(*it)->checkCallBacks();
}

void PegasusEngine::resetIntroTimer() {
	if (!isDemo() && _gameMenu && _gameMenu->getObjectID() == kMainMenuID) {
		_introTimer->stopFuse();
		_introTimer->primeFuse(kIntroTimeOut);
		_introTimer->lightFuse();
	}
}

void PegasusEngine::introTimerExpired() {
	if (_gameMenu && _gameMenu->getObjectID() == kMainMenuID) {
		((MainMenu *)_gameMenu)->stopMainMenuLoop();

		bool skipped = false;

		Video::VideoDecoder *video = new Video::QuickTimeDecoder();
		if (!video->loadFile(_introDirectory + "/LilMovie.movie"))
			error("Failed to load little movie");

		bool saveAllowed = swapSaveAllowed(false);
		bool openAllowed = swapLoadAllowed(false);

		video->start();
		skipped = playMovieScaled(video, 0, 0);

		delete video;

		if (shouldQuit())
			return;

		if (!skipped) {
			runIntro();

			if (shouldQuit())
				return;
		}

		resetIntroTimer();
		_gfx->invalRect(Common::Rect(0, 0, 640, 480));

		swapSaveAllowed(saveAllowed);
		swapLoadAllowed(openAllowed);

		_gfx->updateDisplay();
		((MainMenu *)_gameMenu)->startMainMenuLoop();
	}
}

void PegasusEngine::stopIntroTimer() {
	if (_introTimer)
		_introTimer->stopFuse();
}

void PegasusEngine::delayShell(TimeValue time, TimeScale scale) {
	if (time == 0 || scale == 0)
		return;

	uint32 startTime = g_system->getMillis();
	uint32 timeInMillis = time * 1000 / scale;

	while (g_system->getMillis() < startTime + timeInMillis) {
		checkCallBacks();
		_gfx->updateDisplay();
	}
}

void PegasusEngine::useMenu(GameMenu *newMenu) {
	if (_gameMenu) {
		_gameMenu->restorePreviousHandler();
		delete _gameMenu;
	}

	_gameMenu = newMenu;

	if (_gameMenu)
		_gameMenu->becomeCurrentHandler();
}

bool PegasusEngine::checkGameMenu() {
	GameMenuCommand command = kMenuCmdNoCommand;

	if (_gameMenu) {
		command = _gameMenu->getLastCommand();
		if (command != kMenuCmdNoCommand) {
			_gameMenu->clearLastCommand();
			doGameMenuCommand(command);
		}
	}

	return command != kMenuCmdNoCommand;
}

void PegasusEngine::doGameMenuCommand(const GameMenuCommand command) {
	Common::Error result;

	switch (command) {
	case kMenuCmdStartAdventure:
		stopIntroTimer();
		GameState.setWalkthroughMode(false);
		startNewGame();
		break;
	case kMenuCmdCredits:
		if (isDemo()) {
			showTempScreen("Images/Demo/DemoCredits.pict");
			_gfx->doFadeOutSync();
			_gfx->updateDisplay();
			_gfx->doFadeInSync();
		} else {
			stopIntroTimer();
			_gfx->doFadeOutSync();
			useMenu(new CreditsMenu());
			_gfx->updateDisplay();
			_gfx->doFadeInSync();
		}
		break;
	case kMenuCmdQuit:
	case kMenuCmdDeathQuitDemo:
		if (isDemo())
			showTempScreen("Images/Demo/NGquitScrn.pict");
		_gfx->doFadeOutSync();
		quitGame();
		break;
	case kMenuCmdOverview:
		stopIntroTimer();
		doInterfaceOverview();
		resetIntroTimer();
		break;
	case kMenuCmdStartWalkthrough:
		stopIntroTimer();
		GameState.setWalkthroughMode(true);
		startNewGame();
		break;
	case kMenuCmdRestore:
		stopIntroTimer();
		// fall through
	case kMenuCmdDeathRestore:
		result = showLoadDialog();
		if (command == kMenuCmdRestore && result.getCode() != Common::kNoError)
			resetIntroTimer();
		break;
	case kMenuCmdCreditsMainMenu:
		_gfx->doFadeOutSync();
		useMenu(new MainMenu());
		_gfx->updateDisplay();
		((MainMenu *)_gameMenu)->startMainMenuLoop();
		_gfx->doFadeInSync();
		resetIntroTimer();
		break;
	case kMenuCmdDeathContinue:
		if (((DeathMenu *)_gameMenu)->playerWon()) {
			if (isDemo()) {
				showTempScreen("Images/Demo/DemoCredits.pict");
				_gfx->doFadeOutSync();
				_gfx->updateDisplay();
				_gfx->doFadeInSync();
			} else {
				_gfx->doFadeOutSync();
				useMenu(0);
				_gfx->clearScreen();
				_gfx->updateDisplay();

				Video::VideoDecoder *video = new Video::QuickTimeDecoder();
				if (!video->loadFile(_introDirectory + "/Closing.movie"))
					error("Could not load closing movie");

				uint16 x = (640 - video->getWidth() * 2) / 2;
				uint16 y = (480 - video->getHeight() * 2) / 2;

				video->start();
				playMovieScaled(video, x, y);

				delete video;

				if (shouldQuit())
					return;

				useMenu(new MainMenu());
				_gfx->updateDisplay();
				((MainMenu *)_gameMenu)->startMainMenuLoop();
				_gfx->doFadeInSync();
				resetIntroTimer();
			}
		} else {
			loadFromContinuePoint();
		}
		break;
	case kMenuCmdDeathMainMenuDemo:
	case kMenuCmdDeathMainMenu:
		_gfx->doFadeOutSync();
		useMenu(new MainMenu());
		_gfx->updateDisplay();
		((MainMenu *)_gameMenu)->startMainMenuLoop();
		_gfx->doFadeInSync();
		resetIntroTimer();
		break;
	case kMenuCmdPauseSave:
		if (showSaveDialog().getCode() != Common::kUserCanceled)
			pauseMenu(false);
		break;
	case kMenuCmdPauseContinue:
		pauseMenu(false);
		break;
	case kMenuCmdPauseRestore:
		makeContinuePoint();
		result = showLoadDialog();

		if (result.getCode() == Common::kNoError) {
			// Successfully loaded, unpause the game
			pauseMenu(false);
		} else if (result.getCode() != Common::kUserCanceled) {
			// Try to get us back to a sane state
			loadFromContinuePoint();
		}
		break;
	case kMenuCmdPauseQuit:
		_gfx->doFadeOutSync();
		throwAwayEverything();
		pauseMenu(false);
		useMenu(new MainMenu());
		_gfx->updateDisplay();
		((MainMenu *)_gameMenu)->startMainMenuLoop();
		_gfx->doFadeInSync();
		resetIntroTimer();
		break;
	case kMenuCmdNoCommand:
		break;
	default:
		error("Unknown menu command %d", command);
	}
}

void PegasusEngine::handleInput(const Input &input, const Hotspot *cursorSpot) {
	if (!checkGameMenu())
		shellGameInput(input, cursorSpot);

	// Handle the console here
	if (input.isConsoleRequested()) {
		_console->attach();
		_console->onFrame();
	}

	// Handle save requests here
	if (_saveRequested && _saveAllowed) {
		_saveRequested = false;

		// Can only save during a game and not in the demo
		if (g_neighborhood && !isDemo()) {
			pauseEngine(true);
			showSaveDialog();
			pauseEngine(false);
		}
	}

	// Handle load requests here
	if (_loadRequested && _loadAllowed) {
		_loadRequested = false;

		// WORKAROUND: Do not entertain load requests when the pause menu is up
		// The original did and the game entered a bad state after loading.
		// It's theoretically possible to make it so it does work while the
		// pause menu is up, but the pause state of the engine is just too weird.
		// Just use the pause menu's restore button since it's there for that
		// for you to load anyway.
		if (!isDemo() && !(_gameMenu && _gameMenu->getObjectID() == kPauseMenuID)) {
			pauseEngine(true);

			if (g_neighborhood) {
				makeContinuePoint();

				Common::Error result = showLoadDialog();
				if (result.getCode() != Common::kNoError && result.getCode() != Common::kUserCanceled)
					loadFromContinuePoint();
			} else {
				if (_introTimer)
					_introTimer->stopFuse();

				Common::Error result = showLoadDialog();
				if (result.getCode() != Common::kNoError) {
					if (!_gameMenu) {
						useMenu(new MainMenu());
						((MainMenu *)_gameMenu)->startMainMenuLoop();
					}

					resetIntroTimer();
				}
			}

			pauseEngine(false);
		}
	}
}

void PegasusEngine::doInterfaceOverview() {
	static const short kNumOverviewSpots = 11;
	static const Common::Rect overviewSpots[kNumOverviewSpots] = {
		Common::Rect(354, 318, 354 + 204, 318 + 12),
		Common::Rect(211, 34, 211 + 114, 34 + 28),
		Common::Rect(502, 344, 502 + 138, 344 + 120),
		Common::Rect(132, 40, 132 + 79, 40 + 22),
		Common::Rect(325, 40, 332 + 115, 40 + 22),
		Common::Rect(70, 318, 70 + 284, 318 + 12),
		Common::Rect(76, 334, 76 + 96, 334 + 96),
		Common::Rect(64, 64, 64 + 512, 64 + 256),
		Common::Rect(364, 334, 364 + 96, 334 + 96),
		Common::Rect(172, 334, 172 + 192, 334 + 96),
		Common::Rect(542, 36, 542 + 58, 36 + 20)
	};

	_gfx->doFadeOutSync();
	useMenu(0);

	Picture leftBackground(kNoDisplayElement);
	leftBackground.initFromPICTFile("Images/Interface/OVLeft.mac");
	leftBackground.setDisplayOrder(0);
	leftBackground.moveElementTo(kBackground1Left, kBackground1Top);
	leftBackground.startDisplaying();
	leftBackground.show();

	Picture topBackground(kNoDisplayElement);
	topBackground.initFromPICTFile("Images/Interface/OVTop.mac");
	topBackground.setDisplayOrder(0);
	topBackground.moveElementTo(kBackground2Left, kBackground2Top);
	topBackground.startDisplaying();
	topBackground.show();

	Picture rightBackground(kNoDisplayElement);
	rightBackground.initFromPICTFile("Images/Interface/OVRight.mac");
	rightBackground.setDisplayOrder(0);
	rightBackground.moveElementTo(kBackground3Left, kBackground3Top);
	rightBackground.startDisplaying();
	rightBackground.show();

	Picture bottomBackground(kNoDisplayElement);
	bottomBackground.initFromPICTFile("Images/Interface/OVBottom.mac");
	bottomBackground.setDisplayOrder(0);
	bottomBackground.moveElementTo(kBackground4Left, kBackground4Top);
	bottomBackground.startDisplaying();
	bottomBackground.show();

	Picture controllerHighlight(kNoDisplayElement);
	controllerHighlight.initFromPICTFile("Images/Interface/OVcontrollerHilite.mac");
	controllerHighlight.setDisplayOrder(0);
	controllerHighlight.moveElementTo(kOverviewControllerLeft, kOverviewControllerTop);
	controllerHighlight.startDisplaying();

	Movie overviewText(kNoDisplayElement);
	overviewText.initFromMovieFile("Images/Interface/Overview Mac.movie");
	overviewText.setDisplayOrder(0);
	overviewText.moveElementTo(kNavAreaLeft, kNavAreaTop);
	overviewText.startDisplaying();
	overviewText.show();
	overviewText.redrawMovieWorld();

	DropHighlight highlight(kNoDisplayElement);
	highlight.setDisplayOrder(1);
	highlight.startDisplaying();
	highlight.setHighlightThickness(4);
	highlight.setHighlightColor(g_system->getScreenFormat().RGBToColor(239, 239, 0));
	highlight.setHighlightCornerDiameter(8);

	Input input;
	InputDevice.getInput(input, kFilterAllInput);

	Common::Point cursorLoc;
	input.getInputLocation(cursorLoc);

	uint16 i;
	for (i = 0; i < kNumOverviewSpots; ++i)
		if (overviewSpots[i].contains(cursorLoc))
			break;

	TimeValue time;
	if (i == kNumOverviewSpots)
		time = 5;
	else if (i > 4)
		time = i + 1;
	else
		time = i;

	if (time == 2) {
		highlight.hide();
		controllerHighlight.show();
	} else if (i != kNumOverviewSpots) {
		controllerHighlight.hide();
		Common::Rect r = overviewSpots[i];
		r.grow(5);
		highlight.setBounds(r);
		highlight.show();
	} else {
		highlight.hide();
		controllerHighlight.hide();
	}

	overviewText.setTime(time * 3 + 2, 15);
	overviewText.redrawMovieWorld();

	_cursor->setCurrentFrameIndex(3);
	_cursor->show();

	_gfx->updateDisplay();
	_gfx->doFadeInSync();

	for (;;) {
		InputDevice.getInput(input, kFilterAllInput);

		if (input.anyInput() || shouldQuit() || _loadRequested || _saveRequested)
			break;

		input.getInputLocation(cursorLoc);
		for (i = 0; i < kNumOverviewSpots; ++i)
			if (overviewSpots[i].contains(cursorLoc))
				break;

		if (i == kNumOverviewSpots)
			time = 5;
		else if (i > 4)
			time = i + 1;
		else
			time = i;

		if (time == 2) {
			highlight.hide();
			controllerHighlight.show();
		} else if (i != kNumOverviewSpots) {
			controllerHighlight.hide();
			Common::Rect r = overviewSpots[i];
			r.grow(5);
			highlight.setBounds(r);
			highlight.show();
		} else {
			highlight.hide();
			controllerHighlight.hide();
		}

		// The original just constantly redraws the frame, but that
		// doesn't actually need to be done.
		if ((time * 3 + 2) * 40 != overviewText.getTime()) {
			overviewText.setTime(time * 3 + 2, 15);
			overviewText.redrawMovieWorld();
		}

		refreshDisplay();
		_system->delayMillis(10);
	}

	if (shouldQuit())
		return;

	highlight.hide();
	_cursor->hide();

	_gfx->doFadeOutSync();
	useMenu(new MainMenu());
	_gfx->updateDisplay();
	((MainMenu *)_gameMenu)->startMainMenuLoop();
	_gfx->doFadeInSync();

	_saveRequested = false;
	_loadRequested = false;
}

void PegasusEngine::showTempScreen(const Common::String &fileName) {
	_gfx->doFadeOutSync();

	Picture picture(0);
	picture.initFromPICTFile(fileName);
	picture.setDisplayOrder(kMaxAvailableOrder);
	picture.startDisplaying();
	picture.show();
	_gfx->updateDisplay();

	_gfx->doFadeInSync();

	// Wait for the next event
	bool done = false;
	while (!shouldQuit() && !done) {
		Common::Event event;
		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_LBUTTONUP:
			case Common::EVENT_RBUTTONUP:
			case Common::EVENT_KEYDOWN:
				done = true;
				break;
			default:
				break;
			}
		}

		_system->delayMillis(10);
	}
}

void PegasusEngine::refreshDisplay() {
	giveIdleTime();
	_gfx->updateDisplay();
}

void PegasusEngine::resetEnergyDeathReason() {
	switch (getCurrentNeighborhoodID()) {
	case kMarsID:
		_deathReason = kDeathArrestedInMars;
		break;
	case kNoradAlphaID:
	case kNoradDeltaID:
		_deathReason = kDeathArrestedInNorad;
		break;
	case kWSCID:
		_deathReason = kDeathArrestedInWSC;
		break;
	default:
		_deathReason = kDeathStranded;
		break;
	}
}

bool PegasusEngine::playerHasItem(const Item *item) {
	return playerHasItemID(item->getObjectID());
}

bool PegasusEngine::playerHasItemID(const ItemID itemID) {
	return itemInInventory(itemID) || itemInBiochips(itemID);
}

InventoryItem *PegasusEngine::getCurrentInventoryItem() {
	if (g_interface)
		return g_interface->getCurrentInventoryItem();

	return 0;
}

bool PegasusEngine::itemInInventory(InventoryItem *item) {
	return _items.itemInInventory(item);
}

bool PegasusEngine::itemInInventory(ItemID id) {
	return _items.itemInInventory(id);
}

BiochipItem *PegasusEngine::getCurrentBiochip() {
	if (g_interface)
		return g_interface->getCurrentBiochip();

	return 0;
}

bool PegasusEngine::itemInBiochips(BiochipItem *item) {
	return _biochips.itemInInventory(item);
}

bool PegasusEngine::itemInBiochips(ItemID id) {
	return _biochips.itemInInventory(id);
}

bool PegasusEngine::playerAlive() {
	return (_shellNotification.getNotificationFlags() & kPlayerDiedFlag) == 0;
}

Common::String PegasusEngine::getBriefingMovie() {
	if (_neighborhood)
		return _neighborhood->getBriefingMovie();

	return Common::String();
}

Common::String PegasusEngine::getEnvScanMovie() {
	if (_neighborhood)
		return _neighborhood->getEnvScanMovie();

	return Common::String();
}

uint PegasusEngine::getNumHints() {
	if (_neighborhood)
		return _neighborhood->getNumHints();

	return 0;
}

Common::String PegasusEngine::getHintMovie(uint hintNum) {
	if (_neighborhood)
		return _neighborhood->getHintMovie(hintNum);

	return Common::String();
}

bool PegasusEngine::canSolve() {
	if (_neighborhood)
		return _neighborhood->canSolve();

	return false;
}

void PegasusEngine::prepareForAIHint(const Common::String &movieName) {
	if (g_neighborhood)
		g_neighborhood->prepareForAIHint(movieName);
}

void PegasusEngine::cleanUpAfterAIHint(const Common::String &movieName) {
	if (g_neighborhood)
		g_neighborhood->cleanUpAfterAIHint(movieName);
}

void PegasusEngine::jumpToNewEnvironment(const NeighborhoodID neighborhoodID, const RoomID roomID, const DirectionConstant direction) {
	GameState.setNextLocation(neighborhoodID, roomID, direction);
	_shellNotification.setNotificationFlags(kNeedNewJumpFlag, kNeedNewJumpFlag);
}

void PegasusEngine::checkFlashlight() {
	if (_neighborhood)
		_neighborhood->checkFlashlight();
}

bool PegasusEngine::playMovieScaled(Video::VideoDecoder *video, uint16 x, uint16 y) {
	bool skipped = false;

	while (!shouldQuit() && !video->endOfVideo() && !skipped) {
		if (video->needsUpdate()) {
			const Graphics::Surface *frame = video->decodeNextFrame();

			if (frame)
				drawScaledFrame(frame, x, y);
		}

		Input input;
		InputDevice.getInput(input, kFilterAllInput);
		if (input.anyInput() || _saveRequested || _loadRequested)
			skipped = true;

		_system->delayMillis(10);
	}

	return skipped;
}

void PegasusEngine::die(const DeathReason reason) {
	Input dummy;
	if (isDragging())
		_itemDragger.stopTracking(dummy);

	_deathReason = reason;
	_shellNotification.setNotificationFlags(kPlayerDiedFlag, kPlayerDiedFlag);
}

void PegasusEngine::doDeath() {
	_gfx->doFadeOutSync();
	throwAwayEverything();
	useMenu(new DeathMenu(_deathReason));
	_gfx->updateDisplay();
	_gfx->doFadeInSync();
}

void PegasusEngine::throwAwayEverything() {
	if (_items.getNumItems() != 0 && g_interface)
		_currentItemID = g_interface->getCurrentInventoryItem()->getObjectID();
	else
		_currentItemID = kNoItemID;

	if (_biochips.getNumItems() != 0 && g_interface)
		_currentBiochipID = g_interface->getCurrentBiochip()->getObjectID();
	else
		_currentBiochipID = kNoItemID;

	useMenu(0);
	useNeighborhood(0);

	delete g_interface;
	g_interface = 0;
}

void PegasusEngine::processShell() {
	checkCallBacks();
	checkNotifications();
	InputHandler::pollForInput();
	refreshDisplay();
}

void PegasusEngine::createInterface() {
	if (!g_interface)
		new Interface();

	g_interface->createInterface();
}

void PegasusEngine::setGameMode(const GameMode newMode) {
	if (newMode != _gameMode && canSwitchGameMode(newMode, _gameMode)) {
		switchGameMode(newMode, _gameMode);
		_gameMode = newMode;
	}
}

void PegasusEngine::switchGameMode(const GameMode newMode, const GameMode oldMode) {
	// Start raising panels before lowering panels, to give the activating panel time
	// to set itself up without cutting into the lowering panel's animation time.

	if (_switchModesSync) {
		if (newMode == kModeInventoryPick)
			raiseInventoryDrawerSync();
		else if (newMode == kModeBiochipPick)
			raiseBiochipDrawerSync();
		else if (newMode == kModeInfoScreen)
			showInfoScreen();

		if (oldMode == kModeInventoryPick)
			lowerInventoryDrawerSync();
		else if (oldMode == kModeBiochipPick)
			lowerBiochipDrawerSync();
		else if (oldMode == kModeInfoScreen)
			hideInfoScreen();
	} else {
		if (newMode == kModeInventoryPick)
			raiseInventoryDrawer();
		else if (newMode == kModeBiochipPick)
			raiseBiochipDrawer();
		else if (newMode == kModeInfoScreen)
			showInfoScreen();

		if (oldMode == kModeInventoryPick)
			lowerInventoryDrawer();
		else if (oldMode == kModeBiochipPick)
			lowerBiochipDrawer();
		else if (oldMode == kModeInfoScreen)
			hideInfoScreen();
	}
}

bool PegasusEngine::canSwitchGameMode(const GameMode newMode, const GameMode oldMode) {
	if (newMode == kModeInventoryPick && oldMode == kModeBiochipPick)
		return false;
	if (newMode == kModeBiochipPick && oldMode == kModeInventoryPick)
		return false;
	return true;
}

bool PegasusEngine::itemInLocation(const ItemID itemID, const NeighborhoodID neighborhood, const RoomID room, const DirectionConstant direction) {
	NeighborhoodID itemNeighborhood;
	RoomID itemRoom;
	DirectionConstant itemDirection;

	Item *item = _allItems.findItemByID(itemID);
	item->getItemRoom(itemNeighborhood, itemRoom, itemDirection);

	return itemNeighborhood == neighborhood && itemRoom == room && itemDirection == direction;
}

InventoryResult PegasusEngine::addItemToInventory(InventoryItem *item) {
	InventoryResult result;

	do {
		if (g_interface)
			result = g_interface->addInventoryItem(item);
		else
			result = _items.addItem(item);

		if (result == kTooMuchWeight)
			destroyInventoryItem(pickItemToDestroy());
	} while (result != kInventoryOK);

	GameState.setTakenItem(item, true);
	if (g_neighborhood)
			g_neighborhood->pickedUpItem(item);

	g_AIArea->checkMiddleArea();

	return result;
}

void PegasusEngine::useNeighborhood(Neighborhood *neighborhood) {
	delete _neighborhood;
	_neighborhood = neighborhood;

	if (_neighborhood) {
		InputHandler::setInputHandler(_neighborhood);
		_neighborhood->init();
		_neighborhood->moveNavTo(kNavAreaLeft, kNavAreaTop);
		g_interface->setDate(_neighborhood->getDateResID());
	} else {
		InputHandler::setInputHandler(this);
	}
}

void PegasusEngine::performJump(NeighborhoodID neighborhoodID) {
	if (_neighborhood)
		useNeighborhood(0);

	// Sub chase is special
	if (neighborhoodID == kNoradSubChaseID) {
		throwAwayEverything();
		_loadAllowed = false;
		doSubChase();

		if (shouldQuit())
			return;

		neighborhoodID = kNoradDeltaID;
		GameState.setNextRoom(kNorad41);
		GameState.setNextDirection(kEast);
		_loadAllowed = true;
	}

	Neighborhood *neighborhood;
	makeNeighborhood(neighborhoodID, neighborhood);
	useNeighborhood(neighborhood);

	// Update the CD variable (used just for saves currently)
	_currentCD = getNeighborhoodCD(neighborhoodID);
}

void PegasusEngine::startNeighborhood() {
	if (g_interface && _currentItemID != kNoItemID)
		g_interface->setCurrentInventoryItemID(_currentItemID);

	if (g_interface && _currentBiochipID != kNoItemID)
		g_interface->setCurrentBiochipID(_currentBiochipID);

	setGameMode(kModeNavigation);

	if (_neighborhood)
		_neighborhood->start();
}

void PegasusEngine::startNewGame() {
	// WORKAROUND: The original game ignored the menu difficulty
	// setting. We're going to pass it through here so that
	// the menu actually makes sense now.
	bool isWalkthrough = GameState.getWalkthroughMode();
	GameState.resetGameState();
	GameState.setWalkthroughMode(isWalkthrough);

	// TODO: Enable erase
	_gfx->doFadeOutSync();
	useMenu(0);
	_gfx->updateDisplay();
	_gfx->enableUpdates();

	createInterface();

	if (isDemo()) {
		setLastEnergyValue(kFullEnergy);
		jumpToNewEnvironment(kPrehistoricID, kPrehistoric02, kSouth);
		GameState.setPrehistoricSeenTimeStream(false);
		GameState.setPrehistoricSeenFlyer1(false);
		GameState.setPrehistoricSeenFlyer2(false);
		GameState.setPrehistoricSeenBridgeZoom(false);
		GameState.setPrehistoricBreakerThrown(false);
	} else {
		jumpToNewEnvironment(kCaldoriaID, kCaldoria00, kEast);
	}

	removeAllItemsFromInventory();
	removeAllItemsFromBiochips();

	BiochipItem *biochip = (BiochipItem *)_allItems.findItemByID(kAIBiochip);
	addItemToBiochips(biochip);

	if (isDemo()) {
		biochip = (BiochipItem *)_allItems.findItemByID(kPegasusBiochip);
		addItemToBiochips(biochip);
		biochip = (BiochipItem *)_allItems.findItemByID(kMapBiochip);
		addItemToBiochips(biochip);
		InventoryItem *item = (InventoryItem *)_allItems.findItemByID(kKeyCard);
		addItemToInventory(item);
		item = (InventoryItem *)_allItems.findItemByID(kJourneymanKey);
		addItemToInventory(item);
		_currentItemID = kJourneymanKey;
	} else {
		_currentItemID = kNoItemID;
	}

	_currentBiochipID = kAIBiochip;

	// Clear jump notification flags and just perform the jump...
	_shellNotification.setNotificationFlags(0, kNeedNewJumpFlag);

	performJump(GameState.getNextNeighborhood());

	startNeighborhood();
}

void PegasusEngine::makeNeighborhood(NeighborhoodID neighborhoodID, Neighborhood *&neighborhood) {
	// TODO: CD check

	switch (neighborhoodID) {
	case kCaldoriaID:
		neighborhood = new Caldoria(g_AIArea, this);
		break;
	case kMarsID:
		neighborhood = new Mars(g_AIArea, this);
		break;
	case kPrehistoricID:
		neighborhood = new Prehistoric(g_AIArea, this);
		break;
	case kFullTSAID:
		neighborhood = new FullTSA(g_AIArea, this);
		break;
	case kTinyTSAID:
		neighborhood = new TinyTSA(g_AIArea, this);
		break;
	case kWSCID:
		neighborhood = new WSC(g_AIArea, this);
		break;
	case kNoradAlphaID:
		neighborhood = new NoradAlpha(g_AIArea, this);
		break;
	case kNoradDeltaID:
		createInterface();
		neighborhood = new NoradDelta(g_AIArea, this);
		break;
	default:
		error("Unknown neighborhood %d", neighborhoodID);
	}
}

bool PegasusEngine::wantsCursor() {
	return _gameMenu == 0;
}

void PegasusEngine::updateCursor(const Common::Point, const Hotspot *cursorSpot) {
	if (_itemDragger.isTracking()) {
		_cursor->setCurrentFrameIndex(5);
	} else {
		if (!cursorSpot) {
			_cursor->setCurrentFrameIndex(0);
		} else {
			uint32 id = cursorSpot->getObjectID();

			switch (id) {
			case kCurrentItemSpotID:
				if (countInventoryItems() != 0)
					_cursor->setCurrentFrameIndex(4);
				else
					_cursor->setCurrentFrameIndex(0);
				break;
			default:
				HotSpotFlags flags = cursorSpot->getHotspotFlags();

				if (flags & kZoomInSpotFlag)
					_cursor->setCurrentFrameIndex(1);
				else if (flags & kZoomOutSpotFlag)
					_cursor->setCurrentFrameIndex(2);
				else if (flags & (kPickUpItemSpotFlag | kPickUpBiochipSpotFlag))
					_cursor->setCurrentFrameIndex(4);
				else if (flags & kJMPClickingSpotFlags)
					_cursor->setCurrentFrameIndex(3);
				else
					_cursor->setCurrentFrameIndex(0);
			}
		}
	}
}

void PegasusEngine::toggleInventoryDisplay() {
	if (_gameMode == kModeInventoryPick)
		setGameMode(kModeNavigation);
	else
		setGameMode(kModeInventoryPick);
}

void PegasusEngine::toggleBiochipDisplay() {
	if (_gameMode == kModeBiochipPick)
		setGameMode(kModeNavigation);
	else
		setGameMode(kModeBiochipPick);
}

void PegasusEngine::showInfoScreen() {
	if (g_neighborhood) {
		// Break the input handler chain...
		_savedHandler = InputHandler::getCurrentHandler();
		InputHandler::setInputHandler(this);

		Picture *pushPicture = ((Neighborhood *)g_neighborhood)->getTurnPushPicture();

		_bigInfoMovie.shareSurface(pushPicture);
		_smallInfoMovie.shareSurface(pushPicture);

		g_neighborhood->hideNav();

		_smallInfoMovie.initFromMovieFile("Images/Items/Info Right Movie");
		_smallInfoMovie.setDisplayOrder(kInfoSpinOrder);
		_smallInfoMovie.moveElementTo(kNavAreaLeft + 304, kNavAreaTop + 8);
		_smallInfoMovie.moveMovieBoxTo(304, 8);
		_smallInfoMovie.startDisplaying();
		_smallInfoMovie.show();

		TimeValue startTime, stopTime;
		g_AIArea->getSmallInfoSegment(startTime, stopTime);
		_smallInfoMovie.setSegment(startTime, stopTime);
		_smallInfoMovie.setTime(startTime);
		_smallInfoMovie.setFlags(kLoopTimeBase);

		_bigInfoMovie.initFromMovieFile("Images/Items/Info Left Movie");
		_bigInfoMovie.setDisplayOrder(kInfoBackgroundOrder);
		_bigInfoMovie.moveElementTo(kNavAreaLeft, kNavAreaTop);
		_bigInfoMovie.startDisplaying();
		_bigInfoMovie.show();
		_bigInfoMovie.setTime(g_AIArea->getBigInfoTime());

		_bigInfoMovie.redrawMovieWorld();
		_smallInfoMovie.redrawMovieWorld();
		_smallInfoMovie.start();
	}
}

void PegasusEngine::hideInfoScreen() {
	if (g_neighborhood) {
		InputHandler::setInputHandler(_savedHandler);

		_bigInfoMovie.hide();
		_bigInfoMovie.stopDisplaying();
		_bigInfoMovie.releaseMovie();

		_smallInfoMovie.hide();
		_smallInfoMovie.stopDisplaying();
		_smallInfoMovie.stop();
		_smallInfoMovie.releaseMovie();

		g_neighborhood->showNav();
	}
}

void PegasusEngine::raiseInventoryDrawer() {
	if (g_interface)
		g_interface->raiseInventoryDrawer();
}

void PegasusEngine::raiseBiochipDrawer() {
	if (g_interface)
		g_interface->raiseBiochipDrawer();
}

void PegasusEngine::lowerInventoryDrawer() {
	if (g_interface)
		g_interface->lowerInventoryDrawer();
}

void PegasusEngine::lowerBiochipDrawer() {
	if (g_interface)
		g_interface->lowerBiochipDrawer();
}

void PegasusEngine::raiseInventoryDrawerSync() {
	if (g_interface)
		g_interface->raiseInventoryDrawerSync();
}

void PegasusEngine::raiseBiochipDrawerSync() {
	if (g_interface)
		g_interface->raiseBiochipDrawerSync();
}

void PegasusEngine::lowerInventoryDrawerSync() {
	if (g_interface)
		g_interface->lowerInventoryDrawerSync();
}

void PegasusEngine::lowerBiochipDrawerSync() {
	if (g_interface)
		g_interface->lowerBiochipDrawerSync();
}

void PegasusEngine::toggleInfo() {
	if (_gameMode == kModeInfoScreen)
		setGameMode(kModeNavigation);
	else if (_gameMode == kModeNavigation)
		setGameMode(kModeInfoScreen);
}

void PegasusEngine::dragTerminated(const Input &) {
	Hotspot *finalSpot = _itemDragger.getLastHotspot();
	InventoryResult result;

	if (_dragType == kDragInventoryPickup) {
		if (finalSpot && finalSpot->getObjectID() == kInventoryDropSpotID)
			result = addItemToInventory((InventoryItem *)_draggingItem);
		else
			result = kTooMuchWeight;

		if (result != kInventoryOK)
			autoDragItemIntoRoom(_draggingItem, _draggingSprite);
		else
			delete _draggingSprite;
	} else if (_dragType == kDragBiochipPickup) {
		if (finalSpot && finalSpot->getObjectID() == kBiochipDropSpotID)
			result = addItemToBiochips((BiochipItem *)_draggingItem);
		else
			result = kTooMuchWeight;

		if (result != kInventoryOK)
			autoDragItemIntoRoom(_draggingItem, _draggingSprite);
		else
			delete _draggingSprite;
	} else if (_dragType == kDragInventoryUse) {
		if (finalSpot && (finalSpot->getHotspotFlags() & kDropItemSpotFlag) != 0) {
			// *** Need to decide on a case by case basis what to do here.
			// the crowbar should break the cover off the Mars reactor if its frozen, the
			// global transport card should slide through the slot, the oxygen mask should
			// attach to the filling station, and so on...
			_neighborhood->dropItemIntoRoom(_draggingItem, finalSpot);
			delete _draggingSprite;
		} else {
			autoDragItemIntoInventory(_draggingItem, _draggingSprite);
		}
	}

	_dragType = kDragNoDrag;

	if (g_AIArea)
		g_AIArea->unlockAI();
}


void PegasusEngine::dragItem(const Input &input, Item *item, DragType type) {
	_draggingItem = item;
	_dragType = type;

	// Create the sprite.
	_draggingSprite = _draggingItem->getDragSprite(kDraggingSpriteID);
	Common::Point where;
	input.getInputLocation(where);
	Common::Rect r1;
	_draggingSprite->getBounds(r1);
	r1 = Common::Rect::center(where.x, where.y, r1.width(), r1.height());
	_draggingSprite->setBounds(r1);

	// Set up drag constraints.
	DisplayElement *navMovie = _gfx->findDisplayElement(kNavMovieID);
	Common::Rect r2;
	navMovie->getBounds(r2);
	r2.left -= r1.width() / 3;
	r2.right += r1.width() / 3;
	r2.top -= r1.height() / 3;
	r2.bottom += r2.height() / 3;

	r1 = Common::Rect(-30000, -30000, 30000, 30000);
	_itemDragger.setDragConstraints(r2, r1);

	// Start dragging.
	_draggingSprite->setDisplayOrder(kDragSpriteOrder);
	_draggingSprite->startDisplaying();
	_draggingSprite->show();
	_itemDragger.setDragSprite(_draggingSprite);
	_itemDragger.setNextHandler(_neighborhood);
	_itemDragger.startTracking(input);

	if (g_AIArea)
		g_AIArea->lockAIOut();
}

void PegasusEngine::shellGameInput(const Input &input, const Hotspot *cursorSpot) {
	if (_gameMode == kModeInfoScreen) {
		if (JMPPPInput::isToggleAIMiddleInput(input)) {
			LowerClientSignature middleOwner = g_AIArea->getMiddleAreaOwner();
			g_AIArea->toggleMiddleAreaOwner();

			if (middleOwner != g_AIArea->getMiddleAreaOwner()) {
				_bigInfoMovie.setTime(g_AIArea->getBigInfoTime());
				_smallInfoMovie.stop();
				_smallInfoMovie.setFlags(0);

				TimeValue startTime, stopTime;
				g_AIArea->getSmallInfoSegment(startTime, stopTime);
				_smallInfoMovie.setSegment(startTime, stopTime);
				_smallInfoMovie.setTime(startTime);
				_smallInfoMovie.setFlags(kLoopTimeBase);

				_bigInfoMovie.redrawMovieWorld();
				_smallInfoMovie.redrawMovieWorld();
				_smallInfoMovie.start();
			}
		}
	} else {
		if (JMPPPInput::isRaiseInventoryInput(input))
			toggleInventoryDisplay();

		if (JMPPPInput::isRaiseBiochipsInput(input))
			toggleBiochipDisplay();

		if (JMPPPInput::isTogglePauseInput(input) && _neighborhood)
			pauseMenu(!isPaused());
	}

	if (JMPPPInput::isToggleInfoInput(input))
		toggleInfo();
}

void PegasusEngine::activateHotspots() {
	if (_gameMode == kModeInfoScreen) {
		_allHotspots.activateOneHotspot(kInfoReturnSpotID);
	} else {
		// Set up hot spots.
		if (_dragType == kDragInventoryPickup)
			_allHotspots.activateOneHotspot(kInventoryDropSpotID);
		else if (_dragType == kDragBiochipPickup)
			_allHotspots.activateOneHotspot(kBiochipDropSpotID);
		else if (_dragType == kDragNoDrag)
			_allHotspots.activateMaskedHotspots(kShellSpotFlag);
	}
}

bool PegasusEngine::isClickInput(const Input &input, const Hotspot *cursorSpot) {
	if (_cursor->isVisible() && cursorSpot)
		return JMPPPInput::isClickInput(input);
	else
		return false;
}

InputBits PegasusEngine::getClickFilter() {
	return JMPPPInput::getClickInputFilter();
}

void PegasusEngine::clickInHotspot(const Input &input, const Hotspot *clickedSpot) {
	if (clickedSpot->getObjectID() == kCurrentItemSpotID) {
		InventoryItem *currentItem = getCurrentInventoryItem();
		if (currentItem) {
			removeItemFromInventory(currentItem);
			dragItem(input, currentItem, kDragInventoryUse);
		}
	} else if (clickedSpot->getObjectID() == kInfoReturnSpotID) {
		toggleInfo();
	}
}

InventoryResult PegasusEngine::removeItemFromInventory(InventoryItem *item) {
	InventoryResult result;

	if (g_interface)
		result = g_interface->removeInventoryItem(item);
	else
		result = _items.removeItem(item);

	// This should never happen
	assert(result == kInventoryOK);

	return result;
}

void PegasusEngine::removeAllItemsFromInventory() {
	if (g_interface)
		g_interface->removeAllItemsFromInventory();
	else
		_items.removeAllItems();
}

/////////////////////////////////////////////
//
// Biochip handling.

// Adding biochips to the biochip drawer is a little funny, because of two things:
//      --  We get the map biochip and pegasus biochip at the same time by dragging
//          one sprite in TSA
//      --  We can drag in more than one copy of the various biochips.
// Because of this we need to make sure that no more than one copy of each biochip
// is ever added.

InventoryResult PegasusEngine::addItemToBiochips(BiochipItem *biochip) {
	InventoryResult result;

	if (g_interface)
		result = g_interface->addBiochip(biochip);
	else
		result = _biochips.addItem(biochip);

	// This can never happen
	assert(result == kInventoryOK);

	GameState.setTakenItem(biochip, true);

	if (g_neighborhood)
		g_neighborhood->pickedUpItem(biochip);

	g_AIArea->checkMiddleArea();

	return result;
}

void PegasusEngine::removeAllItemsFromBiochips() {
	if (g_interface)
		g_interface->removeAllItemsFromBiochips();
	else
		_biochips.removeAllItems();
}

void PegasusEngine::setSoundFXLevel(uint16 fxLevel) {
	_FXLevel = fxLevel;
	if (_neighborhood)
		_neighborhood->setSoundFXLevel(fxLevel);
	if (g_AIArea)
		g_AIArea->setAIVolume(fxLevel);
}

void PegasusEngine::setAmbienceLevel(uint16 ambientLevel) {
	_ambientLevel = ambientLevel;
	if (_neighborhood)
		_neighborhood->setAmbienceLevel(ambientLevel);
}

void PegasusEngine::pauseMenu(bool menuUp) {
	if (menuUp) {
		pauseEngine(true);
		_screenDimmer.startDisplaying();
		_screenDimmer.show();
		_gfx->updateDisplay();
		useMenu(new PauseMenu());
	} else {
		pauseEngine(false);
		_screenDimmer.hide();
		_screenDimmer.stopDisplaying();
		useMenu(0);
		g_AIArea->checkMiddleArea();
	}
}

void PegasusEngine::autoDragItemIntoRoom(Item *item, Sprite *draggingSprite) {
	if (g_AIArea)
		g_AIArea->lockAIOut();

	Common::Point start, stop;
	draggingSprite->getLocation(start.x, start.y);

	Hotspot *dropSpot = _neighborhood->getItemScreenSpot(item, draggingSprite);

	if (dropSpot) {
		dropSpot->getCenter(stop.x, stop.y);
	} else {
		stop.x = kNavAreaLeft + 256;
		stop.y = kNavAreaTop + 128;
	}

	Common::Rect bounds;
	draggingSprite->getBounds(bounds);
	stop.x -= bounds.width() >> 1;
	stop.y -= bounds.height() >> 1;

	int dx = ABS(stop.x - start.x);
	int dy = ABS(stop.y - start.y);
	TimeValue time = MAX(dx, dy);

	allowInput(false);
	_autoDragger.autoDrag(draggingSprite, start, stop, time, kDefaultTimeScale);

	while (_autoDragger.isDragging()) {
		checkCallBacks();
		refreshDisplay();
		_system->delayMillis(10);
	}

	_neighborhood->dropItemIntoRoom(_draggingItem, dropSpot);
	allowInput(true);
	delete _draggingSprite;

	if (g_AIArea)
		g_AIArea->unlockAI();
}

void PegasusEngine::autoDragItemIntoInventory(Item *, Sprite *draggingSprite) {
	if (g_AIArea)
		g_AIArea->lockAIOut();

	Common::Point start;
	draggingSprite->getLocation(start.x, start.y);

	Common::Rect r;
	draggingSprite->getBounds(r);

	Common::Point stop((76 + 172 - r.width()) / 2, 334 - (2 * r.height() / 3));

	int dx = ABS(stop.x - start.x);
	int dy = ABS(stop.y - start.y);
	TimeValue time = MAX(dx, dy);

	allowInput(false);
	_autoDragger.autoDrag(draggingSprite, start, stop, time, kDefaultTimeScale);

	while (_autoDragger.isDragging()) {
		checkCallBacks();
		refreshDisplay();
		_system->delayMillis(10);
	}

	addItemToInventory((InventoryItem *)_draggingItem);
	allowInput(true);
	delete _draggingSprite;

	if (g_AIArea)
		g_AIArea->unlockAI();
}

NeighborhoodID PegasusEngine::getCurrentNeighborhoodID() const {
	if (_neighborhood)
		return _neighborhood->getObjectID();

	return kNoNeighborhoodID;
}

void PegasusEngine::pauseEngineIntern(bool pause) {
	Engine::pauseEngineIntern(pause);

	if (pause) {
		for (Common::List<TimeBase *>::iterator it = _timeBases.begin(); it != _timeBases.end(); it++)
			(*it)->pause();
	} else {
		for (Common::List<TimeBase *>::iterator it = _timeBases.begin(); it != _timeBases.end(); it++)
			(*it)->resume();
	}
}

uint PegasusEngine::getRandomBit() {
	return _rnd->getRandomBit();
}

uint PegasusEngine::getRandomNumber(uint max) {
	return _rnd->getRandomNumber(max);
}

void PegasusEngine::shuffleArray(int32 *arr, int32 count) {
	if (count > 1) {
		for (int32 i = 1; i < count; ++i) {
			int32 j = _rnd->getRandomNumber(i);
			if (j != i)
				SWAP(arr[i], arr[j]);
		}
	}
}

void PegasusEngine::playEndMessage() {
	if (g_interface) {
		allowInput(false);
		g_interface->playEndMessage();
		allowInput(true);
	}

	die(kPlayerWonGame);
}

void PegasusEngine::doSubChase() {
	Video::VideoDecoder *video = new Video::QuickTimeDecoder();
	if (!video->loadFile("Images/Norad Alpha/Sub Chase Movie"))
		error("Failed to load sub chase");

	video->setEndTime(Audio::Timestamp(0, 133200, 600));
	video->start();

	while (!shouldQuit() && !video->endOfVideo()) {
		if (video->needsUpdate()) {
			const Graphics::Surface *frame = video->decodeNextFrame();

			if (frame)
				drawScaledFrame(frame, 0, 0);
		}

		Common::Event event;
		while (_eventMan->pollEvent(event))
			;

		_system->delayMillis(10);
	}

	delete video;
}

template<typename PixelInt>
static void scaleFrame(const PixelInt *src, PixelInt *dst, int w, int h, int srcPitch) {
	assert((srcPitch % sizeof(PixelInt)) == 0); // sanity check; allows some simpler code

	PixelInt *dst1 = dst;
	PixelInt *dst2 = dst + w * 2;

	int srcInc = (srcPitch / sizeof(PixelInt)) - w;
	int dstInc = w * 2;

	while (h--) {
		for (int x = 0; x < w; x++) {
			PixelInt pixel = *src++;
			*dst1++ = pixel;
			*dst1++ = pixel;
			*dst2++ = pixel;
			*dst2++ = pixel;
		}

		src += srcInc;
		dst1 += dstInc;
		dst2 += dstInc;
	}
}

void PegasusEngine::drawScaledFrame(const Graphics::Surface *frame, uint16 x, uint16 y) {
	// Scale up the frame doing some simple scaling
	Graphics::Surface scaledFrame;
	scaledFrame.create(frame->w * 2, frame->h * 2, frame->format);

	if (frame->format.bytesPerPixel == 2)
		scaleFrame<uint16>((uint16 *)frame->pixels, (uint16 *)scaledFrame.pixels, frame->w, frame->h, frame->pitch);
	else
		scaleFrame<uint32>((uint32 *)frame->pixels, (uint32 *)scaledFrame.pixels, frame->w, frame->h, frame->pitch);

	_system->copyRectToScreen((byte *)scaledFrame.pixels, scaledFrame.pitch, x, y, scaledFrame.w, scaledFrame.h);
	_system->updateScreen();
	scaledFrame.free();
}

void PegasusEngine::destroyInventoryItem(const ItemID itemID) {
	InventoryItem *item = (InventoryItem *)_allItems.findItemByID(itemID);

	ItemExtraEntry entry;

	switch (itemID) {
	case kAirMask:
		item->findItemExtra(kRemoveAirMask, entry);
		item->setItemRoom(kMarsID, kMars49, kSouth);
		break;
	case kArgonCanister:
		item->findItemExtra(kRemoveArgon, entry);
		item->setItemRoom(kWSCID, kWSC02Morph, kSouth);
		break;
	case kCrowbar:
		item->findItemExtra(kRemoveCrowbar, entry);
		item->setItemRoom(kMarsID, kMars34, kSouth);
		break;
	case kJourneymanKey:
		item->findItemExtra(kRemoveJourneymanKey, entry);
		item->setItemRoom(kFullTSAID, kTSA22Red, kEast);
		break;
	case kMarsCard:
		item->findItemExtra(kRemoveMarsCard, entry);
		item->setItemRoom(kMarsID, kMars31South, kSouth);
		break;
	case kNitrogenCanister:
		item->findItemExtra(kRemoveNitrogen, entry);
		item->setItemRoom(kWSCID, kWSC02Messages, kSouth);
		break;
	case kOrangeJuiceGlassEmpty:
		item->findItemExtra(kRemoveGlass, entry);
		item->setItemRoom(kCaldoriaID, kCaldoriaReplicator, kNorth);
		break;
	case kPoisonDart:
		item->findItemExtra(kRemoveDart, entry);
		item->setItemRoom(kWSCID, kWSC01, kWest);
		break;
	case kSinclairKey:
		item->findItemExtra(kRemoveSinclairKey, entry);
		item->setItemRoom(kWSCID, kWSC02Morph, kSouth);
		break;
	default:
		return;
	}

	g_interface->setCurrentInventoryItemID(itemID);
	g_AIArea->playAIAreaSequence(kInventorySignature, kMiddleAreaSignature, entry.extraStart, entry.extraStop);
	removeItemFromInventory(item);
}

ItemID PegasusEngine::pickItemToDestroy() {
/*
	Must pick an item to destroy

	Part I: Polite -- try to find an item that's been used
	Part II: Desperate -- return the first available item.
*/

	// Polite:
	if (playerHasItemID(kOrangeJuiceGlassEmpty))
		return kOrangeJuiceGlassEmpty;
	if (playerHasItemID(kPoisonDart)) {
		if (GameState.getCurrentNeighborhood() != kWSCID ||
				GameState.getWSCAnalyzedDart())
			return kPoisonDart;
	}
	if (playerHasItemID(kJourneymanKey)) {
		if (GameState.getTSAState() >= kTSAPlayerGotHistoricalLog &&
				GameState.getTSAState() != kPlayerOnWayToPrehistoric &&
				GameState.getTSAState() != kPlayerWentToPrehistoric)
			return kJourneymanKey;
	}
	if (playerHasItemID(kMarsCard)) {
		if (GameState.getCurrentNeighborhood() != kMarsID || GameState.getMarsArrivedBelow())
			return kMarsCard;
	}

	// Don't want to deal with deleting the sinclair key and argon canister, since it's
	// impossible to pick them up one at a time.

	if (playerHasItemID(kNitrogenCanister)) {
		if (GameState.getScoringGotCardBomb() && GameState.getCurrentNeighborhood() != kMarsID)
			return kNitrogenCanister;
	}
	if (playerHasItemID(kCrowbar)) {
		if (GameState.getCurrentNeighborhood() == kWSCID) {
			if (GameState.getCurrentRoom() >= kWSC62)
				return kCrowbar;
		} else if (GameState.getCurrentNeighborhood() == kMarsID) {
			if (GameState.getScoringGotCardBomb())
				return kCrowbar;
		} else
			return kCrowbar;
	}
	if (playerHasItemID(kAirMask)) {
		if (GameState.getCurrentNeighborhood() == kMarsID) {
			if (g_neighborhood->getAirQuality(GameState.getCurrentRoom()) == kAirQualityGood)
				return kAirMask;
		} else if (GameState.getCurrentNeighborhood() != kNoradAlphaID &&
				GameState.getCurrentNeighborhood() != kNoradDeltaID) {
			return kAirMask;
		}
	}

	// Desperate:
	if (playerHasItemID(kPoisonDart))
		return kPoisonDart;
	if (playerHasItemID(kJourneymanKey))
		return kJourneymanKey;
	if (playerHasItemID(kMarsCard))
		return kMarsCard;
	if (playerHasItemID(kNitrogenCanister))
		return kNitrogenCanister;
	if (playerHasItemID(kCrowbar))
		return kCrowbar;
	if (playerHasItemID(kAirMask))
		return kAirMask;

	// Should never get this far...
	error("Could not find item to delete");

	return kNoItemID;
}

uint PegasusEngine::getNeighborhoodCD(const NeighborhoodID neighborhood) const {
	switch (neighborhood) {
	case kCaldoriaID:
	case kNoradAlphaID:
	case kNoradSubChaseID:
		return 1;
	case kFullTSAID:
	case kPrehistoricID:
		return 2;
	case kMarsID:
		return 3;
	case kWSCID:
	case kNoradDeltaID:
		return 4;
	case kTinyTSAID:
		// Tiny TSA exists on three of the CD's, so just continue
		// with the CD we're on
		return _currentCD;
	}

	// Can't really happen, but it's a good fallback anyway :P
	return 1;
}

void PegasusEngine::initKeymap() {
#ifdef ENABLE_KEYMAPPER
	static const char *const kKeymapName = "pegasus";
	Common::Keymapper *const mapper = _eventMan->getKeymapper();

	// Do not try to recreate same keymap over again
	if (mapper->getKeymap(kKeymapName) != 0)
		return;

	Common::Keymap *const engineKeyMap = new Common::Keymap(kKeymapName);

	// Since the game has multiple built-in keys for each of these anyway,
	// this just attempts to remap one of them.
	const Common::KeyActionEntry keyActionEntries[] = {
		{ Common::KEYCODE_UP, "UP", _("Up/Forward") },
		{ Common::KEYCODE_DOWN, "DWN", _("Down/Backward") },
		{ Common::KEYCODE_LEFT, "TL", _("Turn Left") },
		{ Common::KEYCODE_RIGHT, "TR", _("Turn Right") },
		{ Common::KEYCODE_BACKQUOTE, "TIV", _("Toggle Inventory Tray") },
		{ Common::KEYCODE_BACKSPACE, "TBI", _("Toggle Biochip Tray") },
		{ Common::KEYCODE_RETURN, "ENT", _("Enter/Select") },
		{ Common::KEYCODE_t, "TMA", _("Toggle Middle Area") },
		{ Common::KEYCODE_i, "TIN", _("Toggle Info") },
		{ Common::KEYCODE_ESCAPE, "PM", _("Activate Pause Menu") },
		{ Common::KEYCODE_e, "WTF", _("???") } // easter egg key (without being completely upfront about it)
	};

	for (uint i = 0; i < ARRAYSIZE(keyActionEntries); i++) {
		Common::Action *const act = new Common::Action(engineKeyMap, keyActionEntries[i].id, keyActionEntries[i].description);
		act->addKeyEvent(keyActionEntries[i].ks);
	}

	mapper->addGameKeymap(engineKeyMap);
	mapper->pushKeymap(kKeymapName, true);
#endif
}

} // End of namespace Pegasus

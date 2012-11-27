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

#ifndef PEGASUS_H
#define PEGASUS_H

#include "common/list.h"
#include "common/macresman.h"
#include "common/rect.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "common/util.h"

#include "engines/engine.h"

#include "pegasus/graphics.h"
#include "pegasus/hotspot.h"
#include "pegasus/input.h"
#include "pegasus/notification.h"
#include "pegasus/timers.h"
#include "pegasus/items/autodragger.h"
#include "pegasus/items/inventory.h"
#include "pegasus/items/itemdragger.h"
#include "pegasus/neighborhood/neighborhood.h"

namespace Common {
	class RandomSource;
}

namespace Video {
	class VideoDecoder;
}

namespace Pegasus {

class PegasusConsole;
struct PegasusGameDescription;
class SoundManager;
class GraphicsManager;
class Idler;
class Cursor;
class TimeBase;
class GameMenu;
class InventoryItem;
class BiochipItem;
class Neighborhood;

class PegasusEngine : public ::Engine, public InputHandler, public NotificationManager {
friend class InputHandler;

public:
	PegasusEngine(OSystem *syst, const PegasusGameDescription *gamedesc);
	virtual ~PegasusEngine();

	// Engine stuff
	const PegasusGameDescription *_gameDescription;
	bool hasFeature(EngineFeature f) const;
	GUI::Debugger *getDebugger();
	bool canLoadGameStateCurrently();
	bool canSaveGameStateCurrently();
	Common::Error loadGameState(int slot);
	Common::Error saveGameState(int slot, const Common::String &desc);

	// Base classes
	GraphicsManager *_gfx;
	Common::MacResManager *_resFork;
	Cursor *_cursor;

	// Menu
	void useMenu(GameMenu *menu);
	bool checkGameMenu();

	// Misc.
	bool isDemo() const;
	void addIdler(Idler *idler);
	void removeIdler(Idler *idler);
	void addTimeBase(TimeBase *timeBase);
	void removeTimeBase(TimeBase *timeBase);
	void delayShell(TimeValue time, TimeScale scale);
	void resetIntroTimer();
	void introTimerExpired();
	void refreshDisplay();
	bool playerAlive();
	void processShell();
	void checkCallBacks();
	void createInterface();
	void setGameMode(const GameMode);
	GameMode getGameMode() const { return _gameMode; }
	uint getRandomBit();
	uint getRandomNumber(uint max);
	void shuffleArray(int32 *arr, int32 count);
	void drawScaledFrame(const Graphics::Surface *frame, uint16 x, uint16 y);
	HotspotList &getAllHotspots() { return _allHotspots; }

	// Energy
	void setLastEnergyValue(const int32 value) { _savedEnergyValue = value; }
	int32 getSavedEnergyValue() { return _savedEnergyValue; }

	// Death
	void setEnergyDeathReason(const DeathReason reason) { _deathReason = reason; }
	DeathReason getEnergyDeathReason() { return _deathReason; }
	void resetEnergyDeathReason();
	void die(const DeathReason);
	void playEndMessage();

	// Volume
	uint16 getSoundFXLevel() { return _FXLevel; }
	void setSoundFXLevel(uint16);
	uint16 getAmbienceLevel() { return _ambientLevel; }
	void setAmbienceLevel(uint16);

	// Items
	ItemList &getAllItems() { return _allItems; }
	bool playerHasItem(const Item *);
	bool playerHasItemID(const ItemID);
	void checkFlashlight();
	bool itemInLocation(const ItemID, const NeighborhoodID, const RoomID, const DirectionConstant);

	// Inventory Items
	InventoryItem *getCurrentInventoryItem();
	bool itemInInventory(InventoryItem *);
	bool itemInInventory(ItemID);
	Inventory *getItemsInventory() { return &_items; }
	InventoryResult addItemToInventory(InventoryItem *);
	void removeAllItemsFromInventory();
	InventoryResult removeItemFromInventory(InventoryItem *);
	uint32 countInventoryItems() { return _items.getNumItems(); }

	// Biochips
	BiochipItem *getCurrentBiochip();
	bool itemInBiochips(BiochipItem *);
	bool itemInBiochips(ItemID);
	Inventory *getBiochipsInventory() { return &_biochips; }
	void removeAllItemsFromBiochips();
	InventoryResult addItemToBiochips(BiochipItem *);

	// AI
	Common::String getBriefingMovie();
	Common::String getEnvScanMovie();
	uint getNumHints();
	Common::String getHintMovie(uint);
	bool canSolve();
	void prepareForAIHint(const Common::String &);
	void cleanUpAfterAIHint(const Common::String &);
	Common::SeekableReadStream *_aiSaveStream;

	// Neighborhood
	void jumpToNewEnvironment(const NeighborhoodID, const RoomID, const DirectionConstant);
	NeighborhoodID getCurrentNeighborhoodID() const;

	// Dragging
	void dragItem(const Input &, Item *, DragType);
	bool isDragging() const { return _dragType != kDragNoDrag; }
	DragType getDragType() const { return _dragType; }
	Item *getDraggingItem() const { return _draggingItem; }
	void dragTerminated(const Input &);
	void autoDragItemIntoRoom(Item *, Sprite *);
	void autoDragItemIntoInventory(Item *, Sprite*);

	// Save/Load
	void makeContinuePoint();
	bool swapSaveAllowed(bool allow) {
		bool old = _saveAllowed;
		_saveAllowed = allow;
		return old;
	}
	bool swapLoadAllowed(bool allow) {
		bool old = _loadAllowed;
		_loadAllowed = allow;
		return old;
	}
	void requestSave() { _saveRequested = true; }
	bool saveRequested() const { return _saveRequested; }
	void requestLoad() { _loadRequested = true; }
	bool loadRequested() const { return _loadRequested; }

protected:
	Common::Error run();
	void pauseEngineIntern(bool pause);

	Notification _shellNotification;
	virtual void receiveNotification(Notification *notification, const NotificationFlags flags);

	void handleInput(const Input &input, const Hotspot *cursorSpot);
	virtual bool isClickInput(const Input &, const Hotspot *);
	virtual InputBits getClickFilter();

	void clickInHotspot(const Input &, const Hotspot *);
	void activateHotspots(void);

	void updateCursor(const Common::Point, const Hotspot *);
	bool wantsCursor();

private:
	// Console
	PegasusConsole *_console;

	// Intro
	void runIntro();
	void stopIntroTimer();
	bool detectOpeningClosingDirectory();
	Common::String _introDirectory;
	FuseFunction *_introTimer;

	// Idlers
	Idler *_idlerHead;
	void giveIdleTime();

	// Items
	ItemList _allItems;
	void createItems();
	void createItem(ItemID itemID, NeighborhoodID neighborhoodID, RoomID roomID, DirectionConstant direction);
	Inventory _items;
	Inventory _biochips;
	ItemID _currentItemID;
	ItemID _currentBiochipID;
	void destroyInventoryItem(const ItemID itemID);
	ItemID pickItemToDestroy();

	// TimeBases
	Common::List<TimeBase *> _timeBases;

	// Save/Load
	bool loadFromStream(Common::ReadStream *stream);
	bool writeToStream(Common::WriteStream *stream, int saveType);
	void loadFromContinuePoint();
	void writeContinueStream(Common::WriteStream *stream);
	Common::SeekableReadStream *_continuePoint;
	bool _saveAllowed, _loadAllowed; // It's so nice that this was in the original code already :P
	Common::Error showLoadDialog();
	Common::Error showSaveDialog();
	bool _saveRequested, _loadRequested;

	// Misc.
	Hotspot _returnHotspot;
	HotspotList _allHotspots;
	InputHandler *_savedHandler;
	void showTempScreen(const Common::String &fileName);
	bool playMovieScaled(Video::VideoDecoder *video, uint16 x, uint16 y);
	void throwAwayEverything();
	void shellGameInput(const Input &input, const Hotspot *cursorSpot);
	Common::RandomSource *_rnd;
	void doSubChase();
	uint getNeighborhoodCD(const NeighborhoodID neighborhood) const;
	uint _currentCD;
	void initKeymap();

	// Menu
	GameMenu *_gameMenu;
	void doGameMenuCommand(const GameMenuCommand);
	void doInterfaceOverview();
	ScreenDimmer _screenDimmer;
	void pauseMenu(bool menuUp);

	// Energy
	int32 _savedEnergyValue;

	// Death
	DeathReason _deathReason;
	void doDeath();

	// Neighborhood
	Neighborhood *_neighborhood;
	void useNeighborhood(Neighborhood *neighborhood);
	void performJump(NeighborhoodID start);
	void startNewGame();
	void startNeighborhood();
	void makeNeighborhood(NeighborhoodID, Neighborhood *&);

	// Sound
	uint16 _ambientLevel;
	uint16 _FXLevel;

	// Game Mode
	GameMode _gameMode;
	bool _switchModesSync;
	void switchGameMode(const GameMode, const GameMode);
	bool canSwitchGameMode(const GameMode, const GameMode);

	// Dragging
	ItemDragger _itemDragger;
	Item *_draggingItem;
	Sprite *_draggingSprite;
	DragType _dragType;
	AutoDragger _autoDragger;

	// Interface
	void toggleInventoryDisplay();
	void toggleBiochipDisplay();
	void raiseInventoryDrawer();
	void raiseBiochipDrawer();
	void lowerInventoryDrawer();
	void lowerBiochipDrawer();
	void raiseInventoryDrawerSync();
	void raiseBiochipDrawerSync();
	void lowerInventoryDrawerSync();
	void lowerBiochipDrawerSync();
	void showInfoScreen();
	void hideInfoScreen();
	void toggleInfo();
	Movie _bigInfoMovie, _smallInfoMovie;
};

} // End of namespace Pegasus

#endif

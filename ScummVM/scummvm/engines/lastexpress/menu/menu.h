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

#ifndef LASTEXPRESS_MENU_H
#define LASTEXPRESS_MENU_H

#include "lastexpress/data/sequence.h"

#include "lastexpress/eventhandler.h"

#include "lastexpress/shared.h"

#include "common/hashmap.h"

namespace LastExpress {

class LastExpressEngine;
class Scene;
class SceneHotspot;

class Clock;
class TrainLine;

class Menu : public EventHandler {
public:
	Menu(LastExpressEngine *engine);
	~Menu();

	void show(bool doSavegame, SavegameType type, uint32 value);

	// Event handling
	void eventMouse(const Common::Event &ev);
	void eventTick(const Common::Event &ev);

	bool isShown() const { return _isShowingMenu; }

	GameId getGameId() const { return _gameId; }

private:
	// Start menu events
	enum StartMenuAction {
		kMenuContinue = 1,
		kMenuCredits = 2,
		kMenuQuitGame = 3,
		kMenuCase4 = 4,
		kMenuSwitchSaveGame = 6,
		kMenuRewindGame = 7,
		kMenuForwardGame = 8,
		kMenuParis = 10,
		kMenuStrasBourg = 11,
		kMenuMunich = 12,
		kMenuVienna = 13,
		kMenuBudapest = 14,
		kMenuBelgrade = 15,
		kMenuConstantinople = 16,
		kMenuDecreaseVolume = 17,
		kMenuIncreaseVolume = 18,
		kMenuDecreaseBrightness = 19,
		kMenuIncreaseBrightness = 20
	};

	// City buttons
	enum CityButton {
		kParis = 0,
		kStrasbourg = 1,
		kMunich = 2,
		kVienna = 3,
		kBudapest = 4,
		kBelgrade = 5,
		kConstantinople = 6
	};

	// Start menu overlay elements
	enum StartMenuOverlay {
		kOverlayTooltip,            // 0
		kOverlayEggButtons,
		kOverlayButtons,
		kOverlayAcorn,
		kOverlayCity1,
		kOverlayCity2,              // 5
		kOverlayCity3,
		kOverlayCredits
	};

	LastExpressEngine *_engine;

	// Sequences
	Sequence *_seqTooltips;
	Sequence *_seqEggButtons;
	Sequence *_seqButtons;
	Sequence *_seqAcorn;
	Sequence *_seqCity1;
	Sequence *_seqCity2;
	Sequence *_seqCity3;
	Sequence *_seqCredits;

	GameId _gameId;

	// Indicator to know if we need to show the start animation when showMenu is called
	bool _hasShownStartScreen;
	bool _hasShownIntro;

	bool _isShowingCredits;
	bool _isGameStarted;
	bool _isShowingMenu;


	uint16 _creditsSequenceIndex;

	//////////////////////////////////////////////////////////////////////////
	// Event handling
	uint32 _checkHotspotsTicks;
	Common::EventType _mouseFlags;
	SceneHotspot *_lastHotspot;

	void init(bool doSavegame, SavegameType type, uint32 value);
	void setup();
	bool handleEvent(StartMenuAction action, Common::EventType type);
	void checkHotspots();
	void setLogicEventHandlers();

	//////////////////////////////////////////////////////////////////////////
	// Game-related
	void startGame();
	void switchGame();

	//////////////////////////////////////////////////////////////////////////
	// Overlays & elements
	Clock *_clock;
	TrainLine *_trainLine;

	struct MenuOverlays_EqualTo {
		bool operator()(const StartMenuOverlay &x, const StartMenuOverlay &y) const { return x == y; }
	};

	struct MenuOverlays_Hash {
		uint operator()(const StartMenuOverlay &x) const { return x; }
	};

	typedef Common::HashMap<StartMenuOverlay, SequenceFrame *, MenuOverlays_Hash, MenuOverlays_EqualTo> MenuFrames;

	MenuFrames _frames;

	void hideOverlays();
	void showFrame(StartMenuOverlay overlay, int index, bool redraw);

	void clear();

	// TODO: remove?
	void moveToCity(CityButton city, bool clicked);

	//////////////////////////////////////////////////////////////////////////
	// Misc
	Common::String getAcornSequenceName(GameId id) const;

	//////////////////////////////////////////////////////////////////////////
	// Time
	uint32 _currentTime;  // current game time
	uint32 _lowerTime;    // lower time value
	uint32 _time;

	uint32 _currentIndex; // current savegame entry
	uint32 _index;
	uint32 _lastIndex;
	uint32 _delta;
	bool _handleTimeDelta;

	void initTime(SavegameType type, uint32 val);
	void updateTime(uint32 time);
	void adjustTime();
	void adjustIndex(uint32 time1, uint32 time2, bool searchEntry);
	void goToTime(uint32 time);
	void setTime();
	void forwardTime();
	void rewindTime();
	bool hasTimeDelta() { return (_currentTime - _time) >= 1; }

	//////////////////////////////////////////////////////////////////////////
	// Sound/Brightness related
	uint32 getVolume() const;
	void setVolume(uint32 volume) const;
	uint32 getBrightness() const;
	void setBrightness(uint32 brightness) const;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_MENU_H

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

#ifndef PEGASUS_MENU_H
#define PEGASUS_MENU_H

#include "pegasus/constants.h"
#include "pegasus/fader.h"
#include "pegasus/input.h"
#include "pegasus/movie.h"
#include "pegasus/sound.h"
#include "pegasus/surface.h"
#include "pegasus/util.h"

namespace Pegasus {

class GameMenu : public IDObject, public InputHandler {
public:
	GameMenu(const uint32);
	virtual ~GameMenu() {}

	virtual void becomeCurrentHandler();
	virtual void restorePreviousHandler();

	GameMenuCommand getLastCommand() { return _lastCommand; }
	void clearLastCommand() { _lastCommand = kMenuCmdNoCommand; }

protected:
	void setLastCommand(const GameMenuCommand command) { _lastCommand = command; }

	InputHandler *_previousHandler;
	GameMenuCommand _lastCommand;

	void drawScore(GameScoreType, GameScoreType, const Common::Rect &, Surface *);

private:
	void drawNumber(GameScoreType, CoordType &, CoordType, Surface *);
};

class Hotspot;

class MainMenu : public GameMenu {
public:
	MainMenu();
	virtual ~MainMenu();

	virtual void handleInput(const Input &input, const Hotspot *);
	void startMainMenuLoop();
	void stopMainMenuLoop();

protected:
	void updateDisplay();

	uint32 _menuSelection;

	// Full and Demo
	Picture _menuBackground;
	Picture _startButton;
	Picture _creditsButton;
	Picture _quitButton;
	Picture _largeSelect;
	Picture _smallSelect;

	// Full only
	bool _adventureMode;
	Picture _overviewButton;
	Picture _restoreButton;
	Picture _adventureButton;
	Picture _walkthroughButton;

	Sound _menuLoop;
	SoundFader _menuFader;
};

class CreditsMenu : public GameMenu {
public:
	CreditsMenu();
	virtual ~CreditsMenu() {}

	virtual void handleInput(const Input &input, const Hotspot *);

protected:
	void newMenuSelection(const int);
	void newMovieTime(const TimeValue);

	int _menuSelection;
	Picture _menuBackground;
	Movie _creditsMovie;
	Picture _mainMenuButton;
	Picture _largeSelect;
	Picture _smallSelect;
};

class DeathMenu : public GameMenu {
public:
	DeathMenu(const DeathReason);
	virtual ~DeathMenu() {}

	virtual void handleInput(const Input &input, const Hotspot *);

	bool playerWon() { return _playerWon; }

protected:
	void drawAllScores();

	void updateDisplay();

	bool _playerWon;
	int _menuSelection;
	DeathReason _deathReason;

	Picture _deathBackground;
	Picture _continueButton;
	Picture _restoreButton;
	Picture _mainMenuButton;
	Picture _quitButton;

	Picture _largeSelect;
	Picture _smallSelect;

	Sound _triumphSound;
};

class PauseMenu : public GameMenu {
public:
	PauseMenu();
	virtual ~PauseMenu() {}

	virtual void handleInput(const Input &input, const Hotspot *);

protected:
	void updateDisplay();

	uint32 _menuSelection;
	Picture _pauseBackground;
	Picture _saveButton;
	Picture _restoreButton;
	Picture _walkthroughButton;
	Picture _continueButton;
	SoundLevel _soundFXLevel;
	SoundLevel _ambienceLevel;
	Picture _quitButton;
	Picture _largeSelect;
	Picture _smallSelect;
};

} // End of namespace Pegasus

#endif

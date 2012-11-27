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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef ENGINES_ENGINE_H
#define ENGINES_ENGINE_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/language.h"
#include "common/platform.h"

class OSystem;

namespace Audio {
class Mixer;
}
namespace Common {
class Error;
class EventManager;
class SaveFileManager;
class TimerManager;
}
namespace GUI {
class Debugger;
class Dialog;
}

/**
 * Initializes graphics and shows error message.
 */
void GUIErrorMessage(const Common::String &msg);


class Engine {
public:
	OSystem *_system;
	Audio::Mixer *_mixer;

protected:
	Common::TimerManager *_timer;
	Common::EventManager *_eventMan;
	Common::SaveFileManager *_saveFileMan;

	GUI::Dialog *_mainMenuDialog;
	virtual int runDialog(GUI::Dialog &dialog);

	const Common::String _targetName; // target name for saves

private:
	/**
	 * The pause level, 0 means 'running', a positive value indicates
	 * how often the engine has been paused (and hence how often it has
	 * to be un-paused before it resumes running). This makes it possible
	 * to nest code which pauses the engine.
	 */
	int _pauseLevel;

	/**
	 * The time when the pause was started.
	 */
	uint32 _pauseStartTime;

	/**
	 * The time when the engine was started. This value is used to calculate
	 * the current play time of the game running.
	 */
	int32 _engineStartTime;

	/**
	 * Save slot selected via global main menu.
	 * This slot will be loaded after main menu execution (not from inside
	 * the menu loop, to avoid bugs like #2822778).
	 */
	int _saveSlotToLoad;

public:


	/**
	 * A feature in this context means an ability of the engine which can be
	 * either available or not.
	 * @see Engine::hasFeature()
	 */
	enum EngineFeature {
		/**
		 * Enables the subtitle speed and toggle items in the Options section
		 * of the global main menu.
		 */
		kSupportsSubtitleOptions,

		/**
		 * 'Return to launcher' feature is supported, i.e., EVENT_RTL is handled
		 * either directly, or indirectly (that is, the engine calls and honors
		 * the result of the Engine::shouldQuit() method appropriately).
		 */
		kSupportsRTL,

		/**
		 * Loading savestates during runtime is supported, that is, this engine
		 * implements loadGameState() and canLoadGameStateCurrently().
		 * If this feature is supported, then the corresponding MetaEngine *must*
		 * support the kSupportsListSaves feature.
		 */
		kSupportsLoadingDuringRuntime,

		/**
		 * Loading savestates during runtime is supported, that is, this engine
		 * implements saveGameState() and canSaveGameStateCurrently().
		 * If this feature is supported, then the corresponding MetaEngine *must*
		 * support the kSupportsListSaves feature.
		 */
		kSupportsSavingDuringRuntime
	};



	/** @name Overloadable methods
	 *
	 *  All Engine subclasses should consider overloading some or all of the following methods.
	 */
	//@{

	Engine(OSystem *syst);
	virtual ~Engine();

	/**
	 * Init the engine and start its main loop.
	 * @return returns kNoError on success, else an error code.
	 */
	virtual Common::Error run() = 0;

	/**
	 * Prepare an error string, which is printed by the error() function.
	 */
	virtual void errorString(const char *buf_input, char *buf_output, int buf_output_size);

	/**
	 * Return the engine's debugger instance, if any. Used by error() to
	 * invoke the debugger when a severe error is reported.
	 */
	virtual GUI::Debugger *getDebugger() { return 0; }

	/**
	 * Determine whether the engine supports the specified feature.
	 */
	virtual bool hasFeature(EngineFeature f) const { return false; }

//	virtual EnginePlugin *getMetaEnginePlugin() const;

	/**
	 * Notify the engine that the sound settings in the config manager may have
	 * changed and that it hence should adjust any internal volume etc. values
	 * accordingly.
	 * The default implementation sets the volume levels of all mixer sound
	 * types according to the config entries of the active domain.
	 * When overwriting, call the default implementation first, then adjust the
	 * volumes further (if required).
	 *
	 * @note When setting volume levels, respect the "mute" config entry.
	 * @note The volume for the plain sound type is reset to the maximum
	 *       volume. If the engine can associate its own value for this
	 *       type, it needs to overwrite this member and set it accordingly.
	 * @todo find a better name for this
	 */
	virtual void syncSoundSettings();

	/*
	 * Initialize any engine-specific keymaps.
	 */
	virtual void initKeymap() {}

	/*
	 * Cleanup any engine-specific keymaps.
	 */
	virtual void deinitKeymap();

	/**
	 * Flip mute all sound option.
	 */
	virtual void flipMute();

	/**
	 * Load a game state.
	 * @param slot	the slot from which a savestate should be loaded
	 * @return returns kNoError on success, else an error code.
	 */
	virtual Common::Error loadGameState(int slot);

	/**
	 * Sets the game slot for a savegame to be loaded after global
	 * main menu execution. This is to avoid loading a savegame from
	 * inside the menu loop which causes bugs like #2822778.
	 *
	 * @param slot	the slot from which a savestate should be loaded.
	 */
	void setGameToLoadSlot(int slot);

	/**
	 * Indicates whether a game state can be loaded.
	 */
	virtual bool canLoadGameStateCurrently();

	/**
	 * Save a game state.
	 * @param slot	the slot into which the savestate should be stored
	 * @param desc	a description for the savestate, entered by the user
	 * @return returns kNoError on success, else an error code.
	 */
	virtual Common::Error saveGameState(int slot, const Common::String &desc);

	/**
	 * Indicates whether a game state can be saved.
	 */
	virtual bool canSaveGameStateCurrently();

protected:

	/**
	 * Actual implementation of pauseEngine by subclasses. See there
	 * for details.
	 */
	virtual void pauseEngineIntern(bool pause);

	//@}


public:

	/**
	 * Request the engine to quit. Sends a EVENT_QUIT event to the Event
	 * Manager.
	 */
	static void quitGame();

	/**
	 * Return whether the ENGINE should quit respectively should return to the
	 * launcher.
	 */
	static bool shouldQuit();

	/**
	 * Pause or resume the engine. This should stop/resume any audio playback
	 * and other stuff. Called right before the system runs a global dialog
	 * (like a global pause, main menu, options or 'confirm exit' dialog).
	 *
	 * This is a convenience tracker which automatically keeps track on how
	 * often the engine has been paused, ensuring that after pausing an engine
	 * e.g. twice, it has to be unpaused twice before actuallying resuming.
	 *
	 * @param pause		true to pause the engine, false to resume it
	 */
	void pauseEngine(bool pause);

	/**
	 * Return whether the engine is currently paused or not.
	 */
	bool isPaused() const { return _pauseLevel != 0; }

	/**
	 * Run the Global Main Menu Dialog
	 */
	void openMainMenuDialog();

	/**
	 * Display a warning to the user that the game is not fully supported.
	 *
	 * @return true if the user chose to start anyway, false otherwise
	 */
	static bool warnUserAboutUnsupportedGame();

	/**
	 * Get the total play time.
	 *
	 * @return How long the player has been playing in ms.
	 */
	uint32 getTotalPlayTime() const;

	/**
	 * Set the game time counter to the specified time.
	 *
	 * This can be used to set the play time counter after loading a savegame
	 * for example. Another use case is in case the engine wants to exclude
	 * time from the counter the user spent in original engine dialogs.
	 *
	 * @param time Play time to set up in ms.
	 */
	void setTotalPlayTime(uint32 time = 0);

	inline Common::TimerManager *getTimerManager() { return _timer; }
	inline Common::EventManager *getEventManager() { return _eventMan; }
	inline Common::SaveFileManager *getSaveFileManager() { return _saveFileMan; }

public:

	/** On some systems, check if the game appears to be run from CD. */
	void checkCD();

protected:

	/**
	 * Indicate whether an autosave should be performed.
	 */
	bool shouldPerformAutoSave(int lastSaveTime);

};

// FIXME: HACK for MidiEmu & error()
extern Engine *g_engine;

#endif

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

#ifndef SWORD1_H
#define SWORD1_H

#include "engines/engine.h"
#include "common/error.h"
#include "common/keyboard.h"
#include "common/rect.h"
#include "common/util.h"
#include "sword1/sworddefs.h"
#include "sword1/console.h"

/**
 * This is the namespace of the Sword1 engine.
 *
 * Status of this engine: ???
 *
 * Games using this engine:
 * - Broken Sword: The Shadow of the Templars
 */
namespace Sword1 {

enum {
	GF_DEMO = 1 << 0
};

enum ControlPanelMode {
	CP_NORMAL = 0,
	CP_DEATHSCREEN,
	CP_THEEND,
	CP_NEWGAME
};

class Screen;
class Sound;
class Logic;
class Mouse;
class ResMan;
class ObjectMan;
class Menu;
class Music;
class Control;

struct SystemVars {
	bool    runningFromCd;
	uint32  currentCD;          // starts at zero, then either 1 or 2 depending on section being played
	uint32  justRestoredGame;   // see main() in sword.c & New_screen() in gtm_core.c

	uint8   controlPanelMode;   // 1 death screen version of the control panel, 2 = successful end of game, 3 = force restart
	bool    forceRestart;
	bool    wantFade;           // when true => fade during scene change, else cut.
	uint8   playSpeech;
	uint8   showText;
	uint8   language;
	bool    isDemo;
	Common::Platform platform;
	Common::Language realLanguage;
};

class SwordEngine : public Engine {
public:
	SwordEngine(OSystem *syst);
	virtual ~SwordEngine();
	static SystemVars _systemVars;
	void reinitialize();

	uint32 _features;

	bool mouseIsActive();

	static bool isMac() { return _systemVars.platform == Common::kPlatformMacintosh; }
	static bool isPsx() { return _systemVars.platform == Common::kPlatformPSX; }
	static bool isWindows() { return _systemVars.platform == Common::kPlatformWindows ; }

protected:
	// Engine APIs
	Common::Error init();
	Common::Error go();
	virtual Common::Error run() {
		Common::Error err;
		err = init();
		if (err.getCode() != Common::kNoError)
			return err;
		return go();
	}
	virtual bool hasFeature(EngineFeature f) const;
	virtual void syncSoundSettings();

	GUI::Debugger *getDebugger() { return _console; }

	Common::Error loadGameState(int slot);
	bool canLoadGameStateCurrently();
	Common::Error saveGameState(int slot, const Common::String &desc);
	bool canSaveGameStateCurrently();

private:
	void delay(int32 amount);

	void checkCdFiles();
	void checkCd();
	void showFileErrorMsg(uint8 type, bool *fileExists);
	void flagsToBool(bool *dest, uint8 flags);

	void reinitRes(); //Reinits the resources after a GMM load

	SwordConsole *_console;

	uint8 mainLoop();

	Common::Point _mouseCoord;
	uint16 _mouseState;
	Common::KeyState _keyPressed;

	ResMan      *_resMan;
	ObjectMan   *_objectMan;
	Screen      *_screen;
	Mouse       *_mouse;
	Logic       *_logic;
	Sound       *_sound;
	Menu        *_menu;
	Music       *_music;
	Control     *_control;
	static const uint8  _cdList[TOTAL_SECTIONS];
	static const CdFile _pcCdFileList[];
	static const CdFile _macCdFileList[];
	static const CdFile _psxCdFileList[];
};

} // End of namespace Sword1

#endif //BSSWORD1_H

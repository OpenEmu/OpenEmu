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

#ifndef GOB_GAME_H
#define GOB_GAME_H

#include "common/str.h"

#include "gob/util.h"
#include "gob/video.h"
#include "gob/sound/sounddesc.h"

namespace Gob {

class Script;
class Resources;
class Variables;
class Hotspots;

class Environments {
public:
	static const uint8 kEnvironmentCount = 20;

	Environments(GobEngine *vm);
	~Environments();

	void set(uint8 env);
	void get(uint8 env) const;

	const Common::String &getTotFile(uint8 env) const;

	bool has(Variables *variables, uint8 startEnv = 0, int16 except = -1) const;
	bool has(Script    *script   , uint8 startEnv = 0, int16 except = -1) const;
	bool has(Resources *resources, uint8 startEnv = 0, int16 except = -1) const;

	void deleted(Variables *variables);

	void clear();

	bool setMedia(uint8 env);
	bool getMedia(uint8 env);
	bool clearMedia(uint8 env);

private:
	struct Environment {
		int32          cursorHotspotX;
		int32          cursorHotspotY;
		Common::String totFile;
		Variables     *variables;
		Script        *script;
		Resources     *resources;
	};

	struct Media {
		SurfacePtr sprites[10];
		SoundDesc  sounds[10];
		Font      *fonts[17];
	};

	GobEngine *_vm;

	Environment _environments[kEnvironmentCount];
	Media       _media[kEnvironmentCount];
};

class TotFunctions {
public:
	TotFunctions(GobEngine *vm);
	~TotFunctions();

	int find(const Common::String &totFile) const;

	bool load(const Common::String &totFile);
	bool unload(const Common::String &totFile);

	bool call(const Common::String &totFile, const Common::String &function) const;
	bool call(const Common::String &totFile, uint16 offset) const;

private:
	static const uint8 kTotCount = 100;

	struct Function {
		Common::String name;
		byte type;
		uint16 offset;
	};

	struct Tot {
		Common::String file;

		Common::List<Function> functions;

		Script    *script;
		Resources *resources;
	};

	GobEngine *_vm;

	Tot _tots[kTotCount];

	bool loadTot(Tot &tot, const Common::String &file);
	void freeTot(Tot &tot);

	bool loadIDE(Tot &tot);

	int findFree() const;

	bool call(const Tot &tot, uint16 offset) const;
};

class Game {
public:
	Script    *_script;
	Resources *_resources;
	Hotspots  *_hotspots;

	Common::String _curTotFile;
	Common::String _totToLoad;

	int32 _startTimeKey;
	MouseButtons _mouseButtons;

	bool _noScroll;
	bool _preventScroll;

	bool  _wantScroll;
	int16 _wantScrollX;
	int16 _wantScrollY;

	byte _handleMouse;
	char _forceHandleMouse;

	Game(GobEngine *vm);
	virtual ~Game();

	void prepareStart();

	void playTot(int16 function);

	void capturePush(int16 left, int16 top, int16 width, int16 height);
	void capturePop(char doDraw);

	void freeSoundSlot(int16 slot);

	void wantScroll(int16 x, int16 y);
	void evaluateScroll();

	int16 checkKeys(int16 *pMousex = 0, int16 *pMouseY = 0,
			MouseButtons *pButtons = 0, char handleMouse = 0);
	void start();

	void totSub(int8 flags, const Common::String &totFile);
	void switchTotSub(int16 index, int16 function);

	void deletedVars(Variables *variables);

	bool loadFunctions(const Common::String &tot, uint16 flags);
	bool callFunction(const Common::String &tot, const Common::String &function, int16 param);

protected:
	GobEngine *_vm;

	char _tempStr[256];

	// Capture
	Common::Rect _captureStack[20];
	int16 _captureCount;

	// For totSub()
	int8 _curEnvironment;
	int8 _numEnvironments;
	Environments _environments;

	TotFunctions _totFunctions;

	void clearUnusedEnvironment();
};

} // End of namespace Gob

#endif // GOB_GAME_H

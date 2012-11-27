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

#ifndef GROOVIE_H
#define GROOVIE_H

#include "groovie/debug.h"
#include "groovie/font.h"

#include "engines/engine.h"
#include "graphics/pixelformat.h"

namespace Common {
class MacResManager;
}

/**
 * This is the namespace of the Groovie engine.
 *
 * Status of this engine: This engine supports both versions of the Groovie
 * game engine.  The 7th Guest uses the first revision of Groovie, and is
 * now fully completable.  All remaining Groovie games use V2 of the engine,
 * which is under slow development.
 *
 * Games using this engine:
 * - The 7th Guest (completable)
 * - The 11th Hour
 * - Clandestiny
 * - Uncle Henry's Playhouse
 * - Tender Loving Care
 */
namespace Groovie {

class GraphicsMan;
class GrvCursorMan;
class MusicPlayer;
class ResMan;
class Script;
class VideoPlayer;

enum DebugLevels {
	kGroovieDebugAll = 1 << 0,
	kGroovieDebugVideo = 1 << 1,
	kGroovieDebugResource = 1 << 2,
	kGroovieDebugScript = 1 << 3,
	kGroovieDebugUnknown = 1 << 4,
	kGroovieDebugHotspots = 1 << 5,
	kGroovieDebugCursor = 1 << 6,
	kGroovieDebugMIDI = 1 << 7,
	kGroovieDebugScriptvars = 1 << 8,
	kGroovieDebugCell = 1 << 9,
	kGroovieDebugFast = 1 << 10
		// the current limitation is 32 debug levels (1 << 31 is the last one)
};

/**
 * This enum reflects the available movie speed settings:
 * - Normal:  both movies and 'teeth' animations are played at a normal speed
 * - Tweaked: movies are played at a normal speed, 'teeth' animations at
              increased speed
 * - iOS:     both movies and 'teeth' animations are played at increased speed
 */
enum GameSpeed {
	kGroovieSpeedNormal,
	kGroovieSpeediOS,
	kGroovieSpeedTweaked
};

struct GroovieGameDescription;

class GroovieEngine : public Engine {
public:
	GroovieEngine(OSystem *syst, const GroovieGameDescription *gd);
	~GroovieEngine();

	Common::Platform getPlatform() const;

protected:

	// Engine APIs
	Common::Error run();

	virtual bool hasFeature(EngineFeature f) const;

	virtual bool canLoadGameStateCurrently();
	virtual Common::Error loadGameState(int slot);
	virtual void syncSoundSettings();

	virtual Debugger *getDebugger() { return _debugger; }

public:
	void waitForInput();

	Graphics::PixelFormat _pixelFormat;
	bool _mode8bit;
	Script *_script;
	ResMan *_resMan;
	GrvCursorMan *_grvCursorMan;
	VideoPlayer *_videoPlayer;
	MusicPlayer *_musicPlayer;
	GraphicsMan *_graphicsMan;
	const Graphics::Font *_font;

	Common::MacResManager *_macResFork;

	GameSpeed _modeSpeed;

private:
	const GroovieGameDescription *_gameDescription;
	Debugger *_debugger;
	bool _waitingForInput;
	T7GFont _sphinxFont;
};

} // End of namespace Groovie

#endif // GROOVIE_H

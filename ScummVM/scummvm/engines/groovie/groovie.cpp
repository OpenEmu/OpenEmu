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

#include "audio/mididrv.h"
#include "audio/mixer.h"

#include "groovie/groovie.h"
#include "groovie/cursor.h"
#include "groovie/detection.h"
#include "groovie/graphics.h"
#include "groovie/music.h"
#include "groovie/resource.h"
#include "groovie/roq.h"
#include "groovie/vdx.h"

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/file.h"
#include "common/macresman.h"
#include "common/textconsole.h"

#include "backends/audiocd/audiocd.h"
#include "engines/util.h"
#include "graphics/fontman.h"
#include "audio/mixer.h"

namespace Groovie {

GroovieEngine::GroovieEngine(OSystem *syst, const GroovieGameDescription *gd) :
	Engine(syst), _gameDescription(gd), _debugger(NULL), _script(NULL),
	_resMan(NULL), _grvCursorMan(NULL), _videoPlayer(NULL), _musicPlayer(NULL),
	_graphicsMan(NULL), _macResFork(NULL), _waitingForInput(false), _font(NULL) {

	// Adding the default directories
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "groovie");
	SearchMan.addSubDirectoryMatching(gameDataDir, "media");
	SearchMan.addSubDirectoryMatching(gameDataDir, "system");

	_modeSpeed = kGroovieSpeedNormal;
	if (ConfMan.hasKey("t7g_speed")) {
		Common::String speed = ConfMan.get("t7g_speed");
		if (speed.equals("im_an_ios"))
			_modeSpeed = kGroovieSpeediOS;
		else if (speed.equals("tweaked"))
			_modeSpeed = kGroovieSpeedTweaked;
	}

	// Initialize the custom debug levels
	DebugMan.addDebugChannel(kGroovieDebugAll, "All", "Debug everything");
	DebugMan.addDebugChannel(kGroovieDebugVideo, "Video", "Debug video and audio playback");
	DebugMan.addDebugChannel(kGroovieDebugResource, "Resource", "Debug resouce management");
	DebugMan.addDebugChannel(kGroovieDebugScript, "Script", "Debug the scripts");
	DebugMan.addDebugChannel(kGroovieDebugUnknown, "Unknown", "Report values of unknown data in files");
	DebugMan.addDebugChannel(kGroovieDebugHotspots, "Hotspots", "Show the hotspots");
	DebugMan.addDebugChannel(kGroovieDebugCursor, "Cursor", "Debug cursor decompression / switching");
	DebugMan.addDebugChannel(kGroovieDebugMIDI, "MIDI", "Debug MIDI / XMIDI files");
	DebugMan.addDebugChannel(kGroovieDebugScriptvars, "Scriptvars", "Print out any change to script variables");
	DebugMan.addDebugChannel(kGroovieDebugCell, "Cell", "Debug the cell game (in the microscope)");
	DebugMan.addDebugChannel(kGroovieDebugFast, "Fast", "Play videos quickly, with no sound (unstable)");
}

GroovieEngine::~GroovieEngine() {
	// Delete the remaining objects
	delete _debugger;
	delete _resMan;
	delete _grvCursorMan;
	delete _videoPlayer;
	delete _musicPlayer;
	delete _graphicsMan;
	delete _script;
	delete _macResFork;
}

Common::Error GroovieEngine::run() {
	_script = new Script(this, _gameDescription->version);

	// Initialize the graphics
	switch (_gameDescription->version) {
	case kGroovieV2:
		// Request the mode with the highest precision available
		initGraphics(640, 480, true, NULL);

		// Save the enabled mode as it can be both an RGB mode or CLUT8
		_pixelFormat = _system->getScreenFormat();
		_mode8bit = (_pixelFormat == Graphics::PixelFormat::createFormatCLUT8());
		break;
	case kGroovieT7G:
		initGraphics(640, 480, true);
		_pixelFormat = Graphics::PixelFormat::createFormatCLUT8();
		break;
	}

	// Create debugger. It requires GFX to be initialized
	_debugger = new Debugger(this);
	_script->setDebugger(_debugger);

	// Create the graphics manager
	_graphicsMan = new GraphicsMan(this);

	// Create the resource and cursor managers and the video player
	// Prepare the font too
	switch (_gameDescription->version) {
	case kGroovieT7G:
		if (getPlatform() == Common::kPlatformMacintosh) {
			_macResFork = new Common::MacResManager();
			if (!_macResFork->open(_gameDescription->desc.filesDescriptions[0].fileName))
				error("Could not open %s as a resource fork", _gameDescription->desc.filesDescriptions[0].fileName);
			// The Macintosh release used system fonts. We use GUI fonts.
			_font = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
		} else {
			Common::File fontfile;
			if (!fontfile.open("sphinx.fnt")) {
				error("Couldn't open sphinx.fnt");
				return Common::kNoGameDataFoundError;
			} else if (!_sphinxFont.load(fontfile)) {
				error("Error loading sphinx.fnt");
				return Common::kUnknownError;
			}
			fontfile.close();
			_font = &_sphinxFont;
		}

		_resMan = new ResMan_t7g(_macResFork);
		_grvCursorMan = new GrvCursorMan_t7g(_system, _macResFork);
		_videoPlayer = new VDXPlayer(this);
		break;
	case kGroovieV2:
		_resMan = new ResMan_v2();
		_grvCursorMan = new GrvCursorMan_v2(_system);
		_videoPlayer = new ROQPlayer(this);
		break;
	}

	// Detect ScummVM Music Enhancement Project presence (T7G only)
	if (Common::File::exists("gu16.ogg") && _gameDescription->version == kGroovieT7G) {
		// Load player for external files
		_musicPlayer = new MusicPlayerIOS(this);
	} else {
		// Create the music player
		switch (getPlatform()) {
		case Common::kPlatformMacintosh:
			// TODO: The 11th Hour Mac uses QuickTime MIDI files
			// Right now, since the XMIDI are present and it is still detected as
			// the DOS version, we don't have to do anything here.
			_musicPlayer = new MusicPlayerMac(this);
			break;
		case Common::kPlatformIOS:
			_musicPlayer = new MusicPlayerIOS(this);
			break;
		default:
			_musicPlayer = new MusicPlayerXMI(this, _gameDescription->version == kGroovieT7G ? "fat" : "sample");
			break;
		}
	}

	// Load volume levels
	syncSoundSettings();

	// Get the name of the main script
	Common::String filename = _gameDescription->desc.filesDescriptions[0].fileName;
	if (_gameDescription->version == kGroovieT7G) {
		// Run The 7th Guest's demo if requested
		if (ConfMan.hasKey("demo_mode") && ConfMan.getBool("demo_mode"))
			filename = "demo.grv";
		else if (getPlatform() == Common::kPlatformMacintosh)
			filename = "script.grv"; // Stored inside the executable's resource fork
	} else if (_gameDescription->version == kGroovieV2) {
		// Open the disk index
		Common::File disk;
		if (!disk.open(filename)) {
			error("Couldn't open %s", filename.c_str());
			return Common::kNoGameDataFoundError;
		}

		// Search the entry
		bool found = false;
		int index = 0;
		while (!found && !disk.eos()) {
			Common::String line = disk.readLine();
			if (line.hasPrefix("title: ")) {
				// A new entry
				index++;
			} else if (line.hasPrefix("boot: ") && index == _gameDescription->indexEntry) {
				// It's the boot of the entry we're looking for,
				// get the script filename
				filename = line.c_str() + 6;
				found = true;
			}
		}

		// Couldn't find the entry
		if (!found) {
			error("Couldn't find entry %d in %s", _gameDescription->indexEntry, filename.c_str());
			return Common::kUnknownError;
		}
	}

	// Check the script file extension
	if (!filename.hasSuffix(".grv")) {
		error("%s isn't a valid script filename", filename.c_str());
		return Common::kUnknownError;
	}

	// Load the script
	if (!_script->loadScript(filename)) {
		error("Couldn't load the script file %s", filename.c_str());
		return Common::kUnknownError;
	}

	// Should I load a saved game?
	if (ConfMan.hasKey("save_slot")) {
		// Get the requested slot
		int slot = ConfMan.getInt("save_slot");
		_script->directGameLoad(slot);
	}

	// Game timer counter
	uint16 tmr = 0;

	// Check that the game files and the audio tracks aren't together run from
	// the same cd
	if (getPlatform() != Common::kPlatformIOS) {
		checkCD();

		// Initialize the CD
		int cd_num = ConfMan.getInt("cdrom");
		if (cd_num >= 0)
			_system->getAudioCDManager()->openCD(cd_num);
	}

	while (!shouldQuit()) {
		// Give the debugger a chance to act
		_debugger->onFrame();

		// Handle input
		Common::Event ev;
		while (_eventMan->pollEvent(ev)) {
			switch (ev.type) {
			case Common::EVENT_KEYDOWN:
				// CTRL-D: Attach the debugger
				if ((ev.kbd.flags & Common::KBD_CTRL) && ev.kbd.keycode == Common::KEYCODE_d)
					_debugger->attach();

				// Send the event to the scripts
				_script->setKbdChar(ev.kbd.ascii);

				// Continue the script execution to handle the key
				_waitingForInput = false;
				break;

			case Common::EVENT_MAINMENU:
				// Closing the GMM
			case Common::EVENT_MOUSEMOVE:
				// Continue the script execution, the mouse pointer
				// may fall inside a different hotspot now
				_waitingForInput = false;
				break;

			case Common::EVENT_LBUTTONDOWN:
				// Send the event to the scripts
				_script->setMouseClick(1);

				// Continue the script execution to handle
				// the click
				_waitingForInput = false;
				break;

			case Common::EVENT_RBUTTONDOWN:
				// Send the event to the scripts (to skip the video)
				_script->setMouseClick(2);
				break;

			case Common::EVENT_QUIT:
				quitGame();
				break;

			default:
				break;
			}
		}

		// The event loop may have triggered the quit status. In this case,
		// stop the execution.
		if (shouldQuit()) {
			continue;
		}

		if (_waitingForInput) {
			// Still waiting for input, just update the mouse, game timer and then wait a bit more
			_grvCursorMan->animate();
			_system->updateScreen();
			tmr++;
			// Wait a little bit between increments.  While mouse is moving, this triggers
			// only negligably slower.
			if (tmr > 4) {
				_script->timerTick();
				tmr = 0;
			}

			_system->delayMillis(50);
		} else if (_graphicsMan->isFading()) {
			// We're waiting for a fading to end, let the CPU rest
			// for a while and continue
			_system->delayMillis(30);
		} else {
			// Everything's fine, execute another script step
			_script->step();
		}

		// Update the screen if required
		_graphicsMan->update();
	}

	return Common::kNoError;
}

Common::Platform GroovieEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

bool GroovieEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime);
}

void GroovieEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	bool mute = ConfMan.getBool("mute");

	// Set the music volume
	_musicPlayer->setUserVolume(mute ? 0 : ConfMan.getInt("music_volume"));

	// Videos just contain one digital audio track, which can be used for
	// both SFX or Speech, but the engine doesn't know what they contain, so
	// we have to use just one volume setting for videos.
	// We use "speech" because most users will want to change the videos
	// volume when they can't hear the speech because of the music.
	_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType,
		mute ? 0 : ConfMan.getInt("speech_volume"));
}

bool GroovieEngine::canLoadGameStateCurrently() {
	// TODO: verify the engine has been initialized
	return true;
}

Common::Error GroovieEngine::loadGameState(int slot) {
	_script->directGameLoad(slot);

	// TODO: Use specific error codes
	return Common::kNoError;
}

void GroovieEngine::waitForInput() {
	_waitingForInput = true;
}

} // End of namespace Groovie

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

#include "common/debug-channels.h"

#include "backends/audiocd/audiocd.h"
#include "base/plugins.h"
#include "common/config-manager.h"
#include "audio/mididrv.h"

#include "gui/gui-manager.h"
#include "gui/dialog.h"
#include "gui/widget.h"

#include "gob/gob.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/game.h"
#include "gob/sound/sound.h"
#include "gob/init.h"
#include "gob/inter.h"
#include "gob/draw.h"
#include "gob/goblin.h"
#include "gob/map.h"
#include "gob/mult.h"
#include "gob/palanim.h"
#include "gob/scenery.h"
#include "gob/videoplayer.h"
#include "gob/save/saveload.h"

#include "gob/pregob/pregob.h"
#include "gob/pregob/onceupon/abracadabra.h"
#include "gob/pregob/onceupon/babayaga.h"

namespace Gob {

#define MAX_TIME_DELTA 100

const Common::Language GobEngine::_gobToScummVMLang[] = {
	Common::FR_FRA,
	Common::DE_DEU,
	Common::EN_GRB,
	Common::ES_ESP,
	Common::IT_ITA,
	Common::EN_USA,
	Common::NL_NLD,
	Common::KO_KOR,
	Common::HE_ISR,
	Common::PT_BRA,
	Common::JA_JPN
};


class PauseDialog : public GUI::Dialog {
public:
	PauseDialog();

	virtual void reflowLayout();
	virtual void handleKeyDown(Common::KeyState state);

private:
	Common::String _message;
	GUI::StaticTextWidget *_text;
};

PauseDialog::PauseDialog() : GUI::Dialog(0, 0, 0, 0) {
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundSpecial;

	_message = "Game paused. Press Ctrl+p again to continue.";
	_text = new GUI::StaticTextWidget(this, 4, 0, 10, 10,
			_message, Graphics::kTextAlignCenter);
}

void PauseDialog::reflowLayout() {
	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	int width = g_gui.getStringWidth(_message) + 16;
	int height = g_gui.getFontHeight() + 8;

	_w = width;
	_h = height;
	_x = (screenW - width) / 2;
	_y = (screenH - height) / 2;

	_text->setSize(_w - 8, _h);
}

void PauseDialog::handleKeyDown(Common::KeyState state) {
	// Close on CTRL+p
	if ((state.hasFlags(Common::KBD_CTRL)) && (state.keycode == Common::KEYCODE_p))
		close();
}


GobEngine::GobEngine(OSystem *syst) : Engine(syst), _rnd("gob") {
	_sound     = 0; _mult     = 0; _game    = 0;
	_global    = 0; _dataIO   = 0; _goblin  = 0;
	_vidPlayer = 0; _init     = 0; _inter   = 0;
	_map       = 0; _palAnim  = 0; _scenery = 0;
	_draw      = 0; _util     = 0; _video   = 0;
	_saveLoad  = 0; _preGob   = 0;

	_pauseStart = 0;

	// Setup mixer
	bool muteSFX   = ConfMan.getBool("mute") || ConfMan.getBool("sfx_mute");
	bool muteMusic = ConfMan.getBool("mute") || ConfMan.getBool("music_mute");

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType,
			muteSFX   ? 0 : ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType,
			muteMusic ? 0 : ConfMan.getInt("music_volume"));

	_copyProtection = ConfMan.getBool("copy_protection");

	_console = new GobConsole(this);

	DebugMan.addDebugChannel(kDebugFuncOp, "FuncOpcodes", "Script FuncOpcodes debug level");
	DebugMan.addDebugChannel(kDebugDrawOp, "DrawOpcodes", "Script DrawOpcodes debug level");
	DebugMan.addDebugChannel(kDebugGobOp, "GoblinOpcodes", "Script GoblinOpcodes debug level");
	DebugMan.addDebugChannel(kDebugSound, "Sound", "Sound output debug level");
	DebugMan.addDebugChannel(kDebugExpression, "Expression", "Expression parser debug level");
	DebugMan.addDebugChannel(kDebugGameFlow, "Gameflow", "Gameflow debug level");
	DebugMan.addDebugChannel(kDebugFileIO, "FileIO", "File Input/Output debug level");
	DebugMan.addDebugChannel(kDebugSaveLoad, "SaveLoad", "Saving/Loading debug level");
	DebugMan.addDebugChannel(kDebugGraphics, "Graphics", "Graphics debug level");
	DebugMan.addDebugChannel(kDebugVideo, "Video", "IMD/VMD video debug level");
	DebugMan.addDebugChannel(kDebugHotspots, "Hotspots", "Hotspots debug level");
	DebugMan.addDebugChannel(kDebugDemo, "Demo", "Demo script debug level");
}

GobEngine::~GobEngine() {
	deinitGameParts();

	delete _console;
}

const char *GobEngine::getLangDesc(int16 language) const {
	if ((language < 0) || (language > 10))
		language = 2;
	return Common::getLanguageDescription(_gobToScummVMLang[language]);
}

void GobEngine::validateLanguage() {
	if (_global->_languageWanted != _global->_language) {
		warning("Your game version doesn't support the requested language %s",
				getLangDesc(_global->_languageWanted));

		if (((_global->_languageWanted == 2) && (_global->_language == 5)) ||
		    ((_global->_languageWanted == 5) && (_global->_language == 2)))
			warning("Using %s instead", getLangDesc(_global->_language));
		else
			warning("Using the first language available: %s",
					getLangDesc(_global->_language));

		_global->_languageWanted = _global->_language;
	}
}

void GobEngine::validateVideoMode(int16 videoMode) {
	if ((videoMode != 0x10) && (videoMode != 0x13) &&
		  (videoMode != 0x14) && (videoMode != 0x18))
		error("Video mode 0x%X is not supported", videoMode);
}

EndiannessMethod GobEngine::getEndiannessMethod() const {
	return _endiannessMethod;
}

Endianness GobEngine::getEndianness() const {
	if ((getPlatform() == Common::kPlatformAmiga) ||
	    (getPlatform() == Common::kPlatformMacintosh) ||
	    (getPlatform() == Common::kPlatformAtariST))
		return kEndiannessBE;

	return kEndiannessLE;
}

Common::Platform GobEngine::getPlatform() const {
	return _platform;
}

GameType GobEngine::getGameType() const {
	return _gameType;
}

bool GobEngine::isCD() const {
	return (_features & kFeaturesCD) != 0;
}

bool GobEngine::isEGA() const {
	return (_features & kFeaturesEGA) != 0;
}

bool GobEngine::hasAdLib() const {
	return (_features & kFeaturesAdLib) != 0;
}

bool GobEngine::isSCNDemo() const {
	return (_features & kFeaturesSCNDemo) != 0;
}

bool GobEngine::isBATDemo() const {
	return (_features & kFeaturesBATDemo) != 0;
}

bool GobEngine::is640x480() const {
	return (_features & kFeatures640x480) != 0;
}

bool GobEngine::is800x600() const {
	return (_features & kFeatures800x600) != 0;
}

bool GobEngine::isTrueColor() const {
	return (_features & kFeaturesTrueColor) != 0;
}

bool GobEngine::isDemo() const {
	return (isSCNDemo() || isBATDemo());
}

bool GobEngine::hasResourceSizeWorkaround() const {
	return _resourceSizeWorkaround;
}

bool GobEngine::isCurrentTot(const Common::String &tot) const {
	return _game->_curTotFile.equalsIgnoreCase(tot);
}

const Graphics::PixelFormat &GobEngine::getPixelFormat() const {
	return _pixelFormat;
}

void GobEngine::setTrueColor(bool trueColor) {
	if (isTrueColor() == trueColor)
		return;

	_features = (_features & ~kFeaturesTrueColor) | (trueColor ? kFeaturesTrueColor : 0);

	_video->setSize(is640x480());

	_pixelFormat = g_system->getScreenFormat();

	Common::Array<SurfacePtr>::iterator surf;
	for (surf = _draw->_spritesArray.begin(); surf != _draw->_spritesArray.end(); ++surf)
		if (*surf)
			(*surf)->setBPP(_pixelFormat.bytesPerPixel);

	if (_draw->_backSurface)
		_draw->_backSurface->setBPP(_pixelFormat.bytesPerPixel);
	if (_draw->_frontSurface)
		_draw->_frontSurface->setBPP(_pixelFormat.bytesPerPixel);
	if (_draw->_cursorSprites)
		_draw->_cursorSprites->setBPP(_pixelFormat.bytesPerPixel);
	if (_draw->_cursorSpritesBack)
		_draw->_cursorSpritesBack->setBPP(_pixelFormat.bytesPerPixel);
	if (_draw->_scummvmCursor)
		_draw->_scummvmCursor->setBPP(_pixelFormat.bytesPerPixel);
	SurfacePtr _scummvmCursor;
}

Common::Error GobEngine::run() {
	Common::Error err;

	err = initGameParts();
	if (err.getCode() != Common::kNoError)
		return err;

	err = initGraphics();
	if (err.getCode() != Common::kNoError)
		return err;

	// On some systems it's not safe to run CD audio games from the CD.
	if (isCD())
		checkCD();

	int cd_num = ConfMan.getInt("cdrom");
	if (cd_num >= 0)
		_system->getAudioCDManager()->openCD(cd_num);

	_global->_debugFlag = 1;
	_video->_doRangeClamp = true;

	// WORKAROUND: Some versions check the video mode to detect the system
	if (_platform == Common::kPlatformAmiga)
		_global->_fakeVideoMode = 0x11;
	else if (_platform == Common::kPlatformAtariST)
		_global->_fakeVideoMode = 0x10;
	else
		_global->_fakeVideoMode = 0x13;

	_global->_videoMode = 0x13;
	_global->_useMouse = 1;
	_global->_soundFlags = MIDI_FLAG | SPEAKER_FLAG | BLASTER_FLAG | ADLIB_FLAG;

	if (ConfMan.hasKey("language"))
		_language = Common::parseLanguage(ConfMan.get("language"));

	switch (_language) {
	case Common::FR_FRA:
	case Common::RU_RUS:
		_global->_language = kLanguageFrench;
		break;
	case Common::DE_DEU:
		_global->_language = kLanguageGerman;
		break;
	case Common::EN_ANY:
	case Common::EN_GRB:
	case Common::HU_HUN:
		_global->_language = kLanguageBritish;
		break;
	case Common::ES_ESP:
		_global->_language = kLanguageSpanish;
		break;
	case Common::IT_ITA:
		_global->_language = kLanguageItalian;
		break;
	case Common::EN_USA:
		_global->_language = kLanguageAmerican;
		break;
	case Common::NL_NLD:
		_global->_language = kLanguageDutch;
		break;
	case Common::KO_KOR:
		_global->_language = kLanguageKorean;
		break;
	case Common::HE_ISR:
		_global->_language = kLanguageHebrew;
		break;
	case Common::PT_BRA:
		_global->_language = kLanguagePortuguese;
		break;
	case Common::JA_JPN:
		_global->_language = kLanguageJapanese;
		break;
	default:
		_global->_language = kLanguageBritish;
		break;
	}
	_global->_languageWanted = _global->_language;

	_init->initGame();

	return Common::kNoError;
}

void GobEngine::pauseEngineIntern(bool pause) {
	if (pause) {
		_pauseStart = _system->getMillis();
	} else {
		uint32 duration = _system->getMillis() - _pauseStart;

		_util->notifyPaused(duration);

		_game->_startTimeKey += duration;
		_draw->_cursorTimeKey += duration;
		if (_inter && (_inter->_soundEndTimeKey != 0))
			_inter->_soundEndTimeKey += duration;
	}

	if (_vidPlayer)
		_vidPlayer->pauseAll(pause);
	_mixer->pauseAll(pause);
}

void GobEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	_init->updateConfig();
}

void GobEngine::pauseGame() {
	pauseEngineIntern(true);

	PauseDialog pauseDialog;

	pauseDialog.runModal();

	pauseEngineIntern(false);
}

Common::Error GobEngine::initGameParts() {
	_resourceSizeWorkaround = false;

	// just detect some devices some of which will be always there if the music is not disabled
	_noMusic = MidiDriver::getMusicType(MidiDriver::detectDevice(MDT_PCSPK | MDT_MIDI | MDT_ADLIB)) == MT_NULL ? true : false;

	_endiannessMethod = kEndiannessMethodSystem;

	_global    = new Global(this);
	_util      = new Util(this);
	_dataIO    = new DataIO();
	_palAnim   = new PalAnim(this);
	_vidPlayer = new VideoPlayer(this);
	_sound     = new Sound(this);
	_game      = new Game(this);

	switch (_gameType) {
	case kGameTypeGob1:
		_init     = new Init_v1(this);
		_video    = new Video_v1(this);
		_inter    = new Inter_v1(this);
		_mult     = new Mult_v1(this);
		_draw     = new Draw_v1(this);
		_map      = new Map_v1(this);
		_goblin   = new Goblin_v1(this);
		_scenery  = new Scenery_v1(this);
		break;

	case kGameTypeGeisha:
		_init     = new Init_Geisha(this);
		_video    = new Video_v1(this);
		_inter    = new Inter_Geisha(this);
		_mult     = new Mult_v1(this);
		_draw     = new Draw_v1(this);
		_map      = new Map_v1(this);
		_goblin   = new Goblin_v1(this);
		_scenery  = new Scenery_v1(this);
		_saveLoad = new SaveLoad_Geisha(this, _targetName.c_str());

		_endiannessMethod = kEndiannessMethodAltFile;
		break;

	case kGameTypeFascination:
		_init     = new Init_Fascination(this);
		_video    = new Video_v2(this);
		_inter    = new Inter_Fascination(this);
		_mult     = new Mult_v2(this);
		_draw     = new Draw_Fascination(this);
		_map      = new Map_v2(this);
		_goblin   = new Goblin_v2(this);
		_scenery  = new Scenery_v2(this);
		_saveLoad = new SaveLoad_Fascination(this, _targetName.c_str());
		break;

	case kGameTypeWeen:
	case kGameTypeGob2:
		_init     = new Init_v2(this);
		_video    = new Video_v2(this);
		_inter    = new Inter_v2(this);
		_mult     = new Mult_v2(this);
		_draw     = new Draw_v2(this);
		_map      = new Map_v2(this);
		_goblin   = new Goblin_v2(this);
		_scenery  = new Scenery_v2(this);
		_saveLoad = new SaveLoad_v2(this, _targetName.c_str());
		break;

	case kGameTypeBargon:
		_init     = new Init_v2(this);
		_video    = new Video_v2(this);
		_inter    = new Inter_Bargon(this);
		_mult     = new Mult_v2(this);
		_draw     = new Draw_Bargon(this);
		_map      = new Map_v2(this);
		_goblin   = new Goblin_v2(this);
		_scenery  = new Scenery_v2(this);
		_saveLoad = new SaveLoad_v2(this, _targetName.c_str());
		break;

	case kGameTypeLittleRed:
		_init     = new Init_v2(this);
		_video    = new Video_v2(this);
		_inter    = new Inter_LittleRed(this);
		_mult     = new Mult_v2(this);
		_draw     = new Draw_v2(this);
		_map      = new Map_v2(this);
		_goblin   = new Goblin_v2(this);
		_scenery  = new Scenery_v2(this);

		// WORKAROUND: Little Red Riding Hood has a small resource size glitch in the
		//             screen where Little Red needs to find the animals' homes.
		_resourceSizeWorkaround = true;
		break;

	case kGameTypeAJWorld:
		_init     = new Init_v2(this);
		_video    = new Video_v2(this);
		_inter    = new Inter_v2(this);
		_mult     = new Mult_v2(this);
		_draw     = new Draw_v2(this);
		_map      = new Map_v2(this);
		_goblin   = new Goblin_v2(this);
		_scenery  = new Scenery_v2(this);
		_saveLoad = new SaveLoad_AJWorld(this, _targetName.c_str());
		break;

	case kGameTypeGob3:
		_init     = new Init_v3(this);
		_video    = new Video_v2(this);
		_inter    = new Inter_v3(this);
		_mult     = new Mult_v2(this);
		_draw     = new Draw_v2(this);
		_map      = new Map_v2(this);
		_goblin   = new Goblin_v3(this);
		_scenery  = new Scenery_v2(this);
		_saveLoad = new SaveLoad_v3(this, _targetName.c_str(), SaveLoad_v3::kScreenshotTypeGob3);
		break;

	case kGameTypeInca2:
		_init     = new Init_v3(this);
		_video    = new Video_v2(this);
		_inter    = new Inter_Inca2(this);
		_mult     = new Mult_v2(this);
		_draw     = new Draw_v2(this);
		_map      = new Map_v2(this);
		_goblin   = new Goblin_v3(this);
		_scenery  = new Scenery_v2(this);
		_saveLoad = new SaveLoad_Inca2(this, _targetName.c_str());
		break;

	case kGameTypeLostInTime:
		_init     = new Init_v3(this);
		_video    = new Video_v2(this);
		_inter    = new Inter_v3(this);
		_mult     = new Mult_v2(this);
		_draw     = new Draw_v2(this);
		_map      = new Map_v2(this);
		_goblin   = new Goblin_v3(this);
		_scenery  = new Scenery_v2(this);
		_saveLoad = new SaveLoad_v3(this, _targetName.c_str(), SaveLoad_v3::kScreenshotTypeLost);
		break;

	case kGameTypeWoodruff:
		_init     = new Init_v4(this);
		_video    = new Video_v2(this);
		_inter    = new Inter_v4(this);
		_mult     = new Mult_v2(this);
		_draw     = new Draw_v2(this);
		_map      = new Map_v2(this);
		_goblin   = new Goblin_v4(this);
		_scenery  = new Scenery_v2(this);
		_saveLoad = new SaveLoad_v4(this, _targetName.c_str());
		break;

	case kGameTypeDynasty:
		_init     = new Init_v3(this);
		_video    = new Video_v2(this);
		_inter    = new Inter_v5(this);
		_mult     = new Mult_v2(this);
		_draw     = new Draw_v2(this);
		_map      = new Map_v2(this);
		_goblin   = new Goblin_v4(this);
		_scenery  = new Scenery_v2(this);
		_saveLoad = new SaveLoad(this);
		break;

	case kGameTypeUrban:
		_init     = new Init_v6(this);
		_video    = new Video_v6(this);
		_inter    = new Inter_v6(this);
		_mult     = new Mult_v2(this);
		_draw     = new Draw_v2(this);
		_map      = new Map_v2(this);
		_goblin   = new Goblin_v4(this);
		_scenery  = new Scenery_v2(this);
		_saveLoad = new SaveLoad_v6(this, _targetName.c_str());
		break;

	case kGameTypePlaytoons:
	case kGameTypeBambou:
		_init     = new Init_v2(this);
		_video    = new Video_v6(this);
		_inter    = new Inter_Playtoons(this);
		_mult     = new Mult_v2(this);
		_draw     = new Draw_Playtoons(this);
		_map      = new Map_v2(this);
		_goblin   = new Goblin_v4(this);
		_scenery  = new Scenery_v2(this);
		_saveLoad = new SaveLoad_Playtoons(this, _targetName.c_str());
		break;

	case kGameTypeAdibou2:
	case kGameTypeAdi2:
	case kGameTypeAdi4:
		_init     = new Init_v7(this);
		_video    = new Video_v6(this);
		_inter    = new Inter_v7(this);
		_mult     = new Mult_v2(this);
		_draw     = new Draw_v2(this);
		_map      = new Map_v2(this);
		_goblin   = new Goblin_v4(this);
		_scenery  = new Scenery_v2(this);
		_saveLoad = new SaveLoad_v7(this, _targetName.c_str());
		break;

	case kGameTypeAdibou1:
		_init     = new Init_v2(this);
		_video    = new Video_v2(this);
		_inter    = new Inter_v2(this);
		_mult     = new Mult_v2(this);
		_draw     = new Draw_v2(this);
		_map      = new Map_v2(this);
		_goblin   = new Goblin_v2(this);
		_scenery  = new Scenery_v2(this);
		_saveLoad = new SaveLoad_v2(this, _targetName.c_str());
		break;

	case kGameTypeAbracadabra:
		_init     = new Init_v2(this);
		_video    = new Video_v2(this);
		_mult     = new Mult_v2(this);
		_draw     = new Draw_v2(this);
		_map      = new Map_v2(this);
		_goblin   = new Goblin_v2(this);
		_scenery  = new Scenery_v2(this);
		_preGob   = new OnceUpon::Abracadabra(this);
		break;

	case kGameTypeBabaYaga:
		_init     = new Init_v2(this);
		_video    = new Video_v2(this);
		_mult     = new Mult_v2(this);
		_draw     = new Draw_v2(this);
		_map      = new Map_v2(this);
		_goblin   = new Goblin_v2(this);
		_scenery  = new Scenery_v2(this);
		_preGob   = new OnceUpon::BabaYaga(this);
		break;

	default:
		deinitGameParts();
		return Common::kUnsupportedGameidError;
	}

	// Setup mixer
	syncSoundSettings();

	if (_inter)
		_inter->setupOpcodes();

	return Common::kNoError;
}

void GobEngine::deinitGameParts() {
	delete _preGob;    _preGob = 0;
	delete _saveLoad;  _saveLoad = 0;
	delete _mult;      _mult = 0;
	delete _vidPlayer; _vidPlayer = 0;
	delete _game;      _game = 0;
	delete _global;    _global = 0;
	delete _goblin;    _goblin = 0;
	delete _init;      _init = 0;
	delete _inter;     _inter = 0;
	delete _map;       _map = 0;
	delete _palAnim;   _palAnim = 0;
	delete _scenery;   _scenery = 0;
	delete _draw;      _draw = 0;
	delete _util;      _util = 0;
	delete _video;     _video = 0;
	delete _sound;     _sound = 0;
	delete _dataIO;    _dataIO = 0;
}

Common::Error GobEngine::initGraphics() {
	if        (is800x600()) {
		warning("GobEngine::initGraphics(): 800x600 games currently unsupported");
		return Common::kUnsupportedGameidError;
	} else if (is640x480()) {
		_width  = 640;
		_height = 480;
		_mode   = 0x18;
	} else {
		_width  = 320;
		_height = 200;
		_mode   = 0x14;
	}

	_video->setSize(is640x480());

	_pixelFormat = g_system->getScreenFormat();

	_video->_surfWidth    = _width;
	_video->_surfHeight   = _height;
	_video->_splitHeight1 = _height;

	_global->_mouseMaxX = _width;
	_global->_mouseMaxY = _height;

	_global->_primarySurfDesc = SurfacePtr(new Surface(_width, _height, _pixelFormat.bytesPerPixel));

	return Common::kNoError;
}

} // End of namespace Gob

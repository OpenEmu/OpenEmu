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
 *
 */

#ifndef TOLTECS_H
#define TOLTECS_H

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/file.h"
#include "common/keyboard.h"
#include "common/random.h"
#include "common/textconsole.h"

#include "engines/engine.h"

#include "graphics/surface.h"

namespace Toltecs {

struct ToltecsGameDescription;

class AnimationPlayer;
class ArchiveReader;
class Input;
class MenuSystem;
class MoviePlayer;
class Music;
class Palette;
class ResourceCache;
class ScriptInterpreter;
class Screen;
class SegmentMap;
class Sound;

enum SysString {
	kStrLoadingPleaseWait,
	kStrWhatCanIDoForYou,
	kStrLoad,
	kStrSave,
	kStrTextOn,
	kStrTextOff,
	kStrVoicesOn,
	kStrVoicesOff,
	kStrVolume,
	kStrPlay,
	kStrQuit,
	kStrLoadGame,
	kStrSaveGame,
	kStrAdjustVolume,
	kStrMaster,
	kStrVoices,
	kStrMusic,
	kStrSoundFx,
	kStrBackground,
	kStrCancel,
	kStrDone,
	kStrAreYouSure,
	kStrYes,
	kStrNo,
	kSysStrCount
};

enum MenuID {
	kMenuIdNone,
	kMenuIdMain,
	kMenuIdSave,
	kMenuIdLoad,
	kMenuIdVolumes
};

class ToltecsEngine : public ::Engine {
	Common::KeyState _keyPressed;

protected:
	Common::Error run();
//	void shutdown();

public:
	ToltecsEngine(OSystem *syst, const ToltecsGameDescription *gameDesc);
	virtual ~ToltecsEngine();

	virtual bool hasFeature(EngineFeature f) const;

	Common::RandomSource *_rnd;
	const ToltecsGameDescription *_gameDescription;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	const Common::String& getTargetName() const { return _targetName; }
	void syncSoundSettings();

	void setupSysStrings();
	void requestSavegame(int slotNum, Common::String &description);
	void requestLoadgame(int slotNum);

	void loadScene(uint resIndex);

	void updateScreen();
	void drawScreen();
	void updateInput();

	void setGuiHeight(int16 guiHeight);

	void setCamera(int16 x, int16 y);
	bool getCameraChanged();
	void scrollCameraUp(int16 delta);
	void scrollCameraDown(int16 delta);
	void scrollCameraLeft(int16 delta);
	void scrollCameraRight(int16 delta);
	void updateCamera();

	void showMenu(MenuID menuId);

	void talk(int16 slotIndex, int16 slotOffset);

	void walk(byte *walkData);

	int16 findRectAtPoint(byte *rectData, int16 x, int16 y, int16 index, int16 itemSize,
		byte *rectDataEnd);

	int _cfgVoicesVolume, _cfgMusicVolume, _cfgSoundFXVolume;
	bool _cfgText, _cfgVoices;
public:

	AnimationPlayer *_anim;
	ArchiveReader *_arc;
	Input *_input;
	MenuSystem *_menuSystem;
	MoviePlayer *_moviePlayer;
	Music *_music;
	Palette *_palette;
	ResourceCache *_res;
	ScriptInterpreter *_script;
	Screen *_screen;
	SegmentMap *_segmap;
	Sound *_sound;

	Common::String _sysStrings[kSysStrCount];

	int _saveLoadRequested;
	int _saveLoadSlot;
	Common::String _saveLoadDescription;

	uint _sceneResIndex;
	int16 _sceneWidth, _sceneHeight;

	int _counter01, _counter02;
	bool _movieSceneFlag;
	byte _flag01;

	int16 _cameraX, _cameraY;
	int16 _newCameraX, _newCameraY;
	int16 _cameraHeight;
	int16 _guiHeight;

	bool _doSpeech, _doText;

	int16 _walkSpeedY, _walkSpeedX;

	Common::KeyState _keyState;
	int16 _mouseX, _mouseY;
	int16 _mouseDblClickTicks;
	bool _mouseWaitForRelease;
	byte _mouseButton;
	int16 _mouseDisabled;
	bool _leftButtonDown, _rightButtonDown;

	const char *getSysString(int index) const { return _sysStrings[index].c_str(); }

	/* Save/load */

	enum kReadSaveHeaderError {
		kRSHENoError = 0,
		kRSHEInvalidType = 1,
		kRSHEInvalidVersion = 2,
		kRSHEIoError = 3
	};

	struct SaveHeader {
		Common::String description;
		uint32 version;
		byte gameID;
		uint32 flags;
		uint32 saveDate;
		uint32 saveTime;
		uint32 playTime;
		Graphics::Surface *thumbnail;
	};

	bool _isSaveAllowed;

	bool canLoadGameStateCurrently() { return _isSaveAllowed; }
	bool canSaveGameStateCurrently() { return _isSaveAllowed; }
	Common::Error loadGameState(int slot);
	Common::Error saveGameState(int slot, const Common::String &description);
	void savegame(const char *filename, const char *description);
	void loadgame(const char *filename);

	const char *getSavegameFilename(int num);
	static Common::String getSavegameFilename(const Common::String &target, int num);

	static kReadSaveHeaderError readSaveHeader(Common::SeekableReadStream *in, bool loadThumbnail, SaveHeader &header);

};

} // End of namespace Toltecs

#endif /* TOLTECS_H */

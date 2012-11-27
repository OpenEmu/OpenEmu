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

#include "common/config-manager.h"
#include "common/events.h"
#include "common/random.h"
#include "common/str.h"
#include "common/error.h"
#include "common/textconsole.h"

#include "base/plugins.h"
#include "base/version.h"

#include "graphics/cursorman.h"

#include "engines/util.h"

#include "audio/mixer.h"

#include "toltecs/toltecs.h"
#include "toltecs/animation.h"
#include "toltecs/menu.h"
#include "toltecs/movie.h"
#include "toltecs/music.h"
#include "toltecs/palette.h"
#include "toltecs/render.h"
#include "toltecs/resource.h"
#include "toltecs/script.h"
#include "toltecs/screen.h"
#include "toltecs/segmap.h"
#include "toltecs/sound.h"
#include "toltecs/microtiles.h"

namespace Toltecs {

struct GameSettings {
	const char *gameid;
	const char *description;
	byte id;
	uint32 features;
	const char *detectname;
};

ToltecsEngine::ToltecsEngine(OSystem *syst, const ToltecsGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	_rnd = new Common::RandomSource("toltecs");
}

ToltecsEngine::~ToltecsEngine() {
	delete _rnd;
}

Common::Error ToltecsEngine::run() {
	initGraphics(640, 400, true);

	_isSaveAllowed = true;

	_counter01 = 0;
	_counter02 = 0;
	_movieSceneFlag = false;
	_flag01 = 0;

	_saveLoadRequested = 0;

	_cameraX = 0;
	_cameraY = 0;
	_newCameraX = 0;
	_newCameraY = 0;
	_cameraHeight = 0;

	_guiHeight = 26;

	_sceneWidth = 0;
	_sceneHeight = 0;

	_doSpeech = true;
	_doText = true;

	_walkSpeedY = 5;
	_walkSpeedX = 1;

	_mouseX = 0;
	_mouseY = 0;
	_mouseDblClickTicks = 60;
	_mouseWaitForRelease = false;
	_mouseButton = 0;
	_mouseDisabled = 0;
	_leftButtonDown = false;
	_rightButtonDown = false;

	_arc = new ArchiveReader();
	_arc->openArchive("WESTERN");

	_res = new ResourceCache(this);

	_screen = new Screen(this);

	_script = new ScriptInterpreter(this);
	_anim = new AnimationPlayer(this);
	_palette = new Palette(this);
	_segmap = new SegmentMap(this);
	_moviePlayer = new MoviePlayer(this);
	_music = new Music(_arc);
	_menuSystem = new MenuSystem(this);

	_sound = new Sound(this);

	_cfgText = ConfMan.getBool("subtitles");
	_cfgVoices = !ConfMan.getBool("speech_mute");

	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, mute ? 0 : ConfMan.getInt("speech_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType,  mute ? 0 : ConfMan.getInt("music_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType,    mute ? 0 : ConfMan.getInt("sfx_volume"));
	syncSoundSettings();

	CursorMan.showMouse(true);

	setupSysStrings();

#if 0
	// Menu test
	_screen->registerFont(0, 0x0D);
	_screen->registerFont(1, 0x0E);
	_screen->loadMouseCursor(12);
	_palette->loadAddPalette(9, 224);
	_palette->setDeltaPalette(_palette->getMainPalette(), 7, 0, 31, 224);
	_screen->finishTalkTextItems();
	_screen->clearSprites();
	_menuSystem->run();
	/*
	while (1) {
		//updateInput();
		_menuSystem->update();
		updateScreen();
	}
	*/
	return Common::kNoError;
#endif

	// Start main game loop
	setTotalPlayTime(0);
	_script->loadScript(0, 0);
	_script->setMainScript(0);
	if (ConfMan.hasKey("save_slot")) {
		int saveSlot = ConfMan.getInt("save_slot");
		if (saveSlot >= 0 && saveSlot <= 99) {
			_screen->loadMouseCursor(12);
			loadGameState(saveSlot);
		}
	}
	_script->runScript();

	_music->stopSequence();
	_sound->stopAll();

	delete _arc;
	delete _res;
	delete _screen;
	delete _script;
	delete _anim;
	delete _palette;
	delete _segmap;
	delete _music;
	delete _moviePlayer;
	delete _menuSystem;

	delete _sound;

	return Common::kNoError;
}

void ToltecsEngine::setupSysStrings() {
	Resource *sysStringsResource = _res->load(15);
	const char *sysStrings = (const char*)sysStringsResource->data;
	for (int i = 0; i < kSysStrCount; i++) {
		debug(1, "sysStrings[%d] = [%s]", i, sysStrings);
		_sysStrings[i] = sysStrings;
		sysStrings += strlen(sysStrings) + 1;
	}
	// TODO: Set yes/no chars
}

void ToltecsEngine::requestSavegame(int slotNum, Common::String &description) {
	_saveLoadRequested = 2;
	_saveLoadSlot = slotNum;
	_saveLoadDescription = description;
}

void ToltecsEngine::requestLoadgame(int slotNum) {
	_saveLoadRequested = 1;
	_saveLoadSlot = slotNum;
}

void ToltecsEngine::loadScene(uint resIndex) {

	Resource *sceneResource = _res->load(resIndex);
	byte *scene = sceneResource->data;

	uint32 imageSize = READ_LE_UINT32(scene);
	_sceneResIndex = resIndex;
	_sceneHeight = READ_LE_UINT16(scene + 4);
	_sceneWidth = READ_LE_UINT16(scene + 6);

	// Load scene palette
	_palette->loadAddPaletteFrom(scene + 8, 0, 128);

	// Load scene background
	byte *source = scene + 392;
	byte *destp = _screen->_backScreen;
	byte *destEnd = destp + _sceneWidth * _sceneHeight;
 	while (destp < destEnd) {
		int count = 1;
		byte pixel = *source++;
		if (pixel & 0x80) {
			pixel &= 0x7F;
			count = *source++;
			count += 2;
		}
		memset(destp, pixel, count);
		destp += count;
	}

	debug(0, "_sceneWidth = %d; _sceneHeight = %d", _sceneWidth, _sceneHeight);

	// Load scene segmap
 	_segmap->load(scene + imageSize + 4);

	_screen->_fullRefresh = true;
	_screen->_renderQueue->clear();

}

void ToltecsEngine::updateScreen() {

	_sound->updateSpeech();

	_screen->updateShakeScreen();

	// TODO: Set quit flag
	if (shouldQuit())
		return;

	if (!_movieSceneFlag)
		updateInput();
	else
		_mouseButton = 0;

	// TODO? Check keyb

	_counter01--;
	if (_counter01 <= 0) {
		_counter01 = MIN(_counter02, 30);
		_counter02 = 0;
		drawScreen();
		_flag01 = 1;
		_counter02 = 1;
	} else {
		_screen->clearSprites();
		_flag01 = 0;
	}

	static uint32 prevUpdateTime = 0;
	uint32 currUpdateTime;
	do {
		currUpdateTime = _system->getMillis();
		_counter02 = (currUpdateTime - prevUpdateTime) / 13;
	} while (_counter02 == 0);
	prevUpdateTime = currUpdateTime;

}

void ToltecsEngine::drawScreen() {
	// FIXME: Quick hack, sometimes cameraY was negative (the code in updateCamera was at fault)
	if (_cameraY < 0) _cameraY = 0;

	_segmap->addMasksToRenderQueue();
	_screen->addTalkTextItemsToRenderQueue();

	_screen->_renderQueue->update();

	//debug("_guiHeight = %d\n", _guiHeight);

	if (_screen->_guiRefresh && _guiHeight > 0 && _cameraHeight > 0) {
		// Update the GUI when needed and it's visible
		_system->copyRectToScreen(_screen->_frontScreen + _cameraHeight * 640,
			640, 0, _cameraHeight, 640, _guiHeight);
		_screen->_guiRefresh = false;
	}

	_system->updateScreen();
	_system->delayMillis(10);

	updateCamera();
}

void ToltecsEngine::updateInput() {

	Common::Event event;
	Common::EventManager *eventMan = _system->getEventManager();
	while (eventMan->pollEvent(event)) {
	switch (event.type) {
		case Common::EVENT_KEYDOWN:
			_keyState = event.kbd;

			//debug("key: flags = %02X; keycode = %d", _keyState.flags, _keyState.keycode);

			switch (event.kbd.keycode) {
			case Common::KEYCODE_F5:
				showMenu(kMenuIdSave);
				break;
			case Common::KEYCODE_F7:
				showMenu(kMenuIdLoad);
				break;
			case Common::KEYCODE_SPACE:
				// Skip current dialog line, if a dialog is active
				if (_screen->getTalkTextDuration() > 0) {
					_sound->stopSpeech();
					_screen->finishTalkTextItems();
					_keyState.reset();	// event consumed
				}
				break;
			default:
				break;
			}

			break;
		case Common::EVENT_KEYUP:
			_keyState.reset();
			break;
		case Common::EVENT_QUIT:
			quitGame();
			break;
		case Common::EVENT_MOUSEMOVE:
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			break;
		case Common::EVENT_LBUTTONDOWN:
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			_leftButtonDown = true;
			break;
		case Common::EVENT_LBUTTONUP:
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			_leftButtonDown = false;
			break;
		case Common::EVENT_RBUTTONDOWN:
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			_rightButtonDown = true;
			break;
		case Common::EVENT_RBUTTONUP:
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			_rightButtonDown = false;
			break;
		default:
			break;
		}
	}

	if (!_mouseDisabled) {

		if (_mouseDblClickTicks > 0)
			_mouseDblClickTicks--;

		byte mouseButtons = 0;
		if (_leftButtonDown)
			mouseButtons |= 1;
		if (_rightButtonDown)
			mouseButtons |= 2;

		if (mouseButtons != 0) {
			if (!_mouseWaitForRelease) {
				_mouseButton = mouseButtons;
				if (_mouseDblClickTicks > 0)
					_mouseButton = 0x80;
				//if (_mouseButton == 0x80) debug("DBL!");
				_mouseDblClickTicks = 30; // maybe TODO
				_mouseWaitForRelease = true;
			} else {
				_mouseButton = 0;
			}
		} else {
			_mouseWaitForRelease = false;
			_mouseButton = 0;
		}

	}

}

void ToltecsEngine::setGuiHeight(int16 guiHeight) {
	if (guiHeight != _guiHeight) {
		_guiHeight = guiHeight;
		_cameraHeight = 400 - _guiHeight;
		_screen->_guiRefresh = true;
		debug(0, "ToltecsEngine::setGuiHeight() _guiHeight = %d; _cameraHeight = %d", _guiHeight, _cameraHeight);
		// TODO: clearScreen();
	}
}

void ToltecsEngine::setCamera(int16 x, int16 y) {
	_screen->finishTalkTextItems();

	_screen->clearSprites();

	_cameraX = x;
	_newCameraX = x;

	_cameraY = y;
	_newCameraY = y;
}

bool ToltecsEngine::getCameraChanged() {
	return _cameraX != _newCameraX || _cameraY != _newCameraY;
}

void ToltecsEngine::scrollCameraUp(int16 delta) {
	if (_newCameraY > 0) {
		if (_newCameraY < delta)
			_newCameraY = 0;
		else
			_newCameraY -= delta;
	}
}

void ToltecsEngine::scrollCameraDown(int16 delta) {
	debug(0, "ToltecsEngine::scrollCameraDown(%d)", delta);
	if (_newCameraY != _sceneHeight - _cameraHeight) {
		if (_sceneHeight - _cameraHeight < _newCameraY + delta)
			delta += (_sceneHeight - _cameraHeight) - (delta + _newCameraY);
		_newCameraY += delta;
		debug(0, "ToltecsEngine::scrollCameraDown() _newCameraY = %d; delta = %d", _newCameraY, delta);
	}
}

void ToltecsEngine::scrollCameraLeft(int16 delta) {
	if (_newCameraX > 0) {
		if (_newCameraX < delta)
			_newCameraX = 0;
		else
			_newCameraX -= delta;
	}
}

void ToltecsEngine::scrollCameraRight(int16 delta) {
	debug(0, "ToltecsEngine::scrollCameraRight(%d)", delta);
	if (_newCameraX != _sceneWidth - 640) {
		if (_sceneWidth - 640 < delta + _newCameraX)
			delta += (_sceneWidth - 640) - (delta + _newCameraX);
		_newCameraX += delta;
		debug(0, "ToltecsEngine::scrollCameraRight() _newCameraX = %d; delta = %d", _newCameraY, delta);
	}
}

void ToltecsEngine::updateCamera() {

	if (_cameraX != _newCameraX) {
		_cameraX = _newCameraX;
		_screen->_fullRefresh = true;
		_screen->finishTalkTextItems();
	}

	if (_cameraY != _newCameraY) {
		_cameraY = _newCameraY;
		_screen->_fullRefresh = true;
		_screen->finishTalkTextItems();
	}

	//debug(0, "ToltecsEngine::updateCamera() _cameraX = %d; _cameraY = %d", _cameraX, _cameraY);

}

void ToltecsEngine::talk(int16 slotIndex, int16 slotOffset) {

	byte *scanData = _script->getSlotData(slotIndex) + slotOffset;

	while (*scanData < 0xF0) {
		if (*scanData == 0x19) {
			scanData++;
		} else if (*scanData == 0x14) {
			scanData++;
		} else if (*scanData == 0x0A) {
			scanData += 4;
		} else if (*scanData < 0x0A) {
			scanData++;
		}
		scanData++;
	}

	if (*scanData == 0xFE) {
		if (_doSpeech) {
			int16 resIndex = READ_LE_UINT16(scanData + 1);
			debug(0, "ToltecsEngine::talk() playSound(resIndex: %d)", resIndex);
			_sound->playSpeech(resIndex);
		}
		if (_doText) {
			_screen->updateTalkText(slotIndex, slotOffset);
		} else {
			_screen->keepTalkTextItemsAlive();
		}
	} else {
		_screen->updateTalkText(slotIndex, slotOffset);
	}

}

void ToltecsEngine::walk(byte *walkData) {

	int16 xdelta, ydelta, v8, v10, v11;
	int16 xstep, ystep;
	ScriptWalk walkInfo;

	walkInfo.y = READ_LE_UINT16(walkData + 0);
	walkInfo.x = READ_LE_UINT16(walkData + 2);
	walkInfo.y1 = READ_LE_UINT16(walkData + 4);
	walkInfo.x1 = READ_LE_UINT16(walkData + 6);
	walkInfo.y2 = READ_LE_UINT16(walkData + 8);
	walkInfo.x2 = READ_LE_UINT16(walkData + 10);
	walkInfo.yerror = READ_LE_UINT16(walkData + 12);
	walkInfo.xerror = READ_LE_UINT16(walkData + 14);
	walkInfo.mulValue = READ_LE_UINT16(walkData + 16);
	walkInfo.scaling = READ_LE_UINT16(walkData + 18);

	walkInfo.scaling = -_segmap->getScalingAtPoint(walkInfo.x, walkInfo.y);

	if (walkInfo.y1 < walkInfo.y2)
		ystep = -1;
	else
		ystep = 1;
	ydelta = ABS(walkInfo.y1 - walkInfo.y2) * _walkSpeedY;

	if (walkInfo.x1 < walkInfo.x2)
		xstep = -1;
	else
		xstep = 1;
	xdelta = ABS(walkInfo.x1 - walkInfo.x2) * _walkSpeedX;

	debug(0, "ToltecsEngine::walk() xdelta = %d; ydelta = %d", xdelta, ydelta);

	if (xdelta > ydelta)
		SWAP(xdelta, ydelta);

	v8 = 100 * xdelta;
	if (v8 != 0) {
		if (walkInfo.scaling > 0)
			v8 -= v8 * ABS(walkInfo.scaling) / 100;
		else
			v8 += v8 * ABS(walkInfo.scaling) / 100;
		if (ydelta != 0)
			v8 /= ydelta;
	}

	if (ydelta > ABS(walkInfo.x1 - walkInfo.x2) * _walkSpeedX) {
		v10 = 100 - walkInfo.scaling;
		v11 = v8;
  	} else {
		v10 = v8;
  		v11 = 100 - walkInfo.scaling;
	}

	walkInfo.yerror += walkInfo.mulValue * v10;
	while (walkInfo.yerror >= 100 * _walkSpeedY) {
		walkInfo.yerror -= 100 * _walkSpeedY;
		if (walkInfo.y == walkInfo.y1) {
			walkInfo.x = walkInfo.x1;
			break;
		}
		walkInfo.y += ystep;
	}

	walkInfo.xerror += walkInfo.mulValue * v11;
	while (walkInfo.xerror >= 100 * _walkSpeedX) {
		walkInfo.xerror -= 100 * _walkSpeedX;
		if (walkInfo.x == walkInfo.x1) {
			walkInfo.y = walkInfo.y1;
			break;
		}
		walkInfo.x += xstep;
	}

	WRITE_LE_UINT16(walkData + 0, walkInfo.y);
	WRITE_LE_UINT16(walkData + 2, walkInfo.x);
	WRITE_LE_UINT16(walkData + 4, walkInfo.y1);
	WRITE_LE_UINT16(walkData + 6, walkInfo.x1);
	WRITE_LE_UINT16(walkData + 8, walkInfo.y2);
	WRITE_LE_UINT16(walkData + 10, walkInfo.x2);
	WRITE_LE_UINT16(walkData + 12, walkInfo.yerror);
	WRITE_LE_UINT16(walkData + 14, walkInfo.xerror);
	WRITE_LE_UINT16(walkData + 16, walkInfo.mulValue);
	WRITE_LE_UINT16(walkData + 18, walkInfo.scaling);

}

int16 ToltecsEngine::findRectAtPoint(byte *rectData, int16 x, int16 y, int16 index, int16 itemSize,
	byte *rectDataEnd) {

	rectData += index * itemSize;

	while (rectData < rectDataEnd) {
		int16 rectY = READ_LE_UINT16(rectData);
		if (rectY == -10)
			break;
		int16 rectX = READ_LE_UINT16(rectData + 2);
		int16 rectH = READ_LE_UINT16(rectData + 4);
		int16 rectW = READ_LE_UINT16(rectData + 6);

		debug(0, "x = %d; y = %d; x1 = %d; y2 = %d; w = %d; h = %d",
			x, y, rectX, rectY, rectW, rectH);

		if (x >= rectX && x <= rectX + rectW && y >= rectY && y <= rectY + rectH) {
			return index;
		}
		index++;
		rectData += itemSize;
	}

	return -1;
}

void ToltecsEngine::showMenu(MenuID menuId) {
	_screen->loadMouseCursor(12);
	_palette->loadAddPalette(9, 224);
	_palette->setDeltaPalette(_palette->getMainPalette(), 7, 0, 31, 224);
	_screen->finishTalkTextItems();
	_screen->clearSprites();
	CursorMan.showMouse(true);
	_menuSystem->run(menuId);
	_keyState.reset();
	_script->setSwitchLocalDataNear(true);
}

void ToltecsEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	_cfgVoicesVolume  = (mute ? 0 : ConfMan.getInt("speech_volume")) * 20 / Audio::Mixer::kMaxChannelVolume;
	_cfgMusicVolume   = (mute ? 0 : ConfMan.getInt("music_volume"))  * 20 / Audio::Mixer::kMaxChannelVolume;
	_cfgSoundFXVolume = (mute ? 0 : ConfMan.getInt("sfx_volume"))    * 20 / Audio::Mixer::kMaxChannelVolume;
}

} // End of namespace Toltecs

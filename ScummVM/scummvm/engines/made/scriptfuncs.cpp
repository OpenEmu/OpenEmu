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

#include "made/scriptfuncs.h"
#include "made/made.h"
#include "made/screen.h"
#include "made/music.h"
#include "made/database.h"
#include "made/pmvplayer.h"

#include "audio/softsynth/pcspk.h"

#include "backends/audiocd/audiocd.h"

#include "graphics/cursorman.h"
#include "graphics/surface.h"

namespace Made {

ScriptFunctions::ScriptFunctions(MadeEngine *vm) : _vm(vm), _soundStarted(false) {
	// Initialize the two tone generators
	_pcSpeaker1 = new Audio::PCSpeaker();
	_pcSpeaker2 = new Audio::PCSpeaker();
	_vm->_system->getMixer()->playStream(Audio::Mixer::kMusicSoundType, &_pcSpeakerHandle1, _pcSpeaker1);
	_vm->_system->getMixer()->playStream(Audio::Mixer::kMusicSoundType, &_pcSpeakerHandle2, _pcSpeaker2);
}

ScriptFunctions::~ScriptFunctions() {
	for (uint i = 0; i < _externalFuncs.size(); ++i)
			delete _externalFuncs[i];

	_vm->_system->getMixer()->stopHandle(_pcSpeakerHandle1);
	_vm->_system->getMixer()->stopHandle(_pcSpeakerHandle2);
}

typedef Common::Functor2Mem<int16, int16*, int16, ScriptFunctions> ExternalScriptFunc;
#define External(x) \
	_externalFuncs.push_back(new ExternalScriptFunc(this, &ScriptFunctions::x));  \
	_externalFuncNames.push_back(#x);
void ScriptFunctions::setupExternalsTable() {

	External(sfSystemCall);
	External(sfInitGraf);
	External(sfRestoreGraf);
	External(sfDrawPicture);
	External(sfClearScreen);
	External(sfShowPage);
	External(sfPollEvent);
	External(sfGetMouseX);
	External(sfGetMouseY);
	External(sfGetKey);
	External(sfSetVisualEffect);
	External(sfPlaySound);
	External(sfPlayMusic);
	External(sfStopMusic);
	External(sfIsMusicPlaying);
	External(sfSetTextPos);
	External(sfFlashScreen);
	External(sfPlayNote);
	External(sfStopNote);
	External(sfPlayTele);
	External(sfStopTele);
	External(sfHideMouseCursor);
	External(sfShowMouseCursor);
	External(sfGetMusicBeat);
	External(sfSetScreenLock);
	External(sfAddSprite);
	External(sfFreeAnim);
	External(sfDrawSprite);
	External(sfEraseSprites);
	External(sfUpdateSprites);
	External(sfGetTimer);
	External(sfSetTimer);
	External(sfResetTimer);
	External(sfAllocTimer);
	External(sfFreeTimer);
	External(sfSetPaletteLock);
	External(sfSetFont);
	External(sfDrawText);
	External(sfHomeText);
	External(sfSetTextRect);
	External(sfSetTextXY);
	External(sfSetFontDropShadow);
	External(sfSetFontColor);
	External(sfSetFontOutline);
	External(sfLoadMouseCursor);
	External(sfSetSpriteGround);
	External(sfLoadResText);

	if (_vm->getGameID() == GID_MANHOLE || _vm->getGameID() == GID_LGOP2 || _vm->getGameID() == GID_RODNEY) {
		External(sfAddScreenMask);
		External(sfSetSpriteMask);
	} else if (_vm->getGameID() == GID_RTZ) {
		External(sfSetClipArea);
		External(sfSetSpriteClip);
	}

	External(sfSoundPlaying);
	External(sfStopSound);
	External(sfPlayVoice);

	if (_vm->getGameID() == GID_MANHOLE || _vm->getGameID() == GID_RTZ || _vm->getGameID() == GID_RODNEY) {
		External(sfPlayCd);
		External(sfStopCd);
		External(sfGetCdStatus);
		External(sfGetCdTime);
		External(sfPlayCdSegment);
	}

	if (_vm->getGameID() == GID_RTZ) {
		External(sfPrintf);
		External(sfClearMono);
		External(sfGetSoundEnergy);
		External(sfClearText);
		External(sfAnimText);
		External(sfGetTextWidth);
		External(sfPlayMovie);
		External(sfLoadSound);
		External(sfLoadMusic);
		External(sfLoadPicture);
		External(sfSetMusicVolume);
		External(sfRestartEvents);
		External(sfPlaceSprite);
		External(sfPlaceText);
		External(sfDeleteChannel);
		External(sfGetChannelType);
		External(sfSetChannelState);
		External(sfSetChannelLocation);
		External(sfSetChannelContent);
		External(sfSetExcludeArea);
		External(sfSetSpriteExclude);
		External(sfGetChannelState);
		External(sfPlaceAnim);
		External(sfSetAnimFrame);
		External(sfGetAnimFrame);
		External(sfGetAnimFrameCount);
		External(sfGetPictureWidth);
		External(sfGetPictureHeight);
		External(sfSetSoundRate);
		External(sfDrawAnimPic);
		External(sfLoadAnim);
		External(sfReadText);
		External(sfReadMenu);
		External(sfDrawMenu);
		External(sfGetMenuCount);
		External(sfSaveGame);
		External(sfLoadGame);
		External(sfGetGameDescription);
		External(sfShakeScreen);
		External(sfPlaceMenu);
		External(sfSetSoundVolume);
		External(sfGetSynthType);
		External(sfIsSlowSystem);
	}

}
#undef External

int16 ScriptFunctions::sfSystemCall(int16 argc, int16 *argv) {
	// This opcode is empty.
	return 0;
}

int16 ScriptFunctions::sfInitGraf(int16 argc, int16 *argv) {
	// This opcode is empty.
	return 0;
}

int16 ScriptFunctions::sfRestoreGraf(int16 argc, int16 *argv) {
	// This opcode is empty.
	return 0;
}

int16 ScriptFunctions::sfDrawPicture(int16 argc, int16 *argv) {
	return _vm->_screen->drawPic(argv[4], argv[3], argv[2], argv[1], argv[0]);
}

int16 ScriptFunctions::sfClearScreen(int16 argc, int16 *argv) {
	if (_vm->_screen->isScreenLocked())
		return 0;
	if (_vm->_autoStopSound) {
		stopSound();
		_vm->_autoStopSound = false;
	}
	_vm->_screen->clearScreen();
	return 0;
}

int16 ScriptFunctions::sfShowPage(int16 argc, int16 *argv) {
	_vm->_screen->show();
	// NOTE: We need to return something != 0 here or some game scripts won't
	//       work correctly. The actual meaning of this value is unknown to me.
	//       0x38 was found out by analyzing debug output of the original engine.
	return 0x38;
}

int16 ScriptFunctions::sfPollEvent(int16 argc, int16 *argv) {
	_vm->handleEvents();
	_vm->_screen->updateScreenAndWait(10);

	int16 eventNum = _vm->_eventNum;
	_vm->_eventNum = 0;

	return eventNum;
}

int16 ScriptFunctions::sfGetMouseX(int16 argc, int16 *argv) {
	return _vm->_eventMouseX;
}

int16 ScriptFunctions::sfGetMouseY(int16 argc, int16 *argv) {
	return _vm->_eventMouseY;
}

int16 ScriptFunctions::sfGetKey(int16 argc, int16 *argv) {
	return _vm->_eventKey;
}

int16 ScriptFunctions::sfSetVisualEffect(int16 argc, int16 *argv) {
	_vm->_screen->setVisualEffectNum(argv[0]);
	return 0;
}

int16 ScriptFunctions::sfPlaySound(int16 argc, int16 *argv) {
	int16 soundNum = argv[0];
	_vm->_autoStopSound = false;
	stopSound();
	if (argc > 1) {
		soundNum = argv[1];
		_vm->_autoStopSound = (argv[0] == 1);
	}
	if (soundNum > 0) {
		SoundResource *soundRes = _vm->_res->getSound(soundNum);
		_vm->_mixer->playStream(Audio::Mixer::kPlainSoundType, &_audioStreamHandle,
			soundRes->getAudioStream(_vm->_soundRate, false));
		_vm->_soundEnergyArray = soundRes->getSoundEnergyArray();
		_vm->_soundEnergyIndex = 0;
		_soundStarted = true;
		_soundResource = soundRes;
	}
	return 0;
}

int16 ScriptFunctions::sfPlayMusic(int16 argc, int16 *argv) {
	int16 musicNum = argv[0];

	_vm->_musicBeatStart = _vm->_system->getMillis();

	if (_vm->getGameID() == GID_RTZ) {
		if (musicNum > 0) {
			_musicRes = _vm->_res->getXmidi(musicNum);
			if (_musicRes)
				_vm->_music->playXMIDI(_musicRes);
		}
	} else {
		// HACK: music number 2 in LGOP2 is file MT32SET.TON, which
		// is used to set the MT32 instruments. This is not loaded
		// correctly and the game freezes, and since we don't support
		// MT32 music yet, we ignore it here
		// FIXME: Remove this hack and handle this file properly
		if (_vm->getGameID() == GID_LGOP2 && musicNum == 2)
			return 0;
		if (musicNum > 0) {
			_musicRes = _vm->_res->getMidi(musicNum);
			if (_musicRes)
				_vm->_music->playSMF(_musicRes);
		}
	}

	return 0;
}

int16 ScriptFunctions::sfStopMusic(int16 argc, int16 *argv) {
	if (_vm->_music->isPlaying() && _musicRes) {
		_vm->_music->stop();
		_vm->_res->freeResource(_musicRes);
		_musicRes = NULL;
	}
	return 0;
}

int16 ScriptFunctions::sfIsMusicPlaying(int16 argc, int16 *argv) {
	if (_vm->_music->isPlaying())
		return 1;
	else
		return 0;
}

int16 ScriptFunctions::sfSetTextPos(int16 argc, int16 *argv) {
	// Used in Manhole:NE
	// This seems to be some kind of low-level opcode.
	// The original engine calls int 10h to set the VGA cursor position.
	// Since this seems to be used for debugging purposes only it's left out.
	return 0;
}

int16 ScriptFunctions::sfFlashScreen(int16 argc, int16 *argv) {
	_vm->_screen->flash(argv[0]);
	return 0;
}

int16 ScriptFunctions::sfPlayNote(int16 argc, int16 *argv) {
	// This is used when using the piano in the desk screen inside the ship
	// in The Manhole (EGA/NE).

	// It takes 2 parameters:
	// The first parameter is the note number of the key pressed + 1
	// The second parameter is some sort of modifier (volume, perhaps?),
	// depending on which of the 3 keys on the right has been pressed.
	// This value seems to be [12, 14] in NE and [1, 3] in EGA.

	// Note frequencies based on http://www.phy.mtu.edu/~suits/notefreqs.html
	static const int freqTable[] = {
		16, 17, 18, 19, 21, 22, 23, 24, 26, 28, 29,
		30, 32, 35, 37, 39, 41, 44, 46, 49, 52, 55,
		58, 62, 65, 69, 73, 77, 82, 87, 93, 98, 104,
		110, 117, 123, 131, 139, 147, 156, 165, 175, 195,
		196, 208, 220, 233, 247, 262, 277, 294, 311, 330,
		349, 370, 392, 415, 440, 466, 494, 523, 554, 587,
		622, 659, 698, 740, 784, 831, 880, 932, 988, 1047,
		1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760,
		1865, 1976, 2093, 2217, 2349, 2489, 2637, 2794, 2960,
		3136, 3322, 3529, 3729, 3951, 4186, 4435, 4697, 4978
	};

	debug(4, "sfPlayNote: Note = %d, Volume(?) = %d", argv[0] - 1, argv[1]);

	_pcSpeaker1->play(Audio::PCSpeaker::kWaveFormSine, freqTable[argv[0] - 1], -1);

	// TODO: Figure out what to do with the second parameter
	//_pcSpeaker1->setVolume(argv[1]);

	return 0;
}

int16 ScriptFunctions::sfStopNote(int16 argc, int16 *argv) {
	// Used in the same place as sfPlayNote, with the same parameters
	// We just stop the wave generator here
	_pcSpeaker1->stop();
	return 0;
}

int16 ScriptFunctions::sfPlayTele(int16 argc, int16 *argv) {
	// This is used when pressing the phone keys while using the phone in
	// the desk screen inside the ship in The Manhole (EGA/NE).
	// It takes 1 parameter, the key pressed (0-9, 10 for asterisk, 11 for hash)

	// A telephone keypad uses a two tones for each key.
	// See http://en.wikipedia.org/wiki/Telephone_keypad for more info

	static const int freqTable1[] = {
		1336, 1209, 1336, 1477,
		1209, 1336, 1477, 1209,
		1336, 1477, 1209, 1477
	};

	static const int freqTable2[] = {
		941, 697, 697, 697,
		770, 770, 770, 852,
		852, 852, 941, 941
	};

	debug(4, "sfPlayTele: Button = %d", argv[0]);

	_pcSpeaker1->play(Audio::PCSpeaker::kWaveFormSine, freqTable1[argv[0]], -1);
	_pcSpeaker2->play(Audio::PCSpeaker::kWaveFormSine, freqTable2[argv[0]], -1);
	return 0;
}

int16 ScriptFunctions::sfStopTele(int16 argc, int16 *argv) {
	// Used in the same place as sfPlayTele, with the same parameters
	// We just stop both wave generators here
	_pcSpeaker1->stop();
	_pcSpeaker2->stop();
	return 0;
}

int16 ScriptFunctions::sfHideMouseCursor(int16 argc, int16 *argv) {
	_vm->_system->showMouse(false);
	return 0;
}

int16 ScriptFunctions::sfShowMouseCursor(int16 argc, int16 *argv) {
	_vm->_system->showMouse(true);
	return 0;
}

int16 ScriptFunctions::sfGetMusicBeat(int16 argc, int16 *argv) {
	// This is used as timer in some games
	return (_vm->_system->getMillis() - _vm->_musicBeatStart) / 360;
}

int16 ScriptFunctions::sfSetScreenLock(int16 argc, int16 *argv) {
	_vm->_screen->setScreenLock(argv[0] != 0);
	return 0;
}

int16 ScriptFunctions::sfAddSprite(int16 argc, int16 *argv) {
	if (_vm->getGameID() == GID_RTZ) {
		// Unused in RTZ
		return 0;
	} if (_vm->getGameID() == GID_LGOP2 || _vm->getGameID() == GID_MANHOLE || _vm->getGameID() == GID_RODNEY) {
		return _vm->_screen->addToSpriteList(argv[2], argv[1], argv[0]);
	} else {
		return 0;
	}
}

int16 ScriptFunctions::sfFreeAnim(int16 argc, int16 *argv) {
	_vm->_screen->clearChannels();
	if (_vm->getGameID() == GID_LGOP2 || _vm->getGameID() == GID_MANHOLE || _vm->getGameID() == GID_RODNEY) {
		_vm->_screen->clearSpriteList();
	}
	return 0;
}

int16 ScriptFunctions::sfDrawSprite(int16 argc, int16 *argv) {
	if (_vm->getGameID() == GID_RTZ) {
		return _vm->_screen->drawSprite(argv[2], argv[1], argv[0]);
	} if (_vm->getGameID() == GID_LGOP2 || _vm->getGameID() == GID_MANHOLE || _vm->getGameID() == GID_RODNEY) {
		SpriteListItem item = _vm->_screen->getFromSpriteList(argv[2]);
		int16 channelIndex = _vm->_screen->drawSprite(item.index, argv[1] - item.xofs, argv[0] - item.yofs);
		_vm->_screen->setChannelUseMask(channelIndex);
		return 0;
	} else {
		return 0;
	}
}

int16 ScriptFunctions::sfEraseSprites(int16 argc, int16 *argv) {
	_vm->_screen->clearChannels();
	return 0;
}

int16 ScriptFunctions::sfUpdateSprites(int16 argc, int16 *argv) {
	_vm->_screen->updateSprites();
	return 0;
}

int16 ScriptFunctions::sfGetTimer(int16 argc, int16 *argv) {
	return _vm->getTimer(argv[0]);
}

int16 ScriptFunctions::sfSetTimer(int16 argc, int16 *argv) {
	_vm->setTimer(argv[1], argv[0]);
	return 0;
}

int16 ScriptFunctions::sfResetTimer(int16 argc, int16 *argv) {
	_vm->resetTimer(argv[0]);
	return 0;
}

int16 ScriptFunctions::sfAllocTimer(int16 argc, int16 *argv) {
	return _vm->allocTimer();
}

int16 ScriptFunctions::sfFreeTimer(int16 argc, int16 *argv) {
	_vm->freeTimer(argv[0]);
	return 0;
}

int16 ScriptFunctions::sfSetPaletteLock(int16 argc, int16 *argv) {
	_vm->_screen->setPaletteLock(argv[0] != 0);
	return 0;
}

int16 ScriptFunctions::sfSetFont(int16 argc, int16 *argv) {
	_vm->_screen->setFont(argv[0]);
	return 0;
}

int16 ScriptFunctions::sfDrawText(int16 argc, int16 *argv) {

	const char *text = NULL;

	if (_vm->getGameID() == GID_RTZ) {
		text = _vm->_dat->getObjectString(argv[argc - 1]);
	} if (_vm->getGameID() == GID_LGOP2 || _vm->getGameID() == GID_MANHOLE || _vm->getGameID() == GID_RODNEY) {
		text = _vm->_dat->getString(argv[argc - 1]);
	}

	if (text) {
		Common::String finalText;
		switch (argc) {
		case 1:
			finalText = text;
			break;
		case 2:
			finalText = Common::String::format(text, argv[0]);
			break;
		case 3:
			finalText = Common::String::format(text, argv[1], argv[0]);
			break;
		case 4:
			finalText = Common::String::format(text, argv[2], argv[1], argv[0]);
			break;
		case 5:
			finalText = Common::String::format(text, argv[3], argv[2], argv[1], argv[0]);
			break;
		default:
			// Leave it empty
			break;
		}
		_vm->_screen->printText(finalText.c_str());
	}

	return 0;
}

int16 ScriptFunctions::sfHomeText(int16 argc, int16 *argv) {
	_vm->_screen->homeText();
	return 0;
}

int16 ScriptFunctions::sfSetTextRect(int16 argc, int16 *argv) {
	// Used in the save/load screens of RtZ, and perhaps other places as well
	int16 x1 = CLIP<int16>(argv[4], 1, 318);
	int16 y1 = CLIP<int16>(argv[3], 1, 198);
	int16 x2 = CLIP<int16>(argv[2], 1, 318);
	int16 y2 = CLIP<int16>(argv[1], 1, 198);
	//int16 textValue = argv[0];	// looks to be unused

	_vm->_screen->setTextRect(Common::Rect(x1, y1, x2, y2));
	return 0;
}

int16 ScriptFunctions::sfSetTextXY(int16 argc, int16 *argv) {
	int16 x = CLIP<int16>(argv[1], 1, 318);
	int16 y = CLIP<int16>(argv[0], 1, 198);
	_vm->_screen->setTextXY(x, y);
	return 0;
}

int16 ScriptFunctions::sfSetFontDropShadow(int16 argc, int16 *argv) {
	// if the drop shadow color is -1, then text drop shadow is disabled
	// when font drop shadow is enabled, outline is disabled
	_vm->_screen->setDropShadowColor(argv[0]);
	return 0;
}

int16 ScriptFunctions::sfSetFontColor(int16 argc, int16 *argv) {
	_vm->_screen->setTextColor(argv[0]);
	return 0;
}

int16 ScriptFunctions::sfSetFontOutline(int16 argc, int16 *argv) {
	// if the outline color is -1, then text outline is disabled
	// when font outline is enabled, drop shadow is disabled
	_vm->_screen->setOutlineColor(argv[0]);
	return 0;
}

int16 ScriptFunctions::sfLoadMouseCursor(int16 argc, int16 *argv) {
	PictureResource *flex = _vm->_res->getPicture(argv[2]);
	if (flex) {
		Graphics::Surface *surf = flex->getPicture();
		CursorMan.replaceCursor(surf->pixels, surf->w, surf->h, argv[1], argv[0], 0);
		_vm->_res->freeResource(flex);
	}
	return 0;
}

int16 ScriptFunctions::sfSetSpriteGround(int16 argc, int16 *argv) {
	_vm->_screen->setGround(argv[0]);
	return 0;
}

int16 ScriptFunctions::sfLoadResText(int16 argc, int16 *argv) {
	// Never used in LGOP2, RTZ, Manhole:NE, Rodney
	warning("Unimplemented opcode: sfLoadResText");
	return 0;
}

int16 ScriptFunctions::sfSetClipArea(int16 argc, int16 *argv) {
	_vm->_screen->setClipArea(argv[3], argv[2], argv[1], argv[0]);
	return 0;
}

int16 ScriptFunctions::sfSetSpriteClip(int16 argc, int16 *argv) {
	_vm->_screen->setClip(argv[0]);
	return 0;
}

int16 ScriptFunctions::sfAddScreenMask(int16 argc, int16 *argv) {
	_vm->_screen->drawMask(argv[2], argv[1], argv[0]);
	return 0;
}

int16 ScriptFunctions::sfSetSpriteMask(int16 argc, int16 *argv) {
	_vm->_screen->setMask(argv[0]);
	return 0;
}

int16 ScriptFunctions::sfSoundPlaying(int16 argc, int16 *argv) {
	if (_vm->_mixer->isSoundHandleActive(_audioStreamHandle))
		return 1;
	else
		return 0;
}

void ScriptFunctions::stopSound() {
	_vm->_mixer->stopHandle(_audioStreamHandle);
	if (_soundStarted) {
		_vm->_res->freeResource(_soundResource);
		_soundStarted = false;
	}

}


int16 ScriptFunctions::sfStopSound(int16 argc, int16 *argv) {
	stopSound();
	_vm->_autoStopSound = false;
	return 0;
}

int16 ScriptFunctions::sfPlayVoice(int16 argc, int16 *argv) {
	int16 soundNum = argv[0];
	stopSound();
	if (soundNum > 0) {
		_soundResource = _vm->_res->getSound(soundNum);
		_vm->_mixer->playStream(Audio::Mixer::kPlainSoundType, &_audioStreamHandle,
			_soundResource->getAudioStream(_vm->_soundRate, false));
		_vm->_autoStopSound = true;
		_soundStarted = true;
	}
	return 0;
}

int16 ScriptFunctions::sfPlayCd(int16 argc, int16 *argv) {
	g_system->getAudioCDManager()->play(argv[0] - 1, 1, 0, 0);
	_vm->_cdTimeStart = _vm->_system->getMillis();
	if (g_system->getAudioCDManager()->isPlaying()) {
		return 1;
	} else {
		return 0;
	}
}

int16 ScriptFunctions::sfStopCd(int16 argc, int16 *argv) {
	if (g_system->getAudioCDManager()->isPlaying()) {
		g_system->getAudioCDManager()->stop();
		return 1;
	} else {
		return 0;
	}
}

int16 ScriptFunctions::sfGetCdStatus(int16 argc, int16 *argv) {
	return g_system->getAudioCDManager()->isPlaying() ? 1 : 0;
}

int16 ScriptFunctions::sfGetCdTime(int16 argc, int16 *argv) {
	if (g_system->getAudioCDManager()->isPlaying()) {
		uint32 deltaTime = _vm->_system->getMillis() - _vm->_cdTimeStart;
		// This basically converts the time from milliseconds to MSF format to MADE's format
		return (deltaTime / 1000 * 30) + (deltaTime % 1000 / 75 * 30 / 75);
	} else {
		return 32000;
	}
}

int16 ScriptFunctions::sfPlayCdSegment(int16 argc, int16 *argv) {
	// Never used in LGOP2, RTZ, Manhole:NE, Rodney
	warning("Unimplemented opcode: sfPlayCdSegment");
	return 0;
}

int16 ScriptFunctions::sfPrintf(int16 argc, int16 *argv) {
	const char *text = _vm->_dat->getObjectString(argv[argc - 1]);
	debug(4, "--> text = %s", text);
	return 0;
}

int16 ScriptFunctions::sfClearMono(int16 argc, int16 *argv) {
	// Never used in LGOP2, RTZ, Manhole:NE
	warning("Unimplemented opcode: sfClearMono");
	return 0;
}

int16 ScriptFunctions::sfGetSoundEnergy(int16 argc, int16 *argv) {
	// This is called while in-game voices are played to animate
	// mouths when NPCs are talking
	int result = 0;
	if (_vm->_mixer->isSoundHandleActive(_audioStreamHandle) && _vm->_soundEnergyArray) {
		while (_vm->_soundEnergyIndex < _vm->_soundEnergyArray->size()) {
			SoundEnergyItem *soundEnergyItem = &(*_vm->_soundEnergyArray)[_vm->_soundEnergyIndex];
			const Audio::Timestamp ts = _vm->_mixer->getElapsedTime(_audioStreamHandle);
			if (ts.convertToFramerate(_vm->_soundRate).totalNumberOfFrames() < (int)soundEnergyItem->position) {
				result = soundEnergyItem->energy;
				break;
			}
			_vm->_soundEnergyIndex++;
		}
		if (_vm->_soundEnergyIndex >= _vm->_soundEnergyArray->size())
			result = 0;
	}
	return result;
}

int16 ScriptFunctions::sfClearText(int16 argc, int16 *argv) {
	// Never used in LGOP2, RTZ, Manhole:NE
	warning("Unimplemented opcode: sfClearText");
	return 1;
}

int16 ScriptFunctions::sfAnimText(int16 argc, int16 *argv) {
	// Never used in LGOP2, RTZ, Manhole:NE
	warning("Unimplemented opcode: sfAnimText");
	return 0;
}

int16 ScriptFunctions::sfGetTextWidth(int16 argc, int16 *argv) {
	int16 width = 0;
	if (argv[1] > 0) {
		const char *text = _vm->_dat->getObjectString(argv[1]);
		width = _vm->_screen->getTextWidth(argv[0], text);
	}
	return width;
}

int16 ScriptFunctions::sfPlayMovie(int16 argc, int16 *argv) {
	const char *movieName = _vm->_dat->getObjectString(argv[1]);
	_vm->_system->showMouse(false);
	bool completed = _vm->_pmvPlayer->play(movieName);
	_vm->_system->showMouse(true);
	// Return true/false according to if the movie was canceled or not
	return completed ? -1 : 0;
}

int16 ScriptFunctions::sfLoadSound(int16 argc, int16 *argv) {
	SoundResource *sound = _vm->_res->getSound(argv[0]);
	if (sound) {
		_vm->_res->freeResource(sound);
		return 1;
	}
	return 0;
}

int16 ScriptFunctions::sfLoadMusic(int16 argc, int16 *argv) {
	GenericResource *xmidi = _vm->_res->getXmidi(argv[0]);
	if (xmidi) {
		_vm->_res->freeResource(xmidi);
		return 1;
	}
	return 0;
}

int16 ScriptFunctions::sfLoadPicture(int16 argc, int16 *argv) {
	PictureResource *flex = _vm->_res->getPicture(argv[0]);
	if (flex) {
		_vm->_res->freeResource(flex);
		return 1;
	}
	return 0;
}

int16 ScriptFunctions::sfSetMusicVolume(int16 argc, int16 *argv) {
	// Never used in LGOP2, RTZ, Manhole:NE
	warning("Unimplemented opcode: sfSetMusicVolume");
	return 0;
}

int16 ScriptFunctions::sfRestartEvents(int16 argc, int16 *argv) {
	// Used in RTZ
	//warning("Unimplemented opcode: sfRestartEvents");
	// This is used to reset the event recording/queue.
	// Since we don't use either it's left out.
	return 0;
}

int16 ScriptFunctions::sfPlaceSprite(int16 argc, int16 *argv) {
	return _vm->_screen->placeSprite(argv[3], argv[2], argv[1], argv[0]);
}

int16 ScriptFunctions::sfPlaceText(int16 argc, int16 *argv) {
	return _vm->_screen->placeText(argv[6], argv[5], argv[4], argv[3], argv[2], argv[1], argv[0]);
}

int16 ScriptFunctions::sfDeleteChannel(int16 argc, int16 *argv) {
	_vm->_screen->deleteChannel(argv[0]);
	return 0;
}

int16 ScriptFunctions::sfGetChannelType(int16 argc, int16 *argv) {
	return _vm->_screen->getChannelType(argv[0]);
}

int16 ScriptFunctions::sfSetChannelState(int16 argc, int16 *argv) {
	_vm->_screen->setChannelState(argv[1], argv[0]);
	return 0;
}

int16 ScriptFunctions::sfSetChannelLocation(int16 argc, int16 *argv) {
	// Never used in LGOP2, RTZ, Manhole:NE
	warning("Unimplemented opcode: sfSetChannelLocation");
	return 0;
}

int16 ScriptFunctions::sfSetChannelContent(int16 argc, int16 *argv) {
	// Never used in LGOP2, RTZ, Manhole:NE
	warning("Unimplemented opcode: sfSetChannelContent");
	return 0;
}

int16 ScriptFunctions::sfSetExcludeArea(int16 argc, int16 *argv) {
	_vm->_screen->setExcludeArea(argv[3], argv[2], argv[1], argv[0]);
	return 0;
}

int16 ScriptFunctions::sfSetSpriteExclude(int16 argc, int16 *argv) {
	_vm->_screen->setExclude(argv[0]);
	return 0;
}

int16 ScriptFunctions::sfGetChannelState(int16 argc, int16 *argv) {
	return _vm->_screen->getChannelState(argv[0]);
}

int16 ScriptFunctions::sfPlaceAnim(int16 argc, int16 *argv) {
	return _vm->_screen->placeAnim(argv[4], argv[3], argv[2], argv[1], argv[0]);
}

int16 ScriptFunctions::sfSetAnimFrame(int16 argc, int16 *argv) {
	_vm->_screen->setAnimFrame(argv[1], argv[0]);
	return 0;
}

int16 ScriptFunctions::sfGetAnimFrame(int16 argc, int16 *argv) {
	return _vm->_screen->getAnimFrame(argv[0]);
}

int16 ScriptFunctions::sfGetAnimFrameCount(int16 argc, int16 *argv) {
	int16 frameCount = 0;
	AnimationResource *anim = _vm->_res->getAnimation(argv[0]);
	if (anim) {
		frameCount = anim->getCount();
		_vm->_res->freeResource(anim);
	}
	return frameCount;
}

int16 ScriptFunctions::sfGetPictureWidth(int16 argc, int16 *argv) {
	int16 width = 0;
	PictureResource *flex = _vm->_res->getPicture(argv[0]);
	if (flex) {
		width = flex->getPicture()->w;
		_vm->_res->freeResource(flex);
	}
	return width;
}

int16 ScriptFunctions::sfGetPictureHeight(int16 argc, int16 *argv) {
	int16 height = 0;
	PictureResource *flex = _vm->_res->getPicture(argv[0]);
	if (flex) {
		height = flex->getPicture()->h;
		_vm->_res->freeResource(flex);
	}
	return height;
}

int16 ScriptFunctions::sfSetSoundRate(int16 argc, int16 *argv) {
	_vm->_soundRate = argv[0];
	return 1;
}

int16 ScriptFunctions::sfDrawAnimPic(int16 argc, int16 *argv) {
	return _vm->_screen->drawAnimPic(argv[5], argv[4], argv[3], argv[2], argv[1], argv[0]);
}

int16 ScriptFunctions::sfLoadAnim(int16 argc, int16 *argv) {
	AnimationResource *anim = _vm->_res->getAnimation(argv[0]);
	if (anim) {
		_vm->_res->freeResource(anim);
		return 1;
	}
	return 0;
}

int16 ScriptFunctions::sfReadText(int16 argc, int16 *argv) {
	// Never used in LGOP2, RTZ, Manhole:NE
	warning("Unimplemented opcode: sfReadText");
	return 0;
}

int16 ScriptFunctions::sfReadMenu(int16 argc, int16 *argv) {
	int16 objectIndex = argv[2];
	int16 menuIndex = argv[1];
	int16 textIndex = argv[0];
	int16 length = 0;
	MenuResource *menu = _vm->_res->getMenu(menuIndex);
	if (menu) {
		const char *text = menu->getString(textIndex);
		debug(4, "objectIndex = %04X; text = %s\n", objectIndex, text);
		_vm->_dat->setObjectString(objectIndex, text);
		_vm->_res->freeResource(menu);
		if (text)
			length = strlen(text);
	} else {
		_vm->_dat->setObjectString(objectIndex, "");
	}
	return length;
}

int16 ScriptFunctions::sfDrawMenu(int16 argc, int16 *argv) {
	int16 menuIndex = argv[1];
	int16 textIndex = argv[0];
	MenuResource *menu = _vm->_res->getMenu(menuIndex);
	if (menu) {
		const char *text = menu->getString(textIndex);
		if (text)
			_vm->_screen->printText(text);

		_vm->_res->freeResource(menu);
	}
	return 0;
}

int16 ScriptFunctions::sfGetMenuCount(int16 argc, int16 *argv) {
	int16 menuIndex = argv[0];
	int16 count = 0;
	MenuResource *menu = _vm->_res->getMenu(menuIndex);
	if (menu) {
		count = menu->getCount();
		_vm->_res->freeResource(menu);
	}
	return count;
}

int16 ScriptFunctions::sfSaveGame(int16 argc, int16 *argv) {

	int16 saveNum = argv[2];
	int16 descObjectIndex = argv[1];
	int16 version = argv[0];

	if (saveNum > 999)
		return 6;

	const char *description = _vm->_dat->getObjectString(descObjectIndex);
	Common::String filename = _vm->getSavegameFilename(saveNum);
	return _vm->_dat->savegame(filename.c_str(), description, version);

}

int16 ScriptFunctions::sfLoadGame(int16 argc, int16 *argv) {

	int16 saveNum = argv[1];
	int16 version = argv[0];

	if (saveNum > 999)
		return 1;

	Common::String filename = _vm->getSavegameFilename(saveNum);
	return _vm->_dat->loadgame(filename.c_str(), version);

}

int16 ScriptFunctions::sfGetGameDescription(int16 argc, int16 *argv) {

	int16 descObjectIndex = argv[2];
	int16 saveNum = argv[1];
	int16 version = argv[0];
	Common::String description;

	if (saveNum > 999)
		return 1;

	Common::String filename = _vm->getSavegameFilename(saveNum);

	if (_vm->_dat->getSavegameDescription(filename.c_str(), description, version)) {
		_vm->_dat->setObjectString(descObjectIndex, description.c_str());
		return 0;
	} else {
		_vm->_dat->setObjectString(descObjectIndex, "");
		return 1;
	}

}

int16 ScriptFunctions::sfShakeScreen(int16 argc, int16 *argv) {
	// TODO: Used in RTZ
	warning("Unimplemented opcode: sfShakeScreen");
	return 0;
}

int16 ScriptFunctions::sfPlaceMenu(int16 argc, int16 *argv) {
	// Never used in LGOP2, RTZ, Manhole:NE
	warning("Unimplemented opcode: sfPlaceMenu");
	return 0;
}

int16 ScriptFunctions::sfSetSoundVolume(int16 argc, int16 *argv) {
	_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, argv[0] * 25);
	_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, argv[0] * 25);
	return 0;
}

int16 ScriptFunctions::sfGetSynthType(int16 argc, int16 *argv) {
	// 0 = Default
	// 1 = PCSPKR
	// 2 = SBFM/ADLIB
	// 3 = ADLIBG
	// 4 = MT32MPU

	// There doesn't seem to be any difference in the music no matter what this returns

	//warning("Unimplemented opcode: sfGetSynthType");
	return 0;
}

int16 ScriptFunctions::sfIsSlowSystem(int16 argc, int16 *argv) {
	//warning("Unimplemented opcode: sfIsSlowSystem");
	// NOTE: In the original engine this value is set via a command-line parameter
	// I don't think it's needed here
	// Update: I believe this is used to determine which version of the intro/ending to show
	// There are 2 versions of each video: one with sound, and one without
	// An example is FINTRO00.PMV (with sound) and FINTRO01.PMV (without sound)
	// One could maybe think about returning 1 here on actually slower systems.
	return 0;
}

} // End of namespace Made

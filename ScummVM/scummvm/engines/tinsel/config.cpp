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
 * This file contains configuration functionality
 */

//#define USE_3FLAGS 1

#include "tinsel/config.h"
#include "tinsel/dw.h"
#include "tinsel/sound.h"
#include "tinsel/music.h"

#include "common/file.h"
#include "common/config-manager.h"

#include "audio/mixer.h"

namespace Tinsel {

Config::Config(TinselEngine *vm) : _vm(vm) {
	_dclickSpeed = DOUBLE_CLICK_TIME;
	_musicVolume = Audio::Mixer::kMaxChannelVolume;
	_soundVolume = Audio::Mixer::kMaxChannelVolume;
	_voiceVolume = Audio::Mixer::kMaxChannelVolume;
	_textSpeed = DEFTEXTSPEED;
	_useSubtitles = false;
	_swapButtons = false;
	_language = TXT_ENGLISH;
	_isAmericanEnglishVersion = false;
}


/**
 * Write settings to config manager and flush the config file to disk.
 */
void Config::writeToDisk() {
	ConfMan.setInt("dclick_speed", _dclickSpeed);
	ConfMan.setInt("music_volume", _musicVolume);
	ConfMan.setInt("sfx_volume", _soundVolume);
	ConfMan.setInt("speech_volume", _voiceVolume);
	ConfMan.setInt("talkspeed", (_textSpeed * 255) / 100);
	ConfMan.setBool("subtitles", _useSubtitles);
	//ConfMan.setBool("swap_buttons", _swapButtons ? 1 : 0);

	// Store language for multilingual versions
	if ((_vm->getFeatures() & GF_USE_3FLAGS) || (_vm->getFeatures() & GF_USE_4FLAGS) || (_vm->getFeatures() & GF_USE_5FLAGS)) {
		Common::Language lang;
		switch (_language) {
		case TXT_FRENCH:
			lang = Common::FR_FRA;
			break;
		case TXT_GERMAN:
			lang = Common::DE_DEU;
			break;
		case TXT_SPANISH:
			lang = Common::ES_ESP;
			break;
		case TXT_ITALIAN:
			lang = Common::IT_ITA;
			break;
		case TXT_US:
			lang = Common::EN_USA;
			break;
		default:
			lang = Common::EN_ANY;
		}

		ConfMan.set("language", Common::getLanguageCode(lang));
	}

	// Write to disk
	ConfMan.flushToDisk();
}

/**
 * Read configuration settings from the config file into memory
 */
void Config::readFromDisk() {
	if (ConfMan.hasKey("dclick_speed"))
		_dclickSpeed = ConfMan.getInt("dclick_speed");

	// HACK/FIXME:
	// We need to clip the volumes from [0, 256] to [0, 255]
	// here, since for example Tinsel's internal options dialog
	// and also the midi playback code rely on the volumes to be
	// in [0, 255]
	_musicVolume = CLIP(ConfMan.getInt("music_volume"), 0, 255);
	_soundVolume = CLIP(ConfMan.getInt("sfx_volume"), 0, 255);
	_voiceVolume = CLIP(ConfMan.getInt("speech_volume"), 0, 255);

	if (ConfMan.hasKey("talkspeed"))
		_textSpeed = (ConfMan.getInt("talkspeed") * 100) / 255;
	if (ConfMan.hasKey("subtitles"))
		_useSubtitles = ConfMan.getBool("subtitles");

	// FIXME: If JAPAN is set, subtitles are forced OFF in the original engine

	//_swapButtons = ConfMan.getBool("swap_buttons") == 1 ? true : false;
	//ConfigData.language = language;	// not necessary, as language has been set in the launcher
	//ConfigData._isAmericanEnglishVersion = _isAmericanEnglishVersion;		// EN_USA / EN_GRB

	// Set language - we'll be clever here and use the ScummVM language setting
	_language = TXT_ENGLISH;
	Common::Language lang = _vm->getLanguage();
	if (lang == Common::UNK_LANG && ConfMan.hasKey("language"))
		lang = Common::parseLanguage(ConfMan.get("language"));	// For multi-lingual versions, fall back to user settings
	switch (lang) {
	case Common::FR_FRA:
		_language = TXT_FRENCH;
		break;
	case Common::DE_DEU:
		_language = TXT_GERMAN;
		break;
	case Common::ES_ESP:
		_language = TXT_SPANISH;
		break;
	case Common::IT_ITA:
		_language = TXT_ITALIAN;
		break;
	case Common::EN_USA:
		_language = TXT_US;
		break;
	default:
		_language = TXT_ENGLISH;
	}

	if (lang == Common::JA_JPN) {
		// TODO: Add support for JAPAN version
	} else if (lang == Common::HE_ISR) {
		// TODO: Add support for HEBREW version

		// The Hebrew version appears to the software as being English
		// but it needs to have subtitles on...
		_language = TXT_ENGLISH;
		_useSubtitles = true;
	} else if (_vm->getFeatures() & GF_USE_3FLAGS) {
		// 3 FLAGS version supports French, German, Spanish
		// Fall back to German if necessary
		if (_language != TXT_FRENCH && _language != TXT_GERMAN && _language != TXT_SPANISH) {
			_language = TXT_GERMAN;
			_useSubtitles = true;
		}
	} else if (_vm->getFeatures() & GF_USE_4FLAGS) {
		// 4 FLAGS version supports French, German, Spanish, Italian
		// Fall back to German if necessary
		if (_language != TXT_FRENCH && _language != TXT_GERMAN &&
				_language != TXT_SPANISH && _language != TXT_ITALIAN) {
			_language = TXT_GERMAN;
			_useSubtitles = true;
		}
	}
}

bool isJapanMode() {
#ifdef JAPAN
	return true;
#else
	return false;
#endif
}

} // End of namespace Tinsel

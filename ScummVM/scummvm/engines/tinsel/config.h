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

#ifndef TINSEL_CONFIG_H
#define TINSEL_CONFIG_H

#include "tinsel/dw.h"

namespace Tinsel {

// double click timer initial value
enum {
	DOUBLE_CLICK_TIME	= 10,	// 10 @ 18Hz = .55 sec
	DEFTEXTSPEED		= 0
};

class TinselEngine;

class Config {
private:
	TinselEngine *_vm;

public:
	int _dclickSpeed;
	int _musicVolume;
	int _soundVolume;
	int _voiceVolume;
	int _textSpeed;
	int _useSubtitles;
	int _swapButtons;
	LANGUAGE _language;
	int _isAmericanEnglishVersion;

public:
	Config(TinselEngine *vm);

	void writeToDisk();
	void readFromDisk();
};


extern bool isJapanMode();

} // End of namespace Tinsel

#endif

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

#ifndef GOB_GLOBAL_H
#define GOB_GLOBAL_H

#include "common/file.h"
#include "common/endian.h"

#include "gob/video.h"

namespace Gob {

#define VIDMODE_CGA 0x05
#define VIDMODE_EGA 0x0D
#define VIDMODE_VGA 0x13
#define VIDMODE_HER 0x07

#define MIDI_FLAG       0x4000
#define PROAUDIO_FLAG   0x0010
#define ADLIB_FLAG      0x0008
#define BLASTER_FLAG    0x0004
#define INTERSOUND_FLAG 0x0002
#define SPEAKER_FLAG    0x0001

//#define NO    0
//#define YES   1
#define UNDEF 2

#define F1_KEY 0x3B00
#define F2_KEY 0x3C00
#define F3_KEY 0x3D00
#define F4_KEY 0x3E00
#define F5_KEY 0x3F00
#define F6_KEY 0x4000
#define ESCAPE 0x001B
#define ENTER  0x000D

/* Video drivers */
#define UNK_DRIVER 0
#define VGA_DRIVER 1
#define EGA_DRIVER 2
#define CGA_DRIVER 3
#define HER_DRIVER 4

enum Language {
	kLanguageFrench     =  0,
	kLanguageGerman     =  1,
	kLanguageBritish    =  2,
	kLanguageSpanish    =  3,
	kLanguageItalian    =  4,
	kLanguageAmerican   =  5,
	kLanguageDutch      =  6,
	kLanguageKorean     =  7,
	kLanguageHebrew     =  8,
	kLanguagePortuguese =  9,
	kLanguageJapanese   = 10
};

class Global {
public:
	char _pressedKeys[128];

	int16 _presentCGA;
	int16 _presentEGA;
	int16 _presentVGA;
	int16 _presentHER;

	int16 _videoMode;
	int16 _fakeVideoMode;
	int16 _oldMode;

	uint16 _soundFlags;

	uint16 _language;
	uint16 _languageWanted;
	bool   _foundLanguage;

	char _useMouse;
	int16 _mousePresent;
	int16 _mouseXShift;
	int16 _mouseYShift;

	int16 _mouseMinX;
	int16 _mouseMinY;
	int16 _mouseMaxX;
	int16 _mouseMaxY;

	char _useJoystick;

	int16 _primaryWidth;
	int16 _primaryHeight;

	int16 _colorCount;
	char _redPalette[256];
	char _greenPalette[256];
	char _bluePalette[256];

	int16 _unusedPalette1[18];
	int16 _unusedPalette2[16];
	Video::Color _vgaPalette[16];
	Video::PalDesc _paletteStruct;
	Video::PalDesc *_pPaletteDesc;

	bool _setAllPalette;
	bool _dontSetPalette;

	SurfacePtr _primarySurfDesc;

	int16 _debugFlag;

	int16 _inter_animDataSize;

	int16 _inter_mouseX;
	int16 _inter_mouseY;

	// Can be 1, 2 or 3 for normal, double and triple speed, respectively
	uint8 _speedFactor;

	bool _doSubtitles;

	bool _noCd;

	int16 _curWinId;

	Global(GobEngine *vm);
	~Global();

protected:
	GobEngine *_vm;
};

} // End of namespace Gob

#endif // GOB_GLOBAL_H

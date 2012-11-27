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

#ifndef SKY_INTRO_H
#define SKY_INTRO_H


#include "common/scummsys.h"
#include "audio/mixer.h"

namespace Sky {

class Disk;
class Screen;
class MusicBase;
class Sound;
class Text;

class Intro {
public:
	Intro(Disk *disk, Screen *screen, MusicBase *music, Sound *sound, Text *text, Audio::Mixer *mixer, OSystem *system);
	~Intro();
	bool doIntro(bool floppyIntro);
private:
	static uint16 _mainIntroSeq[];
	static uint16 _floppyIntroSeq[];
	static uint16 _cdIntroSeq[];

	Disk *_skyDisk;
	Screen *_skyScreen;
	MusicBase *_skyMusic;
	Sound *_skySound;
	Text *_skyText;
	OSystem *_system;
	Audio::Mixer *_mixer;

	uint8 *_textBuf, *_saveBuf;
	uint8 *_bgBuf;
	uint32 _bgSize;
	Audio::SoundHandle _voice, _bgSfx;

	int32 _relDelay;

	bool escDelay(uint32 msecs);
	bool nextPart(uint16 *&data);
	bool floppyScrollFlirt();
	bool commandFlirt(uint16 *&data);
	void showTextBuf();
	void restoreScreen();
};

} // End of namespace Sky

#endif // INTRO_H

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

#include "gob/gob.h"
#include "gob/global.h"
#include "gob/palanim.h"
#include "gob/draw.h"

#include "gob/sound/sound.h"

#include "gob/pregob/onceupon/title.h"

namespace Gob {

namespace OnceUpon {

Title::Title(GobEngine *vm) : SEQFile(vm, "ville.seq") {
}

Title::~Title() {
}

void Title::play() {
	SEQFile::play(true, 0xFFFF, 15);

	// After playback, fade out and stop the music
	if (!_vm->shouldQuit())
		_vm->_palAnim->fade(0, 0, 0);

	stopMusic();
}

void Title::handleFrameEvent() {
	// On fame 0, start the music and fade in
	if (getFrame() == 0) {
		playMusic();

		_vm->_draw->forceBlit();
		_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, 0, 0);
	}
}

void Title::playMusic() {
	// Look at what platform this is and play the appropriate music type

	if      (_vm->getPlatform() == Common::kPlatformPC)
		playMusicDOS();
	else if (_vm->getPlatform() == Common::kPlatformAmiga)
		playMusicAmiga();
	else if (_vm->getPlatform() == Common::kPlatformAtariST)
		playMusicAtariST();
}

void Title::playMusicDOS() {
	// Play an AdLib track

	_vm->_sound->adlibLoadTBR("babayaga.tbr");
	_vm->_sound->adlibLoadMDY("babayaga.mdy");
	_vm->_sound->adlibSetRepeating(-1);
	_vm->_sound->adlibPlay();
}

void Title::playMusicAmiga() {
	// Play a Protracker track

	_vm->_sound->protrackerPlay("mod.babayaga");
}

void Title::playMusicAtariST() {
	// Play a Soundblaster composition

	static const int16        titleMusic[21] = { 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 2, 0, 1, 0, 0, 0, 0, 0, -1};
	static const char * const titleFiles[ 3] = {"baba1.snd", "baba2.snd", "baba3.snd"};

	for (uint i = 0; i < ARRAYSIZE(titleFiles); i++)
		_vm->_sound->sampleLoad(_vm->_sound->sampleGetBySlot(i), SOUND_SND, titleFiles[i]);

	_vm->_sound->blasterPlayComposition(titleMusic, 0);
	_vm->_sound->blasterRepeatComposition(-1);
}

void Title::stopMusic() {
	// Just stop everything

	_vm->_sound->adlibSetRepeating(0);
	_vm->_sound->blasterRepeatComposition(0);

	_vm->_sound->adlibStop();
	_vm->_sound->blasterStopComposition();
	_vm->_sound->protrackerStop();

	for (int i = 0; i < ::Gob::Sound::kSoundsCount; i++)
		_vm->_sound->sampleFree(_vm->_sound->sampleGetBySlot(i));
}

} // End of namespace OnceUpon

} // End of namespace Gob

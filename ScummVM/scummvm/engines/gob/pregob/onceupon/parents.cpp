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
#include "gob/dataio.h"
#include "gob/palanim.h"
#include "gob/draw.h"
#include "gob/video.h"

#include "gob/sound/sound.h"

#include "gob/pregob/gctfile.h"

#include "gob/pregob/onceupon/palettes.h"
#include "gob/pregob/onceupon/parents.h"

namespace Gob {

namespace OnceUpon {

const char *Parents::kSound[kSoundCount] = {
	"rire.snd", // kSoundCackle
	"tonn.snd"  // kSoundThunder
};

// So that every GCT line is displayed for 12 seconds
const uint16 Parents::kLoop[kLoopCount][3] = {
	{ 72,  77, 33},
	{105, 109, 38},
	{141, 145, 38},
	{446, 454, 23},
	{456, 464, 23},
	{466, 474, 23},
	{476, 484, 23}
};


Parents::Parents(GobEngine *vm, const Common::String &seq, const Common::String &gct,
                 const Common::String &childName, uint8 house, const Font &font,
                 const byte *normalPalette, const byte *brightPalette, uint paletteSize) :
	SEQFile(vm, seq),
	_gct(0), _house(house), _font(&font),
	_paletteSize(paletteSize), _normalPalette(normalPalette), _brightPalette(brightPalette) {

	// Load sounds
	for (int i = 0; i < kSoundCount; i++)
		_vm->_sound->sampleLoad(&_sounds[i], SOUND_SND, kSound[i]);

	// Load GCT
	Common::SeekableReadStream *gctStream = _vm->_dataIO->getFile(gct);
	if (gctStream) {
		_gct = new GCTFile(*gctStream, _vm->_rnd);

		delete gctStream;
	} else
		error("Parents::Parents(): Failed to open \"%s\"", gct.c_str());

	_gct->setArea(17, 18, 303, 41);
	_gct->setText(1, childName);

	_gct->selectLine(2, _house);
	_gct->selectLine(4, _house);

	for (uint i = 0; i < kLoopCount; i++)
		_loopID[i] = addLoop(kLoop[i][0], kLoop[i][1], kLoop[i][2]);
}

Parents::~Parents() {
	delete _gct;
}

void Parents::play() {
	_currentLoop = 0;

	SEQFile::play(true, 496, 15);

	// After playback, fade out
	if (!_vm->shouldQuit())
		_vm->_palAnim->fade(0, 0, 0);
}

void Parents::handleFrameEvent() {
	switch (getFrame()) {
	case 0:
		// On fame 0, fade in
		_vm->_draw->forceBlit();
		_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, 0, 0);
		break;

	case 4:
		drawGCT(0);
		break;

	case 55:
		drawGCT(3, 0);
		break;

	case 79:
		drawGCT(_house + 5, 1);
		break;

	case 110:
		drawGCT(_house + 9, 2);
		break;

	case 146:
		drawGCT(17);
		break;

	case 198:
		drawGCT(13);
		break;

	case 445:
		drawGCT(14, 3);
		break;

	case 455:
		drawGCT(18, 4);
		break;

	case 465:
		drawGCT(19, 5);
		break;

	case 475:
		drawGCT(20, 6);
		break;

	case 188:
	case 228:
	case 237:
	case 257:
	case 275:
	case 426:
		lightningEffect();
		break;

	case 203:
	case 243:
	case 252:
	case 272:
	case 290:
	case 441:
		playSound(kSoundThunder);
		break;

	case 340:
		playSound(kSoundCackle);
		break;
	}
}

void Parents::handleInput(int16 key, int16 mouseX, int16 mouseY, MouseButtons mouseButtons) {
	if ((key == kKeyEscape) || (mouseButtons == kMouseButtonsRight))
		abortPlay();

	if (((key == kKeySpace) || (mouseButtons == kMouseButtonsLeft)) && (_currentLoop < kLoopCount))
		skipLoop(_loopID[_currentLoop]);
}

void Parents::playSound(Sound sound) {
	_vm->_sound->blasterStop(0);
	_vm->_sound->blasterPlay(&_sounds[sound], 0, 0);
}

void Parents::lightningEffect() {
	for (int i = 0; (i < 5) && !_vm->shouldQuit(); i++) {

		setPalette(_brightPalette, _paletteSize);
		_vm->_util->delay(5);

		setPalette(_normalPalette, _paletteSize);
		_vm->_util->delay(5);
	}
}

void Parents::setPalette(const byte *palette, uint size) {
	memcpy(_vm->_draw->_vgaPalette, palette, 3 * size);

	_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
	_vm->_video->retrace();
}

void Parents::drawGCT(uint item, uint loop) {
	int16 left, top, right, bottom;
	if (_gct->clear(*_vm->_draw->_backSurface, left, top, right, bottom))
		_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, left, top, right, bottom);
	if (_gct->draw(*_vm->_draw->_backSurface, item, *_font, 10, left, top, right, bottom))
		_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, left, top, right, bottom);

	_currentLoop = loop;
}

} // End of namespace OnceUpon

} // End of namespace Gob

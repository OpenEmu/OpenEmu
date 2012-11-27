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

#include "graphics/cursorman.h"

#include "gob/gob.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/surface.h"
#include "gob/dataio.h"
#include "gob/palanim.h"
#include "gob/draw.h"
#include "gob/video.h"
#include "gob/aniobject.h"

#include "gob/sound/sound.h"

#include "gob/pregob/pregob.h"
#include "gob/pregob/gctfile.h"


namespace Gob {

const char  PreGob::kLanguageSuffixShort[5] = { 't',  'g',  'a',  'e',  'i'};
const char *PreGob::kLanguageSuffixLong [5] = {"fr", "al", "an", "it", "es"};


PreGob::PreGob(GobEngine *vm) : _vm(vm), _fadedOut(false) {
}

PreGob::~PreGob() {
}

void PreGob::fadeOut() {
	if (_fadedOut || _vm->shouldQuit())
		return;

	// Fade to black
	_vm->_palAnim->fade(0, 0, 0);

	_fadedOut = true;
}

void PreGob::fadeIn() {
	if (!_fadedOut || _vm->shouldQuit())
		return;

	// Fade to palette
	_vm->_draw->blitInvalidated();
	_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, 0, 0);
	_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, 0, 0, 319, 199);

	_fadedOut = false;
}

void PreGob::clearScreen() {
	_vm->_draw->_backSurface->clear();
	_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, 0, 0, 319, 199);
	_vm->_draw->blitInvalidated();
	_vm->_video->retrace();
}

void PreGob::initScreen() {
	_vm->_util->setFrameRate(15);

	_fadedOut = true;

	_vm->_draw->initScreen();

	_vm->_draw->_backSurface->clear();
	_vm->_util->clearPalette();

	_vm->_draw->forceBlit();
	_vm->_video->retrace();

	_vm->_util->processInput();
}

void PreGob::setPalette(const byte *palette, uint16 size) {
	memcpy(_vm->_draw->_vgaPalette, palette, 3 * size);

	// If we didn't fade out prior, immediately set the palette
	if (!_fadedOut)
		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
}

void PreGob::addCursor() {
	CursorMan.pushCursor(0, 0, 0, 0, 0, 0);
}

void PreGob::removeCursor() {
	CursorMan.popCursor();
}

void PreGob::setCursor(Surface &sprite, int16 hotspotX, int16 hotspotY) {
	CursorMan.replaceCursor(sprite.getData(), sprite.getWidth(), sprite.getHeight(), hotspotX, hotspotY, 0);
}

void PreGob::setCursor(Surface &sprite, int16 left, int16 top, int16 right, int16 bottom,
                       int16 hotspotX, int16 hotspotY) {

	const int width  = right  - left + 1;
	const int height = bottom - top  + 1;

	if ((width <= 0) || (height <= 0))
		return;

	Surface cursor(width, height, 1);

	cursor.blit(sprite, left, top, right, bottom, 0, 0);

	setCursor(cursor, hotspotX, hotspotX);
}

void PreGob::showCursor() {
	CursorMan.showMouse(true);

	_vm->_draw->_showCursor = 4;
}

void PreGob::hideCursor() {
	CursorMan.showMouse(false);

	_vm->_draw->_showCursor = 0;
}

bool PreGob::isCursorVisible() const {
	return CursorMan.isVisible();
}

void PreGob::loadSounds(const char * const *sounds, uint soundCount) {
	freeSounds();

	_sounds.resize(soundCount);

	for (uint i = 0; i < soundCount; i++)
		loadSound(_sounds[i], sounds[i]);
}

void PreGob::freeSounds() {
	_sounds.clear();
}

bool PreGob::loadSound(SoundDesc &sound, const Common::String &file) const {
	return _vm->_sound->sampleLoad(&sound, SOUND_SND, file.c_str());
}

void PreGob::playSound(uint sound, int16 frequency, int16 repCount) {
	if (sound >= _sounds.size())
		return;

	_vm->_sound->blasterPlay(&_sounds[sound], repCount, frequency);
}

void PreGob::stopSound() {
	_vm->_sound->blasterStop(0);
}

void PreGob::playSoundFile(const Common::String &file, int16 frequency, int16 repCount, bool interruptible) {
	stopSound();

	SoundDesc sound;
	if (!loadSound(sound, file))
		return;

	_vm->_sound->blasterPlay(&sound, repCount, frequency);

	_vm->_util->forceMouseUp();

	bool finished = false;
	while (!_vm->shouldQuit() && !finished && _vm->_sound->blasterPlayingSound()) {
		endFrame(true);

		finished = hasInput();
	}

	_vm->_util->forceMouseUp();

	stopSound();
}

void PreGob::beep(int16 frequency, int32 length) {
	_vm->_sound->speakerOn(frequency, length);
}

void PreGob::endFrame(bool doInput) {
	_vm->_draw->blitInvalidated();
	_vm->_util->waitEndFrame();

	if (doInput)
		_vm->_util->processInput();
}

int16 PreGob::checkInput(int16 &mouseX, int16 &mouseY, MouseButtons &mouseButtons) {
	_vm->_util->getMouseState(&mouseX, &mouseY, &mouseButtons);
	_vm->_util->forceMouseUp();

	return _vm->_util->checkKey();
}

int16 PreGob::waitInput(int16 &mouseX, int16 &mouseY, MouseButtons &mouseButtons) {
	bool finished = false;

	int16 key = 0;
	while (!_vm->shouldQuit() && !finished) {
		endFrame(true);

		key = checkInput(mouseX, mouseY, mouseButtons);

		finished = (mouseButtons != kMouseButtonsNone) || (key != 0);
	}

	return key;
}

int16 PreGob::waitInput() {
	int16 mouseX, mouseY;
	MouseButtons mouseButtons;

	return waitInput(mouseX, mouseY, mouseButtons);
}

bool PreGob::hasInput() {
	int16 mouseX, mouseY;
	MouseButtons mouseButtons;

	return checkInput(mouseX, mouseY, mouseButtons) || (mouseButtons != kMouseButtonsNone);
}

void PreGob::clearAnim(ANIObject &anim) {
	int16 left, top, right, bottom;

	if (anim.clear(*_vm->_draw->_backSurface, left, top, right, bottom))
		_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, left, top, right, bottom);
}

void PreGob::drawAnim(ANIObject &anim) {
	int16 left, top, right, bottom;

	if (anim.draw(*_vm->_draw->_backSurface, left, top, right, bottom))
		_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, left, top, right, bottom);
	anim.advance();
}

void PreGob::redrawAnim(ANIObject &anim) {
	clearAnim(anim);
	drawAnim(anim);
}

void PreGob::clearAnim(const ANIList &anims) {
	for (int i = (anims.size() - 1); i >= 0; i--)
		clearAnim(*anims[i]);
}

void PreGob::drawAnim(const ANIList &anims) {
	for (ANIList::const_iterator a = anims.begin(); a != anims.end(); ++a)
		drawAnim(**a);
}

void PreGob::redrawAnim(const ANIList &anims) {
	clearAnim(anims);
	drawAnim(anims);
}

void PreGob::loadAnims(ANIList &anims, ANIFile &ani, uint count, const AnimProperties *props) const {
	freeAnims(anims);

	anims.resize(count);
	for (uint i = 0; i < count; i++) {
		anims[i] = new ANIObject(ani);

		setAnim(*anims[i], props[i]);
	}
}

void PreGob::freeAnims(ANIList &anims) const {
	for (ANIList::iterator a = anims.begin(); a != anims.end(); ++a)
		delete *a;

	anims.clear();
}

void PreGob::setAnim(ANIObject &anim, const AnimProperties &props) const {
	anim.setAnimation(props.animation);
	anim.setFrame(props.frame);
	anim.setMode(props.mode);
	anim.setPause(props.paused);
	anim.setVisible(props.visible);

	if (props.hasPosition)
		anim.setPosition(props.x, props.y);
	else
		anim.setPosition();
}

Common::String PreGob::getLocFile(const Common::String &file) const {
	if (_vm->_global->_language >= ARRAYSIZE(kLanguageSuffixShort))
		return file;

	return file + kLanguageSuffixShort[_vm->_global->_language];
}

TXTFile *PreGob::loadTXT(const Common::String &txtFile, TXTFile::Format format) const {
	Common::SeekableReadStream *txtStream = _vm->_dataIO->getFile(txtFile);
	if (!txtStream)
		error("PreGob::loadTXT(): Failed to open \"%s\"", txtFile.c_str());

	TXTFile *txt = new TXTFile(*txtStream, format);

	delete txtStream;

	fixTXTStrings(*txt);

	return txt;
}

void PreGob::fixTXTStrings(TXTFile &txt) const {
}

GCTFile *PreGob::loadGCT(const Common::String &gctFile) const {
	Common::SeekableReadStream *gctStream = _vm->_dataIO->getFile(gctFile);
	if (!gctStream)
		error("PreGob::loadGCT(): Failed to open \"%s\"", gctFile.c_str());

	GCTFile *gct = new GCTFile(*gctStream, _vm->_rnd);

	delete gctStream;

	return gct;
}

} // End of namespace Gob

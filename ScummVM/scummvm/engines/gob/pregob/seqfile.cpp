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

#include "common/str.h"
#include "common/stream.h"

#include "gob/gob.h"
#include "gob/dataio.h"
#include "gob/draw.h"
#include "gob/decfile.h"
#include "gob/anifile.h"
#include "gob/aniobject.h"

#include "gob/pregob/seqfile.h"

namespace Gob {

SEQFile::SEQFile(GobEngine *vm, const Common::String &fileName) : _vm(vm) {
	for (uint i = 0; i < kObjectCount; i++)
		_objects[i].object = 0;

	Common::SeekableReadStream *seq = _vm->_dataIO->getFile(Util::setExtension(fileName, ".SEQ"));
	if (!seq) {
		warning("SEQFile::SEQFile(): No such file \"%s\"", fileName.c_str());
		return;
	}

	load(*seq);

	delete seq;
}

SEQFile::~SEQFile() {
	for (uint i = 0; i < kObjectCount; i++)
		delete _objects[i].object;

	for (Backgrounds::iterator b = _backgrounds.begin(); b != _backgrounds.end(); ++b)
		delete *b;

	for (Animations::iterator a = _animations.begin(); a != _animations.end(); ++a)
		delete *a;
}

void SEQFile::load(Common::SeekableReadStream &seq) {
	const uint16 decCount = (uint16)seq.readByte() + 1;
	const uint16 aniCount = (uint16)seq.readByte() + 1;

	// Load backgrounds
	_backgrounds.reserve(decCount);
	for (uint i = 0; i < decCount; i++) {
		const Common::String dec = Util::readString(seq, 13);

		if (!_vm->_dataIO->hasFile(dec)) {
			warning("SEQFile::load(): No such background \"%s\"", dec.c_str());
			return;
		}

		_backgrounds.push_back(new DECFile(_vm, dec, 320, 200));
	}

	// Load animations
	_animations.reserve(aniCount);
	for (uint i = 0; i < aniCount; i++) {
		const Common::String ani = Util::readString(seq, 13);

		if (!_vm->_dataIO->hasFile(ani)) {
			warning("SEQFile::load(): No such animation \"%s\"", ani.c_str());
			return;
		}

		_animations.push_back(new ANIFile(_vm, ani));
	}

	_frameRate = seq.readUint16LE();

	// Load background change keys

	const uint16 bgKeyCount = seq.readUint16LE();
	_bgKeys.resize(bgKeyCount);

	for (uint16 i = 0; i < bgKeyCount; i++) {
		const uint16 frame = seq.readUint16LE();
		const uint16 index = seq.readUint16LE();

		_bgKeys[i].frame      = frame;
		_bgKeys[i].background = index < _backgrounds.size() ? _backgrounds[index] : 0;
	}

	// Load animation keys for all 4 objects

	for (uint i = 0; i < kObjectCount; i++) {
		const uint16 animKeyCount = seq.readUint16LE();
		_animKeys.reserve(_animKeys.size() + animKeyCount);

		for (uint16 j = 0; j < animKeyCount; j++) {
			_animKeys.push_back(AnimationKey());

			const uint16 frame = seq.readUint16LE();
			const uint16 index = seq.readUint16LE();

			uint16 animation;
			const ANIFile *ani = findANI(index, animation);

			_animKeys.back().object    = i;
			_animKeys.back().frame     = frame;
			_animKeys.back().ani       = ani;
			_animKeys.back().animation = animation;
			_animKeys.back().x         = seq.readSint16LE();
			_animKeys.back().y         = seq.readSint16LE();
			_animKeys.back().order     = seq.readSint16LE();
		}
	}

}

const ANIFile *SEQFile::findANI(uint16 index, uint16 &animation) {
	animation = 0xFFFF;

	// 0xFFFF = remove animation
	if (index == 0xFFFF)
		return 0;

	for (Animations::const_iterator a = _animations.begin(); a != _animations.end(); ++a) {
		if (index < (*a)->getAnimationCount()) {
			animation = index;
			return *a;
		}

		index -= (*a)->getAnimationCount();
	}

	return 0;
}

void SEQFile::play(bool abortable, uint16 endFrame, uint16 frameRate) {
	if (_bgKeys.empty() && _animKeys.empty())
		// Nothing to do
		return;

	// Init

	_frame = 0;
	_abortPlay = false;

	for (uint i = 0; i < kObjectCount; i++) {
		delete _objects[i].object;

		_objects[i].object = 0;
		_objects[i].order  = 0;
	}

	for (Loops::iterator l = _loops.begin(); l != _loops.end(); ++l)
		l->currentLoop = 0;

	// Set the frame rate

	int16 frameRateBack = _vm->_util->getFrameRate();

	if (frameRate == 0)
		frameRate = _frameRate;

	_vm->_util->setFrameRate(frameRate);

	_abortable = abortable;

	while (!_vm->shouldQuit() && !_abortPlay) {
		// Handle the frame contents
		playFrame();

		// Handle extra frame events
		handleFrameEvent();

		// Wait for the frame to end
		_vm->_draw->blitInvalidated();
		_vm->_util->waitEndFrame();

		// Handle input

		_vm->_util->processInput();

		int16 key = _vm->_util->checkKey();

		int16 mouseX, mouseY;
		MouseButtons mouseButtons;
		_vm->_util->getMouseState(&mouseX, &mouseY, &mouseButtons);
		_vm->_util->forceMouseUp();

		handleInput(key, mouseX, mouseY, mouseButtons);

		// Loop

		bool looped = false;
		for (Loops::iterator l = _loops.begin(); l != _loops.end(); ++l) {
			if ((l->endFrame == _frame) && (l->currentLoop < l->loopCount)) {
				_frame = l->startFrame;

				l->currentLoop++;
				looped = true;
			}
		}

		// If we didn't loop, advance the frame and look if we should end here

		if (!looped) {
			_frame++;
			if (_frame >= endFrame)
				break;
		}
	}

	// Restore the frame rate
	_vm->_util->setFrameRate(frameRateBack);
}

void SEQFile::playFrame() {
	// Remove the current animation frames
	clearAnims();

	// Handle background keys, directly updating the background
	for (BackgroundKeys::const_iterator b = _bgKeys.begin(); b != _bgKeys.end(); ++b) {
		if (!b->background || (b->frame != _frame))
			continue;

		b->background->draw(*_vm->_draw->_backSurface);

		_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, 0, 0, 319, 199);
	}

	// Handle the animation keys, updating the objects
	for (AnimationKeys::const_iterator a = _animKeys.begin(); a != _animKeys.end(); ++a) {
		if (a->frame != _frame)
			continue;

		Object &object = _objects[a->object];

		delete object.object;
		object.object = 0;

		// No valid animation => remove
		if ((a->animation == 0xFFFF) || !a->ani)
			continue;

		// Change the animation

		object.object = new ANIObject(*a->ani);

		object.object->setAnimation(a->animation);
		object.object->setPosition(a->x, a->y);
		object.object->setVisible(true);
		object.object->setPause(false);

		object.order = a->order;
	}

	// Draw the animations
	drawAnims();
}

// NOTE: This is really not at all efficient. However, since there's only a
//       small number of objects, it should matter. We really do need a stable
//       sort, though, so Common::sort() is out.
SEQFile::Objects SEQFile::getOrderedObjects() {
	int16 minOrder = (int16)0x7FFF;
	int16 maxOrder = (int16)0x8000;

	Objects objects;

	// Find the span of order values
	for (uint i = 0; i < kObjectCount; i++) {
		if (!_objects[i].object)
			continue;

		minOrder = MIN(minOrder, _objects[i].order);
		maxOrder = MAX(maxOrder, _objects[i].order);
	}

	// Stably sort the objects by order value
	for (int16 o = minOrder; o <= maxOrder; o++)
		for (uint i = 0; i < kObjectCount; i++)
			if (_objects[i].object && (_objects[i].order == o))
				objects.push_back(_objects[i]);

	return objects;
}

void SEQFile::clearAnims() {
	Objects objects = getOrderedObjects();

	// Remove the animation frames, in reverse drawing order
	for (Objects::iterator o = objects.reverse_begin(); o != objects.end(); --o) {
		int16 left, top, right, bottom;

		if (o->object->clear(*_vm->_draw->_backSurface, left, top, right, bottom))
			_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, left, top, right, bottom);
	}
}

void SEQFile::drawAnims() {
	Objects objects = getOrderedObjects();

	// Draw the animation frames and advance the animation
	for (Objects::iterator o = objects.begin(); o != objects.end(); ++o) {
		int16 left, top, right, bottom;

		if (o->object->draw(*_vm->_draw->_backSurface, left, top, right, bottom))
			_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, left, top, right, bottom);

		o->object->advance();
	}
}

uint16 SEQFile::getFrame() const {
	return _frame;
}

void SEQFile::seekFrame(uint16 frame) {
	_frame = frame;
}

uint SEQFile::addLoop(uint16 startFrame, uint16 endFrame, uint16 loopCount) {
	_loops.resize(_loops.size() + 1);

	_loops.back().startFrame  = startFrame;
	_loops.back().endFrame    = endFrame;
	_loops.back().loopCount   = loopCount;
	_loops.back().currentLoop = 0;
	_loops.back().empty       = false;

	return _loops.size() - 1;
}

void SEQFile::skipLoop(uint loopID) {
	if (loopID >= _loops.size())
		return;

	_loops[loopID].currentLoop = 0xFFFF;
}

void SEQFile::delLoop(uint loopID) {
	if (loopID >= _loops.size())
		return;

	_loops[loopID].empty = true;

	cleanLoops();
}

void SEQFile::cleanLoops() {
	while (!_loops.empty() && _loops.back().empty)
		_loops.pop_back();
}

void SEQFile::abortPlay() {
	_abortPlay = true;
}

void SEQFile::handleFrameEvent() {
}

void SEQFile::handleInput(int16 key, int16 mouseX, int16 mouseY, MouseButtons mouseButtons) {
	if (_abortable && ((key != 0) || (mouseButtons != kMouseButtonsNone)))
		abortPlay();
}

} // End of namespace Gob
